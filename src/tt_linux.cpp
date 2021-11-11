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

//void				get_window_title_w(wchar_t *str, int size){}
//void				set_window_title_w(const wchar_t *str){}
//void				set_window_title_a(const char *str){}
//int				GUINPrint(int x, int y, int w, int h, const char *a, ...){return 0;}
//long				GUITPrint(int x, int y, const char *a, ...){return 0;}
//void				GUIPrint(int x, int y, const char *a, ...){}
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
#if 0
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
#endif
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
	if(filename.size())
		snprintf(g_buf, g_buf_size, "Save changes to %s?", filename.c_str());
	else
		snprintf(g_buf, g_buf_size, "Save changes to Untitled?");
	const SDL_MessageBoxButtonData buttons[]=
	{
		{0, 0, "no" },
		{SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "yes"},
		{SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 2, "cancel"},
	};
	const SDL_MessageBoxColorScheme colorScheme=
	{
		{	//.r, .g, .b
			{255,   0,   0},//[SDL_MESSAGEBOX_COLOR_BACKGROUND]
			{  0, 255,   0},//[SDL_MESSAGEBOX_COLOR_TEXT]
			{255, 255,   0},//[SDL_MESSAGEBOX_COLOR_BUTTON_BORDER]
			{  0,   0, 255},//[SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND]
			{255,   0, 255},//[SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED]
		}
	};
	SDL_MessageBoxData data=
	{
		SDL_MESSAGEBOX_INFORMATION,//.flags
		nullptr,//.window
		"example message box",//.title
		"select a button",//.message
		SDL_arraysize(buttons),//.numbuttons
		buttons,//.buttons
		&colorScheme,//.colorScheme
	};
	int choice=0;
	int success=SDL_ShowMessageBox(&data, &choice);	SDL_ASSERT(success>=0);
	switch(choice)
	{
	case -1:choice=2;break;
	case 0:choice=1;break;
	case 1:choice=0;break;
	case 2:break;
	}
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

/*float				level=0, increment=0.1f;
void				render()
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

//GTK
#if 0
//callbacks for GTK:
static void			on_realize(GtkGLArea *glarea)
{
	// Make current:
	gtk_gl_area_make_current(glarea);

	// Print version info:
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	// Enable depth buffer:
	gtk_gl_area_set_has_depth_buffer(glarea, TRUE);

	// Init program:
	if(!wnd_on_init())
		abort();

	// Get frame clock:
	//GdkGLContext *glcontext = gtk_gl_area_get_context(glarea);
	//GdkWindow *glwindow = gdk_gl_context_get_window(glcontext);
	//GdkFrameClock *frame_clock = gdk_window_get_frame_clock(glwindow);

	// Connect update signal:
	//g_signal_connect_swapped(frame_clock, "update", G_CALLBACK(gtk_gl_area_queue_render), glarea) ;

	// Start updating:
	//gdk_frame_clock_begin_updating(frame_clock);
}
static int				on_render(GtkGLArea *glarea, GdkGLContext *context)
{
	wnd_on_render();
	return 1;
}
static void				on_resize(GtkGLArea *area, gint width, gint height)
{
	w=width, h=height;
	wnd_on_resize();
	//view_set_window(width, height);
	//background_set_window(width, height);
}
static int				on_scroll(GtkWidget* widget, GdkEventScroll *event)
{
	switch(event->direction)
	{
	case GDK_SCROLL_UP:
		wnd_on_mousewheel(true);
		break;
	case GDK_SCROLL_DOWN:
		wnd_on_mousewheel(false);
		break;
	}
	return FALSE;
}

static int				on_button_press(GtkWidget *widget, GdkEventButton *event)
{
	GtkAllocation allocation;
	gtk_widget_get_allocation(widget, &allocation);

	//TODO

/*	if(event->button==1&&!panning)
	{
		panning=TRUE;
		model_pan_start(event->x, allocation.height - event->y);
	}//*/
	return FALSE;
}
static int				on_button_release(GtkWidget *widget, GdkEventButton *event)
{
//	if (event->button == 1)
//		panning = FALSE;
	return FALSE;
}
static int				on_motion_notify(GtkWidget *widget, GdkEventMotion *event)
{
	GtkAllocation allocation;
	gtk_widget_get_allocation(widget, &allocation);

	//TODO

	//if (panning == TRUE)
	//	model_pan_move(event->x, allocation.height - event->y);

	return FALSE;
}
//end of callbacks

