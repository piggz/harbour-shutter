/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2019, Google Inc.
 * Copyright (C) 2020, Raspberry Pi Ltd
 *
 * v4l2_pixelformat.cpp - V4L2 Pixel Format
 */

#include <functional>
#include "v4l2_pixelformat.h"

#include <ctype.h>
#include <linux/videodev2.h>
#include <map>
#include <string.h>

#include <libcamera/pixel_format.h>
#include <libcamera/formats.h>
#include "formats.h"

/**
 * \file v4l2_pixelformat.h
 * \brief V4L2 Pixel Format
 */

/**
 * \class SHV4L2PixelFormat
 * \brief V4L2 pixel format FourCC wrapper
 *
 * The SHV4L2PixelFormat class describes the pixel format of a V4L2 buffer. It
 * wraps the V4L2 numerical FourCC, and shall be used in all APIs that deal with
 * V4L2 pixel libcamera::formats. Its purpose is to prevent unintentional confusion of
 * V4L2 and DRM FourCCs in code by catching implicit conversion attempts at
 * compile time.
 *
 * To achieve this goal, construction of a SHV4L2PixelFormat from an integer value
 * is explicit. To retrieve the integer value of a SHV4L2PixelFormat, both the
 * explicit value() and implicit uint32_t conversion operators may be used.
 */

