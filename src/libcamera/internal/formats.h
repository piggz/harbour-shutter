/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2019, Google Inc.
 *
 * formats.h - libcamera image formats
 */

#pragma once

#include <array>
#include <map>
#include <vector>

#include <libcamera/geometry.h>
#include <libcamera/pixel_format.h>

#include "v4l2_pixelformat.h"

class SHPixelFormatInfo
{
public:
	enum ColourEncoding {
		ColourEncodingRGB,
		ColourEncodingYUV,
		ColourEncodingRAW,
	};

	struct Plane {
		unsigned int bytesPerGroup;
		unsigned int verticalSubSampling;
	};

	bool isValid() const { return format.isValid(); }

    static const SHPixelFormatInfo &info(const libcamera::PixelFormat &format);
    static const SHPixelFormatInfo &info(const SHV4L2PixelFormat &format);
    static const SHPixelFormatInfo &info(const std::string &name);

	unsigned int stride(unsigned int width, unsigned int plane,
			    unsigned int align = 1) const;
    unsigned int planeSize(const libcamera::Size &size, unsigned int plane,
			       unsigned int align = 1) const;
	unsigned int planeSize(unsigned int height, unsigned int plane,
			       unsigned int stride) const;
    unsigned int frameSize(const libcamera::Size &size, unsigned int align = 1) const;
    unsigned int frameSize(const libcamera::Size &size,
			       const std::array<unsigned int, 3> &strides) const;

	unsigned int numPlanes() const;

	/* \todo Add support for non-contiguous memory planes */
	const char *name;
    libcamera::PixelFormat format;
    std::vector<SHV4L2PixelFormat> v4l2Formats;
	unsigned int bitsPerPixel;
	enum ColourEncoding colourEncoding;
	bool packed;

	unsigned int pixelsPerGroup;

	std::array<Plane, 3> planes;
};
