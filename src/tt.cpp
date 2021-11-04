//tt.cpp - Teletext Editor implementation
//Copyright (C) 2021 Ayman Wagih
//
//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include			"tt.h"
#include			<string.h>
#include			<vector>
#define				STB_IMAGE_IMPLEMENTATION
#include			"stb_image.h"
const char			file[]=__FILE__;
char				g_buf[G_BUF_SIZE]={};
wchar_t				g_wbuf[G_BUF_SIZE]={};
int					w=0, h=0;
short				mx=0, my=0, dx=0, dy=0, tab_count=4;

//shaders
#if 1
#define				DECL_SHADER_VAR(NAME)	{&NAME, #NAME}
#define				DECL_SHADER_END(NAME)	ns_##NAME::attributes, ns_##NAME::uniforms, SIZEOF(ns_##NAME::attributes), SIZEOF(ns_##NAME::uniforms)
namespace			ns_2d
{
	int a_coords=-1, u_color=-1;
	ShaderVar2 attributes[]=
	{
		DECL_SHADER_VAR(a_coords),
	};
	ShaderVar2 uniforms[]=
	{
		DECL_SHADER_VAR(u_color),
	};
}
ShaderProgram		shader_2d=
{
	"shader_2d",

	"#version 120\n"
	"attribute vec2 a_coords;\n"
	"void main()\n"
	"{\n"
	"    gl_Position=vec4(a_coords, 0., 1.);\n"
	"}",

	"#version 120\n"
	"uniform vec4 u_color;\n"
	"void main()\n"
	"{\n"
	"    gl_FragColor=u_color;\n"
	"}",

	DECL_SHADER_END(2d),
	//ns_2d::attributes, ns_2d::uniforms, SIZEOF(ns_2d::attributes), SIZEOF(ns_2d::uniforms),
	0
};
namespace			ns_text
{
	int a_coords=-1, u_atlas=-1, u_txtColor=-1, u_bkColor=-1;
	ShaderVar2 attributes[]=
	{
		DECL_SHADER_VAR(a_coords),
	};
	ShaderVar2 uniforms[]=
	{
		DECL_SHADER_VAR(u_atlas),
		DECL_SHADER_VAR(u_txtColor),
		DECL_SHADER_VAR(u_bkColor),
	};
}
ShaderProgram		shader_text=
{
	"shader_text",

	"#version 120\n"
	"attribute vec4 a_coords;"			//attributes: a_coords
	"varying vec2 v_texcoord;\n"
	"void main()\n"
	"{\n"
	"    gl_Position=vec4(a_coords.xy, 0., 1.);\n"
	"    v_texcoord=a_coords.zw;\n"
	"}",

	"#version 120\n"
	"varying vec2 v_texcoord;\n"
	"uniform sampler2D u_atlas;\n"	//uniforms: u_atlas, u_txtColor, u_bkColor
	"uniform vec4 u_txtColor, u_bkColor;\n"
	"void main()\n"
	"{\n"
	"    vec4 region=texture2D(u_atlas, v_texcoord);\n"
	"    gl_FragColor=mix(u_txtColor, u_bkColor, region.r);\n"
	"}",

	DECL_SHADER_END(text),
	0
};
namespace			ns_texture
{
	int a_coords=-1, u_texture=-1, u_alpha=-1;
	ShaderVar2 attributes[]=
	{
		DECL_SHADER_VAR(a_coords),
	};
	ShaderVar2 uniforms[]=
	{
		DECL_SHADER_VAR(u_texture),
		DECL_SHADER_VAR(u_alpha),
	};
}
ShaderProgram		shader_texture=
{
	"shader_texture",

	"#version 120\n"
	"attribute vec4 a_coords;"			//attributes: a_coords
	"varying vec2 v_texcoord;\n"
	"void main()\n"
	"{\n"
	"    gl_Position=vec4(a_coords.xy, 0., 1.);\n"
	"    v_texcoord=a_coords.zw;\n"
	"}",

	"#version 120\n"
	"varying vec2 v_texcoord;\n"
	"uniform sampler2D u_texture;\n"	//uniforms: u_texture, u_alpha
	"uniform float u_alpha;\n"
	"void main()\n"
	"{\n"
	"    gl_FragColor=texture2D(u_texture, v_texcoord);\n"
	"    gl_FragColor.a*=u_alpha;\n"
	"}",

	DECL_SHADER_END(texture),
	0
};
#undef				DECL_SHADER_VAR
#undef				DECL_SHADER_END
#endif

