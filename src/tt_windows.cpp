//tt_system.cpp - Windows-dependent code
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

//TODO: UTF8 everywhere

#ifdef _WINDOWS
#include			"tt.h"
#include			<stdio.h>
#include			<vector>
#include			<algorithm>
#include			<io.h>//for console
#include			<fcntl.h>
#include			<iostream>
#define				scanf	scanf_s
//#include			<Windows.h>//included in tt.h
#pragma				comment(lib, "OpenGL32.lib")
#pragma				comment(lib, "GLU32.lib")
const char			file[]=__FILE__;

wchar_t				g_wbuf[G_BUF_SIZE]={};
static HINSTANCE	ghInstance=nullptr;
static HWND			ghWnd=nullptr;
static RECT			R={};
HDC					ghDC=nullptr;
static HGLRC		hRC=nullptr;
#if 0
static HDC			ghMemDC=nullptr;
static HBITMAP		hBitmap=nullptr;
int					*rgb=nullptr, rgbn=0;
static HFONT		hFont=nullptr;
#endif
char				keyboard[256]={};

#if 0
struct				Font
{
	int type;
	LOGFONTW lf;
	TEXTMETRICW tm;
	std::vector<int> sizes;//if empty, use truetypesizes
	Font():type(-1){}
	Font(int type, LOGFONTW const *lf, TEXTMETRICW const *tm):type(type)
	{
		memcpy(&this->lf, lf, sizeof LOGFONTW);
		memcpy(&this->tm, tm, sizeof TEXTMETRICW);

		static const wchar_t *bitmapfontnames[]=
		{
			L"Courier",
			L"Fixedsys",
			L"MS Sans Serif",
			L"MS Serif",
			L"Small Fonts",
			L"System",
		};
		enum BitmapFont{BF_COURIER, BF_FIXEDSYS, BF_MS_SANS_SERIF, BF_MS_SERIF, BF_SMALL_FONTS, BF_SYSTEM,		NBITMAPFONTS};
		static const int
			courier		[]={10, 12, 15},					courier_size	=sizeof courier>>2,
			fixedsys	[]={9},								fixedsys_size	=sizeof fixedsys>>2,
			mssansserif	[]={8, 10, 12, 14, 18, 24},			mssansserif_size=sizeof mssansserif>>2,
			msserif		[]={6, 7, 8, 10, 12, 14, 18, 24},	msserif_size	=sizeof msserif>>2,
			smallfonts	[]={2, 3, 4, 5, 6, 7, 8},			smallfonts_size	=sizeof smallfonts>>2,
			system		[]={10},							system_size		=sizeof system>>2;
		for(int k=0;k<NBITMAPFONTS;++k)
		{
			if(!wcscmp(lf->lfFaceName, bitmapfontnames[k]))//match
			{
				const int *sizes=nullptr;
				int size=0;
				switch(k)
				{
				case BF_COURIER:		sizes=courier,		size=courier_size;		break;
				case BF_FIXEDSYS:		sizes=fixedsys,		size=fixedsys_size;		break;
				case BF_MS_SANS_SERIF:	sizes=mssansserif,	size=mssansserif_size;	break;
				case BF_MS_SERIF:		sizes=msserif,		size=msserif_size;		break;
				case BF_SMALL_FONTS:	sizes=smallfonts,	size=smallfonts_size;	break;
				case BF_SYSTEM:			sizes=system,		size=system_size;		break;
				}
				this->sizes.assign(sizes, sizes+size);
				break;
			}
		}
	}
};
bool				operator<(Font const &a, Font const &b){return wcscmp(a.lf.lfFaceName, b.lf.lfFaceName)<0;}
const int			truetypesizes[]={8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72};
const int			truetypesizes_size=sizeof(truetypesizes)>>2;
std::vector<Font>	fonts;
bool				bold=false, italic=false, underline=false;
#endif

