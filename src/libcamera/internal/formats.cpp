/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2019, Google Inc.
 *
 * formats.cpp - libcamera image formats
 */

#include "formats.h"

#include <algorithm>
#include <errno.h>

#include <libcamera/formats.h>
#include <linux/videodev2.h>

/**
 * \file internal/formats.h
 * \brief Types and helper functions to handle libcamera image formats
 */

/**
 * \class SHPixelFormatInfo
 * \brief Information about pixel formats
 *
 * The SHPixelFormatInfo class groups together information describing a pixel
 * format. It facilitates handling of pixel formats by providing data commonly
 * used in pipeline handlers.
 *
 * \var SHPixelFormatInfo::name
 * \brief The format name as a human-readable string, used as the test
 * representation of the PixelFormat
 *
 * \var SHPixelFormatInfo::format
 * \brief The PixelFormat described by this instance
 *
 * \var SHPixelFormatInfo::v4l2Formats
 * \brief The V4L2 pixel formats corresponding to the PixelFormat
 *
 * Multiple V4L2 formats may exist for one PixelFormat, as V4L2 defines
 * separate 4CCs for contiguous and non-contiguous versions of the same image
 * format.
 *
 * \var SHPixelFormatInfo::bitsPerPixel
 * \brief The average number of bits per pixel
 *
 * The number per pixel averages the total number of bits for all colour
 * components over the whole image, excluding any padding bits or padding
 * pixels.
 *
 * For formats that store pixels with bit padding within words, only the
 * effective bits are taken into account. For instance, 12-bit Bayer data
 * stored in two bytes per pixel report 12, not 16, in this field.
 *
 * Formats that don't have a fixed number of bits per pixel, such as compressed
 * formats, report 0 in this field.
 *
 * \var SHPixelFormatInfo::colourEncoding
 * \brief The colour encoding type
 *
 * \var SHPixelFormatInfo::packed
 * \brief Tell if multiple pixels are packed in the same bytes
 *
 * Packed formats are defined as storing data from multiple pixels in the same
 * bytes. For instance, 12-bit Bayer data with two pixels stored in three bytes
 * is packed, while the same data stored with 4 bits of padding in two bytes
 * per pixel is not packed.
 *
 * \var SHPixelFormatInfo::pixelsPerGroup
 * \brief The number of pixels in a pixel group
 *
 * A pixel group is defined as the minimum number of pixels (including padding)
 * necessary in a row when the image has only one column of effective pixels.
 * pixelsPerGroup refers to this value. SHPixelFormatInfo::Plane::bytesPerGroup,
 * then, refers to the number of bytes that a pixel group consumes. This
 * definition of a pixel group allows simple calculation of stride, as
 * ceil(width / pixelsPerGroup) * bytesPerGroup. These values are determined
 * only in terms of a row. The ceiling accounts for padding.
 *
 * A pixel group has a second constraint, such that the pixel group
 * (bytesPerGroup and pixelsPerGroup) is the smallest repeatable unit.
 * What this means is that, for example, in the IPU3 formats, if there is only
 * one column of effective pixels, it looks like it could be fit in 5 bytes
 * with 3 padding pixels (for a total of 4 pixels over 5 bytes). However, this
 * unit is not repeatable, as at the 7th group in the same row, the pattern
 * is broken. Therefore, the pixel group for IPU3 formats must be 25 pixels
 * over 32 bytes.
 *
 * For example, for something simple like BGR888, it is self-explanatory:
 * the pixel group size is 1, and the bytes necessary is 3, and there is
 * only one plane with no (= 1) vertical subsampling. For YUYV, the
 * CbCr pair is shared between two pixels, so even if you have only one
 * pixel, you would still need a padded second Y sample, therefore the pixel
 * group size is 2, and bytes necessary is 4. YUYV also has no vertical
 * subsampling. NV12 has a pixel group size of 2 pixels, due to the CbCr plane.
 * The bytes per group then, for both planes, is 2. The first plane has no
 * vertical subsampling, but the second plane is subsampled by a factor of 2.
 *
 * The IPU3 raw Bayer formats are single-planar, and have a pixel group size of
 * 25, consuming 32 bytes, due to the packing pattern being repeated in memory
 * every 32 bytes. The IPU3 hardware, however, has an additional constraint on
 * the DMA burst size, requiring lines to be multiple of 64 bytes. This isn't an
 * intrinsic property of the formats and is thus not reflected here. It is
 * instead enforced by the corresponding pipeline handler.
 *
 * \var SHPixelFormatInfo::planes
 * \brief Information about pixels for each plane
 *
 * \sa SHPixelFormatInfo::Plane
 */

