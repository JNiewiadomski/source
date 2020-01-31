/* state.h -- January 11, 1990 */

/* Shift State Library */

typedef enum { S_KEEP_STATE,
               S_ALARM,
               S_ALARM_HELP,
               S_ALT,
               S_BUMPLES,
               S_BYPASS,
               S_DELAY,
               S_HAND,
               S_HELP,
               S_HELP_CLEAR,
               S_MANUAL,
               S_NUMBER,
               S_OFF,
               S_PROC_VAR,
               S_SELECT,
	       S_TEST
               } S_STATE;

S_STATE ShiftStateGet(void);
S_STATE ShiftStatePop(void);
void    ShiftStatePush(S_STATE state);
void    ShiftStateReset(void);
void    ShiftStateSet(S_STATE state);
