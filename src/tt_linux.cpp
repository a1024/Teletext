//tt_system.cpp - Linux-dependent code
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

#ifdef __linux__
#include			"tt.h"
#include			<GL/glx.h>
#include			<string.h>

#include			<stdio.h>
#include			<stdarg.h>
const char			file[]=__FILE__;

	#define			PRINT_TO_CONSOLE

//void				get_window_title_w(wchar_t *str, int size){}
//void				set_window_title_w(const wchar_t *str){}
//void				set_window_title_a(const char *str){}
//int				GUINPrint(int x, int y, int w, int h, const char *a, ...){return 0;}
//long				GUITPrint(int x, int y, const char *a, ...){return 0;}
//void				GUIPrint(int x, int y, const char *a, ...){}
bool				get_key_state(int key)
{
	//XkbGetState();
	return false;//
}
static Display		*dpy=nullptr;
static unsigned long win=0;
const char*			xevent2str(int event_type)
{
	const char *a=nullptr;
	switch(event_type)
	{
	case 2:a="KeyPress";break;//https://stackoverflow.com/questions/35745561/how-can-i-convert-an-xevent-to-a-string
	case 3:a="KeyRelease";break;
	case 4:a="ButtonPress";break;
	case 5:a="ButtonRelease";break;
	case 6:a="MotionNotify";break;
	case 7:a="EnterNotify";break;
	case 8:a="LeaveNotify";break;
	case 9:a="FocusIn";break;
	case 10:a="FocusOut";break;
	case 11:a="KeymapNotify";break;
	case 12:a="Expose";break;
	case 13:a="GraphicsExpose";break;
	case 14:a="NoExpose";break;
	case 15:a="VisibilityNotify";break;
	case 16:a="CreateNotify";break;
	case 17:a="DestroyNotify";break;
	case 18:a="UnmapNotify";break;
	case 19:a="MapNotify";break;
	case 20:a="MapRequest";break;
	case 21:a="ReparentNotify";break;
	case 22:a="ConfigureNotify";break;
	case 23:a="ConfigureRequest";break;
	case 24:a="GravityNotify";break;
	case 25:a="ResizeRequest";break;
	case 26:a="CirculateNotify";break;
	case 27:a="CirculateRequest";break;
	case 28:a="PropertyNotify";break;
	case 29:a="SelectionClear";break;
	case 30:a="SelectionRequest";break;
	case 31:a="SelectionNotify";break;
	case 32:a="ColormapNotify";break;
	case 33:a="ClientMessage";break;
	case 34:a="MappingNotify";break;
	case 35:a="GenericEvent";break;
	case 36:a="LASTEvent";break;
	}
	if(!a)
		return "<Undefined>";
	return a;
}
/*void				messageboxw(const wchar_t *title, const wchar_t *format, ...)
{
#ifdef PRINT_TO_CONSOLE
	wprintf(L"%s:\n", title);
	va_list args;
	va_start(args, format);
	vwprintf(format, args);
	va_end(args);
#endif
}//*/
void				messagebox(const char *title, const char *format, ...)
{
#ifdef PRINT_TO_CONSOLE
	printf("%s:\n", title);
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
#endif
}
void				copy_to_clipboard_c(const char *a, int size)//size not including null terminator
{
#ifdef PRINT_TO_CONSOLE
	printf("Copy to clipboard:\n%.*s\n", size, a);
#endif
}
bool				paste_from_clipboard(char *&str, int &len)
{
	str=nullptr, len=0;
/*	OpenClipboard(ghWnd);
	char *a=(char*)GetClipboardData(CF_OEMTEXT);
	if(!a)
	{
		CloseClipboard();
		messageboxa("Error", "Failed to paste from clipboard");
		return false;
	}
	int len0=strlen(a);

	str=new char[len0+1];
	len=0;
	for(int k2=0;k2<len0;++k2)
	{
		if(a[k2]!='\r')
			str[len]=a[k2], ++len;
	}
	str[len]='\0';

	CloseClipboard();//*/
	return true;
}
void				get_window_title(char *str, int size)
{
	//GetWindowTextW(ghWnd, str, size);
}
void				set_window_title(const char *str)
{
	//int success=SetWindowTextW(ghWnd, str);
	//SYS_ASSERT(success);
}
//void				set_window_title_a(const char *str)
//{
//	//int success=SetWindowTextA(ghWnd, str);
//	//SYS_ASSERT(success);
//}
int					GUINPrint(int x, int y, int w, int h, const char *a, ...)//should support newlines
{
#ifdef PRINT_TO_CONSOLE
	va_list args;
	va_start(args, a);
	int printed=vprintf(a, args);
	va_end(args);
	printf("\n");
	return printed;
#else
	return 0;
#endif

	//int length=vsprintf_s(g_buf, g_buf_size, a, (char*)(&a+1));
	//if(length>0)
	//{
	//	RECT r={x, y, x+w, y+h};
	//	return DrawTextA(ghDC, g_buf, length, &r, DT_NOCLIP|DT_EXPANDTABS);
	//}
	//return 0;
}
long				GUITPrint(int x, int y, const char *a, ...)//return value: 0xHHHHWWWW		width=(short&)ret, height=((short*)&ret)[1]
{
#ifdef PRINT_TO_CONSOLE
	va_list args;
	va_start(args, a);
	int printed=vprintf(a, args);
	va_end(args);
	printf("\n");
	return printed;
#else
	return 0;
#endif

	//int length=vsprintf_s(g_buf, g_buf_size, a, (char*)(&a+1));
	//if(length>0)
	//	return TabbedTextOutA(ghDC, x, y, g_buf, length, 0, 0, 0);
	//return 0;
}
void				GUIPrint(int x, int y, const char *a, ...)//no newlines nor tabs
{
#ifdef PRINT_TO_CONSOLE
	va_list args;
	va_start(args, a);
	int printed=vprintf(a, args);
	va_end(args);
	printf("\n");
#endif

	//int length=vsprintf_s(g_buf, g_buf_size, a, (char*)(&a+1)), success;
	//if(length>0)
	//{
	//	success=TextOutA(ghDC, x, y, g_buf, length);	SYS_ASSERT(success);
	//}
}
const char*			open_file_dialog()
{
	return nullptr;
#if 0
	g_wbuf[0]=0;
	OPENFILENAMEW ofn=
	{
		sizeof(OPENFILENAMEW),
		ghWnd, ghInstance,
		L"All Files (*.*)\0*.*\0", 0, 0, 1,
		g_wbuf, g_buf_size,
		0, 0,//initial filename
		0,
		0,//dialog title
		OFN_CREATEPROMPT|OFN_PATHMUSTEXIST,
		0,//file offset
		0,//extension offset
		L"TXT",//default extension
		0, 0,//data & hook
		0,//template name
		0,//reserved
		0,//reserved
		0,//flags ex
	};
	int success=GetOpenFileNameW(&ofn);
	if(!success)
		return 0;
	memcpy(g_wbuf, ofn.lpstrFile, wcslen(ofn.lpstrFile)*sizeof(wchar_t));
	return g_wbuf;
#endif
}
const char*			save_file_dialog()
{
	return 0;
#if 0
	g_wbuf[0]=0;
	OPENFILENAMEW ofn=
	{
		sizeof(OPENFILENAMEW),
		ghWnd, ghInstance,
		L"Text File (.txt)\0*.txt\0"
		L"C/C++ Source (.c; .cpp)\0*.txt\0"
		L"C/C++ Header (.h; .hpp)\0*.txt\0", 0, 0, 1,
		g_wbuf, g_buf_size,
		0, 0,//initial filename
		0,
		0,//dialog title
		OFN_CREATEPROMPT|OFN_PATHMUSTEXIST,
		0,//file offset
		0,//extension offset
		L"TXT",//default extension
		0, 0,//data & hook
		0,//template name
		0,//reserved
		0,//reserved
		0,//flags ex
	};
	int success=GetSaveFileNameW(&ofn);
	if(!success)
		return nullptr;
	memcpy(g_wbuf, ofn.lpstrFile, wcslen(ofn.lpstrFile)*sizeof(wchar_t));
	return g_wbuf;
#endif
}
bool				open_text_file(const char *filename, std::string &str)
{
	FILE *file=fopen(filename, "r");
	if(!file)
	{
		messagebox("Error", "Cannot open \'%s\'.\n%s", filename, strerror(errno));
		return false;
	}
	fseek(file, 0, SEEK_END);
	int bytesize=ftell(file);
	fseek(file, 0, SEEK_SET);
	str.resize(bytesize);
	fread(&str[0], 1, bytesize, file);
	fclose(file);
	str.resize(strlen(str.c_str()));//remove extra null terminators at the end
	return true;
}
bool				save_text_file(const char *filename, std::string &str)
{
	FILE *file=fopen(filename, "w");
	if(!file)
	{
		messagebox("Error", "Cannot save \'%s\'.\n%s", filename, strerror(errno));
		return false;
	}
	fwrite(str.c_str(), 1, str.size(), file);
	fclose(file);
	return true;
}
int					ask_to_save()
{
	return 0;
/*	if(filename.size())
		swprintf_s(g_wbuf, L"Save changes to %s?", filename.c_str());
	else
		swprintf_s(g_wbuf, L"Save changes to Untitled?");
	int result=MessageBoxW(ghWnd, g_wbuf, L"Paint++", MB_YESNOCANCEL);
	return result;//*/
}
void				mouse_capture()
{
}
void				mouse_release()
{
}

