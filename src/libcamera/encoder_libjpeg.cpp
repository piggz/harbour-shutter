/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2020, Google Inc.
 *
 * encoder_libjpeg.cpp - JPEG encoding using libjpeg native API
 */

#include "encoder_libjpeg.h"

#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <libcamera/libcamera/formats.h>
#include <sstream>
#include <string.h>
#include <unistd.h>
#include <vector>

#include <libcamera/camera.h>
#include <libcamera/pixel_format.h>

using namespace libcamera;


namespace {

struct JPEGPixelFormatInfo {
	J_COLOR_SPACE colorSpace;
    const SHPixelFormatInfo &pixelFormatInfo;
	bool nvSwap;
};

const std::map<PixelFormat, JPEGPixelFormatInfo> pixelInfo{
    { libcamera::formats::R8, { JCS_GRAYSCALE, SHPixelFormatInfo::info(libcamera::formats::R8), false } },

    { libcamera::formats::RGB888, { JCS_EXT_BGR, SHPixelFormatInfo::info(libcamera::formats::RGB888), false } },
    { libcamera::formats::BGR888, { JCS_EXT_RGB, SHPixelFormatInfo::info(libcamera::formats::BGR888), false } },

    { libcamera::formats::NV12, { JCS_YCbCr, SHPixelFormatInfo::info(libcamera::formats::NV12), false } },
    { libcamera::formats::NV21, { JCS_YCbCr, SHPixelFormatInfo::info(libcamera::formats::NV21), true } },
    { libcamera::formats::NV16, { JCS_YCbCr, SHPixelFormatInfo::info(libcamera::formats::NV16), false } },
    { libcamera::formats::NV61, { JCS_YCbCr, SHPixelFormatInfo::info(libcamera::formats::NV61), true } },
    { libcamera::formats::NV24, { JCS_YCbCr, SHPixelFormatInfo::info(libcamera::formats::NV24), false } },
    { libcamera::formats::NV42, { JCS_YCbCr, SHPixelFormatInfo::info(libcamera::formats::NV42), true } },
    { libcamera::formats::YUYV, { JCS_YCbCr, SHPixelFormatInfo::info(libcamera::formats::YUYV), true } },
};

const struct JPEGPixelFormatInfo &findPixelInfo(const PixelFormat &format)
{
    static const struct JPEGPixelFormatInfo invalidPixelFormat {
        JCS_UNKNOWN, SHPixelFormatInfo(), false
	};

	const auto iter = pixelInfo.find(format);
	if (iter == pixelInfo.end()) {
//		LOG(JPEG, Error) << "Unsupported pixel format for JPEG encoder: "
//				 << format;
		return invalidPixelFormat;
	}

	return iter->second;
}

} /* namespace */

EncoderLibJpeg::EncoderLibJpeg()
{
	/* \todo Expand error handling coverage with a custom handler. */
	compress_.err = jpeg_std_error(&jerr_);

	jpeg_create_compress(&compress_);
}

EncoderLibJpeg::~EncoderLibJpeg()
{
	jpeg_destroy_compress(&compress_);
}

int EncoderLibJpeg::configure(const StreamConfiguration &cfg)
{
    const struct JPEGPixelFormatInfo info = findPixelInfo(cfg.pixelFormat);
	if (info.colorSpace == JCS_UNKNOWN)
		return -ENOTSUP;

	compress_.image_width = cfg.size.width;
	compress_.image_height = cfg.size.height;
	compress_.in_color_space = info.colorSpace;

	compress_.input_components = info.colorSpace == JCS_GRAYSCALE ? 1 : 3;

	jpeg_set_defaults(&compress_);

    pixelFormatInfo_ = &info.pixelFormatInfo;

    nv_ = pixelFormatInfo_->numPlanes() == 2;
	nvSwap_ = info.nvSwap;

	return 0;
}

void EncoderLibJpeg::compressRGB(const std::vector<Span<uint8_t>> &planes)
{
	unsigned char *src = const_cast<unsigned char *>(planes[0].data());
	/* \todo Stride information should come from buffer configuration. */
    unsigned int stride = pixelFormatInfo_->stride(compress_.image_width, 0);

	JSAMPROW row_pointer[1];

	while (compress_.next_scanline < compress_.image_height) {
		row_pointer[0] = &src[compress_.next_scanline * stride];
		jpeg_write_scanlines(&compress_, row_pointer, 1);
	}
}

/*
 * Compress the incoming buffer from a supported NV format.
 * This naively unpacks the semi-planar NV12 to a YUV888 format for libjpeg.
 */
