#pragma once
#ifndef TT_H
#define TT_H
#ifdef _WINDOWS
#include			<Windows.h>
#elif defined __linux__
#include			<X11/Xlib.h>
#include			<X11/Xutil.h>
#define				GL_GLEXT_PROTOTYPES
#endif
#include			<GL/gl.h>
#include			<GL/glu.h>
#include			<math.h>
#include			<string>

#define				SIZEOF(STATIC_ARRAY)	(sizeof(STATIC_ARRAY)/sizeof(*(STATIC_ARRAY)))

#define				G_BUF_SIZE	4096
const int			g_buf_size=G_BUF_SIZE;
extern char			g_buf[G_BUF_SIZE];
extern wchar_t		g_wbuf[G_BUF_SIZE];

extern int			w, h;
extern short		mx, my, dx, dy;
extern int			*rgb, rgbn;
extern char			keyboard[256];
extern int			font_idx, font_size;

//math
inline int			mod(int x, int n)
{
	x%=n;
	x+=n&-(x<0);
	return x;
}
inline int			floor_log2(unsigned n)
{
	int logn=0;
	int sh=(((short*)&n)[1]!=0)<<4;	logn^=sh, n>>=sh;	//21.54
		sh=(((char*)&n)[1]!=0)<<3;	logn^=sh, n>>=sh;
		sh=((n&0x000000F0)!=0)<<2;	logn^=sh, n>>=sh;
		sh=((n&0x0000000C)!=0)<<1;	logn^=sh, n>>=sh;
		sh=((n&0x00000002)!=0);		logn^=sh;
	return logn;
}
inline int			minimum(int a, int b){return (a+b-abs(a-b))>>1;}
inline int			maximum(int a, int b){return (a+b+abs(a-b))>>1;}
//inline float		maximum(float x, float y){return (x+y+abs(x-y))*0.5f;}
inline char			maximum(char x1, char x2, char x3, char x4, char x5, char x6)
{
	int m1=(x1+x2+abs(x1-x2)), m2=(x3+x4+abs(x3-x4)), m3=(x5+x6+abs(x5-x6));
	int m4=(m1+m2+abs(m1-m2)), m5=m3<<1;
	return (m4+m5+abs(m4-m5))>>3;
}
inline int			clamp(int lo, int x, int hi)
{
	hi<<=1;
	int temp=x+lo+abs(x-lo);
	return (temp+hi-abs(temp-hi))>>2;
}
inline float		clamp(float lo, float x, float hi)
{
	hi+=hi;
	float temp=x+lo+abs(x-lo);
	return (temp+hi-abs(temp-hi))*0.25f;
}

//error handling
bool 				log_error(const char *file, int line, const char *format, ...);
#define				LOG_ERROR(format, ...)	log_error(file, __LINE__, format, __VA_ARGS__)

//performance
#if 1
//[PROFILER SETTINGS]

	#define PROF_INITIAL_STATE	true
//	#define PROF_INITIAL_STATE	false

//comment or uncomment
//	#define PROFILER_CYCLES

//select one or nothing (profiler on screen)
//	#define PROFILER_TITLE
//	#define PROFILER_CMD

//comment or uncomment
//	#define PROFILER_CLIPBOARD

//select one
#ifdef _MSC_VER
	#define TIMING_USE_QueryPerformanceCounter
//	#define	TIMING_USE_rdtsc
//	#define TIMING_USE_GetProcessTimes	//~16ms resolution
//	#define TIMING_USE_GetTickCount		//~16ms resolution
//	#define TIMING_USE_timeGetTime		//~16ms resolution
#elif defined __linux__
	#define	TIMING_USE_clock_gettime
//	#define	TIMING_USE_rdtsc
#endif

//END OF [PROFILER SETTINGS]
double				time_sec();
double				time_ms();
double				elapsed_ms();//since last call
double				elapsed_cycles();//since last call