const std::map<SHV4L2PixelFormat, SHV4L2PixelFormat::Info> vpf2pf{
    /* RGB libcamera::formats. */
    { SHV4L2PixelFormat(V4L2_PIX_FMT_RGB565),
        { libcamera::formats::RGB565, "16-bit RGB 5-6-5" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_RGB565X),
        { libcamera::formats::RGB565_BE, "16-bit RGB 5-6-5 BE" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_RGB24),
        { libcamera::formats::BGR888, "24-bit RGB 8-8-8" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_BGR24),
        { libcamera::formats::RGB888, "24-bit BGR 8-8-8" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_XBGR32),
        { libcamera::formats::XRGB8888, "32-bit BGRX 8-8-8-8" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_XRGB32),
        { libcamera::formats::BGRX8888, "32-bit XRGB 8-8-8-8" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_RGBX32),
        { libcamera::formats::XBGR8888, "32-bit RGBX 8-8-8-8" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_BGRX32),
        { libcamera::formats::RGBX8888, "32-bit XBGR 8-8-8-8" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_RGBA32),
        { libcamera::formats::ABGR8888, "32-bit RGBA 8-8-8-8" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_ABGR32),
        { libcamera::formats::ARGB8888, "32-bit BGRA 8-8-8-8" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_ARGB32),
        { libcamera::formats::BGRA8888, "32-bit ARGB 8-8-8-8" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_BGRA32),
        { libcamera::formats::RGBA8888, "32-bit ABGR 8-8-8-8" } },

    /* YUV packed libcamera::formats. */
    { SHV4L2PixelFormat(V4L2_PIX_FMT_YUYV),
        { libcamera::formats::YUYV, "YUYV 4:2:2" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_YVYU),
        { libcamera::formats::YVYU, "YVYU 4:2:2" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_UYVY),
        { libcamera::formats::UYVY, "UYVY 4:2:2" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_VYUY),
        { libcamera::formats::VYUY, "VYUY 4:2:2" } },
    /*{ SHV4L2PixelFormat(V4L2_PIX_FMT_YUVA32),
        { libcamera::formats::AVUY8888, "32-bit YUVA 8-8-8-8" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_YUVX32),
        { libcamera::formats::XVUY8888, "32-bit YUVX 8-8-8-8" } },*/

    /* YUV planar libcamera::formats. */
    { SHV4L2PixelFormat(V4L2_PIX_FMT_NV16),
        { libcamera::formats::NV16, "Y/CbCr 4:2:2" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_NV16M),
        { libcamera::formats::NV16, "Y/CbCr 4:2:2 (N-C)" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_NV61),
        { libcamera::formats::NV61, "Y/CrCb 4:2:2" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_NV61M),
        { libcamera::formats::NV61, "Y/CrCb 4:2:2 (N-C)" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_NV12),
        { libcamera::formats::NV12, "Y/CbCr 4:2:0" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_NV12M),
        { libcamera::formats::NV12, "Y/CbCr 4:2:0 (N-C)" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_NV21),
        { libcamera::formats::NV21, "Y/CrCb 4:2:0" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_NV21M),
        { libcamera::formats::NV21, "Y/CrCb 4:2:0 (N-C)" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_NV24),
        { libcamera::formats::NV24, "Y/CbCr 4:4:4" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_NV42),
        { libcamera::formats::NV42, "Y/CrCb 4:4:4" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_YUV420),
        { libcamera::formats::YUV420, "Planar YUV 4:2:0" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_YUV420M),
        { libcamera::formats::YUV420, "Planar YUV 4:2:0 (N-C)" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_YVU420),
        { libcamera::formats::YVU420, "Planar YVU 4:2:0" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_YVU420M),
        { libcamera::formats::YVU420, "Planar YVU 4:2:0 (N-C)" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_YUV422P),
        { libcamera::formats::YUV422, "Planar YUV 4:2:2" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_YUV422M),
        { libcamera::formats::YUV422, "Planar YUV 4:2:2 (N-C)" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_YVU422M),
        { libcamera::formats::YVU422, "Planar YVU 4:2:2 (N-C)" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_YUV444M),
        { libcamera::formats::YUV444, "Planar YUV 4:4:4 (N-C)" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_YUV444M),
        { libcamera::formats::YVU444, "Planar YVU 4:4:4 (N-C)" } },

    /* Greyscale libcamera::formats. */
    { SHV4L2PixelFormat(V4L2_PIX_FMT_GREY),
        { libcamera::formats::R8, "8-bit Greyscale" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_Y10),
        { libcamera::formats::R10, "10-bit Greyscale" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_Y10P),
        { libcamera::formats::R10_CSI2P, "10-bit Greyscale Packed" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_Y12),
        { libcamera::formats::R12, "12-bit Greyscale" } },

    /* Bayer libcamera::formats. */
    { SHV4L2PixelFormat(V4L2_PIX_FMT_SBGGR8),
        { libcamera::formats::SBGGR8, "8-bit Bayer BGBG/GRGR" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_SGBRG8),
        { libcamera::formats::SGBRG8, "8-bit Bayer GBGB/RGRG" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_SGRBG8),
        { libcamera::formats::SGRBG8, "8-bit Bayer GRGR/BGBG" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_SRGGB8),
        { libcamera::formats::SRGGB8, "8-bit Bayer RGRG/GBGB" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_SBGGR10),
        { libcamera::formats::SBGGR10, "10-bit Bayer BGBG/GRGR" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_SGBRG10),
        { libcamera::formats::SGBRG10, "10-bit Bayer GBGB/RGRG" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_SGRBG10),
        { libcamera::formats::SGRBG10, "10-bit Bayer GRGR/BGBG" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_SRGGB10),
        { libcamera::formats::SRGGB10, "10-bit Bayer RGRG/GBGB" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_SBGGR10P),
        { libcamera::formats::SBGGR10_CSI2P, "10-bit Bayer BGBG/GRGR Packed" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_SGBRG10P),
        { libcamera::formats::SGBRG10_CSI2P, "10-bit Bayer GBGB/RGRG Packed" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_SGRBG10P),
        { libcamera::formats::SGRBG10_CSI2P, "10-bit Bayer GRGR/BGBG Packed" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_SRGGB10P),
        { libcamera::formats::SRGGB10_CSI2P, "10-bit Bayer RGRG/GBGB Packed" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_SBGGR12),
        { libcamera::formats::SBGGR12, "12-bit Bayer BGBG/GRGR" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_SGBRG12),
        { libcamera::formats::SGBRG12, "12-bit Bayer GBGB/RGRG" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_SGRBG12),
        { libcamera::formats::SGRBG12, "12-bit Bayer GRGR/BGBG" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_SRGGB12),
        { libcamera::formats::SRGGB12, "12-bit Bayer RGRG/GBGB" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_SBGGR12P),
        { libcamera::formats::SBGGR12_CSI2P, "12-bit Bayer BGBG/GRGR Packed" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_SGBRG12P),
        { libcamera::formats::SGBRG12_CSI2P, "12-bit Bayer GBGB/RGRG Packed" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_SGRBG12P),
        { libcamera::formats::SGRBG12_CSI2P, "12-bit Bayer GRGR/BGBG Packed" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_SRGGB12P),
        { libcamera::formats::SRGGB12_CSI2P, "12-bit Bayer RGRG/GBGB Packed" } },
    /*{ SHV4L2PixelFormat(V4L2_PIX_FMT_SBGGR14),
        { libcamera::formats::SBGGR14, "14-bit Bayer BGBG/GRGR" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_SGBRG14),
        { libcamera::formats::SGBRG14, "14-bit Bayer GBGB/RGRG" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_SGRBG14),
        { libcamera::formats::SGRBG14, "14-bit Bayer GRGR/BGBG" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_SRGGB14),
        { libcamera::formats::SRGGB14, "14-bit Bayer RGRG/GBGB" } },*/
    { SHV4L2PixelFormat(V4L2_PIX_FMT_SBGGR14P),
        { libcamera::formats::SBGGR14_CSI2P, "14-bit Bayer BGBG/GRGR Packed" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_SGBRG14P),
        { libcamera::formats::SGBRG14_CSI2P, "14-bit Bayer GBGB/RGRG Packed" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_SGRBG14P),
        { libcamera::formats::SGRBG14_CSI2P, "14-bit Bayer GRGR/BGBG Packed" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_SRGGB14P),
        { libcamera::formats::SRGGB14_CSI2P, "14-bit Bayer RGRG/GBGB Packed" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_SBGGR16),
        { libcamera::formats::SBGGR16, "16-bit Bayer BGBG/GRGR" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_SGBRG16),
        { libcamera::formats::SGBRG16, "16-bit Bayer GBGB/RGRG" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_SGRBG16),
        { libcamera::formats::SGRBG16, "16-bit Bayer GRGR/BGBG" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_SRGGB16),
        { libcamera::formats::SRGGB16, "16-bit Bayer RGRG/GBGB" } },

    /* Compressed libcamera::formats. */
    { SHV4L2PixelFormat(V4L2_PIX_FMT_MJPEG),
        { libcamera::formats::MJPEG, "Motion-JPEG" } },
    { SHV4L2PixelFormat(V4L2_PIX_FMT_JPEG),
        { libcamera::formats::MJPEG, "JPEG JFIF" } },
};