/**
 * \enum SHPixelFormatInfo::ColourEncoding
 * \brief The colour encoding type
 *
 * \var SHPixelFormatInfo::ColourEncodingRGB
 * \brief RGB colour encoding
 *
 * \var SHPixelFormatInfo::ColourEncodingYUV
 * \brief YUV colour encoding
 *
 * \var SHPixelFormatInfo::ColourEncodingRAW
 * \brief RAW colour encoding
 */

/**
 * \struct SHPixelFormatInfo::Plane
 * \brief Information about a single plane of a pixel format
 *
 * \var SHPixelFormatInfo::Plane::bytesPerGroup
 * \brief The number of bytes that a pixel group consumes
 *
 * \sa SHPixelFormatInfo::pixelsPerGroup
 *
 * \var SHPixelFormatInfo::Plane::verticalSubSampling
 * \brief Vertical subsampling multiplier
 *
 * This value is the ratio between the number of rows of pixels in the frame
 * to the number of rows of pixels in the plane.
 */

const SHPixelFormatInfo SHPixelFormatInfoInvalid{};

const std::map<libcamera::PixelFormat, SHPixelFormatInfo> pixelFormatInfo{
	/* RGB formats. */
    { libcamera::formats::RGB565, {
		.name = "RGB565",
        .format = libcamera::formats::RGB565,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_RGB565), },
		.bitsPerPixel = 16,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRGB,
		.packed = false,
		.pixelsPerGroup = 1,
		.planes = {{ { 3, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::RGB565_BE, {
		.name = "RGB565_BE",
        .format = libcamera::formats::RGB565_BE,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_RGB565X), },
		.bitsPerPixel = 16,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRGB,
		.packed = false,
		.pixelsPerGroup = 1,
		.planes = {{ { 3, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::BGR888, {
		.name = "BGR888",
        .format = libcamera::formats::BGR888,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_RGB24), },
		.bitsPerPixel = 24,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRGB,
		.packed = false,
		.pixelsPerGroup = 1,
		.planes = {{ { 3, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::RGB888, {
		.name = "RGB888",
        .format = libcamera::formats::RGB888,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_BGR24), },
		.bitsPerPixel = 24,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRGB,
		.packed = false,
		.pixelsPerGroup = 1,
		.planes = {{ { 3, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::XRGB8888, {
		.name = "XRGB8888",
        .format = libcamera::formats::XRGB8888,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_XBGR32), },
		.bitsPerPixel = 32,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRGB,
		.packed = false,
		.pixelsPerGroup = 1,
		.planes = {{ { 4, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::XBGR8888, {
		.name = "XBGR8888",
        .format = libcamera::formats::XBGR8888,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_RGBX32), },
		.bitsPerPixel = 32,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRGB,
		.packed = false,
		.pixelsPerGroup = 1,
		.planes = {{ { 4, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::RGBX8888, {
		.name = "RGBX8888",
        .format = libcamera::formats::RGBX8888,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_BGRX32), },
		.bitsPerPixel = 32,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRGB,
		.packed = false,
		.pixelsPerGroup = 1,
		.planes = {{ { 4, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::BGRX8888, {
		.name = "BGRX8888",
        .format = libcamera::formats::BGRX8888,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_XRGB32), },
		.bitsPerPixel = 32,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRGB,
		.packed = false,
		.pixelsPerGroup = 1,
		.planes = {{ { 4, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::ABGR8888, {
		.name = "ABGR8888",
        .format = libcamera::formats::ABGR8888,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_RGBA32), },
		.bitsPerPixel = 32,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRGB,
		.packed = false,
		.pixelsPerGroup = 1,
		.planes = {{ { 4, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::ARGB8888, {
		.name = "ARGB8888",
        .format = libcamera::formats::ARGB8888,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_ABGR32), },
		.bitsPerPixel = 32,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRGB,
		.packed = false,
		.pixelsPerGroup = 1,
		.planes = {{ { 4, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::BGRA8888, {
		.name = "BGRA8888",
        .format = libcamera::formats::BGRA8888,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_ARGB32), },
		.bitsPerPixel = 32,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRGB,
		.packed = false,
		.pixelsPerGroup = 1,
		.planes = {{ { 4, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::RGBA8888, {
		.name = "RGBA8888",
        .format = libcamera::formats::RGBA8888,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_BGRA32), },
		.bitsPerPixel = 32,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRGB,
		.packed = false,
		.pixelsPerGroup = 1,
		.planes = {{ { 4, 1 }, { 0, 0 }, { 0, 0 } }},
	} },

	/* YUV packed formats. */
    { libcamera::formats::YUYV, {
		.name = "YUYV",
        .format = libcamera::formats::YUYV,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_YUYV), },
		.bitsPerPixel = 16,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingYUV,
		.packed = false,
		.pixelsPerGroup = 2,
		.planes = {{ { 4, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::YVYU, {
		.name = "YVYU",
        .format = libcamera::formats::YVYU,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_YVYU), },
		.bitsPerPixel = 16,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingYUV,
		.packed = false,
		.pixelsPerGroup = 2,
		.planes = {{ { 4, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::UYVY, {
		.name = "UYVY",
        .format = libcamera::formats::UYVY,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_UYVY), },
		.bitsPerPixel = 16,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingYUV,
		.packed = false,
		.pixelsPerGroup = 2,
		.planes = {{ { 4, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::VYUY, {
		.name = "VYUY",
        .format = libcamera::formats::VYUY,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_VYUY), },
		.bitsPerPixel = 16,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingYUV,
		.packed = false,
		.pixelsPerGroup = 2,
		.planes = {{ { 4, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::AVUY8888, {
		.name = "AVUY8888",
        .format = libcamera::formats::AVUY8888,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_YUVA32), },
		.bitsPerPixel = 32,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingYUV,
		.packed = false,
		.pixelsPerGroup = 1,
		.planes = {{ { 4, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::XVUY8888, {
		.name = "XVUY8888",
        .format = libcamera::formats::XVUY8888,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_YUVX32), },
		.bitsPerPixel = 32,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingYUV,
		.packed = false,
		.pixelsPerGroup = 1,
		.planes = {{ { 4, 1 }, { 0, 0 }, { 0, 0 } }},
	} },

	/* YUV planar formats. */
    { libcamera::formats::NV12, {
		.name = "NV12",
        .format = libcamera::formats::NV12,
		.v4l2Formats = {
            SHV4L2PixelFormat(V4L2_PIX_FMT_NV12),
            SHV4L2PixelFormat(V4L2_PIX_FMT_NV12M),
		},
		.bitsPerPixel = 12,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingYUV,
		.packed = false,
		.pixelsPerGroup = 2,
		.planes = {{ { 2, 1 }, { 2, 2 }, { 0, 0 } }},
	} },
    { libcamera::formats::NV21, {
		.name = "NV21",
        .format = libcamera::formats::NV21,
		.v4l2Formats = {
            SHV4L2PixelFormat(V4L2_PIX_FMT_NV21),
            SHV4L2PixelFormat(V4L2_PIX_FMT_NV21M),
		},
		.bitsPerPixel = 12,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingYUV,
		.packed = false,
		.pixelsPerGroup = 2,
		.planes = {{ { 2, 1 }, { 2, 2 }, { 0, 0 } }},
	} },
    { libcamera::formats::NV16, {
		.name = "NV16",
        .format = libcamera::formats::NV16,
		.v4l2Formats = {
            SHV4L2PixelFormat(V4L2_PIX_FMT_NV16),
            SHV4L2PixelFormat(V4L2_PIX_FMT_NV16M),
		},
		.bitsPerPixel = 16,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingYUV,
		.packed = false,
		.pixelsPerGroup = 2,
		.planes = {{ { 2, 1 }, { 2, 1 }, { 0, 0 } }},
	} },
    { libcamera::formats::NV61, {
		.name = "NV61",
        .format = libcamera::formats::NV61,
		.v4l2Formats = {
            SHV4L2PixelFormat(V4L2_PIX_FMT_NV61),
            SHV4L2PixelFormat(V4L2_PIX_FMT_NV61M),
		},
		.bitsPerPixel = 16,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingYUV,
		.packed = false,
		.pixelsPerGroup = 2,
		.planes = {{ { 2, 1 }, { 2, 1 }, { 0, 0 } }},
	} },
    { libcamera::formats::NV24, {
		.name = "NV24",
        .format = libcamera::formats::NV24,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_NV24), },
		.bitsPerPixel = 24,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingYUV,
		.packed = false,
		.pixelsPerGroup = 1,
		.planes = {{ { 1, 1 }, { 2, 1 }, { 0, 0 } }},
	} },
    { libcamera::formats::NV42, {
		.name = "NV42",
        .format = libcamera::formats::NV42,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_NV42), },
		.bitsPerPixel = 24,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingYUV,
		.packed = false,
		.pixelsPerGroup = 1,
		.planes = {{ { 1, 1 }, { 2, 1 }, { 0, 0 } }},
	} },
    { libcamera::formats::YUV420, {
		.name = "YUV420",
        .format = libcamera::formats::YUV420,
		.v4l2Formats = {
            SHV4L2PixelFormat(V4L2_PIX_FMT_YUV420),
            SHV4L2PixelFormat(V4L2_PIX_FMT_YUV420M),
		},
		.bitsPerPixel = 12,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingYUV,
		.packed = false,
		.pixelsPerGroup = 2,
		.planes = {{ { 2, 1 }, { 1, 2 }, { 1, 2 } }},
	} },
    { libcamera::formats::YVU420, {
		.name = "YVU420",
        .format = libcamera::formats::YVU420,
		.v4l2Formats = {
            SHV4L2PixelFormat(V4L2_PIX_FMT_YVU420),
            SHV4L2PixelFormat(V4L2_PIX_FMT_YVU420M),
		},
		.bitsPerPixel = 12,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingYUV,
		.packed = false,
		.pixelsPerGroup = 2,
		.planes = {{ { 2, 1 }, { 1, 2 }, { 1, 2 } }},
	} },
    { libcamera::formats::YUV422, {
		.name = "YUV422",
        .format = libcamera::formats::YUV422,
		.v4l2Formats = {
            SHV4L2PixelFormat(V4L2_PIX_FMT_YUV422P),
            SHV4L2PixelFormat(V4L2_PIX_FMT_YUV422M),
		},
		.bitsPerPixel = 16,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingYUV,
		.packed = false,
		.pixelsPerGroup = 2,
		.planes = {{ { 2, 1 }, { 1, 1 }, { 1, 1 } }},
	} },
    { libcamera::formats::YVU422, {
		.name = "YVU422",
        .format = libcamera::formats::YVU422,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_YVU422M), },
		.bitsPerPixel = 16,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingYUV,
		.packed = false,
		.pixelsPerGroup = 2,
		.planes = {{ { 2, 1 }, { 1, 1 }, { 1, 1 } }},
	} },
    { libcamera::formats::YUV444, {
		.name = "YUV444",
        .format = libcamera::formats::YUV444,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_YUV444M), },
		.bitsPerPixel = 24,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingYUV,
		.packed = false,
		.pixelsPerGroup = 1,
		.planes = {{ { 1, 1 }, { 1, 1 }, { 1, 1 } }},
	} },
    { libcamera::formats::YVU444, {
		.name = "YVU444",
        .format = libcamera::formats::YVU444,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_YVU444M), },
		.bitsPerPixel = 24,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingYUV,
		.packed = false,
		.pixelsPerGroup = 1,
		.planes = {{ { 1, 1 }, { 1, 1 }, { 1, 1 } }},
	} },

	/* Greyscale formats. */
    { libcamera::formats::R8, {
		.name = "R8",
        .format = libcamera::formats::R8,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_GREY), },
		.bitsPerPixel = 8,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingYUV,
		.packed = false,
		.pixelsPerGroup = 1,
		.planes = {{ { 1, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::R10, {
		.name = "R10",
        .format = libcamera::formats::R10,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_Y10), },
		.bitsPerPixel = 10,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingYUV,
		.packed = false,
		.pixelsPerGroup = 1,
		.planes = {{ { 2, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::R12, {
		.name = "R12",
        .format = libcamera::formats::R12,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_Y12), },
		.bitsPerPixel = 12,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingYUV,
		.packed = false,
		.pixelsPerGroup = 1,
		.planes = {{ { 2, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::R10_CSI2P, {
		.name = "R10_CSI2P",
        .format = libcamera::formats::R10_CSI2P,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_Y10P), },
		.bitsPerPixel = 10,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingYUV,
		.packed = true,
		.pixelsPerGroup = 4,
		.planes = {{ { 5, 1 }, { 0, 0 }, { 0, 0 } }},
	} },

	/* Bayer formats. */
    { libcamera::formats::SBGGR8, {
		.name = "SBGGR8",
        .format = libcamera::formats::SBGGR8,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_SBGGR8), },
		.bitsPerPixel = 8,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRAW,
		.packed = false,
		.pixelsPerGroup = 2,
		.planes = {{ { 2, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::SGBRG8, {
		.name = "SGBRG8",
        .format = libcamera::formats::SGBRG8,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_SGBRG8), },
		.bitsPerPixel = 8,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRAW,
		.packed = false,
		.pixelsPerGroup = 2,
		.planes = {{ { 2, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::SGRBG8, {
		.name = "SGRBG8",
        .format = libcamera::formats::SGRBG8,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_SGRBG8), },
		.bitsPerPixel = 8,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRAW,
		.packed = false,
		.pixelsPerGroup = 2,
		.planes = {{ { 2, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::SRGGB8, {
		.name = "SRGGB8",
        .format = libcamera::formats::SRGGB8,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_SRGGB8), },
		.bitsPerPixel = 8,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRAW,
		.packed = false,
		.pixelsPerGroup = 2,
		.planes = {{ { 2, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::SBGGR10, {
		.name = "SBGGR10",
        .format = libcamera::formats::SBGGR10,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_SBGGR10), },
		.bitsPerPixel = 10,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRAW,
		.packed = false,
		.pixelsPerGroup = 2,
		.planes = {{ { 4, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::SGBRG10, {
		.name = "SGBRG10",
        .format = libcamera::formats::SGBRG10,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_SGBRG10), },
		.bitsPerPixel = 10,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRAW,
		.packed = false,
		.pixelsPerGroup = 2,
		.planes = {{ { 4, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::SGRBG10, {
		.name = "SGRBG10",
        .format = libcamera::formats::SGRBG10,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_SGRBG10), },
		.bitsPerPixel = 10,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRAW,
		.packed = false,
		.pixelsPerGroup = 2,
		.planes = {{ { 4, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::SRGGB10, {
		.name = "SRGGB10",
        .format = libcamera::formats::SRGGB10,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_SRGGB10), },
		.bitsPerPixel = 10,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRAW,
		.packed = false,
		.pixelsPerGroup = 2,
		.planes = {{ { 4, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::SBGGR10_CSI2P, {
		.name = "SBGGR10_CSI2P",
        .format = libcamera::formats::SBGGR10_CSI2P,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_SBGGR10P), },
		.bitsPerPixel = 10,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRAW,
		.packed = true,
		.pixelsPerGroup = 4,
		.planes = {{ { 5, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::SGBRG10_CSI2P, {
		.name = "SGBRG10_CSI2P",
        .format = libcamera::formats::SGBRG10_CSI2P,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_SGBRG10P), },
		.bitsPerPixel = 10,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRAW,
		.packed = true,
		.pixelsPerGroup = 4,
		.planes = {{ { 5, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::SGRBG10_CSI2P, {
		.name = "SGRBG10_CSI2P",
        .format = libcamera::formats::SGRBG10_CSI2P,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_SGRBG10P), },
		.bitsPerPixel = 10,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRAW,
		.packed = true,
		.pixelsPerGroup = 4,
		.planes = {{ { 5, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::SRGGB10_CSI2P, {
		.name = "SRGGB10_CSI2P",
        .format = libcamera::formats::SRGGB10_CSI2P,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_SRGGB10P), },
		.bitsPerPixel = 10,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRAW,
		.packed = true,
		.pixelsPerGroup = 4,
		.planes = {{ { 5, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::SBGGR12, {
		.name = "SBGGR12",
        .format = libcamera::formats::SBGGR12,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_SBGGR12), },
		.bitsPerPixel = 12,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRAW,
		.packed = false,
		.pixelsPerGroup = 2,
		.planes = {{ { 4, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::SGBRG12, {
		.name = "SGBRG12",
        .format = libcamera::formats::SGBRG12,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_SGBRG12), },
		.bitsPerPixel = 12,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRAW,
		.packed = false,
		.pixelsPerGroup = 2,
		.planes = {{ { 4, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::SGRBG12, {
		.name = "SGRBG12",
        .format = libcamera::formats::SGRBG12,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_SGRBG12), },
		.bitsPerPixel = 12,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRAW,
		.packed = false,
		.pixelsPerGroup = 2,
		.planes = {{ { 4, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::SRGGB12, {
		.name = "SRGGB12",
        .format = libcamera::formats::SRGGB12,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_SRGGB12), },
		.bitsPerPixel = 12,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRAW,
		.packed = false,
		.pixelsPerGroup = 2,
		.planes = {{ { 4, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::SBGGR12_CSI2P, {
		.name = "SBGGR12_CSI2P",
        .format = libcamera::formats::SBGGR12_CSI2P,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_SBGGR12P), },
		.bitsPerPixel = 12,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRAW,
		.packed = true,
		.pixelsPerGroup = 2,
		.planes = {{ { 3, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::SGBRG12_CSI2P, {
		.name = "SGBRG12_CSI2P",
        .format = libcamera::formats::SGBRG12_CSI2P,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_SGBRG12P), },
		.bitsPerPixel = 12,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRAW,
		.packed = true,
		.pixelsPerGroup = 2,
		.planes = {{ { 3, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::SGRBG12_CSI2P, {
		.name = "SGRBG12_CSI2P",
        .format = libcamera::formats::SGRBG12_CSI2P,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_SGRBG12P), },
		.bitsPerPixel = 12,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRAW,
		.packed = true,
		.pixelsPerGroup = 2,
		.planes = {{ { 3, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::SRGGB12_CSI2P, {
		.name = "SRGGB12_CSI2P",
        .format = libcamera::formats::SRGGB12_CSI2P,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_SRGGB12P), },
		.bitsPerPixel = 12,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRAW,
		.packed = true,
		.pixelsPerGroup = 2,
		.planes = {{ { 3, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::SBGGR14, {
		.name = "SBGGR14",
        .format = libcamera::formats::SBGGR14,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_SBGGR14), },
		.bitsPerPixel = 14,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRAW,
		.packed = false,
		.pixelsPerGroup = 2,
		.planes = {{ { 4, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::SGBRG14, {
		.name = "SGBRG14",
        .format = libcamera::formats::SGBRG14,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_SGBRG14), },
		.bitsPerPixel = 14,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRAW,
		.packed = false,
		.pixelsPerGroup = 2,
		.planes = {{ { 4, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::SGRBG14, {
		.name = "SGRBG14",
        .format = libcamera::formats::SGRBG14,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_SGRBG14), },
		.bitsPerPixel = 14,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRAW,
		.packed = false,
		.pixelsPerGroup = 2,
		.planes = {{ { 4, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::SRGGB14, {
		.name = "SRGGB14",
        .format = libcamera::formats::SRGGB14,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_SRGGB14), },
		.bitsPerPixel = 14,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRAW,
		.packed = false,
		.pixelsPerGroup = 2,
		.planes = {{ { 4, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::SBGGR14_CSI2P, {
		.name = "SBGGR14_CSI2P",
        .format = libcamera::formats::SBGGR14_CSI2P,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_SBGGR14P), },
		.bitsPerPixel = 14,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRAW,
		.packed = true,
		.pixelsPerGroup = 4,
		.planes = {{ { 7, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::SGBRG14_CSI2P, {
		.name = "SGBRG14_CSI2P",
        .format = libcamera::formats::SGBRG14_CSI2P,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_SGBRG14P), },
		.bitsPerPixel = 14,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRAW,
		.packed = true,
		.pixelsPerGroup = 4,
		.planes = {{ { 7, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::SGRBG14_CSI2P, {
		.name = "SGRBG14_CSI2P",
        .format = libcamera::formats::SGRBG14_CSI2P,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_SGRBG14P), },
		.bitsPerPixel = 14,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRAW,
		.packed = true,
		.pixelsPerGroup = 4,
		.planes = {{ { 7, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::SRGGB14_CSI2P, {
		.name = "SRGGB14_CSI2P",
        .format = libcamera::formats::SRGGB14_CSI2P,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_SRGGB14P), },
		.bitsPerPixel = 14,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRAW,
		.packed = true,
		.pixelsPerGroup = 4,
		.planes = {{ { 7, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::SBGGR16, {
		.name = "SBGGR16",
        .format = libcamera::formats::SBGGR16,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_SBGGR16), },
		.bitsPerPixel = 16,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRAW,
		.packed = false,
		.pixelsPerGroup = 2,
		.planes = {{ { 4, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::SGBRG16, {
		.name = "SGBRG16",
        .format = libcamera::formats::SGBRG16,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_SGBRG16), },
		.bitsPerPixel = 16,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRAW,
		.packed = false,
		.pixelsPerGroup = 2,
		.planes = {{ { 4, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::SGRBG16, {
		.name = "SGRBG16",
        .format = libcamera::formats::SGRBG16,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_SGRBG16), },
		.bitsPerPixel = 16,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRAW,
		.packed = false,
		.pixelsPerGroup = 2,
		.planes = {{ { 4, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::SRGGB16, {
		.name = "SRGGB16",
        .format = libcamera::formats::SRGGB16,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_SRGGB16), },
		.bitsPerPixel = 16,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRAW,
		.packed = false,
		.pixelsPerGroup = 2,
		.planes = {{ { 4, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::SBGGR10_IPU3, {
		.name = "SBGGR10_IPU3",
        .format = libcamera::formats::SBGGR10_IPU3,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_IPU3_SBGGR10), },
		.bitsPerPixel = 10,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRAW,
		.packed = true,
		/* \todo remember to double this in the ipu3 pipeline handler */
		.pixelsPerGroup = 25,
		.planes = {{ { 32, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::SGBRG10_IPU3, {
		.name = "SGBRG10_IPU3",
        .format = libcamera::formats::SGBRG10_IPU3,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_IPU3_SGBRG10), },
		.bitsPerPixel = 10,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRAW,
		.packed = true,
		.pixelsPerGroup = 25,
		.planes = {{ { 32, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::SGRBG10_IPU3, {
		.name = "SGRBG10_IPU3",
        .format = libcamera::formats::SGRBG10_IPU3,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_IPU3_SGRBG10), },
		.bitsPerPixel = 10,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRAW,
		.packed = true,
		.pixelsPerGroup = 25,
		.planes = {{ { 32, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
    { libcamera::formats::SRGGB10_IPU3, {
		.name = "SRGGB10_IPU3",
        .format = libcamera::formats::SRGGB10_IPU3,
        .v4l2Formats = { SHV4L2PixelFormat(V4L2_PIX_FMT_IPU3_SRGGB10), },
		.bitsPerPixel = 10,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingRAW,
		.packed = true,
		.pixelsPerGroup = 25,
		.planes = {{ { 32, 1 }, { 0, 0 }, { 0, 0 } }},
	} },

	/* Compressed formats. */
    { libcamera::formats::MJPEG, {
		.name = "MJPEG",
        .format = libcamera::formats::MJPEG,
		.v4l2Formats = {
            SHV4L2PixelFormat(V4L2_PIX_FMT_MJPEG),
            SHV4L2PixelFormat(V4L2_PIX_FMT_JPEG),
		},
		.bitsPerPixel = 0,
        .colourEncoding = SHPixelFormatInfo::ColourEncodingYUV,
		.packed = false,
		.pixelsPerGroup = 1,
		.planes = {{ { 1, 1 }, { 0, 0 }, { 0, 0 } }},
	} },
};

/**
 * \fn bool SHPixelFormatInfo::isValid() const
 * \brief Check if the pixel format info is valid
 * \return True if the pixel format info is valid, false otherwise
 */

/**
 * \brief Retrieve information about a pixel format
 * \param[in] format The pixel format
 * \return The SHPixelFormatInfo describing the \a format if known, or an invalid
 * SHPixelFormatInfo otherwise
 */
const SHPixelFormatInfo &SHPixelFormatInfo::info(const libcamera::PixelFormat &format)
{
    const auto iter = pixelFormatInfo.find(format);
    if (iter == pixelFormatInfo.end()) {
        return SHPixelFormatInfoInvalid;
	}

	return iter->second;
}

/**
 * \brief Retrieve information about a V4L2 pixel format
 * \param[in] format The V4L2 pixel format
 * \return The SHPixelFormatInfo describing the V4L2 \a format if known, or an
 * invalid SHPixelFormatInfo otherwise
 */
const SHPixelFormatInfo &SHPixelFormatInfo::info(const SHV4L2PixelFormat &format)
{
    libcamera::PixelFormat pixelFormat = format.toPixelFormat(false);
	if (!pixelFormat.isValid())
        return SHPixelFormatInfoInvalid;

    const auto iter = pixelFormatInfo.find(pixelFormat);
    if (iter == pixelFormatInfo.end())
        return SHPixelFormatInfoInvalid;

	return iter->second;
}

/**
 * \brief Retrieve information about a pixel format
 * \param[in] name The name of pixel format
 * \return The SHPixelFormatInfo describing the PixelFormat matching the
 * \a name if known, or an invalid SHPixelFormatInfo otherwise
 */
const SHPixelFormatInfo &SHPixelFormatInfo::info(const std::string &name)
{
    for (const auto &info : pixelFormatInfo) {
		if (info.second.name == name)
			return info.second;
	}

    return SHPixelFormatInfoInvalid;
}

/**
 * \brief Compute the stride
 * \param[in] width The width of the line, in pixels
 * \param[in] plane The index of the plane whose stride is to be computed
 * \param[in] align The stride alignment, in bytes
 *
 * The stride is the number of bytes necessary to store a full line of a frame,
 * including padding at the end of the line. This function takes into account
 * the alignment constraints intrinsic to the format (for instance, the
 * SGRBG12_CSI2P format stores two 12-bit pixels in 3 bytes, and thus has a
 * required stride alignment of 3 bytes). Additional alignment constraints may
 * be specified through the \a align parameter, which will cause the stride to
 * be rounded up to the next multiple of \a align.
 *
 * For multi-planar formats, different planes may have different stride values.
 * The \a plane parameter selects which plane to compute the stride for.
 *
 * \return The number of bytes necessary to store a line, or 0 if the
 * SHPixelFormatInfo instance or the \a plane is not valid
 */
unsigned int SHPixelFormatInfo::stride(unsigned int width, unsigned int plane,
				     unsigned int align) const
{
	if (!isValid()) {
		return 0;
	}

	if (plane > planes.size() || !planes[plane].bytesPerGroup) {
		return 0;
	}

	/* ceil(width / pixelsPerGroup) * bytesPerGroup */
	unsigned int stride = (width + pixelsPerGroup - 1) / pixelsPerGroup
			    * planes[plane].bytesPerGroup;

	/* ceil(stride / align) * align */
	return (stride + align - 1) / align * align;
}

/**
 * \brief Compute the number of bytes necessary to store a plane of a frame
 * \param[in] size The size of the frame, in pixels
 * \param[in] plane The plane index
 * \param[in] align The stride alignment, in bytes (1 for default alignment)
 *
 * The plane size is computed by multiplying the line stride and the frame
 * height, taking subsampling and other format characteristics into account.
 * Stride alignment constraints may be specified through the \a align parameter.
 *
 * \sa stride()
 *
 * \return The number of bytes necessary to store the plane, or 0 if the
 * SHPixelFormatInfo instance is not valid or the plane number isn't valid for the
 * format
 */
unsigned int SHPixelFormatInfo::planeSize(const libcamera::Size &size, unsigned int plane,
					unsigned int align) const
{
    unsigned int stride = SHPixelFormatInfo::stride(size.width, plane, align);
	if (!stride)
		return 0;

	return planeSize(size.height, plane, stride);
}

/**
 * \brief Compute the number of bytes necessary to store a plane of a frame
 * \param[in] height The height of the frame, in pixels
 * \param[in] plane The plane index
 * \param[in] stride The plane stride, in bytes
 *
 * The plane size is computed by multiplying the line stride and the frame
 * height, taking subsampling and other format characteristics into account.
 * Stride alignment constraints may be specified through the \a align parameter.
 *
 * \return The number of bytes necessary to store the plane, or 0 if the
 * SHPixelFormatInfo instance is not valid or the plane number isn't valid for the
 * format
 */
unsigned int SHPixelFormatInfo::planeSize(unsigned int height, unsigned int plane,
					unsigned int stride) const
{
	unsigned int vertSubSample = planes[plane].verticalSubSampling;
	if (!vertSubSample)
		return 0;

	/* stride * ceil(height / verticalSubSampling) */
	return stride * ((height + vertSubSample - 1) / vertSubSample);
}

/**
 * \brief Compute the number of bytes necessary to store a frame
 * \param[in] size The size of the frame, in pixels
 * \param[in] align The stride alignment, in bytes (1 for default alignment)
 *
 * The frame size is computed by adding the size of all planes, as computed by
 * planeSize(), using the specified alignment constraints for all planes. For
 * more complex stride constraints, use the frameSize() overloaded version that
 * takes an array of stride values.
 *
 * \sa planeSize()
 *
 * \return The number of bytes necessary to store the frame, or 0 if the
 * SHPixelFormatInfo instance is not valid
 */
/*
unsigned int SHPixelFormatInfo::frameSize(const libcamera::Size &size, unsigned int align) const
{
	unsigned int sum = 0;

	for (const auto &[i, plane] : utils::enumerate(planes)) {
		if (plane.bytesPerGroup == 0)
			break;

		sum += planeSize(size, i, align);
	}

	return sum;
}
*/
/**
 * \brief Compute the number of bytes necessary to store a frame
 * \param[in] size The size of the frame, in pixels
 * \param[in] strides The strides to use for each plane
 *
 * This function is an overloaded version that takes custom strides for each
 * plane, to be used when the device has custom alignment constraints that
 * can't be described by just an alignment value.
 *
 * \return The number of bytes necessary to store the frame, or 0 if the
 * SHPixelFormatInfo instance is not valid
 */
unsigned int
SHPixelFormatInfo::frameSize(const libcamera::Size &size,
			   const std::array<unsigned int, 3> &strides) const
{
	/* stride * ceil(height / verticalSubSampling) */
	unsigned int sum = 0;
	for (unsigned int i = 0; i < 3; i++) {
		unsigned int vertSubSample = planes[i].verticalSubSampling;
		if (!vertSubSample)
			continue;
		sum += strides[i]
		     * ((size.height + vertSubSample - 1) / vertSubSample);
	}

	return sum;
}

/**
 * \brief Retrieve the number of planes represented by the format
 * \return The number of planes used by the format
 */
unsigned int SHPixelFormatInfo::numPlanes() const
{
	unsigned int count = 0;

	for (const Plane &p : planes) {
		if (p.bytesPerGroup == 0)
			break;

		count++;
	}

	return count;
}