extern int			prof_on;
void				prof_toggle();
void				prof_start();
void				prof_add(const char *label, int divisor=1);
void				prof_sum(const char *label, int count);//add the sum of last 'count' steps
void				prof_loop_start(const char **labels, int n);//describe the loop body parts in 'labels'
void				prof_add_loop(int idx);//call on each part of loop body
void				prof_print();
#endif

//system
#ifdef _WINDOWS
#define				ALIGN(AMMOUNT)	__declspec(align(AMMOUNT))
extern HDC			ghDC;
extern bool			console_active;
void				console_show();
void				console_hide();
void				console_pause();
#elif defined __linux__
#define				sprintf_s snprintf
#define				vsprintf_s vsnprintf
#define				_HUGE HUGE_VAL
#define				ALIGN(AMMOUNT)	__attribute__((aligned(AMMOUNT)))
#define				console_show()
#define				console_hide()
#define				console_pause()
#endif
//void				messageboxw(const wchar_t *title, const wchar_t *format, ...);
void				messagebox(const char *title, const char *format, ...);
void				copy_to_clipboard_c(const char *a, int size);
bool				paste_from_clipboard(char *&str, int &len);//filters out '\r', don't forget to delete[] str
void				get_window_title(char *str, int size);
void				set_window_title(const char *str);
//void				set_window_title(const char *str);
int					GUINPrint(int x, int y, int w, int h, const char *a, ...);
long				GUITPrint(int x, int y, const char *a, ...);
void				GUIPrint(int x, int y, const char *a, ...);
const char*			open_file_dialog();
const char*			save_file_dialog();
bool				open_text_file(const char *filename, std::string &str);
bool				save_text_file(const char *filename, std::string &str);
int					ask_to_save();
void				mouse_capture();
void				mouse_release();
bool				get_key_state(int key);
//inline bool			get_key_state(int key)
//{
//#ifdef _WINDOWS
//	return GetAsyncKeyState(key)>>15!=0;
//#elif defined __linux__
//	XkbGetState();
//	return false;
//#else
//	return false;
//#endif
//}
//void				update_key_state(char key);
//void				update_main_key_states();
void				swap_buffers();

//function removers
//#ifdef __linux__
//#define messageboxa(...)
//#define copy_to_clipboard_c(...)
//#endif

//key macros
#ifdef _WINDOWS

#define TK_LBUTTON		VK_LBUTTON
#define TK_RBUTTON		VK_RBUTTON
#define TK_MBUTTON		VK_MBUTTON

#define TK_ESC			VK_ESCAPE
#define TK_TAB			VK_TAB
#define TK_CAPSLOCK		VK_CAPITAL
#define	TK_LSHIFT		VK_LSHIFT
#define	TK_RSHIFT		VK_RSHIFT
#define	TK_LCTRL		VK_LCONTROL
#define	TK_RCTRL		VK_RCONTROL
#define	TK_LALT			VK_LMENU
#define	TK_RALT			VK_RMENU
#define TK_LSTART		VK_LWIN
#define TK_RSTART		VK_RWIN

#define TK_DELETE		VK_DELETE
#define TK_BACKSPACE	VK_BACK
#define TK_ENTER		VK_RETURN

#define TK_HOME			VK_HOME
#define TK_END			VK_END
#define TK_PGUP			VK_PRIOR
#define	TK_PGDOWN		VK_NEXT
#define TK_LEFT			VK_LEFT
#define TK_RIGHT		VK_RIGHT
#define TK_UP			VK_UP
#define TK_DOWN			VK_DOWN
//#define TK_PRINTSCREEN	VK_SNAPSHOT

#define TK_F1			VK_F1
#define TK_F2			VK_F2
#define TK_F3			VK_F3
#define TK_F4			VK_F4
#define TK_F5			VK_F5
#define TK_F6			VK_F6
#define TK_F7			VK_F7
#define TK_F8			VK_F8
#define TK_F9			VK_F9
#define TK_F10			VK_F10
#define TK_F11			VK_F11
#define TK_F12			VK_F12

