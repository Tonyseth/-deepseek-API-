#include "flashcard.h"
#include <stdlib.h>

void flashcard_init(Flashcard* card) {
    if (card == NULL) return;
    
    card->repetition = DEFAULT_REPETITION;
    card->interval = DEFAULT_INTERVAL;
    card->easiness_factor = DEFAULT_EASINESS_FACTOR;
    card->session_count = 1;
    card->next_session = 1;
} 