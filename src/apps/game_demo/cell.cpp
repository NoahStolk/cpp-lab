#include "cell.h"

#include <algorithm>

void Cell::hit()
{
    health = std::max(health - 1, 0);
}