struct				QuadCoords
{
	float x1, x2, y1, y2;
};
QuadCoords			font_coords[128-32]={};
unsigned			font_txid=0, vertex_buffer=0;

std::vector<float>	vrtx;
void				set_text_colors(int txt, int bk)
{
	setGLProgram(shader_text.program);
	send_color(ns_text::u_txtColor, txt);
	send_color(ns_text::u_bkColor, bk);
}
int					print_line(short x, short y, const char *msg, int msg_length, short tab_origin, short zoom)
{
	if(msg_length<1)
		return 0;
	float rect[4]={};
	QuadCoords *txc=nullptr;
	int msg_width=0, width, idx, printable_count=0, tab_width=tab_count*dx, w2=dx*zoom, height=dy*zoom;
	vrtx.resize(msg_length<<4);//vx, vy, txx, txy		x4 vertices/char
	for(int k=0;k<msg_length;++k)
	{
		char c=msg[k];
		if(c=='\t')
			width=(tab_width-(x+msg_width-tab_origin)%tab_width)*zoom, c=' ';
		else if(c>=32&&c<0xFF)
			width=w2;
		else
			width=0;
		if(width)
		{
			toNDC(float(x+msg_width			), float(y			), rect[0], rect[1]);
			toNDC(float(x+msg_width+width	), float(y+height	), rect[2], rect[3]);//y2<y1
			idx=k<<4;
			txc=font_coords+c-32;
			vrtx[idx   ]=rect[0], vrtx[idx+ 1]=rect[1],		vrtx[idx+ 2]=txc->x1, vrtx[idx+ 3]=txc->y1;//top left
			vrtx[idx+ 4]=rect[0], vrtx[idx+ 5]=rect[3],		vrtx[idx+ 6]=txc->x1, vrtx[idx+ 7]=txc->y2;//bottom left
			vrtx[idx+ 8]=rect[2], vrtx[idx+ 9]=rect[3],		vrtx[idx+10]=txc->x2, vrtx[idx+11]=txc->y2;//bottom right
			vrtx[idx+12]=rect[2], vrtx[idx+13]=rect[1],		vrtx[idx+14]=txc->x2, vrtx[idx+15]=txc->y1;//top right

			msg_width+=width;
			++printable_count;
		}
	}
	setGLProgram(shader_text.program);
	select_texture(font_txid, ns_text::u_atlas);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);								GL_CHECK();
	glBufferData(GL_ARRAY_BUFFER, msg_length<<6, vrtx.data(), GL_STATIC_DRAW);	GL_CHECK();//set vertices & texcoords
	glVertexAttribPointer(ns_text::a_coords, 4, GL_FLOAT, GL_TRUE, 0, 0);		GL_CHECK();

	glEnableVertexAttribArray(ns_text::a_coords);	GL_CHECK();
	glDrawArrays(GL_QUADS, 0, msg_length<<2);		GL_CHECK();//draw the quads: 4 vertices per character quad
	glDisableVertexAttribArray(ns_text::a_coords);	GL_CHECK();
	return msg_width;
}
int					print(short zoom, short tab_origin, short x, short y, const char *format, ...)
{
	va_list args;
	va_start(args, format);
#ifdef _MSC_VER
	int len=vsprintf_s(g_buf, g_buf_size, format, args);
#else
	int len=vsnprintf(g_buf, g_buf_size, format, args);
#endif
	va_end(args);
	return print_line(x, y, g_buf, len, tab_origin, zoom);
}

