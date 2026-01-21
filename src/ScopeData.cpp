#include "ScopeData.h"

namespace disyn
{

    float gScopeBuffer[kScopeSize] = {};
    volatile int gScopeIndex = 0;

} // namespace disyn
