#ifndef TT_TEXT_H
#define TT_TEXT_H
#ifdef __cplusplus
extern "C"
{
#endif
	
	#define	CHECK_USAGE//can be disabled if passes all tests
	#define	CHECK_ALLOC//must be on, no matter what

typedef enum	TextTypeEnum
{
	TEXT_NORMAL,
	//TEXT_SINGLE_LINE,//useless
	TEXT_NO_HISTORY,
} TextType;
typedef enum	ActionTypeEnum
{
	ACTION_ERROR,

	ACTION_CHECKPOINT,

	//character manipulation
	//  line = the affected line
	//  idx = position
	//  len = number of characters inserted
	//  rep = repeat count
	//  data[] = payload
	ACTION_INSERT,
	
	//  line = the affected line
	//  idx = position
	//  len = number of characters removed
	//  rep = 1
	//  data[] = payload
	ACTION_REMOVE,
	
	//line manipulation
	//  line = the removed line
	//  idx = 0
	//  len = 0
	//  rep = COUNT OF INSERTED LINES
	//  data[] = INACCESSIBLE
	ACTION_INSERT_LINES,

	//  line = the removed line
	//  idx = 0
	//  len = length of removed line
	//  rep = 1
	//  data[] = payload
	ACTION_REMOVE_LINE,
} ActionType;
void*		text_create(TextType type, const void *payload, size_t p_bytes);
void*		text_deepcopy(const void *hText);
void		text_destroy(void *hText);
void		text_clear(void **hText, TextType type, const void *payload, size_t p_bytes);

size_t		text_get_nlines(const void *hText);
size_t		text_get_len(const void *hText, size_t line);
char*		text_get_line(const void *hText, size_t line, size_t *ret_len);

void		text_insert_lines(void *hText, size_t l0, size_t nlines);
void		text_erase_lines(void *hText, size_t l0, size_t nlines);
void		text_replace(void *hText, size_t l0, size_t idx1, size_t idx2, const char *str, size_t len, size_t repeat);

ActionType	text_get_last_action(const void *hText);
void		text_push_checkpoint(void *hText, const void *payload, size_t payload_bytes);//store cursor info in payload
void		text_pop_checkpoint(void *hText);

int			text_undo(void *hText, void *payload, size_t *payload_bytes);//returns number of steps appplied, retrieve cursor info from payload
int			text_redo(void *hText, void *payload, size_t *payload_bytes);
void		text_mark_saved(void *hText);
int			text_is_modified(void *hText);
void		text_clear_history(void *hText);


#ifdef __cplusplus
}
#endif
#endif