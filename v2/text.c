//tt_text.c - Teletext Editor text and history implementation
//Copyright (C) 2022 Ayman Wagih
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

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include"text.h"
#include"util.h"
#include<stdlib.h>
#include<string.h>
static const char file[]=__FILE__;

#define CHECK_PTR(PTR, RET) if(!PTR){LOG_ERROR("Alloc error"); return RET;}
#define ASSERT_IDX(IDX, LIMIT) ((IDX)<(LIMIT)||LOG_ERROR("%s = %d  >=  %s = %d", #IDX, IDX, #LIMIT, LIMIT))

#if 0
#if defined CHECK_USAGE || defined CHECK_ALLOC
#include<stdio.h>
#include<stdarg.h>

int messagebox_okcancel(const char *title, const char *format, ...);
int valid(const void *p)
{
	switch((size_t)p)
	{
	case 0:
	case 0xCCCCCCCC:
	case 0xFEEEFEEE:
	case 0xEEFEEEFE:
	case 0xCDCDCDCD:
	case 0xFDFDFDFD:
	case 0xBAAD0000:
		return 0;
	}
	return 1;
}
static char g_buf[1024];
static int error(const char *func, int line, const char *format, ...)
{
	va_list args;
	int ret;

	if(format)
	{
		va_start(args, format);
		vsprintf(g_buf, format, args);
		va_end(args);
	}
	ret=messagebox_okcancel("Error", "%s(%d):\n%s\nContinue?", func, line, g_buf);
	if(ret)//cancel
		exit(1);
	return 0;
}
#define ERROR(FORMAT, ...) error(__FUNCTION__, __LINE__, FORMAT, ##__VA_ARGS__)
#else
#define ERROR(...)
#endif
#ifdef CHECK_ALLOC
#define ASSERT_A(POINTER) (valid(POINTER)||error(__FUNCTION__, __LINE__, "malloc returned "#POINTER " = %p", POINTER))//NEVER ALLOCATE INSIDE THE MACRO
#define ASSERT_R(POINTER) (valid(POINTER)||error(__FUNCTION__, __LINE__, "realloc returned "#POINTER " = %p", POINTER))//NEVER ALLOCATE INSIDE THE MACRO
#else
#define ASSERT_A(...)
#define ASSERT_R(...)
#endif
#if defined CHECK_USAGE
#define ASSERT_P(POINTER) (valid(POINTER)||error(__FUNCTION__, __LINE__, #POINTER " = %p", POINTER))
#define ASSERT_IDX(IDX, LIMIT) ((IDX)<(LIMIT)||error(__FUNCTION__, __LINE__, #IDX " = %d  >=  " #LIMIT " = %d", IDX, LIMIT))
#else
#define ASSERT_P(...)
#define ASSERT_IDX(...)
#endif

static void memfill(void *dst, const void *src, size_t dstbytes, size_t srcbytes)
{
	unsigned copied;
	char *d=(char*)dst;
	const char *s=(const char*)src;
	if(dstbytes<srcbytes)
	{
		memcpy(dst, src, dstbytes);
		return;
	}
	copied=srcbytes;
	memcpy(d, s, copied);
	while(copied<<1<=dstbytes)
	{
		memcpy(d+copied, d, copied);
		copied<<=1;
	}
	if(copied<dstbytes)
		memcpy(d+copied, d, dstbytes-copied);
}
#endif