//OS-dependent error handling:
const char*			wm2str(int message)
{
	const char *a="???";
	switch(message)
	{//message case
#define		MC(x)	case x:a=#x;break;
	MC(WM_NULL)
	MC(WM_CREATE)
	MC(WM_DESTROY)
	MC(WM_MOVE)
	MC(WM_SIZE)

	MC(WM_ACTIVATE)
	MC(WM_SETFOCUS)
	MC(WM_KILLFOCUS)
	MC(WM_ENABLE)
	MC(WM_SETREDRAW)
	MC(WM_SETTEXT)
	MC(WM_GETTEXT)
	MC(WM_GETTEXTLENGTH)
	MC(WM_PAINT)
	MC(WM_CLOSE)

	MC(WM_QUERYENDSESSION)
	MC(WM_QUERYOPEN)
	MC(WM_ENDSESSION)

	MC(WM_QUIT)
	MC(WM_ERASEBKGND)
	MC(WM_SYSCOLORCHANGE)
	MC(WM_SHOWWINDOW)
	MC(WM_WININICHANGE)
//	MC(WM_SETTINGCHANGE)//==WM_WININICHANGE

	MC(WM_DEVMODECHANGE)
	MC(WM_ACTIVATEAPP)
	MC(WM_FONTCHANGE)
	MC(WM_TIMECHANGE)
	MC(WM_CANCELMODE)
	MC(WM_SETCURSOR)
	MC(WM_MOUSEACTIVATE)
	MC(WM_CHILDACTIVATE)
	MC(WM_QUEUESYNC)

	MC(WM_GETMINMAXINFO)

	MC(WM_PAINTICON)
	MC(WM_ICONERASEBKGND)
	MC(WM_NEXTDLGCTL)
	MC(WM_SPOOLERSTATUS)
	MC(WM_DRAWITEM)
	MC(WM_MEASUREITEM)
	MC(WM_DELETEITEM)
	MC(WM_VKEYTOITEM)
	MC(WM_CHARTOITEM)
	MC(WM_SETFONT)
	MC(WM_GETFONT)
	MC(WM_SETHOTKEY)
	MC(WM_GETHOTKEY)
	MC(WM_QUERYDRAGICON)
	MC(WM_COMPAREITEM)

	MC(WM_GETOBJECT)

	MC(WM_COMPACTING)
	MC(WM_COMMNOTIFY)
	MC(WM_WINDOWPOSCHANGING)
	MC(WM_WINDOWPOSCHANGED)

	MC(WM_POWER)

	MC(WM_COPYDATA)
	MC(WM_CANCELJOURNAL)

	MC(WM_NOTIFY)
	MC(WM_INPUTLANGCHANGEREQUEST)
	MC(WM_INPUTLANGCHANGE)
	MC(WM_TCARD)
	MC(WM_HELP)
	MC(WM_USERCHANGED)
	MC(WM_NOTIFYFORMAT)

	MC(WM_CONTEXTMENU)
	MC(WM_STYLECHANGING)
	MC(WM_STYLECHANGED)
	MC(WM_DISPLAYCHANGE)
	MC(WM_GETICON)
	MC(WM_SETICON)

	MC(WM_NCCREATE)
	MC(WM_NCDESTROY)
	MC(WM_NCCALCSIZE)
	MC(WM_NCHITTEST)
	MC(WM_NCPAINT)
	MC(WM_NCACTIVATE)
	MC(WM_GETDLGCODE)

	MC(WM_SYNCPAINT)

	MC(WM_NCMOUSEMOVE)
	MC(WM_NCLBUTTONDOWN)
	MC(WM_NCLBUTTONUP)
	MC(WM_NCLBUTTONDBLCLK)
	MC(WM_NCRBUTTONDOWN)
	MC(WM_NCRBUTTONUP)
	MC(WM_NCRBUTTONDBLCLK)
	MC(WM_NCMBUTTONDOWN)
	MC(WM_NCMBUTTONUP)
	MC(WM_NCMBUTTONDBLCLK)

	MC(WM_NCXBUTTONDOWN  )
	MC(WM_NCXBUTTONUP    )
	MC(WM_NCXBUTTONDBLCLK)

	MC(WM_INPUT_DEVICE_CHANGE)

	MC(WM_INPUT)

//	MC(WM_KEYFIRST   )//==WM_KEYDOWN
	MC(WM_KEYDOWN    )
	MC(WM_KEYUP      )
	MC(WM_CHAR       )
	MC(WM_DEADCHAR   )
	MC(WM_SYSKEYDOWN )
	MC(WM_SYSKEYUP   )
	MC(WM_SYSCHAR    )
	MC(WM_SYSDEADCHAR)

	MC(WM_UNICHAR)
//	MC(WM_KEYLAST)		//==WM_UNICHAR
	MC(UNICODE_NOCHAR)	//0xFFFF

	MC(WM_IME_STARTCOMPOSITION)
	MC(WM_IME_ENDCOMPOSITION)
	MC(WM_IME_COMPOSITION)
//	MC(WM_IME_KEYLAST)	//==WM_IME_KEYLAST

	MC(WM_INITDIALOG   )
	MC(WM_COMMAND      )
	MC(WM_SYSCOMMAND   )
	MC(WM_TIMER        )
	MC(WM_HSCROLL      )
	MC(WM_VSCROLL      )
	MC(WM_INITMENU     )
	MC(WM_INITMENUPOPUP)

	MC(WM_GESTURE      )
	MC(WM_GESTURENOTIFY)

	MC(WM_MENUSELECT)
	MC(WM_MENUCHAR  )
	MC(WM_ENTERIDLE )

	MC(WM_MENURBUTTONUP  )
	MC(WM_MENUDRAG       )
	MC(WM_MENUGETOBJECT  )
	MC(WM_UNINITMENUPOPUP)
	MC(WM_MENUCOMMAND    )

	MC(WM_CHANGEUISTATE)
	MC(WM_UPDATEUISTATE)
	MC(WM_QUERYUISTATE )

	MC(WM_CTLCOLORMSGBOX   )
	MC(WM_CTLCOLOREDIT     )
	MC(WM_CTLCOLORLISTBOX  )
	MC(WM_CTLCOLORBTN      )
	MC(WM_CTLCOLORDLG      )
	MC(WM_CTLCOLORSCROLLBAR)
	MC(WM_CTLCOLORSTATIC   )
	MC(MN_GETHMENU         )

//	MC(WM_MOUSEFIRST   )
	MC(WM_MOUSEMOVE    )
	MC(WM_LBUTTONDOWN  )
	MC(WM_LBUTTONUP    )
	MC(WM_LBUTTONDBLCLK)
	MC(WM_RBUTTONDOWN  )
	MC(WM_RBUTTONUP    )
	MC(WM_RBUTTONDBLCLK)
	MC(WM_MBUTTONDOWN  )
	MC(WM_MBUTTONUP    )
	MC(WM_MBUTTONDBLCLK)

	MC(WM_MOUSEWHEEL)

	MC(WM_XBUTTONDOWN  )
	MC(WM_XBUTTONUP    )
	MC(WM_XBUTTONDBLCLK)

//	MC(WM_MOUSELAST)	//==WM_MOUSEWHEEL

	MC(WM_PARENTNOTIFY )
	MC(WM_ENTERMENULOOP)
	MC(WM_EXITMENULOOP )

	MC(WM_NEXTMENU      )
	MC(WM_SIZING        )
	MC(WM_CAPTURECHANGED)
	MC(WM_MOVING        )

	MC(WM_POWERBROADCAST)

	MC(WM_DEVICECHANGE)

	MC(WM_MDICREATE     )
	MC(WM_MDIDESTROY    )
	MC(WM_MDIACTIVATE   )
	MC(WM_MDIRESTORE    )
	MC(WM_MDINEXT       )
	MC(WM_MDIMAXIMIZE   )
	MC(WM_MDITILE       )
	MC(WM_MDICASCADE    )
	MC(WM_MDIICONARRANGE)
	MC(WM_MDIGETACTIVE  )

	MC(WM_MDISETMENU    )
	MC(WM_ENTERSIZEMOVE )
	MC(WM_EXITSIZEMOVE  )
	MC(WM_DROPFILES     )
	MC(WM_MDIREFRESHMENU)

//	MC(WM_POINTERDEVICECHANGE    )
//	MC(WM_POINTERDEVICEINRANGE   )
//	MC(WM_POINTERDEVICEOUTOFRANGE)

	MC(WM_TOUCH)

//	MC(WM_NCPOINTERUPDATE      )
//	MC(WM_NCPOINTERDOWN        )
//	MC(WM_NCPOINTERUP          )
//	MC(WM_POINTERUPDATE        )
//	MC(WM_POINTERDOWN          )
//	MC(WM_POINTERUP            )
//	MC(WM_POINTERENTER         )
//	MC(WM_POINTERLEAVE         )
//	MC(WM_POINTERACTIVATE      )
//	MC(WM_POINTERCAPTURECHANGED)
//	MC(WM_TOUCHHITTESTING      )
//	MC(WM_POINTERWHEEL         )
//	MC(WM_POINTERHWHEEL        )
//	MC(DM_POINTERHITTEST       )

	MC(WM_IME_SETCONTEXT     )
	MC(WM_IME_NOTIFY         )
	MC(WM_IME_CONTROL        )
	MC(WM_IME_COMPOSITIONFULL)
	MC(WM_IME_SELECT         )
	MC(WM_IME_CHAR           )

	MC(WM_IME_REQUEST)

	MC(WM_IME_KEYDOWN)
	MC(WM_IME_KEYUP  )

	MC(WM_MOUSEHOVER)
	MC(WM_MOUSELEAVE)

	MC(WM_NCMOUSEHOVER)
	MC(WM_NCMOUSELEAVE)

	MC(WM_WTSSESSION_CHANGE)

	MC(WM_TABLET_FIRST)
	MC(WM_TABLET_LAST )

//	MC(WM_DPICHANGED)

	MC(WM_CUT              )
	MC(WM_COPY             )
	MC(WM_PASTE            )
	MC(WM_CLEAR            )
	MC(WM_UNDO             )
	MC(WM_RENDERFORMAT     )
	MC(WM_RENDERALLFORMATS )
	MC(WM_DESTROYCLIPBOARD )
	MC(WM_DRAWCLIPBOARD    )
	MC(WM_PAINTCLIPBOARD   )
	MC(WM_VSCROLLCLIPBOARD )
	MC(WM_SIZECLIPBOARD    )
	MC(WM_ASKCBFORMATNAME  )
	MC(WM_CHANGECBCHAIN    )
	MC(WM_HSCROLLCLIPBOARD )
	MC(WM_QUERYNEWPALETTE  )
	MC(WM_PALETTEISCHANGING)
	MC(WM_PALETTECHANGED   )
	MC(WM_HOTKEY           )

	MC(WM_PRINT      )
	MC(WM_PRINTCLIENT)

	MC(WM_APPCOMMAND)

	MC(WM_THEMECHANGED)

	MC(WM_CLIPBOARDUPDATE)

	MC(WM_DWMCOMPOSITIONCHANGED      )
	MC(WM_DWMNCRENDERINGCHANGED      )
	MC(WM_DWMCOLORIZATIONCOLORCHANGED)
	MC(WM_DWMWINDOWMAXIMIZEDCHANGE   )

	MC(WM_DWMSENDICONICTHUMBNAIL        )
	MC(WM_DWMSENDICONICLIVEPREVIEWBITMAP)

	MC(WM_GETTITLEBARINFOEX)

	MC(WM_HANDHELDFIRST)
	MC(WM_HANDHELDLAST )

	MC(WM_AFXFIRST)
	MC(WM_AFXLAST )

	MC(WM_PENWINFIRST)
	MC(WM_PENWINLAST )

	MC(WM_APP)

	MC(WM_USER)
#undef		MC
	}
	return a;
}
int					sys_check(const char *file, int line)
{
	int error=GetLastError();
	if(error)
	{
		char *messageBuffer=nullptr;
		size_t size=FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS, NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
		log_error(file, line, "System %d: %s", error, messageBuffer);
		LocalFree(messageBuffer);
	}
	return 0;
}
#define				SYS_ASSERT(SUCCESS)		((void)((SUCCESS)!=0||sys_check(file, __LINE__)))

