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
#include			"stb_image.h"//https://github.com/nothings/stb
const char			file[]=__FILE__;
char				g_buf[G_BUF_SIZE]={};
int					w=0, h=0;
short				mx=0, my=0, dx=0, dy=0, tab_count=4;

void				display_help()
{
	messagebox("Controls",
		"Ctrl+O:\t\tOpen\n"
		"Ctrl+S:\t\tSave\n"
		"Ctrl+Shift+S:\tSave as\n"
		"Ctrl+Z:\t\tUndo\n"
		"Ctrl+Y:\t\tRedo\n"
		"Ctrl+X:\t\tCut\n"
		"Ctrl+C:\t\tCopy\n"
		"Ctrl+V:\t\tPaste\n"
		"\n"
		"Ctrl+Up/Down:\tScroll\n"
		"Ctrl+Left/Right:\tSkip word\n"
	//	"Ctrl+-:\t\tPrevious location\n"//TODO
	//	"Ctrl+Shift+-:\tNext location\n"
	//	"Ctrl+F:\t\tFind\n"
	//	"Ctrl+R:\t\tReplace\n"
		"\n"
		"Shift+Arrows/Home/End:\tSelect\n"
		"Ctrl+A:\t\t\tSelect all\n"
		"Alt+Drag:\t\t\tRectangular selection\n"
		"Shift+Drag:\t\tResize selection\n"
		"Ctrl+Shift+Left/Right:\tSelect words\n"
		"Escape:\t\t\tDeselect\n"
		"\n"
		"F4:\tToggle benchmark");
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

//string encoding
void				utf16_to_utf8(const wchar_t *utf16, int len, std::wstring &utf8)
{
	for(int k=0;k<len;++k)
	{
	}
}
void				utf8_to_utf16(const char *utf8, int len, std::wstring &utf16)
{
}

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
void				draw_line_i(short x1, short y1, short x2, short y2, int color){draw_line((float)x1, (float)y1, (float)x2, (float)y2, color);}
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
void				draw_rectangle_i(short x1, short x2, short y1, short y2, int color){draw_rectangle((float)x1, (float)x2, (float)y1, (float)y2, color);}

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
int					calc_width(short x, short y, const char *msg, int msg_length, short tab_origin, short zoom)
{
	if(msg_length<1)
		return 0;
	int msg_width=0, width, printable_count=0, tab_width=tab_count*dx*zoom, w2=dx*zoom;
	for(int k=0;k<msg_length;++k)
	{
		char c=msg[k];
		//if(c=='\n')
		//	break;
		if(c=='\t')
			width=tab_width-mod(x+msg_width-tab_origin, tab_width), c=' ';
		else if(c>=32&&c<0xFF)
			width=w2;
		else
			width=0;
		if(width)
		{
			msg_width+=width;
			++printable_count;
		}
	}
	return msg_width;
}
void				inv_calc_width(short x, short y, const char *msg, int msg_length, short tab_origin, short zoom, int width, int *out_cols, int *out_k)//returns index to fit [msg, msg+index[ in width
{
	if(msg_length<1)
	{
		if(out_cols)
			*out_cols=0;
		if(out_k)
			*out_k=0;
		return;
	}
	int msg_width=0, temp_width, printable_count=0, tab_width=tab_count*dx*zoom, w2=dx*zoom, k=0;
	for(;k<msg_length;++k)
	{
		char c=msg[k];
		if(c=='\n')
			break;
		if(c=='\t')
			temp_width=tab_width-mod(x+msg_width-tab_origin, tab_width), c=' ';
		else if(c>=32&&c<0xFF)
			temp_width=w2;
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
		*out_cols=msg_width/w2;
	if(out_k)
		*out_k=k;
}
int					print_line(short x, short y, const char *msg, int msg_length, short tab_origin, short zoom)
{
	if(msg_length<1)
		return 0;
	float rect[4]={};
	QuadCoords *txc=nullptr;
	int msg_width=0, width, idx, printable_count=0, tab_width=tab_count*dx*zoom, w2=dx*zoom, height=dy*zoom;
	if(y+height<0||y>=h)
		return calc_width(x, y, msg, msg_length, tab_origin, zoom);
	vrtx.resize(msg_length<<4);//vx, vy, txx, txy		x4 vertices/char
	for(int k=0;k<msg_length;++k)
	{
		char c=msg[k];
		if(c=='\t')
			width=tab_width-mod(x+msg_width-tab_origin, tab_width), c=' ';
		else if(c>=32&&c<0xFF)
			width=w2;
		else
			width=0;
		if(width)
		{
			if(x+msg_width+width>=0&&x+msg_width<w)
			{
				toNDC_nobias(float(x+msg_width		), float(y			), rect[0], rect[1]);
				toNDC_nobias(float(x+msg_width+width), float(y+height	), rect[2], rect[3]);//y2<y1
				idx=k<<4;
				txc=font_coords+c-32;
				vrtx[idx   ]=rect[0], vrtx[idx+ 1]=rect[1],		vrtx[idx+ 2]=txc->x1, vrtx[idx+ 3]=txc->y1;//top left
				vrtx[idx+ 4]=rect[0], vrtx[idx+ 5]=rect[3],		vrtx[idx+ 6]=txc->x1, vrtx[idx+ 7]=txc->y2;//bottom left
				vrtx[idx+ 8]=rect[2], vrtx[idx+ 9]=rect[3],		vrtx[idx+10]=txc->x2, vrtx[idx+11]=txc->y2;//bottom right
				vrtx[idx+12]=rect[2], vrtx[idx+13]=rect[1],		vrtx[idx+14]=txc->x2, vrtx[idx+15]=txc->y1;//top right

				++printable_count;
			}
			msg_width+=width;
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
};
int					print_line_rect(short x, short y, const char *msg, int msg_length, short tab_origin, short zoom, short nchars, int &nconsumed)
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

//U64				colors_text=0xFFABABABFF000000;//black on white
U64					colors_text=0xFF000000FFABABAB;//dark mode
U64					colors_selection=0xFFFF0000FFABABAB;
int					color_cursorlinebk=0xFF202020;
U64					colors_cursorline=(u64)color_cursorlinebk<<32|0xFFABABAB;
short				font_zoom=1;//font pixel size
//struct			Cursor
//{
//	bool rectmode;
//	int cursor=0, selcur=0,//text indices
//		ccx=0, ccy=0,//cursor coordinates in the text buffer, in character units
//		scx=0, scy=0;//selcur coordinates in the text buffer, in character units
//};
int					wpx=0, wpy=0,//window position inside the text buffer, in pixels
					cursor=0, selcur=0,//text indices
					ccx=0, ccy=0,//cursor coordinates in the text buffer, in character units
					scx=0, scy=0,//selcur coordinates in the text buffer, in character units
					nlines=1, text_width=0;//in characters
bool				rectsel=false;
//struct			TextFile
//{
//	std::string filename, text;
//	bool modified;
//};
//std::vector<TextFile> tabs;
std::string			filename;//utf8
std::string			text;//
//std::string		text="Hello.\nSample Text.\nWhat\'s going on???\n";
char				caps_lock=false;

enum				DragType
{
	DRAG_NONE,
	DRAG_SELECT,
	DRAG_VSCROLL,
	DRAG_HSCROLL,
	DRAG_RECT,
};
DragType			drag;
struct				Range
{
	unsigned
		linestart, lineend,//absolute index
		linecols,//line width in character units
		i, f;//offsets from linestart
	Range():linestart(0), lineend(0), linecols(0), i(-1), f(0){}
	//Range(unsigned linestart, unsigned i, unsigned f):linestart(linestart), i(i), f(f){}
	void set(unsigned linestart, unsigned p, int &extent)
	{
		this->linestart=linestart;
		if(i>p)
			i=p;
		if(f<p)
			f=p;
		if(extent<int(f-i))
			extent=f-i;
	}
};
typedef std::vector<Range> Ranges;

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

enum				ActionType
{
	ACTION_UNINITIALIZED,
	ACTION_INSERT,
	ACTION_INSERT_RECT,
	ACTION_ERASE,
	ACTION_ERASE_SELECTION,
	ACTION_ERASE_RECT_SELECTION,
};
struct				ActionFragment
{
	int idx;
	std::string str;
	ActionFragment():idx(0){}
	ActionFragment(ActionFragment const &af):idx(af.idx), str(af.str){}
	ActionFragment(ActionFragment &&af):idx(af.idx), str((std::string&&)af.str){}
	ActionFragment(int idx, const char *a, int len):idx(idx), str(a, len){}
};
typedef std::vector<ActionFragment> ActionData;
struct				History
{
	ActionType type;
	ActionData data;
	union//initial cursor positions
	{
		struct{int scx0, ccx0, scy0, ccy0;};
		struct{int cursor0, selcur0, z_unused0, z_unused1;};
	};
	History():type(ACTION_UNINITIALIZED), scx0(0), ccx0(0), scy0(0), ccy0(0){}
	History(History const &h):type(h.type), data(h.data), scx0(h.scx0), ccx0(h.ccx0), scy0(h.scy0), ccy0(h.ccy0){}
	History(History &&h):type(h.type), data((ActionData&&)h.data), scx0(h.scx0), ccx0(h.ccx0), scy0(h.scy0), ccy0(h.ccy0){}
	History(ActionType type, int idx, const char *a, int len, int cursor0, int selcur0):type(type), cursor0(cursor0), selcur0(selcur0), z_unused0(0), z_unused1(0)//normal editing
	{
		data.push_back(ActionFragment(idx, a, len));
	}
	History(ActionType type, ActionData &&data, int scx0, int ccx0, int scy0, int ccy0):type(type), data((ActionData&&)data), scx0(scx0), ccx0(ccx0), scy0(scy0), ccy0(ccy0){}//rectangular editing
};
std::vector<History> history;
int					histpos=-1;//pointing at previous action
bool				hist_cont=true;

bool				modified=false, dimensions_known=false, wnd_to_cursor=false;

//struct			TextBox
//{
//};
//struct			TextTab
//{
//};
void				set_title(std::string const &name, bool _modified)
{
	modified=_modified;
	set_window_title((name+" - Teletext").c_str());
}
void				set_modified()
{
	if(!modified)
	{
		modified=true;
		get_window_title(g_buf+1, g_buf_size-1);
		g_wbuf[0]=L'*';
		set_window_title(g_buf);
	}
}
void				clear_modified()
{
	if(modified)
	{
		modified=false;
		get_window_title(g_buf, g_buf_size);
		set_window_title(g_buf+1);
	}
}

void				calc_cursor_coords(short x_chars, short tab_origin_chars)
{
	ccx=0, ccy=0;
	for(int k=0;k<cursor;++k)
	{
		if(text[k]=='\n')
			ccx=0, ++ccy;
		else if(text[k]=='\t')
			ccx+=tab_count-mod(x_chars+ccx-tab_origin_chars, tab_count);
		else
			++ccx;
	}
}
void				calc_dimensions_chars(short x_chars, short y_chars, const char *msg, int msg_length, short tab_origin_chars, int &cwidth, int &cheight, int kcursor, int &cx, int &cy, int kselcur, int &sx, int &sy)
{
	cheight=1, cwidth=0;
	int k=0, linecols=0;
	for(;k<msg_length;++k)
	{
		if(k==kcursor)
			cx=linecols, cy=cheight-1;
		if(k==kselcur)
			sx=linecols, sy=cheight-1;
		if(msg[k]=='\t')
			linecols+=tab_count-mod(x_chars+linecols-tab_origin_chars, tab_count);
		else if(msg[k]=='\n')
		{
			if(cwidth<linecols)
				cwidth=linecols;
			++cheight, linecols=0;
		}
		else
			++linecols;
	}
	if(cwidth<linecols)
		cwidth=linecols;
	if(k==kcursor)
		cx=linecols, cy=cheight-1;
	if(k==kselcur)
		sx=linecols, sy=cheight-1;
	if(rectsel)
	{
		if(cwidth<cx)
			cwidth=cx;
		if(cwidth<sx)
			cwidth=sx;
	}
}

inline int			find_line_start(int i)
{
	i-=text[i]=='\n';
	for(;i>=0&&text[i]!='\n';--i);
	return i+1;
}
inline int			find_line_end(int i)
{
	for(;i<(int)text.size()&&text[i]!='\n';++i);
	return i;
}
void				hist_undo()//ctrl z
{
	if(histpos<0)
		return;

	auto &action=history[histpos];
	--histpos;
	switch(action.type)
	{
	case ACTION_INSERT:
		{
			auto &frag=action.data[0];
			text.erase(text.begin()+frag.idx, text.begin()+frag.idx+frag.str.size());
			//selcur=cursor=frag.idx;
			rectsel=false;
		}
		break;
	case ACTION_INSERT_RECT:
		for(int k=action.data.size()-1;k>=0;--k)
		{
			auto &frag=action.data[k];
			text.erase(text.begin()+frag.idx, text.begin()+frag.idx+frag.str.size());
		}
		rectsel=true;
		break;
	case ACTION_ERASE:
		{
			auto &frag=action.data[0];
			text.insert(text.begin()+frag.idx, frag.str.begin(), frag.str.end());
			//selcur=cursor=frag.idx+frag.str.size();
			rectsel=false;
		}
		break;
	case ACTION_ERASE_SELECTION:
		{
			auto &frag=action.data[0];
			text.insert(text.begin()+frag.idx, frag.str.begin(), frag.str.end());
			//selcur=frag.idx, cursor=frag.idx+frag.str.size();
			rectsel=false;
		}
		break;
	case ACTION_ERASE_RECT_SELECTION:
		for(int k=action.data.size()-1;k>=0;--k)
		{
			auto &frag=action.data[k];
			text.insert(text.begin()+frag.idx, frag.str.begin(), frag.str.end());
		}
		rectsel=true;
		break;
	}
	if(rectsel)
		scx=action.scx0, scy=action.scy0, ccx=action.ccx0, ccy=action.ccy0;//set selection rect
	else
	{
		cursor=action.cursor0, selcur=action.selcur0;
		calc_cursor_coords(-wpx, -wpx);
	}
	//if(histpos<0)//
	//	clear_modified();//

	hist_cont=false;
	dimensions_known=false;
	wnd_to_cursor=true;
}
void				hist_redo()//ctrl y
{
	if(histpos>=(int)history.size()-1)
		return;
	++histpos;
	auto &action=history[histpos];
	switch(action.type)
	{
	case ACTION_INSERT:
		{
			auto &frag=action.data[0];
			text.insert(text.begin()+frag.idx, frag.str.begin(), frag.str.end());
			cursor=selcur=frag.idx+frag.str.size();
			//selcur=h.idx, cursor=h.idx+h.data.size();
			rectsel=false;
		}
		break;
	case ACTION_INSERT_RECT:
		{
			int extent=0;
			for(int k=0;k<(int)action.data.size();++k)
			{
				auto &frag=action.data[k];
				text.insert(text.begin()+frag.idx, frag.str.begin(), frag.str.end());
				if(extent<(int)frag.str.size())
					extent=frag.str.size();
			}
			int minx=minimum(action.scx0, action.ccx0);
			scx=minx+extent, scy=action.scy0, ccx=minx+extent, ccy=action.ccy0;
		}
		rectsel=true;
		break;
	case ACTION_ERASE:
	case ACTION_ERASE_SELECTION:
		{
			auto &frag=action.data[0];
			text.erase(text.begin()+frag.idx, text.begin()+frag.idx+frag.str.size());
			cursor=selcur=frag.idx;
			rectsel=false;
		}
		break;
	case ACTION_ERASE_RECT_SELECTION:
		{
			for(int k=0;k<(int)action.data.size();++k)
			{
				auto &frag=action.data[k];
				text.erase(text.begin()+frag.idx, text.begin()+frag.idx+frag.str.size());
			}
			int minx=minimum(action.scx0, action.ccx0);
			scx=minx, scy=action.scy0, ccx=minx, ccy=action.ccy0;
			rectsel=true;
		}
		break;
	}
	if(!rectsel)
		calc_cursor_coords(-wpx, -wpx);

	//set_modified();//
	hist_cont=false;
	dimensions_known=false;
	wnd_to_cursor=true;
}
void				hist_clear()
{
	histpos=-1;
	history.clear();
	//clear_modified();
}
int					calc_ranges(Ranges &ranges)
{
	int rx1, rx2, ry1, ry2;//rect coordinates in character units
	if(scx<ccx)
		rx1=scx, rx2=ccx;
	else
		rx1=ccx, rx2=scx;
	if(scy<ccy)
		ry1=scy, ry2=ccy;
	else
		ry1=ccy, ry2=scy;
	ranges.resize(ry2+1-ry1);
	int linestart=0, linecols=0, lineno=0, r_idx=0, k=0, extent=0;
	Range *r=nullptr;
	for(;k<(int)text.size();++k)
	{
		//if(lineno>=ry1&&lineno<=ry2&&linecols>=rx1&&linecols<rx2+(rx1==rx2))
		if(lineno>=ry1&&lineno<=ry2&&linecols>=rx1&&linecols<=rx2)
			ranges[r_idx].set(linestart, k-linestart, extent);//i&f are idx! because inside
		if(text[k]=='\n')
		{
			if(lineno>=ry1&&lineno<=ry2)
			{
				r=ranges.data()+r_idx;
				if(r->i==-1)
					r->linestart=linestart, r->i=k-linestart+rx1-linecols, r->f=r->i+rx2-rx1;//always idx						//i&f are cols! because didn't reach selection X
				r->lineend=k, r->linecols=linecols;
				++r_idx;
			}
			linestart=k+1, linecols=0, ++lineno;
		}
		else if(text[k]=='\t')
			linecols+=tab_count-mod(-wpx+linecols-(-wpx), tab_count);
		else
			++linecols;
	}
	//if(lineno>=ry1&&lineno<=ry2&&linecols>=rx1&&linecols<rx2+(rx1==rx2))
	if(lineno>=ry1&&lineno<=ry2)
	{
		r=ranges.data()+r_idx;
		if(linecols>=rx1&&linecols<=rx2)
			r->set(linestart, k-linestart, extent);
		if(r->i==-1)
			r->linestart=linestart, r->i=k-linestart+rx1-linecols, r->f=r->i+rx2-rx1;
		r->lineend=k, r->linecols=linecols;
	}
	return extent;
}
void				text_replace_rect(Ranges &ranges, const char *a, int len)//ranges are updated
{
	set_modified();
	history.resize(histpos+1);
	ActionData data;
	bool padded=false, erased=false;
	for(int k=0, cumulative_delta=0;k<(int)ranges.size();++k)	//1) pad selection
	{
		auto &range=ranges[k];
		range.linestart+=cumulative_delta;
		range.lineend+=cumulative_delta;
		int linecount=range.lineend-range.linestart;
		if(linecount<(int)range.i)//line ends before selection - pad this line
		{
			padded=true;
			int delta=range.i-linecount;
			text.insert(text.begin()+range.lineend, delta, ' ');
			data.push_back(ActionFragment(range.lineend, text.c_str()+range.lineend, delta));
			cumulative_delta+=delta;
			range.lineend+=delta;
		}
	//	else if(linecount>=range.f)//line ends after selection - no pad necessary
	//	{
	//	}
	//	else//line ends inside selection - no pad necessary
	//	{
	//	}
	}
	if(padded)
	{
		history.push_back(History(ACTION_INSERT_RECT, std::move(data), scx, ccx, scy, ccy));
		++histpos;
	}
	for(int k=0, cumulative_delta=0;k<(int)ranges.size();++k)	//2) erase selection
	{
		auto &range=ranges[k];
		range.linestart-=cumulative_delta;
		range.lineend-=cumulative_delta;
		if(range.i<range.f)
		{
			unsigned linecount=range.lineend-range.linestart, delta=0;
			//if(linecount<range.i)//unreachable
			//{
			//}
			//else
			if(linecount>=range.f)//erase [linestart+i, linestart+f[
			{
				erased=true;
				delta=range.f-range.i;
				data.push_back(ActionFragment(range.linestart+range.i, text.c_str()+range.linestart+range.i, delta));
				text.erase(text.begin()+range.linestart+range.i, text.begin()+range.linestart+range.f);
			}
			else if(range.i<linecount&&linecount<range.f)//erase [linestart+i, lineend[
			{
				erased=true;
				delta=linecount-range.i;
				data.push_back(ActionFragment(range.linestart+range.i, text.c_str()+range.linestart+range.i, delta));
				text.erase(text.begin()+range.linestart+range.i, text.begin()+range.lineend);
			}
			cumulative_delta+=delta;
			range.lineend-=delta;
			range.f=range.i;
		}
	}
	if(erased)
	{
		history.push_back(History(ACTION_ERASE_RECT_SELECTION, std::move(data), scx, ccx, scy, ccy));
		++histpos;
	}
	
	ccx=scx=minimum(ccx, scx);
	if(len>0)													//3) insert text
	{
		if(padded||erased||!hist_cont||histpos<0||history[histpos].type!=ACTION_INSERT_RECT||history[histpos].data.size()!=ranges.size())
		{
			for(int k=0, cumulative_delta=0;k<(int)ranges.size();++k)
			{
				auto &range=ranges[k];
				range.linestart+=cumulative_delta;
				text.insert(text.begin()+range.linestart+range.i, a, a+len);

				data.push_back(ActionFragment(range.linestart+range.i, a, len));
				cumulative_delta+=len;
				range.lineend+=cumulative_delta;
			}
			history.push_back(History(ACTION_INSERT_RECT, std::move(data), scx, ccx, scy, ccy));
			++histpos;
		}
		else
		{
			auto &h=history[histpos];
			for(int k=0, cumulative_delta=0;k<(int)ranges.size();++k)
			{
				auto &range=ranges[k];
				range.linestart+=cumulative_delta;
				h.data[k].idx+=cumulative_delta;
				text.insert(text.begin()+range.linestart+range.i, a, a+len);

				auto &dk=h.data[k];
				dk.str.insert(dk.str.end(), a, a+len);
				cumulative_delta+=len;
				range.lineend+=cumulative_delta;
			}
		}
		ccx=scx+=len;
	}
	hist_cont=true;
	dimensions_known=false;
	wnd_to_cursor=true;
}
void				text_replace(int i, int f, const char *a, int len)
{
	set_modified();
	history.resize(histpos+1);
	history.push_back(History(ACTION_ERASE_SELECTION, i, text.c_str()+i, f-i, cursor, selcur));

	text.replace(text.begin()+i, text.begin()+f, a, a+len);

	history.push_back(History(ACTION_INSERT, i, text.c_str()+i, len, i, i));
	histpos+=2;
	
	cursor=selcur=i+len;
	calc_cursor_coords(-wpx, -wpx);
	hist_cont=false;
	dimensions_known=false;
	wnd_to_cursor=true;
	//++total_action_count;
}
void				text_insert(int i, const char *a, int len)
{
	set_modified();
	text.insert(text.begin()+i, a, a+len);

	++histpos;
	history.resize(histpos);
	history.push_back(History(ACTION_INSERT, i, text.c_str()+i, len, cursor, selcur));

	cursor=selcur=i+len;
	calc_cursor_coords(-wpx, -wpx);
	hist_cont=false;
	dimensions_known=false;
	wnd_to_cursor=true;
	//++total_action_count;
}
void				text_erase(int i, int f)
{
	set_modified();
	++histpos;
	history.resize(histpos);
	history.push_back(History(f-i>2?ACTION_ERASE_SELECTION:ACTION_ERASE, i, text.c_str()+i, f-i, cursor, selcur));

	text.erase(text.begin()+i, text.begin()+f);

	cursor=selcur=i;
	calc_cursor_coords(-wpx, -wpx);
	hist_cont=false;
	dimensions_known=false;
	wnd_to_cursor=true;
	//++total_action_count;
}
void				text_insert1(int i, char c)
{
	set_modified();
	text.insert(text.begin()+i, c);

	if(!hist_cont||c=='\n'||histpos<0||history[histpos].type!=ACTION_INSERT)
	{
		++histpos;
		history.resize(histpos);
		history.push_back(History(ACTION_INSERT, i, text.c_str()+i, 1, cursor, selcur));
	}
	else
		history[histpos].data[0].str.push_back(c);

	cursor=selcur=i+1;
	calc_cursor_coords(-wpx, -wpx);
	hist_cont=true;
	dimensions_known=false;
	wnd_to_cursor=true;
	//++total_action_count;
}
void				text_erase1_bksp(int i)
{
	if(i<1)
		return;
	set_modified();
	if(!hist_cont||histpos<0||history[histpos].type!=ACTION_ERASE)
	{
		++histpos;
		history.resize(histpos);
		history.push_back(History(ACTION_ERASE, i-1, text.c_str()+i-1, 1, cursor, selcur));
	}
	else
	{
		auto &str=history[histpos].data[0].str;
		str.insert(str.begin(), text[i]);
	}

	text.erase(text.begin()+i-1);

	cursor=selcur=i-1;
	calc_cursor_coords(-wpx, -wpx);
	hist_cont=true;
	dimensions_known=false;
	wnd_to_cursor=true;
	//++total_action_count;
}
void				text_erase1_del(int i)
{
	if(i>=(int)text.size())
		return;
	set_modified();
	if(!hist_cont||histpos<0||history[histpos].type!=ACTION_ERASE)
	{
		++histpos;
		history.resize(histpos);
		history.push_back(History(ACTION_ERASE, i, text.c_str()+i, 1, cursor, selcur));
	}
	else
		history[histpos].data[0].str.push_back(text[i]);

	text.erase(text.begin()+i);
	
	cursor=selcur=i;
	calc_cursor_coords(-wpx, -wpx);
	hist_cont=true;
	dimensions_known=false;
	wnd_to_cursor=true;
	//++total_action_count;
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
void				cursor_at_mouse()//sets cursor, ccx & ccy
{
	int dypx=dy*font_zoom, ypos=(wpy+my)/dypx;
	cursor=0, ccy=0;
	if(ypos>0)
	{
		for(int k=0;k<(int)text.size();++k)
		{
			if(text[k]=='\n')
			{
				cursor=k+1, ++ccy;
				if(ccy==ypos)
					break;
			}
		}
	}
	int d_idx=0;
	inv_calc_width(-wpx, -wpy, text.c_str()+cursor, text.size()-cursor, -wpx, font_zoom, wpx+mx+(dx*font_zoom>>1), &ccx, &d_idx);
	cursor+=d_idx;
}
bool				cursor_at_mouse_rect(int &xpos, int &ypos)//sets ccx & ccy dummies
{
	int dxpx=dx*font_zoom, dypx=dy*font_zoom,
		ypos0=(wpy+my)/dypx, linestart=0, ypos2=0;
	if(ypos0>0)
	{
		for(int k=0;k<(int)text.size();++k)
		{
			if(text[k]=='\n')
			{
				linestart=k+1, ++ypos2;
				if(ypos2==ypos0)
					break;
			}
		}
	}
	int d_idx=0;
	inv_calc_width(-wpx, -wpy, text.c_str()+linestart, text.size()-linestart, -wpx, font_zoom, wpx+mx+(dxpx>>1), &xpos, &d_idx);

	d_idx=find_line_end(linestart+d_idx);
	int ncols=calc_width(-wpx, -wpy, text.c_str()+linestart, d_idx-linestart, -wpx, font_zoom)/dxpx;
	int xpos2=(wpx+mx+(dxpx>>1))/dxpx;
	if(ncols<xpos2)//if mx is beyond the reach of the line
		xpos=xpos2;

	if(ypos2>=0&&ypos2<nlines)
	{
		ypos=ypos2;
		return true;
	}
	return false;
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
//	const int scrollbarwidth=17;
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

inline void			lbutton_down_text()
{
	//if(get_key_state(TK_CONTROL))
	if(rectsel=get_key_state(TK_LALT)||get_key_state(TK_RALT))
	{
		int xpos=0, ypos=0;
		if(cursor_at_mouse_rect(xpos, ypos))
			drag=DRAG_RECT, ccx=xpos, ccy=ypos;
		if(!get_key_state(TK_LSHIFT)&&!get_key_state(TK_RSHIFT))
			scx=ccx, scy=ccy;
	}
	else
	{
		drag=DRAG_SELECT;
		cursor_at_mouse();
		if(!get_key_state(TK_LSHIFT)&&!get_key_state(TK_RSHIFT))
			selcur=cursor, scx=ccx, scy=ccy;
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
	auto rgb=(int*)stbi_load("font.PNG", &iw, &ih, &bytespp, 4);
	if(!rgb)
	{
		messagebox("Error", "Font texture not found.\nPlace a \'font.PNG\' file with the program.\n");
		return false;
	}
	dx=rgb[0]&0xFF, dy=rgb[1]&0xFF;
	//dx=iw/8, dy=ih/16;
	for(int k=0, size=iw*ih;k<size;++k)
		if(rgb[k]&0x00FFFFFF)
			rgb[k]=0xFFFFFFFF;
	for(int c=32;c<127;++c)
	{
		auto &rect=font_coords[c-32];
		int px=(iw>>3)*(c&7), py=(ih>>4)*(c>>3);
		rect.x1=float(px)/iw, rect.x2=float(px+dx)/iw;
		rect.y1=float(py)/ih, rect.y2=float(py+dy)/ih;
		//rect.x1=float(px-0.5f)/iw, rect.x2=float(px+dx-0.5f)/iw;
		//rect.y1=float(py-0.5f)/ih, rect.y2=float(py+dy-0.5f)/ih;
	}
	glGenTextures(1, &font_txid);
	send_texture_pot(font_txid, rgb, iw, ih);
	stbi_image_free(rgb);
	prof_add("Load font");

	//set_text_colors(0x80FF00FF8000FF00);
	//set_text_colors(0xFFFFFFFFFF000000);
	set_text_colors(colors_text);//dark mode

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

	//static float level=0, inc=0.1;
	//glClearColor(level, level, level, 1);
	//level+=inc;
	//if(level<0)
	//	level=0, inc=0.1;
	//if(level>1)
	//	level=1, inc=-0.1;
	glClear(GL_COLOR_BUFFER_BIT);
	
	//calculate text dimensions & cursor coordinates
	auto arr=text.c_str();
	int textlen=text.size();
	if(!dimensions_known)
	{
		dimensions_known=true;
		if(rectsel)
			calc_dimensions_chars(-wpx, -wpy, arr, textlen, -wpx, text_width, nlines, -1, ccx, ccy, -1, scx, scy);
		else
			calc_dimensions_chars(-wpx, -wpy, arr, textlen, -wpx, text_width, nlines, cursor, ccx, ccy, selcur, scx, scy);
		prof_add("calc dimensions");
	}
	int dxpx=dx*font_zoom, dypx=dy*font_zoom;
	int iw=text_width*dxpx, ih=nlines*dypx;
	
	//decide if need scrollbars
	hscroll.decide(iw+scrollbarwidth>w);
	vscroll.decide(ih+scrollbarwidth>h);

	int cpx=ccx*dxpx, cpy=ccy*dypx;
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

	//sprintf_s(g_buf, g_buf_size, "cc(%d, %d), sc(%d, %d)", ccx, ccy, scx, scy);//
	//set_window_title_a(g_buf);//
	int y=0;
	if(rectsel)//rectangular selection
	{
		int rx1, rx2, ry1, ry2;//rect coordinates in characters
		if(scx<ccx)
			rx1=scx, rx2=ccx;
		else
			rx1=ccx, rx2=scx;
		if(scy<ccy)
			ry1=scy, ry2=ccy;
		else
			ry1=ccy, ry2=scy;
		int px1=rx1*dxpx, px2=rx2*dxpx, py1=ry1*dypx, py2=ry2*dypx;
		draw_rectangle_i(px1-wpx, px2-wpx, py1-wpy, py2+dypx-wpy, colors_selection.hi);
		for(int start=0, line_start=0, lineno=0, x=0, k=0;;++k)
		{
			if(lineno>=ry1&&lineno<=ry2)
			{
				int ypos=y-wpy, d_idx=0;
				inv_calc_width(x-wpx, ypos, arr+start, textlen-start, -wpx, font_zoom, px1, nullptr, &d_idx);
				k=start+d_idx;
				if(ypos>-dxpx&&ypos<h+dxpx)
					x+=print_line(x-wpx, ypos, arr+start, k-start, -wpx, font_zoom);
				if(k<textlen&&text[k]=='\n')
					start=k+1, x=0, y+=dypx, line_start=start=k+1, ++lineno;
				else
				{
					start=k;
					if(k>=textlen)
						break;
					set_text_colors(colors_selection);
					inv_calc_width(x-wpx, ypos, arr+start, textlen-start, -wpx, font_zoom, px2-px1, nullptr, &d_idx);
					k=start+d_idx;
					int nconsumed=0;
					if(ypos>-dxpx&&ypos<h+dxpx)
						x+=print_line_rect(x-wpx, ypos, arr+start, k-start, -wpx, font_zoom, rx2-rx1, nconsumed);
					start+=nconsumed, k=start;
					set_text_colors(colors_text);

					if(k>=textlen)
						break;
					if(text[k]!='\n')
					{
						k=find_line_end(k);
						int ypos=y-wpy;
						if(ypos>-dxpx&&ypos<h+dxpx)
							x+=print_line(x-wpx, ypos, arr+start, k-start, -wpx, font_zoom);
						if(k>=textlen)
							break;
					}
					x=0, y+=dypx, line_start=start=k+1, ++lineno;
				}
			}
			else if(k>=textlen||text[k]=='\n')
			{
				int ypos=y-wpy;
				if(ypos>-dxpx&&ypos<h+dxpx)
					x+=print_line(x-wpx, ypos, arr+start, k-start, -wpx, font_zoom);
				y+=dypx;
				if(k>=textlen)
					break;
				x=0, line_start=start=k+1, ++lineno;
			}
		}
		draw_line_i(cpx-wpx, py1-wpy, cpx-wpx, py2+dypx-wpy, 0xFFFFFFFF);//draw cursor
	}
	else if(cursor!=selcur)//normal selection
	{
		int selstart, selend;
		if(cursor<selcur)
			selstart=cursor, selend=selcur;
		else
			selstart=selcur, selend=cursor;
		for(int start=0, line_start=0, x=0, k=0;;++k)
		{
			if(k==selstart)
			{
				int ypos=y-wpy;
				if(ypos>-dxpx&&ypos<h+dxpx)
					x+=print_line(x-wpx, ypos, arr+start, k-start, -wpx, font_zoom);
				start=k;
				set_text_colors(colors_selection);
			}
			if(k==selend)
			{
				int ypos=y-wpy;
				if(ypos>-dxpx&&ypos<h+dxpx)
					x+=print_line(x-wpx, ypos, arr+start, k-start, -wpx, font_zoom);
				start=k;
				set_text_colors(colors_text);
			}
			if(k>=textlen||text[k]=='\n')
			{
				int ypos=y-wpy;
				if(ypos>-dxpx&&ypos<h+dxpx)
					x+=print_line(x-wpx, ypos, arr+start, k-start, -wpx, font_zoom);
				y+=dypx;
				if(k>=textlen)
					break;
				line_start=start=k+1;
				x=0;
			}
		}
		draw_line_i(cpx-wpx, cpy-wpy, cpx-wpx, cpy+dypx-wpy, 0xFFFFFFFF);//draw cursor
	}
	else//no selection
	{
		draw_rectangle_i(0, w, cpy-wpy, cpy+dypx-wpy, color_cursorlinebk);//highlight cursor line
		for(int start=0, line_start=0, lineno=0, x=0, k=0;;++k)
		{
			if(k>=textlen||text[k]=='\n')
			{
				int ypos=y-wpy;
				if(ypos>-dxpx&&ypos<h+dxpx)
				{
					if(lineno==ccy)
						set_text_colors(colors_cursorline);
					x+=print_line(x-wpx, ypos, arr+start, k-start, -wpx, font_zoom);
					if(lineno==ccy)
						set_text_colors(colors_text);
				}
				y+=dypx;
				if(k>=textlen)
					break;
				line_start=start=k+1;
				x=0;
				++lineno;
			}
		}
		draw_line_i(cpx-wpx, cpy-wpy, cpx-wpx, cpy+dypx-wpy, 0xFFFFFFFF);//draw cursor
	}
	prof_add("text");

	//draw possible scrollbars
	{
		int x1=0, x2=w, y1=0, y2=h;
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
	}
	prof_add("scrollbars");

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
bool				wnd_on_mousemove()
{
	switch(drag)
	{
	case DRAG_SELECT:
	case DRAG_RECT:
		if(rectsel)
		{
			drag=DRAG_RECT;
			cursor_at_mouse_rect(ccx, ccy);
		}
		else
		{
			drag=DRAG_SELECT;
			cursor_at_mouse();
		}
		return true;
	case DRAG_VSCROLL:
	case DRAG_HSCROLL:
		return true;
	}
	return false;
}
bool				wnd_on_mousewheel(bool mw_forward)
{
	if(get_key_state(TK_LCTRL)||get_key_state(TK_RCTRL))
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
		if(mw_forward)
			wpy-=dy*font_zoom*3;
		else
			wpy+=dy*font_zoom*3;
	}
	return true;
}
bool				wnd_on_lbuttondown()
{
	if(vscroll.dwidth)
	{
		if(hscroll.dwidth)	//both scrollbars present
		{
			if(mx<w-scrollbarwidth)
			{
				if(my<h-scrollbarwidth)
					lbutton_down_text();
				else
					drag=DRAG_HSCROLL, hscroll.click_on_slider(mx);
			}
			else
			{
				if(my<h-scrollbarwidth)
					drag=DRAG_VSCROLL, vscroll.click_on_slider(my);
			}
		}
		else				//only vscroll present
		{
			if(mx<w-scrollbarwidth)
				lbutton_down_text();
			else
				drag=DRAG_VSCROLL, vscroll.click_on_slider(my);
		}
	}
	else
	{
		if(hscroll.dwidth)	//only hscroll present
		{
			if(my<h-scrollbarwidth)
				lbutton_down_text();
			else
				drag=DRAG_HSCROLL, hscroll.click_on_slider(mx);
		}
		else				//no scrollbars
			lbutton_down_text();
	}
	mouse_capture();
	return true;
}
bool				wnd_on_lbuttonup()
{
	drag=DRAG_NONE;
	mouse_release();
	return true;
}
bool				wnd_on_rbuttondown(){return false;}
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
bool				wnd_on_open()
{
	//TODO: tabs
	auto str=open_file_dialog();
	if(str&&open_text_file(str, text))
	{
		filename=str;
		set_title(filename, false);
		hist_cont=true;
		dimensions_known=false;
	}
	return true;
}
bool				wnd_on_select_all()
{
	selcur=0, cursor=text.size(), rectsel=false;
	ccy=nlines-1, ccx=cursor-find_line_start(cursor);
	wnd_to_cursor=true, hist_cont=false;
	return true;
}
bool				wnd_on_deselect()
{
	selcur=cursor, scx=ccx, scy=ccy, rectsel=false;
	wnd_to_cursor=true;
	return true;
}
bool				wnd_on_save(bool save_as)
{
	if(save_as||!filename.size())
	{
		auto str=save_file_dialog();
		if(str&&save_text_file(str, text))
		{
			filename=str;
			set_title(filename, false);
		}
	}
	else
	{
		save_text_file(filename.c_str(), text);
		set_title(filename, false);
	}
	return true;
}
bool				wnd_on_clear_hist()
{
	hist_clear();//TODO: inline
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
bool				wnd_on_copy()
{
	if(rectsel)
	{
		if(ccx!=scx||ccy!=scy)
		{
			std::string str;
			int rx1, rx2, ry1, ry2;//rect coordinates in character units
			if(scx<ccx)
				rx1=scx, rx2=ccx;
			else
				rx1=ccx, rx2=scx;
			if(scy<ccy)
				ry1=scy, ry2=ccy;
			else
				ry1=ccy, ry2=scy;
			str.reserve((rx2-rx1)*(ry2-ry1));
			for(int k=0, linecols=0, lineno=0;k<(int)text.size();++k)
			{
				if(lineno>=ry1&&lineno<=ry2&&linecols>=rx1&&linecols<rx2)
					str+=text[k];
				if(text[k]=='\n')
				{
					if(lineno>=ry1&&lineno<=ry2)
						str+='\n';
					linecols=0, ++lineno;
				}
				else if(text[k]=='\t')
					linecols+=tab_count-mod(-wpx+linecols-(-wpx), tab_count);
				else
					++linecols;
			}
			if(str.back()!='\n')
				str+='\n';
			copy_to_clipboard(str);
		}
	}
	else if(cursor!=selcur)
	{
		int start, end;
		if(selcur<cursor)
			start=selcur, end=cursor;
		else
			start=cursor, end=selcur;
		copy_to_clipboard_c(text.c_str()+start, end-start);
	}
	return false;
}
bool				wnd_on_paste()
{
	char *str=nullptr;
	int len=0;
	if(!paste_from_clipboard(str, len))
		return false;
	if(rectsel)
	{
		Ranges ranges;
		calc_ranges(ranges);
		text_replace_rect(ranges, str, len);
		ccx=scx=minimum(ccx, scx);
	}
	else if(cursor!=selcur)
	{
		int start, end;
		if(selcur<cursor)
			start=selcur, end=cursor;
		else
			start=cursor, end=selcur;
		text_replace(start, end, str, len);
	}
	else
		text_insert(cursor, str, len);
	delete[] str;
	calc_cursor_coords(-wpx, -wpx);
	return true;
}
bool				wnd_on_new()
{
	hist_clear();
	text.clear();
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
	if(cursor>0)
	{
		hist_cont=false;
		--cursor;
		char initial=group_char(text[cursor]);
		for(;cursor&&group_char(text[cursor-1])==initial;--cursor);
		calc_cursor_coords(-wpx, -wpx);
	}
	if(!get_key_state(TK_LSHIFT)&&!get_key_state(TK_RSHIFT))
		selcur=cursor, scx=ccx, scy=ccy, rectsel=false;
	wnd_to_cursor=true;
	return true;
}
bool				wnd_on_skip_word_right()
{
	if(cursor<(int)text.size())
	{
		hist_cont=false;
		char initial=group_char(text[cursor]);
		++cursor;
		for(;cursor<(int)text.size()&&group_char(text[cursor])==initial;++cursor);
		calc_cursor_coords(-wpx, -wpx);
	}
	if(!get_key_state(TK_LSHIFT)&&!get_key_state(TK_RSHIFT))
		selcur=cursor, scx=ccx, scy=ccy, rectsel=false;
	wnd_to_cursor=true;
	return true;
}
bool				wnd_on_goto_file_start()
{
	if(cursor)
	{
		hist_cont=false;
		cursor=0, ccx=0, ccy=0;
	}
	if(!get_key_state(TK_LSHIFT)&&!get_key_state(TK_RSHIFT))
		selcur=cursor, scx=ccx, scy=ccy, rectsel=false;
	wnd_to_cursor=true;
	return true;
}
bool				wnd_on_goto_file_end()
{
	if(cursor<(int)text.size())
	{
		hist_cont=false;
		cursor=text.size();
		calc_cursor_coords(-wpx, -wpx);
	}
	if(!get_key_state(TK_LSHIFT)&&!get_key_state(TK_RSHIFT))
		selcur=cursor, scx=ccx, scy=ccy, rectsel=false;
	wnd_to_cursor=true;
	return true;
}
bool				wnd_on_goto_line_start()
{
	int c2=find_line_start(cursor);
	if(cursor!=c2)
	{
		cursor=c2, hist_cont=false;
		calc_cursor_coords(-wpx, -wpx);
	}
	if(!get_key_state(TK_LSHIFT)&&!get_key_state(TK_RSHIFT))
		selcur=cursor, scx=ccx, scy=ccy, rectsel=false;
	wnd_to_cursor=true;
	return true;
}
bool				wnd_on_goto_line_end()
{
	int c2=find_line_end(cursor);
	if(cursor!=c2)
	{
		cursor=c2, hist_cont=false;
		calc_cursor_coords(-wpx, -wpx);
	}
	if(!get_key_state(TK_LSHIFT)&&!get_key_state(TK_RSHIFT))
		selcur=cursor, scx=ccx, scy=ccy, rectsel=false;
	wnd_to_cursor=true;
	return true;
}
bool				wnd_on_begin_rectsel()
{
	if(drag==DRAG_SELECT)
	{
		drag=DRAG_RECT, rectsel=true;
		return true;
	}
	return false;
}
bool				wnd_on_deletechar()
{
	if(rectsel)
	{
		Ranges ranges;
		int extent=calc_ranges(ranges);
		if(!extent)
		{
			for(int k=0;k<(int)ranges.size();++k)//selection trick
			{
				auto &range=ranges[k];
				range.f+=range.f<(int)text.size()&&text[range.f]!='\n';
			}
		}
		text_replace_rect(ranges, nullptr, 0);
	}
	else if(cursor!=selcur)
	{
		int start, end;
		if(selcur<cursor)
			start=selcur, end=cursor;
		else
			start=cursor, end=selcur;
		text_erase(start, end);
		//cursor=selcur=start, rectsel=false;
	}
	else
		text_erase1_del(cursor);
	return true;
}
bool				wnd_on_backspace()
{
	if(rectsel)
	{
		Ranges ranges;
		int extent=calc_ranges(ranges);
		if(!extent)
		{
			for(int k=0;k<(int)ranges.size();++k)//selection trick
			{
				auto &range=ranges[k];
				range.i-=range.i>0&&text[range.i-1]!='\n';
			}
			--ccx, --scx;
		}
		text_replace_rect(ranges, nullptr, 0);
	}
	else if(cursor!=selcur)
	{
		int start, end;
		if(selcur<cursor)
			start=selcur, end=cursor;
		else
			start=cursor, end=selcur;
		text_erase(start, end);
		//cursor=selcur=start, rectsel=false;
	}
	else
		text_erase1_bksp(cursor);
	return true;
}
bool				wnd_on_cursor_up()
{
	if(!get_key_state(TK_LSHIFT)&&!get_key_state(TK_RSHIFT)&&cursor!=selcur)
	{
		cursor=selcur=minimum(cursor, selcur), rectsel=false;
		calc_cursor_coords(-wpx, -wpx);
		scx=ccx, scy=ccy;
	}
	else
	{
		hist_cont=false;
		int start=find_line_start(cursor);
		if(!start)
			cursor=0;
		else
		{
			int width=calc_width(0, 0, text.c_str()+start, cursor-start, 0, font_zoom);
			int s0=find_line_start(start-1), d_idx=0;
			inv_calc_width(0, 0, text.c_str()+s0, text.size()-s0, 0, font_zoom, width, nullptr, &d_idx);
			cursor=s0+d_idx;
		}
		calc_cursor_coords(-wpx, -wpx);
		if(!get_key_state(TK_LSHIFT)&&!get_key_state(TK_RSHIFT))
			selcur=cursor, scx=ccx, scy=ccy, rectsel=false;
	}
	wnd_to_cursor=true;
	return true;
}
bool				wnd_on_cursor_down()
{
	if(!get_key_state(TK_LSHIFT)&&!get_key_state(TK_RSHIFT)&&cursor!=selcur)
	{
		cursor=selcur=maximum(cursor, selcur), rectsel=false;
		calc_cursor_coords(-wpx, -wpx);
		scx=ccx, scy=ccy;
	}
	else
	{
		hist_cont=false;
		int end=find_line_end(cursor);
		if(end>=(int)text.size())
			cursor=text.size();
		else
		{
			int start=find_line_start(cursor);
			int width=calc_width(0, 0, text.c_str()+start, cursor-start, 0, font_zoom);
			int s2=end+1, d_idx=0;
			inv_calc_width(0, 0, text.c_str()+s2, text.size()-s2, 0, font_zoom, width, nullptr, &d_idx);
			cursor=s2+d_idx;
		}
		calc_cursor_coords(-wpx, -wpx);
		if(!get_key_state(TK_LSHIFT)&&!get_key_state(TK_RSHIFT))
			selcur=cursor, scx=ccx, scy=ccy, rectsel=false;
	}
	wnd_to_cursor=true;
	return true;
}
bool				wnd_on_cursor_left()
{
	if(!get_key_state(TK_LSHIFT)&&!get_key_state(TK_RSHIFT)&&cursor!=selcur)
	{
		cursor=selcur=minimum(cursor, selcur), rectsel=false;
		calc_cursor_coords(-wpx, -wpx);
		scx=ccx, scy=ccy;
	}
	else
	{
		if(cursor>0)
		{
			hist_cont=false;
			--cursor;
			calc_cursor_coords(-wpx, -wpx);
		}
		if(!get_key_state(TK_LSHIFT)&&!get_key_state(TK_RSHIFT))
			selcur=cursor, scx=ccx, scy=ccy, rectsel=false;
	}
	wnd_to_cursor=true;
	return true;
}
bool				wnd_on_cursor_right()
{
	if(!get_key_state(TK_LSHIFT)&&!get_key_state(TK_RSHIFT)&&cursor!=selcur)
	{
		cursor=selcur=maximum(cursor, selcur), rectsel=false;
		calc_cursor_coords(-wpx, -wpx);
		scx=ccx, scy=ccy;
	}
	else
	{
		if(cursor<(int)text.size())
		{
			hist_cont=false;
			++cursor;
			calc_cursor_coords(-wpx, -wpx);
		}
		if(!get_key_state(TK_LSHIFT)&&!get_key_state(TK_RSHIFT))
			selcur=cursor, scx=ccx, scy=ccy, rectsel=false;
	}
	wnd_to_cursor=true;
	return true;
}
bool				wnd_on_type(char character)
{
	if(rectsel)
	{
		Ranges ranges;
		calc_ranges(ranges);
		text_replace_rect(ranges, &character, 1);
	}
	else if(cursor!=selcur)
	{
		int start, end;
		if(selcur<cursor)
			start=selcur, end=cursor;
		else
			start=cursor, end=selcur;
		text_replace(start, end, &character, 1);
		//cursor=selcur=start, rectsel=false;
	}
	else
		text_insert1(cursor, character);
	return true;
}
#if 0
bool				wnd_on_input(int message, int wParam, int lParam)
{
	switch(message)
	{
	case WM_MOUSEMOVE:
		switch(drag)
		{
		case DRAG_SELECT:
		case DRAG_RECT:
			if(rectsel)
			//if(rectsel=get_key_state(TK_MENU)!=0)
			{
				drag=DRAG_RECT;
				cursor_at_mouse_rect(ccx, ccy);
			}
			else
			{
				drag=DRAG_SELECT;
				cursor_at_mouse();
			}
			return true;
		case DRAG_VSCROLL:
		case DRAG_HSCROLL:
			return true;
		}
		return false;

	case WM_MOUSEWHEEL:
		{
			bool mw_forward=((short*)&wParam)[1]>0;
			if(get_key_state(TK_LCTRL)||get_key_state(TK_RCTRL))
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
				if(mw_forward)
					wpy-=dy*font_zoom*3;
				else
					wpy+=dy*font_zoom*3;
			}
		}
		break;

	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		if(vscroll.dwidth)
		{
			if(hscroll.dwidth)	//both scrollbars present
			{
				if(mx<w-scrollbarwidth)
				{
					if(my<h-scrollbarwidth)
						lbutton_down_text();
					else
						drag=DRAG_HSCROLL, hscroll.click_on_slider(mx);
				}
				else
				{
					if(my<h-scrollbarwidth)
						drag=DRAG_VSCROLL, vscroll.click_on_slider(my);
				}
			}
			else				//only vscroll present
			{
				if(mx<w-scrollbarwidth)
					lbutton_down_text();
				else
					drag=DRAG_VSCROLL, vscroll.click_on_slider(my);
			}
		}
		else
		{
			if(hscroll.dwidth)	//only hscroll present
			{
				if(my<h-scrollbarwidth)
					lbutton_down_text();
				else
					drag=DRAG_HSCROLL, hscroll.click_on_slider(mx);
			}
			else				//no scrollbars
				lbutton_down_text();
		}
		mouse_capture();
		break;
	case WM_LBUTTONUP:
		drag=DRAG_NONE;
		mouse_release();
		break;

	case WM_RBUTTONDOWN:
	case WM_RBUTTONDBLCLK:
		break;
	case WM_RBUTTONUP:
		break;
		
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
#if 1
		if(get_key_state(TK_LCTRL)||get_key_state(TK_RCTRL))
		{
			switch(wParam)
			{
			case TK_CAPSLOCK:				return false;
			case TK_NUMLOCK:				return false;
		//	case VK_OEM_3:					return false;
			case '1':						return false;
			case '2':						return false;
			case '3':						return false;
			case '4':						return false;
			case '5':						return false;
			case '6':						return false;
			case '7':						return false;
			case '8':						return false;
			case '9':						return false;
			case '0':						return false;
		//	case VK_OEM_MINUS:				return false;
		//	case VK_OEM_PLUS:				return false;
			case 'Q':						return false;
			case 'W':						return false;
			case 'E':						return false;
			case 'R':						return false;
			case 'T':						return false;
			case 'Y':
				hist_redo();
				break;
			case 'U':						return false;
			case 'I':						return false;
			case 'O'://open
				{
					//TODO: tabs
					auto str=open_file_dialog();
					if(str&&open_text_file(str, text))
					{
						filename=str;
						set_title(filename, false);
						hist_cont=true;
						dimensions_known=false;
					}
				}
				return true;
			case 'P':						return false;
		//	case VK_OEM_4:					return false;
		//	case VK_OEM_6:					return false;
			case 'A':
				selcur=0, cursor=text.size(), rectsel=false;
				ccy=nlines-1, ccx=cursor-find_line_start(cursor);
				wnd_to_cursor=true, hist_cont=false;
				break;
			case 'S'://save
				if(get_key_state(TK_LSHIFT)||get_key_state(TK_RSHIFT)||!filename.size())
				{
					auto str=save_file_dialog();
					if(str&&save_text_file(str, text))
					{
						filename=str;
						set_title(filename, false);
					}
				}
				else
				{
					save_text_file(filename.c_str(), text);
					set_title(filename, false);
				}
				break;
			case 'D'://dump history
				hist_clear();
				//histpos=-1, history.clear();
				break;
			case 'F':						return false;
			case 'G':						return false;
			case 'H':						return false;
			case 'J':						return false;
			case 'K':						return false;
			case 'L':						return false;
		//	case VK_OEM_1:					return false;
		//	case VK_OEM_7:					return false;
		//	case VK_OEM_5:case VK_OEM_102:	return false;
			case 'Z':
				hist_undo();
				break;
			case 'X':
			case 'C':
				if(rectsel)
				{
					if(ccx!=scx||ccy!=scy)
					{
						std::string str;
						int rx1, rx2, ry1, ry2;//rect coordinates in character units
						if(scx<ccx)
							rx1=scx, rx2=ccx;
						else
							rx1=ccx, rx2=scx;
						if(scy<ccy)
							ry1=scy, ry2=ccy;
						else
							ry1=ccy, ry2=scy;
						str.reserve((rx2-rx1)*(ry2-ry1));
						for(int k=0, linecols=0, lineno=0;k<(int)text.size();++k)
						{
							if(lineno>=ry1&&lineno<=ry2&&linecols>=rx1&&linecols<rx2)
								str+=text[k];
							if(text[k]=='\n')
							{
								if(lineno>=ry1&&lineno<=ry2)
									str+='\n';
								linecols=0, ++lineno;
							}
							else if(text[k]=='\t')
								linecols+=tab_count-mod(-wpx+linecols-(-wpx), tab_count);
							else
								++linecols;
						}
						if(str.back()!='\n')
							str+='\n';
						copy_to_clipboard(str);
					}
				}
				else if(cursor!=selcur)
				{
					int start, end;
					if(selcur<cursor)
						start=selcur, end=cursor;
					else
						start=cursor, end=selcur;
					copy_to_clipboard_c(text.c_str()+start, end-start);
				}
				break;
			case 'V':
				{
					char *str=nullptr;
					int len=0;
					if(!paste_from_clipboard(str, len))
						return false;
					if(rectsel)
					{
						Ranges ranges;
						calc_ranges(ranges);
						text_replace_rect(ranges, str, len);
						ccx=scx=minimum(ccx, scx);
					}
					else if(cursor!=selcur)
					{
						int start, end;
						if(selcur<cursor)
							start=selcur, end=cursor;
						else
							start=cursor, end=selcur;
						text_replace(start, end, str, len);
					}
					else
						text_insert(cursor, str, len);
					delete[] str;
					calc_cursor_coords(-wpx, -wpx);
				}
				break;
			case 'B':						return false;
			case 'N'://TODO: tabs
				hist_clear();
				text.clear();
				return true;
			case 'M':						return false;
		//	case VK_OEM_COMMA:				return false;
		//	case VK_OEM_PERIOD:				return false;
		//	case VK_OEM_2:					return false;
			case ' ':						return false;
		//	case VK_NUMPAD0:				return false;
		//	case VK_NUMPAD1:				return false;
		//	case VK_NUMPAD2:				return false;
		//	case VK_NUMPAD3:				return false;
		//	case VK_NUMPAD4:				return false;
		//	case VK_NUMPAD5:				return false;
		//	case VK_NUMPAD6:				return false;
		//	case VK_NUMPAD7:				return false;
		//	case VK_NUMPAD8:				return false;
		//	case VK_NUMPAD9:				return false;
		//	case VK_DECIMAL:				return false;
		//	case VK_ADD:					return false;
		//	case VK_SUBTRACT:				return false;
		//	case VK_MULTIPLY:				return false;
		//	case VK_DIVIDE:					return false;
		//	case VK_DELETE:					return false;
			case TK_BACKSPACE:				return false;
			case TK_ENTER:					return false;
			case TK_UP://scroll navigation
				wpy-=dy*font_zoom;
				break;
			case TK_DOWN:
				wpy+=dy*font_zoom;
				break;
			case TK_LEFT://skip word
				if(cursor>0)
				{
					hist_cont=false;
					--cursor;
					char initial=group_char(text[cursor]);
					for(;cursor&&group_char(text[cursor-1])==initial;--cursor);
					calc_cursor_coords(-wpx, -wpx);
				}
				if(!get_key_state(TK_LSHIFT)&&!get_key_state(TK_RSHIFT))
					selcur=cursor, scx=ccx, scy=ccy, rectsel=false;
				wnd_to_cursor=true;
				break;
			case TK_RIGHT://skip word
				if(cursor<(int)text.size())
				{
					hist_cont=false;
					char initial=group_char(text[cursor]);
					++cursor;
					for(;cursor<(int)text.size()&&group_char(text[cursor])==initial;++cursor);
					calc_cursor_coords(-wpx, -wpx);
				}
				if(!get_key_state(TK_LSHIFT)&&!get_key_state(TK_RSHIFT))
					selcur=cursor, scx=ccx, scy=ccy, rectsel=false;
				wnd_to_cursor=true;
				break;
			case TK_HOME:
				if(cursor)
				{
					hist_cont=false;
					cursor=0, ccx=0, ccy=0;
				}
				if(!get_key_state(TK_LSHIFT)&&!get_key_state(TK_RSHIFT))
					selcur=cursor, scx=ccx, scy=ccy, rectsel=false;
				wnd_to_cursor=true;
				break;
			case TK_END:
				if(cursor<(int)text.size())
				{
					hist_cont=false;
					cursor=text.size();
					calc_cursor_coords(-wpx, -wpx);
				}
				if(!get_key_state(TK_LSHIFT)&&!get_key_state(TK_RSHIFT))
					selcur=cursor, scx=ccx, scy=ccy, rectsel=false;
				wnd_to_cursor=true;
				break;
			case TK_ESC:					return false;
			case TK_F1:						return false;
			case TK_F2:						return false;
			case TK_F3:						return false;
			case TK_F4:						return false;
			case TK_F5:						return false;
			case TK_F6:						return false;
			case TK_F7:						return false;
			case TK_F8:						return false;
			case TK_F9:						return false;
			case TK_F10:					return false;
			case TK_F11:					return false;
			case TK_F12:					return false;
			case TK_INSERT:					return false;
			case TK_TAB:					return false;
			case TK_LSHIFT:case TK_RSHIFT:	return false;
			case TK_LCTRL:case TK_RCTRL:	return false;
			case TK_LSTART:					return false;
			case TK_PAGEUP:case TK_PAGEDOWN:return false;
			default:						return false;
			}
		}
		else//ctrl is up
		{
			char character=caps_lock!=(get_key_state(TK_LSHIFT)||get_key_state(TK_RSHIFT));
			char shiftdown=get_key_state(TK_LSHIFT)||get_key_state(TK_RSHIFT);
			switch(wParam)
			{
			case TK_LALT:case TK_RALT:
				if(drag==DRAG_SELECT)
				{
					drag=DRAG_RECT, rectsel=true;
					return true;
				}
				return false;
			case TK_CAPSLOCK:				return false;
			case TK_NUMLOCK:				return false;
		//	case VK_OEM_3:					character=shiftdown	?'~':	'`'	;	break;
			case '0':						character=shiftdown	?')':	'0'	;	break;
			case '1':						character=shiftdown	?'!':	'1'	;	break;
			case '2':						character=shiftdown	?'@':	'2'	;	break;
			case '3':						character=shiftdown	?'#':	'3'	;	break;
			case '4':						character=shiftdown	?'$':	'4'	;	break;
			case '5':						character=shiftdown	?'%':	'5'	;	break;
			case '6':						character=shiftdown	?'^':	'6'	;	break;
			case '7':						character=shiftdown	?'&':	'7'	;	break;
			case '8':						character=shiftdown	?'*':	'8'	;	break;
			case '9':						character=shiftdown	?'(':	'9'	;	break;
		//	case VK_OEM_MINUS:				character=shiftdown	?'_':	'-'	;	break;
		//	case VK_OEM_PLUS:				character=shiftdown	?'+':	'='	;	break;
			case '\t':						character=						'\t';			break;
			case 'Q':						character=character				?'Q':	'q'	;	break;
			case 'W':						character=character				?'W':	'w'	;	break;
			case 'E':						character=character				?'E':	'e'	;	break;
			case 'R':						character=character				?'R':	'r'	;	break;
			case 'T':						character=character				?'T':	't'	;	break;
			case 'Y':						character=character				?'Y':	'y'	;	break;
			case 'U':						character=character				?'U':	'u'	;	break;
			case 'I':						character=character				?'I':	'i'	;	break;
			case 'O':						character=character				?'O':	'o'	;	break;
			case 'P':						character=character				?'P':	'p'	;	break;
		//	case VK_OEM_4:					character=shiftdown	?'{':	'['	;	break;
		//	case VK_OEM_6:					character=shiftdown	?'}':	']'	;	break;
			case 'A':						character=character				?'A':	'a'	;	break;
			case 'S':						character=character				?'S':	's'	;	break;
			case 'D':						character=character				?'D':	'd'	;	break;
			case 'F':						character=character				?'F':	'f'	;	break;
			case 'G':						character=character				?'G':	'g'	;	break;
			case 'H':						character=character				?'H':	'h'	;	break;
			case 'J':						character=character				?'J':	'j'	;	break;
			case 'K':						character=character				?'K':	'k'	;	break;
			case 'L':						character=character				?'L':	'l'	;	break;
		//	case VK_OEM_1:					character=shiftdown	?':':	';'	;	break;
		//	case VK_OEM_7:					character=shiftdown	?'"':	'\'';	break;
		//	case VK_OEM_5:case VK_OEM_102:	character=shiftdown	?'|':	'\\';	break;
			case 'Z':						character=character				?'Z':	'z'	;	break;
			case 'X':						character=character				?'X':	'x'	;	break;
			case 'C':						character=character				?'C':	'c'	;	break;
			case 'V':						character=character				?'V':	'v'	;	break;
			case 'B':						character=character				?'B':	'b'	;	break;
			case 'N':						character=character				?'N':	'n'	;	break;
			case 'M':						character=character				?'M':	'm'	;	break;
		//	case VK_OEM_COMMA:				character=shiftdown	?'<':	','	;	break;
		//	case VK_OEM_PERIOD:				character=shiftdown	?'>':	'.'	;	break;
		//	case VK_OEM_2:					character=shiftdown	?'?':	'/'	;	break;
		//	case VK_SPACE:					character=						' '	;			break;
		//	case VK_NUMPAD0:				character=						'0'	;			break;
		//	case VK_NUMPAD1:				character=						'1'	;			break;
		//	case VK_NUMPAD2:				character=						'2'	;			break;
		//	case VK_NUMPAD3:				character=						'3'	;			break;
		//	case VK_NUMPAD4:				character=						'4'	;			break;
		//	case VK_NUMPAD5:				character=						'5'	;			break;
		//	case VK_NUMPAD6:				character=						'6'	;			break;
		//	case VK_NUMPAD7:				character=						'7'	;			break;
		//	case VK_NUMPAD8:				character=						'8'	;			break;
		//	case VK_NUMPAD9:				character=						'9'	;			break;
		//	case VK_DECIMAL:				character=						'.'	;			break;
		//	case VK_ADD:					character=						'+'	;			break;
		//	case VK_SUBTRACT:				character=						'-'	;			break;
		//	case VK_MULTIPLY:				character=						'*'	;			break;
		//	case VK_DIVIDE:					character=						'/'	;			break;
			case TK_DELETE:
				if(rectsel)
				{
					Ranges ranges;
					int extent=calc_ranges(ranges);
					if(!extent)
					{
						for(int k=0;k<(int)ranges.size();++k)//selection trick
						{
							auto &range=ranges[k];
							range.f+=range.f<(int)text.size()&&text[range.f]!='\n';
						}
					}
					text_replace_rect(ranges, nullptr, 0);
				}
				else if(cursor!=selcur)
				{
					int start, end;
					if(selcur<cursor)
						start=selcur, end=cursor;
					else
						start=cursor, end=selcur;
					text_erase(start, end);
					//cursor=selcur=start, rectsel=false;
				}
				else
					text_erase1_del(cursor);
				return true;
			case TK_BACKSPACE://backspace
				if(rectsel)
				{
					Ranges ranges;
					int extent=calc_ranges(ranges);
					if(!extent)
					{
						for(int k=0;k<(int)ranges.size();++k)//selection trick
						{
							auto &range=ranges[k];
							range.i-=range.i>0&&text[range.i-1]!='\n';
						}
						--ccx, --scx;
					}
					text_replace_rect(ranges, nullptr, 0);
				}
				else if(cursor!=selcur)
				{
					int start, end;
					if(selcur<cursor)
						start=selcur, end=cursor;
					else
						start=cursor, end=selcur;
					text_erase(start, end);
					//cursor=selcur=start, rectsel=false;
				}
				else
					text_erase1_bksp(cursor);
				return true;
			case TK_ENTER:					character=						'\n';	break;
			case TK_UP:
				if(!get_key_state(TK_LSHIFT)&&!get_key_state(TK_RSHIFT)&&cursor!=selcur)
				{
					cursor=selcur=minimum(cursor, selcur), rectsel=false;
					calc_cursor_coords(-wpx, -wpx);
					scx=ccx, scy=ccy;
				}
				else
				{
					hist_cont=false;
					int start=find_line_start(cursor);
					if(!start)
						cursor=0;
					else
					{
						int width=calc_width(0, 0, text.c_str()+start, cursor-start, 0, font_zoom);
						int s0=find_line_start(start-1), d_idx=0;
						inv_calc_width(0, 0, text.c_str()+s0, text.size()-s0, 0, font_zoom, width, nullptr, &d_idx);
						cursor=s0+d_idx;
					}
					calc_cursor_coords(-wpx, -wpx);
					if(!get_key_state(TK_LSHIFT)&&!get_key_state(TK_RSHIFT))
						selcur=cursor, scx=ccx, scy=ccy, rectsel=false;
				}
				wnd_to_cursor=true;
				return true;
			case TK_DOWN:
				if(!get_key_state(TK_LSHIFT)&&!get_key_state(TK_RSHIFT)&&cursor!=selcur)
				{
					cursor=selcur=maximum(cursor, selcur), rectsel=false;
					calc_cursor_coords(-wpx, -wpx);
					scx=ccx, scy=ccy;
				}
				else
				{
					hist_cont=false;
					int end=find_line_end(cursor);
					if(end>=(int)text.size())
						cursor=text.size();
					else
					{
						int start=find_line_start(cursor);
						int width=calc_width(0, 0, text.c_str()+start, cursor-start, 0, font_zoom);
						int s2=end+1, d_idx=0;
						inv_calc_width(0, 0, text.c_str()+s2, text.size()-s2, 0, font_zoom, width, nullptr, &d_idx);
						cursor=s2+d_idx;
					}
					calc_cursor_coords(-wpx, -wpx);
					if(!get_key_state(TK_LSHIFT)&&!get_key_state(TK_RSHIFT))
						selcur=cursor, scx=ccx, scy=ccy, rectsel=false;
				}
				wnd_to_cursor=true;
				return true;
			case TK_LEFT:
				if(!get_key_state(TK_LSHIFT)&&!get_key_state(TK_RSHIFT)&&cursor!=selcur)
				{
					cursor=selcur=minimum(cursor, selcur), rectsel=false;
					calc_cursor_coords(-wpx, -wpx);
					scx=ccx, scy=ccy;
				}
				else
				{
					if(cursor>0)
					{
						hist_cont=false;
						--cursor;
						calc_cursor_coords(-wpx, -wpx);
					}
					if(!get_key_state(TK_LSHIFT)&&!get_key_state(TK_RSHIFT))
						selcur=cursor, scx=ccx, scy=ccy, rectsel=false;
				}
				wnd_to_cursor=true;
				return true;
			case TK_RIGHT:
				if(!get_key_state(TK_LSHIFT)&&!get_key_state(TK_RSHIFT)&&cursor!=selcur)
				{
					cursor=selcur=maximum(cursor, selcur), rectsel=false;
					calc_cursor_coords(-wpx, -wpx);
					scx=ccx, scy=ccy;
				}
				else
				{
					if(cursor<(int)text.size())
					{
						hist_cont=false;
						++cursor;
						calc_cursor_coords(-wpx, -wpx);
					}
					if(!get_key_state(TK_LSHIFT)&&!get_key_state(TK_RSHIFT))
						selcur=cursor, scx=ccx, scy=ccy, rectsel=false;
				}
				wnd_to_cursor=true;
				return true;
			case TK_HOME:
				{
					int c2=find_line_start(cursor);
					if(cursor!=c2)
					{
						cursor=c2, hist_cont=false;
						calc_cursor_coords(-wpx, -wpx);
					}
					if(!get_key_state(TK_LSHIFT)&&!get_key_state(TK_RSHIFT))
						selcur=cursor, scx=ccx, scy=ccy, rectsel=false;
					wnd_to_cursor=true;
				}
				return true;
			case TK_END:
				{
					int c2=find_line_end(cursor);
					if(cursor!=c2)
					{
						cursor=c2, hist_cont=false;
						calc_cursor_coords(-wpx, -wpx);
					}
					if(!get_key_state(TK_LSHIFT)&&!get_key_state(TK_RSHIFT))
						selcur=cursor, scx=ccx, scy=ccy, rectsel=false;
					wnd_to_cursor=true;
				}
				return true;
			case TK_ESC://deselect
				selcur=cursor, scx=ccx, scy=ccy, rectsel=false;
				wnd_to_cursor=true;
				return true;
			case TK_F1:
				display_help();
				return false;
			case TK_F2:
			case TK_F3:
				return false;
			case TK_F4:
				prof_toggle();
				return true;
			case TK_F5:
			case TK_F6:
			case TK_F7:
			case TK_F8:
			case TK_F9:
			case TK_F10:
			case TK_F11:
			case TK_F12:
			case TK_INSERT:
				return false;
			case TK_LSHIFT:case TK_RSHIFT:	return false;
			case TK_LCTRL:case TK_RCTRL:	return false;
			case TK_LSTART:					return false;
			case TK_PAGEUP:case TK_PAGEDOWN:return false;
			default:						return false;
			}
			if(rectsel)
			{
				Ranges ranges;
				calc_ranges(ranges);
				text_replace_rect(ranges, &character, 1);
			}
			else if(cursor!=selcur)
			{
				int start, end;
				if(selcur<cursor)
					start=selcur, end=cursor;
				else
					start=cursor, end=selcur;
				text_replace(start, end, &character, 1);
				//cursor=selcur=start, rectsel=false;
			}
			else
				text_insert1(cursor, character);
		}
#endif
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		break;
	}
	return true;
}
#endif
bool				wnd_on_quit()//return false to deny exit
{
	return true;
}