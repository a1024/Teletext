#include"teletext.h"
#include"text.h"
static const char file[]=__FILE__;

typedef void *Text;
int mline=0, mcol=0,//current mouse coordinates
	start_mx=0, start_my=0,//LBUTTON down coordinates
	tab_count=4;
ArrayHandle exedir=0;
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

int col2idx(const char *text, int text_length, int tab0_cols, int idx0, int col0, float req_col, int *ret_idx, int *ret_col, float colroundbias)//returns true when runs out of characters (OOB)
{
	int line_too_short=1;
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
			line_too_short=0;
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
int idx2col(const char *text, int text_length, int tab0_cols)
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
void bookmark_update_col(Bookmark *bm, Text const *text)
{
	size_t len=0;
	char *_line=text_get_line(*text, bm->line, &len);
	bm->col=idx2col(_line, bm->idx, 0);//what if idx > size ?
}
void bookmark_update_idx(Bookmark *bm, Text const *text)
{
	size_t len=0;
	char *_line=text_get_line(*text, bm->line, &len);
	col2idx(_line, len, 0, 0, 0, (float)bm->col, &bm->idx, &bm->col, 0.5f);
}
int bookmark_set_gui(Bookmark *bm, Text const *text, int line, float col, int clampcol, float colroundbias)
{
	int hit=1;
	size_t nlines=text_get_nlines(*text);
	if(line<0)
		line=0, hit=0;
	else if(line>(int)nlines-1)
			line=nlines-1, hit=0;
	bm->line=line;

	size_t len=0;
	char *_line=text_get_line(*text, line, &len);
	col2idx(_line, len, 0, 0, 0, col, &bm->idx, clampcol?&bm->col:0, 1-colroundbias);
	if(!clampcol)
		bm->col=(int)(col+colroundbias);
	return hit;
}
void bookmark_set_text(Bookmark *bm, Text const *text, int line, int idx)
{
	size_t nlines=text_get_nlines(*text);
	if(line<0)
		line=0;
	else if(line>(int)nlines-1)
		line=nlines-1;
	bm->line=line, bm->idx=idx;
	bookmark_update_col(bm, *text);
}
void bookmark_setzero(Bookmark *bm)
{
	bm->line=bm->idx=bm->col=0;
}
void bookmark_setend(Bookmark *bm, Text const *text)
{
	size_t nlines=text_get_nlines(*text);
	bm->line=nlines-1;
	bm->idx=text_get_len(*text, bm->line);
	bookmark_update_col(bm, *text);
}
void bookmark_set_linestart(Bookmark *bm)
{
	bm->idx=bm->col=0;
}
void bookmark_set_lineend(Bookmark *bm, Text const *text)
{
	bm->idx=text_get_len(*text, bm->line);
	bookmark_update_col(bm, *text);
}
int bookmark_get_absidx(Bookmark const *bm, Text const *text)
{
	int absidx=0;
	for(int kl=0;kl<bm->line;++kl)
		absidx+=text_get_len(*text, kl)+1;//account for newline
	absidx+=bm->idx;
	return absidx;
}
void bookmark_set_absidx(Bookmark *bm, Text const *text, int absidx)
{
	for(bm->line=0;;++bm->line)
	{
		int delta=text_get_len(*text, bm->line)+1;
		if(absidx<delta)
			break;
		absidx-=delta;
	}
	bm->idx=absidx;
	bookmark_update_col(bm, *text);
}

void bookmark_jump_vertical(Bookmark *bm, Text const *text, int delta_line)
{
	bm->line+=delta_line;
	int nlines=text_get_nlines(*text);
	if(bm->line<0)
		bm->line=0, bm->idx=0, bm->col=0;
	else if(bm->line>nlines-1)
	{
		bm->line=nlines-1, bm->idx=text_get_len(*text, bm->line);
		bookmark_update_col(bm, *text);
	}
	else
		bookmark_update_idx(bm, *text);
}
void bookmark_increment_idx_skipnewline(Bookmark *bm, Text const *text)
{
	if(bm->idx<(int)text_get_len(*text, bm->line)-1)
		++bm->idx;
	else if(bm->line<(int)text_get_nlines(*text)-1)
		++bm->line, bm->idx=0;
}
void bookmark_decrement_idx_skipnewline(Bookmark *bm, Text const *text)
{
	if(bm->idx)
		--bm->idx;
	else if(bm->line>0)
		--bm->line, bm->idx=(int)text_get_len(*text, bm->line)-1;
}
int bookmark_increment_idx(Bookmark *bm, Text const *text)
{
	size_t len=text_get_len(*text, bm->line);
	if(bm->idx<(int)len)
	{
		++bm->idx;
		return 1;
	}
	else
	{
		size_t nlines=text_get_nlines(*text);
		if(bm->line<(int)nlines-1)
		{
			++bm->line, bm->idx=0;
			return 1;
		}
	}
	return 0;
}
int bookmark_decrement_idx(Bookmark *bm, Text const *text)
{
	if(bm->idx)
	{
		--bm->idx;
		return 1;
	}
	else if(bm->line>0)
	{
		--bm->line, bm->idx=(int)text_get_len(*text, bm->line);
		return 1;
	}
	return 0;
}
char bookmark_dereference_idx(Bookmark const *bm, Text const *text)
{
	if(bm->line>=0&&bm->line<(int)text_get_nlines(*text))
	{
		size_t len=0;
		auto _line=text_get_line(text, bm->line, &len);
		if(bm->idx==len)
			return '\n';
		if(bm->idx>=0&&bm->idx<(int)len)
			return _line[bm->idx];
	}
	return '\0';
}

#define BOOKMARK_EQ(BM1, BM2) ((BM1)->line==(BM2)->line&&(BM1)->idx==(BM2)->idx)
#define BOOMMARK_LT(BM1, BM2) ((BM1)->line<(BM2)->line||(BM1)->idx<(BM2)->idx)
//#define BOOMMARK_LT(BM1, BM2) ((BM1)->line==(BM2)->line&&(BM1)->idx<(BM2)->idx||(BM1)->line<(BM2)->line)

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
		switch(key)
		{
		case KEY_F1:
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
			break;
		}
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