void				swap_buffers()
{
	glXSwapBuffers(dpy, win);
}

float				level=0, increment=0.1f;
/*void				render()
{
	glClearColor(level, level, level, 1);
	level+=increment;
	if(level<0)
		level=0, increment=-increment;
	if(level>1)
		level=1, increment=-increment;
	glClear(GL_COLOR_BUFFER_BIT);
	swap_buffers();
}//*/

int					main(int argc, char **argv)
{
	//printf("%ld\n", sizeof(long));//8

	//char LOL_1[4];
	//memset(LOL_1, 0, 4);
	//LOL_1[1]='a';
	//printf("%s\n", LOL_1);

	//std::string str="Hello";
	//str+=" World";
	//for(int k=0;k<(int)str.size();++k)
	//	printf("%c", str[k]);

#if 1//https://www.khronos.org/opengl/wiki/Programming_OpenGL_in_Linux:_GLX_and_Xlib
	dpy=XOpenDisplay(nullptr);
	if(!dpy)
	{
		printf("Cannot connect to X11 server.\n");
		return 1;
	}
	unsigned long root=DefaultRootWindow(dpy);
	int att[]={GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};
	auto vi=glXChooseVisual(dpy, 0, att);
	if(!vi)
	{
		printf("No appropriate visual found\n");
		return 1;
	}
	auto cmap=XCreateColormap(dpy, root, vi->visual, AllocNone);
	XSetWindowAttributes swa={};
	swa.event_mask=ExposureMask|PointerMotionMask|KeyReleaseMask|KeyPressMask;
	swa.colormap=cmap;
	//win=XCreateSimpleWindow(dpy, root, 0, 0, w=640, h=480);
	win=XCreateWindow(dpy, root, 0, 0, w=640, h=480, 0, vi->depth, InputOutput, vi->visual, CWColormap|CWEventMask, &swa);
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "Teletext");
	auto glc=glXCreateContext(dpy, vi, nullptr, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);

	GLversion=(const char*)glGetString(GL_VERSION);		GL_CHECK();//get OpenGL version
	//glGetIntegerv(GL_MAJOR_VERSION, &glMajorVer);		GL_CHECK();
	//glGetIntegerv(GL_MINOR_VERSION, &glMinorVer);		GL_CHECK();
	glDisable(GL_DEPTH_TEST);							GL_CHECK();
	glEnable(GL_BLEND);									GL_CHECK();
	glBlendEquation(GL_FUNC_ADD);						GL_CHECK();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	GL_CHECK();
	if(!wnd_on_init())
		return 1;

	XWindowAttributes gwa;
	XEvent xev;
	char temp[100]={};
	KeySym key={};
	for(;;)
	{
		XNextEvent(dpy, &xev);
		//printf("%s\n", xevent2str(xev.type));//
		switch(xev.type)
		{
		case Expose://show/resize
			XGetWindowAttributes(dpy, win, &gwa);
			w=gwa.width, h=gwa.height;
			resize_gl();
			//glViewport(0, 0, gwa.width, gwa.height);
			wnd_on_render();
		//	render();
		//	//DrawAQuad();
		//	glXSwapBuffers(dpy, win);
			break;
		case KeyPress:
			{
				int i=XLookupString(&xev.xkey, temp, 10, &key, nullptr);
				printf("pressed %04lX: i=%d, \'%s\'\n", key, i, temp);
				//wnd_on_input(WM_KEYDOWN, key, 0);
				wnd_on_render();
			}
			//printf("%04d: %c\n", xev.xbutton.button, (char)(xev.xbutton.button&0xFF));
		//	glXMakeCurrent(dpy, None, nullptr);//
		//	glXDestroyContext(dpy, glc);
		//	XDestroyWindow(dpy, win);
		//	XCloseDisplay(dpy);
		//	exit(0);//
			break;
		case KeyRelease:
			{
				int i=XLookupString(&xev.xkey, temp, 10, &key, nullptr);
				printf("released %04lX: i=%d, \'%s\'\n", key, i, temp);
				//wnd_on_input(WM_KEYDOWN, key, 0);
				wnd_on_render();
			}
			break;
		default:
			wnd_on_render();
		//	render();
			break;
		}
	}
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
#endif