void				wnd_on_create(){}
bool				wnd_on_init()
{
	make_gl_program(shader_2d);
	make_gl_program(shader_text);
	make_gl_program(shader_texture);
	prof_add("Compile shaders");

	glGenBuffers(1, &vertex_buffer);

	int iw=0, ih=0, bytespp=0;
	auto rgb=(int*)stbi_load("font.PNG", &iw, &ih, &bytespp, 4);
	if(!rgb)
	{
		messageboxa("Error", "Font texture not found.\nPlace a \'font.PNG\' file with the program.\n");
		return false;
	}
	for(int k=0, size=iw*ih;k<size;++k)
		if(rgb[k]&0x00FFFFFF)
			rgb[k]=0xFFFFFFFF;
	dx=iw/8, dy=ih/16;
	for(int c=32;c<127;++c)
	{
		auto &rect=font_coords[c-32];
		int kx=c&7, ky=c>>3;
		rect.x1=float(kx)/ 8, rect.x2=float(kx+1)/ 8;
		rect.y1=float(ky)/16, rect.y2=float(ky+1)/16;
		if(rect.x1>1)
			continue;
	}
	glGenTextures(1, &font_txid);
	send_texture_pot(font_txid, rgb, iw, ih);
	stbi_image_free(rgb);
	prof_add("Load font");

	set_text_colors(0x8000FF00, 0x80FF00FF);
	//set_text_colors(0xFF000000, 0xFFFFFFFF);

	//font_set(L"Consolas", 10);
	//font_use();
	//font_get_dimensions(dx, dy);
	//font_drop();
	return true;
}
void				wnd_on_resize(){}
void				wnd_on_render()
{
	prof_add("Render entry");

	glClear(GL_COLOR_BUFFER_BIT);

	print(1, 0, 0, 0, "Hello. Sample Text. What\'s going on???");
	for(int k=0;k<1000;++k)
	{
		int zoom=1+rand()%3;
		print(zoom, 0, rand()%w, rand()%h, "Sample Text %d", zoom);
	}
	prof_add("Benchmark");

	//memset(rgb, 0, rgbn*sizeof(int));
	//font_use();
	//int ret=GUITPrint(0, 0, "Teletext");
	//GUITPrint(10, 100, "A");
	//font_drop();

	update_screen();
	prof_add("Swap");
	prof_print();
}
void				wnd_on_input(HWND hWnd, int message, int wParam, int lParam)
{
	switch(message)
	{
	case WM_MOUSEMOVE:
		InvalidateRect(hWnd, nullptr, 0);//
		break;

	case WM_MOUSEWHEEL:
		break;

	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		break;
	case WM_LBUTTONUP:
		break;

	case WM_RBUTTONDOWN:
	case WM_RBUTTONDBLCLK:
		break;
	case WM_RBUTTONUP:
		break;
		
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		{
			int iw=0, ih=0, bytespp=0;
			auto rgb=(int*)stbi_load("font.PNG", &iw, &ih, &bytespp, 4);
			if(!rgb)
			{
				messageboxa("Error", "Font texture not found.\nPlace a \'font.PNG\' file with the program.\n");
				return;
			}
			for(int k=0, size=iw*ih;k<size;++k)
				if(rgb[k]&0x00FFFFFF)
					rgb[k]=0xFFFFFFFF;
			for(int ky=0;ky<ih;++ky)
				for(int kx=0;kx<iw;++kx)
					SetPixel(ghDC, kx, ky, rgb[iw*ky+kx]&0x00FFFFFF);
			stbi_image_free(rgb);
		}
		switch(wParam)
		{
		case VK_F4:
			prof_toggle();
			break;
		}
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		break;
	}
}
bool				wnd_on_quit()//return false to deny exit
{
	return true;
}