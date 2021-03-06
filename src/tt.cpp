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
					start_mx=0, start_my=0,//LBUTTON down coordinates
					tab_count=4;
float				dx=0, dy=0;//non-tab character dimensions
char				sdf_available=false, sdf_active=false;
float				sdf_dx=0, sdf_dy=0;
float				sdf_dzoom=1.01f, sdf_slope=0.062023f;
std::string			exe_dir;

void				display_help()
{
#ifdef __linux__//spaces for monospace font (SDL messagebox doesn't print tabs)
	messagebox("Controls",
		"Ctrl+O:         Open\n"//TODO: config file
		"Ctrl+S:         Save\n"
		"Ctrl+Shift+S:   Save as\n"
		"Ctrl+Z/Y:       Undo/Redo\n"
		"Ctrl+D:         Clear undo/redo history\n"
		"Ctrl+X/C/V:     Cut/Copy/Paste\n"
		"Ctrl+U:         To lower case\n"
		"Ctrl+Shift+U:   To upper case\n"
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
		"Alt+Left/Right:         Previous/Next location\n"
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
		"F4:             Toggle benchmark\n"
		"F6:\tToggle signed distance field text render\n"
		"\n"
		"OpenGL %s\n"
		"Build: %s %s", GLversion, __DATE__, __TIME__);
#else//tabs for proportional font
	messagebox("Controls",
		"Ctrl+O:\t\tOpen\n"//TODO: config file
		"Ctrl+S:\t\tSave\n"
		"Ctrl+Shift+S:\tSave as\n"
		"Ctrl+Z/Y:\t\tUndo/Redo\n"
		"Ctrl+D:\t\tClear undo/redo history\n"
		"Ctrl+X/C/V:\tCut/Copy/Paste\n"
		"Ctrl+U:\t\tTo lower case\n"
		"Ctrl+Shift+U:\tTo upper case\n"
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
		"F4:\tToggle benchmark\n"
		"F6:\tToggle signed distance field text render\n"
		"\n"
		"OpenGL %s\n"
		"Build: %s %s", GLversion, __DATE__, __TIME__);
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
	"    gl_FragColor=mix(u_txtColor, u_bkColor, region.r);\n"//u_txtColor*(1-region.r) + u_bkColor*region.r
	"}",

	DECL_SHADER_END(text)
};
namespace			ns_sdftext
{
	int a_coords=-1, u_atlas=-1, u_txtColor=-1, u_bkColor=-1, u_zoom=-1;
	ShaderVar2 attributes[]=
	{
		DECL_SHADER_VAR(a_coords),
	};
	ShaderVar2 uniforms[]=
	{
		DECL_SHADER_VAR(u_atlas),
		DECL_SHADER_VAR(u_txtColor),
		DECL_SHADER_VAR(u_bkColor),
		DECL_SHADER_VAR(u_zoom),
	};
}
ShaderProgram		shader_sdftext=
{
	"shader_sdftext",

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
	"uniform sampler2D u_atlas;\n"		//uniforms: u_atlas, u_txtColor, u_bkColor, u_zoom
	"uniform vec4 u_txtColor, u_bkColor;\n"
	"uniform float u_zoom;\n"
	"void main()\n"
	"{\n"
	"    vec4 region=texture2D(u_atlas, v_texcoord);\n"

	"    float temp=clamp(u_zoom*(0.5f+0.45f/u_zoom-region.r), 0, 1);\n"
//	"    float temp=clamp(u_zoom*(0.5f+0.001f*u_zoom-region.r), 0, 1);\n"
	"    gl_FragColor=mix(u_txtColor, u_bkColor, temp);\n"

//	"    gl_FragColor=region.r>=0.5f?u_txtColor:u_bkColor;\n"//no anti-aliasing
	"}",

	DECL_SHADER_END(sdftext)
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
	toNDC_nobias(x1, y1, X1, Y1);
	toNDC_nobias(x2, y2, X2, Y2);
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
void				draw_rectangle_hollow(float x1, float x2, float y1, float y2, int color)
{
	toNDC(x1, y1, g_fbuf[0], g_fbuf[1]);
	toNDC(x1, y2, g_fbuf[2], g_fbuf[3]);
	toNDC(x2, y2, g_fbuf[4], g_fbuf[5]);
	toNDC(x2, y1, g_fbuf[6], g_fbuf[7]);
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

bool				col2idx(const char *text, int text_length, int tab0_cols, int idx0, int col0, float req_col, int *ret_idx, int *ret_col, float colroundbias)//returns true when runs out of characters (OOB)
{
	bool line_too_short=true;
	int idx=idx0, col=col0;
	for(;idx<text_length;++idx)
	{
		char c=text[idx];
		int dcol=0;
		if(c=='\t')
			dcol=tab_count-mod(col-tab0_cols, tab_count);
		else if(c>=32&&c<0xFF)
			dcol=1;
		if(col+dcol*colroundbias>=req_col)//dcol in [1 ~ tab_count]
		{
			line_too_short=false;
			break;
		}
		col+=dcol;
	}
	if(ret_col)
		*ret_col=col;
	if(ret_idx)
		*ret_idx=idx;
	return line_too_short;
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
struct				Bookmark
{
	int line,
		idx,//-1 means OOB, deduce it from col
		col;
	Bookmark():line(0), idx(0), col(0){}
	Bookmark(int line, int idx, int col):line(line), idx(idx), col(col){}
	bool set_gui(Text const &text, int line, float col, bool clampcol, float colroundbias)//returns true if text is hit
	{
		bool hit=true;
		size_t nlines=text_get_nlines(text);
		if(line<0)
			line=0, hit=false;
		else if(line>(int)nlines-1)
				line=nlines-1, hit=false;
		this->line=line;

		size_t len=0;
		auto _line=text_get_line(text, line, &len);
		col2idx(_line, len, 0, 0, 0, col, &this->idx, clampcol?&this->col:nullptr, 1-colroundbias);
		if(!clampcol)
			this->col=(int)(col+colroundbias);
		return hit;
	}
	void set_text(Text const &text, int line, int idx)
	{
		size_t nlines=text_get_nlines(text);
		if(line<0)
			line=0;
		else if(line>(int)nlines-1)
			line=nlines-1;
		this->line=line, this->idx=idx;
		update_col(text);
	}
	void setzero(){line=idx=col=0;}
	void setend(Text const &text)
	{
		size_t nlines=text_get_nlines(text);
		line=nlines-1;
		idx=text_get_len(text, line);
		update_col(text);
	}
	void set_linestart(){idx=col=0;}
	void set_lineend(Text const &text)
	{
		idx=text_get_len(text, line);
		update_col(text);
	}
	int get_absidx(Text const &text)const
	{
		int absidx=0;
		for(int kl=0;kl<line;++kl)
			absidx+=text_get_len(text, kl)+1;//account for newline
		absidx+=idx;
		return absidx;
	}
	void set_absidx(Text const &text, int absidx)
	{
		for(line=0;;++line)
		{
			int delta=text_get_len(text, line)+1;
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
		int nlines=text_get_nlines(text);
		if(line<0)
			line=0, idx=0, col=0;
		else if(line>nlines-1)
		{
			line=nlines-1, idx=text_get_len(text, line);
			update_col(text);
		}
		else
			update_idx(text);
	}
	void increment_idx_skipnewline(Text const &text)
	{
		if(idx<(int)text_get_len(text, line)-1)
			++idx;
		else if(line<(int)text_get_nlines(text)-1)
			++line, idx=0;
	}
	void decrement_idx_skipnewline(Text const &text)
	{
		if(idx)
			--idx;
		else if(line>0)
			--line, idx=(int)text_get_len(text, line)-1;
	}
	bool increment_idx(Text const &text)
	{
		size_t len=text_get_len(text, line);
		if(idx<(int)len)
		{
			++idx;
			return true;
		}
		else
		{
			size_t nlines=text_get_nlines(text);
			if(line<(int)nlines-1)
			{
				++line, idx=0;
				return true;
			}
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
			--line, idx=(int)text_get_len(text, line);
			return true;
		}
		return false;
	}
	//char dereference_idx_unchecked(Text const &text)const{return text[line][idx];}
	char dereference_idx(Text const &text)const
	{
		if(line>=0&&line<(int)text_get_nlines(text))
		{
			size_t len=0;
			auto _line=text_get_line(text, line, &len);
			if(idx==len)
				return '\n';
			if(idx>=0&&idx<(int)len)
				return _line[idx];
		}
		return '\0';
	}
	void update_col(Text const &text)
	{
		size_t len=0;
		auto _line=text_get_line(text, line, &len);
		this->col=idx2col(_line, idx, 0);//what if idx > size ?
	}
	void update_idx(Text const &text)
	{
		size_t len=0;
		auto _line=text_get_line(text, line, &len);
		col2idx(_line, len, 0, 0, 0, (float)col, &this->idx, &this->col, 0.5f);
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
		diff-=text_get_len(text, p1->line)+1-p1->idx;
		for(int kl=p1->line+1;kl<p2->line-1;++kl)
			diff-=text_get_len(text, kl)+1;//account for newline
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
		int line_max=(int)text_get_nlines(text)-1;
		for(;;)
		{
			size_t len=0;
			auto line=text_get_line(text, b.line, &len);
			if(b.idx+offset<(int)len+1)
			{
				b.idx+=offset;
				offset=0;
				break;
			}
			if(b.line<line_max)
			{
				offset-=len+1-b.idx;
				++b.line, b.idx=0;
			}
			else//overflow
			{
				b.idx=len;
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
				b.idx=text_get_len(text, b.line);
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

struct				QuadCoords
{
	float x1, x2, y1, y2;
};
QuadCoords			font_coords[128-32]={}, sdf_glyph_coords[128-32]={};
unsigned			font_txid=0, sdf_atlas_txid=0;
void				set_text_colors(U64 const &colors)
{
	if(sdf_active)
	{
		setGLProgram(shader_sdftext.program);
		send_color(ns_sdftext::u_txtColor, colors.lo);
		send_color(ns_sdftext::u_bkColor, colors.hi);
	}
	else
	{
		setGLProgram(shader_text.program);
		send_color(ns_text::u_txtColor, colors.lo);
		send_color(ns_text::u_bkColor, colors.hi);
	}
}
std::vector<float>	vrtx;
float				print_line(float x, float y, const char *msg, int msg_length, float tab_origin, float zoom, int req_cols, int *ret_idx, int *ret_cols)
{
	if(msg_length<1)
		return 0;
	float rect[4]={};
	QuadCoords *txc=nullptr, *atlas=sdf_active?sdf_glyph_coords:font_coords;
	if(sdf_active)
		zoom*=16.f/sdf_dy;
	float width=(sdf_active?sdf_dx:dx)*zoom, height=(sdf_active?sdf_dy:dy)*zoom;
	int tab_origin_cols=(int)(tab_origin/width), idx, printable_count=0, cursor_cols=ret_cols?*ret_cols:0, advance_cols;
	int rx1=0, ry1=0, rdx=0, rdy=0;
	get_current_region(rx1, ry1, rdx, rdy);
	if(y+height<ry1||y>=ry1+rdy)//off-screen optimization
		return 0;//no need to do anything, this line is outside the screen
	//	return idx2col(msg, msg_length, (int)(tab_origin/width))*width;
	float CX1=2.f/rdx, CX0=CX1*(x-rx1)-1;//delta optimization
	rect[1]=1-(y-ry1)*2.f/rdy;
	rect[3]=1-(y+height-ry1)*2.f/rdy;
	vrtx.resize(msg_length<<4);//vx, vy, txx, txy		x4 vertices/char
	int k=ret_idx?*ret_idx:0;
	if(req_cols<0||cursor_cols<req_cols)
	{
		CX1*=width;
		for(;k<msg_length;++k)
		{
			char c=msg[k];
			if(c>=32&&c<0xFF)
				advance_cols=1;
			else if(c=='\t')
				advance_cols=tab_count-mod(cursor_cols-tab_origin_cols, tab_count), c=' ';
			else
				advance_cols=0;
			if(advance_cols)
			{
				if(x+(cursor_cols+advance_cols)*width>=rx1&&x+cursor_cols*width<rx1+rdx)//off-screen optimization
				{
					rect[0]=CX1*cursor_cols+CX0;//xn1
					cursor_cols+=advance_cols;
					rect[2]=CX1*cursor_cols+CX0;//xn2

					//rect[0]=(x+msg_width-rx1)*2.f/rdx-1;//xn1
					//rect[1]=1-(y-ry1)*2.f/rdy;//yn1
					//rect[2]=(x+msg_width+width-rx1)*2.f/rdx-1;//xn2
					//rect[3]=1-(y+height-ry1)*2.f/rdy;//yn2

					//toNDC_nobias(float(x+msg_width		), float(y			), rect[0], rect[1]);
					//toNDC_nobias(float(x+msg_width+width	), float(y+height	), rect[2], rect[3]);//y2<y1

					idx=printable_count<<4;
					txc=atlas+c-32;
					vrtx[idx   ]=rect[0], vrtx[idx+ 1]=rect[1],		vrtx[idx+ 2]=txc->x1, vrtx[idx+ 3]=txc->y1;//top left
					vrtx[idx+ 4]=rect[0], vrtx[idx+ 5]=rect[3],		vrtx[idx+ 6]=txc->x1, vrtx[idx+ 7]=txc->y2;//bottom left
					vrtx[idx+ 8]=rect[2], vrtx[idx+ 9]=rect[3],		vrtx[idx+10]=txc->x2, vrtx[idx+11]=txc->y2;//bottom right
					vrtx[idx+12]=rect[2], vrtx[idx+13]=rect[1],		vrtx[idx+14]=txc->x2, vrtx[idx+15]=txc->y1;//top right

					++printable_count;
				}
				else
					cursor_cols+=advance_cols;
				if(req_cols>=0&&cursor_cols>=req_cols)
				{
					++k;
					break;
				}
			}
		}
		if(printable_count)
		{
			if(sdf_active)
			{
				setGLProgram(shader_sdftext.program);
				glUniform1f(ns_sdftext::u_zoom, zoom/sdf_slope);
				select_texture(sdf_atlas_txid, ns_sdftext::u_atlas);
				glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);									GL_CHECK();
				glBufferData(GL_ARRAY_BUFFER, printable_count<<6, vrtx.data(), GL_STATIC_DRAW);	GL_CHECK();
				glVertexAttribPointer(ns_sdftext::a_coords, 4, GL_FLOAT, GL_TRUE, 0, 0);		GL_CHECK();

				glEnableVertexAttribArray(ns_sdftext::a_coords);	GL_CHECK();
				glDrawArrays(GL_QUADS, 0, printable_count<<2);		GL_CHECK();
				glDisableVertexAttribArray(ns_sdftext::a_coords);	GL_CHECK();
			}
			else
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
	}
	if(ret_idx)
		*ret_idx=k;
	if(ret_cols)
		*ret_cols=cursor_cols;
	return cursor_cols*width;
}
void				print_text(float tab_origin, float x, float y, Text const &text, Bookmark const &i, Bookmark const &f, float zoom, int *line=nullptr, int *col=nullptr)//returns vertical advance in pixels
{
	if(i>=f)
		return;
	float ndc[4];//x1, x2, y1, y2
	QuadCoords *txc=nullptr, *atlas=sdf_active?sdf_glyph_coords:font_coords;
	if(sdf_active)
		zoom*=16.f/sdf_dy;
	float width=(sdf_active?sdf_dx:dx)*zoom, height=(sdf_active?sdf_dy:dy)*zoom;
	int tab_origin_cols=(int)(tab_origin/width), cursor_line=line?*line:0, cursor_col=col?*col:0, advance_cols, printable_count=0;
	int rx1=0, ry1=0, rdx=0, rdy=0;
	get_current_region(rx1, ry1, rdx, rdy);
	float
		CX1=2.f/rdx, CX0=CX1*(x-rx1)-1,
		CY1=-2.f/rdy, CY0=1+CY1*(y-ry1);
	CX1*=width;
	CY1*=height;
	int view_l1=(int)((ry1-y    )/height)-1, view_c1=(int)((rx1-x    )/width)-1,
		view_l2=(int)((ry1-y+rdy)/height)+1, view_c2=(int)((rx1-x+rdx)/width)+1;
	if(cursor_line>=view_l2)//text is below region bottom
		return;//nothing needs to be done
	Bookmark k=i;
	if(cursor_line<view_l1)//first line is above region top
	{
		k.idx=0, k.line+=view_l1-cursor_line;
		if(k>=f)//nothing left to print
			return;//nothing needs to be done
		cursor_col=0, cursor_line=view_l1;
	}
	ndc[2]=CY1*cursor_line+CY0, ndc[3]=ndc[2]+CY1;
	vrtx.resize((view_c2-view_c1)*(view_l2-view_l1)<<4);//nchars in grid	*	{vx, vy, txx, txy		x4 vertices/char}	~= 5MB at FHD screen
	size_t len=0;
	char *_line=text_get_line(text, k.line, &len);
	for(;k<f;)
	{
		if(k.idx>=len)
		{
			cursor_col=0, ++cursor_line;
			if(cursor_line>=view_l2)//following lines are below region bottom
				break;
			ndc[2]=CY1*cursor_line+CY0, ndc[3]=ndc[2]+CY1;
			k.increment_idx(text);
			_line=text_get_line(text, k.line, &len);
			continue;
		}
		char c=_line[k.idx];
		if(c>=32&&c<0xFF)
			advance_cols=1;
		else if(c=='\t')
			advance_cols=tab_count-mod(cursor_col-tab_origin_cols, tab_count), c=' ';
		else
			advance_cols=0;
		if(advance_cols)
		{
			if(cursor_col+advance_cols>=view_c1&&cursor_col<view_c2)
			{
				ndc[0]=CX1*cursor_col+CX0;//xn1
				cursor_col+=advance_cols;
				ndc[1]=CX1*cursor_col+CX0;//xn2

				int idx=printable_count<<4;
				if(idx>=(int)vrtx.size())
					vrtx.resize(vrtx.size()+(vrtx.size()>>1));//grow by x1.5
				txc=atlas+c-32;
				vrtx[idx   ]=ndc[0], vrtx[idx+ 1]=ndc[2],		vrtx[idx+ 2]=txc->x1, vrtx[idx+ 3]=txc->y1;//top left
				vrtx[idx+ 4]=ndc[0], vrtx[idx+ 5]=ndc[3],		vrtx[idx+ 6]=txc->x1, vrtx[idx+ 7]=txc->y2;//bottom left
				vrtx[idx+ 8]=ndc[1], vrtx[idx+ 9]=ndc[3],		vrtx[idx+10]=txc->x2, vrtx[idx+11]=txc->y2;//bottom right
				vrtx[idx+12]=ndc[1], vrtx[idx+13]=ndc[2],		vrtx[idx+14]=txc->x2, vrtx[idx+15]=txc->y1;//top right

				++printable_count;
			}
			else
				cursor_col+=advance_cols;
		}
		k.increment_idx(text);
	}
	if(printable_count)
	{
		if(sdf_active)
		{
			setGLProgram(shader_sdftext.program);
			glUniform1f(ns_sdftext::u_zoom, zoom/sdf_slope);
			select_texture(sdf_atlas_txid, ns_sdftext::u_atlas);
			glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);									GL_CHECK();
			glBufferData(GL_ARRAY_BUFFER, printable_count<<6, vrtx.data(), GL_STATIC_DRAW);	GL_CHECK();
			glVertexAttribPointer(ns_sdftext::a_coords, 4, GL_FLOAT, GL_TRUE, 0, 0);		GL_CHECK();

			glEnableVertexAttribArray(ns_sdftext::a_coords);	GL_CHECK();
			glDrawArrays(GL_QUADS, 0, printable_count<<2);		GL_CHECK();
			glDisableVertexAttribArray(ns_sdftext::a_coords);	GL_CHECK();
		}
		else
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
	if(line)
		*line=cursor_line;
	if(col)
		*col=cursor_col;
}
#ifdef DEBUG_CURSOR
float				debug_print_cols(float x, float y, float zoom, int req_cols, int den)
{
	for(int k=0;k<1000;++k)
		g_buf[k]='0'+k/den%10;
	g_buf[1000]='\0';
	return print_line(x, y, g_buf, 1000, 0, zoom);
}
#endif

float				print(short zoom, float tab_origin, float x, float y, const char *format, ...)
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
U64					colors_text=0x20ABABABFFBBBBBB;//dark mode
//U64				colors_text=0xFF000000FFABABAB;//dark mode, opaque black on black?
U64					colors_selection=0xA0FF9933FFFFFFFF;//colors of selected text
//U64				colors_selection=0xA0FF0000FFABABAB;
int					color_cursor=0xFFFFFFFF,//color of the cursor stick
					color_cursorlinebk=0xFF202820;//color of current line highlight
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
struct				Cursor
{
	bool rectsel;
	char reserved[3];
	Bookmark cursor, selcur;

	Cursor(){*(int*)&rectsel=0;}
	void deselect(Text const &text, int direction)//direction: -1: left, 0: don't care, 1: right
	{
		rectsel=false;
		if(direction)
		{
			if(rectsel)
			{
				auto r=get_rectsel();
				if(direction==-1)
					cursor.set_gui(text, r.y1, (float)r.x1, true, 0.5f);
				else
					cursor.set_gui(text, r.y2, (float)r.x2, true, 0.5f);
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
		cursor.set_gui(text, l1, (float)col1, false, 0.5f);
		selcur.set_gui(text, l2, (float)col2, false, 0.5f);
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
float				font_zoom=1, zoom_min=1, zoom_max=32;//font pixel size
int					wpx=0, wpy=0,//window position inside the text buffer, in pixels
					text_width=0;//in characters
Cursor				*cur=nullptr;
void				text2str(Text const &text, std::string &str)
{
	str.clear();
	int nlines_m1=(int)text_get_nlines(text)-1;
	if(nlines_m1+1)
	{
		size_t len=0;
		char *line;
		for(int kl=0;kl<nlines_m1;++kl)
		{
			line=text_get_line(text, kl, &len);
			str.append(line, line+len);
			str+='\n';
		}
		line=text_get_line(text, nlines_m1, &len);
		str.append(line, line+len);
	}
}
void				str2text(const char *str, int len, Text &text, TextType type, void *payload, size_t p_bytes)
{
	text_clear(&text, type, payload, p_bytes);
	for(int k=0, k2=0;;++k2)
	{
		if(k2>=len)
		{
			int l0=text_get_nlines(text);
			text_insert_lines(text, l0, 1, ACT_TYPE);
			text_replace(text, l0, 0, 0, str+k, k2-k, 1, ACT_TYPE);
			break;
		}
		if(str[k2]=='\n')
		{
			int l0=text_get_nlines(text);
			text_insert_lines(text, l0, 1, ACT_TYPE);
			text_replace(text, l0, 0, 0, str+k, k2-k, 1, ACT_TYPE);
			k=k2+1;
		}
	}
}
void				rectsel_copy(Text const &src, Cursor const &cur, Text &dst)
{
	text_clear(&dst, TEXT_NO_HISTORY, 0, 0);
	int l1, l2, col1, col2;
	cur.get_rectsel(l1, l2, col1, col2);
	std::string temp;
	if(l1<l2+1)
	{
		text_insert_lines(dst, 0, l2-l1, ACT_TYPE);//first line is already there
		for(int kl=l1;kl<=l2;++kl)
		{
			size_t len=0;
			auto line=text_get_line(src, kl, &len);

			int idx1, c1, prepad=0,
				idx2, c2;

			col2idx(line, len, 0, 0, 0, (float)col1, &idx1, &c1, 0.5f);
			if(c1<col1&&idx1<(int)len)
			{
				int c1_2=c1+tab_count-mod(c1, tab_count);
				if(c1_2<=col2)
					++idx1, c1=c1_2;
			}
			if(c1>col1)
				prepad=c1-col1;

			col2idx(line, len, 0, idx1, c1, (float)col2, &idx2, &c2, 0.5f);
			idx2-=c2>col2;

			temp.clear();
			temp.append(prepad, ' ');
			if(idx1<idx2)
				temp.append(line+idx1, line+idx2);
			temp.append(col2-col1-temp.size(), ' ');

			text_replace(dst, kl-l1, 0, 0, temp.c_str(), temp.size(), 1, ACT_TYPE);
		}
	}
}
void				rectsel_insert(Text const &src, int l1, int col1, Text &dst, ActionType action)
{
	int kls=0, kld=l1;
	int srclines=text_get_nlines(src), dstlines=text_get_nlines(dst);
	for(;kls<srclines&&kld<dstlines;++kls, ++kld)
	{
		size_t srclen=0, dstlen=0;
		auto srcline=text_get_line(src, kls, &srclen);
		auto dstline=text_get_line(dst, kld, &dstlen);
		int idx1, c1;
		col2idx(dstline, dstlen, 0, 0, 0, (float)col1, &idx1, &c1, 0.5f);
		if(c1<col1)
		{
			int ntabs=col1/tab_count-c1/tab_count, nspaces=mod(col1, tab_count);
			text_replace(dst, kld, idx1, idx1, "\t", 1, ntabs, action);
			idx1+=ntabs;
			text_replace(dst, kld, idx1, idx1, " ", 1, nspaces, action);
			idx1+=nspaces;
		}
		text_replace(dst, kld, idx1, idx1, srcline, srclen, 1, action);
	}
	if(kls<srclines)
	{
		text_insert_lines(dst, dstlines, srclines-kls, action);
		dstlines+=srclines-kls;
	}
	for(;kls<srclines;++kls, ++kld)
	{
		size_t srclen=0;
		auto srcline=text_get_line(src, kls, &srclen);
		text_replace(dst, kld, 0, 0, "\t", 1, col1/tab_count, action);
		size_t dstlen=text_get_len(dst, kld);
		text_replace(dst, kld, dstlen, dstlen, " ", 1, mod(col1, tab_count), action);
		dstlen=text_get_len(dst, kld);
		text_replace(dst, kld, dstlen, dstlen, srcline, srclen, 1, action);
	}
}
void				selection_copy(Text const &src, Cursor const &cur, Text &dst)
{
	text_clear(&dst, TEXT_NO_HISTORY, 0, 0);
	//dst.clear();
	Bookmark i, f;
	cur.get_selection(i, f);
	text_insert_lines(dst, 0, f.line-i.line, ACT_TYPE);//one line is already there
	if(i.line==f.line)
	{
		size_t len=0;
		auto line=text_get_line(src, i.line, &len);
		text_replace(dst, 0, 0, 0, line+i.idx, f.idx-i.idx, 1, ACT_TYPE);
	}
	else
	{
		size_t len=0;
		auto line=text_get_line(src, i.line, &len);
		text_replace(dst, 0, 0, 0, line+i.idx, len-i.idx, 1, ACT_TYPE);
		for(int kl=i.line+1;kl<f.line;++kl)
		{
			line=text_get_line(src, kl, &len);
			text_replace(dst, kl-i.line, 0, 0, line, len, 1, ACT_TYPE);
		}
		line=text_get_line(src, f.line, &len);
		text_replace(dst, f.line-i.line, 0, 0, line, f.idx, 1, ACT_TYPE);
	}
}
void				selection_insert(Text const &src, int l1, int idx1, Text &dst, ActionType action)
{
	int nlines=text_get_nlines(src);
	if(!nlines)
		return;
	size_t len=0, len2=0;
	char *line=nullptr;
	if(nlines==1)
	{
		//---src[0]---
		line=text_get_line(src, 0, &len);
		text_replace(dst, l1, idx1, idx1, line, len, 1, action);
	}
	else
	{
		//----src[0]
		//src[1]
		//...
		//src[n-1]---
		text_insert_lines(dst, l1+1, nlines-1, action);
		for(int kls=1, kld=l1+1;kls<nlines;++kls, ++kld)
		{
			line=text_get_line(src, kls, &len);
			text_replace(dst, kld, 0, 0, line, len, 1, action);
		}

		line=text_get_line(dst, l1, &len);
		len2=text_get_len(dst, l1+nlines-1);
		text_replace(dst, l1+nlines-1, len2, len2, line+idx1, len-idx1, 1, action);//append the rest of dstline[0] to dstline[n-1]

		line=text_get_line(src, 0, &len);
		len2=text_get_len(dst, l1);
		text_replace(dst, l1, idx1, len2, line, len, 1, action);//replace the rest of dstline[0] with srcline[0]
	}
}

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

bool				dimensions_known=false, wnd_to_cursor=false;

struct				TextFile
{
	std::string m_filename;//UTF-8
	int m_untitled_idx;

	Text m_text;//ASCII

	float m_wcx, m_wcy;//window position inside the text buffer, in character units
	Cursor m_cur;

//	History m_history;
//	int m_histpos_saved, m_histpos;//pointing at previous action

	TextFile():m_untitled_idx(0), m_wcx(0), m_wcy(0)//default constructor
		//m_histpos_saved(-1), m_histpos(-1)
	{
		m_text=text_create(TEXT_NORMAL, &m_cur, sizeof(m_cur));
	}
	TextFile(TextFile const &f):m_filename(f.m_filename), m_text(text_deepcopy(f.m_text)), m_untitled_idx(f.m_untitled_idx),//copy constructor
		m_wcx(f.m_wcx), m_wcy(f.m_wcy), m_cur(f.m_cur)
		//m_history(f.m_history), m_histpos_saved(f.m_histpos_saved), m_histpos(f.m_histpos)
	{}
	TextFile(TextFile &&f):m_filename(std::move(f.m_filename)), m_text(f.m_text), m_untitled_idx(f.m_untitled_idx),//move constructor
		m_wcx(f.m_wcx), m_wcy(f.m_wcy), m_cur(f.m_cur)
		//m_history(std::move(f.m_history)), m_histpos_saved(f.m_histpos_saved), m_histpos(f.m_histpos)
	{
		f.m_text=0;
	}
	~TextFile()
	{
		if(m_text)
			text_destroy(m_text);
	}
	TextFile& operator=(TextFile const &f)//copy assignment
	{
		if(&f!=this)
		{
			m_filename=f.m_filename;
			m_untitled_idx=f.m_untitled_idx;

			m_text=text_deepcopy(f.m_text);

			m_wcx=f.m_wcx, m_wcy=f.m_wcy;
			m_cur=f.m_cur;

			//m_history=f.m_history;
			//m_histpos_saved=f.m_histpos_saved, m_histpos=f.m_histpos;
		}
		return *this;
	}
	TextFile& operator=(TextFile &&f)//move assignment
	{
		if(&f!=this)
		{
			m_filename=std::move(f.m_filename);
			m_untitled_idx=f.m_untitled_idx;

			m_text=f.m_text, f.m_text=0;

			m_wcx=f.m_wcx, m_wcy=f.m_wcy;
			m_cur=f.m_cur;

			//m_history=std::move(f.m_history);
			//m_histpos_saved=f.m_histpos_saved, m_histpos=f.m_histpos;
		}
		return *this;
	}
	void clear()
	{
		m_filename.clear();
		m_cur.reset();
		text_clear(&m_text, TEXT_NORMAL, &m_cur, sizeof(m_cur));
		//m_text.clear();
		m_untitled_idx=0;
		m_wcx=0, m_wcy=0;
		//m_history.clear();
		//m_histpos_saved=m_histpos=-1;
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
	float right;
	int idx;
	TabPosition():right(0), idx(0){}
	TabPosition(float right, int idx):right(right), idx(idx){}
	void set(float right, int idx){this->right=right, this->idx=idx;}
};
std::vector<TabPosition> tabbar_tabs;//TODO: merge TabPosition with TextFile

bool				tabs_ismodified(int tab_idx){return text_is_modified(openfiles[tab_idx].m_text)!=0;}
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
	
	tabbar_calc_positions();
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
	//history=&of->m_history;
	//histpos=&of->m_histpos;

	wpx=int(of->m_wcx*font_zoom);
	wpy=int(of->m_wcy*font_zoom);
	cur=&of->m_cur;

	if(change_title)
		set_title();
	dimensions_known=false;
}

int					calc_text_cols(Text const &text)
{
	size_t nlines=text_get_nlines(text);
	int cols=0;
	for(int kl=0;kl<(int)nlines;++kl)
	{
		size_t len=0;
		auto line=text_get_line(text, kl, &len);
		int temp=idx2col(line, len, 0);
		if(cols<temp)
			cols=temp;
	}
	return cols;
}

void				text_erase(Text &text, int l1, int idx1, int l2, int idx2, ActionType action)
{
	if(l1==l2)
	{
		if(idx1!=idx2)
		{
			if(idx2<idx1)
				std::swap(idx1, idx2);
			text_replace(text, l1, idx1, idx2, 0, 0, 0, action);
		}
	}
	else
	{
		if(l2<l1)
		{
			std::swap(l1, l2);
			std::swap(idx1, idx2);
		}
		size_t len1=text_get_len(text, l1), len2=0;
		auto line2=text_get_line(text, l2, &len2);
		text_replace(text, l1, idx1, len1, line2+idx2, len2-idx2, 1, action);
		text_erase_lines(text, l1+1, l2-l1, action);
	}
}
void				text_insert(Text &text, int l0, int idx0, const char *a, int len, int *ret_line, int *ret_idx, ActionType action)
{
	int k=0, k2=0;
	for(int kl=l0;;++kl)
	{
		for(;k2<len&&a[k2]!='\n';++k2);
		if(k2>=len)
		{
			int idx=k2-k;
			if(k)
				text_replace(text, kl, 0, 0, a+k, k2-k, 1, action);
			else
			{
				text_replace(text, kl, idx0, idx0, a+k, k2-k, 1, action);
				idx+=idx0;
			}
			if(ret_line)
				*ret_line=kl;
			if(ret_idx)
				*ret_idx=idx;
			break;
		}
		else if(k)
		{
			text_insert_lines(text, kl, 1, action);
			text_replace(text, kl, 0, 0, a+k, k2-k, 1, action);
		}
		else
		{
			text_insert_lines(text, kl+1, 1, action);
			size_t len=0;
			auto line=text_get_line(text, l0, &len);
			text_replace(text, l0+1, 0, 0, line+idx0, len-idx0, 1, action);
			text_replace(text, l0, idx0, len-idx0, a+k, k2-k, 1, action);
		}
		++k2;
		k=k2;
	}
}
void				replace_tab_with_spaces(Text &text, int l0, int idx, ActionType action)
{
	size_t len=0;
	auto line=text_get_line(text, l0, &len);
	int col0=idx2col(line, idx, 0);
	int nspaces=tab_count-mod(col0, tab_count);
	text_replace(text, l0, idx, idx+1, " ", 1, nspaces, action);
}
bool				text_erase_rect(Text &text, int l1, int l2, int col1, int col2, ActionType action)//returns true if any text was erased
{
	bool erased=false;
	if(l2<l1)
		std::swap(l1, l2);
	if(col2<col1)
		std::swap(col1, col2);
	size_t nlines=text_get_nlines(text);
	for(int kl=l1;kl<=l2&&kl<(int)nlines;++kl)
	{
		size_t len=0;
		auto line=text_get_line(text, kl, &len);
		int idx1=0, c1=0;
		bool line_OOB=col2idx(line, len, 0, 0, 0, (float)col1, &idx1, &c1, 0.5f);
		if(!line_OOB)
		{
			erased=true;
			if(c1!=col1)//tab caused misalign
			{
				if(idx1<(int)len&&line[idx1]=='\t')
				{
					replace_tab_with_spaces(text, kl, idx1, action);
					col2idx(line, len, 0, 0, 0, (float)col1, &idx1, &c1, 0.5f);
				}
				else if(idx1>0&&line[idx1-1]=='\t')
				{
					replace_tab_with_spaces(text, kl, idx1-1, action);
					col2idx(line, len, 0, 0, 0, (float)col1, &idx1, &c1, 0.5f);
				}
			}
			if(c1==col1)
			{
				int idx2=0, c2=0;
				col2idx(line, len, 0, 0, 0, (float)col2, &idx2, &c2, 0.5f);
				if(idx1<idx2)
					text_replace(text, kl, idx1, idx2, 0, 0, 0, action);
			}
		}
	}
	return erased;
}
#if 0
void				text_insert_rect(Text &text, int l1, int l2, int col0, const char *a, int len, ActionType action)
{
	for(int kl=l1;kl<=l2;++kl)
	{
		size_t len2=0;
		auto line=text_get_line(text, kl, &len2);
		int idx1=0, c1=0;
		col2idx(line, len2, 0, 0, 0, (float)col0, &idx1, &c1, 0.5f);
		if(c1<col0)
		{
			int ntabs=col0/tab_count-c1/tab_count, nspaces=mod(col0, tab_count);

			text_replace(text, kl, len2, len2, "\t", 1, ntabs, action);
			len2+=ntabs;
			text_replace(text, kl, len2, len2, " ", 1, nspaces, action);
			idx1=len2+=nspaces;
		}
		text_replace(text, kl, idx1, len2, a, len, 1, action);
	}
}
#endif
void				text_insert1(Text &text, int l0, int idx0, char c, ActionType action)
{
	if(c=='\n')
	{
		text_insert_lines(text, l0+1, 1, action);
		size_t len=0;
		auto line=text_get_line(text, l0, &len);
		text_replace(text, l0+1, 0, 0, line+idx0, len-idx0, 1, action);
		text_replace(text, l0, idx0, len, 0, 0, 0, action);
	}
	else
		text_replace(text, l0, idx0, idx0, &c, 1, 1, action);
}
void				text_erase1_bksp(Text &text, int l0, int idx0, ActionType action)
{
	if(idx0>0)
		text_replace(text, l0, idx0-1, idx0, 0, 0, 0, action);
	else if(l0>0)
	{
		size_t len=0, len2=text_get_len(text, l0-1);
		auto line=text_get_line(text, l0, &len);
		text_replace(text, l0-1, len2, len2, line, len, 1, action);
		text_erase_lines(text, l0, 1, action);
	}
}
void				text_erase1_del(Text &text, int l0, int idx0, ActionType action)
{
	int len=text_get_len(text, l0);
	if(idx0<len)
		text_replace(text, l0, idx0, idx0+1, 0, 0, 0, action);
	else if(l0<(int)text_get_nlines(text))
	{
		size_t len=0;
		auto line=text_get_line(text, l0+1, &len);
		text_replace(text, l0, idx0, idx0, line, len, 1, action);
		text_erase_lines(text, l0+1, 1, action);
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
	for(int kl=l1;kl<=l2;++kl)//find col_clearance
	{
		//a   [b]b	here col_clearance = 1 space
		//aaa [b]b
		//aa  [b]b
		size_t len=0;
		auto line=text_get_line(text, kl, &len);
		int idx, c;
		col2idx(line, len, 0, 0, 0, (float)col0, &idx, &c, 0.5f);
		for(;idx<(int)len&&!isspace(line[idx]);++idx, ++c);
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
			size_t len=0;
			auto line=text_get_line(text, kl, &len);
			int idx1, c1, idx2, c2;
			col2idx(line, len, 0, 0, 0, (float)dst_col, &idx1, &c1, 0.5f);
			col2idx(line, len, 0, 0, 0, (float)col1, &idx2, &c2, 0.5f);
			text_replace(text, kl, idx1, idx2, 0, 0, 0, ACT_INDENT);
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
		size_t len=0;
		auto line=text_get_line(text, kl, &len);
		int idx1, c1;
		col2idx(line, len, 0, 0, 0, (float)col1, &idx1, &c1, 0.5f);
		text_replace(text, kl, idx1, idx1, "\t", 1, 1, ACT_INDENT);
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
		size_t len=0;
		auto line=text_get_line(text, kl, &len);
		int idx1, c1;
		col2idx(line, len, 0, 0, 0, (float)tab_count, &idx1, &c1, 0.5f);
		int kc=0;
		for(;kc<idx1&&isspace(line[kc]);++kc);
		if(kl==i.line)
			i_diff=idx2col(line, kc, 0);
		if(kl==f.line)
			f_diff=idx2col(line, kc, 0);
		if(kc)
			text_replace(text, kl, 0, kc, 0, 0, 0, ACT_INDENT);
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
		size_t len=0;
		auto line=text_get_line(text, kl, &len);
		text_replace(text, kl, 0, 0, "\t", 1, 1, ACT_INDENT);
	}
	cur.cursor.col+=tab_count;
	cur.selcur.col+=tab_count;
	cur.cursor.update_idx(*text);
	cur.selcur.update_idx(*text);
}
void				line_insert(Text &text, int l0, int c1, int c2, const char *src, int len, ActionType action)
{
	int nlines=text_get_nlines(text);
	size_t len2=0;
	if(l0<nlines)
	{
		auto line=text_get_line(text, l0, &len2);
		int idx1, col1, idx2, col2;
		col2idx(line, len2, 0, 0, 0, (float)c1, &idx1, &col1, 0.5f);

		if(col1>c1)//a tab caused misalign
		{
			--idx1;
			col1=idx2col(line, idx1, 0);
		}
		col2idx(line, len2, 0, idx1, col1, (float)c2, &idx2, &col2, 0.5f);
		if(col1<c1)//a tab caused misalign or line is not long enough
		{
			int ntabs=c1/tab_count-col1/tab_count, nspaces=mod(c1, tab_count);
			if(ntabs==0)
				nspaces-=mod(col1, tab_count);
			text_replace(text, l0, idx1, idx2, src, len, 1, action);
			if(nspaces>0)
				text_replace(text, l0, idx1, idx1, " ", 1, nspaces, action);
			if(ntabs>0)
				text_replace(text, l0, idx1, idx1, "\t", 1, ntabs, action);
		}
		else
			text_replace(text, l0, idx1, idx2, src, len, 1, action);
	}
	else
	{
		int ntabs=c1/tab_count, nspaces=mod(c1, tab_count);
		text_insert_lines(text, nlines, 1, action);
		text_replace(text, nlines, len2, len2, "\t", 1, ntabs, action), len2=text_get_len(text, nlines);
		text_replace(text, nlines, len2, len2, " ", 1, nspaces, action), len2=text_get_len(text, nlines);
		text_replace(text, nlines, 0, 0, src, len, 1, action);
	}
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
bool				cursor_at_mouse(Text const &text, short mx, short my, Bookmark &cursor, bool clampcol, float colroundbias)//sets the cursor to mouse coordinates, returns true if text was hit
{
	int mousex=mx-window_editor.x1, mousey=my-window_editor.y1;
	float dypx=dy*font_zoom, dxpx=dx*font_zoom,
		line=(wpy+mousey)/dypx, col=float(wpx+mousex)/dxpx;
	mline=(int)line, mcol=(int)(col+colroundbias);

	return cursor.set_gui(text, (int)line, col, clampcol, colroundbias);
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
float				tabbar_printname(float x, float y, int tab_idx, bool test=false, bool qualified=false, const char **pbuf=nullptr, int *len=nullptr)
{
	auto &f=openfiles[tab_idx];
	int modified=text_is_modified(f.m_text);
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
	float msg_width=0;
	if(test)
	{
		msg_width=dx*idx2col(g_buf+!modified, printed+modified, 0);
		if(pbuf)
			*pbuf=g_buf+!modified;
		if(len)
			*len=printed+modified;
	}
	else
	{
		if(tab_idx==current_file)
			set_text_colors(colors_selection);
		msg_width=print_line(x, y, g_buf+!modified, printed+modified, 0, 1);
		if(tab_idx==current_file)
			set_text_colors(colors_text);
	}
	return msg_width;
}
void				tabbar_calc_positions()
{
	tabbar_tabs.resize(openfiles.size());
	tabbar_printnames_init();
	float x=0;
	for(int k=0;k<(int)openfiles.size();++k)
	{
		auto &of=openfiles[k];
		float x0=x;
		x+=10+tabbar_printname(x, 0, k, true);
		tabbar_tabs[k].set(x, k);
	}
}
void				bring_object_to_view(int &winpos, int winsize, float objstart, float objend)
{
	if(winsize>objend-objstart)
	{
		if(winpos+winsize<objend)
			winpos=(int)floor(objend-winsize);
		if(winpos>objstart)
			winpos=(int)floor(objstart);
	}
	else
		winpos=(int)floor(objstart);
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
			float top=tab_idx*dy;
			bring_object_to_view(tabbar_wpy, h, top, top+dy);
		}
		break;
	}
}
void				general_selection_erase(ActionType action)
{
	if(cur->selection_exists())
	{
		if(cur->rectsel)
		{
			bool erased=text_erase_rect(*text, cur->cursor.line, cur->selcur.line, cur->cursor.col, cur->selcur.col, action);
			if(erased)
			{
				int colstart=cur->cursor.col;
				if(colstart>cur->selcur.col)
					colstart=cur->selcur.col;
				cur->selcur.col=cur->cursor.col=colstart;
			}
			else
				cur->selcur.col=cur->cursor.col;
		}
		else
		{
			text_erase(*text, cur->cursor.line, cur->cursor.idx, cur->selcur.line, cur->selcur.idx, action);
			if(cur->cursor<cur->selcur)
				cur->selcur=cur->cursor;
			else
				cur->cursor=cur->selcur;
		}
	}
}

struct				SDFTextureHeader
{
	double slope;
	char
		grid_start_x, grid_start_y,
		cell_size_x, cell_size_y,
		csize_x, csize_y,
		reserved[2];
};
void				wnd_on_create(){}
bool				wnd_on_init(int argc, char **argv)
{
	make_gl_program(shader_2d);
	make_gl_program(shader_text);
	make_gl_program(shader_sdftext);
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
	dx=(float)(rgb[0]&0xFF), dy=(float)(rgb[1]&0xFF);
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
	
	auto bmp=(unsigned char*)stbi_load((exe_dir+"font_sdf.PNG").c_str(), &iw, &ih, &bytespp, 1);
	if(bmp)
	{
		sdf_available=true;
		SDFTextureHeader header;
		memcpy(&header, bmp, sizeof(header));
		sdf_dx=header.csize_x;
		sdf_dy=header.csize_y;
		sdf_slope=(float)header.slope;
		for(int c=32;c<127;++c)
		{
			auto rect=sdf_glyph_coords+c-32;
			int px=header.grid_start_x+header.cell_size_x*(c&7),
				py=header.grid_start_y+header.cell_size_y*((c>>3)-4);
			rect->x1=(float)px/iw;
			rect->x2=(float)(px+sdf_dx)/iw;
			rect->y1=(float)py/ih;
			rect->y2=(float)(py+sdf_dy)/ih;
		}
		glGenTextures(1, &sdf_atlas_txid);
		send_texture_pot_linear(sdf_atlas_txid, bmp, iw, ih);
		stbi_image_free(bmp);
		sdf_active=true;
		set_text_colors(colors_text);
		sdf_active=false;
	}
	set_text_colors(colors_text);
	wnd_on_toggle_renderer();
	prof_add("Load font");
	
	int files_opened=0;
	if(argc>0)
	{
		std::string str;
		for(int k=0;k<argc;++k)
		{
			if(open_text_file(argv[k], str))
			{
				openfiles.push_back(TextFile());
				auto &of=openfiles.back();
				of.m_filename=argv[k];
				str2text(str.c_str(), str.size(), of.m_text, TEXT_NORMAL, cur, sizeof(*cur));
				text_mark_saved(of.m_text);
				++files_opened;
			}
			else
				messagebox("Error", "Failed to open:\n%s", argv[k]);
		}
	}
	tabbar_calc_positions();
	if(files_opened>0)
	{
		current_file=openfiles.size()-1;
		tabs_switchto(current_file);
		tabbar_scroll_to(current_file);
		dimensions_known=false;
	}
	else
	{
		openfiles.push_back(TextFile());
		current_file=0;
		tabs_switchto(current_file);
	}
	prof_add("Open tabs");
	return true;
}
void				wnd_on_resize(){}
void				tabbar_draw_horizontal(float ty1, float ty2, float wy1, float wy2)
{
	if(h>ty2-ty1)
	{
		if(tabbar_wpx>tabbar_tabs.back().right-(w>>1))
			tabbar_wpx=(int)tabbar_tabs.back().right-(w>>1);
		if(tabbar_wpx<0)
			tabbar_wpx=0;
		window_tabbar.set(0, w, (short)ty1, (short)ty2);
		set_region_immediate(0, w, (short)ty1, (short)ty2);
		tabbar_printnames_init();
		float x=-(float)tabbar_wpx;
		for(int k=0;k<(int)openfiles.size();++k)
		{
			auto &of=openfiles[k];
			float x0=x;
			x+=10+tabbar_printname(x, (float)ty1, k);
			//if(k==current_file)
			//{
			//	draw_rectangle(x-10, x, ty1, ty1+dy, colors_selection.hi);
			//	draw_rectangle(x0, x, ty1+dy, ty1+(dy<<1), colors_selection.hi);
			//}
			draw_rectangle(x-10, x-1, (float)ty1, ty1+dy, k==current_file?colors_selection.hi:colors_text.hi);
			draw_rectangle(x0, x-1, ty1+dy, ty1+dy*2, k==current_file?colors_selection.hi:colors_text.hi);
		}
		window_editor.set(0, w, (short)wy1, (short)wy2);
		set_region_immediate(0, w, (short)wy1, (short)wy2);
	}
}
void				tabbar_draw_sidebar(float tx1, float tx2, float wx1, float wx2)
{
	if(w>tx2-tx1)
	{
		int ylimit=(int)(openfiles.size()*dy)-(h>>1);
		if(tabbar_wpy>ylimit)
			tabbar_wpy=ylimit;
		if(tabbar_wpy<0)
			tabbar_wpy=0;
		window_tabbar.set((short)tx1, (short)tx2, 0, h);
		set_region_immediate((short)tx1, (short)tx2, 0, h);
		tabbar_printnames_init();
		for(int k=0;k<(int)openfiles.size();++k)
		{
			auto of=openfiles[k];
			tabbar_printname((float)tx1, k*dy-tabbar_wpy, k);
		}
		window_editor.set((short)wx1, (short)wx2, 0, h);
		set_region_immediate((short)wx1, (short)wx2, 0, h);
	}
}
int					tab_drag_get_h_idx()//duplicate!, see tabbar_get_horizontal_idx
{
	int k=0;
	for(float left=0, right=0;k<(int)tabbar_tabs.size();++k)
	{
		left=right, right=tabbar_tabs[k].right;
		if(tabbar_wpx+mx<(left+right)*0.5f)
			break;
	}
	return k;
}
int					tab_drag_get_v_idx(){return clamp(0, (int)((tabbar_wpy+my+dy*2)/dy), tabbar_tabs.size());}
void				wnd_on_render()
{
	prof_add("Render entry");
	glClear(GL_COLOR_BUFFER_BIT);

	switch(tabbar_position)
	{
	case TABBAR_TOP:	tabbar_draw_horizontal	(0,						dy*2,				dy*2,				(float)h);				break;
	case TABBAR_BOTTOM:	tabbar_draw_horizontal	(h-dy*2,				(float)h,			0,					h-dy*2);				break;
	case TABBAR_LEFT:	tabbar_draw_sidebar		(0,						(float)tabbar_dx,	(float)tabbar_dx,	(float)w);				break;
	case TABBAR_RIGHT:	tabbar_draw_sidebar		((float)(w-tabbar_dx),	(float)w,			0,					(float)(w-tabbar_dx));	break;
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
		text_width=calc_text_cols(*text);
	}
	float
		font_width=dx*font_zoom,
		font_height=dy*font_zoom,//non-tab character dimensions in pixels
		iw=text_width*font_width,
		ih=text_get_nlines(*text)*font_height;//text dimensions in pixels
	
	//decide if need scrollbars
	hscroll.decide(iw+scrollbarwidth>w);
	vscroll.decide(ih+scrollbarwidth>h);

	float cpx=x1+cur->cursor.col*font_width, cpy=y1+cur->cursor.line*font_height, cpy2=y1+cur->selcur.line*font_height;//cursor position in pixels
	if(wnd_to_cursor)
	{
		wnd_to_cursor=false;
		int xpad=(int)(font_width*2*(w>font_width*4)), ypad=(int)(font_height*2*(h>font_height*4));
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
		if(wpx>iw-font_width)
			wpx=(int)(iw-font_width);
	}
	else
		wpx=0;
	if(vscroll.dwidth)
	{
		if(wpy<0)
			wpy=0;
		if(wpy>ih-font_height)
			wpy=(int)(ih-font_height);
	}
	else
		wpy=0;
	//int xend=0+iw-wpx, yend=0+ih-wpy;
	//hscroll.decide_orwith(!hscroll.dwidth&&vscroll.dwidth&&xend>=w-scrollbarwidth);
	//vscroll.decide_orwith(!vscroll.dwidth&&vscroll.dwidth&&yend>=h-74-scrollbarwidth);
	
	size_t nlines=text_get_nlines(*text);
	float tab_origin=(float)(x1-wpx);
	if(cur->rectsel)//rectangular selection
	{
		int rx1, rx2, ry1, ry2;//rect coordinates in characters
		cur->get_rectsel(ry1, ry2, rx1, rx2);
		draw_rectangle_i(//selection rectangle
			x1+rx1*font_width-wpx,
			x1+rx2*font_width-wpx,
			y1+ry1*font_height-wpy,
			y1+ry2*font_height+font_width-wpy,
			colors_text.hi);
		if(ry1==ry2)
			draw_rectangle((float)x1, (float)x2, (float)(cpy-wpy), cpy+font_height-wpy, color_cursorlinebk);//highlight cursor line
		float x=(float)x1, y=(float)y1;
		for(int kl=0;kl<(int)nlines;++kl, y+=font_height)
		{
			size_t len=0;
			auto line=text_get_line(*text, kl, &len);
			if(kl>=ry1&&kl<=ry2)
			{
				int printed_idx=0, printed_cols=0;
				x+=print_line(tab_origin, y-wpy, line, len, tab_origin, font_zoom, rx1, &printed_idx, &printed_cols);
				set_text_colors(colors_selection);
				x+=print_line(tab_origin, y-wpy, line, len, tab_origin, font_zoom, rx2, &printed_idx, &printed_cols);
				set_text_colors(colors_text);
				print_line(tab_origin, y-wpy, line, len, tab_origin, font_zoom, -1, &printed_idx, &printed_cols);
				x=(float)x1;
			}
			else
				print_line(tab_origin, y-wpy, line, len, tab_origin, font_zoom);
		}
		if(cpy<cpy2)//draw cursor
			draw_line((float)(cpx-wpx), (float)(cpy-wpy), (float)(cpx-wpx), cpy2-wpy+font_height, color_cursor);
		else
			draw_line((float)(cpx-wpx), (float)(cpy2-wpy), (float)(cpx-wpx), cpy-wpy+font_height, color_cursor);
	}
	else
	{
		draw_rectangle((float)x1, (float)x2, (float)(cpy-wpy), cpy+font_height-wpy, color_cursorlinebk);//highlight cursor line
		Bookmark i(0, 0, 0), f((int)nlines-1, (int)text_get_len(*text, nlines-1), 0);
		float x=tab_origin, y=(float)(y1-wpy);
		if(cur->selection_exists())//normal selection
		{
			Bookmark sel_i, sel_f;
			cur->get_selection(sel_i, sel_f);

			int line=0, col=0;//must be initialized
			print_text(tab_origin, x, y, *text, i, sel_i, font_zoom, &line, &col);
			set_text_colors(colors_selection);
			print_text(tab_origin, x, y, *text, sel_i, sel_f, font_zoom, &line, &col);
			set_text_colors(colors_text);
			print_text(tab_origin, x, y, *text, sel_f, f, font_zoom, &line, &col);
		}
		else//no selection
			print_text(tab_origin, x, y, *text, i, f, font_zoom);
		draw_line((float)(cpx-wpx), (float)(cpy-wpy), (float)(cpx-wpx), cpy-wpy+font_height, color_cursor);//draw cursor
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
			scrollbar_scroll(wpx, (int)iw, x2-x1-vsbw, x2-x1-(scrollbarwidth<<1)-vsbw, hscroll.s0, hscroll.m_start, mx, font_zoom, hscroll.start, hscroll.size);
		else
			scrollbar_slider(wpx, (int)iw, x2-x1-vsbw, x2-x1-(scrollbarwidth<<1)-vsbw, font_zoom, hscroll.start, hscroll.size);
		draw_rectangle_i(x1+scrollbarwidth+hscroll.start, x1+scrollbarwidth+hscroll.start+hscroll.size, y2-scrollbarwidth, y2, color_slider);//horizontal slider
		y2-=scrollbarwidth;
	}
	if(vscroll.dwidth)
	{
		draw_vscrollbar(x2-scrollbarwidth, scrollbarwidth, y1, y2, wpy, (int)ih, vscroll.s0, vscroll.m_start, font_zoom, vscroll.start, vscroll.size, DRAG_VSCROLL);
		x2-=scrollbarwidth;
	}
	prof_add("scrollbars");

	set_region_immediate(0, w, 0, h);
#ifdef DEBUG_CURSOR
	debug_print_cols((float)(x1-wpx), y1-font_height*2, font_zoom, (int)(w/font_width)+1, 10);//
	debug_print_cols((float)(x1-wpx), y1-font_height, font_zoom, (int)(w/font_width)+1, 1);//
#endif
	switch(drag)
	{
	case DRAG_TAB:
		{
			int marker_size=10;
			float xtip=0, ytip=0;
			const char *buf=nullptr;
			int len=0;
			float msg_width=tabbar_printname(0, 0, drag_tab_idx, true, true, &buf, &len);
			switch(tabbar_position)
			{
			case TABBAR_TOP:
			case TABBAR_BOTTOM:
				{
					int k=tab_drag_get_h_idx();
					float xmarker=k?tabbar_tabs[k-1].right:0;
					float my1, my2;
					if(tabbar_position==TABBAR_TOP)
						my1=dy*2+1, my2=my1+marker_size, xtip=mx, ytip=(float)(my+20);
					else
						my1=h-dy*2-1, my2=my1-marker_size, xtip=mx, ytip=(float)(my-dy-20);
					draw_line(xmarker-tabbar_wpx, my1, xmarker-tabbar_wpx-marker_size, my2, color_cursor);
					draw_line(xmarker-tabbar_wpx, my1, xmarker-tabbar_wpx+marker_size, my2, color_cursor);
				}
				break;
			case TABBAR_LEFT:
			case TABBAR_RIGHT:
				{
					float ymarker=tab_drag_get_v_idx()*dy;
					float mx1, mx2;
					if(tabbar_position==TABBAR_LEFT)
						mx1=(float)(tabbar_dx+1), mx2=mx1+marker_size, xtip=(float)(mx+20), ytip=my;
					else
						mx1=(float)(w-tabbar_dx-1), mx2=mx1-marker_size, xtip=(float)(mx-msg_width-20), ytip=my;
					draw_line(mx1, ymarker-tabbar_wpy, mx2, ymarker-tabbar_wpy-marker_size, color_cursor);
					draw_line(mx1, ymarker-tabbar_wpy, mx2, ymarker-tabbar_wpy+marker_size, color_cursor);
				}
				break;
			}
			draw_rectangle((float)xtip, xtip+msg_width+4, (float)ytip, ytip+dy+4, 0x40FFFFFF);
			set_text_colors(colors_selection);
			print_line(xtip+2, ytip+2, buf, len, 0, 1);
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
				float
					Lx=x1+r.x1*font_width-wpx, Ty=y1+r.y1*font_height-wpy,
					Rx=x1+r.x2*font_width-wpx, By=y1+r.y2*font_height-wpy;
				draw_rectangle_hollow(Lx, Rx, Ty, By, 0xFF808080);
				draw_line(Lx, Ty, Lx, By, color_cursor);
			}
			else
			{
				float dstpx=x1+cur->cursor.col*font_width-wpx, dstpy=y1+cur->cursor.line*font_height-wpy;
				draw_line(dstpx, dstpy, dstpx, dstpy+font_height, color_cursor);
			}
		}
		break;
	}

	//for(int k=0;k<w;k+=2)//rounding bias test
	//{
	//	int color=k&0xFF;
	//	//draw_line(k, 0, k, h, 0xFF000000|color<<16|color<<8|color);
	//	//draw_line(k, k, 0, h, 0xFF000000|color<<16|color<<8|color);
	//	draw_rectangle(k, k+1, 0, h, 0xFF000000|color<<16|color<<8|color);
	//}
#if 0
	int counter=w;
	//int counter=text_get_len(*text, 0);
	//static int counter=0;
	//++counter;
	//float sdf_zoom=10*cos(0.005f*counter);
	//float sdf_zoom=1.f/4;
	float sdf_zoom=(float)font_zoom*0.5f;
	float font_height=sdf_zoom*dy*16/12;
	int x0=0, y0=h>>2;
	//int x0=w>>3, y0=h>>2;
	print_sdf_line(x0, y0, " !\"#$%&\'()*+,-./", 16, 0, sdf_zoom);
	print_sdf_line(x0, y0+font_height, "0123456789:;<=>?", 16, 0, sdf_zoom);
	print_sdf_line(x0, y0+font_height*2, "@ABCDEFGHIJKLMNO", 16, 0, sdf_zoom);
	print_sdf_line(x0, y0+font_height*3, "PQRSTUVWXYZ[\\]^_", 16, 0, sdf_zoom);
	print_sdf_line(x0, y0+font_height*4, "`abcdefghijklmno", 16, 0, sdf_zoom);
	print_sdf_line(x0, y0+font_height*5, "pqrstuvwxyz{|}~", 16, 0, sdf_zoom);
#endif

	//for(int k=0;k<1000;++k)
	//{
	//	int zoom=1+rand()%3;
	//	print(zoom, 0, rand()%w, rand()%h, "Sample Text %d", zoom);
	//}
	//print(1, 0, 0, 0, "Hello. Sample Text. What\'s going on???");
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
				text2str(*text, str);
				if(filename&&save_text_file(filename, str))
					openfiles[tab_idx].m_filename=filename;
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
		cursor_at_mouse(*text, mx, my, cur->cursor, !cur->rectsel, 0.5f);
		text_push_checkpoint(*text, ACT_RELOCATE_CURSOR, cur, sizeof(*cur));
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
		//if(drag_cursor.rectsel)
		//{
		//	int sx=window_editor.x1+cur->selcur.col*dx*font_zoom-wpx,
		//		sy=window_editor.y1+cur->selcur.line*dy*font_zoom-wpy;
		//	auto r=drag_cursor.get_rectsel();
		//	int rx=window_editor.x1+r.x2*dx*font_zoom-wpx,
		//		by=window_editor.y1+r.y2*dy*font_zoom-wpy;
		//	cursor_at_mouse(*text, mx-(start_mx-cpx), my-(start_my-cpy), cur->cursor, 0.5f);
		//}
		//else
			cursor_at_mouse(*text, mx, my, cur->cursor, !cur->rectsel, 0.5f);
		return true;
	}
	return false;
}
bool				wnd_on_mousewheel(bool mw_forward)
{
	if(is_ctrl_down())//change zoom
	{
		float dzoom=sdf_active?sdf_dzoom:2;
		if(mw_forward)
		{
			if(font_zoom<zoom_max)
				font_zoom*=dzoom, wpx=(int)(wpx*dzoom), wpy=(int)(wpy*dzoom);
		}
		else if(font_zoom>zoom_min)
			font_zoom/=dzoom, wpx=(int)(wpx/dzoom), wpy=(int)(wpy/dzoom);
	}
	else//scroll
	{
		switch(tabbar_position)
		{
		case TABBAR_TOP:
			if(h>dy*2&&(my<dy*2||drag==DRAG_TAB)&&tabbar_tabs.back().right>w)
			{
				//int k=tabbar_get_horizontal_idx(1);//npp: skip one tab
				if(mw_forward)//predictable
					tabbar_wpx=(int)(tabbar_wpx-dx*8);
				else
					tabbar_wpx=(int)(tabbar_wpx+dx*8);
				return true;
			}
			break;
		case TABBAR_BOTTOM:
			if(h>dy*2&&(my>h-dy*2||drag==DRAG_TAB)&&tabbar_tabs.back().right>w)
			{
				if(mw_forward)
					tabbar_wpx=(int)(tabbar_wpx-dx*8);
				else
					tabbar_wpx=(int)(tabbar_wpx+dx*8);
				return true;
			}
			break;
		case TABBAR_LEFT:
			if(w>tabbar_dx&&(mx<tabbar_dx||drag==DRAG_TAB)&&(int)openfiles.size()*dy>h)
			{
				if(mw_forward)
					tabbar_wpy=(int)(tabbar_wpy-dy*3);
				else
					tabbar_wpy=(int)(tabbar_wpy+dy*3);
				return true;
			}
			break;
		case TABBAR_RIGHT:
			if(w>tabbar_dx&&(mx>w-tabbar_dx||drag==DRAG_TAB)&&(int)openfiles.size()*dy>h)
			{
				if(mw_forward)
					tabbar_wpy=(int)(tabbar_wpy-dy*3);
				else
					tabbar_wpy=(int)(tabbar_wpy+dy*3);
				return true;
			}
			break;
		}

		if(is_shift_down())
		{
			if(mw_forward)
				wpx=(int)(wpx-dy*font_zoom*3);
			else
				wpx=(int)(wpx+dy*font_zoom*3);
		}
		else
		{
			if(mw_forward)
				wpy=(int)(wpy-dy*font_zoom*3);
			else
				wpy=(int)(wpy+dy*font_zoom*3);
		}
	}
	return true;
}
bool				wnd_on_zoomin()
{
	if(font_zoom<zoom_max)
	{
		float dzoom=sdf_active?sdf_dzoom:2;
		font_zoom*=dzoom, wpx=(int)(wpx*dzoom), wpy=(int)(wpy*dzoom);
		return true;
	}
	return false;
}
bool				wnd_on_zoomout()
{
	if(font_zoom>zoom_min)
	{
		float dzoom=sdf_active?sdf_dzoom:2;
		font_zoom/=dzoom, wpx=(int)(wpx/dzoom), wpy=(int)(wpy/dzoom);
		return true;
	}
	return false;
}
bool				wnd_on_reset_zoom()
{
	wpx=(int)(wpx/font_zoom), wpy=(int)(wpy/font_zoom), font_zoom=1;
	return true;
}
bool				drag_selection_click(DragType drag_type)//checks if you clicked on the selection to drag it
{
	if(cur->selection_exists())
	{
		Bookmark click;
		bool hit=cursor_at_mouse(*text, mx, my, click, !cur->rectsel, 0);
		if(hit)
		{
			if(cur->rectsel)
			{
				auto r=cur->get_rectsel();
				if(click.line>=r.y1&&click.line<=r.y2&&click.col>=r.x1&&click.col<=r.x2)
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
	start_mx=mx, start_my=my;
	bool ctrldown=is_ctrl_down();
	if(drag_selection_click(ctrldown?DRAG_COPY_SELECTION:DRAG_MOVE_SELECTION))
		return;
	if(doubleclick||ctrldown)//select word
	{
		cursor_at_mouse(*text, mx, my, cur->cursor, true, 0.5f);
		cur->selcur=cur->cursor;
		char initial=group_char(cur->cursor.dereference_idx(*text));
		int nlines=text_get_nlines(*text);
		Bookmark start(0, 0, 0), finish(nlines-1, text_get_len(*text, nlines-1), 0);

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
			if(cursor_at_mouse(*text, mx, my, temp, false, 0.5f))
				drag=DRAG_RECT, cur->cursor=temp;
			if(!is_shift_down())
				cur->selcur=cur->cursor;
		}
		else//place cursor / normal selection
		{
			drag=DRAG_SELECT;
			cursor_at_mouse(*text, mx, my, cur->cursor, true, 0.5f);
			if(!is_shift_down())
				cur->selcur=cur->cursor, cur->selcur=cur->cursor;
		}
	}
	text_action_end(*text);
	text_push_checkpoint(*text, ACT_RELOCATE_CURSOR, cur, sizeof(*cur));
	//text_action_end(*text, ACT_RELOCATE_CURSOR);
	//text_push_checkpoint(*text, ACT_RELOCATE_CURSOR, 1, cur, sizeof(*cur));
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
		if(h>dy*2)
		{
			if(my<dy*2)
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
			x1=0, x2=w, y1=(int)(dy*2), y2=h;
		}
		break;
	case TABBAR_BOTTOM:
		if(h>dy*2)
		{
			if(my>h-dy*2)
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
			x1=0, x2=w, y1=0, y2=(int)(h-dy*2);
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
			Text selection=0;
			//Cursor c0=drag_cursor;
			if(drag_cursor.rectsel)
			{
				rectsel_copy(*text, drag_cursor, selection);
				auto r=drag_cursor.get_rectsel();
				if(drag==DRAG_MOVE_SELECTION)
				{
					size_t len=0;
					auto line=text_get_line(*text, r.y1, &len);
					int before=idx2col(line, len, 0);

					text_erase_rect(*text, r.y1, r.y2, r.x1, r.x2, ACT_MOVE_SELECTION);
					
					line=text_get_line(*text, r.y1, &len);
					int after=idx2col(line, len, 0);

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
					dstcol=cur->cursor.col>=r.x1&&cur->cursor.col<r.x2 ? r.x1 : r.x1+cur->cursor.col-cur->selcur.col;//TODO: fix rectsel drag
#endif
				if(dstline<0)
					dstline=0;
				if(dstcol<0)
					dstcol=0;
				rectsel_insert(selection, dstline, dstcol, *text, ACT_MOVE_SELECTION);

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
					text_erase(*text, drag_cursor.cursor.line, drag_cursor.cursor.idx, drag_cursor.selcur.line, drag_cursor.selcur.idx, ACT_MOVE_SELECTION);
					if(relocate)
						cur->cursor.set_absidx(*text, absidx-selsize);
				}
				selection_insert(selection, cur->cursor.line, cur->cursor.idx, *text, ACT_MOVE_SELECTION);

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
			text_action_end(*text);
			text_push_checkpoint(*text, ACT_RELOCATE_CURSOR, cur, sizeof(*cur));
			//text_action_end(*text, ACT_MOVE_SELECTION);
			//text_push_checkpoint(*text, ACT_MOVE_SELECTION, 1, cur, sizeof(*cur));
		}
		dimensions_known=false;
		break;
	}
	mouse_release();
	drag=DRAG_NONE;
	return true;
}
bool				wnd_on_mbuttondown(bool doubleclick)
{
	int k=0;
	switch(tabbar_position)//close tab
	{
	case TABBAR_TOP:
		if(h>dy*2&&my<dy*2)
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
		if(h>dy*2&&my>h-dy*2)
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
bool				wnd_on_toggle_renderer()
{
	static float dx0=0, dy0=0;
	if(sdf_available)
	{
		sdf_active=!sdf_active;
		if(sdf_active)
		{
			dx0=dx, dx=sdf_dx*16/sdf_dy;
			dy0=dy, dy=16;
			zoom_min=0.25, zoom_max=64;
		}
		else
		{
			dx=dx0;
			dy=dy0;
			if(font_zoom<1)
				font_zoom=1;
			else
				font_zoom=(float)(1<<floor_log2((int)font_zoom));
			zoom_min=1, zoom_max=32;
		}
		return true;
	}
	return false;
}
bool				wnd_on_select_all()
{
	cur->selcur.setzero(), cur->cursor.setend(*text), cur->rectsel=false;
	text_action_end(*text);
	text_push_checkpoint(*text, ACT_RELOCATE_CURSOR, cur, sizeof(*cur));
	//text_action_end(*text, ACT_RELOCATE_CURSOR);
	//text_push_checkpoint(*text, ACT_RELOCATE_CURSOR, 1, cur, sizeof(*cur));
	wnd_to_cursor=true;
	return true;
}
bool				wnd_on_deselect()
{
	drag=DRAG_NONE;
	cur->cursor.update_idx(*text);
	cur->selcur=cur->cursor, cur->rectsel=false;
	text_action_end(*text);
	text_push_checkpoint(*text, ACT_RELOCATE_CURSOR, cur, sizeof(*cur));
	//text_action_end(*text, ACT_RELOCATE_CURSOR);
	//text_push_checkpoint(*text, ACT_RELOCATE_CURSOR, 1, cur, sizeof(*cur));
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

bool				wnd_on_type(char character)
{
	//auto action=text_get_last_action(*text);
	//if(action==ACTION_INSERT)
	//	text_pop_checkpoint(*text);
	//else if(action!=ACTION_CHECKPOINT)
	//	text_push_checkpoint(*text, cur, sizeof(*cur));
	if(cur->selection_exists())
	{
		bool multiline_selection=cur->cursor.line!=cur->selcur.line;
		if(character=='\t'&&multiline_selection)//indentation
		{
			if(cur->rectsel)//indent rectsel
			{
				if(is_shift_down())//shift tab
					indent_rectsel_back(*cur);
				else//tab
					indent_rectsel_forward(*cur);
			}
			else//indent selection
			{
				if(is_shift_down())//shift tab
					indent_selection_back(*cur);
				else//tab
					indent_selection_forward(*cur);
			}
			text_action_end(*text);
			text_push_checkpoint(*text, ACT_RELOCATE_CURSOR, cur, sizeof(*cur));
			//text_action_end(*text, ACT_INDENT);
			//text_push_checkpoint(*text, cur, sizeof(*cur));
			dimensions_known=false;
			set_title();
			return true;
		}//else erase selection and continue below
		general_selection_erase(ACT_TYPE);
	}
	if(cur->rectsel)//tall cursor
	{
		auto r=cur->get_rectsel();
		if(character=='\n')//remove tall cursor, continue below to insert newline
		{
			cur->cursor.set_gui(*text, r.y2, (float)r.x2, true, 0.5f);
			cur->selcur=cur->cursor;
		}
		else//rectangular typing
		{
			//while typing a character with rectsel, if a tab causes misalign:
			//	either	1) insert (tab_count-mod(col, tab_count)) spaces before the character in lines without tab, so that all typed characters are aligned (probably not what was intended)
			//	or		2) relocate the cursor to tab (unpredictable)
			//	or		3) (best solution) break (replace) each encountered tab into spaces
			for(int kl=r.y1;kl<=r.y2;++kl)
				line_insert(*text, kl, r.x1, r.x2, &character, 1, ACT_TYPE);
			if(cur->cursor.col<cur->selcur.col)//TODO: optimize this
			{
				cur->cursor.update_idx(*text);
				++cur->cursor.idx;
				cur->cursor.update_col(*text);
				cur->selcur.col=cur->cursor.col;
				cur->selcur.update_idx(*text);
			}
			else
			{
				cur->selcur.update_idx(*text);
				++cur->selcur.idx;
				cur->selcur.update_col(*text);
				cur->cursor.col=cur->selcur.col;
				cur->cursor.update_idx(*text);
			}
			text_push_checkpoint(*text, ACT_RELOCATE_CURSOR, cur, sizeof(*cur));
			//text_action_end(*text);
			//text_action_end(*text, ACT_TYPE);
			//text_push_checkpoint(*text, cur, sizeof(*cur));
			dimensions_known=false;
			return true;
		}
	}

	text_insert1(*text, cur->cursor.line, cur->cursor.idx, character, ACT_TYPE);

	cur->cursor.increment_idx(*text);
	cur->cursor.update_col(*text);
	cur->selcur=cur->cursor;
	
	text_push_checkpoint(*text, ACT_RELOCATE_CURSOR, cur, sizeof(*cur));
	//text_action_end(*text);
	//text_action_end(*text, ACT_TYPE);
	//text_push_checkpoint(*text, cur, sizeof(*cur));
	dimensions_known=false;
	set_title();
	return true;
}
bool				wnd_on_deletechar()
{
	//auto action=text_get_last_action(*text);
	//if(action==ACTION_REMOVE)
	//	text_pop_checkpoint(*text);
	//else if(action!=ACTION_CHECKPOINT)
	//	text_push_checkpoint(*text, cur, sizeof(*cur));

	if(cur->selection_exists())//delete selection
		general_selection_erase(ACT_ERASE);
	else if(cur->rectsel)//tall cursor delete
	{
		auto r=cur->get_rectsel();
		for(int kl=r.y1;kl<=r.y2;++kl)
		{
			size_t len=0;
			auto line=text_get_line(*text, kl, &len);
			int idx1=0, c1=0;
			//int c0=0;
			bool line_OOB=col2idx(line, len, 0, 0, 0, (float)r.x1, &idx1, &c1, 0.5f);
			if(!line_OOB&&idx1<(int)len)
			{
				//if(line[idx1]=='\t')//break tab into spaces
				//{
				//	c0=idx2col(line, idx1, 0);
				//	text_replace(*text, kl, idx1, idx1+1, " ", 1, c1-c0-1, ACT_ERASE);
				//}
				//else
					text_replace(*text, kl, idx1, idx1+1, 0, 0, 0, ACT_ERASE);
			}
		}
	}
	else//delete
		text_erase1_del(*text, cur->cursor.line, cur->cursor.idx, ACT_ERASE);
	
	text_push_checkpoint(*text, ACT_RELOCATE_CURSOR, cur, sizeof(*cur));
	//text_action_end(*text);
	//text_action_end(*text, ACT_ERASE);
	//text_push_checkpoint(*text, cur, sizeof(*cur));
	dimensions_known=false;
	set_title();
	return true;
}
bool				wnd_on_backspace()
{
	//auto action=text_get_last_action(*text);
	//if(action==ACTION_REMOVE)
	//	text_pop_checkpoint(*text);
	//else if(action!=ACTION_CHECKPOINT)
	//	text_push_checkpoint(*text, cur, sizeof(*cur));

	if(cur->selection_exists())//erase selection
		general_selection_erase(ACT_ERASE);
	else if(cur->rectsel)//tall cursor backspace
	{
		auto r=cur->get_rectsel();
		for(int kl=r.y1;kl<=r.y2;++kl)
		{
			size_t len=0;
			auto line=text_get_line(*text, kl, &len);
			int idx1=0, c1=0, c0=0;
			bool line_OOB=col2idx(line, len, 0, 0, 0, (float)r.x1, &idx1, &c1, 0.5f);
			if(!line_OOB&&idx1>0)
			{
				--idx1;
				if(line[idx1]=='\t')//break tab into spaces
				{
					c0=idx2col(line, idx1, 0);
					text_replace(*text, kl, idx1, idx1+1, " ", 1, c1-c0-1, ACT_ERASE);
				}
				else
					text_replace(*text, kl, idx1, idx1+1, 0, 0, 0, ACT_ERASE);
			}
		}
		--cur->cursor.col;
		--cur->selcur.col;
		cur->cursor.update_idx(*text);
		cur->selcur.update_idx(*text);
	}
	else//backspace
	{
		int l0=cur->cursor.line, idx0=cur->cursor.idx;
		cur->cursor.decrement_idx(*text);
		cur->cursor.update_col(*text);
		cur->selcur=cur->cursor;
		text_erase1_bksp(*text, l0, idx0, ACT_ERASE);
	}
	text_push_checkpoint(*text, ACT_RELOCATE_CURSOR, cur, sizeof(*cur));
	//text_action_end(*text);
	//text_action_end(*text, ACT_ERASE);
	//text_push_checkpoint(*text, cur, sizeof(*cur));
	dimensions_known=false;
	set_title();
	return true;
}
bool				wnd_on_copy(bool cut)
{
	if(cur->selection_exists())
	{
		std::string str;
		//Text selection=text_create(TEXT_NO_HISTORY);
		Text selection=0;
		if(cur->rectsel)
			rectsel_copy(*text, *cur, selection);
		else
			selection_copy(*text, *cur, selection);
		text2str(selection, str);
		copy_to_clipboard_c(str.c_str(), str.size());
		if(cut)
		{
			general_selection_erase(ACT_CUT);
			text_action_end(*text);
			text_push_checkpoint(*text, ACT_RELOCATE_CURSOR, cur, sizeof(*cur));
			//text_action_end(*text, ACT_CUT);
			//text_push_checkpoint(*text, cur, sizeof(*cur));
			dimensions_known=false;
			set_title();
		}
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
	int knl=0;
	if(cur->rectsel)
	{
		Text src=0;
		str2text(str, len, src, TEXT_NO_HISTORY, 0, 0);
		auto r=cur->get_rectsel();
		size_t nlines=text_get_nlines(src);
		if(nlines==1)//single line rectsel paste
		{
			for(int kl=r.y1;kl<=r.y2;++kl)
				line_insert(*text, kl, r.x1, r.x2, str, len, ACT_PASTE);
		}
		else//multiline rectsel paste
		{
			int ntabs=r.x1/tab_count, nspaces=mod(r.x1, tab_count);
			for(int kls=0, kld=r.y1;kls<(int)nlines;++kls, ++kld)
			{
				size_t len=0;
				auto line=text_get_line(src, kls, &len);
				line_insert(*text, kld, r.x1, r.x2, line, len, ACT_PASTE);
			}
		}
	}
	else
	{
		general_selection_erase(ACT_PASTE);
		text_insert(*text, cur->cursor.line, cur->cursor.idx, str, len, &cur->cursor.line, &cur->cursor.idx, ACT_PASTE);
	}
	delete[] str;
	cur->cursor.update_col(*text);
	if(!is_shift_down())
		cur->deselect(*text, 0);
	text_action_end(*text);
	text_push_checkpoint(*text, ACT_RELOCATE_CURSOR, cur, sizeof(*cur));
	//text_action_end(*text, ACT_RELOCATE_CURSOR);
	//text_push_checkpoint(*text, cur, sizeof(*cur));
	dimensions_known=false;
	set_title();
	return true;
}
void				line_setcase(Text &text, int l0, const char *line, int idx1, int idx2, std::string &temp, bool upper)
{
	temp.assign(line+idx1, line+idx2);
	if(upper)
	{
		for(int k=0;k<(int)temp.size();++k)
			temp[k]=toupper(temp[k]);
	}
	else
	{
		for(int k=0;k<(int)temp.size();++k)
			temp[k]=tolower(temp[k]);
	}
	text_replace(text, l0, idx1, idx2, temp.c_str(), temp.size(), 1, ACT_CHANGE_CASE);
}
bool				wnd_on_setcase(bool upper)
{
	if(cur->selection_exists())
	{
		std::string temp;
		bool modified=false;
		if(cur->rectsel)
		{
			auto r=cur->get_rectsel();
			for(int kl=r.y1;kl<=r.y2;++kl)
			{
				size_t len=0;
				auto line=text_get_line(*text, kl, &len);
				int idx1, c1, idx2, c2;
				col2idx(line, len, 0, 0, 0, (float)r.x1, &idx1, &c1, 0.5f);
				col2idx(line, len, 0, idx1, c1, (float)r.x2, &idx2, &c2, 0.5f);
				if(idx1<idx2)
				{
					modified=true;
					line_setcase(*text, kl, line, idx1, idx2, temp, upper);
				}
			}
		}
		else
		{
			Bookmark i, f;
			cur->get_selection(i, f);
			for(int kl=i.line;kl<=f.line;++kl)
			{
				size_t len=0;
				auto line=text_get_line(*text, kl, &len);
				int idx1=kl==i.line?i.idx:0, idx2=kl==f.line?f.idx:len;
				if(idx1<idx2)
				{
					modified=true;
					line_setcase(*text, kl, line, idx1, idx2, temp, upper);
				}
			}
		}
		if(modified)
		{
			text_action_end(*text);
			set_title();
			return true;
		}
	}
	return false;
}

bool				wnd_on_undo()
{
	text_undo(*text, cur, 0);
	dimensions_known=false;
	set_title();
	return true;
}
bool				wnd_on_redo()
{
	text_redo(*text, cur, 0);
	dimensions_known=false;
	set_title();
	return true;
}
bool				wnd_on_clear_hist()
{
	if(!messagebox_okcancel("Teletext", "Are you sure you want to clear the undo/redo history?"))
	{
		text_clear_history(*text);
		set_title();
	}
	return true;
}

bool				wnd_on_scroll_up_key()
{
	wpy=(int)(wpy-dy*font_zoom);
	return true;
}
bool				wnd_on_scroll_down_key()
{
	wpy=(int)(wpy+dy*font_zoom);
	return true;
}
bool				wnd_on_pageup()
{
	float fontH=dy*font_zoom;//
	float delta=h-fontH*4;
	if(delta<fontH)
		delta=fontH;
	wpy=(int)(wpy-delta);
	return true;
}
bool				wnd_on_pagedown()
{
	float fontH=dy*font_zoom;//
	float delta=h-fontH*3;
	if(delta<fontH)
		delta=fontH;
	wpy=(int)(wpy+delta);
	return true;
}
bool				wnd_on_cursor_up()
{
	bool shiftdown=is_shift_down(), altdown=is_alt_down();
	//if(!cur->selection_exists()||shiftdown||altdown)
		cur->cursor.jump_vertical(*text, -1);
	cur->rectsel=altdown;
	if(!shiftdown&&!altdown)
		cur->deselect(*text, -1);
	text_action_end(*text);
	text_push_checkpoint(*text, ACT_RELOCATE_CURSOR, cur, sizeof(*cur));
	//text_action_end(*text, ACT_RELOCATE_CURSOR);
	//text_push_checkpoint(*text, cur, sizeof(*cur));
	wnd_to_cursor=true;
	return true;
}
bool				wnd_on_cursor_down()
{
	bool shiftdown=is_shift_down(), altdown=is_alt_down();
	//if(!cur->selection_exists()||shiftdown||altdown)
		cur->cursor.jump_vertical(*text, 1);
	cur->rectsel=altdown;
	if(!shiftdown&&!altdown)
		cur->deselect(*text, 1);
	text_action_end(*text);
	text_push_checkpoint(*text, ACT_RELOCATE_CURSOR, cur, sizeof(*cur));
	//text_action_end(*text, ACT_RELOCATE_CURSOR);
	//text_push_checkpoint(*text, cur, sizeof(*cur));
	wnd_to_cursor=true;
	return true;
}
bool				wnd_on_cursor_left()
{
	bool shiftdown=is_shift_down(), altdown=is_alt_down();
	if(!cur->selection_exists()||shiftdown||altdown)
	{
		cur->cursor.decrement_idx(*text);
		cur->cursor.update_col(*text);
		cur->rectsel=altdown;
	}
	if(!shiftdown&&!altdown)
		cur->deselect(*text, -1);
	text_action_end(*text);
	text_push_checkpoint(*text, ACT_RELOCATE_CURSOR, cur, sizeof(*cur));
	//text_action_end(*text, ACT_RELOCATE_CURSOR);
	//text_push_checkpoint(*text, cur, sizeof(*cur));
	wnd_to_cursor=true;
	return true;
}
bool				wnd_on_cursor_right()
{
	bool shiftdown=is_shift_down(), altdown=is_alt_down();
	if(!cur->selection_exists()||shiftdown||altdown)
	{
		cur->cursor.increment_idx(*text);
		cur->cursor.update_col(*text);
		cur->rectsel=altdown;
	}
	if(!shiftdown&&!altdown)
		cur->deselect(*text, 1);
	text_action_end(*text);
	text_push_checkpoint(*text, ACT_RELOCATE_CURSOR, cur, sizeof(*cur));
	//text_action_end(*text, ACT_RELOCATE_CURSOR);
	//text_push_checkpoint(*text, cur, sizeof(*cur));
	wnd_to_cursor=true;
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

	if(!is_shift_down()&&!is_alt_down())
		cur->deselect(*text, -1);
	text_action_end(*text);
	text_push_checkpoint(*text, ACT_RELOCATE_CURSOR, cur, sizeof(*cur));
	//text_action_end(*text, ACT_RELOCATE_CURSOR);
	//text_push_checkpoint(*text, cur, sizeof(*cur));
	wnd_to_cursor=true;
	return true;
}
bool				wnd_on_skip_word_right()
{
	size_t nlines=text_get_nlines(*text);
	Bookmark finish(nlines-1, text_get_len(*text, nlines-1), 0);
	char initial=group_char(cur->cursor.dereference_idx(*text));
	cur->cursor.increment_idx(*text);
	while(cur->cursor<finish)
	{
		if(group_char(cur->cursor.dereference_idx(*text))!=initial)
			break;
		cur->cursor.increment_idx(*text);
	}
	cur->cursor.update_col(*text);

	if(!is_shift_down()&&!is_alt_down())
		cur->deselect(*text, 1);
	text_action_end(*text);
	text_push_checkpoint(*text, ACT_RELOCATE_CURSOR, cur, sizeof(*cur));
	//text_action_end(*text, ACT_RELOCATE_CURSOR);
	//text_push_checkpoint(*text, cur, sizeof(*cur));
	wnd_to_cursor=true;
	return true;
}
bool				wnd_on_goto_line_start()
{
	cur->cursor.set_linestart();
	if(!is_shift_down()&&!is_alt_down())
		cur->deselect(*text, 0);
	text_action_end(*text);
	text_push_checkpoint(*text, ACT_RELOCATE_CURSOR, cur, sizeof(*cur));
	//text_action_end(*text, ACT_RELOCATE_CURSOR);
	//text_push_checkpoint(*text, cur, sizeof(*cur));
	wnd_to_cursor=true;
	return true;
}
bool				wnd_on_goto_line_end()
{
	cur->cursor.set_lineend(*text);
	if(!is_shift_down()&&!is_alt_down())
		cur->deselect(*text, 0);
	text_action_end(*text);
	text_push_checkpoint(*text, ACT_RELOCATE_CURSOR, cur, sizeof(*cur));
	//text_action_end(*text, ACT_RELOCATE_CURSOR);
	//text_push_checkpoint(*text, cur, sizeof(*cur));
	wnd_to_cursor=true;
	return true;
}
bool				wnd_on_goto_file_start()
{
	cur->cursor.setzero();
	if(!is_shift_down()&&!is_alt_down())
		cur->deselect(*text, 0);
	text_action_end(*text);
	text_push_checkpoint(*text, ACT_RELOCATE_CURSOR, cur, sizeof(*cur));
	//text_action_end(*text, ACT_RELOCATE_CURSOR);
	//text_push_checkpoint(*text, cur, sizeof(*cur));
	wnd_to_cursor=true;
	return true;
}
bool				wnd_on_goto_file_end()
{
	cur->cursor.setend(*text);
	if(!is_shift_down()&&!is_alt_down())
		cur->deselect(*text, 0);
	text_action_end(*text);
	text_push_checkpoint(*text, ACT_RELOCATE_CURSOR, cur, sizeof(*cur));
	//text_action_end(*text, ACT_RELOCATE_CURSOR);
	//text_push_checkpoint(*text, cur, sizeof(*cur));
	wnd_to_cursor=true;
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
				f.m_filename=file.filename;
				str2text(str.c_str(), str.size(), f.m_text, TEXT_NORMAL, &f.m_cur, sizeof(f.m_cur));
				text_mark_saved(f.m_text);
				int idx=file.tab_idx;
				if(idx>(int)openfiles.size())//crash guard
					idx=openfiles.size();
				openfiles.insert(openfiles.begin()+idx, std::move(f));
				dimensions_known=false;
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
		std::string str;
		if(open_text_file(filename2, str))
		{
			openfiles.push_back(TextFile());
			auto &of=openfiles.back();
			of.m_filename=filename2;
			str2text(str.c_str(), str.size(), of.m_text, TEXT_NORMAL, &of.m_cur, sizeof(of.m_cur));
			text_mark_saved(of.m_text);
			dimensions_known=false;
			tabbar_calc_positions();
			tabs_switchto(openfiles.size()-1);
			tabbar_scroll_to(current_file);
			return true;
		}
		else
			messagebox("Error", "Failed to open:\n%s", filename2);
	}
	return false;
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
			text_action_end(*text);
			//text_action_end(*text, ACT_MOVE_SELECTION);//placeholder
			//text_push_checkpoint(*text, cur, sizeof(*cur));
		}
	}
	else
	{
		std::string str;
		text2str(*text, str);
		save_text_file(filename->c_str(), str);
		text_action_end(*text);
		//text_action_end(*text, ACT_MOVE_SELECTION);
		//text_push_checkpoint(*text, cur, sizeof(*cur));
	}
	set_title();
	text_mark_saved(openfiles[current_file].m_text);
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