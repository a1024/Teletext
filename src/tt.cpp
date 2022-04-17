//tt.cpp - Teletext Editor implementation
//Copyright (C) 2021, 2022 Ayman Wagih
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
#define				STB_IMAGE_IMPLEMENTATION
#include			"stb_image.h"//https://github.com/nothings/stb

//	#define			DEBUG_CURSOR
//	#define			HELP_SHOWALL

//on: rectsel target horizontally starts at mouse
//off: rectsel target horizontally shifted by mouse
//	#define			DRAG_RECTSEL_X_AT_MOUSE

const char			file[]=__FILE__;
char				g_buf[G_BUF_SIZE]={};
int					w=0, h=0;
short				mx=0, my=0, mline=0, mcol=0,//current mouse coordinates
					dx=0, dy=0,//non-tab character dimensions
					tab_count=4;
std::string			exe_dir;

void				display_help()
{
#ifdef __linux__
	messagebox("Controls",
		"Ctrl+O:         Open\n"//TODO: config file
		"Ctrl+S:         Save\n"
		"Ctrl+Shift+S:   Save as\n"
		"Ctrl+Z/Y:       Undo/Redo\n"
		"Ctrl+D:         Clear undo/redo history\n"
		"Ctrl+X/C/V:     Cut/Copy/Paste\n"
		"\n"
		"Ctrl+N/T:       New tab\n"
		"Ctrl+W:         Close tab\n"
		"Ctrl+Shift+T:   Reopen closed tab\n"
		"Ctrl+Tab:       Next tab\n"
		"Ctrl+Shift+Tab: Previous tab\n"
		"Ctrl+R:         Change tab bar orientation\n"
		"\n"
		"Ctrl+Up/Down:           Scroll\n"
		"Ctrl+Left/Right:        Skip word\n"
#ifdef HELP_SHOWALL
		"Alt+Left:               Previous location\n"
		"Alt+Right:              Next location\n"
		"Ctrl+F:                 Find/Replace\n"
		"F3:                     Find next\n"
#endif
		"Ctrl+=/-/wheel:         Zoom in/out\n"
		"\n"
		"Shift+Arrows/Home/End:  Select\n"
		"Ctrl+Mouse1:            Select word\n"
		"Ctrl+A:                 Select all\n"
		"Alt+Drag:               Rectangular selection\n"
		"Shift+Drag:             Resize selection\n"
		"Ctrl+Shift+Left/Right:  Select words\n"
		"Escape:                 Deselect\n"
		"\n"
#ifdef HELP_SHOWALL
		"F11:            Full-screen\n"
#endif
		"F4:             Toggle benchmark");
#else
	messagebox("Controls",
		"Ctrl+O:\t\tOpen\n"//TODO: config file
		"Ctrl+S:\t\tSave\n"
		"Ctrl+Shift+S:\tSave as\n"
		"Ctrl+Z/Y:\t\tUndo/Redo\n"
		"Ctrl+D:\t\tClear undo/redo history\n"
		"Ctrl+X/C/V:\tCut/Copy/Paste\n"
		"\n"
		"Ctrl+N/T:\t\tNew tab\n"
		"Ctrl+W:\t\tClose tab\n"
		"Ctrl+Shift+T:\tReopen closed tab\n"
		"Ctrl+Tab:\t\tNext tab\n"
		"Ctrl+Shift+Tab:\tPrevious tab\n"
		"Ctrl+R:\t\tChange tab bar orientation\n"
		"\n"
		"Ctrl+Up/Down:\tScroll\n"
		"Ctrl+Left/Right:\tSkip word\n"
#ifdef HELP_SHOWALL
		"Alt+Left/Right:\tPrevious/Next location\n"
		"Ctrl+F:\t\tFind/Replace\n"
		"F3:\t\tFind next\n"
#endif
		"Ctrl+=/-/wheel:\tZoom in/out\n"
		"\n"
		"Shift+Arrows/Home/End:\tSelect\n"
		"Ctrl+Mouse1:\t\tSelect word\n"
		"Ctrl+A:\t\t\tSelect all\n"
		"Alt+Drag:\t\t\tRectangular selection\n"
		"Shift+Drag:\t\tResize selection\n"
		"Ctrl+Shift+Left/Right:\tSelect words\n"
		"Escape:\t\t\tDeselect\n"
		"\n"
#ifdef HELP_SHOWALL
		"F11:\tFull-screen\n"
#endif
		"F4:\tToggle benchmark");
#endif
}

void				copy_to_clipboard(std::string const &str){copy_to_clipboard_c(str.c_str(), str.size());}
void				buffer2clipboard(const int *buffer, int bw, int bh)
{
	std::string str;
	for(int ky=0;ky<bh;++ky)
	{
		for(int kx=0;kx<bw;++kx)
		{
			sprintf_s(g_buf, g_buf_size, " %08X", buffer[bw*ky+kx]);
			str+=g_buf;
		}
		str+='\n';
	}
	copy_to_clipboard(str);
}
void				int2clipboard(int i)
{
	int printed=sprintf_s(g_buf, g_buf_size, " %08X", i);
	copy_to_clipboard_c(g_buf, printed);
}

//shaders
#if 1
#define				DECL_SHADER_VAR(NAME)	{&NAME, #NAME}
#define				DECL_SHADER_END(NAME)	ns_##NAME::attributes, ns_##NAME::uniforms, SIZEOF(ns_##NAME::attributes), SIZEOF(ns_##NAME::uniforms), 0
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
	"attribute vec2 a_coords;\n"		//attributes: a_coords
	"void main()\n"
	"{\n"
	"    gl_Position=vec4(a_coords, 0., 1.);\n"
	"}",

	"#version 120\n"
	"uniform vec4 u_color;\n"			//uniforms: u_color
	"void main()\n"
	"{\n"
	"    gl_FragColor=u_color;\n"
	"}",

	DECL_SHADER_END(2d)
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
	"uniform sampler2D u_atlas;\n"		//uniforms: u_atlas, u_txtColor, u_bkColor
	"uniform vec4 u_txtColor, u_bkColor;\n"
	"void main()\n"
	"{\n"
	"    vec4 region=texture2D(u_atlas, v_texcoord);\n"
	"    gl_FragColor=mix(u_txtColor, u_bkColor, region.r);\n"
	"}",

	DECL_SHADER_END(text)
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

	DECL_SHADER_END(texture)
};
#undef				DECL_SHADER_VAR
#undef				DECL_SHADER_END
#endif

float				g_fbuf[16]={0};
unsigned			vertex_buffer=0;
void				draw_line(float x1, float y1, float x2, float y2, int color)
{
	toNDC(x1, y1, g_fbuf[0], g_fbuf[1]);
	toNDC(x2, y2, g_fbuf[2], g_fbuf[3]);
	setGLProgram(shader_2d.program);			GL_CHECK();
	send_color(ns_2d::u_color, color);			GL_CHECK();
	glEnableVertexAttribArray(ns_2d::a_coords);	GL_CHECK();
	
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);						GL_CHECK();
	glBufferData(GL_ARRAY_BUFFER, 4<<2, g_fbuf, GL_STATIC_DRAW);		GL_CHECK();
	glVertexAttribPointer(ns_2d::a_coords, 2, GL_FLOAT, GL_FALSE, 0, 0);GL_CHECK();
	
//	glBindBuffer(GL_ARRAY_BUFFER, 0);											GL_CHECK();
//	glVertexAttribPointer(ns_2d::a_coords, 2, GL_FLOAT, GL_FALSE, 0, g_fbuf);	GL_CHECK();
	
	glEnableVertexAttribArray(ns_2d::a_coords);	GL_CHECK();
	glDrawArrays(GL_LINES, 0, 2);				GL_CHECK();
	glDisableVertexAttribArray(ns_2d::a_coords);GL_CHECK();
}
void				draw_line_i(int x1, int y1, int x2, int y2, int color){draw_line((float)x1, (float)y1, (float)x2, (float)y2, color);}
void				draw_rectangle(float x1, float x2, float y1, float y2, int color)
{
	float X1, X2, Y1, Y2;
	toNDC(x1, y1, X1, Y1);
	toNDC(x2, y2, X2, Y2);
	g_fbuf[0]=X1, g_fbuf[1]=Y1;
	g_fbuf[2]=X2, g_fbuf[3]=Y1;
	g_fbuf[4]=X2, g_fbuf[5]=Y2;
	g_fbuf[6]=X1, g_fbuf[7]=Y2;
	g_fbuf[8]=X1, g_fbuf[9]=Y1;
	setGLProgram(shader_2d.program);			GL_CHECK();
	send_color(ns_2d::u_color, color);			GL_CHECK();
	glEnableVertexAttribArray(ns_2d::a_coords);	GL_CHECK();
		
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);						GL_CHECK();
	glBufferData(GL_ARRAY_BUFFER, 10<<2, g_fbuf, GL_STATIC_DRAW);		GL_CHECK();
	glVertexAttribPointer(ns_2d::a_coords, 2, GL_FLOAT, GL_FALSE, 0, 0);GL_CHECK();

//	glVertexAttribPointer(ns_2d::a_coords, 2, GL_FLOAT, GL_FALSE, 0, g_fbuf);	GL_CHECK();//use buffer
	
	glEnableVertexAttribArray(ns_2d::a_coords);	GL_CHECK();
	glDrawArrays(GL_TRIANGLE_FAN, 0, 5);		GL_CHECK();
	glDisableVertexAttribArray(ns_2d::a_coords);GL_CHECK();
}
void				draw_rectangle_i(int x1, int x2, int y1, int y2, int color){draw_rectangle((float)x1, (float)x2, (float)y1, (float)y2, color);}
void				draw_rectangle_hollow(int x1, int x2, int y1, int y2, int color)
{
	toNDC(x1, y1, g_fbuf[0], g_fbuf[1]);
	toNDC(x1, y2, g_fbuf[2], g_fbuf[3]);
	toNDC(x2, y2, g_fbuf[4], g_fbuf[5]);
	toNDC(x2, y1, g_fbuf[6], g_fbuf[7]);
	//toNDC(x1, y1, g_fbuf[8], g_fbuf[9]);
	setGLProgram(shader_2d.program);			GL_CHECK();
	send_color(ns_2d::u_color, color);			GL_CHECK();
	glEnableVertexAttribArray(ns_2d::a_coords);	GL_CHECK();
	
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);							GL_CHECK();
	glBufferData(GL_ARRAY_BUFFER, 8*sizeof(float), g_fbuf, GL_STATIC_DRAW);	GL_CHECK();
	glVertexAttribPointer(ns_2d::a_coords, 2, GL_FLOAT, GL_FALSE, 0, 0);	GL_CHECK();
	
//	glBindBuffer(GL_ARRAY_BUFFER, 0);											GL_CHECK();
//	glVertexAttribPointer(ns_2d::a_coords, 2, GL_FLOAT, GL_FALSE, 0, g_fbuf);	GL_CHECK();
	
	glEnableVertexAttribArray(ns_2d::a_coords);	GL_CHECK();
	glDrawArrays(GL_LINE_LOOP, 0, 4);			GL_CHECK();
	glDisableVertexAttribArray(ns_2d::a_coords);GL_CHECK();
}