#elif defined __linux__

enum WindowMessages
{
	WM_IGNORED,

	WM_MOUSEMOVE,
	WM_MOUSEWHEEL,
	WM_LBUTTONDOWN,
	WM_LBUTTONDBLCLK,
	WM_LBUTTONUP,
	WM_RBUTTONDOWN,
	WM_RBUTTONDBLCLK,
	WM_RBUTTONUP,
	WM_KEYDOWN,
	WM_SYSKEYDOWN,
	WM_KEYUP,
	WM_SYSKEYUP,
};

#define TK_LBUTTON		//???
#define TK_RBUTTON		//???
#define TK_MBUTTON		//???

#define TK_ESC			XK_Escape
#define TK_TAB			XK_Tab
#define TK_CAPSLOCK		XK_Caps_Lock
#define TK_NUMLOCK		XK_Num_Lock
#define	TK_LSHIFT		XK_Shift_L
#define	TK_RSHIFT		XK_Shift_R
#define	TK_LCTRL		XK_Control_L
#define	TK_RCTRL		XK_Control_R
#define	TK_LALT			XK_Alt_L
#define	TK_RALT			XK_Alt_R
#define TK_LSTART		XK_Super_L
#define TK_RSTART		XK_Super_R

#define	TK_INSERT		XK_Insert
#define TK_DELETE		XK_Delete
#define TK_BACKSPACE	XK_BackSpace
#define TK_ENTER		XK_Return

#define TK_HOME			XK_Home
#define TK_END			XK_End
#define TK_PAGEUP		XK_Page_Up
#define	TK_PAGEDOWN		XK_Page_Down
#define TK_LEFT			XK_Left
#define TK_RIGHT		XK_Right
#define TK_UP			XK_Up
#define TK_DOWN			XK_Down
//#define TK_PRINTSCREEN	XK_Print

#define TK_F1			XK_F1
#define TK_F2			XK_F2
#define TK_F3			XK_F3
#define TK_F4			XK_F4
#define TK_F5			XK_F5
#define TK_F6			XK_F6
#define TK_F7			XK_F7
#define TK_F8			XK_F8
#define TK_F9			XK_F9
#define TK_F10			XK_F10
#define TK_F11			XK_F11
#define TK_F12			XK_F12

#endif

