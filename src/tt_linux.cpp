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

char				keyboard[256]={};
SDL_Window			*window=nullptr;
int					exit_failure(const char * msg, ...)
{
	if(msg)
	{
		va_list args;
		va_start(args, msg);
		vprintf(msg, args);
		va_end(args);
	}
	return 1;
}
bool				sdl_check(const char *file, int line)
{
	log_error(file, line, "%s", SDL_GetError());
	//printf("%s(%s): %s\n", file, line, SDL_GetError());
	return false;
}
#define				SDL_ASSERT(SUCCESS)		((void)((SUCCESS)!=0||sdl_check(file, __LINE__)))

bool				get_key_state(int key)
{
	switch(key)
	{
#define CASE(LABEL)	case SDLK_##LABEL:return keyboard[SDL_SCANCODE_##LABEL];
	CASE(LCTRL)
	CASE(RCTRL)
	CASE(LSHIFT)
	CASE(RSHIFT)
	CASE(LALT)
	CASE(RALT)
#undef	CASE
	}
	//XkbGetState();
	return false;//
}
void				messagebox(const char *title, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	int len=vsnprintf(g_buf, g_buf_size, format, args);
	va_end(args);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, title, g_buf, window);
}
void				copy_to_clipboard_c(const char *a, int size)//size not including null terminator
{
	int fail=SDL_SetClipboardText(a);	SDL_ASSERT(fail>=0);
}
bool				paste_from_clipboard(char *&str, int &len)
{
	char *str0=SDL_GetClipboardText();
	if(!str0)
		return false;
	int len0=strlen(str0);
	str=new char[len0+1];
	len=0;
	for(int k2=0;k2<len0;++k2)
	{
		if(str0[k2]!='\r')
			str[len]=str0[k2], ++len;
	}
	str[len]='\0';
	return true;
}
void				get_window_title(char *str, int size)
{
	auto str0=SDL_GetWindowTitle(window);
	if(str0)
	{
		int len=strlen(str0);
		memcpy(str, str0, minimum(len+1, size));//with null terminator
	}
}
void				set_window_title(const char *str)
{
	SDL_SetWindowTitle(window, str);
}
int					GUINPrint(int x, int y, int w, int h, const char *a, ...)//should support newlines
{
#ifdef PRINT_TO_CONSOLE
	va_list args;
	va_start(args, a);
	int printed=vprintf(a, args);
	va_end(args);
	//printf("\n");
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
	//printf("\n");
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
	//printf("\n");
#endif

	//int length=vsprintf_s(g_buf, g_buf_size, a, (char*)(&a+1)), success;
	//if(length>0)
	//{
	//	success=TextOutA(ghDC, x, y, g_buf, length);	SYS_ASSERT(success);
	//}
}
//bool				dialog_openfile(const char *default_path, const char *filter_list, std::string &result);
struct				osdialog_filters
{
	const char *name, **patterns;
	int npatterns;
};
bool				osdialog_file(bool open, const char *default_path, const char *filename, osdialog_filters *filters, int nfilters, std::string &result);
const char*			open_file_dialog()
{
	const char *all_files[]={"*.*"};
	osdialog_filters filters[]=
	{
		{"All Files (*.*)", all_files, 1},
	};
	std::string result;
	if(!osdialog_file(true, nullptr, nullptr, filters, 1, result))
		return nullptr;
	memcpy(g_buf, result.c_str(), result.size()+1);
	return g_buf;
}
const char*			save_file_dialog()
{
	const char *ext[]=
	{
		"*.txt",
		"*.c", "*.cpp",
		"*.h", "*.hpp",
	};
	osdialog_filters filters[]=
	{
		{"Text File (.txt)", ext+0, 1},
		{"C/C++ Source (.c; .cpp)", ext+1, 2},
		{"C/C++ Header (.h; .hpp)", ext+3, 2},
	};
	std::string result;
	if(!osdialog_file(false, nullptr, nullptr, filters, 1, result))
		return nullptr;
	memcpy(g_buf, result.c_str(), result.size()+1);
	return g_buf;
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
	int read=fread(&str[0], 1, bytesize, file);
	fclose(file);
	str.resize(read);
	//str.resize(strlen(str.c_str()));//remove extra null terminators at the end
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
	if(filename.size())
		snprintf(g_buf, g_buf_size, "Save changes to %s?", filename.c_str());
	else
		snprintf(g_buf, g_buf_size, "Save changes to Untitled?");
	const SDL_MessageBoxButtonData buttons[]=
	{
		{SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "Yes"},
		{0, 1, "No" },
		{SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 2, "Cancel"},
	};
	//const SDL_MessageBoxColorScheme colorScheme=
	//{
	//	{	//.r, .g, .b
	//		{255,   0,   0},//[SDL_MESSAGEBOX_COLOR_BACKGROUND]
	//		{  0, 255,   0},//[SDL_MESSAGEBOX_COLOR_TEXT]
	//		{255, 255,   0},//[SDL_MESSAGEBOX_COLOR_BUTTON_BORDER]
	//		{  0,   0, 255},//[SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND]
	//		{255,   0, 255},//[SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED]
	//	}
	//};
	SDL_MessageBoxData data=
	{
		SDL_MESSAGEBOX_INFORMATION,//.flags
		nullptr,//.window
		"",//.title
		g_buf,//.message
		SDL_arraysize(buttons),//.numbuttons
		buttons,//.buttons
		nullptr,
	//	&colorScheme,//.colorScheme
	};
	int choice=0;
	int success=SDL_ShowMessageBox(&data, &choice);	SDL_ASSERT(success>=0);
	if(choice==-1)
		choice=2;
	return choice;
}
void				mouse_capture()
{
	int fail=SDL_CaptureMouse(SDL_TRUE);
	SDL_ASSERT(!fail);
}
void				mouse_release()
{
	int fail=SDL_CaptureMouse(SDL_FALSE);
	SDL_ASSERT(!fail);
}