struct				QuadCoords
{
	float x1, x2, y1, y2;
};
QuadCoords			font_coords[128-32]={};
unsigned			font_txid=0;
std::vector<float>	vrtx;
void				set_text_colors(U64 const &colors)
{
	setGLProgram(shader_text.program);
	send_color(ns_text::u_txtColor, colors.lo);
	send_color(ns_text::u_bkColor, colors.hi);
}
void				col2idx(const char *text, int text_length, int tab0_cols, int idx0, int col0, float req_col, int *ret_idx, int *ret_col)
{
	int idx=idx0, col=col0;
	for(;idx<text_length;++idx)
	{
		char c=text[idx];
		int dcol=0;
		if(c=='\t')
			dcol=tab_count-mod(col-tab0_cols, tab_count);
		else if(c>=32&&c<0xFF)
			dcol=1;
		if(col+dcol*0.5f>=req_col)
			break;
		col+=dcol;
	}
	if(ret_col)
		*ret_col=col;
	if(ret_idx)
		*ret_idx=idx;
}
int					idx2col(const char *text, int text_length, int tab0_cols)
{
	int idx=0, col=0;
	for(;idx<text_length;++idx)
	{
		char c=text[idx];
		if(c=='\t')
			col+=tab_count-mod(col-tab0_cols, tab_count);
		else if(c>=32&&c<0xFF)
			++col;
	}
	return col;
}
int					calc_width		(int x, int y, const char *msg, int msg_length, int tab_origin, short zoom)
{
	if(msg_length<1)
		return 0;
	int msg_width=0, width, tab_width=tab_count*dx*zoom, w2=dx*zoom;
	for(int k=0;k<msg_length;++k)
	{
		char c=msg[k];
		if(c=='\t')
			width=tab_width-mod(x+msg_width-tab_origin, tab_width), c=' ';
		else if(c>=32&&c<0xFF)
			width=w2;
		else
			width=0;
		msg_width+=width;
	}
	return msg_width;
}
void				inv_calc_width	(int x, int y, const char *msg, int msg_length, int tab_origin, short zoom, int width, int *out_cols, int *out_k)//returns index to fit [msg, msg+index[ in width
{
	if(msg_length<1)
	{
		if(out_cols)
			*out_cols=0;
		if(out_k)
			*out_k=0;
		return;
	}
	int msg_width=0, temp_width, printable_count=0, tab_width=tab_count*dx*zoom, dxpx=dx*zoom, k=0;
	for(;k<msg_length;++k)
	{
		char c=msg[k];
		if(c=='\n')
			break;
		if(c=='\t')
			temp_width=tab_width-mod(x+msg_width-tab_origin, tab_width), c=' ';
		else if(c>=32&&c<0xFF)
			temp_width=dxpx;
		else
			temp_width=0;
		if(temp_width)
		{
			int sum=msg_width+temp_width;
			if(sum>width)
				break;
			if(sum==width)
			{
				++k;
				break;
			}
			msg_width=sum;
			++printable_count;
		}
	}
	if(out_cols)
		*out_cols=msg_width/dxpx;
	if(out_k)
		*out_k=k;
}
int					print_line		(int x, int y, const char *msg, int msg_length, int tab_origin, short zoom, int req_cols, int *ret_idx)
{
	if(msg_length<1)
		return 0;
	float rect[4]={};
	QuadCoords *txc=nullptr;
	int msg_width=0, width, idx, printable_count=0, tab_width=tab_count*dx*zoom, w2=dx*zoom, height=dy*zoom, req_width=req_cols*dx*zoom;
	if(y+height<0||y>=h)//off-screen optimization
		return calc_width(x, y, msg, msg_length, tab_origin, zoom);
	vrtx.resize(msg_length<<4);//vx, vy, txx, txy		x4 vertices/char
	int rx1=0, ry1=0, rdx=0, rdy=0;
	get_current_region(rx1, ry1, rdx, rdy);
	float CX1=2.f/rdx, CX0=CX1*(x-rx1)-1;
	rect[1]=1-(y-ry1)*2.f/rdy;
	rect[3]=1-(y+height-ry1)*2.f/rdy;
	int k=ret_idx?*ret_idx:0;
	if(req_width<0||x+msg_width<req_width)
	{
		for(;k<msg_length;++k)
		{
			char c=msg[k];
			if(c>=32&&c<0xFF)
				width=w2;
			else if(c=='\t')
				width=tab_width-mod(x+msg_width-tab_origin, tab_width), c=' ';
			else
				width=0;
			if(width)
			{
				if(x+msg_width+width>=0&&x+msg_width<w)//off-screen optimization
				{
					rect[0]=CX1*msg_width+CX0;//xn1
					msg_width+=width;
					rect[2]=CX1*msg_width+CX0;//xn2

					//rect[0]=(x+msg_width-rx1)*2.f/rdx-1;//xn1
					//rect[1]=1-(y-ry1)*2.f/rdy;//yn1
					//rect[2]=(x+msg_width+width-rx1)*2.f/rdx-1;//xn2
					//rect[3]=1-(y+height-ry1)*2.f/rdy;//yn2

					//toNDC_nobias(float(x+msg_width		), float(y			), rect[0], rect[1]);
					//toNDC_nobias(float(x+msg_width+width	), float(y+height	), rect[2], rect[3]);//y2<y1

					idx=printable_count<<4;
					txc=font_coords+c-32;
					vrtx[idx   ]=rect[0], vrtx[idx+ 1]=rect[1],		vrtx[idx+ 2]=txc->x1, vrtx[idx+ 3]=txc->y1;//top left
					vrtx[idx+ 4]=rect[0], vrtx[idx+ 5]=rect[3],		vrtx[idx+ 6]=txc->x1, vrtx[idx+ 7]=txc->y2;//bottom left
					vrtx[idx+ 8]=rect[2], vrtx[idx+ 9]=rect[3],		vrtx[idx+10]=txc->x2, vrtx[idx+11]=txc->y2;//bottom right
					vrtx[idx+12]=rect[2], vrtx[idx+13]=rect[1],		vrtx[idx+14]=txc->x2, vrtx[idx+15]=txc->y1;//top right

					++printable_count;
				}
				else
					msg_width+=width;
				if(req_width>=0&&x+msg_width>=req_width)
				{
					++k;
					break;
				}
			}
		}
		if(printable_count)
		{
			setGLProgram(shader_text.program);
			select_texture(font_txid, ns_text::u_atlas);
			glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);									GL_CHECK();
			glBufferData(GL_ARRAY_BUFFER, printable_count<<6, vrtx.data(), GL_STATIC_DRAW);	GL_CHECK();//set vertices & texcoords
			glVertexAttribPointer(ns_text::a_coords, 4, GL_FLOAT, GL_TRUE, 0, 0);			GL_CHECK();

			glEnableVertexAttribArray(ns_text::a_coords);	GL_CHECK();
			glDrawArrays(GL_QUADS, 0, printable_count<<2);	GL_CHECK();//draw the quads: 4 vertices per character quad
			glDisableVertexAttribArray(ns_text::a_coords);	GL_CHECK();
		}
	}
	if(ret_idx)
		*ret_idx=k;
	return msg_width;
}
int					debug_print_cols(int x, int y, short zoom, int req_cols, int den)
{
	float rect[4]={};
	QuadCoords *txc=nullptr;
	int msg_width=0, width, idx, printable_count=0, tab_width=tab_count*dx*zoom, w2=dx*zoom, height=dy*zoom, req_width=req_cols*dx*zoom;
	vrtx.resize(req_cols<<4);//vx, vy, txx, txy		x4 vertices/char
	int rx1=0, ry1=0, rdx=0, rdy=0;
	get_current_region(rx1, ry1, rdx, rdy);
	float CX1=2.f/rdx, CX0=CX1*(x-rx1)-1;
	rect[1]=1-(y-ry1)*2.f/rdy;
	rect[3]=1-(y+height-ry1)*2.f/rdy;
	int k=0;
	for(;;++k)
	{
		char c='0'+k/den%10;
		width=w2;
		if(x+msg_width+width>=0&&x+msg_width<w)//off-screen optimization
		{
			rect[0]=CX1*msg_width+CX0;//xn1
			msg_width+=width;
			rect[2]=CX1*msg_width+CX0;//xn2

			idx=printable_count<<4;
			txc=font_coords+c-32;
			vrtx[idx   ]=rect[0], vrtx[idx+ 1]=rect[1],		vrtx[idx+ 2]=txc->x1, vrtx[idx+ 3]=txc->y1;//top left
			vrtx[idx+ 4]=rect[0], vrtx[idx+ 5]=rect[3],		vrtx[idx+ 6]=txc->x1, vrtx[idx+ 7]=txc->y2;//bottom left
			vrtx[idx+ 8]=rect[2], vrtx[idx+ 9]=rect[3],		vrtx[idx+10]=txc->x2, vrtx[idx+11]=txc->y2;//bottom right
			vrtx[idx+12]=rect[2], vrtx[idx+13]=rect[1],		vrtx[idx+14]=txc->x2, vrtx[idx+15]=txc->y1;//top right

			++printable_count;
		}
		else
			msg_width+=width;
		if(req_width>=0&&x+msg_width>=req_width)
		{
			++k;
			break;
		}
	}
	if(printable_count)
	{
		setGLProgram(shader_text.program);
		select_texture(font_txid, ns_text::u_atlas);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);									GL_CHECK();
		glBufferData(GL_ARRAY_BUFFER, printable_count<<6, vrtx.data(), GL_STATIC_DRAW);	GL_CHECK();//set vertices & texcoords
		glVertexAttribPointer(ns_text::a_coords, 4, GL_FLOAT, GL_TRUE, 0, 0);			GL_CHECK();

		glEnableVertexAttribArray(ns_text::a_coords);	GL_CHECK();
		glDrawArrays(GL_QUADS, 0, printable_count<<2);	GL_CHECK();//draw the quads: 4 vertices per character quad
		glDisableVertexAttribArray(ns_text::a_coords);	GL_CHECK();
	}
	return msg_width;
}
int					print(short zoom, int tab_origin, int x, int y, const char *format, ...)
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
};
int					print_line_rect(int x, int y, const char *msg, int msg_length, int tab_origin, short zoom, int nchars, int &nconsumed)
{
	if(msg_length<1)
		return 0;
	float rect[4]={};
	QuadCoords *txc=nullptr;
	int msg_width=0, width, idx, printable_count=0, tab_width=tab_count*dx*zoom, w2=dx*zoom, height=dy*zoom, k2=0;
	char overflow=false;
	vrtx.resize(nchars<<4);//vx, vy, txx, txy		x4 vertices/char
	for(int k=0, k3=0;k3<nchars;++k)
	{
		char c;
		if(overflow||k>=msg_length||msg[k]=='\n')
			c=' ', overflow=true;
		else
			c=msg[k], ++k2;
		if(c=='\t')
			width=tab_width-mod(x+msg_width-tab_origin, tab_width), c=' ', k3+=width/w2;
		else if(c>=32&&c<0xFF)
			width=w2, ++k3;
		else
			width=0, ++k3;
		if(width)
		{
			toNDC_nobias(float(x+msg_width		), float(y			), rect[0], rect[1]);
			toNDC_nobias(float(x+msg_width+width), float(y+height	), rect[2], rect[3]);//y2<y1
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
	nconsumed=k2;
	return msg_width;
}
void				relocate_range(int dst, int &a, int &b)
{
	if(a<b)
	{
		b=dst+(b-a);
		a=dst;
	}
	else
	{
		a=dst+(a-b);
		b=dst;
	}
}

//U64				colors_text=0xFFABABABFF000000;//black on white		0xBKBKBKBK_TXTXTXTX
U64					colors_text=0x20ABABABFFABABAB;//dark mode
//U64				colors_text=0xFF000000FFABABAB;//dark mode, opaque black on black?
U64					colors_selection=0xA0FF9933FFFFFFFF;
//U64				colors_selection=0xA0FF0000FFABABAB;
int					color_cursorlinebk=0xFF202020;
U64					colors_cursorline=(u64)color_cursorlinebk<<32|0xFFABABAB;
struct				SmallRect
{
	short x1, y1, x2, y2;
	SmallRect():x1(0), y1(0), x2(0), y2(0){}
	void set(short x1, short x2, short y1, short y2){this->x1=x1, this->x2=x2, this->y1=y1, this->y2=y2;}
};
struct				Rect
{
	int x1, y1, x2, y2;
	Rect():x1(0), y1(0), x2(0), y2(0){}
	void set(int x1, int x2, int y1, int y2){this->x1=x1, this->x2=x2, this->y1=y1, this->y2=y2;}
	void set_sort(int x1, int x2, int y1, int y2)
	{
		if(x1<x2)
			this->x1=x1, this->x2=x2;
		else
			this->x1=x2, this->x2=x1;
		if(y1<y2)
			this->y1=y1, this->y2=y2;
		else
			this->y1=y2, this->y2=y1;
	}
};
SmallRect			window_tabbar, window_editor;
struct				Bookmark
{
	int line, idx, col;
	Bookmark():line(0), idx(0), col(0){}
	Bookmark(int line, int idx, int col):line(line), idx(idx), col(col){}
	bool set_gui(Text const &text, int line, float col)//returns true if text is hit
	{
		bool hit=true;
		if(line<0)
			line=0, hit=false;
		else if(line>(int)text.size()-1)
			line=(int)text.size()-1, hit=false;
		this->line=line;
		auto &_line=text[line];
		this->col=col;
		//col2idx(_line.c_str(), _line.size(), 0, col, &this->idx, nullptr);//
		col2idx(_line.c_str(), _line.size(), 0, 0, 0, col, &this->idx, &this->col);
	//	update_idx(text);
		return hit;
	}
	void set_text(Text const &text, int line, int idx)
	{
		if(line<0)
			line=0;
		else if(line>(int)text.size()-1)
			line=(int)text.size()-1;
		this->line=line, this->idx=idx;
		update_col(text);
	}
	void setzero(){line=idx=col=0;}
	void setend(Text const &text)
	{
		line=text.size()-1, idx=text[line].size();
		update_col(text);
	}
	void set_linestart(){idx=col=0;}
	void set_lineend(Text const &text)
	{
		idx=text[line].size();
		update_col(text);
	}
	int get_absidx(Text const &text)const
	{
		int absidx=0;
		for(int kl=0;kl<line;++kl)
			absidx+=text[kl].size()+1;
		absidx+=idx;
		return absidx;
	}
	void set_absidx(Text const &text, int absidx)
	{
		for(line=0;;++line)
		{
			int delta=text[line].size()+1;
			if(absidx<delta)
				break;
			absidx-=delta;
		}
		idx=absidx;
		update_col(text);
	}

	void jump_vertical(Text const &text, int delta_line)
	{
		line+=delta_line;
		if(line<0)
			line=0, idx=0, col=0;
		else if(line>(int)text.size()-1)
		{
			line=(int)text.size()-1, idx=text[line].size();
			update_col(text);
		}
		else
			update_idx(text);
	}
	void increment_idx_skipnewline(Text const &text)
	{
		if(idx<(int)text[line].size()-1)
			++idx;
		else if(line<(int)text.size()-1)
			++line, idx=0;
	}
	void decrement_idx_skipnewline(Text const &text)
	{
		if(idx)
			--idx;
		else if(line>0)
			--line, idx=(int)text[line].size()-1;
	}
	bool increment_idx(Text const &text)
	{
		if(idx<(int)text[line].size())
		{
			++idx;
			return true;
		}
		else if(line<(int)text.size()-1)
		{
			++line, idx=0;
			return true;
		}
		return false;
	}
	bool decrement_idx(Text const &text)
	{
		if(idx)
		{
			--idx;
			return true;
		}
		else if(line>0)
		{
			--line, idx=(int)text[line].size();
			return true;
		}
		return false;
	}
	//char dereference_idx_unchecked(Text const &text)const{return text[line][idx];}
	char dereference_idx(Text const &text)const
	{
		if(line>=0&&line<(int)text.size())
		{
			auto &_line=text[line];
			if(idx==_line.size())
				return '\n';
			if(idx>=0&&idx<(int)_line.size())
				return _line[idx];
		}
		return '\0';
	}
	void update_col(Text const &text)
	{
		auto &_line=text[line];
		this->col=idx2col(_line.c_str(), idx, 0);//what if idx > size ?
	}
	void update_idx(Text const &text)
	{
		auto &_line=text[line];
		col2idx(_line.c_str(), _line.size(), 0, 0, 0, col, &this->idx, &this->col);
	}
};
bool				operator==(Bookmark const &left, Bookmark const &right){return left.line==right.line&&left.idx==right.idx;}
bool				operator!=(Bookmark const &left, Bookmark const &right){return left.line!=right.line||left.idx!=right.idx;}
bool				operator<(Bookmark const &left, Bookmark const &right)
{
	if(left.line==right.line)
		return left.idx<right.idx;
	return left.line<right.line;
}
bool				operator<=(Bookmark const &left, Bookmark const &right){return !(right<left);}
bool				operator>(Bookmark const &left, Bookmark const &right){return right<left;}
bool				operator>=(Bookmark const &left, Bookmark const &right){return right<=left;}
int					bookmark_subtract(Bookmark const &a, Bookmark const &b, Text const &text)//returns a-b
{
	Bookmark const *p1, *p2;
	bool negative=a<b;
	if(negative)
		p1=&a, p2=&b;
	else
		p1=&b, p2=&a;
	int diff=0;
	if(p1->line==p2->line)
		diff=p1->idx-p2->idx;
	else
	{
		diff-=text[p1->line].size()+1-p1->idx;
		for(int kl=p1->line+1;kl<p2->line-1;++kl)
			diff-=text[kl].size()+1;//account for newline
		diff-=p2->idx;
	}
	if(!negative)
		diff=-diff;
	return diff;
}
int					bookmark_add(Bookmark &b, int offset, Text const &text)//returns remainder on overflow
{
	if(offset>0)
	{
		int line_max=(int)text.size()-1;
		for(;;)
		{
			auto &line=text[b.line];
			if(b.idx+offset<(int)line.size()+1)
			{
				b.idx+=offset;
				offset=0;
				break;
			}
			if(b.line<line_max)
			{
				offset-=line.size()+1-b.idx;
				++b.line, b.idx=0;
			}
			else//overflow
			{
				b.idx=line.size();
				break;
			}
		}
	}
	else if(offset<0)
	{
		for(;;)
		{
			if(b.idx+offset>0)
			{
				b.idx+=offset;
				offset=0;
				break;
			}
			if(b.line>0)
			{
				offset+=b.idx;
				--b.line;
				b.idx=text[b.line].size();
			}
			else//overflow
			{
				offset+=b.idx;
				b.idx=0;
				break;
			}
		}
	}
	return offset;
}
void				bookmark_relocate(Bookmark const &i, Bookmark const &f, Bookmark &b, Text const &text)
{
	int diff=bookmark_subtract(f, i, text);
	bookmark_add(b, diff, text);
}
struct				Cursor
{
	bool rectsel;
	Bookmark cursor, selcur;

	Cursor():rectsel(false){}
	void deselect(Text const &text, int direction)//direction: -1: left, 0: don't care, 1: right
	{
		rectsel=false;
		if(direction)
		{
			if(rectsel)
			{
				auto r=get_rectsel();
				if(direction==-1)
					cursor.set_gui(text, r.y1, r.x1);
				else
					cursor.set_gui(text, r.y2, r.x2);
			}
			else
			{
				Bookmark i, f;
				get_selection(i, f);
				if(direction==-1)
					cursor=i;
				else
					cursor=f;
			}
		}
		selcur=cursor;
	}
	void reset()
	{
		rectsel=false;
		cursor.setzero();
		selcur.setzero();
	}
	void get_selection(Bookmark &i, Bookmark &f)const
	{
		if(selcur<cursor)
			i=selcur, f=cursor;
		else
			i=cursor, f=selcur;
	}
	Rect get_rectsel()const
	{
		Rect r;
		r.set_sort(cursor.col, selcur.col, cursor.line, selcur.line);
		return r;
	}
	void get_rectsel(int &l1, int &l2, int &col1, int &col2)const
	{
		Rect r=get_rectsel();
		l1=r.y1, l2=r.y2;
		col1=r.x1, col2=r.x2;
	}
	void set_rectsel(Text const &text, int l1, int l2, int col1, int col2)
	{
		cursor.set_gui(text, l1, col1);
		selcur.set_gui(text, l2, col2);
	}
	bool selection_exists()const
	{
		if(rectsel)
			return cursor.col!=selcur.col;
		return cursor.line!=selcur.line||cursor.col!=selcur.col;
	}
	bool does_contain(Bookmark const &b)const
	{
		if(rectsel)
		{
			auto r=get_rectsel();
			return b.line>=r.y1&&b.line<=r.y2&&b.col>=r.x1&&b.col<r.x2;
		}
		Bookmark i, f;
		get_selection(i, f);
		return b>=i&&b<=f;
	}
};
short				font_zoom=1;//font pixel size
int					wpx=0, wpy=0,//window position inside the text buffer, in pixels
					text_width=0;//in characters
Cursor				*cur=nullptr;
void				text2str(Text const &text, std::string &str)
{
	str.clear();
	if(text.size())
	{
		int nlines_m1=(int)text.size()-1;
		for(int kl=0;kl<nlines_m1;++kl)
		{
			str+=text[kl];
			str+='\n';
		}
		str+=text[nlines_m1];
	}
}
void				str2text(const char *str, int len, Text &text)
{
	text.clear();
	for(int k=0, k2=0;;++k2)
	{
		if(k2>=len)
		{
			text.push_back(std::string(str+k, str+k2));
			break;
		}
		if(str[k2]=='\n')
		{
			text.push_back(std::string(str+k, str+k2));
			k=k2+1;
		}
	}
}
void				rectsel_copy(Text const &src, Cursor const &cur, Text &dst)
{
	dst.clear();
	int l1, l2, col1, col2;
	cur.get_rectsel(l1, l2, col1, col2);
	std::string temp;
	for(int kl=l1;kl<=l2;++kl)
	{
		auto &line=src[kl];

		int idx1, c1, prepad=0,
			idx2, c2;

		col2idx(line.c_str(), line.size(), 0, 0, 0, col1, &idx1, &c1);
		if(c1<col1&&idx1<(int)line.size())
		{
			int c1_2=c1+tab_count-mod(c1, tab_count);
			if(c1_2<=col2)
				++idx1, c1=c1_2;
		}
		if(c1>col1)
			prepad=c1-col1;

		col2idx(line.c_str(), line.size(), 0, idx1, c1, col2, &idx2, &c2);
		idx2-=c2>col2;

		temp.append(prepad, ' ');
		if(idx1<idx2)
			temp.append(line.begin()+idx1, line.begin()+idx2);
		temp.append(col2-col1-temp.size(), ' ');
		dst.push_back(std::move(temp));
	}
}
void				rectsel_insert(Text const &src, int l1, int col1, Text &dst)
{
	int kls=0, kld=l1;
	for(;kls<(int)src.size()&&kld<(int)dst.size();++kls, ++kld)
	{
		auto &srcline=src[kls];
		auto &dstline=dst[kld];
		int idx1, c1;
		col2idx(dstline.c_str(), dstline.size(), 0, 0, 0, col1, &idx1, &c1);
		if(c1<col1)
		{
			dstline.insert(dstline.begin()+idx1, col1-c1, ' ');
			idx1+=col1-c1;
		}
		dstline.insert(dstline.begin()+idx1, srcline.begin(), srcline.end());
	}
	for(;kls<(int)src.size();++kls)
	{
		dst.push_back(std::string(col1/tab_count, '\t'));
		auto &line=dst.back();
		line.append(mod(col1, tab_count), ' ');
		line+=src[kls];
	}
}
void				selection_copy(Text const &src, Cursor const &cur, Text &dst)
{
	dst.clear();
	Bookmark i, f;
	cur.get_selection(i, f);
	if(i.line==f.line)
	{
		auto &line=src[i.line];
		dst.push_back(std::string(line.begin()+i.idx, line.begin()+f.idx));
	}
	else
	{
		auto &firstline=src[i.line], &lastline=src[f.line];
		dst.push_back(std::string(firstline.begin()+i.idx, firstline.end()));
		for(int kl=i.line+1;kl<f.line;++kl)
			dst.push_back(src[kl]);
		dst.push_back(std::string(lastline.begin(), lastline.begin()+f.idx));
	}
}
void				selection_insert(Text const &src, int l1, int idx1, Text &dst)
{
	auto &srcline=src[0];
	if(src.size()==1)
	{
		auto &dstline=dst[l1];
		dstline.insert(dstline.begin()+idx1, srcline.begin(), srcline.end());
	}
	else if(src.size()>1)
	{
		dst.insert(dst.begin()+l1+1, src.begin()+1, src.end());
		auto &dst1=dst[l1], &dst2=dst[l1+src.size()-1];
		dst2.insert(dst2.end(), dst1.begin()+idx1, dst1.end());
		dst1.replace(dst1.begin()+idx1, dst1.end(), srcline.begin(), srcline.end());
	}
}

enum				ActionType
{
	ACTION_UNINITIALIZED,
	ACTION_INSERT,
	ACTION_INSERT_RECT,
	ACTION_ERASE,
	ACTION_ERASE_SELECTION,
	ACTION_ERASE_RECT_SELECTION,
	ACTION_INDENT_FORWARD,
	ACTION_INDENT_BACK,
	ACTION_REPLACE,
	ACTION_REPLACE_RECT,
};
/*struct				ActionFragment
{
	int idx;
	std::string str;
	ActionFragment():idx(0){}
	ActionFragment(ActionFragment const &af):idx(af.idx), str(af.str){}
	ActionFragment(ActionFragment &&af):idx(af.idx), str((std::string&&)af.str){}
	ActionFragment(int idx, const char *a, int len):idx(idx), str(a, len){}
	ActionFragment& operator=(ActionFragment const &af)
	{
		if(&af!=this)
		{
			idx=af.idx;
			str=af.str;
		}
		return *this;
	}
	ActionFragment& operator=(ActionFragment &&af)
	{
		if(&af!=this)
		{
			idx=af.idx;
			str=std::move(af.str);
		}
		return *this;
	}
};
typedef std::vector<ActionFragment> ActionData;//*/
struct				HistoryAction
{
	ActionType type;
	Text data_ins, data_rem;
	//ActionData data;
	//Bookmark b1, b2;
	Cursor cur_before, cur_control, cur_after;
	//union//initial cursor positions
	//{
	//	struct{int scx0, ccx0, scy0, ccy0;};
	//	struct{int cursor0, selcur0, z_unused0, z_unused1;};
	//};
	HistoryAction():type(ACTION_UNINITIALIZED){}
	HistoryAction(HistoryAction const &h):type(h.type), data_ins(h.data_ins), data_rem(h.data_rem), cur_before(h.cur_before), cur_after(h.cur_after){}
	HistoryAction(HistoryAction &&h):type(h.type), data_ins(std::move(h.data_ins)), data_rem(std::move(h.data_rem)), cur_before(h.cur_before), cur_after(h.cur_after){}
	//HistoryAction(ActionType type, int idx, const char *a, int len, Cursor const &cur_before, Cursor const &cur_after):type(type), cur_before(cur_before), cur_after(cur_after)//normal editing
	//{
	//	data.push_back(ActionFragment(idx, a, len));
	//}
	//HistoryAction(ActionType type, Text &&data, Cursor const &cur_before, Cursor const &cur_after):type(type), data(std::move(data)), cur_before(cur_before), cur_after(cur_after){}//rectangular editing
	/*HistoryAction(ActionType type, Text const &text, Cursor const &cur1, Cursor const &cur2):type(type), cur_before(cur1), cur_after(cur2)
	{
		switch(type)
		{
		case ACTION_UNINITIALIZED:
			break;
		case ACTION_INSERT:
			selection_copy(text, cur2, data);
			break;
		case ACTION_INSERT_RECT:
			rectsel_copy(text, cur2, data);
			break;
		case ACTION_ERASE:
			break;
		case ACTION_ERASE_SELECTION:
		case ACTION_ERASE_RECT_SELECTION:
		case ACTION_INDENT_BACK:
			break;
		}
	}//*/
	HistoryAction& operator=(HistoryAction const &h)
	{
		if(&h!=this)
		{
			type=h.type;
			data_ins=h.data_ins;
			data_rem=h.data_rem;
			cur_before=h.cur_before;
			cur_after=h.cur_after;
		}
		return *this;
	}
	HistoryAction& operator=(HistoryAction &&h)
	{
		if(&h!=this)
		{
			type=h.type;
			data_ins=std::move(h.data_ins);
			data_rem=std::move(h.data_rem);
			cur_before=h.cur_before;
			cur_after=h.cur_after;
		}
		return *this;
	}
};
typedef std::vector<HistoryAction> History;

std::string			*filename;//utf8
Text				*text=nullptr;
//std::string		text="Hello.\nSample Text.\nWhat\'s going on???\n";
char				caps_lock=false;

enum				DragType
{
	DRAG_NONE,
	DRAG_SELECT,
	DRAG_RECT,
	DRAG_VSCROLL,
	DRAG_HSCROLL,
	DRAG_TAB,
	DRAG_MOVE_SELECTION,
	DRAG_COPY_SELECTION,
};
DragType			drag;
int					drag_tab_idx=0;
Cursor				drag_cursor;
//std::vector<std::string> drag_selection;

const int			scrollbarwidth=17;
struct				Scrollbar
{
	short
		dwidth,	//dynamic width: 0 means hidden, 'scrollbarwidth' means exists
		m_start,//starting mouse position
		s0,		//initial scrollbar slider start
		start,	//scrollbar slider start
		size;	//scrollbar slider size
	void click_on_slider(int mp){m_start=mp, s0=start;}
	void decide(char condition){dwidth=scrollbarwidth&-condition;}//decide if the scrollbar will be drawn
	void decide_orwith(char condition){dwidth|=scrollbarwidth&-condition;}
	void leftbuttondown(int m){m_start=m, s0=start;}
};
Scrollbar			vscroll={}, hscroll={};

History				*history=nullptr;
#define				ACTION(K)	history->operator[](K)
int					*histpos=nullptr;//pointing at previous action
bool				hist_cont=true;

bool				dimensions_known=false, wnd_to_cursor=false;

struct				TextFile
{
	std::string m_filename;//UTF-8
	Text m_text;//ASCII
	int m_untitled_idx;

	float m_wcx, m_wcy;//window position inside the text buffer, in character units
	Cursor m_cur;

	History m_history;
	int m_histpos_saved, m_histpos;//pointing at previous action

	TextFile():m_text(1, std::string()), m_untitled_idx(0), m_wcx(0), m_wcy(0), m_histpos_saved(-1), m_histpos(-1){}
	TextFile(TextFile const &f):m_filename(f.m_filename), m_text(f.m_text), m_untitled_idx(f.m_untitled_idx),
		m_wcx(f.m_wcx), m_wcy(f.m_wcy), m_cur(f.m_cur),
		m_history(f.m_history), m_histpos_saved(f.m_histpos_saved), m_histpos(f.m_histpos){}
	TextFile(TextFile &&f):m_filename(std::move(f.m_filename)), m_text(std::move(f.m_text)), m_untitled_idx(f.m_untitled_idx),
		m_wcx(f.m_wcx), m_wcy(f.m_wcy), m_cur(f.m_cur),
		m_history(std::move(f.m_history)), m_histpos_saved(f.m_histpos_saved), m_histpos(f.m_histpos){}
	TextFile& operator=(TextFile const &f)
	{
		if(&f!=this)
		{
			m_filename=f.m_filename;
			m_text=f.m_text;
			m_untitled_idx=f.m_untitled_idx;

			m_wcx=f.m_wcx, m_wcy=f.m_wcy;
			m_cur=f.m_cur;

			m_history=f.m_history;
			m_histpos_saved=f.m_histpos_saved, m_histpos=f.m_histpos;
		}
		return *this;
	}
	TextFile& operator=(TextFile &&f)
	{
		if(&f!=this)
		{
			m_filename=std::move(f.m_filename);
			m_text=std::move(f.m_text);
			m_untitled_idx=f.m_untitled_idx;

			m_wcx=f.m_wcx, m_wcy=f.m_wcy;
			m_cur=f.m_cur;

			m_history=std::move(f.m_history);
			m_histpos_saved=f.m_histpos_saved, m_histpos=f.m_histpos;
		}
		return *this;
	}
	void clear()
	{
		m_filename.clear();
		m_text.clear();
		m_untitled_idx=0;
		m_wcx=0, m_wcy=0;
		m_cur.reset();
		m_history.clear();
		m_histpos_saved=m_histpos=-1;
	}
};
std::vector<TextFile> openfiles;
int					current_file=0;

int					gen_untitled_idx()
{
	int max_idx=0;
	for(int k=0;k<(int)openfiles.size();++k)
	{
		auto &file=openfiles[k];
		if(!file.m_filename.size()&&max_idx<file.m_untitled_idx)
			max_idx=file.m_untitled_idx;
	}
	std::vector<bool> reserved(max_idx+1);
	for(int k=0;k<(int)openfiles.size();++k)
	{
		auto &file=openfiles[k];
		if(!file.m_filename.size())
			reserved[file.m_untitled_idx]=true;
	}
	int k2=0;
	for(;k2<(int)reserved.size()&&reserved[k2];++k2);
	return k2;
}

struct				ClosedFile
{
	int tab_idx;
	std::string filename;
	ClosedFile():tab_idx(0){}
	ClosedFile(int tab_idx, std::string const &filename):tab_idx(tab_idx), filename(filename){}
};
std::vector<ClosedFile> closedfiles;

enum				TabbarPosition
{
	TABBAR_NONE,
	TABBAR_TOP,
	TABBAR_LEFT,
	TABBAR_RIGHT,
	TABBAR_BOTTOM,
	TABBAR_ORIENT_COUNT,
};
TabbarPosition		tabbar_position=TABBAR_TOP;
int					tabbar_dx=150,//sidebar width		TODO: resizable
					tabbar_wpx=0, tabbar_wpy;//scroll position
struct				TabPosition
{
	int right, idx;
	TabPosition():right(0), idx(0){}
	TabPosition(int right, int idx):right(right), idx(idx){}
	void set(int right, int idx){this->right=right, this->idx=idx;}
};
std::vector<TabPosition> tabbar_tabs;//TODO: merge TabPosition with TextFile

bool				tabs_ismodified(int tab_idx)
{
	auto &of=openfiles[tab_idx];
	return of.m_histpos!=of.m_histpos_saved;
}
void				set_title()
{
	int printed=0;

	if(tabs_ismodified(current_file))
		printed+=sprintf_s(g_buf+printed, g_buf_size-printed, "*");

	printed+=sprintf_s(g_buf+printed, g_buf_size-printed, "%d/%d - ", current_file+1, (int)openfiles.size());

	if(filename->size())
		printed+=sprintf_s(g_buf+printed, g_buf_size-printed, "%s - ", filename->c_str());

	printed+=sprintf_s(g_buf+printed, g_buf_size-printed, "Teletext");
	set_window_title(g_buf);
}
void				debug_set_window_title(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vsprintf_s(g_buf, g_buf_size, format, args);
	va_end(args);
	set_window_title(g_buf);
}
void				tabs_switchto(int k, bool change_title=true)
{
	TextFile *of=nullptr;
	if(current_file>=0&&current_file<(int)openfiles.size())
	{
		of=&openfiles[current_file];
		of->m_wcx=(float)wpx/font_zoom;
		of->m_wcy=(float)wpy/font_zoom;
	}

	if(openfiles.size()<1)
		openfiles.push_back(TextFile());
	current_file=mod(k, openfiles.size());

	of=&openfiles[current_file];
	text=&of->m_text;
	filename=&of->m_filename;
	history=&of->m_history;
	histpos=&of->m_histpos;

	wpx=int(of->m_wcx*font_zoom);
	wpy=int(of->m_wcy*font_zoom);
	cur=&of->m_cur;

	if(change_title)
		set_title();
	dimensions_known=false;
}

int					calc_cols(Text const &text)
{
	int cols=0;
	for(int kl=0;kl<(int)text.size();++kl)
	{
		auto &line=text[kl];
		int temp=idx2col(line.c_str(), line.size(), 0);
		if(cols<temp)
			cols=temp;
	}
	return cols;
}

void				text_erase(Text &text, int l1, int idx1, int l2, int idx2)
{
	if(l1==l2)
	{
		if(idx1!=idx2)
		{
			auto &line=text[l1];
			if(idx2<idx1)
				std::swap(idx1, idx2);
			line.erase(line.begin()+idx1, line.begin()+idx2);
		}
	}
	else
	{
		if(l2<l1)
		{
			std::swap(l1, l2);
			std::swap(idx1, idx2);
		}
		auto &line1=text[l1], &line2=text[l2];
		line1.replace(line1.begin()+idx1, line1.end(), line2.begin()+idx2, line2.end());
		text.erase(text.begin()+l1+1, text.begin()+l2+1);
	}
}
void				text_insert(Text &text, int l0, int idx0, const char *a, int len, int *ret_line, int *ret_idx)
{
	int k=0, k2=0;
	for(int kl=l0;;++kl)
	{
		for(;k2<len&&a[k2]!='\n';++k2);
		if(k2>=len)
		{
			int idx=k2-k;
			if(k)
				text[kl].insert(text[kl].begin(), a+k, a+k2);
			else
			{
				text[kl].insert(text[kl].begin()+idx0, a+k, a+k2);
				idx+=idx0;
			}
			if(ret_line)
				*ret_line=kl;
			if(ret_idx)
				*ret_idx=idx;
			break;
		}
		else if(k)
			text.insert(text.begin()+kl, std::string(a+k, a+k2));
		else
		{
			text.insert(text.begin()+l0+1, text[l0].substr(idx0, text[l0].size()-idx0));
			text[l0].replace(text[l0].begin()+idx0, text[l0].end(), a+k, a+k2);
		}
		++k2;
		k=k2;
	}
}
void				replace_tab_with_spaces(std::string &line, int idx)
{
	int col0=idx2col(line.c_str(), idx, 0);
	int nspaces=tab_count-mod(col0, tab_count);
	line.replace(idx, 1, nspaces, ' ');
}
void				text_erase_rect(Text &text, int l1, int l2, int col1, int col2)
{
	if(l2<l1)
		std::swap(l1, l2);
	if(col2<col1)
		std::swap(col1, col2);
	//int before=idx2col(text[l1].c_str(), text[l1].size(), 0);
	for(int kl=l1;kl<=l2&&kl<(int)text.size();++kl)
	{
		auto &line=text[kl];
		int idx1=0, c1=0;
		col2idx(line.c_str(), line.size(), 0, 0, 0, col1, &idx1, &c1);
		if(c1!=col1)//tab caused misalign
		{
			if(idx1<(int)line.size()&&line[idx1]=='\t')
			{
				replace_tab_with_spaces(line, idx1);
				col2idx(line.c_str(), line.size(), 0, 0, 0, col1, &idx1, &c1);
			}
			else if(idx1>0&&line[idx1-1]=='\t')
			{
				replace_tab_with_spaces(line, idx1-1);
				col2idx(line.c_str(), line.size(), 0, 0, 0, col1, &idx1, &c1);
			}
		}
		if(c1==col1)
		{
			int idx2=0, c2=0;
			//col2idx(line.c_str()+idx1, line.size()-idx1, -c1, col2-c1, &idx2, &c2);
			col2idx(line.c_str(), line.size(), 0, 0, 0, col2, &idx2, &c2);
			if(idx1<idx2)
				line.erase(line.begin()+idx1, line.begin()+idx2);
		}
	}
	//int after=idx2col(text[l1].c_str(), text[l1].size(), 0);
	//return after-before;
}
void				text_insert_rect(Text &text, int l1, int l2, int col0, const char *a, int len)
{
	for(int kl=l1;kl<=l2;++kl)
	{
		auto &line=text[kl];
		int idx1=0, c1=0;
		col2idx(line.c_str(), line.size(), 0, 0, 0, col0, &idx1, &c1);
		if(c1<col0)
		{
			int ntabs=col0/tab_count-c1/tab_count, nspaces=mod(col0, tab_count);
			line.insert(line.end(), ntabs, '\t');
			line.insert(line.end(), nspaces, ' ');
			idx1=line.size();
		}
		line.insert(line.begin()+idx1, a, a+len);
	}
}
void				text_insert1(Text &text, int l0, int idx0, char c)
{
	if(c=='\n')
	{
		text.insert(text.begin()+l0+1, std::string());
		auto &line1=text[l0];
		auto &line2=text[l0+1];
		line2.insert(line2.begin(), line1.begin()+idx0, line1.end());
		line1.erase(line1.begin()+idx0, line1.end());
	}
	else
		text[l0].insert(idx0, 1, c);
}
void				text_erase1_bksp(Text &text, int l0, int idx0)
{
	if(idx0>0)
		text[l0].erase(idx0-1, 1);
	else if(l0>0)
	{
		text[l0-1].append(text[l0]);
		text.erase(text.begin()+l0);
	}
}
void				text_erase1_del(Text &text, int l0, int idx0)
{
	auto &line=text[l0];
	if(idx0<(int)line.size())
		line.erase(idx0, 1);
	else if(l0<(int)text.size()-1)
	{
		text[l0].append(text[l0+1]);
		text.erase(text.begin()+l0+1);
	}
}
void				indent_rectsel_back(Cursor &cur)
{
	int l1, l2, col1, col2, col0;
	cur.get_rectsel(l1, l2, col1, col2);
	int remainder=mod(col1, tab_count);//tab_count can be NPOT
	remainder+=tab_count&-!remainder;
	col0=col1-remainder;
	int dst_col=col0;
	//std::vector<int> indices(l2+1-l1);
	for(int kl=l1;kl<=l2;++kl)//find col_clearance
	{
		//a   [b]b	here col_clearance = 1 space
		//aaa [b]b
		//aa  [b]b
		auto &line=text->operator[](kl);
		int idx, c;
		col2idx(line.c_str(), line.size(), 0, 0, 0, col0, &idx, &c);
		for(;idx<(int)line.size()&&!isspace(line[idx]);++idx, ++c);
		if(dst_col<c)
		{
			if(c>col1)
			{
				dst_col=-1;
				break;
			}
			dst_col=c;
		}
	}
	if(dst_col>=0)
	{
		for(int kl=l1;kl<=l2;++kl)
		{
			auto &line=text->operator[](kl);
			int idx1, c1, idx2, c2;
			col2idx(line.c_str(), line.size(), 0, 0, 0, dst_col, &idx1, &c1);
			col2idx(line.c_str(), line.size(), 0, 0, 0, col1, &idx2, &c2);
			line.erase(line.begin()+idx1, line.begin()+idx2);
		}
		int col_diff=col1-dst_col;
		cur.cursor.col-=col_diff;
		cur.selcur.col-=col_diff;
		cur.cursor.update_idx(*text);
		cur.selcur.update_idx(*text);
	}
}
void				indent_rectsel_forward(Cursor &cur)
{
	int l1, l2, col1, col2;
	cur.get_rectsel(l1, l2, col1, col2);
	for(int kl=l1;kl<l2;++kl)
	{
		auto &line=text->operator[](kl);
		int idx1, c1;
		col2idx(line.c_str(), line.size(), 0, 0, 0, col1, &idx1, &c1);
		line.insert(line.begin()+idx1, '\t');
	}
	int col_diff=tab_count-mod(col1, tab_count);
	cur.cursor.col+=col_diff;
	cur.selcur.col+=col_diff;
	cur.cursor.update_idx(*text);
	cur.selcur.update_idx(*text);
}
void				indent_selection_back(Cursor &cur)
{
	Bookmark i, f;
	cur.get_selection(i, f);
	int i_diff=0, f_diff=0;
	for(int kl=i.line;kl<=f.line;++kl)
	{
		auto &line=text->operator[](kl);
		int idx1, c1;
		col2idx(line.c_str(), line.size(), 0, 0, 0, tab_count, &idx1, &c1);
		int kc=0;
		for(;kc<idx1&&isspace(line[kc]);++kc);
		if(kl==i.line)
			i_diff=idx2col(line.c_str(), kc, 0);
		if(kl==f.line)
			f_diff=idx2col(line.c_str(), kc, 0);
		if(kc)
			line.erase(line.begin(), line.begin()+kc);
	}
	if(cur.cursor<cur.selcur)
		cur.cursor.col-=i_diff, cur.selcur.col-=f_diff;
	else
		cur.selcur.col-=i_diff, cur.cursor.col-=f_diff;
	cur.cursor.update_idx(*text);
	cur.selcur.update_idx(*text);
}
void				indent_selection_forward(Cursor &cur)
{
	Bookmark i, f;
	cur.get_selection(i, f);
	for(int kl=i.line;kl<=f.line;++kl)
	{
		auto &line=text->operator[](kl);
		line.insert(line.begin(), '\t');
	}
	cur.cursor.col+=tab_count;
	cur.selcur.col+=tab_count;
	cur.cursor.update_idx(*text);
	cur.selcur.update_idx(*text);
}

void				hist_undo()//ctrl z
{
	if(*histpos<0)
		return;

	auto &action=ACTION(*histpos);
	--*histpos;
	switch(action.type)
	{
	//case ACTION_INSERT:
	//	text_erase(*text, action.cur_control.cursor.line, action.cur_control.cursor.idx, action.cur_control.selcur.line, action.cur_control.selcur.idx);
	//	break;
	//case ACTION_INSERT_RECT:
	//case ACTION_INDENT_FORWARD:
	//	text_erase_rect(*text, action.cur_control.cursor.line, action.cur_control.selcur.line, action.cur_control.cursor.col, action.cur_control.selcur.col);
	//	break;
	//case ACTION_ERASE:
	//case ACTION_ERASE_SELECTION:
	//	selection_insert(action.data, action.cur_control.cursor.line, action.cur_control.cursor.idx, *text);
	//	break;
	//case ACTION_ERASE_RECT_SELECTION:
	//case ACTION_INDENT_BACK:
	//	rectsel_insert(action.data, action.cur_control.cursor.line, action.cur_control.cursor.col, *text);
	//	break;
	case ACTION_REPLACE:
		if(action.data_ins.size())
			text_erase(*text, action.cur_control.cursor.line, action.cur_control.cursor.idx, action.cur_control.selcur.line, action.cur_control.selcur.idx);
		if(action.data_rem.size())
			selection_insert(action.data_rem, action.cur_control.cursor.line, action.cur_control.cursor.idx, *text);
		break;
	case ACTION_REPLACE_RECT:
		if(action.data_ins.size())
			text_erase_rect(*text, action.cur_control.cursor.line, action.cur_control.selcur.line, action.cur_control.cursor.col, action.cur_control.selcur.col);
		if(action.data_rem.size())
			rectsel_insert(action.data_rem, action.cur_control.cursor.line, action.cur_control.cursor.col, *text);
		break;
	}
	*cur=action.cur_before;

	hist_cont=false;
	dimensions_known=false;
	wnd_to_cursor=true;
	set_title();
}
void				hist_redo()//ctrl y
{
	if(*histpos>=(int)history->size()-1)
		return;
	++*histpos;
	auto &action=ACTION(*histpos);
	switch(action.type)
	{
	//case ACTION_INSERT:
	//	selection_insert(action.data, action.cur_control.cursor.line, action.cur_control.cursor.idx, *text);
	//	break;
	//case ACTION_INSERT_RECT:
	//case ACTION_INDENT_FORWARD:
	//	rectsel_insert(action.data, action.cur_control.cursor.line, action.cur_control.cursor.col, *text);
	//	break;
	//case ACTION_ERASE:
	//case ACTION_ERASE_SELECTION:
	//	text_erase(*text, action.cur_control.cursor.line, action.cur_control.cursor.idx, action.cur_control.selcur.line, action.cur_control.selcur.idx);
	//	break;
	//case ACTION_ERASE_RECT_SELECTION:
	//case ACTION_INDENT_BACK:
	//	text_erase_rect(*text, action.cur_control.cursor.line, action.cur_control.selcur.line, action.cur_control.cursor.col, action.cur_control.selcur.col);
	//	break;
	case ACTION_REPLACE:
		if(action.data_rem.size())
			text_erase(*text, action.cur_control.cursor.line, action.cur_control.cursor.idx, action.cur_control.selcur.line, action.cur_control.selcur.idx);
		if(action.data_ins.size())
			selection_insert(action.data_ins, action.cur_control.cursor.line, action.cur_control.cursor.idx, *text);
		break;
	case ACTION_REPLACE_RECT:
		if(action.data_rem.size())
			text_erase_rect(*text, action.cur_control.cursor.line, action.cur_control.selcur.line, action.cur_control.cursor.col, action.cur_control.selcur.col);
		if(action.data_ins.size())
			rectsel_insert(action.data_ins, action.cur_control.cursor.line, action.cur_control.cursor.col, *text);
		break;
	}
	*cur=action.cur_after;

	hist_cont=false;
	dimensions_known=false;
	wnd_to_cursor=true;
	set_title();
}

char				group_char(char c)
{
	if(c>='0'&&c<='9'||c=='.')
		return '0';
	if(c>='A'&&c<='Z'||c>='a'&&c<='z'||c=='_')
		return 'A';
	if(c==' '||c=='\t')
		return ' ';
	if(c=='\r'||c=='\n')
		return '\n';
	return c;
}
bool				cursor_at_mouse(Text const &text, Bookmark &cursor)//sets the cursor to mouse coordinates, returns true if text was hit
{
	int mousex=mx-window_editor.x1, mousey=my-window_editor.y1;
	int dypx=dy*font_zoom, dxpx=dx*font_zoom,
		line=(wpy+mousey)/dypx;
	float col=float(wpx+mousex)/dxpx;

	mline=line, mcol=(int)(col+0.5f);
	return cursor.set_gui(text, line, col);
}

//scrollbar functions
const int			color_barbk=0x30F0F0F0, color_slider=0x50CDCDCD, color_button=0x50E5E5E5;//0x80DDDDDD
void				scrollbar_slider(int &winpos_ip, int imsize_ip, int winsize, int barsize, double zoom, short &sliderstart, short &slidersize)
{
	double winsize_ip=winsize/zoom;
	imsize_ip+=int(winsize_ip*0.5);
	slidersize=int(barsize*winsize_ip/imsize_ip);
	if(slidersize<scrollbarwidth)
	{
		slidersize=scrollbarwidth;
		sliderstart=int((barsize-scrollbarwidth)*winpos_ip/(imsize_ip-winsize_ip));
	}
	else
		sliderstart=barsize*winpos_ip/imsize_ip;
	if(sliderstart>barsize-slidersize)
	{
		sliderstart=barsize-slidersize;
		winpos_ip=int((imsize_ip+2-winsize_ip)*sliderstart/(barsize-slidersize));
	}
}
void				scrollbar_scroll(int &winpos_ip, int imsize_ip, int winsize, int barsize, int slider0, int mp_slider0, int mp_slider, double zoom, short &sliderstart, short &slidersize)//_ip: in image pixels, mp_slider: mouse position starting from start of slider
{
	double winsize_ip=winsize/zoom;
	imsize_ip+=int(winsize_ip*0.5);
	double r=winsize_ip/imsize_ip;
	slidersize=int(barsize*r);
	bool minsize=slidersize<scrollbarwidth;
	if(minsize)
		slidersize=scrollbarwidth;
	sliderstart=slider0+mp_slider-mp_slider0;
	if(sliderstart<0)
		sliderstart=0;
	if(sliderstart>barsize-slidersize)
		sliderstart=barsize-slidersize;
	winpos_ip=int((imsize_ip+2-winsize_ip)*sliderstart/(barsize-slidersize));
}
void				draw_vscrollbar(int x, int sbwidth, int y1, int y2, int &winpos, int content_h, int vscroll_s0, int vscroll_my_start, double zoom, short &vscroll_start, short &vscroll_size, int dragtype)//vertical scrollbar
{
	draw_rectangle_i(x, x+sbwidth, y1, y1+sbwidth, color_button);//up
	draw_rectangle_i(x, x+sbwidth, y1+sbwidth, y2-sbwidth, color_barbk);//vertical
	draw_rectangle_i(x, x+sbwidth, y2-sbwidth, y2, color_button);//down
	int win_size=y2-y1, scroll_size=win_size-(sbwidth<<1);
	if(drag==dragtype)
		scrollbar_scroll(winpos, content_h, win_size, scroll_size, vscroll_s0, vscroll_my_start, my, zoom, vscroll_start, vscroll_size);
	else
		scrollbar_slider(winpos, content_h, win_size, scroll_size, zoom, vscroll_start, vscroll_size);
	draw_rectangle_i(x, x+sbwidth, y1+sbwidth+vscroll_start, y1+sbwidth+vscroll_start+vscroll_size, color_slider);//vertical slider
//	x2-=sbwidth;
}

inline void			tabbar_printnames_init(){g_buf[0]='*';}//don't use g_buf while printing tab names
int					tabbar_printname(int x, int y, int tab_idx, bool test=false, bool qualified=false, const char **pbuf=nullptr, int *len=nullptr)
{
	auto &f=openfiles[tab_idx];
	int not_modified=f.m_histpos==f.m_histpos_saved;
	int printed=0;
	if(f.m_filename.size())
	{
		int k2=0;
		if(!qualified)
		{
			k2=f.m_filename.size()-1;
			for(;k2>0&&f.m_filename[k2-1]!='/'&&f.m_filename[k2-1]!='\\';--k2);
		}
		printed=sprintf_s(g_buf+1, g_buf_size-1, "%s", f.m_filename.c_str()+k2);
	}
	else
		printed=sprintf_s(g_buf+1, g_buf_size-1, "Untitled %d", f.m_untitled_idx+1);
	int msg_width=0;
	if(test)
	{
		msg_width=calc_width(x, y, g_buf+not_modified, printed+!not_modified, 0, 1);
		if(pbuf)
			*pbuf=g_buf+not_modified;
		if(len)
			*len=printed+!not_modified;
	}
	else
	{
		if(tab_idx==current_file)
			set_text_colors(colors_selection);
		msg_width=print_line(x, y, g_buf+not_modified, printed+!not_modified, 0, 1);
		if(tab_idx==current_file)
			set_text_colors(colors_text);
	}
	return msg_width;
}
void				tabbar_calc_positions()
{
	tabbar_tabs.resize(openfiles.size());
	tabbar_printnames_init();
	for(int k=0, x=0;k<(int)openfiles.size();++k)
	{
		auto of=openfiles[k];
		int x0=x;
		x+=10+tabbar_printname(x, 0, k, true);
		tabbar_tabs[k].set(x, k);
	}
}
void				bring_object_to_view(int &winpos, int winsize, int objstart, int objend)
{
	if(winsize>objend-objstart)
	{
		if(winpos+winsize<objend)
			winpos=objend-winsize;
		if(winpos>objstart)
			winpos=objstart;
	}
	else
		winpos=objstart;
}
void				tabbar_scroll_to(int tab_idx)
{
	if(tab_idx<0||tab_idx>=(int)tabbar_tabs.size())
		return;
	switch(tabbar_position)
	{
	case TABBAR_TOP:
	case TABBAR_BOTTOM:
		if(tabbar_tabs.back().right>w)
			bring_object_to_view(tabbar_wpx, w, tab_idx?tabbar_tabs[tab_idx-1].right:0, tabbar_tabs[tab_idx].right);
		break;
	case TABBAR_LEFT:
	case TABBAR_RIGHT:
		if((int)openfiles.size()*dy>h)
		{
			int top=tab_idx*dy;
			bring_object_to_view(tabbar_wpy, h, top, top+dy);
		}
		break;
	}
}
void				general_selection_erase()
{
	if(cur->selection_exists())
	{
		if(cur->rectsel)
		{
			text_erase_rect(*text, cur->cursor.line, cur->selcur.line, cur->cursor.col, cur->selcur.col);
			int colstart=cur->cursor.col;
			if(colstart>cur->selcur.col)
				colstart=cur->selcur.col;
			cur->selcur.col=cur->cursor.col=colstart;
			cur->cursor.update_idx(*text);
			cur->selcur.update_idx(*text);
		}
		else
		{
			text_erase(*text, cur->cursor.line, cur->cursor.idx, cur->selcur.line, cur->selcur.idx);
			if(cur->cursor<cur->selcur)
				cur->selcur=cur->cursor;
			else
				cur->cursor=cur->selcur;
		}
	}
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
	auto rgb=(int*)stbi_load((exe_dir+"font.PNG").c_str(), &iw, &ih, &bytespp, 4);
	if(!rgb)
	{
		messagebox("Error", "Font texture not found.\nPlace a \'font.PNG\' file with the program.\n");
		return false;
	}
	dx=rgb[0]&0xFF, dy=rgb[1]&0xFF;
	//dx=iw/8, dy=ih/16;
	if(!dx||!dy)
	{
		messagebox("Error", "Invalid font texture character dimensions: dx=%d, dy=%d", dx, dy);
		return false;
	}
	for(int k=0, size=iw*ih;k<size;++k)
		if(rgb[k]&0x00FFFFFF)
			rgb[k]=0xFFFFFFFF;
	for(int c=32;c<127;++c)
	{
		auto &rect=font_coords[c-32];
		int px=(iw>>3)*(c&7), py=(ih>>4)*(c>>3);
		rect.x1=float(px)/iw, rect.x2=float(px+dx)/iw;
		rect.y1=float(py)/ih, rect.y2=float(py+dy)/ih;
	}
	glGenTextures(1, &font_txid);
	send_texture_pot(font_txid, rgb, iw, ih);
	stbi_image_free(rgb);
	prof_add("Load font");

	//set_text_colors(0x80FF00FF8000FF00);
	//set_text_colors(0xFFFFFFFFFF000000);
	set_text_colors(colors_text);//dark mode

	openfiles.push_back(TextFile());
	current_file=0;
	tabbar_calc_positions();
	tabs_switchto(current_file);
	return true;
}
void				wnd_on_resize(){}
void				print_text(int tab_origin, int x0, int x, int y, Text const &text, Bookmark const &i, Bookmark const &f, short zoom, int *final_x=nullptr, int *final_y=nullptr)
{
	if(i>=f)
		return;
	float ndc[4]={};//x1, x2, y1, y2
	QuadCoords *txc=nullptr;
	int width, idx, printable_count=0, tab_width=tab_count*dx*zoom, dxpx=dx*zoom, dypx=dy*zoom;
	int rx1=0, ry1=0, rdx=0, rdy=0;
	get_current_region(rx1, ry1, rdx, rdy);
	float
		CX1=2.f/rdx, CX0=-CX1*rx1-1,
		CY1=-2.f/rdy, CY0=1-CY1*ry1;
	float currentX=(float)x0, currentY=(float)y, nextY=(float)(y+dypx);
	ndc[2]=CY1*currentY+CY0;
	ndc[3]=CY1*nextY+CY0;
	//ndc[2]=1-(y-ry1)*2.f/rdy;
	//ndc[3]=1-(y+height-ry1)*2.f/rdy;
	if(currentY>=ry1+rdy)//first line is below region bottom
		return;
	Bookmark k=i;
	if(nextY<ry1)//first line is above region top
	{
		++k.line, k.idx=0;
		for(;k.line<(int)text.size();++k.line)
		{
			currentY=nextY, nextY+=dypx;
			if(nextY>=ry1)
			{
				currentX=(float)x;
				ndc[2]=CY1*currentY+CY0;
				ndc[3]=CY1*nextY+CY0;
				break;
			}
		}
		if(k.line>=(int)text.size())//nothing left to print
			return;
	}
	vrtx.resize((w+dxpx)*(h+dypx)/(dxpx*dypx)<<4);//nchars in grid	*	{vx, vy, txx, txy		x4 vertices/char}	~= 5MB at FHD screen
	for(;k<f;k.increment_idx(text))
	{
		char c=k.dereference_idx(text);
		//char c=msg[k];
		if(c>=32&&c<0xFF)
			width=dxpx;
		else if(c=='\t')
			width=tab_width-mod((int)currentX-tab_origin, tab_width), c=' ';
		else
		{
			if(c=='\n')
			{
				currentX=(float)x, currentY=nextY, nextY+=dypx;
				if(currentY>=ry1+rdy)//following lines are below region bottom
					break;
				ndc[2]=CY1*currentY+CY0;
				ndc[3]=CY1*nextY+CY0;
			}
			width=0;
		}
		if(width)
		{
			if(currentX+width>=0&&currentX<w)
			{
				ndc[0]=CX1*currentX+CX0;//xn1
				currentX+=width;
				ndc[1]=CX1*currentX+CX0;//xn2
				idx=printable_count<<4;
				if(idx>=(int)vrtx.size())
					vrtx.resize(vrtx.size()+(vrtx.size()>>1));//grow by x1.5
				txc=font_coords+c-32;
				vrtx[idx   ]=ndc[0], vrtx[idx+ 1]=ndc[2],		vrtx[idx+ 2]=txc->x1, vrtx[idx+ 3]=txc->y1;//top left
				vrtx[idx+ 4]=ndc[0], vrtx[idx+ 5]=ndc[3],		vrtx[idx+ 6]=txc->x1, vrtx[idx+ 7]=txc->y2;//bottom left
				vrtx[idx+ 8]=ndc[1], vrtx[idx+ 9]=ndc[3],		vrtx[idx+10]=txc->x2, vrtx[idx+11]=txc->y2;//bottom right
				vrtx[idx+12]=ndc[1], vrtx[idx+13]=ndc[2],		vrtx[idx+14]=txc->x2, vrtx[idx+15]=txc->y1;//top right

				++printable_count;
			}
			else
				currentX+=width;
		}
	}
	if(printable_count)
	{
		setGLProgram(shader_text.program);
		select_texture(font_txid, ns_text::u_atlas);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);									GL_CHECK();
		glBufferData(GL_ARRAY_BUFFER, printable_count<<6, vrtx.data(), GL_STATIC_DRAW);	GL_CHECK();//set vertices & texcoords
		glVertexAttribPointer(ns_text::a_coords, 4, GL_FLOAT, GL_TRUE, 0, 0);			GL_CHECK();

		glEnableVertexAttribArray(ns_text::a_coords);	GL_CHECK();
		glDrawArrays(GL_QUADS, 0, printable_count<<2);	GL_CHECK();//draw the quads: 4 vertices per character quad
		glDisableVertexAttribArray(ns_text::a_coords);	GL_CHECK();
	}
	if(final_x)
		*final_x=(int)currentX;
	if(final_y)
		*final_y=(int)currentY;
}
void				print_text(int tab_origin, int x0, int x, int y, const char *msg, int msg_length, short zoom, int *final_x=nullptr, int *final_y=nullptr)
{
	if(msg_length<1)
		return;
	float ndc[4]={};//x1, x2, y1, y2
	QuadCoords *txc=nullptr;
	int width, idx, printable_count=0, tab_width=tab_count*dx*zoom, dxpx=dx*zoom, dypx=dy*zoom;
	int rx1=0, ry1=0, rdx=0, rdy=0;
	get_current_region(rx1, ry1, rdx, rdy);
	float
		CX1=2.f/rdx, CX0=-CX1*rx1-1,
		CY1=-2.f/rdy, CY0=1-CY1*ry1;
	float currentX=(float)x0, currentY=(float)y, nextY=(float)(y+dypx);
	ndc[2]=CY1*currentY+CY0;
	ndc[3]=CY1*nextY+CY0;
	//ndc[2]=1-(y-ry1)*2.f/rdy;
	//ndc[3]=1-(y+height-ry1)*2.f/rdy;
	if(currentY>=ry1+rdy)//first line is below region bottom
		return;
	int k=0;
	if(nextY<ry1)//first line is above region top
	{
		for(;k<msg_length;++k)
		{
			if(msg[k]=='\n')
			{
				currentY=nextY, nextY+=dypx;
				if(nextY>=ry1)
				{
					currentX=(float)x;
					ndc[2]=CY1*currentY+CY0;
					ndc[3]=CY1*nextY+CY0;
					++k;
					break;
				}
			}
		}
		if(k>=msg_length)//nothing left to print
			return;
	}
	vrtx.resize((w+dxpx)*(h+dypx)/(dxpx*dypx)<<4);//nchars in grid	*	{vx, vy, txx, txy		x4 vertices/char}	~= 5MB at FHD screen
	for(;k<msg_length;++k)
	{
		char c=msg[k];
		if(c>=32&&c<0xFF)
			width=dxpx;
		else if(c=='\t')
			width=tab_width-mod((int)currentX-tab_origin, tab_width), c=' ';
		else
		{
			if(c=='\n')
			{
				currentX=(float)x, currentY=nextY, nextY+=dypx;
				if(currentY>=ry1+rdy)//following lines are below region bottom
					break;
				ndc[2]=CY1*currentY+CY0;
				ndc[3]=CY1*nextY+CY0;
			}
			width=0;
		}
		if(width)
		{
			if(currentX+width>=0&&currentX<w)
			{
				ndc[0]=CX1*currentX+CX0;//xn1
				currentX+=width;
				ndc[1]=CX1*currentX+CX0;//xn2
				idx=printable_count<<4;
				if(idx>=(int)vrtx.size())
					vrtx.resize(vrtx.size()+(vrtx.size()>>1));//grow by x1.5
				txc=font_coords+c-32;
				vrtx[idx   ]=ndc[0], vrtx[idx+ 1]=ndc[2],		vrtx[idx+ 2]=txc->x1, vrtx[idx+ 3]=txc->y1;//top left
				vrtx[idx+ 4]=ndc[0], vrtx[idx+ 5]=ndc[3],		vrtx[idx+ 6]=txc->x1, vrtx[idx+ 7]=txc->y2;//bottom left
				vrtx[idx+ 8]=ndc[1], vrtx[idx+ 9]=ndc[3],		vrtx[idx+10]=txc->x2, vrtx[idx+11]=txc->y2;//bottom right
				vrtx[idx+12]=ndc[1], vrtx[idx+13]=ndc[2],		vrtx[idx+14]=txc->x2, vrtx[idx+15]=txc->y1;//top right

				++printable_count;
			}
			else
				currentX+=width;
		}
	}
	if(printable_count)
	{
		setGLProgram(shader_text.program);
		select_texture(font_txid, ns_text::u_atlas);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);									GL_CHECK();
		glBufferData(GL_ARRAY_BUFFER, printable_count<<6, vrtx.data(), GL_STATIC_DRAW);	GL_CHECK();//set vertices & texcoords
		glVertexAttribPointer(ns_text::a_coords, 4, GL_FLOAT, GL_TRUE, 0, 0);			GL_CHECK();

		glEnableVertexAttribArray(ns_text::a_coords);	GL_CHECK();
		glDrawArrays(GL_QUADS, 0, printable_count<<2);	GL_CHECK();//draw the quads: 4 vertices per character quad
		glDisableVertexAttribArray(ns_text::a_coords);	GL_CHECK();
	}
	if(final_x)
		*final_x=(int)currentX;
	if(final_y)
		*final_y=(int)currentY;
}
void				tabbar_draw_horizontal(int ty1, int ty2, int wy1, int wy2)
{
	if(h>ty2-ty1)
	{
		if(tabbar_wpx>tabbar_tabs.back().right-(w>>1))
			tabbar_wpx=tabbar_tabs.back().right-(w>>1);
		if(tabbar_wpx<0)
			tabbar_wpx=0;
		window_tabbar.set(0, w, ty1, ty2);
		set_region_immediate(0, w, ty1, ty2);
		tabbar_printnames_init();
		for(int k=0, x=-tabbar_wpx;k<(int)openfiles.size();++k)
		{
			auto of=openfiles[k];
			int x0=x;
			x+=10+tabbar_printname(x, ty1, k);
			//if(k==current_file)
			//{
			//	draw_rectangle_i(x-10, x, ty1, ty1+dy, colors_selection.hi);
			//	draw_rectangle_i(x0, x, ty1+dy, ty1+(dy<<1), colors_selection.hi);
			//}
			draw_rectangle_i(x-10, x-1, ty1, ty1+dy, k==current_file?colors_selection.hi:colors_text.hi);
			draw_rectangle_i(x0, x-1, ty1+dy, ty1+(dy<<1), k==current_file?colors_selection.hi:colors_text.hi);
		}
		window_editor.set(0, w, wy1, wy2);
		set_region_immediate(0, w, wy1, wy2);
	}
}
void				tabbar_draw_sidebar(int tx1, int tx2, int wx1, int wx2)
{
	if(w>tx2-tx1)
	{
		if(tabbar_wpy>(int)openfiles.size()*dy-(h>>1))
			tabbar_wpy=openfiles.size()*dy-(h>>1);
		if(tabbar_wpy<0)
			tabbar_wpy=0;
		window_tabbar.set(tx1, tx2, 0, h);
		set_region_immediate(tx1, tx2, 0, h);
		tabbar_printnames_init();
		for(int k=0;k<(int)openfiles.size();++k)
		{
			auto of=openfiles[k];
			tabbar_printname(tx1, k*dy-tabbar_wpy, k);
		}
		window_editor.set(wx1, wx2, 0, h);
		set_region_immediate(wx1, wx2, 0, h);
	}
}
int					tab_drag_get_h_idx()//duplicate!, see tabbar_get_horizontal_idx
{
	int k=0;
	for(int left=0, right=0;k<(int)tabbar_tabs.size();++k)
	{
		left=right, right=tabbar_tabs[k].right;
		if(tabbar_wpx+mx<((left+right)>>1))
			break;
	}
	return k;
}
int					tab_drag_get_v_idx()
{
	return clamp(0, (tabbar_wpy+my+(dy>>1))/dy, tabbar_tabs.size());
}
void				wnd_on_render()
{
	prof_add("Render entry");
	glClear(GL_COLOR_BUFFER_BIT);

	switch(tabbar_position)
	{
	case TABBAR_TOP:	tabbar_draw_horizontal	(0,				dy<<1,		dy<<1,		h);				break;
	case TABBAR_BOTTOM:	tabbar_draw_horizontal	(h-(dy<<1),		h,			0,			h-(dy<<1));		break;
	case TABBAR_LEFT:	tabbar_draw_sidebar		(0,				tabbar_dx,	tabbar_dx,	w);				break;
	case TABBAR_RIGHT:	tabbar_draw_sidebar		(w-tabbar_dx,	w,			0,			w-tabbar_dx);	break;
	}
	prof_add("tab bar");
	
	int x1=0, x2=0, y1=0, y2=0;
	get_current_region_abs(x1, x2, y1, y2);

	//calculate text dimensions & cursor coordinates
	if(drag==DRAG_MOVE_SELECTION||drag==DRAG_COPY_SELECTION)
		std::swap(*cur, drag_cursor);
	if(!dimensions_known)
	{
		dimensions_known=true;
		calc_cols(*text);
	}
	int dxpx=dx*font_zoom, dypx=dy*font_zoom;//non-tab character dimensions in pixels
	int iw=text_width*dxpx, ih=text->size()*dypx;
	
	//decide if need scrollbars
	hscroll.decide(iw+scrollbarwidth>w);
	vscroll.decide(ih+scrollbarwidth>h);

	int cpx=x1+cur->cursor.col*dxpx, cpy=y1+cur->cursor.line*dypx, cpy2=y1+cur->selcur.line*dypx;//cursor position in pixels
	if(wnd_to_cursor)
	{
		wnd_to_cursor=false;
		int xpad=dxpx<<1&-(w>(dxpx<<2)), ypad=dypx<<1&-(h>(dypx<<2));
		if(wpx>cpx-xpad)
			wpx=cpx-xpad;
		if(wpx<cpx-w+xpad+vscroll.dwidth)
			wpx=cpx-w+xpad+vscroll.dwidth;
		if(wpy>cpy-ypad)
			wpy=cpy-ypad;
		if(wpy<cpy-h+ypad+hscroll.dwidth)
			wpy=cpy-h+ypad+hscroll.dwidth;
	}
	if(hscroll.dwidth)
	{
		if(wpx<0)
			wpx=0;
		if(wpx>iw-dxpx)
			wpx=iw-dxpx;
	}
	else
		wpx=0;
	if(vscroll.dwidth)
	{
		if(wpy<0)
			wpy=0;
		if(wpy>ih-dypx)
			wpy=ih-dypx;
	}
	else
		wpy=0;
	//int xend=0+iw-wpx, yend=0+ih-wpy;
	//hscroll.decide_orwith(!hscroll.dwidth&&vscroll.dwidth&&xend>=w-scrollbarwidth);
	//vscroll.decide_orwith(!vscroll.dwidth&&vscroll.dwidth&&yend>=h-74-scrollbarwidth);

	if(cur->rectsel)//rectangular selection
	{
		int rx1, rx2, ry1, ry2;//rect coordinates in characters
		cur->get_rectsel(ry1, ry2, rx1, rx2);
		if(ry1==ry2)
			draw_rectangle_i(x1, x2, cpy-wpy, cpy+dypx-wpy, color_cursorlinebk);//highlight cursor line
		for(int kl=0, x=x1, y=y1;kl<(int)text->size();++kl, y+=dypx)
		{
			auto &line=text->operator[](kl);
			if(kl>=ry1&&kl<=ry2)
			{
				int idx=0;
				x+=print_line(x-wpx, y-wpy, line.c_str(), line.size(), x1-wpx, font_zoom, rx1, &idx);
				set_text_colors(colors_selection);
				x+=print_line(x-wpx, y-wpy, line.c_str(), line.size(), x1-wpx, font_zoom, rx2, &idx);
				set_text_colors(colors_text);
				print_line(x-wpx, y-wpy, line.c_str()+idx, line.size()-idx, x1-wpx, font_zoom);
				x=x1;
			}
			else
				print_line(x-wpx, y-wpy, line.c_str(), line.size(), x1-wpx, font_zoom);
		}
		if(cpy<cpy2)//draw cursor
			draw_line_i(cpx-wpx, cpy-wpy, cpx-wpx, cpy2-wpy+dypx, 0xFFFFFFFF);
		else
			draw_line_i(cpx-wpx, cpy2-wpy, cpx-wpx, cpy-wpy+dypx, 0xFFFFFFFF);
	}
	else
	{
		draw_rectangle_i(x1, x2, cpy-wpy, cpy+dypx-wpy, color_cursorlinebk);//highlight cursor line
		Bookmark i(0, 0, 0), f(text->size()-1, text->operator[](text->size()-1).size(), 0);
		if(cur->selection_exists())//normal selection
		{
			Bookmark sel_i, sel_f;
			cur->get_selection(sel_i, sel_f);
		
			int x=x1-wpx, y=y1-wpy;
			print_text(x1-wpx, x, x1-wpx, y, *text, i, sel_i, font_zoom, &x, &y);//BROKEN
			set_text_colors(colors_selection);
			print_text(x1-wpx, x, x1-wpx, y, *text, sel_i, sel_f, font_zoom, &x, &y);
			set_text_colors(colors_text);
			print_text(x1-wpx, x, x1-wpx, y, *text, sel_f, f, font_zoom, &x, &y);
		}
		else//no selection
			print_text(x1-wpx, x1-wpx, x1-wpx, y1-wpy, *text, i, f, font_zoom);
		draw_line_i(cpx-wpx, cpy-wpy, cpx-wpx, cpy-wpy+dypx, 0xFFFFFFFF);//draw cursor
	}
	prof_add("text");

	//draw possible scrollbars
	if(hscroll.dwidth&&vscroll.dwidth)
		draw_rectangle_i(x2-scrollbarwidth, x2, y2-scrollbarwidth, y2, color_barbk);//corner square
	if(hscroll.dwidth)
	{
		int vsbw=vscroll.dwidth;
		draw_rectangle_i(x1+scrollbarwidth, x2-scrollbarwidth-vsbw, y2-scrollbarwidth, y2, color_barbk);//horizontal
		draw_rectangle_i(x1, x1+scrollbarwidth, y2-scrollbarwidth, y2, color_button);//left
		draw_rectangle_i(x2-scrollbarwidth-vsbw, x2-vsbw, y2-scrollbarwidth, y2, color_button);//right
		if(drag==DRAG_HSCROLL)
			scrollbar_scroll(wpx, iw, x2-x1-vsbw, x2-x1-(scrollbarwidth<<1)-vsbw, hscroll.s0, hscroll.m_start, mx, font_zoom, hscroll.start, hscroll.size);
		else
			scrollbar_slider(wpx, iw, x2-x1-vsbw, x2-x1-(scrollbarwidth<<1)-vsbw, font_zoom, hscroll.start, hscroll.size);
		draw_rectangle_i(x1+scrollbarwidth+hscroll.start, x1+scrollbarwidth+hscroll.start+hscroll.size, y2-scrollbarwidth, y2, color_slider);//horizontal slider
		y2-=scrollbarwidth;
	}
	if(vscroll.dwidth)
	{
		draw_vscrollbar(x2-scrollbarwidth, scrollbarwidth, y1, y2, wpy, ih, vscroll.s0, vscroll.m_start, font_zoom, vscroll.start, vscroll.size, DRAG_VSCROLL);
		x2-=scrollbarwidth;
	}
	prof_add("scrollbars");

	set_region_immediate(0, w, 0, h);
#ifdef DEBUG_CURSOR
	debug_print_cols(x1-wpx, y1-(dypx<<1), font_zoom, w/dxpx, 10);//
	debug_print_cols(x1-wpx, y1-dypx, font_zoom, w/dxpx, 1);//
#endif
	switch(drag)
	{
	case DRAG_TAB:
		{
			int marker_size=10;
			int xtip=0, ytip=0;
			const char *buf=nullptr;
			int len=0;
			int msg_width=tabbar_printname(0, 0, drag_tab_idx, true, true, &buf, &len);
			switch(tabbar_position)
			{
			case TABBAR_TOP:
			case TABBAR_BOTTOM:
				{
					int k=tab_drag_get_h_idx();
					int xmarker=k?tabbar_tabs[k-1].right:0;
					int my1, my2;
					if(tabbar_position==TABBAR_TOP)
						my1=(dy<<1)+1, my2=my1+marker_size, xtip=mx, ytip=my+20;
					else
						my1=h-(dy<<1)-1, my2=my1-marker_size, xtip=mx, ytip=my-dy-20;
					draw_line_i(xmarker-tabbar_wpx, my1, xmarker-tabbar_wpx-marker_size, my2, 0xFFFFFFFF);
					draw_line_i(xmarker-tabbar_wpx, my1, xmarker-tabbar_wpx+marker_size, my2, 0xFFFFFFFF);
				}
				break;
			case TABBAR_LEFT:
			case TABBAR_RIGHT:
				{
					int ymarker=tab_drag_get_v_idx()*dy;
					int mx1, mx2;
					if(tabbar_position==TABBAR_LEFT)
						mx1=tabbar_dx+1, mx2=mx1+marker_size, xtip=mx+20, ytip=my;
					else
						mx1=w-tabbar_dx-1, mx2=mx1-marker_size, xtip=mx-msg_width-20, ytip=my;
					draw_line_i(mx1, ymarker-tabbar_wpy, mx2, ymarker-tabbar_wpy-marker_size, 0xFFFFFFFF);
					draw_line_i(mx1, ymarker-tabbar_wpy, mx2, ymarker-tabbar_wpy+marker_size, 0xFFFFFFFF);
				}
				break;
			}
			draw_rectangle_i(xtip, xtip+msg_width+4, ytip, ytip+dy+4, 0x40FFFFFF);
			set_text_colors(colors_selection);
			print_line(xtip+2, ytip+2, buf, len, 0, -1, 1);
			set_text_colors(colors_text);
		}
		break;
	case DRAG_MOVE_SELECTION://draw drag target cursor
	case DRAG_COPY_SELECTION:
		{
			std::swap(*cur, drag_cursor);
			if(drag_cursor.rectsel)
			{
				auto r=drag_cursor.get_rectsel();
#ifdef DRAG_RECTSEL_X_AT_MOUSE
				relocate_range(cur->cursor.col, r.x1, r.x2);
#else
				int dcol=cur->cursor.col-cur->selcur.col;
				r.x1+=dcol, r.x2+=dcol;
#endif
				int dline=cur->cursor.line-cur->selcur.line;
				r.y1+=dline, r.y2+=dline+1;
				if(r.x1<0)
					r.x2-=r.x1, r.x1=0;
				if(r.y1<0)
					r.y2-=r.y1, r.y1=0;
				int Lx=x1+r.x1*dxpx-wpx, Ty=y1+r.y1*dypx-wpy,
					Rx=x1+r.x2*dxpx-wpx, By=y1+r.y2*dypx-wpy;
				draw_rectangle_hollow(Lx, Rx, Ty, By, 0xFF808080);
				draw_line_i(Lx, Ty, Lx, By, 0xFFFFFFFF);
			}
			else
			{
				int dstpx=x1+cur->cursor.col*dxpx-wpx,
					dstpy=y1+cur->cursor.line*dypx-wpy;
				draw_line_i(dstpx, dstpy, dstpx, dstpy+dypx, 0xFFFFFFFF);
			}
		}
		break;
	}

	//print(1, 0, 0, 0, "Hello. Sample Text. What\'s going on???");
	//for(int k=0;k<1000;++k)
	//{
	//	int zoom=1+rand()%3;
	//	print(zoom, 0, rand()%w, rand()%h, "Sample Text %d", zoom);
	//}
	//prof_add("Benchmark");

	//memset(rgb, 0, rgbn*sizeof(int));
	//font_use();
	//int ret=GUITPrint(0, 0, "Teletext");
	//GUITPrint(10, 100, "A");
	//font_drop();

	swap_buffers();
	prof_add("swap");
	prof_print();
	report_errors();
}