//OpenGL
#if 1
extern const char		*GLversion;
//extern int		glMajorVer, glMinorVer;
#ifdef _WINDOWS
enum				GL_State
{
	GL2_NOTHING,
	GL2_LOADING_API, GL2_API_LOADED,
	GL2_CREATING_CONTEXT, GL2_CONTEXT_CREATED,
	GL2_COMPILING_SHADERS, GL2_SHADERS_COMPILED,
	GL2_READY,
};
extern GL_State		GL2_state;
#define				GL_FUNC_ADD				0x8006//GL/glew.h
#define				GL_MIN					0x8007
#define				GL_MAX					0x8008
#define				GL_MAJOR_VERSION		0x821B
#define				GL_MINOR_VERSION		0x821C
#define				GL_TEXTURE0				0x84C0
#define				GL_TEXTURE1				0x84C1
#define				GL_TEXTURE2				0x84C2
#define				GL_TEXTURE3				0x84C3
#define				GL_TEXTURE4				0x84C4
#define				GL_TEXTURE5				0x84C5
#define				GL_TEXTURE6				0x84C6
#define				GL_TEXTURE7				0x84C7
#define				GL_TEXTURE8				0x84C8
#define				GL_TEXTURE9				0x84C9
#define				GL_TEXTURE10			0x84CA
#define				GL_TEXTURE11			0x84CB
#define				GL_TEXTURE12			0x84CC
#define				GL_TEXTURE13			0x84CD
#define				GL_TEXTURE14			0x84CE
#define				GL_TEXTURE15			0x84CF
#define				GL_TEXTURE_RECTANGLE	0x84F5
#define				GL_PROGRAM_POINT_SIZE	0x8642
#define				GL_BUFFER_SIZE			0x8764
#define				GL_ARRAY_BUFFER			0x8892
#define				GL_ELEMENT_ARRAY_BUFFER	0x8893
#define				GL_STATIC_DRAW			0x88E4
#define				GL_FRAGMENT_SHADER		0x8B30
#define				GL_VERTEX_SHADER		0x8B31
#define				GL_COMPILE_STATUS		0x8B81
#define				GL_LINK_STATUS			0x8B82
#define				GL_INFO_LOG_LENGTH		0x8B84
#define				GL_DEBUG_OUTPUT			0x92E0//OpenGL 4.3+
extern void			(__stdcall *glBlendEquation)(unsigned mode);
//extern void		(__stdcall *glGenVertexArrays)(int n, unsigned *arrays);//OpenGL 3.0
//extern void		(__stdcall *glDeleteVertexArrays)(int n, unsigned *arrays);//OpenGL 3.0
extern void			(__stdcall *glBindVertexArray)(unsigned arr);//OpenGL 3.0
extern void			(__stdcall *glGenBuffers)(int n, unsigned *buffers);
extern void			(__stdcall *glBindBuffer)(unsigned target, unsigned buffer);
extern void			(__stdcall *glBufferData)(unsigned target, int size, const void *data, unsigned usage);
extern void			(__stdcall *glBufferSubData)(unsigned target, int offset, int size, const void *data);
extern void			(__stdcall *glEnableVertexAttribArray)(unsigned index);
extern void			(__stdcall *glVertexAttribPointer)(unsigned index, int size, unsigned type, unsigned char normalized, int stride, const void *pointer);
extern void			(__stdcall *glDisableVertexAttribArray)(unsigned index);
extern unsigned		(__stdcall *glCreateShader)(unsigned shaderType);
extern void			(__stdcall *glShaderSource)(unsigned shader, int count, const char **string, const int *length);
extern void			(__stdcall *glCompileShader)(unsigned shader);
extern void			(__stdcall *glGetShaderiv)(unsigned shader, unsigned pname, int *params);
extern void			(__stdcall *glGetShaderInfoLog)(unsigned shader, int maxLength, int *length, char *infoLog);
extern unsigned		(__stdcall *glCreateProgram)();
extern void			(__stdcall *glAttachShader)(unsigned program, unsigned shader);
extern void			(__stdcall *glLinkProgram)(unsigned program);
extern void			(__stdcall *glGetProgramiv)(unsigned program, unsigned pname, int *params);
extern void			(__stdcall *glGetProgramInfoLog)(unsigned program, int maxLength, int *length, char *infoLog);
extern void			(__stdcall *glDetachShader)(unsigned program, unsigned shader);
extern void			(__stdcall *glDeleteShader)(unsigned shader);
extern void			(__stdcall *glUseProgram)(unsigned program);
extern int			(__stdcall *glGetAttribLocation)(unsigned program, const char *name);
extern void			(__stdcall *glDeleteProgram)(unsigned program);
extern void			(__stdcall *glDeleteBuffers)(int n, const unsigned *buffers);
extern int			(__stdcall *glGetUniformLocation)(unsigned program, const char *name);
extern void			(__stdcall *glUniformMatrix3fv)(int location, int count, unsigned char transpose, const float *value);
extern void			(__stdcall *glUniformMatrix4fv)(int location, int count, unsigned char transpose, const float *value);
extern void			(__stdcall *glGetBufferParameteriv)(unsigned target, unsigned value, int *data);
extern void			(__stdcall *glActiveTexture)(unsigned texture);
extern void			(__stdcall *glUniform1i)(int location, int v0);
extern void			(__stdcall *glUniform2i)(int location, int v0, int v1);
extern void			(__stdcall *glUniform1f)(int location, float v0);
extern void			(__stdcall *glUniform2f)(int location, float v0, float v1);
extern void			(__stdcall *glUniform3f)(int location, float v0, float v1, float v2);
extern void			(__stdcall *glUniform3fv)(int location, int count, const float *value);
extern void			(__stdcall *glUniform4f)(int location, float v0, float v1, float v2, float v3);
void				load_OGL_API();
#endif

