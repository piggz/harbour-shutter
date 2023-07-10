/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2019, Google Inc.
 * Copyright (C) 2020, Raspberry Pi Ltd
 *
 * v4l2_pixelformat.h - V4L2 Pixel Format
 */

#pragma once

#include <functional>
#include <ostream>
#include <stdint.h>
#include <string>
#include <vector>
#include <map>

#include <libcamera/pixel_format.h>

class SHV4L2PixelFormat
{
public:
	struct Info {
        libcamera::PixelFormat format;
		const char *description;
	};

    SHV4L2PixelFormat()
		: fourcc_(0)
	{
	}

    explicit SHV4L2PixelFormat(uint32_t fourcc)
		: fourcc_(fourcc)
	{
	}

	bool isValid() const { return fourcc_ != 0; }
	uint32_t fourcc() const { return fourcc_; }
	operator uint32_t() const { return fourcc_; }

	std::string toString() const;
	const char *description() const;

    libcamera::PixelFormat toPixelFormat(bool warn = true) const;
    static const std::vector<SHV4L2PixelFormat> &
    fromPixelFormat(const libcamera::PixelFormat &pixelFormat);

private:
	uint32_t fourcc_;
};

std::ostream &operator<<(std::ostream &out, const SHV4L2PixelFormat &f);

template<>
struct std::hash<SHV4L2PixelFormat> {
    size_t operator()(SHV4L2PixelFormat const &format) const noexcept
	{
		return format.fourcc();
	}
};