void EncoderLibJpeg::compressNV(const std::vector<Span<uint8_t>> &planes)
{
	uint8_t tmprowbuf[compress_.image_width * 3];

	/*
	 * \todo Use the raw api, and only unpack the cb/cr samples to new line
	 * buffers. If possible, see if we can set appropriate pixel strides
	 * too to save even that copy.
	 *
	 * Possible hints at:
	 * https://sourceforge.net/p/libjpeg/mailman/message/30815123/
	 */
    unsigned int y_stride = pixelFormatInfo_->stride(compress_.image_width, 0);
    unsigned int c_stride = pixelFormatInfo_->stride(compress_.image_width, 1);

	unsigned int horzSubSample = 2 * compress_.image_width / c_stride;
    unsigned int vertSubSample = pixelFormatInfo_->planes[1].verticalSubSampling;

	unsigned int c_inc = horzSubSample == 1 ? 2 : 0;
	unsigned int cb_pos = nvSwap_ ? 1 : 0;
	unsigned int cr_pos = nvSwap_ ? 0 : 1;

	const unsigned char *src = planes[0].data();
	const unsigned char *src_c = planes[1].data();

	JSAMPROW row_pointer[1];
	row_pointer[0] = &tmprowbuf[0];

	for (unsigned int y = 0; y < compress_.image_height; y++) {
		unsigned char *dst = &tmprowbuf[0];

		const unsigned char *src_y = src + y * y_stride;
		const unsigned char *src_cb = src_c + (y / vertSubSample) * c_stride + cb_pos;
		const unsigned char *src_cr = src_c + (y / vertSubSample) * c_stride + cr_pos;

		for (unsigned int x = 0; x < compress_.image_width; x += 2) {
			dst[0] = *src_y;
			dst[1] = *src_cb;
			dst[2] = *src_cr;
			src_y++;
			src_cb += c_inc;
			src_cr += c_inc;
			dst += 3;

			dst[0] = *src_y;
			dst[1] = *src_cb;
			dst[2] = *src_cr;
			src_y++;
			src_cb += 2;
			src_cr += 2;
			dst += 3;
		}

		jpeg_write_scanlines(&compress_, row_pointer, 1);
	}
}

/*
int EncoderLibJpeg::encode(Camera3RequestDescriptor::StreamBuffer *buffer,
			   libcamera::Span<const uint8_t> exifData,
			   unsigned int quality)
{
	MappedFrameBuffer frame(buffer->srcBuffer,
				MappedFrameBuffer::MapFlag::Read);
	if (!frame.isValid()) {
		LOG(JPEG, Error) << "Failed to map FrameBuffer : "
				 << strerror(frame.error());
		return frame.error();
	}

	return encode(frame.planes(), buffer->dstBuffer->plane(0),
		      exifData, quality);
}
*/
bool EncoderLibJpeg::encode(const std::vector<Span<uint8_t>> &src,
               std::string outFileName,
               Span<const uint8_t> exifData,
			   unsigned int quality)
{
    //unsigned char *destination = dest.data();
    unsigned long size = 0;;

	/*
	 * The jpeg_mem_dest will reallocate if the required size is not
	 * sufficient. That means the output won't be written to the correct
	 * buffers.
	 *
	 * \todo Implement our own custom memory destination to prevent
	 * reallocation and prefer failure with correct reporting.
	 */
    //jpeg_mem_dest(&compress_, &destination, &size);
    FILE * outfile;
    if ((outfile = fopen(outFileName.c_str(), "wb")) == NULL) {
        std::cerr << "can't open " << outFileName << "\n";
        exit(1);
    }
    jpeg_stdio_dest(&compress_, outfile);

    jpeg_set_defaults(&compress_);

    jpeg_set_quality(&compress_, quality, TRUE);

	jpeg_start_compress(&compress_, TRUE);

	if (exifData.size())
		/* Store Exif data in the JPEG_APP1 data block. */
		jpeg_write_marker(&compress_, JPEG_APP0 + 1,
				  static_cast<const JOCTET *>(exifData.data()),
				  exifData.size());

//	LOG(JPEG, Debug) << "JPEG Encode Starting:" << compress_.image_width
//			 << "x" << compress_.image_height;

//	ASSERT(src.size() == SHPixelFormatInfo_->numPlanes());

	if (nv_)
		compressNV(src);
	else
		compressRGB(src);

	jpeg_finish_compress(&compress_);

    return true;
}
