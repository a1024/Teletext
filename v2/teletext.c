#include "teletext.h"

ArrayHandle openfiles=0;
int current_file=0;

/*void tabs_switchto(int k, int change_title)
{
	TextFile *of=0;
	if(current_file>=0&&current_file<(int)openfiles->count)
	{
		of=(TextFile*)array_at(&openfiles, current_file);
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
}//*/

int io_init(int argc, char **argv)//return false to abort
{
	set_window_title("Teletext");
	glClearColor(1, 1, 1, 1);
	return 1;
}
void io_resize()
{
}
int io_mousemove()//return true to redraw
{
	return 0;
}
int io_mousewheel(int forward)
{
	return 0;
}
int io_keydn(IOKey key, char c)
{
	if(GET_KEY_STATE(KEY_CTRL))
	{
		switch(key)
		{
		case KEY_TAB:
			if(GET_KEY_STATE(VK_SHIFT))//prev tab
			{
				if(openfiles->count>1)
			}
			else//next tab
			{
			}
			break;
		case KEY_PLUS://zoom in
			break;
		case KEY_MINUS://zoom out
			break;
		}
	}
	else
	{
	}
	return 0;
}
int io_keyup(IOKey key, char c)
{
	return 0;
}
void io_timer()
{
}
void io_render()
{
}
int io_quit_request()//return 1 to exit
{
	return 1;
}
void io_cleanup()
{
}