/**
 * \struct SHV4L2PixelFormat::Info
 * \brief Information about a V4L2 pixel format
 *
 * \var SHV4L2PixelFormat::Info::format
 * \brief The corresponding libcamera PixelFormat
 *
 * \sa PixelFormat
 *
 * \var SHV4L2PixelFormat::Info::description
 * \brief The human-readable description of the V4L2 pixel format
 */

/**
 * \fn SHV4L2PixelFormat::SHV4L2PixelFormat()
 * \brief Construct a SHV4L2PixelFormat with an invalid format
 *
 * SHV4L2PixelFormat instances constructed with the default constructor are
 * invalid, calling the isValid() function returns false.
 */

/**
 * \fn SHV4L2PixelFormat::SHV4L2PixelFormat(uint32_t fourcc)
 * \brief Construct a SHV4L2PixelFormat from a FourCC value
 * \param[in] fourcc The pixel format FourCC numerical value
 */

/**
 * \fn bool SHV4L2PixelFormat::isValid() const
 * \brief Check if the pixel format is valid
 *
 * SHV4L2PixelFormat instances constructed with the default constructor are
 * invalid. Instances constructed with a FourCC defined in the V4L2 API are
 * valid. The behaviour is undefined otherwise.
 *
 * \return True if the pixel format is valid, false otherwise
 */