void				swap_buffers()
{
	//glXSwapBuffers(dpy, win);
}
const char			numrow[]=")!@#$%^&*(";
SDL_Event			events[2048];
int					main(int argc, char **argv)
{
	{
		exe_dir+=argv[0];
		int k=exe_dir.size();
		for(;k>0&&exe_dir[k-1]!='/'&&exe_dir[k-1]!='\\';--k);
		exe_dir.resize(k);
	}
	//SDL2
	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS))
		return exit_failure("Failed to initialize SDL");
	window=SDL_CreateWindow("Teletext", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE|SDL_WINDOW_MAXIMIZED);
	if(!window)
		return exit_failure("Failed to create SDL window\n");

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);//
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);//
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GLContext glContext=SDL_GL_CreateContext(window);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if(!wnd_on_init())
		goto quit;

	for(;;)
	{
		int success=SDL_WaitEvent(events);	SDL_ASSERT(success);
		SDL_PumpEvents();
		int nevents=1+SDL_PeepEvents(events+1, 2047, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);

		//SDL_PumpEvents();
		//int nevents=SDL_PeepEvents(events, 2048, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);

	//	int success=SDL_WaitEvent(&e);	SDL_ASSERT(success);
	//	int success=SDL_PollEvent(&e);	SDL_ASSERT(success);

		bool redraw=false;
		for(int ke=0;ke<nevents;++ke)
		{
			auto e=events+ke;
			//printf("Event: %d\n", e->type);//
			switch(e->type)
			{
			case SDL_DISPLAYEVENT:
				break;
			case SDL_WINDOWEVENT:
				if(e->window.event==SDL_WINDOWEVENT_SIZE_CHANGED)
				{
					redraw=true;
					SDL_GetWindowSize(window, &w, &h);
					resize_gl();
				}
				break;
			case SDL_MOUSEMOTION:
				for(;ke+1<nevents&&events[ke+1].type==e->type;++ke);
				e=events+ke;
				mx=e->motion.x, my=e->motion.y;
				redraw|=wnd_on_mousemove();
				break;
			case SDL_MOUSEWHEEL:
				redraw|=wnd_on_mousewheel(e->wheel.y>0);
				break;
			case SDL_MOUSEBUTTONDOWN:
				if(e->button.button==SDL_BUTTON_LEFT)
					redraw|=wnd_on_lbuttondown();
				else if(e->button.button==SDL_BUTTON_RIGHT)
					redraw|=wnd_on_rbuttondown();
				break;
			case SDL_MOUSEBUTTONUP:
				if(e->button.button==SDL_BUTTON_LEFT)
					redraw|=wnd_on_lbuttonup();
				else if(e->button.button==SDL_BUTTON_RIGHT)
					redraw|=wnd_on_rbuttonup();
				break;
			case SDL_KEYDOWN:
				{
					int key=e->key.keysym.scancode;
					if(is_ctrl_down())
					{
						switch(key)
						{
						case SDL_SCANCODE_UP:	redraw|=wnd_on_scroll_up_key();		break;
						case SDL_SCANCODE_DOWN:	redraw|=wnd_on_scroll_down_key();	break;
						case SDL_SCANCODE_LEFT:	redraw|=wnd_on_skip_word_left();		break;
						case SDL_SCANCODE_RIGHT:redraw|=wnd_on_skip_word_right();	break;
						case SDL_SCANCODE_HOME:	redraw|=wnd_on_goto_file_start();	break;
						case SDL_SCANCODE_END:	redraw|=wnd_on_goto_file_end();		break;
						case SDL_SCANCODE_A:	redraw|=wnd_on_select_all();			break;
						case SDL_SCANCODE_C:	redraw|=wnd_on_copy();				break;
						case SDL_SCANCODE_D:	redraw|=wnd_on_clear_hist();			break;
						case SDL_SCANCODE_N:	redraw|=wnd_on_new();				break;
						case SDL_SCANCODE_O:	redraw|=wnd_on_open();				break;
						case SDL_SCANCODE_S:	redraw|=wnd_on_save(is_shift_down());break;
						case SDL_SCANCODE_V:	redraw|=wnd_on_paste();				break;
						case SDL_SCANCODE_Y:	redraw|=wnd_on_redo();				break;
						case SDL_SCANCODE_Z:	redraw|=wnd_on_undo();				break;
						}
					}
					else
					{
						char shift=is_shift_down(), lowercase=caps_lock==shift;
						switch(key)
						{
						case SDL_SCANCODE_CAPSLOCK:	caps_lock=!caps_lock;			break;
						case SDL_SCANCODE_LALT:
						case SDL_SCANCODE_RALT:		redraw|=wnd_on_begin_rectsel();	break;
						case SDL_SCANCODE_ESCAPE:	redraw|=wnd_on_deselect();		break;
						case SDL_SCANCODE_F1:		redraw|=wnd_on_display_help();	break;
						case SDL_SCANCODE_F4:
							if(is_alt_down())
							{
								if(wnd_on_quit())
									goto quit;
							}
							else
								redraw|=wnd_on_toggle_profiler();
							break;
						case SDL_SCANCODE_UP:		redraw|=wnd_on_cursor_up();		break;
						case SDL_SCANCODE_DOWN:		redraw|=wnd_on_cursor_down();	break;
						case SDL_SCANCODE_LEFT:		redraw|=wnd_on_cursor_left();	break;
						case SDL_SCANCODE_RIGHT:	redraw|=wnd_on_cursor_right();	break;
						case SDL_SCANCODE_HOME:		redraw|=wnd_on_goto_line_start();break;
						case SDL_SCANCODE_END:		redraw|=wnd_on_goto_line_end();	break;
						case SDL_SCANCODE_DELETE:	redraw|=wnd_on_deletechar();		break;
						case SDL_SCANCODE_BACKSPACE:redraw|=wnd_on_backspace();		break;

						case SDL_SCANCODE_SEMICOLON:	redraw|=wnd_on_type(shift?	':':';'		);break;
						case SDL_SCANCODE_SLASH:		redraw|=wnd_on_type(shift?	'?':'/'		);break;
						case SDL_SCANCODE_GRAVE:		redraw|=wnd_on_type(shift?	'~':'`'		);break;
						case SDL_SCANCODE_LEFTBRACKET:	redraw|=wnd_on_type(shift?	'{':'['		);break;
						case SDL_SCANCODE_BACKSLASH:	redraw|=wnd_on_type(shift?	'|':'\\'	);break;
						case SDL_SCANCODE_RIGHTBRACKET:	redraw|=wnd_on_type(shift?	'}':']'		);break;
						case SDL_SCANCODE_APOSTROPHE:	redraw|=wnd_on_type(shift?	'\"':'\''	);break;
						case SDL_SCANCODE_MINUS:		redraw|=wnd_on_type(shift?	'_':'-'		);break;
						case SDL_SCANCODE_EQUALS:		redraw|=wnd_on_type(shift?	'+':'='		);break;
						case SDL_SCANCODE_COMMA:		redraw|=wnd_on_type(shift?	'<':','		);break;
						case SDL_SCANCODE_PERIOD:		redraw|=wnd_on_type(shift?	'>':'.'		);break;
						case SDL_SCANCODE_KP_PERIOD:	redraw|=wnd_on_type(			'.'			);break;
						case SDL_SCANCODE_KP_PLUS:		redraw|=wnd_on_type(			'+'			);break;
						case SDL_SCANCODE_KP_MINUS:		redraw|=wnd_on_type(			'-'			);break;
						case SDL_SCANCODE_KP_MULTIPLY:	redraw|=wnd_on_type(			'*'			);break;
						case SDL_SCANCODE_KP_DIVIDE:	redraw|=wnd_on_type(			'/'			);break;
						case SDL_SCANCODE_SPACE:		redraw|=wnd_on_type(			' '			);break;
						case SDL_SCANCODE_TAB:			redraw|=wnd_on_type(			'\t'		);break;
						case SDL_SCANCODE_RETURN:		redraw|=wnd_on_type(			'\n'		);break;
						default:
							if(key>=SDL_SCANCODE_A&&key<=SDL_SCANCODE_Z)
							{
								if(lowercase)
									redraw|=wnd_on_type(key+'a'-SDL_SCANCODE_A);
								else
									redraw|=wnd_on_type(key+'A'-SDL_SCANCODE_A);
							}
							else if(key>=SDL_SCANCODE_1&&key<=SDL_SCANCODE_0)
							{
								if(shift)
									redraw|=wnd_on_type(key==SDL_SCANCODE_0?numrow[0]:numrow[key+1-SDL_SCANCODE_1]);
								else
									redraw|=wnd_on_type(key==SDL_SCANCODE_0?'0':key+'1'-SDL_SCANCODE_1);
							}
							else if(key>=SDLK_KP_1&&key<=SDLK_KP_0)
								redraw|=wnd_on_type(key==SDLK_KP_0?'0':key+'1'-SDLK_KP_1);
							break;
						}
					}
					keyboard[e->key.keysym.scancode]=true;
				}
				//PrintKeyInfo(&e->key);
				//printf("Key press detected\n");
				break;
			case SDL_KEYUP:
				keyboard[e->key.keysym.scancode]=false;
				//PrintKeyInfo(&e->key);
				//printf("Key release detected\n");
				break;
			case SDL_QUIT:
				if(wnd_on_quit())
					goto quit;
				break;
			}
		}
		if(redraw)
		{
			wnd_on_render();
			SDL_GL_SwapWindow(window);
			//SDL_Delay(1);
		}
	}
quit:
	SDL_Quit();
	return 0;
}
#endif