int					ask_to_save(int tab_idx)
{
	auto &f=openfiles[tab_idx];
	if(f.m_filename.size())
		return messagebox_yesnocancel("Teletext", "Save changes to \'%s\'?", f.m_filename.c_str());
	return messagebox_yesnocancel("Teletext", "Save changes to \'Untitled %d\'?", f.m_untitled_idx+1);
}
bool				close_tab(int tab_idx)//returns true if tab got closed
{
	if(tabs_ismodified(tab_idx))
	{
		switch(ask_to_save(tab_idx))
		{
		case 0://yes
			{
				auto filename=save_file_dialog();
				std::string str;
				if(filename&&save_text_file(filename, str))
				{
					str2text(str.c_str(), str.size(), openfiles[tab_idx].m_text);
					openfiles[tab_idx].m_filename=filename;
				}
			}
			break;
		case 1://no
			break;
		case 2://cancel
			return false;
		}
	}
	if(filename->size())
		closedfiles.push_back(ClosedFile(tab_idx, std::move(openfiles[tab_idx].m_filename)));
	if(openfiles.size()>1)
		openfiles.erase(openfiles.begin()+tab_idx);
	else
		openfiles.front().clear();
	tabbar_calc_positions();
	current_file-=tab_idx<current_file;
	return true;
}
bool				wnd_on_mousemove()
{
	switch(drag)
	{
	case DRAG_SELECT:
	case DRAG_RECT:
		if(cur->rectsel)
			drag=DRAG_RECT;
		else
			drag=DRAG_SELECT;
		cursor_at_mouse(*text, cur->cursor);
#ifdef DEBUG_CURSOR
		{
			const char *selectiontype=nullptr;
			if(cur->selection_exists())
			{
				if(cur->rectsel)
					selectiontype="RECT SEL.";
				else
					selectiontype="NORMAL SEL.";
			}
			else
				selectiontype="NO SEL.";
			debug_set_window_title("%s cur L %d I %d C %d  sel L %d I %d C %d", selectiontype, cur->cursor.line, cur->cursor.idx, cur->cursor.col, cur->selcur.line, cur->selcur.idx, cur->selcur.col);
		}
#endif
		return true;
	case DRAG_VSCROLL:
	case DRAG_HSCROLL:
	case DRAG_TAB:
		return true;
	case DRAG_MOVE_SELECTION:
	case DRAG_COPY_SELECTION:
		cursor_at_mouse(*text, cur->cursor);
		return true;
	}
	return false;
}
bool				wnd_on_mousewheel(bool mw_forward)
{
	if(is_ctrl_down())
	{
		if(mw_forward)
		{
			if(font_zoom<32)
				font_zoom<<=1, wpx<<=1, wpy<<=1;
		}
		else if(font_zoom>1)
			font_zoom>>=1, wpx>>=1, wpy>>=1;
	}
	else
	{
		switch(tabbar_position)
		{
		case TABBAR_TOP:
			if(h>(dy<<1)&&(my<(dy<<1)||drag==DRAG_TAB)&&tabbar_tabs.back().right>w)
			{
				//int k=tabbar_get_horizontal_idx(1);//npp: skip one tab
				if(mw_forward)//predictable
					tabbar_wpx-=dx<<3;
				else
					tabbar_wpx+=dx<<3;
				return true;
			}
			break;
		case TABBAR_BOTTOM:
			if(h>(dy<<1)&&(my>h-(dy<<1)||drag==DRAG_TAB)&&tabbar_tabs.back().right>w)
			{
				if(mw_forward)
					tabbar_wpx-=dx<<3;
				else
					tabbar_wpx+=dx<<3;
			}
			break;
		case TABBAR_LEFT:
			if(w>tabbar_dx&&(mx<tabbar_dx||drag==DRAG_TAB)&&(int)openfiles.size()*dy>h)
			{
				if(mw_forward)
					tabbar_wpy-=dy*3;
				else
					tabbar_wpy+=dy*3;
			}
			break;
		case TABBAR_RIGHT:
			if(w>tabbar_dx&&(mx>w-tabbar_dx||drag==DRAG_TAB)&&(int)openfiles.size()*dy>h)
			{
				if(mw_forward)
					tabbar_wpy-=dy*3;
				else
					tabbar_wpy+=dy*3;
			}
			break;
		}
		if(mw_forward)
			wpy-=dy*font_zoom*3;
		else
			wpy+=dy*font_zoom*3;
	}
	return true;
}
bool				wnd_on_zoomin()
{
	if(font_zoom<32)
	{
		font_zoom<<=1, wpx<<=1, wpy<<=1;
		return true;
	}
	return false;
}
bool				wnd_on_zoomout()
{
	if(font_zoom>1)
	{
		font_zoom>>=1, wpx>>=1, wpy>>=1;
		return true;
	}
	return false;
}
bool				drag_selection_click(DragType drag_type)//checks if you clicked on the selection to drag it
{
	if(cur->selection_exists())
	{
		Bookmark click;
		bool hit=cursor_at_mouse(*text, click);
		if(hit)
		{
			if(cur->rectsel)
			{
				auto r=cur->get_rectsel();
				if(click.line>=r.y1&&click.line<=r.y2&&click.col>=r.x1&&click.col<r.x2)
				{
					drag=drag_type;
					drag_cursor=*cur;
					cur->cursor=cur->selcur=click;
					return true;
				}
			}
			else//normal selection
			{
				Bookmark i, f;
				cur->get_selection(i, f);
				if(click>=i&&click<f)
				{
					drag=drag_type;
					drag_cursor=*cur;
					cur->cursor=cur->selcur=click;
					return true;
				}
			}
		}
	}
	return false;
}
void				lbutton_down_text(bool doubleclick)
{
	bool ctrldown=is_ctrl_down();
	if(drag_selection_click(ctrldown?DRAG_COPY_SELECTION:DRAG_MOVE_SELECTION))
		return;
	if(doubleclick||ctrldown)//select word
	{
		cursor_at_mouse(*text, cur->cursor);
		cur->selcur=cur->cursor;
		char initial=group_char(cur->cursor.dereference_idx(*text));
		Bookmark start(0, 0, 0), finish(text->size()-1, text->operator[](text->size()-1).size(), 0);

		while(cur->selcur>start)
		{
			cur->selcur.decrement_idx(*text);
			if(group_char(cur->selcur.dereference_idx(*text))!=initial)
				break;
		}
		cur->selcur.increment_idx(*text);

		cur->cursor.increment_idx(*text);
		while(cur->cursor<finish)
		{
			if(group_char(cur->cursor.dereference_idx(*text))!=initial)
				break;
			cur->cursor.increment_idx(*text);
		}

		cur->cursor.update_col(*text);
		cur->selcur.update_col(*text);
	}
	else
	{
		if(cur->rectsel=is_alt_down())//rectsel
		{
			Bookmark temp;
			if(cursor_at_mouse(*text, temp))
				drag=DRAG_RECT, cur->cursor=temp;
			if(!is_shift_down())
				cur->selcur=cur->cursor;
		}
		else//place cursor / normal selection
		{
			drag=DRAG_SELECT;
			cursor_at_mouse(*text, cur->cursor);
			if(!is_shift_down())
				cur->selcur=cur->cursor, cur->selcur=cur->cursor;
		}
	}
}
int					tabbar_get_horizontal_idx(int mousex)
{
	int k=0;
	for(;k<(int)tabbar_tabs.size()&&tabbar_wpx+mousex>=tabbar_tabs[k].right;++k);
	if(k>=0&&k<(int)tabbar_tabs.size())
		return k;
	return -1;
}
inline void			drag_tab()
{
	if(openfiles.size()>1)
	{
		drag=DRAG_TAB, drag_tab_idx=current_file;
		mouse_capture();
	}
}
bool				wnd_on_lbuttondown(bool doubleclick)
{
	int x1=0, x2=w, y1=0, y2=h;
	switch(tabbar_position)
	{
	case TABBAR_TOP:
		if(h>(dy<<1))
		{
			if(my<(dy<<1))
			{
				int k=tabbar_get_horizontal_idx(mx);
				if(k!=-1)
				{
					tabs_switchto(tabbar_tabs[k].idx);
					tabbar_scroll_to(current_file);
					drag_tab();
					return true;
				}
				return false;
			}
			x1=0, x2=w, y1=dy<<1, y2=h;
		}
		break;
	case TABBAR_BOTTOM:
		if(h>(dy<<1))
		{
			if(my>h-(dy<<1))
			{
				int k=tabbar_get_horizontal_idx(mx);
				if(k!=-1)
				{
					tabs_switchto(tabbar_tabs[k].idx);
					tabbar_scroll_to(current_file);
					drag_tab();
					return true;
				}
				return false;
			}
			x1=0, x2=w, y1=0, y2=h-(dy<<1);
		}
		break;
	case TABBAR_LEFT:
		if(w>tabbar_dx)
		{
			if(mx<tabbar_dx)
			{
				int k=tab_drag_get_v_idx();
				if(k>(int)openfiles.size()-1)
					k=openfiles.size()-1;
				tabbar_scroll_to(current_file);
				drag_tab();
				return true;
			}
			x1=tabbar_dx, x2=w, y1=0, y2=h;
		}
		break;
	case TABBAR_RIGHT:
		if(w>tabbar_dx)
		{
			if(mx>w-tabbar_dx)
			{
				int k=tab_drag_get_v_idx();
				if(k>(int)openfiles.size()-1)
					k=openfiles.size()-1;
				tabbar_scroll_to(current_file);
				drag_tab();
				return true;
			}
			x1=0, x2=w-tabbar_dx, y1=0, y2=h;
		}
		break;
	}
	if(vscroll.dwidth)
	{
		if(hscroll.dwidth)	//both scrollbars present
		{
			if(mx<x2-scrollbarwidth)
			{
				if(my<y2-scrollbarwidth)
					lbutton_down_text(doubleclick);
				else
					drag=DRAG_HSCROLL, hscroll.click_on_slider(mx);
			}
			else
			{
				if(my<y2-scrollbarwidth)
					drag=DRAG_VSCROLL, vscroll.click_on_slider(my);
			}
		}
		else				//only vscroll present
		{
			if(mx<x2-scrollbarwidth)
				lbutton_down_text(doubleclick);
			else
				drag=DRAG_VSCROLL, vscroll.click_on_slider(my);
		}
	}
	else
	{
		if(hscroll.dwidth)	//only hscroll present
		{
			if(my<y2-scrollbarwidth)
				lbutton_down_text(doubleclick);
			else
				drag=DRAG_HSCROLL, hscroll.click_on_slider(mx);
		}
		else				//no scrollbars
			lbutton_down_text(doubleclick);
	}
	mouse_capture();
	return true;
}
bool				wnd_on_lbuttonup()
{
	switch(drag)
	{
	case DRAG_TAB:
		{
			int dst=-1;
			switch(tabbar_position)
			{
			case TABBAR_TOP:
			case TABBAR_BOTTOM:
				dst=tab_drag_get_h_idx();
				break;
			case TABBAR_LEFT:
			case TABBAR_RIGHT:
				dst=tab_drag_get_v_idx();
				break;
			}
			dst-=dst>drag_tab_idx;
			if(dst!=drag_tab_idx)
			{
				TextFile f=std::move(openfiles[drag_tab_idx]);
				openfiles.erase(openfiles.begin()+drag_tab_idx);
				openfiles.insert(openfiles.begin()+dst, std::move(f));

				tabbar_calc_positions();
				tabs_switchto(dst);
			}
		}
		break;
	case DRAG_MOVE_SELECTION:
	case DRAG_COPY_SELECTION:
		if(drag_cursor.selection_exists())
		{
			//drag_cursor is selection, cur->cursor is target
			Text selection;
			if(drag_cursor.rectsel)
			{
				rectsel_copy(*text, drag_cursor, selection);
				auto r=drag_cursor.get_rectsel();
				if(drag==DRAG_MOVE_SELECTION)
				{
					int before=idx2col(text[0][r.y1].c_str(), text[0][r.y1].size(), 0);
					text_erase_rect(*text, r.y1, r.y2, r.x1, r.x2);
					int after=idx2col(text[0][r.y1].c_str(), text[0][r.y1].size(), 0);
					if(cur->cursor.line>=r.y1&&cur->cursor.line<=r.y2)
					{
						if(cur->cursor.col>r.x2)
							cur->cursor.col+=after-before;
						else if(cur->cursor.col>r.x1)
							cur->cursor.col=r.x1;
					}
				}
				int dstline=r.y1+cur->cursor.line-cur->selcur.line,
#ifdef DRAG_RECTSEL_X_AT_MOUSE
					dstcol=cur->cursor.col;
#else
					dstcol=r.x1+cur->cursor.col-cur->selcur.col;
#endif
				if(dstline<0)
					dstline=0;
				if(dstcol<0)
					dstcol=0;
				rectsel_insert(selection, dstline, dstcol, *text);

				relocate_range(dstline, drag_cursor.cursor.line, drag_cursor.selcur.line);
				relocate_range(dstcol, drag_cursor.cursor.col, drag_cursor.selcur.col);
				drag_cursor.cursor.update_idx(*text);
				drag_cursor.selcur.update_idx(*text);
				cur->cursor=drag_cursor.cursor;
				cur->selcur=drag_cursor.selcur;
			}
			else if(!drag_cursor.does_contain(cur->cursor))
			{
				Bookmark i, f;
				drag_cursor.get_selection(i, f);
				int selsize=bookmark_subtract(f, i, *text);
				selection_copy(*text, drag_cursor, selection);
				if(drag==DRAG_MOVE_SELECTION)
				{
					bool relocate=cur->cursor>f;
					int absidx=0;
					if(relocate)
						absidx=cur->cursor.get_absidx(*text);
					text_erase(*text, drag_cursor.cursor.line, drag_cursor.cursor.idx, drag_cursor.selcur.line, drag_cursor.selcur.idx);
					if(relocate)
						cur->cursor.set_absidx(*text, absidx-selsize);
				}
				selection_insert(selection, cur->cursor.line, cur->cursor.idx, *text);

				cur->selcur=cur->cursor;
				if(drag_cursor.cursor<drag_cursor.selcur)
					bookmark_add(cur->selcur, selsize, *text);
				else//selcur<cursor
					bookmark_add(cur->cursor, selsize, *text);
				cur->selcur.update_col(*text);
				cur->cursor.update_col(*text);
			}
			else if(cur->cursor!=cur->selcur)
				*cur=drag_cursor;
		}
		break;
	}
	mouse_release();
	drag=DRAG_NONE;
	return true;
}
bool				wnd_on_mbuttondown(bool doubleclick)
{
	int k=0;
	switch(tabbar_position)
	{
	case TABBAR_TOP:
		if(h>(dy<<1)&&my<(dy<<1))
		{
			k=tabbar_get_horizontal_idx(mx);
			if(k!=-1)
			{
				close_tab(k);
				tabs_switchto(current_file);
				return true;
			}
		}
		break;
	case TABBAR_BOTTOM:
		if(h>(dy<<1)&&my>h-(dy<<1))
		{
			k=tabbar_get_horizontal_idx(mx);
			if(k!=-1)
			{
				close_tab(k);
				tabs_switchto(current_file);
				return true;
			}
		}
		break;
	case TABBAR_LEFT:
		if(w>tabbar_dx&&mx<tabbar_dx)
		{
			k=tab_drag_get_h_idx();
			if(k<(int)openfiles.size())
			{
				close_tab(k);
				tabs_switchto(current_file);
				return true;
			}
		}
		break;
	case TABBAR_RIGHT:
		if(w>tabbar_dx&&mx>w-tabbar_dx)
		{
			k=tab_drag_get_h_idx();
			if(k<(int)openfiles.size())
			{
				close_tab(k);
				tabs_switchto(current_file);
				return true;
			}
		}
		break;
	}
	return false;
}
bool				wnd_on_mbuttonup(){return false;}
bool				wnd_on_rbuttondown(bool doubleclick){return false;}
bool				wnd_on_rbuttonup(){return false;}

