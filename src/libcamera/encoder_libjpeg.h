/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2020, Google Inc.
 *
 * encoder_libjpeg.h - JPEG encoding using libjpeg
 */

#pragma once

#include <vector>
#include "internal/formats.h"
#include "src/format_converter.h"
#include <libcamera/stream.h>
#include <jpeglib.h>

class EncoderLibJpeg
{
public:
	EncoderLibJpeg();
	~EncoderLibJpeg();

    int configure(const libcamera::StreamConfiguration &cfg);
    bool encode(const libcamera::StreamConfiguration &cfg, libcamera::FrameBuffer *buffer, class Image *image, std::string outFileName);
    bool encode(const unsigned char *src,
           std::string outFileName,
		   libcamera::Span<const uint8_t> exifData,
		   unsigned int quality);

private:
	void compressRGB(const std::vector<libcamera::Span<uint8_t>> &planes);
	void compressNV(const std::vector<libcamera::Span<uint8_t>> &planes);

	struct jpeg_compress_struct compress_;
	struct jpeg_error_mgr jerr_;

    const SHPixelFormatInfo *pixelFormatInfo_;
    libcamera::PixelFormat pixelFormat_;
    FormatConverter converter_;
    libcamera::FrameBuffer *buffer_;

	bool nv_;
	bool nvSwap_;
};
