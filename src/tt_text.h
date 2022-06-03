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
typedef enum ActionTypeEnum//must be in this order
{
	//ignored actions
	ACT_RELOCATE_CURSOR,
	
	ACT_TYPE, ACT_BEGIN_SOFT=ACT_TYPE,//soft actions: undone/redone in burst manner unless cursor is relocated in the middle or action type changes
	ACT_ERASE,
	
	ACT_INDENT, ACT_BEGIN_HARD=ACT_INDENT,//hard actions: undone one at a time
	ACT_MOVE_SELECTION,
	ACT_CUT,
	ACT_PASTE,
	ACT_CHANGE_CASE,

	ACT_COUNT,
} ActionType;
typedef union ActionInfoStruct
{
	struct
	{
#if defined _MSC_VER && defined _DEBUG
		ActionType action_type;
#else
		char action_type;
#endif
		char action_ctr;
	};
#if defined _MSC_VER && defined _DEBUG
	unsigned long long action_id;
#else
	unsigned short action_id;
#endif
} ActionInfo;

//constructors
void*		text_create(TextType type, const void *payload, size_t p_bytes);
void*		text_deepcopy(const void *hText);
void		text_destroy(void *hText);
void		text_clear(void **hText, TextType type, const void *payload, size_t p_bytes);

//basic getters
size_t		text_get_nlines(const void *hText);
size_t		text_get_len(const void *hText, size_t line);
char*		text_get_line(const void *hText, size_t line, size_t *ret_len);

//modifiers
void		text_insert_lines(void *hText, size_t l0, size_t nlines, ActionType action);
void		text_erase_lines(void *hText, size_t l0, size_t nlines, ActionType action);
void		text_replace(void *hText, size_t l0, size_t idx1, size_t idx2, const char *str, size_t len, size_t repeat, ActionType action);

void		text_action_end(void *hText);
ActionInfo	text_get_last_action(const void *hText);
void		text_push_checkpoint(void *hText, ActionType action, const void *payload, size_t payload_bytes);//store cursor info in payload
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