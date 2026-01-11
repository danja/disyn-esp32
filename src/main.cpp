#include <Arduino.h>

#include "dsp/DspTask.h"
#include "ui/UiTask.h"

#include "IntercoreQueue.h"
#include "Parameters.h"

TaskHandle_t uiHandle = nullptr;
TaskHandle_t dspHandle = nullptr;

QueueHandle_t disyn::gParamQueue = nullptr;
QueueHandle_t disyn::gStatusQueue = nullptr;

void setup()
{
    Serial.begin(115200);

    disyn::gParamQueue = xQueueCreate(1, sizeof(disyn::ParamMessage));
    disyn::gStatusQueue = xQueueCreate(1, sizeof(disyn::StatusMessage));

    xTaskCreatePinnedToCore(
        disyn::dsp::Task,
        "DisynDSP",
        4096,
        nullptr,
        2,
        &dspHandle,
        1);

    xTaskCreatePinnedToCore(
        disyn::ui::Task,
        "DisynUI",
        4096,
        nullptr,
        2,
        &uiHandle,
        0);
}

void loop()
{
    delay(1000);
}