typedef enum ChangeTypeEnum
{
	CHANGE_ERROR,

	//data is usually the cursor info
	CHANGE_CHECKPOINT,

	//character manipulation
	//  line = the affected line
	//  idx = position
	//  len = number of characters inserted
	//  rep = repeat count
	//  data[] = payload
	CHANGE_INSERT,
	
	//  line = the affected line
	//  idx = position
	//  len = number of characters removed
	//  rep = 1
	//  data[] = payload
	CHANGE_REMOVE,
	
	//line manipulation
	//  line = the removed line
	//  idx = 0
	//  len = 0
	//  rep = COUNT OF INSERTED LINES
	//  data[] = INACCESSIBLE
	CHANGE_INSERT_LINES,

	//  line = the removed line
	//  idx = 0
	//  len = length of removed line
	//  rep = 1
	//  data[] = payload
	CHANGE_REMOVE_LINE,
} ChangeType;
typedef struct HistoryNodeStruct//32 bytes
{
	struct HistoryNodeStruct *prev, *next;//double linked list
	ActionInfo actinfo;
#ifdef _DEBUG
	ChangeType change_type;
#else
	short change_type;
#endif
	size_t line, idx, len, rep;
	char data[];//null terminated, when accessible
} HistoryNode;
typedef struct LineStruct
{
	size_t len;//doesn't include null terminator
	char data[];
} Line;
typedef struct TextStruct
{
	TextType type;
	char action_histogram[ACT_COUNT];
	HistoryNode
		*hist,
		*checkpoint;
	size_t nlines;
	Line **lines;
} Text;

static HistoryNode* hist_node_alloc(ChangeType change_type, char action_type, char action_ctr, size_t line, size_t idx, const void *str, size_t len, size_t rep)
{
	HistoryNode *p;
	
	p=(HistoryNode*)malloc(sizeof(HistoryNode)+(str?len+1:0));
	CHECK_PTR(p, 0)
	if(str)
	{
		memcpy(p->data, str, len);
		p->data[len]='\0';
	}
	p->prev=p->next=0;
	p->change_type=change_type;
	p->actinfo.action_id=0;
	p->actinfo.action_type=action_type;
	p->actinfo.action_ctr=action_ctr;
	p->line=line;
	p->idx=idx;
	p->len=len;
	p->rep=rep;
	return p;
}
static size_t hist_get_node_size(HistoryNode const *src)
{
	switch(src->change_type)
	{
	case CHANGE_ERROR:		return sizeof(HistoryNode);
	case CHANGE_CHECKPOINT:		return sizeof(HistoryNode)+src->len;
	case CHANGE_INSERT:		return sizeof(HistoryNode)+src->len;
	case CHANGE_REMOVE:		return sizeof(HistoryNode)+src->len;
	case CHANGE_INSERT_LINES:	return sizeof(HistoryNode);
	case CHANGE_REMOVE_LINE:	return sizeof(HistoryNode)+src->len;
	}
	return 0;
}
static HistoryNode* hist_node_copy(HistoryNode const *src, HistoryNode *prev, HistoryNode *next)
{
	HistoryNode *dst;
	size_t size;

	size=hist_get_node_size(src);
	dst=(HistoryNode*)malloc(size);
	CHECK_PTR(dst, 0)
	memcpy(dst, src, size);
	dst->prev=prev;
	dst->next=next;

	return dst;
}
static HistoryNode* hist_deepcopy(HistoryNode const *src, HistoryNode const *checkpoint, HistoryNode **ret_checkpoint)
{
	HistoryNode *dst, *p, *p2;
	HistoryNode const *i1, *f1;
	HistoryNode *i2, *f2;

	if(!src)
		return 0;

	dst=hist_node_copy(src, 0, 0);//copy present

	if(p=src->prev)//copy future
	{
		p2=dst;
		for(;p->prev;)
		{
			p=p->prev;
			p2->prev=hist_node_copy(p, 0, p2);
			p2=p2->prev;
		}
		i1=p;
		i2=p2->prev=hist_node_copy(p, 0, p2);
	}
	else
	{
		i1=src;
		i2=dst;
	}

	if(p=src->next)//copy past
	{
		p2=dst;
		for(;p->next;)
		{
			p=p->next;
			p2->next=hist_node_copy(p, p2, 0);
			p2=p2->next;
		}
		f1=p;
		f2=p2->next=hist_node_copy(p, p2, 0);
	}
	else
	{
		f1=src;
		f2=dst;
	}

	if(ret_checkpoint)//find checkpoint
	{
		*ret_checkpoint=0;
		for(;i1!=f1&&i2!=f2;i1=i1->next, i2=i2->next)
		{
			if(i1==checkpoint)
			{
				*ret_checkpoint=i2;
				break;
			}
		}
	}
	return dst;
}
static size_t hist_erase_past(HistoryNode **hist)
{
	size_t count;
	HistoryNode *p;

	count=0;
	if(*hist)
	{
		if(p=hist[0]->prev)
		{
			for(;p->prev;)
			{
				p=p->prev;
				free(p->next), ++count;
			}
			free(p), ++count;
			hist[0]->prev=0;
		}
	}
	return count;
}
static size_t hist_erase_future(HistoryNode **hist)
{
	size_t count;
	HistoryNode *p;
	
	count=0;
	if(*hist)
	{
		if(p=hist[0]->next)
		{
			for(;p->next;)
			{
				p=p->next;
				free(p->prev), ++count;
			}
			free(p), ++count;
			hist[0]->next=0;
		}
	}
	return count;
}
static size_t hist_erase_all(HistoryNode **hist)
{
	size_t count;

	count=hist_erase_past(hist)+hist_erase_future(hist);
	if(hist)
	{
		free(*hist), ++count;
		*hist=0;
	}
	return count;
}
static size_t hist_insert(HistoryNode **hist, ChangeType change_type, char action_type, char action_ctr, int line, int idx, const void *str, int len, int rep)//returns the signed number of nodes inserted
{
	HistoryNode *p;
	size_t count;

	p=hist_node_alloc(change_type, action_type, action_ctr, line, idx, str, len, rep);

	count=1-hist_erase_future(hist);
	if(*hist)
	{
		if(hist[0]->change_type==CHANGE_CHECKPOINT
			&&hist[0]->prev
			&&hist[0]->prev->actinfo.action_type==action_type
			&&hist[0]->prev->actinfo.action_ctr==action_ctr)//if (current is checkpoint and prev has same action_id) insert between prev and current nodes
		{
			//[0] prev=null, next=[1]
			//[1] prev=[0], next=[2]	<-hist[0]->prev
			//[2] prev=[1], next=null	<-hist[0]
			//
			//insert [p] between [1] and [2]
			//
			//[0] prev=null, next=[1]
			//[1] prev=[0], next=[p]
			//[p] prev=[1], next=[2]
			//[2] prev=[p], next=null	<-hist[0]
			p->prev=hist[0]->prev;
			p->next=hist[0];
			hist[0]->prev->next=p;
			hist[0]->prev=p;
		}
		//else if()//if (current is checkpoint and change) type is checkpoint, replace current node
		//{
		//}
		else
		{
			p->prev=*hist;
			hist[0]->next=p;
			*hist=hist[0]->next;
		}
	}
	else
		*hist=p;
	return count;
}

