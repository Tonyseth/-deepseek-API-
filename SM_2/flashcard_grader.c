#include "flashcard_grader.h"
#include <math.h>

void grade_flashcard(Flashcard* card, unsigned int grade, unsigned int current_session) {
    if (card == NULL) return;

    if (grade >= 3) {
        switch (card->repetition) {
            case 0:
                card->interval = 1;
                card->repetition = 1;
                break;
            case 1:
                card->interval = 2;
                card->repetition = 2;
                break;
            default:
                unsigned int new_interval = (unsigned int)round(
                    (float)card->interval * card->easiness_factor
                );
                if (new_interval <= card->interval) {
                    new_interval = card->interval + 1;
                }
                card->interval = new_interval;
                card->repetition = card->repetition + 1;
                break;
        }
    } else {
        card->interval = 1;
        card->repetition = 0;
    }

    float new_easiness_factor = card->easiness_factor + 
        (0.1f - (5 - grade) * (0.08f + (5 - grade) * 0.02f));

    if (new_easiness_factor < 1.3f) {
        card->easiness_factor = 1.3f;
    } else if (new_easiness_factor > 2.5f) {
        card->easiness_factor = 2.5f;
    } else {
        card->easiness_factor = new_easiness_factor;
    }

    card->session_count = current_session;
    card->next_session = current_session + card->interval;
} 