bool				wnd_on_display_help()
{
	display_help();
	return true;
}
bool				wnd_on_toggle_profiler()
{
	prof_toggle();
	return true;
}
bool				wnd_on_select_all()
{
	cur->selcur.setzero(), cur->cursor.setend(*text), cur->rectsel=false;
	wnd_to_cursor=true, hist_cont=false;
	return true;
}
bool				wnd_on_deselect()
{
	cur->selcur=cur->cursor;
	cur->rectsel=false;
	wnd_to_cursor=true;
	return true;
}
bool				wnd_on_save(bool save_as)
{
	if(save_as||!filename->size())
	{
		auto path=save_file_dialog();
		if(path)
		{
			std::string str;
			text2str(*text, str);
			if(save_text_file(path, str))
				*filename=path;
		}
	}
	else
	{
		std::string str;
		text2str(*text, str);
		save_text_file(filename->c_str(), str);
	}
	set_title();
	openfiles[current_file].m_histpos_saved=openfiles[current_file].m_histpos;
	return true;
}
bool				wnd_on_clear_hist()
{
	if(!messagebox_okcancel("Teletext", "Are you sure you want to clear the undo/redo history?"))
	{
		*histpos=-1;
		history->clear();
		set_title();
	}
	return true;
}
bool				wnd_on_undo()
{
	hist_undo();
	return true;
}
bool				wnd_on_redo()
{
	hist_redo();
	return true;
}
bool				wnd_on_copy(bool cut)
{
	if(cur->selection_exists())
	{
		std::string str;
		Text selection;
		if(cur->rectsel)
			rectsel_copy(*text, *cur, selection);
		else
			selection_copy(*text, *cur, selection);
		if(cut)
			general_selection_erase();
		text2str(selection, str);
		copy_to_clipboard_c(str.c_str(), str.size());
		return cut;
	}
	return false;
}
bool				wnd_on_paste()
{
	char *str=nullptr;
	int len=0;
	if(!paste_from_clipboard(str, len))
		return false;
	general_selection_erase();
	text_insert(*text, cur->cursor.line, cur->cursor.idx, str, len, &cur->cursor.line, &cur->cursor.idx);
	delete[] str;
	cur->cursor.update_col(*text);
	if(!is_shift_down())
		cur->deselect(*text, 0);
	return true;
}
bool				wnd_on_scroll_up_key()
{
	wpy-=dy*font_zoom;
	return true;
}
bool				wnd_on_scroll_down_key()
{
	wpy+=dy*font_zoom;
	return true;
}
bool				wnd_on_skip_word_left()
{
	Bookmark start(0, 0, 0);
	if(cur->cursor.decrement_idx(*text))
	{
		char initial=group_char(cur->cursor.dereference_idx(*text));
		while(cur->cursor>start)
		{
			if(group_char(cur->cursor.dereference_idx(*text))!=initial)
			{
				cur->cursor.increment_idx(*text);
				break;
			}
			cur->cursor.decrement_idx(*text);
		}
		cur->cursor.update_col(*text);
	}

	if(!is_shift_down())
		cur->deselect(*text, -1);
	wnd_to_cursor=true;
	return true;
}
bool				wnd_on_skip_word_right()
{
	Bookmark finish(text->size()-1, text->operator[](text->size()-1).size(), 0);
	char initial=group_char(cur->cursor.dereference_idx(*text));
	cur->cursor.increment_idx(*text);
	while(cur->cursor<finish)
	{
		if(group_char(cur->cursor.dereference_idx(*text))!=initial)
			break;
		cur->cursor.increment_idx(*text);
	}
	cur->cursor.update_col(*text);

	if(!is_shift_down())
		cur->deselect(*text, 1);
	wnd_to_cursor=true;
	return true;
}
bool				wnd_on_goto_file_start()
{
	cur->cursor.setzero();
	if(!is_shift_down())
		cur->deselect(*text, 0);
	wnd_to_cursor=true;
	return true;
}
bool				wnd_on_goto_file_end()
{
	cur->cursor.setend(*text);
	if(!is_shift_down())
		cur->deselect(*text, 0);
	wnd_to_cursor=true;
	return true;
}
bool				wnd_on_goto_line_start()
{
	cur->cursor.set_linestart();
	if(!is_shift_down())
		cur->deselect(*text, 0);
	wnd_to_cursor=true;
	return true;
}
bool				wnd_on_goto_line_end()
{
	cur->cursor.set_lineend(*text);
	if(!is_shift_down())
		cur->deselect(*text, 0);
	wnd_to_cursor=true;
	return true;
}
bool				wnd_on_begin_rectsel()
{
	if(drag==DRAG_SELECT)
	{
		drag=DRAG_RECT, cur->rectsel=true;
		return true;
	}
	return false;
}
bool				wnd_on_deletechar()
{
	if(cur->selection_exists())
		general_selection_erase();
	else
		text_erase1_del(*text, cur->cursor.line, cur->cursor.idx);
	return true;
}
bool				wnd_on_backspace()
{
	if(cur->selection_exists())
		general_selection_erase();
	else
	{
		int l0=cur->cursor.line, idx0=cur->cursor.idx;
		cur->cursor.decrement_idx(*text);
		cur->cursor.update_col(*text);
		cur->selcur=cur->cursor;
		text_erase1_bksp(*text, l0, idx0);
	}
	return true;
}
bool				wnd_on_cursor_up()
{
	bool shiftdown=is_shift_down();
	//if(!cur->selection_exists()||shiftdown)
		cur->cursor.jump_vertical(*text, -1);
	if(!shiftdown)
		cur->deselect(*text, -1);
	wnd_to_cursor=true;
	return true;
}
bool				wnd_on_cursor_down()
{
	bool shiftdown=is_shift_down();
	//if(!cur->selection_exists()||shiftdown)
		cur->cursor.jump_vertical(*text, 1);
	if(!shiftdown)
		cur->deselect(*text, 1);
	wnd_to_cursor=true;
	return true;
}
bool				wnd_on_cursor_left()
{
	bool shiftdown=is_shift_down();
	if(!cur->selection_exists()||shiftdown)
	{
		cur->cursor.decrement_idx(*text);
		cur->cursor.update_col(*text);
	}
	if(!shiftdown)
		cur->deselect(*text, -1);
	wnd_to_cursor=true;
	return true;
}
bool				wnd_on_cursor_right()
{
	bool shiftdown=is_shift_down();
	if(!cur->selection_exists()||shiftdown)
	{
		cur->cursor.increment_idx(*text);
		cur->cursor.update_col(*text);
	}
	if(!shiftdown)
		cur->deselect(*text, 1);
	wnd_to_cursor=true;
	return true;
}
bool				wnd_on_type(char character)
{
	if(cur->selection_exists())
	{
		bool multiline_selection=cur->cursor.line!=cur->selcur.line;
		if(character=='\t'&&multiline_selection)//indentation
		{
			if(cur->rectsel)
			{
				if(is_shift_down())//shift tab
					indent_rectsel_back(*cur);
				else//tab
					indent_rectsel_forward(*cur);
			}
			else
			{
				if(is_shift_down())//shift tab
					indent_selection_back(*cur);
				else//tab
					indent_selection_forward(*cur);
			}
			return true;
		}
		general_selection_erase();
	}
	if(cur->rectsel)
	{
		auto r=cur->get_rectsel();
		if(character=='\n')
		{
			cur->cursor.set_gui(*text, r.y2, r.x2);
			cur->selcur=cur->cursor;
		}
		else
		{
			//while typing a character with rectsel, if a tab causes misalign:
			//	either	1) insert (tab_count-mod(col, tab_count)) spaces before the character in lines without tab, so that all typed characters are aligned (probably not what was intended)
			//	or		2) relocate the cursor to tab (unpredictable)
			//	or		3) (best solution) break (replace) each encountered tab into spaces
			for(int kl=r.y1;kl<=r.y2;++kl)
			{
				auto &line=text->operator[](kl);
				int idx1=0, col1=0, idx2=0, col2=0;
				col2idx(line.c_str(), line.size(), 0, 0, 0, r.x1, &idx1, &col1);
				if(col1<r.x1)//line is not long enough
				{
					int ntabs=r.x1/tab_count-line.size()/tab_count, nspaces=mod(r.x1, tab_count);
					if(ntabs>0)
						line.append(ntabs, '\t');
					if(nspaces>0)
						line.append(nspaces, ' ');
					line.append(1, character);
				}
				else
				{
					col2idx(line.c_str(), line.size(), 0, 0, 0, r.x2, &idx2, &col2);
					line.replace(line.begin()+idx1, line.begin()+idx2, &character, &character+1);
				}
			}
			if(cur->cursor.col<cur->selcur.col)
			{
				++cur->cursor.idx;
				cur->cursor.update_col(*text);
				cur->selcur.col=cur->cursor.col;
				cur->selcur.update_idx(*text);
			}
			else
			{
				++cur->selcur.idx;
				cur->selcur.update_col(*text);
				cur->cursor.col=cur->selcur.col;
				cur->cursor.update_idx(*text);
			}
			//cur->selcur.idx=++cur->cursor.idx;
			//cur->cursor.update_col(*text);
			//cur->selcur.update_col(*text);
			return true;
		}
	}
	text_insert1(*text, cur->cursor.line, cur->cursor.idx, character);
	cur->cursor.increment_idx(*text);
	cur->cursor.update_col(*text);
	cur->selcur=cur->cursor;
	return true;
}

