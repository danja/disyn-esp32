#include "ui/UiTask.h"

#include <array>
#include <cstdio>
#include <cstring>
#include <cmath>

#include "AlgorithmInfo.h"
#include "Calibration.h"
#include "IntercoreQueue.h"
#include "Parameters.h"
#include "PinConfig.h"
#include "hal/Adc.h"
#include "hal/Display.h"
#include "hal/Encoder.h"

namespace disyn::ui {

struct MenuItem
{
    const char *label;
};

constexpr std::array<MenuItem, 9> kMenuItems = {{
    {"Alg"},
    {"Atk"},
    {"Dec"},
    {"Rev Sz"},
    {"Rev Lv"},
    {"P1"},
    {"P2"},
    {"Mast"},
    {"Stat"},
}};

static disyn::hal::Display display;
static disyn::hal::Encoder encoder;
static disyn::hal::Adc adc;
static disyn::Parameters params;

static int currentIndex = 0;
static int topIndex = 0;
static int32_t lastPosition = 0;
static disyn::StatusMessage status;

static float smoothCv0 = 0.0f;
static float smoothCv1 = 0.0f;
static float smoothCv2 = 0.0f;
static float smoothPot0 = 0.0f;
static float smoothPot1 = 0.0f;
static float smoothPot2 = 0.0f;
static float lastCv0 = 0.0f;
static float lastCv1 = 0.0f;
static float lastCv2 = 0.0f;
static float lastPot0 = 0.0f;
static float lastPot1 = 0.0f;
static float lastPot2 = 0.0f;
static bool cv0Changed = false;
static bool cv1Changed = false;
static bool cv2Changed = false;
static bool pot0Changed = false;
static bool pot1Changed = false;
static bool pot2Changed = false;
static uint32_t lastReportMs = 0;
static uint32_t pressStartMs = 0;
static bool longPressHandled = false;
static uint32_t resetNoticeUntilMs = 0;
static uint16_t rawInputs[6] = {};
static uint16_t minInputs[6] = {4095, 4095, 4095, 4095, 4095, 4095};
static uint16_t maxInputs[6] = {0, 0, 0, 0, 0, 0};

static const char *kInputLabels[6] = {"C0", "C1", "C2", "P0", "P1", "P2"};

static float smoothValue(float current, float target, float alpha)
{
    return current + alpha * (target - current);
}

static void updateMinMax(int index, uint16_t value)
{
    rawInputs[index] = value;
    if (value < minInputs[index])
    {
        minInputs[index] = value;
    }
    if (value > maxInputs[index])
    {
        maxInputs[index] = value;
    }
}

static float clamp(float value, float minValue, float maxValue)
{
    if (value < minValue)
    {
        return minValue;
    }
    if (value > maxValue)
    {
        return maxValue;
    }
    return value;
}

static bool isUnusedParam(const char *label)
{
    return label != nullptr && std::strcmp(label, "Unused") == 0;
}

static void adjustCurrentValue(int delta)
{
    const auto &info = disyn::GetAlgorithmInfo(params.algorithm);
    switch (currentIndex)
    {
    case 0:
        {
            int algorithm = static_cast<int>(params.algorithm) + delta;
            int maxAlgorithm = static_cast<int>(disyn::kAlgorithmCount) - 1;
            if (algorithm < 0)
            {
                algorithm = maxAlgorithm;
            }
            if (algorithm > maxAlgorithm)
            {
                algorithm = 0;
            }
            params.algorithm = static_cast<uint8_t>(algorithm);
        }
        break;
    case 1:
        params.attack = clamp(params.attack + delta * 0.01f, 0.0f, 1.0f);
        break;
    case 2:
        params.decay = clamp(params.decay + delta * 0.01f, 0.0f, 1.0f);
        break;
    case 3:
        params.reverbSize = clamp(params.reverbSize + delta * 0.01f, 0.0f, 1.0f);
        break;
    case 4:
        params.reverbLevel = clamp(params.reverbLevel + delta * 0.01f, 0.0f, 1.0f);
        break;
    case 5:
        if (!isUnusedParam(info.param1.label))
        {
            params.param1 = clamp(params.param1 + delta * 0.01f, 0.0f, 1.0f);
        }
        break;
    case 6:
        if (!isUnusedParam(info.param2.label))
        {
            params.param2 = clamp(params.param2 + delta * 0.01f, 0.0f, 1.0f);
        }
        break;
    case 7:
        params.masterGain = clamp(params.masterGain + delta * 0.01f, 0.0f, 1.0f);
        break;
    case 8:
        break;
    default:
        break;
    }
}

static void formatValue(int index, char *buffer, size_t bufferSize)
{
    const auto &info = disyn::GetAlgorithmInfo(params.algorithm);
    switch (index)
    {
    case 0:
        snprintf(buffer, bufferSize, "%s", info.name);
        break;
    case 1:
        dtostrf(params.attack, 0, 2, buffer);
        break;
    case 2:
        dtostrf(params.decay, 0, 2, buffer);
        break;
    case 3:
        dtostrf(params.reverbSize, 0, 2, buffer);
        break;
    case 4:
        dtostrf(params.reverbLevel, 0, 2, buffer);
        break;
    case 5:
        if (isUnusedParam(info.param1.label))
        {
            snprintf(buffer, bufferSize, "-");
        }
        else
        {
            float value = disyn::MapNormalized(info.param1, params.param1);
            if (info.param1.integer)
            {
                snprintf(buffer, bufferSize, "%d", static_cast<int>(value + 0.5f));
            }
            else
            {
                dtostrf(value, 0, 2, buffer);
            }
        }
        break;
    case 6:
        if (isUnusedParam(info.param2.label))
        {
            snprintf(buffer, bufferSize, "-");
        }
        else
        {
            float value = disyn::MapNormalized(info.param2, params.param2);
            if (info.param2.integer)
            {
                snprintf(buffer, bufferSize, "%d", static_cast<int>(value + 0.5f));
            }
            else
            {
                dtostrf(value, 0, 2, buffer);
            }
        }
        break;
    case 7:
        dtostrf(params.masterGain, 0, 2, buffer);
        break;
    case 8:
        snprintf(buffer, bufferSize, "-");
        break;
    default:
        snprintf(buffer, bufferSize, "-");
        break;
    }
}

static void Init()
{
    display.begin();
    encoder.begin(kPinEncClk, kPinEncDt, kPinEncSw);
    adc.begin();
}

static void Tick()
{
    encoder.update();

    uint16_t rawCv0 = adc.readCv0();
    uint16_t rawCv1 = adc.readCv1();
    uint16_t rawCv2 = adc.readCv2();
    uint16_t rawPot0 = adc.readPot0();
    uint16_t rawPot1 = adc.readPot1();
    uint16_t rawPot2 = adc.readPot2();

    updateMinMax(0, rawCv0);
    updateMinMax(1, rawCv1);
    updateMinMax(2, rawCv2);
    updateMinMax(3, rawPot0);
    updateMinMax(4, rawPot1);
    updateMinMax(5, rawPot2);

    float cv0 = disyn::NormalizeAdc(rawCv0, disyn::kCv0Cal);
    float cv1 = disyn::NormalizeAdc(rawCv1, disyn::kCv1Cal);
    float cv2 = disyn::NormalizeAdc(rawCv2, disyn::kCv2Cal);
    float pot0 = disyn::NormalizeAdc(rawPot0, disyn::kPot0Cal);
    float pot1 = disyn::NormalizeAdc(rawPot1, disyn::kPot1Cal);
    float pot2 = disyn::NormalizeAdc(rawPot2, disyn::kPot2Cal);

    constexpr float kAlpha = 0.1f;
    smoothCv0 = smoothValue(smoothCv0, cv0, kAlpha);
    smoothCv1 = smoothValue(smoothCv1, cv1, kAlpha);
    smoothCv2 = smoothValue(smoothCv2, cv2, kAlpha);
    smoothPot0 = smoothValue(smoothPot0, pot0, kAlpha);
    smoothPot1 = smoothValue(smoothPot1, pot1, kAlpha);
    smoothPot2 = smoothValue(smoothPot2, pot2, kAlpha);

    params.cv0 = smoothCv0;
    params.cv1 = smoothCv1;
    params.cv2 = smoothCv2;
    params.pot0 = smoothPot0;
    params.pot1 = smoothPot1;
    params.pot2 = smoothPot2;

    constexpr float kChangeThreshold = 0.02f;
    cv0Changed = std::fabs(params.cv0 - lastCv0) > kChangeThreshold;
    cv1Changed = std::fabs(params.cv1 - lastCv1) > kChangeThreshold;
    cv2Changed = std::fabs(params.cv2 - lastCv2) > kChangeThreshold;
    pot0Changed = std::fabs(params.pot0 - lastPot0) > kChangeThreshold;
    pot1Changed = std::fabs(params.pot1 - lastPot1) > kChangeThreshold;
    pot2Changed = std::fabs(params.pot2 - lastPot2) > kChangeThreshold;

    uint32_t nowMs = millis();
    if ((cv0Changed || cv1Changed || cv2Changed || pot0Changed || pot1Changed || pot2Changed) &&
        (nowMs - lastReportMs > 200))
    {
        lastReportMs = nowMs;
        Serial.print("Inputs ");
        if (cv0Changed)
        {
            Serial.print("CV0=");
            Serial.print(params.cv0, 3);
            Serial.print(" ");
        }
        if (cv1Changed)
        {
            Serial.print("CV1=");
            Serial.print(params.cv1, 3);
            Serial.print(" ");
        }
        if (cv2Changed)
        {
            Serial.print("CV2=");
            Serial.print(params.cv2, 3);
            Serial.print(" ");
        }
        if (pot0Changed)
        {
            Serial.print("P0=");
            Serial.print(params.pot0, 3);
            Serial.print(" ");
        }
        if (pot1Changed)
        {
            Serial.print("P1=");
            Serial.print(params.pot1, 3);
            Serial.print(" ");
        }
        if (pot2Changed)
        {
            Serial.print("P2=");
            Serial.print(params.pot2, 3);
            Serial.print(" ");
        }
        Serial.println();
    }

    lastCv0 = params.cv0;
    lastCv1 = params.cv1;
    lastCv2 = params.cv2;
    lastPot0 = params.pot0;
    lastPot1 = params.pot1;
    lastPot2 = params.pot2;
    int32_t position = encoder.position();
    if (position != lastPosition)
    {
        int delta = (position > lastPosition) ? 1 : -1;
        adjustCurrentValue(delta);
        lastPosition = position;
    }

    bool pressed = encoder.pressed();
    bool down = encoder.isDown();
    if (down)
    {
        if (pressStartMs == 0)
        {
            pressStartMs = millis();
            longPressHandled = false;
        }
    }
    else
    {
        pressStartMs = 0;
        longPressHandled = false;
    }

    bool suppressClick = false;
    if (currentIndex == 8 && down && !longPressHandled && pressStartMs != 0)
    {
        if (millis() - pressStartMs > 1000)
        {
            for (int i = 0; i < 6; ++i)
            {
                minInputs[i] = rawInputs[i];
                maxInputs[i] = rawInputs[i];
            }
            resetNoticeUntilMs = millis() + 1000;
            Serial.println("CAL RESET");
            longPressHandled = true;
            suppressClick = true;
        }
    }

    if (pressed && !suppressClick)
    {
        currentIndex = (currentIndex + 1) % static_cast<int>(kMenuItems.size());
        if (currentIndex < topIndex)
        {
            topIndex = currentIndex;
        }
        if (currentIndex >= topIndex + 4)
        {
            topIndex = currentIndex - 3;
        }
    }

    disyn::ParamMessage message{params};
    if (disyn::gParamQueue != nullptr)
    {
        xQueueOverwrite(disyn::gParamQueue, &message);
    }

    if (disyn::gStatusQueue != nullptr)
    {
        disyn::StatusMessage nextStatus{};
        if (xQueueReceive(disyn::gStatusQueue, &nextStatus, 0) == pdTRUE)
        {
            status = nextStatus;
        }
    }

    display.clear();
    display.setCursor(0, 0);

    if (currentIndex == 8)
    {
        char line[22] = {0};

        bool isTest = params.algorithm == disyn::kTestAlgorithmIndex;
        if (resetNoticeUntilMs != 0 && millis() < resetNoticeUntilMs)
        {
            snprintf(line, sizeof(line), "CAL RESET");
            display.println(line);
            display.println("Hold to clear");
            display.println("min/max");
            display.println(" ");
        }
        else if (isTest)
        {
            int page = static_cast<int>((millis() / 1000) % 6);
            snprintf(line, sizeof(line), "CAL %s", kInputLabels[page]);
            display.println(line);
            snprintf(line, sizeof(line), "Cur %u", static_cast<unsigned>(rawInputs[page]));
            display.println(line);
            snprintf(line, sizeof(line), "Min %u", static_cast<unsigned>(minInputs[page]));
            display.println(line);
            snprintf(line, sizeof(line), "Max %u", static_cast<unsigned>(maxInputs[page]));
            display.println(line);
        }
        else
        {
            if (!status.audioOk)
            {
                snprintf(line, sizeof(line), "AUD FAIL");
            }
            else
            {
                snprintf(line, sizeof(line), "Und %lu", static_cast<unsigned long>(status.underruns));
            }
            display.println(line);

            char cv0Flag = cv0Changed ? '*' : ' ';
            char cv1Flag = cv1Changed ? '*' : ' ';
            snprintf(line, sizeof(line), "C0%c %.2f C1%c %.2f", cv0Flag, params.cv0, cv1Flag, params.cv1);
            display.println(line);

            char cv2Flag = cv2Changed ? '*' : ' ';
            char pot0Flag = pot0Changed ? '*' : ' ';
            snprintf(line, sizeof(line), "C2%c %.2f P0%c %.2f", cv2Flag, params.cv2, pot0Flag, params.pot0);
            display.println(line);

            char pot1Flag = pot1Changed ? '*' : ' ';
            char pot2Flag = pot2Changed ? '*' : ' ';
            snprintf(line, sizeof(line), "P1%c %.2f P2%c %.2f", pot1Flag, params.pot1, pot2Flag, params.pot2);
            display.println(line);
        }

        display.display();
        return;
    }
    for (int i = 0; i < 4; ++i)
    {
        int itemIndex = topIndex + i;
        if (itemIndex >= static_cast<int>(kMenuItems.size()))
        {
            break;
        }
        char valueBuffer[22] = {0};
        formatValue(itemIndex, valueBuffer, sizeof(valueBuffer));
        const auto &info = disyn::GetAlgorithmInfo(params.algorithm);
        const char *label = kMenuItems[itemIndex].label;
        if (itemIndex == 5)
        {
            label = info.param1.label;
        }
        else if (itemIndex == 6)
        {
            label = info.param2.label;
        }
        if (itemIndex == currentIndex)
        {
            display.print(">");
        }
        else
        {
            display.print(" ");
        }

        display.print(label);
        display.print(" ");
        display.println(valueBuffer);
    }
    display.display();
}

void Task(void *parameters)
{
    (void)parameters;
    Init();

    for (;;)
    {
        Tick();
        vTaskDelay(pdMS_TO_TICKS(16));
    }
}

} // namespace disyn::ui
