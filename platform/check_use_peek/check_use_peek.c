#include "app_cfg.h"
#include "queue.h"
#include "check_use_peek.h"
#include "../utilities/arm/app_type.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#define this (*ptThis)
#define TASK_RESET_FSM()  \
    do {                      \
        this.chState = START; \
    } while (0)

bool check_use_peek_init(check_words_t *ptThis, const check_words_cfg_t *ptCFG)
{
    enum {
        START
    };
    if ((NULL == ptThis) || (NULL == ptCFG)) {
        return false;
    }
    this.chState = START;
    this.chWordsNumber = ptCFG->chWordsNumber;
    this.pTarget = ptCFG->pTarget;
    this.fnCheckWords = ptCFG->fnCheckWords;
    return true;
}

fsm_rt_t task_check_use_peek(check_words_t *ptThis)
{
    enum {
        START,
        CHECK_WORDS,
        DROP,
        CHECK_WORDS_NUMBER
    };
    uint8_t s_chVoteDropCount;
    uint8_t s_chWordsCount;
    bool bIsRequestDrop;
    uint8_t chByteDrop;
    switch (this.chState) {
        case START:
            s_chVoteDropCount = 0;
            s_chWordsCount = 0;
            bIsRequestDrop = false;
            this.chState = CHECK_WORDS;
            // break;
        case CHECK_WORDS:
        GOTO_CHECK_WORDS:
            RESET_PEEK_BYTE(this.pTarget);
            if (fsm_rt_cpl == (*(&this.fnCheckWords+s_chWordsCount))(this.pTarget, &bIsRequestDrop)) {
                GET_ALL_PEEKED_BYTE(this.pTarget);
                TASK_RESET_FSM();
                return fsm_rt_cpl;
            }
            s_chWordsCount++;
            if (bIsRequestDrop) {
                s_chVoteDropCount++;
            }
            bIsRequestDrop = false;
            this.chState = DROP;
            //break;
        case DROP:
            if (s_chVoteDropCount >= this.chWordsNumber) {
                DEQUEUE_BYTE(this.pTarget, &chByteDrop);
                RESET_PEEK_BYTE(this.pTarget);
                s_chVoteDropCount = 0;
            }
            this.chState = CHECK_WORDS_NUMBER;
            //break;
        case CHECK_WORDS_NUMBER:
            if (s_chWordsCount >= this.chWordsNumber) {
                s_chWordsCount = 0;
                TASK_RESET_FSM();
                break;
            }
            goto GOTO_CHECK_WORDS;
        default:
            return fsm_rt_err;
            break;
    }
    return fsm_rt_on_going;
}