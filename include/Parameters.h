#pragma once

#include <cstdint>

namespace disyn
{

    struct Parameters
    {
        uint8_t algorithm = 0;
        float attack = 0.1f;
        float decay = 0.5f;
        float reverbSize = 0.1f;
        float reverbLevel = 0.1f;
        float param1 = 0.55f;
        float param2 = 0.5f;
        float pitchMin = 30.0f;
        float pitchMax = 4000.0f;
        float masterGain = 0.8f;
        float cv0 = 0.0f;
        float cv1 = 0.0f;
        float cv2 = 0.0f;
        float pot0 = 0.0f;
        float pot1 = 0.0f;
        float pot2 = 0.0f;
    };

    struct ParamMessage
    {
        Parameters params;
    };

    struct StatusMessage
    {
        uint32_t underruns = 0;
        bool audioOk = true;
    };

} // namespace disyn