static void swap(size_t *a, size_t *b)
{
	int temp=*a;
	*a=*b, *b=temp;
}
static Line** lines_deepcopy(Line const **src, size_t nlines)
{
	size_t kl, buflen;
	Line **dst=(Line**)malloc(nlines*sizeof(void*));
	CHECK_PTR(dst, 0)

	for(kl=0;kl<nlines;++kl)
	{
		buflen=sizeof(Line)+src[kl]->len+1;
		dst[kl]=(Line*)malloc(buflen);
		CHECK_PTR(dst[kl], 0)
		memcpy(dst[kl], src[kl], buflen);
	}
	return dst;
}
static void lines_add(Text *text, size_t l1, size_t l2)//not history-tracked
{
	void *p;
	size_t kl;
	ASSERT_P(text->lines);
	ASSERT_IDX(l1, l2+1);
	ASSERT_IDX(l1, text->nlines+1);
	
	if(l1<l2)
	{
		text->nlines+=l2-l1;
		p=realloc(text->lines, text->nlines*sizeof(void*));
		CHECK_PTR(p, 0)
		text->lines=(Line**)p;

		if(text->nlines)
			ASSERT_R(text->lines);
		memmove(text->lines+l2, text->lines+l1, (text->nlines-l2)*sizeof(void*));
		for(kl=l1;kl<l2;++kl)
		{
			text->lines[kl]=(Line*)malloc(sizeof(Line)+1);
			CHECK_PTR(text->lines[kl], 0)
			text->lines[kl]->len=0;
			text->lines[kl]->data[0]='\0';
		}
	}

	ASSERT_IDX(l2, text->nlines+1);
}
static void lines_erase(Text *text, size_t l1, size_t l2)//not history-tracked
{
	void *p;
	size_t kl;
	ASSERT_P(text->lines);
	ASSERT_IDX(l1, l2+1);
	ASSERT_IDX(l1, text->nlines+1);
	ASSERT_IDX(l2, text->nlines+1);

	if(l1<l2)
	{
		for(kl=l1;kl<l2;++kl)
			free(text->lines[kl]);
		if(l2<text->nlines)
			memmove(text->lines+l1, text->lines+l2, (text->nlines-l2)*sizeof(void*));
		text->nlines-=l2-l1;
		p=realloc(text->lines, text->nlines*sizeof(void*));
		CHECK_PTR(p, 0)
		text->lines=(Line**)p;
		if(text->nlines)
			ASSERT_R(text->lines);
	}
}
static void line_insert(Line **line, size_t idx, const char *str, size_t len, size_t rep)//not history-tracked
{
	void *p;
	int inslen;
	ASSERT_P(*line);
	ASSERT_IDX(idx, line[0]->len+1);
	
	inslen=len*rep;
	if(inslen)
	{
		ASSERT_P(str);

		line[0]->len+=inslen;
		p=realloc(*line, sizeof(Line)+line[0]->len+1);
		CHECK_PTR(p, 0)
		*line=(Line*)p;
		ASSERT_R(*line);
		memmove(line[0]->data+idx+inslen, line[0]->data+idx, line[0]->len+1-inslen-idx);
		memfill(line[0]->data+idx, str, inslen, len);
	}
}
static void line_erase(Line **line, size_t idx1, size_t idx2)//not history-tracked
{
	void *p;
	ASSERT_P(*line);
	ASSERT_IDX(idx1, line[0]->len);
	ASSERT_IDX(idx2, line[0]->len+1);

	if(idx1<idx2)
	{
		memmove(line[0]->data+idx1, line[0]->data+idx2, line[0]->len+1-idx2);
		line[0]->len-=idx2-idx1;
		if(line[0]->len<0)
			line[0]->len=0;
		p=realloc(*line, sizeof(Line)+line[0]->len+1);
		CHECK_PTR(p, 0)
		*line=(Line*)p;
		ASSERT_R(*line);
	}
}