struct Signal
{
	const char *signal;
	GCallback handler;
	GdkEventMask mask;
};
void				connect_signals (GtkWidget *widget, Signal *signals, size_t count)
{
	for(int k=0;k<count;++k)
	{
		auto s=signals+k;
		gtk_widget_add_events(widget, s->mask);
		g_signal_connect(G_OBJECT(widget), s->signal, s->handler, NULL);
	}
}
#endif

#if 0
void PrintModifiers( SDLMod mod )//Print modifier info
{
	printf( "Modifers: " );

	//If there are none then say so and return
	if( mod == KMOD_NONE ){
		printf( "None\n" );
		return;
	}

	//Check for the presence of each SDLMod value
	//This looks messy, but there really isn't a clearer way.
	if( mod & KMOD_NUM ) printf( "NUMLOCK " );
	if( mod & KMOD_CAPS ) printf( "CAPSLOCK " );
	if( mod & KMOD_LCTRL ) printf( "LCTRL " );
	if( mod & KMOD_RCTRL ) printf( "RCTRL " );
	if( mod & KMOD_RSHIFT ) printf( "RSHIFT " );
	if( mod & KMOD_LSHIFT ) printf( "LSHIFT " );
	if( mod & KMOD_RALT ) printf( "RALT " );
	if( mod & KMOD_LALT ) printf( "LALT " );
	if( mod & KMOD_CTRL ) printf( "CTRL " );
	if( mod & KMOD_SHIFT ) printf( "SHIFT " );
	if( mod & KMOD_ALT ) printf( "ALT " );
	printf( "\n" );
}
void PrintKeyInfo( SDL_KeyboardEvent *key )//Print all information about a key event
{
	if(key->type==SDL_KEYUP)//Is it a release or a press?
		printf( "Release:- " );
	else
		printf( "Press:- " );

	printf("Scancode: 0x%02X", key->keysym.scancode);//Print the hardware scancode first
	printf(", Name: %s", SDL_GetKeyName( key->keysym.sym));//Print the name of the key

	//printf("\n");

	if(key->type==SDL_KEYDOWN)//We want to print the unicode info, but we need to make sure its a press event first (remember, release events don't have unicode info)
	{
		//If the Unicode value is less than 0x80 then the
		//unicode value can be used to get a printable
		//representation of the key, using (char)unicode.
		printf(", Unicode: " );
		if( key->keysym.unicode < 0x80 && key->keysym.unicode > 0 )
			printf( "%c (0x%04X)", (char)key->keysym.unicode, key->keysym.unicode );
		else
			printf( "? (0x%04X)", key->keysym.unicode );
	}
	printf("\n");
	PrintModifiers( key->keysym.mod );//Print modifier info//*/
}
#endif
const char			numrow[]=")!@#$%^&*(";
int					main(int argc, char **argv)
{
	{
		exe_dir+=argv[0];
		int k=exe_dir.size();
		for(;k>0&&exe_dir[k-1]!='/'&&exe_dir[k-1]!='\\';--k);
		exe_dir.resize(k);
	}
	//SDL2
#if 1
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

	for(int loop=1;loop;)
	{
		SDL_Event e={};
		int success=SDL_WaitEvent(&e);	SDL_ASSERT(success);
	//	printf("Event: %d\n", e.type);//
		bool redraw=false;
		switch(e.type)
		{
		case SDL_DISPLAYEVENT:
			break;
		case SDL_WINDOWEVENT:
			if(e.window.event==SDL_WINDOWEVENT_SIZE_CHANGED)
			{
				SDL_GetWindowSize(window, &w, &h);
				resize_gl();
				wnd_on_render();
			}
			break;
		case SDL_MOUSEMOTION:
			mx=e.motion.x, my=e.motion.y;
			wnd_on_mousemove();
			break;
		case SDL_MOUSEWHEEL:
			wnd_on_mousewheel(e.wheel.y>0);
			break;
		case SDL_MOUSEBUTTONDOWN:
			if(e.button.button==SDL_BUTTON_LEFT)
				wnd_on_lbuttondown();
			else if(e.button.button==SDL_BUTTON_RIGHT)
				wnd_on_rbuttondown();
			break;
		case SDL_MOUSEBUTTONUP:
			if(e.button.button==SDL_BUTTON_LEFT)
				wnd_on_lbuttonup();
			else if(e.button.button==SDL_BUTTON_RIGHT)
				wnd_on_rbuttonup();
			break;
		case SDL_KEYDOWN:
			{
				int key=e.key.keysym.scancode;
				if(is_ctrl_down())
				{
					switch(key)
					{
					case SDL_SCANCODE_UP:	redraw=wnd_on_scroll_up_key();		break;
					case SDL_SCANCODE_DOWN:	redraw=wnd_on_scroll_down_key();	break;
					case SDL_SCANCODE_LEFT:	redraw=wnd_on_skip_word_left();		break;
					case SDL_SCANCODE_RIGHT:redraw=wnd_on_skip_word_right();	break;
					case SDL_SCANCODE_HOME:	redraw=wnd_on_goto_file_start();	break;
					case SDL_SCANCODE_END:	redraw=wnd_on_goto_file_end();		break;
					case SDL_SCANCODE_A:	redraw=wnd_on_select_all();			break;
					case SDL_SCANCODE_C:	redraw=wnd_on_copy();				break;
					case SDL_SCANCODE_D:	redraw=wnd_on_clear_hist();			break;
					case SDL_SCANCODE_N:	redraw=wnd_on_new();				break;
					case SDL_SCANCODE_O:	redraw=wnd_on_open();				break;
					case SDL_SCANCODE_S:	redraw=wnd_on_save(is_shift_down());break;
					case SDL_SCANCODE_V:	redraw=wnd_on_paste();				break;
					case SDL_SCANCODE_Y:	redraw=wnd_on_redo();				break;
					case SDL_SCANCODE_Z:	redraw=wnd_on_undo();				break;
					}
				}
				else
				{
					char shift=is_shift_down(), lowercase=caps_lock==shift;
					switch(key)
					{
					case SDL_SCANCODE_CAPSLOCK:	caps_lock=!caps_lock;			break;
					case SDL_SCANCODE_LALT:
					case SDL_SCANCODE_RALT:		redraw=wnd_on_begin_rectsel();	break;
					case SDL_SCANCODE_ESCAPE:	redraw=wnd_on_deselect();		break;
					case SDL_SCANCODE_F1:		redraw=wnd_on_display_help();	break;
					case SDL_SCANCODE_F4:
						if(is_alt_down())
						{
							if(wnd_on_quit())
								goto quit;
						}
						else
							redraw=wnd_on_toggle_profiler();
						break;
					case SDL_SCANCODE_UP:		redraw=wnd_on_cursor_up();		break;
					case SDL_SCANCODE_DOWN:		redraw=wnd_on_cursor_down();	break;
					case SDL_SCANCODE_LEFT:		redraw=wnd_on_cursor_left();	break;
					case SDL_SCANCODE_RIGHT:	redraw=wnd_on_cursor_right();	break;
					case SDL_SCANCODE_HOME:		redraw=wnd_on_goto_line_start();break;
					case SDL_SCANCODE_END:		redraw=wnd_on_goto_line_end();	break;
					case SDL_SCANCODE_DELETE:	redraw=wnd_on_deletechar();		break;
					case SDL_SCANCODE_BACKSPACE:redraw=wnd_on_backspace();		break;

					case SDL_SCANCODE_SEMICOLON:	redraw=wnd_on_type(shift?	':':';'		);break;
					case SDL_SCANCODE_SLASH:		redraw=wnd_on_type(shift?	'?':'/'		);break;
					case SDL_SCANCODE_GRAVE:		redraw=wnd_on_type(shift?	'~':'`'		);break;
					case SDL_SCANCODE_LEFTBRACKET:	redraw=wnd_on_type(shift?	'{':'['		);break;
					case SDL_SCANCODE_BACKSLASH:	redraw=wnd_on_type(shift?	'|':'\\'	);break;
					case SDL_SCANCODE_RIGHTBRACKET:	redraw=wnd_on_type(shift?	'}':']'		);break;
					case SDL_SCANCODE_APOSTROPHE:	redraw=wnd_on_type(shift?	'\"':'\''	);break;
					case SDL_SCANCODE_MINUS:		redraw=wnd_on_type(shift?	'_':'-'		);break;
					case SDL_SCANCODE_EQUALS:		redraw=wnd_on_type(shift?	'+':'='		);break;
					case SDL_SCANCODE_COMMA:		redraw=wnd_on_type(shift?	'<':','		);break;
					case SDL_SCANCODE_PERIOD:		redraw=wnd_on_type(shift?	'>':'.'		);break;
					case SDL_SCANCODE_KP_PERIOD:	redraw=wnd_on_type(			'.'			);break;
					case SDL_SCANCODE_KP_PLUS:		redraw=wnd_on_type(			'+'			);break;
					case SDL_SCANCODE_KP_MINUS:		redraw=wnd_on_type(			'-'			);break;
					case SDL_SCANCODE_KP_MULTIPLY:	redraw=wnd_on_type(			'*'			);break;
					case SDL_SCANCODE_KP_DIVIDE:	redraw=wnd_on_type(			'/'			);break;
					case SDL_SCANCODE_SPACE:		redraw=wnd_on_type(			' '			);break;
					case SDL_SCANCODE_TAB:			redraw=wnd_on_type(			'\t'		);break;
					case SDL_SCANCODE_RETURN:		redraw=wnd_on_type(			'\n'		);break;
					default:
						if(key>=SDL_SCANCODE_A&&key<=SDL_SCANCODE_Z)
						{
							if(lowercase)
								redraw=wnd_on_type(key+'a'-SDL_SCANCODE_A);
							else
								redraw=wnd_on_type(key+'A'-SDL_SCANCODE_A);
						}
						else if(key>=SDL_SCANCODE_1&&key<=SDL_SCANCODE_0)
						{
							if(shift)
								redraw=wnd_on_type(key==SDL_SCANCODE_0?numrow[0]:numrow[key+1-SDL_SCANCODE_1]);
							else
								redraw=wnd_on_type(key==SDL_SCANCODE_0?'0':key+'1'-SDL_SCANCODE_1);
						}
						else if(key>=SDLK_KP_1&&key<=SDLK_KP_0)
							redraw=wnd_on_type(key==SDLK_KP_0?'0':key+'1'-SDLK_KP_1);
						break;
					}
				}
#if 0
				int key=e.key.keysym.sym;
				if(is_ctrl_down())
				{
					switch(key)
					{
					case SDLK_UP:	redraw=wnd_on_scroll_up_key();		break;
					case SDLK_DOWN:	redraw=wnd_on_scroll_down_key();	break;
					case SDLK_LEFT:	redraw=wnd_on_skip_word_left();		break;
					case SDLK_RIGHT:redraw=wnd_on_skip_word_right();	break;
					case SDLK_HOME:	redraw=wnd_on_goto_file_start();	break;
					case SDLK_END:	redraw=wnd_on_goto_file_end();		break;
					case SDLK_a:	redraw=wnd_on_select_all();			break;
					case SDLK_c:	redraw=wnd_on_copy();				break;
					case SDLK_d:	redraw=wnd_on_clear_hist();			break;
					case SDLK_n:	redraw=wnd_on_new();				break;
					case SDLK_o:	redraw=wnd_on_open();				break;
					case SDLK_s:	redraw=wnd_on_save(is_shift_down());break;
					case SDLK_v:	redraw=wnd_on_paste();				break;
					case SDLK_y:	redraw=wnd_on_redo();				break;
					case SDLK_z:	redraw=wnd_on_undo();				break;
					}
				}
				else
				{
					char shift=is_shift_down(), lowercase=caps_lock==shift;
					switch(key)
					{
					case SDLK_CAPSLOCK:	caps_lock=!caps_lock;						break;
					case SDLK_LALT:case SDLK_RALT:	redraw=wnd_on_begin_rectsel();	break;
					case SDLK_ESCAPE:				redraw=wnd_on_deselect();		break;
					case SDLK_F1:					redraw=wnd_on_display_help();	break;
					case SDLK_F4:
						if(is_alt_down())
						{
							if(wnd_on_quit())
								goto quit;
						}
						else
							redraw=wnd_on_toggle_profiler();
						break;
					case SDLK_UP:		redraw=wnd_on_cursor_up();		break;
					case SDLK_DOWN:		redraw=wnd_on_cursor_down();	break;
					case SDLK_LEFT:		redraw=wnd_on_cursor_left();	break;
					case SDLK_RIGHT:	redraw=wnd_on_cursor_right();	break;
					case SDLK_HOME:		redraw=wnd_on_goto_line_start();break;
					case SDLK_END:		redraw=wnd_on_goto_line_end();	break;
					case SDLK_DELETE:	redraw=wnd_on_deletechar();		break;
					case SDLK_BACKSPACE:redraw=wnd_on_backspace();		break;

					case SDLK_SEMICOLON:	redraw=wnd_on_type(shift?	':':';'		);break;
					case SDLK_SLASH:		redraw=wnd_on_type(shift?	'?':'/'		);break;
					case SDLK_BACKQUOTE:	redraw=wnd_on_type(shift?	'~':'`'		);break;
					case SDLK_LEFTBRACKET:	redraw=wnd_on_type(shift?	'{':'['		);break;
					case SDLK_BACKSLASH:	redraw=wnd_on_type(shift?	'|':'\\'	);break;
					case SDLK_RIGHTBRACKET:	redraw=wnd_on_type(shift?	'}':']'		);break;
					case SDLK_QUOTE:		redraw=wnd_on_type(shift?	'\"':'\''	);break;
					case SDLK_MINUS:		redraw=wnd_on_type(shift?	'_':'-'		);break;
					case SDLK_EQUALS:		redraw=wnd_on_type(shift?	'+':'='		);break;
					case SDLK_COMMA:		redraw=wnd_on_type(shift?	'<':','		);break;
					case SDLK_PERIOD:		redraw=wnd_on_type(shift?	'>':'.'		);break;
					case SDLK_KP_PERIOD:	redraw=wnd_on_type(			'.'			);break;
					case SDLK_KP_PLUS:		redraw=wnd_on_type(			'+'			);break;
					case SDLK_KP_MINUS:		redraw=wnd_on_type(			'-'			);break;
					case SDLK_KP_MULTIPLY:	redraw=wnd_on_type(			'*'			);break;
					case SDLK_KP_DIVIDE:	redraw=wnd_on_type(			'/'			);break;
					case SDLK_SPACE:		redraw=wnd_on_type(			' '			);break;
					case SDLK_TAB:			redraw=wnd_on_type(			'\t'		);break;
					case SDLK_RETURN:		redraw=wnd_on_type(			'\n'		);break;
					default:
						if(key>='a'&&key<='z')
							redraw=wnd_on_type(key+('A'-'a')*!lowercase);
						else if(key>='0'&&key<='9')
						{
							if(shift)
								redraw=wnd_on_type(numrow[key-'0']);
							else
								redraw=wnd_on_type(key);
						}
						else if(key>=SDLK_KP_0&&key<=SDLK_KP_9)
							redraw=wnd_on_type(char(key+'0'-SDLK_KP_0));
						break;
					}
				}
#endif
				keyboard[e.key.keysym.scancode]=true;
			}
			//PrintKeyInfo(&e.key);
			//printf("Key press detected\n");
			break;
		case SDL_KEYUP:
			keyboard[e.key.keysym.scancode]=false;
			//PrintKeyInfo(&e.key);
			//printf("Key release detected\n");
			break;
		case SDL_QUIT:
			loop=0;
			break;
		}
		wnd_on_render();
		SDL_GL_SwapWindow(window);
		SDL_Delay(1);
	}
quit:
	SDL_Quit();
#endif
	//GTK+
#if 0//https://github.com/aklomp/gtk3-opengl
	if(!gtk_init_check(&argc, &argv))
		return exit_failure("Could not initialize GTK");

	// Create toplevel window, add GtkGLArea:
	auto window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	auto glarea=gtk_gl_area_new();
	gtk_container_add(GTK_CONTAINER(window), glarea);

	//gdk_gl_context_is_legacy();

	// Connect GTK signals:
	Signal s1[]=
	{
		{"destroy", G_CALLBACK(gtk_main_quit), (GdkEventMask)0},
	};
	Signal s2[]=
	{
		{"realize",				G_CALLBACK(on_realize),			(GdkEventMask)0			},
		{"render",				G_CALLBACK(on_render),			(GdkEventMask)0			},
		{"resize",				G_CALLBACK(on_resize),			(GdkEventMask)0			},
		{"scroll-event",		G_CALLBACK(on_scroll),			GDK_SCROLL_MASK			},
		{"button-press-event",	G_CALLBACK(on_button_press),	GDK_BUTTON_PRESS_MASK	},
		{"button-release-event",G_CALLBACK(on_button_release),	GDK_BUTTON_RELEASE_MASK	},
		{"motion-notify-event",	G_CALLBACK(on_motion_notify),	GDK_BUTTON1_MOTION_MASK	},
	};
	connect_signals(window, s1, SIZEOF(s1));
	connect_signals(glarea, s2, SIZEOF(s2));
	gtk_widget_show_all(window);
	gtk_main();
#endif

	//Xlib
#if 0//https://www.khronos.org/opengl/wiki/Programming_OpenGL_in_Linux:_GLX_and_Xlib
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
	//Xlib tutorial
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