/**
 * \fn uint32_t SHV4L2PixelFormat::fourcc() const
 * \brief Retrieve the pixel format FourCC numerical value
 * \return The pixel format FourCC numerical value
 */

/**
 * \fn SHV4L2PixelFormat::operator uint32_t() const
 * \brief Convert to the pixel format FourCC numerical value
 * \return The pixel format FourCC numerical value
 */

/**
 * \brief Assemble and return a string describing the pixel format
 * \return A string describing the pixel format
 */
std::string SHV4L2PixelFormat::toString() const
{
	if (fourcc_ == 0)
		return "<INVALID>";

	char ss[8] = { static_cast<char>(fourcc_ & 0x7f),
		       static_cast<char>((fourcc_ >> 8) & 0x7f),
		       static_cast<char>((fourcc_ >> 16) & 0x7f),
		       static_cast<char>((fourcc_ >> 24) & 0x7f) };

	for (unsigned int i = 0; i < 4; i++) {
		if (!isprint(ss[i]))
			ss[i] = '.';
	}

	if (fourcc_ & (1 << 31))
		strcat(ss, "-BE");

	return ss;
}

/**
 * \brief Retrieve the V4L2 description for the format
 *
 * The description matches the value used by the kernel, as would be reported
 * by the VIDIOC_ENUM_FMT ioctl.
 *
 * \return The V4L2 description corresponding to the V4L2 format, or a
 * placeholder description if not found
 */
const char *SHV4L2PixelFormat::description() const
{
	const auto iter = vpf2pf.find(*this);
	if (iter == vpf2pf.end()) {
		return "Unsupported format";
	}

	return iter->second.description;
}

/**
 * \brief Convert the V4L2 pixel format to the corresponding PixelFormat
 * \param[in] warn When true, log a warning message if the V4L2 pixel format
 * isn't known
 *
 * Users of this function might try to convert a SHV4L2PixelFormat to a
 * PixelFormat just to check if the format is supported or not. In that case,
 * they can suppress the warning message by setting the \a warn argument to
 * false to not pollute the log with unnecessary messages.
 *
 * \return The PixelFormat corresponding to the V4L2 pixel format
 */
libcamera::PixelFormat SHV4L2PixelFormat::toPixelFormat(bool warn) const
{
	const auto iter = vpf2pf.find(*this);
	if (iter == vpf2pf.end()) {
        return libcamera::PixelFormat();
	}

	return iter->second.format;
}

/**
 * \brief Retrieve the list of SHV4L2PixelFormat associated with \a pixelFormat
 * \param[in] pixelFormat The PixelFormat to convert
 *
 * Multiple V4L2 libcamera::formats may exist for one PixelFormat as V4L2 defines separate
 * 4CCs for contiguous and non-contiguous versions of the same image format.
 *
 * \return The list of SHV4L2PixelFormat corresponding to \a pixelFormat
 */
/*
const std::vector<SHV4L2PixelFormat> &
SHV4L2PixelFormat::fromPixelFormat(const libcamera::PixelFormat &pixelFormat)
{
    static const std::vector<SHV4L2PixelFormat> empty;

    const SHPixelFormatInfo &info = libcamera::PixelFormat::info(pixelFormat);
	if (!info.isValid())
		return empty;

    return info.v4l2libcamera::formats;
}*/

/**
 * \brief Insert a text representation of a SHV4L2PixelFormat into an output
 * stream
 * \param[in] out The output stream
 * \param[in] f The SHV4L2PixelFormat
 * \return The output stream \a out
 */
std::ostream &operator<<(std::ostream &out, const SHV4L2PixelFormat &f)
{
	out << f.toString();
	return out;
}