extern unsigned		current_program;
struct				ShaderVar2
{
	int *pvar;//initialize to -1
	const char *name;
};
struct				ShaderProgram
{
	const char *name,//program name for error reporting
		*vsrc, *fsrc;
	ShaderVar2 *attributes, *uniforms;
	int n_attr, n_unif;
	unsigned program;//initialize to 0
};
void 				gl_check(const char *file, int line);
#define				GL_CHECK()		gl_check(file, __LINE__)
void				gl_error(const char *file, int line);
#define				GL_ERROR()		gl_error(file, __LINE__)
void				make_gl_program(ShaderProgram &p);
void				setGLProgram(unsigned program);
void				send_color(unsigned location, int color);
void				send_color_rgb(unsigned location, int color);
void				send_vec3(unsigned location, const float *v);
void				send_texture_pot(unsigned gl_texture, int *rgb, int txw, int txh);
void				select_texture(unsigned tx_id, int u_location);
void				set_region_immediate(int x1, int x2, int y1, int y2);
void				resize_gl();
void				toNDC_nobias(float xs, float ys, float &xn, float &yn);
void				toNDC(float xs, float ys, float &xn, float &yn);//2021-01-14 test: fill the screen with horizontal or vertical lines
#endif

//tt
extern char			caps_lock;
extern unsigned		font_txid;
typedef unsigned long long u64;
union				U64
{
	u64 data;
	struct{unsigned lo, hi;};
	U64(u64 data):data(data){}
	U64(unsigned lo, unsigned hi):data(hi)
	{
		data<<=32;
		data|=lo;
	}
};
extern std::string	filename;
void				set_text_colors(U64 const &colors);
int					print_line(short x, short y, const char *msg, int msg_length, short tab_origin, short zoom);
int					print(short zoom, short tab_origin, short x, short y, const char *format, ...);

void				wnd_on_create();
bool				wnd_on_init();//return false for EXIT_FAILURE
void				wnd_on_resize();
void				wnd_on_render();

//return true to redraw:
bool				wnd_on_mousemove();
bool				wnd_on_mousewheel(bool mw_forward);
bool				wnd_on_lbuttondown();
bool				wnd_on_lbuttonup();
bool				wnd_on_rbuttondown();
bool				wnd_on_rbuttonup();

bool				wnd_on_display_help();
bool				wnd_on_toggle_profiler();
bool				wnd_on_open();
bool				wnd_on_select_all();
bool				wnd_on_deselect();
bool				wnd_on_save(bool save_as);
bool				wnd_on_clear_hist();
bool				wnd_on_undo();
bool				wnd_on_redo();
bool				wnd_on_copy();
bool				wnd_on_paste();
bool				wnd_on_new();
bool				wnd_on_scroll_up_key();
bool				wnd_on_scroll_down_key();
bool				wnd_on_skip_word_left();
bool				wnd_on_skip_word_right();
bool				wnd_on_goto_file_start();
bool				wnd_on_goto_file_end();
bool				wnd_on_goto_line_start();
bool				wnd_on_goto_line_end();
bool				wnd_on_begin_rectsel();
bool				wnd_on_deletechar();
bool				wnd_on_backspace();
bool				wnd_on_cursor_up();
bool				wnd_on_cursor_down();
bool				wnd_on_cursor_left();
bool				wnd_on_cursor_right();
bool				wnd_on_type(char character);
//bool				wnd_on_input(int message, int wParam, int lParam);

bool				wnd_on_quit();//return false to deny exit

void				report_errors();

#endif