#if 0//https://gist.github.com/whosaysni/5733660
	auto dpy=XOpenDisplay(nullptr);
	auto screen=DefaultScreen(dpy);
	auto foreground = BlackPixel(dpy, screen);
	auto background = WhitePixel(dpy, screen);
	XSizeHints hint=
	{
		PPosition|PSize,//flags
		200, 300,//x, y
		350, 250,//width, height
	};
	auto window = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), hint.x, hint.y, hint.width, hint.height, 5, foreground, background);

	//window manager properties (yes, use of StdProp is obsolete)
	const char hello[]="Hello World", hi[]="Hi";
	XSetStandardProperties(dpy, window, hello, hello, None, argv, argc, &hint);

	//graphics context
	auto gc = XCreateGC(dpy, window, 0, 0);
	XSetBackground(dpy, gc, background);
	XSetForeground(dpy, gc, foreground);

	//allow receiving mouse events
	XSelectInput(dpy, window, ButtonPressMask|KeyPressMask|ExposureMask);

	//show up window
	XMapRaised(dpy, window);

	//event loop
	char text[100]={};
	XEvent e={};
	KeySym key={};
	for(int done=0;done==0;)
	{
		//fetch event
		XNextEvent(dpy, &e);
		switch(e.type)
		{
		case Expose:
			//Window was showed.
			if(e.xexpose.count==0)
				XDrawImageString(e.xexpose.display, e.xexpose.window, gc, 50, 50, hello, strlen(hello));
			break;
		case MappingNotify:
			//Modifier key was up/down.
			XRefreshKeyboardMapping((XMappingEvent*)&e);
			break;
		case ButtonPress:
			//Mouse button was pressed.
			XDrawImageString(e.xbutton.display, e.xbutton.window, gc, e.xbutton.x, e.xbutton.y, hi, strlen(hi));
			break;
		case KeyPress:
			{
				//Key input.
				int i=XLookupString((XKeyEvent*)&e, text, 10, &key, nullptr);
				if(i==1&&text[0]=='q')
					done=1;
			}
			break;
		}
	}//end for
	XFreeGC(dpy, gc);
	XDestroyWindow(dpy, window);
	XCloseDisplay(dpy);
#endif

	return 0;
}
#endif