static int related(HistoryNode *node, HistoryNode *next)//O(n)
{
	if(!node||!next)
		return 0;
	while(node->prev&&node->actinfo.action_type==ACT_RELOCATE_CURSOR)
		node=node->prev;
	if(node->actinfo.action_type==ACT_RELOCATE_CURSOR)
		return 1;
	while(next->next&&next->actinfo.action_type==ACT_RELOCATE_CURSOR)
		next=next->next;
	if(next->actinfo.action_type==ACT_RELOCATE_CURSOR)
		return 1;
	return node->actinfo.action_id==next->actinfo.action_id;
}
void text_init(void *hText, TextType type, const void *payload, size_t p_bytes)
{
	Text *text=(Text*)hText;
	Line **line;
	ASSERT_P(text);

	text->type=type;

	memset(text->action_histogram, 0, ACT_COUNT);
	if(type==TEXT_NO_HISTORY)
		text->hist=0;
	else
	{
		text->hist=hist_node_alloc(CHANGE_CHECKPOINT, 0, 0, 0, 0, payload, p_bytes, 1);
		ASSERT_P(text->hist);
	}
	text->checkpoint=text->hist;

	text->nlines=1;
	text->lines=(Line**)malloc(text->nlines*sizeof(void*));
	CHECK_PTR(text->lines, 0)
	if(!text->lines)
	{
		if(text->hist)
			free(text->hist);
		memset(text, 0, sizeof(Text));
		return;
	}
	ASSERT_P(text->lines);

	line=text->lines;
	*line=(Line*)malloc(sizeof(Line)+1);
	CHECK_PTR(line, 0)
	line[0]->len=0;
	line[0]->data[0]='\0';
}