bool				wnd_on_newtab()
{
	if(is_shift_down())//reopen closed tab
	{
		if(closedfiles.size())
		{
			auto &file=closedfiles.back();
			std::string str;
			if(open_text_file(file.filename.c_str(), str))
			{
				TextFile f;
				str2text(str.c_str(), str.size(), f.m_text);
				hist_cont=false;
				dimensions_known=false;
				f.m_filename=file.filename;
				int idx=file.tab_idx;
				if(idx>(int)openfiles.size())//crash guard
					idx=openfiles.size();
				openfiles.insert(openfiles.begin()+idx, std::move(f));
				tabbar_calc_positions();
				tabs_switchto(idx);
				tabbar_scroll_to(current_file);
			}
			else
				messagebox("Information", "Cannot reopen \'%s\'.", file.filename.c_str());
			closedfiles.pop_back();
		}
	}
	else//new tab
	{
		openfiles.push_back(TextFile());
		openfiles.back().m_untitled_idx=gen_untitled_idx();
		tabbar_calc_positions();
		tabs_switchto(openfiles.size()-1);
		tabbar_scroll_to(current_file);
	}
	return true;
}
bool				wnd_on_open()
{
	auto filename2=open_file_dialog();
	if(filename2)
	{
		openfiles.push_back(TextFile());
		auto &of=openfiles.back();
		std::string str;
		text2str(of.m_text, str);
		if(open_text_file(filename2, str))
		{
			hist_cont=false;
			dimensions_known=false;
			of.m_filename=filename2;
			tabbar_calc_positions();
			tabs_switchto(openfiles.size()-1);
			tabbar_scroll_to(current_file);
		}
	}
	return true;
}
bool				wnd_on_closetab()
{
	if(close_tab(current_file))
	{
		if(current_file>(int)openfiles.size()-1)
			current_file=openfiles.size()-1;
		tabs_switchto(current_file);
		tabbar_scroll_to(current_file);
		return true;
	}
	return false;
}
bool				wnd_on_next_tab()
{
	if(openfiles.size()>1)
	{
		tabs_switchto(current_file+1);
		tabbar_scroll_to(current_file);
		return true;
	}
	return false;
}
bool				wnd_on_prev_tab()
{
	if(openfiles.size()>1)
	{
		tabs_switchto(current_file-1);
		tabbar_scroll_to(current_file);
		return true;
	}
	return false;
}
bool				wnd_on_barorient()
{
	if(is_shift_down())
		tabbar_position=(TabbarPosition)mod(tabbar_position-1, TABBAR_ORIENT_COUNT);
	else
		tabbar_position=(TabbarPosition)mod(tabbar_position+1, TABBAR_ORIENT_COUNT);
	return true;
}

bool				wnd_on_quit()//return false to deny exit
{
	for(int k=0;k<(int)openfiles.size();++k)
	{
		tabs_switchto(k, false);
		if(tabs_ismodified(current_file))
		{
			switch(ask_to_save(current_file))
			{
			case 0://yes
				{
					auto _filename=save_file_dialog();
					if(_filename)
					{
						std::string str;
						text2str(*text, str);
						if(save_text_file(_filename, str))
						{
							*filename=_filename;
							set_title();
						}
					}
				}
				return false;
			case 1://no
				break;
			case 2://cancel
				return false;
			}
		}
	}
	return true;
}