/*
	SuperEagle vertex shader
	based on libretro SuperEagle shader
	https://github.com/libretro/glsl-shaders/blob/master/eagle/shaders

	MIT License

	Copyright (c) 2020 Oleksiy Ryabchun

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

uniform sampler2D tex01;
uniform vec2 texSize;

in vec4 vCoord;
in vec2 vTex;

out vec4 t1;
out vec4 t2;
out vec4 t3;
out vec4 t4;
out vec4 t5;
out vec4 t6;
out vec4 t7;
out vec4 t8;
out vec2 fTex;

void main() {
	gl_Position = vCoord;
	
	vec2 d = 1.0 / texSize;
	t1.xy = vTex + vec2(-d.x,-d.y);
	t1.zw = vTex + vec2(-d.x,  0);
	t2.xy = vTex + vec2(+d.x,-d.y);
	t2.zw = vTex + vec2(+d.x+d.x,-d.y);
	t3.xy = vTex + vec2(-d.x,  0);
	t3.zw = vTex + vec2(+d.x,  0);
	t4.xy = vTex + vec2(+d.x+d.x,  0);
	t4.zw = vTex + vec2(-d.x,+d.y);
	t5.xy = vTex + vec2(  0,+d.y);
	t5.zw = vTex + vec2(+d.x,+d.y);
	t6.xy = vTex + vec2(+d.x+d.x,+d.y);
	t6.zw = vTex + vec2(-d.x,+d.y+d.y);
	t7.xy = vTex + vec2(  0,+d.y+d.y);
	t7.zw = vTex + vec2(+d.x,+d.y+d.y);
	t8.xy = vTex + vec2(+d.x+d.x,+d.y+d.y);
	
	fTex = vTex;
}