#include "scorer.h"

#include <string.h>

void reset_scorer(Scorer* scorer) {
    memset(scorer, 0, sizeof(Scorer));
    return;
}