//API
size_t text_get_nlines(const void *hText)
{
	Text const *text=(Text const*)hText;
	ASSERT_P(text);
	ASSERT_P(text->lines);

	return text->nlines;
}
size_t text_get_len(const void *hText, size_t line)
{
	Text const *text=(Text const*)hText;
	ASSERT_P(text);
	ASSERT_P(text->lines);
	ASSERT_IDX(line, text->nlines);

	return text->lines[line]->len;
}
char* text_get_line(const void *hText, size_t line, size_t *ret_len)
{
	Text const *text=(Text const*)hText;
	ASSERT_P(text);
	ASSERT_P(text->lines);
	ASSERT_IDX(line, text->nlines);

	if(ret_len)
		*ret_len=text->lines[line]->len;
	return text->lines[line]->data;
}
void text_insert_lines(void *hText, size_t l0, size_t nlines, ActionType action)
{
	Text *text=(Text*)hText;
	ASSERT_P(text);
	ASSERT_P(text->lines);
	ASSERT_IDX(l0, text->nlines+1);

	if(text->type!=TEXT_NO_HISTORY)
		hist_insert(&text->hist, CHANGE_INSERT_LINES, action, text->action_histogram[action], l0, 0, 0, 0, nlines);
	lines_add(text, l0, l0+nlines);
}
void text_erase_lines(void *hText, size_t l0, size_t nlines, ActionType action)
{
	Text *text=(Text*)hText;
	size_t kl;
	ASSERT_P(text);

	if(text->type!=TEXT_NO_HISTORY)
	{
		ASSERT_P(text->hist);

		for(kl=l0+nlines-1;kl>=l0;--kl)
			hist_insert(&text->hist, CHANGE_REMOVE_LINE, action, text->action_histogram[action], kl, 0, text->lines[kl]->data, text->lines[kl]->len, 1);
	}
	lines_erase(text, l0, l0+nlines);
}
void* text_create(TextType type, const void *payload, size_t p_bytes)
{
	Text *text=(Text*)malloc(sizeof(Text));
	ASSERT_A(text);

	text_init(text, type, payload, p_bytes);
	return text;
}
void* text_deepcopy(const void *hText)
{
	Text const *src=(Text const*)hText;
	Text *dst;
	ASSERT_P(src);

	dst=(Text*)malloc(sizeof(Text));
	CHECK_PTR(dst, 0)
	memcpy(dst, src, sizeof(Text));

	if(src->type==TEXT_NORMAL)
	{
		ASSERT_P(src->hist);
		dst->hist=hist_deepcopy(src->hist, src->checkpoint, &dst->checkpoint);
	}
	else
		dst->hist=dst->checkpoint=0;

	dst->lines=lines_deepcopy((Line const**)src->lines, src->nlines);
	return dst;
}
void text_destroy(void *hText)
{
	Text *text=(Text*)hText;
	//if(text)
	//{
		ASSERT_P(text);

		if(text->type==TEXT_NORMAL)
		{
			ASSERT_P(text->hist);
			hist_erase_all(&text->hist);
		}
		lines_erase(text, 0, text->nlines);
		free(text);
	//}
}
void text_clear(void **hText, TextType type, const void *payload, size_t p_bytes)
{
	Text **text=(Text**)hText;
	if(*text)
	{
		hist_erase_all(&text[0]->hist);
		lines_erase(*text, 0, text[0]->nlines);
	}
	else
	{
		*text=(Text*)malloc(sizeof(Text));
		CHECK_PTR(text, 0)
	}
	text_init(*text, type, payload, p_bytes);
}
void text_replace(void *hText, size_t l0, size_t idx1, size_t idx2, const char *str, size_t len, size_t repeat, ActionType action_type)
{
	void *p;
	Line **line;
	Text *text=(Text*)hText;
	size_t inssize=len*repeat;
	char *act_number;
	ASSERT_P(text);
	ASSERT_IDX(l0, text->nlines);
	ASSERT_IDX(idx1, text->lines[l0]->len+1);
	ASSERT_IDX(idx2, text->lines[l0]->len+1);

	act_number=text->action_histogram+action_type;
	if(idx2<idx1)
		swap(&idx1, &idx2);
	line=text->lines+l0;
	if(idx1<idx2)
	{
		if(text->type!=TEXT_NO_HISTORY)
			hist_insert(&text->hist, CHANGE_REMOVE, action_type, *act_number, l0, idx1, line[0]->data+idx1, idx2-idx1, 1);
		memmove(line[0]->data+idx1, line[0]->data+idx2, line[0]->len+1-idx2);
	}
	line[0]->len+=inssize-(idx2-idx1);
	p=realloc(line[0], sizeof(Line)+line[0]->len+1);
	CHECK_PTR(p, 0)
	line[0]=(Line*)p;
	ASSERT_R(line[0]);
	if(inssize)
	{
		memmove(line[0]->data+idx1+inssize, line[0]->data+idx1, (line[0]->len+1-inssize)-idx1);//make way for array insertion
		memfill(line[0]->data+idx1, str, inssize, len);
		if(text->type!=TEXT_NO_HISTORY)
			hist_insert(&text->hist, CHANGE_INSERT, action_type, *act_number, l0, idx1, line[0]->data+idx1, len, repeat);
	}
	//++*act_number;
}
void text_action_end(void *hText)
{
	Text *text=(Text*)hText;
	char acttype;
	ASSERT_P(text);
	if(text->type==TEXT_NO_HISTORY)
		return;
	ASSERT_P(text->hist);

	acttype=text->hist->actinfo.action_type;
	++text->action_histogram[acttype];
}
#if 1
ActionInfo text_get_last_action(const void *hText)
{
	ActionInfo action_error={-1, -1};
	Text const *text=(Text const*)hText;
	ASSERT_P(text);
	if(text->type==TEXT_NO_HISTORY)
		return action_error;
	ASSERT_P(text->hist);

	if(!text->hist->prev)
		return action_error;
	return text->hist->prev->actinfo;
}
void text_push_checkpoint(void *hText, ActionType action_type, const void *payload, size_t p_bytes)
{
	void *p;
	HistoryNode *p0;
	char *act_number;
	Text *text=(Text*)hText;
	ASSERT_P(text);
	if(text->type==TEXT_NO_HISTORY)
		return;
	ASSERT_P(text->hist);
	
	act_number=text->action_histogram+action_type;
	if(text->hist->change_type!=CHANGE_CHECKPOINT)//TODO: update this before build
		hist_insert(&text->hist, CHANGE_CHECKPOINT, action_type, *act_number, 0, 0, payload, p_bytes, 1);
	else if(p_bytes)
	{
		if(text->hist->len!=p_bytes)
		{
			text->hist->len=p_bytes;
			p=realloc(text->hist, hist_get_node_size(text->hist));
			CHECK_PTR(p, 0)
			p0=(HistoryNode*)p;
			if(text->checkpoint==text->hist)
				text->checkpoint=p0;
			text->hist=p0;
		}
		memcpy(text->hist->data, payload, p_bytes);
	}
}
void text_pop_checkpoint(void *hText)
{
	Text *text=(Text*)hText;
	ASSERT_P(text);
	if(text->type==TEXT_NO_HISTORY)
		return;
	ASSERT_P(text->hist);
	if(text->hist->change_type!=CHANGE_CHECKPOINT)//TODO: update this before build
		ERROR("History pointer is not on ACTION_CHECKPOINT. hist->change_type = %d", text->hist->change_type);

	if(text->hist->prev)
		text->hist=text->hist->prev;
}
#endif
int text_undo(void *hText, void *payload, size_t *p_bytes)
{
	Text *text=(Text*)hText;
	int modified=0;
	HistoryNode **ph, *hist;
	ActionInfo current_action;
	ASSERT_P(text);
	if(text->type==TEXT_NO_HISTORY)
		return 0;
	ASSERT_P(text->hist);

	ph=&text->hist;
	if(ph[0]->prev)
	{
		if(ph[0]->actinfo.action_type==ACT_RELOCATE_CURSOR)
			*ph=ph[0]->prev;
		current_action=ph[0]->actinfo;
		for(;;*ph=ph[0]->prev, ++modified)
	//	for(;ph[0]->prev&&(ph[0]->actinfo.action_type<ACT_BEGIN_SOFT||ph[0]->change_type!=CHANGE_CHECKPOINT||ph[0]->actinfo.action_id==current_action.action_id);*ph=ph[0]->prev, ++modified)
		{
			hist=*ph;
			switch(hist->change_type)
			{
			case CHANGE_INSERT:
				line_erase(text->lines+hist->line, hist->idx, hist->idx+hist->len*hist->rep);
				break;
			case CHANGE_REMOVE:
				line_insert(text->lines+hist->line, hist->idx, hist->data, hist->len, hist->rep);
				break;
			case CHANGE_INSERT_LINES:
				lines_erase(text, hist->line, hist->line+hist->rep);
				break;
			case CHANGE_REMOVE_LINE:
				lines_add(text, hist->line, hist->line+1);
				line_insert(text->lines+hist->line, hist->idx, hist->data, hist->len, hist->rep);
				break;
			}
			if(!related(ph[0]->prev, ph[0]))
				break;
			//if(ph[0]->actinfo.action_type>ACT_BEGIN_HARD)//hard actions are undone/redone one at a time
			//	break;
		}
		if(ph[0]->prev
			&&ph[0]->actinfo.action_type!=ACT_RELOCATE_CURSOR
			&&ph[0]->prev->actinfo.action_type==ACT_RELOCATE_CURSOR)
			ph[0]=ph[0]->prev;
		if(payload)
		{
			memcpy(payload, ph[0]->data, ph[0]->len);
			if(p_bytes)
				*p_bytes=ph[0]->len;
		}
		--text->action_histogram[current_action.action_type];
	}
	return modified;
}
int text_redo(void *hText, void *payload, size_t *p_bytes)
{
	Text *text=(Text*)hText;
	int modified=0;
	HistoryNode **ph, *hist;
	ActionInfo current_action;
	ASSERT_P(text);
	if(text->type==TEXT_NO_HISTORY)
		return 0;
	ASSERT_P(text->hist);

	ph=&text->hist;
	if(ph[0]->next)
	{
		*ph=ph[0]->next;
		current_action=ph[0]->actinfo;
		for(;;*ph=ph[0]->next, ++modified)
	//	for(;ph[0]->actinfo.action_type<ACT_BEGIN_SOFT||ph[0]->actinfo.action_id==current_action.action_id;*ph=ph[0]->next, ++modified)
		{
			hist=*ph;
			switch(hist->change_type)
			{
			case CHANGE_INSERT:
				line_insert(text->lines+hist->line, hist->idx, hist->data, hist->len, hist->rep);
				break;
			case CHANGE_REMOVE:
				line_erase(text->lines+hist->line, hist->idx, hist->idx+hist->len*hist->rep);
				break;
			case CHANGE_INSERT_LINES:
				lines_add(text, hist->line, hist->line+hist->rep);
				break;
			case CHANGE_REMOVE_LINE:
				lines_erase(text, hist->line, hist->line+1);
				break;
			}
			if(!related(ph[0], ph[0]->next))
				break;
			//if(!ph[0]->next||ph[0]->actinfo.action_type>ACT_BEGIN_HARD)//hard actions are undone/redone one at a time
			//	break;
		}
		if(ph[0]->next
			&&ph[0]->actinfo.action_type!=ACT_RELOCATE_CURSOR
			&&ph[0]->next->actinfo.action_type==ACT_RELOCATE_CURSOR)
			ph[0]=ph[0]->next;
		if(payload)
		{
			memcpy(payload, ph[0]->data, ph[0]->len);
			if(p_bytes)
				*p_bytes=ph[0]->len;
		}
		++text->action_histogram[current_action.action_type];
	}
	return modified;
}
void text_mark_saved(void *hText)
{
	Text *text=(Text*)hText;
	ASSERT_P(text);

	text->checkpoint=text->hist;
}
int text_is_modified(void *hText)
{
	Text *text=(Text*)hText;
	ASSERT_P(text);

	return text->checkpoint!=text->hist;
}
void text_clear_history(void *hText)
{
	Text *text=(Text*)hText;
	ASSERT_P(text);
	if(text->type==TEXT_NO_HISTORY)
		return;
	ASSERT_P(text->hist);

	hist_erase_all(&text->hist);
	text->hist=hist_node_alloc(CHANGE_CHECKPOINT, 0, 0, 0, 0, 0, 0, 1);
}