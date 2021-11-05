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

void				display_help()
{
	messageboxa("Controls",
		"Ctrl+Up/Down:\tScroll\n"
		"Ctrl+Left/Right:\tSkip word\n"
		"\n"
		"Shift+Arrows/Home/End:\tSelect\n"
		"Ctrl A:\t\t\tSelect all\n"
		"Ctrl+Drag:\t\t\tRectangular selection\n"
		"Shift+Drag:\t\tResize selection\n"
		"Ctrl+Shift+Left/Right:\tSelect words\n"
		"Escape:\t\t\tDeselect\n"
		"\n"
		"Ctrl Z:\tUndo\n"
		"Ctrl Y:\tRedo\n"
		"Ctrl X:\tCut\n"
		"Ctrl C:\tCopy\n"
		"Ctrl V:\tPaste");
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
		if(c=='\t')
			width=tab_width-(x+msg_width-tab_origin)%tab_width, c=' ';
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
int					inv_calc_width(short x, short y, const char *msg, int msg_length, short tab_origin, short zoom, int width)//returns index to fit [msg, msg+index[ in width
{
	if(msg_length<1)
		return 0;
	int msg_width=0, temp_width, printable_count=0, tab_width=tab_count*dx*zoom, w2=dx*zoom, k=0;
	for(;k<msg_length;++k)
	{
		char c=msg[k];
		if(c=='\n')
			return k-1;
		if(c=='\t')
			temp_width=tab_width-(x+msg_width-tab_origin)%tab_width, c=' ';
		else if(c>=32&&c<0xFF)
			temp_width=w2;
		else
			temp_width=0;
		if(temp_width)
		{
			int sum=msg_width+temp_width;
			if(sum>width)
				return k;
			if(sum==width)
				return k+1;
			msg_width=sum;
			++printable_count;
		}
	}
	return k;
}
int					print_line(short x, short y, const char *msg, int msg_length, short tab_origin, short zoom)
{
	if(msg_length<1)
		return 0;
	float rect[4]={};
	QuadCoords *txc=nullptr;
	int msg_width=0, width, idx, printable_count=0, tab_width=tab_count*dx*zoom, w2=dx*zoom, height=dy*zoom;
	vrtx.resize(msg_length<<4);//vx, vy, txx, txy		x4 vertices/char
	for(int k=0;k<msg_length;++k)
	{
		char c=msg[k];
		if(c=='\t')
			width=tab_width-(x+msg_width-tab_origin)%tab_width, c=' ';
		else if(c>=32&&c<0xFF)
			width=w2;
		else
			width=0;
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

//U64				colors_text=0xFFABABABFF000000;//black on white
U64					colors_text=0xFF000000FFABABAB;//dark mode
U64					colors_selection=0xFFFF0000FF000000;
short				font_zoom=1,//font pixel size
					cx=0, cy=0;//cursor coordinates
int					wx=0, wy=0, nlines=1, text_width=0, cursor=0, selcur=0;
bool				vertical_mode=false;
std::string			text;
//std::string		text="Hello.\nSample Text.\nWhat\'s going on???\n";
char				caps_lock=false;

enum				DragType
{
	DRAG_NONE,
	DRAG_SELECT,
};
DragType			drag;
//void				insert(char c)
//{
//	text.insert(text.begin()+cursor, c);
//	++cursor;
//	selcur=cursor;
//}

enum				MoveType
{
	M_UNINITIALIZED,
	M_INSERT,
	M_ERASE,
	M_ERASE_SELECTION,
};
struct				History
{
	MoveType type;
	int idx;
	std::string data;
	History():type(M_UNINITIALIZED), idx(0){}
	History(History const &h):type(h.type), idx(h.idx), data(h.data){}
	History(History &&h):type(h.type), idx(h.idx), data((std::string&&)h.data){}
	History(MoveType type, int idx, const char *a, int len):type(type), idx(idx), data(a, a+len){}
};
/*struct			History
{
	int idx;
	bool continuous, insert;
	char c;
};//*/
std::vector<History> history;
int					histpos=-1;//pointing at previous action
bool				hist_cont=true;

void				cursor_teleport()//preset the cursor before calling, to scroll window till cursor is visible
{
	//hist_cont=false;
	//if(!keyboard[VK_SHIFT])
	//	selcur=cursor;
}
void				hist_undo()//ctrl z
{
	if(histpos<0)
		return;
	auto &h=history[histpos];
	if(h.type==M_INSERT)
	{
		text.erase(text.begin()+h.idx, text.begin()+h.idx+h.data.size());
		selcur=cursor=h.idx;
	}
	else if(h.type==M_ERASE)
	{
		text.insert(text.begin()+h.idx, h.data.begin(), h.data.end());
		selcur=cursor=h.idx+h.data.size();
	}
	else if(h.type==M_ERASE_SELECTION)
	{
		text.insert(text.begin()+h.idx, h.data.begin(), h.data.end());
		selcur=h.idx, cursor=h.idx+h.data.size();
	}
	--histpos;

	hist_cont=false;
	cursor_teleport();
}
void				hist_redo()//ctrl y
{
	if(histpos>=(int)history.size()-1)
		return;
	++histpos;
	auto &h=history[histpos];
	if(h.type==M_INSERT)
	{
		text.insert(text.begin()+h.idx, h.data.begin(), h.data.end());
		cursor=selcur=h.idx+h.data.size();
		//selcur=h.idx, cursor=h.idx+h.data.size();
	}
	else if(h.type==M_ERASE||h.type==M_ERASE_SELECTION)
	{
		text.erase(text.begin()+h.idx, text.begin()+h.idx+h.data.size());
		cursor=selcur=h.idx;
	}

	hist_cont=false;
	cursor_teleport();
}
void				text_replace(int i, int f, const char *a, int len)
{
	history.resize(histpos+1);
	history.push_back(History(M_ERASE_SELECTION, i, text.c_str()+i, f-i));

	text.replace(text.begin()+i, text.end()+f, a, a+len);

	history.push_back(History(M_INSERT, i, text.c_str()+i, len));
	histpos+=2;
	
	hist_cont=false;
	cursor=selcur=i+len;
	cursor_teleport();
}
void				text_insert(int i, const char *a, int len)
{
	text.insert(text.begin()+i, a, a+len);

	++histpos;
	history.resize(histpos);
	history.push_back(History(M_INSERT, i, text.c_str()+i, len));

	hist_cont=false;
	cursor=selcur=i+len;
	cursor_teleport();
}
void				text_erase(int i, int f)
{
	++histpos;
	history.resize(histpos);
	history.push_back(History(f-i>2?M_ERASE_SELECTION:M_ERASE, i, text.c_str()+i, f-i));

	text.erase(text.begin()+i, text.begin()+f);

	hist_cont=false;
	cursor=selcur=i;
	cursor_teleport();
}
void				text_insert1(int i, char c)
{
	text.insert(text.begin()+i, c);

	if(!hist_cont||c=='\n'||histpos<0||history[histpos].type==M_ERASE||history[histpos].type==M_ERASE_SELECTION)
	{
		++histpos;
		history.resize(histpos);
		history.push_back(History(M_INSERT, i, text.c_str()+i, 1));
	}
	else
		history[histpos].data.push_back(c);

	hist_cont=true;
	cursor=selcur=i+1;
	cursor_teleport();
}
void				text_erase1_bksp(int i)
{
	if(i<1)
		return;
	if(!hist_cont||histpos<0||history[histpos].type==M_INSERT)
	{
		++histpos;
		history.resize(histpos);
		history.push_back(History(M_ERASE, i-1, text.c_str()+i-1, 1));
	}
	else
	{
		auto &str=history[histpos].data;
		str.insert(str.begin(), text[i]);
	}

	text.erase(text.begin()+i-1);

	hist_cont=true;
	cursor=selcur=i-1;
	cursor_teleport();
}
void				text_erase1_del(int i)
{
	if(i>=(int)text.size())
		return;
	if(!hist_cont||histpos<0||history[histpos].type==M_INSERT)
	{
		++histpos;
		history.resize(histpos);
		history.push_back(History(M_ERASE, i, text.c_str()+i, 1));
	}
	else
		history[histpos].data.push_back(text[i]);

	text.erase(text.begin()+i);
	
	hist_cont=true;
	cursor=selcur=i;
	cursor_teleport();
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

void				copy_to_clipboard(std::string const &str){copy_to_clipboard(str.c_str(), str.size());}
void				buffer2clipboard(int *buffer, int bw, int bh)
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

	glClear(GL_COLOR_BUFFER_BIT);

	int selstart, selend;
	if(cursor<selcur)
		selstart=cursor, selend=selcur;
	else
		selstart=selcur, selend=cursor;
	int y=0, font_height=dy*font_zoom;
	nlines=1, text_width=0;
	for(int start=0, line_start=0, x=0, k=0;;++k)
	{
		if(k==selstart)
		{
			x+=print_line(x, y, text.c_str()+start, k-start, 0, font_zoom);
			set_text_colors(colors_selection);
			start=k;
			if(k==cursor)
				cx=x, cy=nlines-1;
		}
		if(k==selend)
		{
			x+=print_line(x, y, text.c_str()+start, k-start, 0, font_zoom);
			set_text_colors(colors_text);
			start=k;
			if(k==cursor)
				cx=x, cy=nlines-1;
		}
		if(k>=(int)text.size()||text[k]=='\n')
		{
			x+=print_line(x, y, text.c_str()+start, k-start, 0, font_zoom);
			if(text_width<x)
				text_width=x;
			y+=font_height;
			if(k>=(int)text.size())
				break;
			line_start=start=k+1;
			++nlines, x=0;
		}
	}
	cy*=dy*font_zoom;
	draw_line_i(cx, cy, cx, cy+dy*font_zoom, 0xFFFFFFFF);

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

	update_screen();
	prof_add("Swap");
	prof_print();
}
bool			wnd_on_input(HWND hWnd, int message, int wParam, int lParam)
{
	switch(message)
	{
	case WM_MOUSEMOVE:
		return false;

	case WM_MOUSEWHEEL:
		if(keyboard[VK_CONTROL])
		{
			bool mw_forward=((short*)&wParam)[1]>0;
			if(mw_forward)
			{
				if(font_zoom<32)
					font_zoom<<=1;
			}
			else
			{
				if(font_zoom>1)
					font_zoom>>=1;
			}
		}
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
#if 1
		if(keyboard[VK_CONTROL])
		{
			switch(wParam)
			{
			case VK_CAPITAL:				return false;
			case VK_NUMLOCK:				return false;
			case VK_OEM_3:					return false;
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
			case VK_OEM_MINUS:				return false;
			case VK_OEM_PLUS:				return false;
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
			case 'O':						return false;
			case 'P':						return false;
			case VK_OEM_4:					return false;
			case VK_OEM_6:					return false;
			case 'A':
				hist_cont=false;
				selcur=0, cursor=text.size();
				cursor_teleport();
				break;
			case 'S':
				//TODO: save dialog
				break;
			case 'D'://dump history
				histpos=-1, history.clear();
				break;
			case 'F':						return false;
			case 'G':						return false;
			case 'H':						return false;
			case 'J':						return false;
			case 'K':						return false;
			case 'L':						return false;
			case VK_OEM_1:					return false;
			case VK_OEM_7:					return false;
			case VK_OEM_5:case VK_OEM_102:	return false;
			case 'Z':
				hist_undo();
				break;
			case 'X':
			case 'C':
				if(cursor!=selcur)
				{
					int start, end;
					if(selcur<cursor)
						start=selcur, end=cursor;
					else
						start=cursor, end=selcur;
					copy_to_clipboard(text.c_str()+start, end-start);
				}
				break;
			case 'V':
				if(cursor!=selcur)
				{
					int start, end;
					if(selcur<cursor)
						start=selcur, end=cursor;
					else
						start=cursor, end=selcur;
					
					OpenClipboard(hWnd);
					char *a=(char*)GetClipboardData(CF_OEMTEXT);
					if(!a)
					{
						CloseClipboard();
						return true;
					}

					int len=strlen(a);
					text_replace(start, end, a, len);
					cursor=selcur=start+len;

					CloseClipboard();
				}
				break;
			case 'B':						return false;
			case 'N':						return false;
			case 'M':						return false;
			case VK_OEM_COMMA:				return false;
			case VK_OEM_PERIOD:				return false;
			case VK_OEM_2:					return false;
			case VK_SPACE:					return false;
			case VK_NUMPAD0:				return false;
			case VK_NUMPAD1:				return false;
			case VK_NUMPAD2:				return false;
			case VK_NUMPAD3:				return false;
			case VK_NUMPAD4:				return false;
			case VK_NUMPAD5:				return false;
			case VK_NUMPAD6:				return false;
			case VK_NUMPAD7:				return false;
			case VK_NUMPAD8:				return false;
			case VK_NUMPAD9:				return false;
			case VK_DECIMAL:				return false;
			case VK_ADD:					return false;
			case VK_SUBTRACT:				return false;
			case VK_MULTIPLY:				return false;
			case VK_DIVIDE:					return false;
			case VK_DELETE:					return false;
			case VK_BACK:					return false;
			case VK_RETURN:					return false;
			case VK_UP://TODO: scroll navigation
				//if(th>bh)
				//{
				//	tpy=tpy-fontH<0?0:tpy-fontH;
				//	return 1;
				//}
				break;
			case VK_DOWN:
				//if(th>bh)
				//{
				//	tpy=tpy+fontH>th-bh?th-bh:tpy+fontH;
				//	return 1;
				//}
				break;
			case VK_LEFT:
			/*	if(cursor)
				{
					char initial=mapChar(text[--cursor]);
					for(;cursor;--cursor)
						if(mapChar(text[cursor-1])!=initial)
							break;
					cont=0;
					cursorTeleport();
					return 1;
				}
				if(selcur!=cursor&&!keyboard[VK_SHIFT])
				{
					selcur=cursor;
					return 1;
				}//*/
				break;
			case VK_RIGHT:
			/*	if(cursor!=textlen)
				{
					char initial=mapChar(text[cursor++]);
					for(;cursor!=textlen;++cursor)
						if(mapChar(text[cursor])!=initial)
							break;
					cont=0;
					cursorTeleport();
					return 1;
				}
				if(selcur!=cursor&&!keyboard[VK_SHIFT])
				{
					selcur=cursor;
					return 1;
				}//*/
				break;
			case VK_HOME:
			/*	if(cursor)
				{
					cursor=0, cont=0;
					cursorTeleport();
					return 1;
				}
				if(selcur!=cursor&&!keyboard[VK_SHIFT])
				{
					selcur=cursor;
					return 1;
				}//*/
				break;
			case VK_END:
			/*	if(cursor!=textlen)
				{
					cursor=textlen, cont=0;
					cursorTeleport();
					return 1;
				}
				if(selcur!=cursor&&!keyboard[VK_SHIFT])
				{
					selcur=cursor;
					return 1;
				}//*/
				break;
			case VK_ESCAPE:case VK_F1:case VK_F2:case VK_F3:case VK_F4:case VK_F5:case VK_F6:case VK_F7:case VK_F8:case VK_F9:case VK_F10:case VK_F11:case VK_F12:case VK_INSERT:	return false;
			case VK_F13:case VK_F14:case VK_F15:case VK_F16:case VK_F17:case VK_F18:case VK_F19:case VK_F20:case VK_F21:case VK_F22:case VK_F23:case VK_F24:						return false;
			case VK_TAB:					return false;
			case VK_SHIFT:					return false;
			case VK_CONTROL:case VK_LWIN:	return false;
			case VK_PRIOR:case VK_NEXT:		return false;
			default:						return false;
			}
		}
		else//ctrl is up
		{
			char character=caps_lock!=keyboard[VK_SHIFT];
			switch(wParam)
			{
			case VK_CAPITAL:				return false;
			case VK_NUMLOCK:				return false;
			case VK_OEM_3:					character=keyboard[VK_SHIFT]	?'~':	'`'	;	break;
			case '0':						character=keyboard[VK_SHIFT]	?')':	'0'	;	break;
			case '1':						character=keyboard[VK_SHIFT]	?'!':	'1'	;	break;
			case '2':						character=keyboard[VK_SHIFT]	?'@':	'2'	;	break;
			case '3':						character=keyboard[VK_SHIFT]	?'#':	'3'	;	break;
			case '4':						character=keyboard[VK_SHIFT]	?'$':	'4'	;	break;
			case '5':						character=keyboard[VK_SHIFT]	?'%':	'5'	;	break;
			case '6':						character=keyboard[VK_SHIFT]	?'^':	'6'	;	break;
			case '7':						character=keyboard[VK_SHIFT]	?'&':	'7'	;	break;
			case '8':						character=keyboard[VK_SHIFT]	?'*':	'8'	;	break;
			case '9':						character=keyboard[VK_SHIFT]	?'(':	'9'	;	break;
			case VK_OEM_MINUS:				character=keyboard[VK_SHIFT]	?'_':	'-'	;	break;
			case VK_OEM_PLUS:				character=keyboard[VK_SHIFT]	?'+':	'='	;	break;
			case VK_TAB:					character=						'\t';			break;
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
			case VK_OEM_4:					character=keyboard[VK_SHIFT]	?'{':	'['	;	break;
			case VK_OEM_6:					character=keyboard[VK_SHIFT]	?'}':	']'	;	break;
			case 'A':						character=character				?'A':	'a'	;	break;
			case 'S':						character=character				?'S':	's'	;	break;
			case 'D':						character=character				?'D':	'd'	;	break;
			case 'F':						character=character				?'F':	'f'	;	break;
			case 'G':						character=character				?'G':	'g'	;	break;
			case 'H':						character=character				?'H':	'h'	;	break;
			case 'J':						character=character				?'J':	'j'	;	break;
			case 'K':						character=character				?'K':	'k'	;	break;
			case 'L':						character=character				?'L':	'l'	;	break;
			case VK_OEM_1:					character=keyboard[VK_SHIFT]	?':':	';'	;	break;
			case VK_OEM_7:					character=keyboard[VK_SHIFT]	?'"':	'\'';	break;
			case VK_OEM_5:case VK_OEM_102:	character=keyboard[VK_SHIFT]	?'|':	'\\';	break;
			case 'Z':						character=character				?'Z':	'z'	;	break;
			case 'X':						character=character				?'X':	'x'	;	break;
			case 'C':						character=character				?'C':	'c'	;	break;
			case 'V':						character=character				?'V':	'v'	;	break;
			case 'B':						character=character				?'B':	'b'	;	break;
			case 'N':						character=character				?'N':	'n'	;	break;
			case 'M':						character=character				?'M':	'm'	;	break;
			case VK_OEM_COMMA:				character=keyboard[VK_SHIFT]	?'<':	','	;	break;
			case VK_OEM_PERIOD:				character=keyboard[VK_SHIFT]	?'>':	'.'	;	break;
			case VK_OEM_2:					character=keyboard[VK_SHIFT]	?'?':	'/'	;	break;
			case VK_SPACE:					character=						' '	;			break;
			case VK_NUMPAD0:				character=						'0'	;			break;
			case VK_NUMPAD1:				character=						'1'	;			break;
			case VK_NUMPAD2:				character=						'2'	;			break;
			case VK_NUMPAD3:				character=						'3'	;			break;
			case VK_NUMPAD4:				character=						'4'	;			break;
			case VK_NUMPAD5:				character=						'5'	;			break;
			case VK_NUMPAD6:				character=						'6'	;			break;
			case VK_NUMPAD7:				character=						'7'	;			break;
			case VK_NUMPAD8:				character=						'8'	;			break;
			case VK_NUMPAD9:				character=						'9'	;			break;
			case VK_DECIMAL:				character=						'.'	;			break;
			case VK_ADD:					character=						'+'	;			break;
			case VK_SUBTRACT:				character=						'-'	;			break;
			case VK_MULTIPLY:				character=						'*'	;			break;
			case VK_DIVIDE:					character=						'/'	;			break;
			case VK_DELETE:
				if(cursor!=selcur)
				{
					int start, end;
					if(selcur<cursor)
						start=selcur, end=cursor;
					else
						start=cursor, end=selcur;
					text_erase(start, end);
					cursor=selcur=start;
				}
				else
					text_erase1_del(cursor);
				return true;
			case VK_BACK://backspace
				if(cursor!=selcur)
				{
					int start, end;
					if(selcur<cursor)
						start=selcur, end=cursor;
					else
						start=cursor, end=selcur;
					text_erase(start, end);
					cursor=selcur=start;
				}
				else
					text_erase1_bksp(cursor);
				return true;
			case VK_RETURN:					character=						'\n';	break;
			case VK_UP:
				{
					hist_cont=false;
					if(!keyboard[VK_SHIFT]&&cursor!=selcur)
						cursor=minimum(cursor, selcur);
					int start=find_line_start(cursor);
					if(!start)
						cursor=0;
					else
					{
						int width=calc_width(0, 0, text.c_str()+start, cursor-start, 0, font_zoom);
						int s0=find_line_start(start-1);
						cursor=s0+inv_calc_width(0, 0, text.c_str()+s0, text.size()-s0, 0, font_zoom, width);
					}
					if(!keyboard[VK_SHIFT])
						selcur=cursor;
					cursor_teleport();
				}
				return true;
			case VK_DOWN:
				{
					hist_cont=false;
					if(!keyboard[VK_SHIFT]&&cursor!=selcur)
						cursor=maximum(cursor, selcur);
					int end=find_line_end(cursor);
					if(end>=(int)text.size())
						cursor=text.size();
					else
					{
						int start=find_line_start(cursor);
						int width=calc_width(0, 0, text.c_str()+start, cursor-start, 0, font_zoom);
						int s2=end+1;
						cursor=s2+inv_calc_width(0, 0, text.c_str()+s2, text.size()-s2, 0, font_zoom, width);
					}
					if(!keyboard[VK_SHIFT])
						selcur=cursor;
					cursor_teleport();
				}
				return true;
			case VK_LEFT:
				if(cursor>0)
				{
					hist_cont=false;
					--cursor;
				}
				if(!keyboard[VK_SHIFT])
					selcur=cursor;
				cursor_teleport();
				return true;
			case VK_RIGHT:
				if(cursor<(int)text.size())
				{
					hist_cont=false;
					++cursor;
				}
				if(!keyboard[VK_SHIFT])
					selcur=cursor;
				cursor_teleport();
				return true;
			case VK_HOME:
				{
					int c2=find_line_start(cursor);
					if(cursor!=c2)
						cursor=c2, hist_cont=false;
					if(!keyboard[VK_SHIFT])
						selcur=cursor;
					cursor_teleport();
				}
				return true;
			case VK_END:
				{
					int c2=find_line_end(cursor);
					if(cursor!=c2)
						cursor=c2, hist_cont=false;
					if(!keyboard[VK_SHIFT])
						selcur=cursor;
					cursor_teleport();
				}
				return true;
			case VK_ESCAPE://deselect
				selcur=cursor;
				cursor_teleport();
				return true;
			case VK_F1:
				display_help();
				return false;
			case VK_F2:
			case VK_F3:
				return false;
			case VK_F4:
				prof_toggle();
				return true;
			case VK_F5:case VK_F6:case VK_F7:case VK_F8:case VK_F9:case VK_F10:case VK_F11:case VK_F12:case VK_INSERT:	return false;
			case VK_F13:case VK_F14:case VK_F15:case VK_F16:case VK_F17:case VK_F18:case VK_F19:case VK_F20:case VK_F21:case VK_F22:case VK_F23:case VK_F24:	return false;
			case VK_SHIFT:					return false;
			case VK_CONTROL:case VK_LWIN:	return false;
			case VK_PRIOR:case VK_NEXT:		return false;
			default:						return false;
			}
			if(cursor!=selcur)
			{
				int start, end;
				if(selcur<cursor)
					start=selcur, end=cursor;
				else
					start=cursor, end=selcur;
				text_erase(start, end);
				cursor=selcur=start;
			}
			else
				text_insert1(cursor, character);
			//insert(character);
		/*	if(character>0)
			{
				int start, end;
				if(cursor<selcur)
					start=cursor, end=selcur;
				else
					start=selcur, end=cursor;
				if(character!='\r')
					replaceText(start, end, &character, 1), cont=1;
				else
				{
					static const char newline[]="\r\n";
					replaceText(start, end, newline, 2), cont=0;
				}
			//	toTeleportCursor=true;
				cursorTeleport();
				return 2;
			}//*/
		}
#endif
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		break;
	}
	return true;
}
bool				wnd_on_quit()//return false to deny exit
{
	return true;
}