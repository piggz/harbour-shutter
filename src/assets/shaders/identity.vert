/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2020, Linaro
 *
 * identity.vert - Identity vertex shader for pixel format conversion
 */

attribute vec2 vertexIn;
attribute vec2 textureIn;

varying mediump vec3 colourOut;
varying mediump vec2 textureOut;

uniform mat4 proj_matrix;
uniform float stride_factor;

void main(void)
{
        gl_Position = proj_matrix * vec4(vertexIn, 0.0, 1.0);
        textureOut = vec2(textureIn.x, textureIn.y);
}
