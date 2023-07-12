#pragma once

#include <vector>
#include "src/format_converter.h"
#include <libcamera/stream.h>

class EncoderJpeg
{
public:
    EncoderJpeg();

    int configure(const libcamera::StreamConfiguration &cfg);
    bool encode(const libcamera::StreamConfiguration &cfg, libcamera::FrameBuffer *buffer, class Image *image, std::string outFileName);

private:
    FormatConverter converter_;
    libcamera::FrameBuffer *buffer_;
};