//OS-dependent utilities:
bool				console_active=false;
void				print_closewarning(){printf("\n\tWARNING: CLOSING THIS WINDOW WILL CLOSE THE PROGRAM\n\n");}
void				console_show()//https://stackoverflow.com/questions/191842/how-do-i-get-console-output-in-c-with-a-windows-program
{
	if(!console_active)
	{
		console_active=true;
		int hConHandle;
		long lStdHandle;
		CONSOLE_SCREEN_BUFFER_INFO coninfo;
		FILE *fp;

		// allocate a console for this app
		AllocConsole();

		// set the screen buffer to be big enough to let us scroll text
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
		coninfo.dwSize.Y=4000;
		SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

		// redirect unbuffered STDOUT to the console
		lStdHandle=(long)GetStdHandle(STD_OUTPUT_HANDLE);
		hConHandle=_open_osfhandle(lStdHandle, _O_TEXT);
		fp=_fdopen(hConHandle, "w");
		*stdout=*fp;
		setvbuf(stdout, nullptr, _IONBF, 0);

		// redirect unbuffered STDIN to the console
		lStdHandle=(long)GetStdHandle(STD_INPUT_HANDLE);
		hConHandle=_open_osfhandle(lStdHandle, _O_TEXT);
		fp=_fdopen(hConHandle, "r");
		*stdin=*fp;
		setvbuf(stdin, nullptr, _IONBF, 0);

		// redirect unbuffered STDERR to the console
		lStdHandle=(long)GetStdHandle(STD_ERROR_HANDLE);
		hConHandle=_open_osfhandle(lStdHandle, _O_TEXT);
		fp=_fdopen(hConHandle, "w");
		*stderr=*fp;
		setvbuf(stderr, nullptr, _IONBF, 0);

		// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
		// point to console as well
		std::ios::sync_with_stdio();

		print_closewarning();//
	}
}
void				console_hide()
{
	if(console_active)
	{
		FreeConsole();
		console_active=false;
	}
}
void				console_pause()
{
	char c=0;
	scanf_s("%c", &c);
}
void				messagebox(const char *title, const char *format, ...)
{
	int len=vsprintf_s(g_buf, g_buf_size, format, (char*)(&format+1));
	len=MultiByteToWideChar(CP_UTF8, 0, g_buf, len, g_wbuf, g_buf_size);	SYS_ASSERT(len);
	g_wbuf[len]='\0';
	++len;
	int len2=MultiByteToWideChar(CP_UTF8, 0, title, strlen(title), g_wbuf+len, g_buf_size-len);	SYS_ASSERT(len2);
	g_wbuf[len+len2]='\0';
	MessageBoxW(ghWnd, g_wbuf, g_wbuf+len, MB_OK);
	//MessageBoxA(ghWnd, g_buf, title, MB_OK);
}
void				copy_to_clipboard_c(const char *a, int size)//size not including null terminator
{
	char *clipboard=(char*)LocalAlloc(LMEM_FIXED, (size+1)*sizeof(char));
	memcpy(clipboard, a, (size+1)*sizeof(char));
	clipboard[size]='\0';
	OpenClipboard(ghWnd);
	EmptyClipboard();
	SetClipboardData(CF_OEMTEXT, (void*)clipboard);
	CloseClipboard();
}
bool				paste_from_clipboard(char *&str, int &len)
{
	OpenClipboard(ghWnd);
	char *a=(char*)GetClipboardData(CF_OEMTEXT);
	if(!a)
	{
		CloseClipboard();
		messagebox("Error", "Failed to paste from clipboard");
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

	CloseClipboard();
	return true;
}
void				get_window_title(char *str, int size)
{
	int success=GetWindowTextW(ghWnd, g_wbuf, g_buf_size);	SYS_ASSERT(success);
	//char c[]="?";
	//int invalid=false;
	int len=WideCharToMultiByte(CP_UTF8, 0, g_wbuf, wcslen(g_wbuf), str, size, nullptr, nullptr);	SYS_ASSERT(len);
	if(!len)
	{
		str[0]='\0';
		return;
	}
	str[len]='\0';
	//GetWindowTextA(ghWnd, str, size);
}
//void				set_window_title(const char *str)
//{
//	int success=SetWindowTextA(ghWnd, str);
//	SYS_ASSERT(success);
//}
void				set_window_title(const char *str)
{
	int len=MultiByteToWideChar(CP_UTF8, 0, str, strlen(str), g_wbuf, g_buf_size);	SYS_ASSERT(len);
	if(!len)
		return;
	g_wbuf[len]='\0';
	int success=SetWindowTextW(ghWnd, g_wbuf);	SYS_ASSERT(success);
	//int success=SetWindowTextA(ghWnd, str);	SYS_ASSERT(success);
}
int					GUINPrint(int x, int y, int w, int h, const char *a, ...)
{
	int length=vsprintf_s(g_buf, g_buf_size, a, (char*)(&a+1));
	if(length>0)
	{
		RECT r={x, y, x+w, y+h};
		return DrawTextA(ghDC, g_buf, length, &r, DT_NOCLIP|DT_EXPANDTABS);
	}
	return 0;
}
long				GUITPrint(int x, int y, const char *a, ...)//return value: 0xHHHHWWWW		width=(short&)ret, height=((short*)&ret)[1]
{
	int length=vsprintf_s(g_buf, g_buf_size, a, (char*)(&a+1));
	if(length>0)
		return TabbedTextOutA(ghDC, x, y, g_buf, length, 0, 0, 0);
	return 0;
}
void				GUIPrint(int x, int y, const char *a, ...)
{
	int length=vsprintf_s(g_buf, g_buf_size, a, (char*)(&a+1)), success;
	if(length>0)
	{
		success=TextOutA(ghDC, x, y, g_buf, length);	SYS_ASSERT(success);
	}
}

const char*			open_file_dialog()
{
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
		return nullptr;

	//char c[]="?";
	//int invalid=false;
	int len=WideCharToMultiByte(CP_UTF8, 0, ofn.lpstrFile, wcslen(ofn.lpstrFile), g_buf, g_buf_size, nullptr, nullptr);	SYS_ASSERT(len);
	if(!len)
		return nullptr;
	g_buf[len]='\0';
	return g_buf;
	//memcpy(g_wbuf, ofn.lpstrFile, wcslen(ofn.lpstrFile)*sizeof(wchar_t));
	//return g_wbuf;
}
const char*			save_file_dialog()
{
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

	//char c[]="?";
	//int invalid=false;
	int len=WideCharToMultiByte(CP_UTF8, 0, ofn.lpstrFile, wcslen(ofn.lpstrFile), g_buf, g_buf_size, nullptr, nullptr);	SYS_ASSERT(len);
	if(!len)
		return nullptr;
	g_buf[len]='\0';
	return g_buf;
	//memcpy(g_wbuf, ofn.lpstrFile, wcslen(ofn.lpstrFile)*sizeof(wchar_t));
	//return g_wbuf;
}
bool				open_text_file(const char *filename, std::string &str)
{
	FILE *file=nullptr;
	int ret=fopen_s(&file, filename, "r");
	if(ret)
	{
		strerror_s(g_buf, g_buf_size, ret);
		messagebox("Error", "Cannot open \'%s\'.\n%s", filename, g_buf);
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
	FILE *file=nullptr;
	int ret=fopen_s(&file, filename, "w");
	if(ret)
	{
		strerror_s(g_buf, g_buf_size, ret);
		messagebox("Error", "Cannot save \'%s\'.\n%s", filename, g_buf);
		return false;
	}
	fwrite(str.c_str(), 1, str.size(), file);
	fclose(file);
	return true;
}
int					ask_to_save()
{
	if(filename.size())
		swprintf_s(g_wbuf, g_buf_size, L"Save changes to %s?", filename.c_str());
	else
		swprintf_s(g_wbuf, g_buf_size, L"Save changes to Untitled?");
	int result=MessageBoxW(ghWnd, g_wbuf, L"Paint++", MB_YESNOCANCEL);
	switch(result)
	{
	case IDYES:		return 0;
	case IDNO:		return 1;
	case IDCANCEL:	return 2;
	}
	return -1;
}

void				mouse_capture()
{
	SetCapture(ghWnd);
}
void				mouse_release()
{
	int success=ReleaseCapture();	SYS_ASSERT(success);
}
bool				get_key_state(int key)
{
	keyboard[key]=(GetAsyncKeyState(key)>>15)!=0;
	return keyboard[key]!=0;
}
void				update_main_key_states()
{
	get_key_state(VK_MENU);
	get_key_state(VK_SHIFT);
	get_key_state(VK_CONTROL);
}

#if 0
static void			font_change(int font_idx, int size)
{
	auto &font=fonts[font_idx];
	auto &lf=font.lf;
	if(hFont)
		DeleteObject(hFont);
	int nHeight=-MulDiv(size, GetDeviceCaps(ghMemDC, LOGPIXELSY), 72);
	hFont=CreateFontW(nHeight, 0, lf.lfEscapement, lf.lfOrientation, lf.lfWeight, italic, underline, lf.lfStrikeOut, lf.lfCharSet, lf.lfOutPrecision, lf.lfClipPrecision, lf.lfQuality, lf.lfPitchAndFamily, lf.lfFaceName);
}
void				font_use	(){hFont=(HFONT)SelectObject(ghMemDC, hFont);}
void				font_drop	(){hFont=(HFONT)SelectObject(ghMemDC, hFont);}
bool				font_set(const wchar_t *name, int size)
{
	for(int k=0;k<(int)fonts.size();++k)
	{
		auto &font=fonts[k];
		if(!wcscmp(name, font.lf.lfFaceName))
		{
			font_idx=k;
			font_change(font_idx, size);
			return true;
		}
	}
	LOG_ERROR("Font not found: \'%S\'", name);
	return false;
}
void				font_get_dimensions(short &x, short &y)
{
	SIZE size={};
	int success=GetTextExtentExPointW(ghMemDC, L"A", 1, 0, 0, 0, &size);
	x=(short)size.cx, y=(short)size.cy;
}
#endif

bool				resize_window()
{
	int w0=w, h0=h;
	RECT R;
	int success=GetClientRect(ghWnd, &R);	SYS_ASSERT(success);
	w=short(R.right-R.left), h=short(R.bottom-R.top);
	if(h<=0)
		h=1;
	return w0!=w||h0!=h;
}
void				swap_buffers()
{
	SwapBuffers(ghDC);
	//BitBlt(ghDC, 0, 0, w, h, ghMemDC, 0, 0, SRCCOPY);
}
const char			numrow[]=")!@#$%^&*(";
long				__stdcall WndProc(HWND__ *hWnd, unsigned message, unsigned wParam, long lParam)
{
	switch(message)
	{
	case WM_CREATE:
		wnd_on_create();
		break;
	case WM_SIZE:
		if(resize_window())
		{
			resize_gl();
			wnd_on_render();
		}
		/*{
			GetClientRect(ghWnd, &R);
			int h2=R.bottom-R.top, w2=R.right-R.left;
			if(h!=h2||w!=w2)
			{
				if(!h2)
					h2=1;
				h=h2, w=w2, rgbn=w*h;
				if(hBitmap)
				{
					hBitmap=(HBITMAP)SelectObject(ghMemDC, hBitmap);
					DeleteObject(hBitmap);
				}
				BITMAPINFO bmpInfo={{sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB, 0, 0, 0, 0, 0}};
				hBitmap=CreateDIBSection(0, &bmpInfo, DIB_RGB_COLORS, (void**)&rgb, 0, 0);
				hBitmap=(HBITMAP)SelectObject(ghMemDC, hBitmap);
			}
		}//*/
		break;
	case WM_PAINT:
		wnd_on_render();
		break;

	case WM_MOUSEMOVE:
		mx=(short&)lParam, my=((short*)&lParam)[1];
		if(wnd_on_mousemove())
			InvalidateRect(hWnd, nullptr, 0);
		break;

	case WM_MOUSEWHEEL:
		if(wnd_on_mousewheel(((short*)&wParam)[1]>0))
			InvalidateRect(hWnd, nullptr, 0);
		break;

	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		if(wnd_on_lbuttondown())
			InvalidateRect(hWnd, nullptr, 0);
		keyboard[VK_LBUTTON]=true;
		break;
	case WM_LBUTTONUP:
		if(wnd_on_lbuttonup())
			InvalidateRect(hWnd, nullptr, 0);
		keyboard[VK_LBUTTON]=false;
		break;

	case WM_RBUTTONDOWN:
	case WM_RBUTTONDBLCLK:
		if(wnd_on_rbuttondown())
			InvalidateRect(hWnd, nullptr, 0);
		keyboard[VK_RBUTTON]=true;
		break;
	case WM_RBUTTONUP:
		if(wnd_on_rbuttonup())
			InvalidateRect(hWnd, nullptr, 0);
		keyboard[VK_RBUTTON]=false;
		break;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		{
			bool redraw=false;
			if(keyboard[VK_CONTROL])
			{
				switch(wParam)
				{
				case VK_UP:		redraw=wnd_on_scroll_up_key();				break;
				case VK_DOWN:	redraw=wnd_on_scroll_down_key();			break;
				case VK_LEFT:	redraw=wnd_on_skip_word_left();				break;
				case VK_RIGHT:	redraw=wnd_on_skip_word_right();			break;
				case VK_HOME:	redraw=wnd_on_goto_file_start();			break;
				case VK_END:	redraw=wnd_on_goto_file_end();				break;
				case 'A':		redraw=wnd_on_select_all();					break;
				case 'C':		redraw=wnd_on_copy();						break;
				case 'D':		redraw=wnd_on_clear_hist();					break;
				case 'N':		redraw=wnd_on_new();						break;
				case 'O':		redraw=wnd_on_open();						break;
				case 'S':		redraw=wnd_on_save(keyboard[VK_SHIFT]!=0);	break;
				case 'V':		redraw=wnd_on_paste();						break;
				case 'Y':		redraw=wnd_on_redo();						break;
				case 'Z':		redraw=wnd_on_undo();						break;
				}
			}
			else
			{
				char shift=keyboard[VK_SHIFT], lowercase=caps_lock==shift;
				switch(wParam)
				{
				case VK_CAPITAL:caps_lock=!caps_lock;			break;
				case VK_MENU:	redraw=wnd_on_begin_rectsel();	break;
				case VK_ESCAPE:	redraw=wnd_on_deselect();		break;
				case VK_F1:		redraw=wnd_on_display_help();	break;
				case VK_F4:
					if(keyboard[VK_MENU])
					{
						if(wnd_on_quit())
							PostQuitMessage(0);
					}
					else
						redraw=wnd_on_toggle_profiler();
					break;
				case VK_UP:		redraw=wnd_on_cursor_up();		break;
				case VK_DOWN:	redraw=wnd_on_cursor_down();	break;
				case VK_LEFT:	redraw=wnd_on_cursor_left();	break;
				case VK_RIGHT:	redraw=wnd_on_cursor_right();	break;
				case VK_HOME:	redraw=wnd_on_goto_line_start();break;
				case VK_END:	redraw=wnd_on_goto_line_end();	break;
				case VK_DELETE:	redraw=wnd_on_deletechar();		break;
				case VK_BACK:	redraw=wnd_on_backspace();		break;

				case VK_OEM_1:		redraw=wnd_on_type(shift?	':':';'		);break;
				case VK_OEM_2:		redraw=wnd_on_type(shift?	'?':'/'		);break;
				case VK_OEM_3:		redraw=wnd_on_type(shift?	'~':'`'		);break;
				case VK_OEM_4:		redraw=wnd_on_type(shift?	'{':'['		);break;
				case VK_OEM_5:		redraw=wnd_on_type(shift?	'|':'\\'	);break;
				case VK_OEM_6:		redraw=wnd_on_type(shift?	'}':']'		);break;
				case VK_OEM_7:		redraw=wnd_on_type(shift?	'\"':'\''	);break;
				case VK_OEM_MINUS:	redraw=wnd_on_type(shift?	'_':'-'		);break;
				case VK_OEM_PLUS:	redraw=wnd_on_type(shift?	'+':'='		);break;
				case VK_OEM_COMMA:	redraw=wnd_on_type(shift?	'<':','		);break;
				case VK_OEM_PERIOD:	redraw=wnd_on_type(shift?	'>':'.'		);break;
				case VK_DECIMAL:	redraw=wnd_on_type(			'.'			);break;
				case VK_ADD:		redraw=wnd_on_type(			'+'			);break;
				case VK_SUBTRACT:	redraw=wnd_on_type(			'-'			);break;
				case VK_MULTIPLY:	redraw=wnd_on_type(			'*'			);break;
				case VK_DIVIDE:		redraw=wnd_on_type(			'/'			);break;
				case ' ':			redraw=wnd_on_type(			' '			);break;
				case '\t':			redraw=wnd_on_type(			'\t'		);break;
				case '\r':			redraw=wnd_on_type(			'\n'		);break;
				default:
					if(wParam>='A'&&wParam<='Z')
						redraw=wnd_on_type(wParam+('a'-'A')*lowercase);
					else if(wParam>='0'&&wParam<='9')
					{
						if(shift)
							redraw=wnd_on_type(numrow[wParam-'0']);
						else
							redraw=wnd_on_type(wParam);
					}
					else if(wParam>=VK_NUMPAD0&&wParam<=VK_NUMPAD9)
						redraw=wnd_on_type(wParam+'0'-VK_NUMPAD0);
					break;
				}
			}
			if(redraw)
				InvalidateRect(hWnd, nullptr, 0);
		}
		//if(wParam==VK_F4&&keyboard[VK_MENU])
		//{
		//	if(wnd_on_quit())
		//		PostQuitMessage(0);
		//}
		//if(wnd_on_input(hWnd, message, wParam, lParam))
		//	InvalidateRect(hWnd, nullptr, 0);
		keyboard[wParam]=true;
		update_main_key_states();
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		//if(wnd_on_input(hWnd, message, wParam, lParam))
		//	InvalidateRect(hWnd, nullptr, 0);
		keyboard[wParam]=false;
		update_main_key_states();
		break;

	case WM_CLOSE:
//	case WM_DESTROY:
		if(wnd_on_quit())
			PostQuitMessage(0);
		else
			return 0;
#if 0
		if(modified)
		{
			int result=ask_to_save();
			switch(result)
			{
			case IDYES:
				if(filepath.size())
				{
					save_media(filepath);
					PostQuitMessage(0);
				}
				else if(save_media_as())
					PostQuitMessage(0);
				else
					return 0;
				break;
			case IDNO:
				PostQuitMessage(0);
				break;
			case IDCANCEL:
				return 0;
			}
		}
		else
			PostQuitMessage(0);
#endif
		break;
	}
	return DefWindowProcA(hWnd, message, wParam, lParam);
}
#if 0
int					__stdcall FontProc(LOGFONTW const *lf, TEXTMETRICW const *tm, unsigned long FontType, long lParam)
{
	fonts.push_back(Font(FontType, lf, tm));
	return true;//nonzero to continue enumeration
}
#endif
int					__stdcall WinMain(HINSTANCE__ *hInstance, HINSTANCE__ *hPrevInstance, char *lpCmdLine, int nCmdShow)
{
	//char LOL_1[10]={};
	//wchar_t LOL_2[10]=L"Hello";
	//int length=WideCharToMultiByte(CP_UTF8, 0, LOL_2, 10, LOL_1, strlen(LOL_1), nullptr, nullptr);
	prof_start();
	{
		int len=GetModuleFileNameW(nullptr, g_wbuf, g_buf_size);
		int len2=WideCharToMultiByte(CP_UTF8, 0, g_wbuf, len, g_buf, g_buf_size, nullptr, nullptr);
		int k=len2-1;
		for(;k>0&&g_buf[k-1]!='/'&&g_buf[k-1]!='\\';--k);
		exe_dir.assign(g_buf, g_buf+k);
		for(k=0;k<(int)exe_dir.size();++k)
			if(exe_dir[k]=='\\')
				exe_dir[k]='/';
	}
	ghInstance=hInstance;
	tagWNDCLASSEXA wndClassEx=
	{
		sizeof(tagWNDCLASSEXA),
		CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS,
		WndProc, 0, 0, hInstance,

		LoadIconA(nullptr, (char*)IDI_APPLICATION),//icon

		LoadCursorA(nullptr, (char*)IDC_IBEAM),//cursor
		//LoadCursorA(nullptr, (char*)IDC_ARROW),

		nullptr,//bk brush
		0, "New format", 0
	};
	short success=RegisterClassExA(&wndClassEx);	SYS_ASSERT(success);
	ghWnd=CreateWindowExA(WS_EX_ACCEPTFILES, wndClassEx.lpszClassName, "Teletext", WS_CAPTION|WS_SYSMENU|WS_THICKFRAME|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_CLIPCHILDREN, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);	SYS_ASSERT(ghWnd);//2021-11-02

		//init callback
		GetClientRect(ghWnd, &R);
		w=R.right-R.left, h=R.bottom-R.top;
		ghDC=GetDC(ghWnd);
#if 0
		ghMemDC=CreateCompatibleDC(ghDC);
		BITMAPINFO bmpInfo={{sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB, 0, 0, 0, 0, 0}};
		hBitmap=CreateDIBSection(0, &bmpInfo, DIB_RGB_COLORS, (void**)&rgb, 0, 0);
		hBitmap=(HBITMAP)SelectObject(ghMemDC, hBitmap);

		tagLOGFONTW lf=//load fonts
		{
			0, 0,//h, w
			100,//escapement (*0.1 degree)
			0,//orientation
			0,//weight
			0, 0, 0,//italic, underline, strikeout
			0,//charset
			0, 0, 0,//outprecision, clipprecision, quality
			0,//pitch and family
			{L'\0'},//facename
		};
		EnumFontFamiliesExW(ghMemDC, &lf, FontProc, 0, 0);
		std::sort(fonts.begin(), fonts.end());
#endif
		
		//initialize OpenGL
		tagPIXELFORMATDESCRIPTOR pfd=
		{
			sizeof(tagPIXELFORMATDESCRIPTOR), 1,
			PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER,
			PFD_TYPE_RGBA, 32,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			16,//depth bits
			0, 0, PFD_MAIN_PLANE, 0, 0, 0, 0
		};
		int PixelFormat=ChoosePixelFormat(ghDC, &pfd);
		prof_add("ChoosePixelFormat");//either this takes 60ms
		SetPixelFormat(ghDC, PixelFormat, &pfd);
		prof_add("SetPixelFormat");//...or this 60ms
		hRC=wglCreateContext(ghDC);
		prof_add("wglCreateContext");//37.5ms
		wglMakeCurrent(ghDC, hRC);
		prof_add("wglMakeCurrent");
		resize_gl();
		prof_add("glViewport");
		glShadeModel(GL_SMOOTH);
		prof_add("glShadeModel");
	//	auto p=(byte*)&clearcolor;
	//	glClearColor(p[0]*inv255, p[1]*inv255, p[2]*inv255, p[3]*inv255);
		glClearColor(0, 0, 0, 1);
		prof_add("glClearColor");
		glClearDepth(1);
		prof_add("glClearDepth");
		glDisable(GL_DEPTH_TEST);//this program is pure 2D
		prof_add("disable depth");
	//	glDepthFunc(GL_LEQUAL);
	//	prof_add("glDepthFunc");
	//	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	//	prof_add("glHint");

		GLversion=(const char*)glGetString(GL_VERSION);//get OpenGL version
		//glGetIntegerv(GL_MAJOR_VERSION, &glMajorVer);
		//glGetIntegerv(GL_MINOR_VERSION, &glMinorVer);

		load_OGL_API();
		prof_add("load API");

		glEnable(GL_BLEND);									GL_CHECK();
		glBlendEquation(GL_FUNC_ADD);						GL_CHECK();
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	GL_CHECK();
		prof_add("enable blend");

		if(!wnd_on_init())
			return EXIT_FAILURE;

	ShowWindow(ghWnd, nCmdShow);
	
	tagMSG msg;
	int ret=0;
	for(;ret=GetMessageA(&msg, 0, 0, 0);)
	{
		if(ret==-1)
		{
			LOG_ERROR("GetMessage returned -1 with: hwnd=%08X, msg=%s, wP=%d, lP=%d. \nQuitting.", msg.hwnd, wm2str(msg.message), msg.wParam, msg.lParam);
			break;
		}
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}

		//finish callback
#if 0
		if(hBitmap)
		{
			hBitmap=(HBITMAP)SelectObject(ghMemDC, hBitmap);
			DeleteObject(hBitmap);
		}
		if(hFont)
			DeleteObject(hFont);
		DeleteDC(ghMemDC);
#endif
		ReleaseDC(ghWnd, ghDC);

	return msg.wParam;
}
#endif