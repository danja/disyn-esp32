#pragma once

namespace disyn
{

    constexpr int kScopeSize = 128;
    extern float gScopeBuffer[kScopeSize];
    extern volatile int gScopeIndex;

} // namespace disyn
