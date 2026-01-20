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
    Serial.println("BOOT: setup start");

    disyn::gParamQueue = xQueueCreate(1, sizeof(disyn::ParamMessage));
    disyn::gStatusQueue = xQueueCreate(1, sizeof(disyn::StatusMessage));
    Serial.print("BOOT: queues ");
    Serial.print(disyn::gParamQueue != nullptr ? "ok" : "fail");
    Serial.print(" / ");
    Serial.println(disyn::gStatusQueue != nullptr ? "ok" : "fail");

    xTaskCreatePinnedToCore(
        disyn::dsp::Task,
        "DisynDSP",
        4096,
        nullptr,
        2,
        &dspHandle,
        1);
    Serial.println("BOOT: DSP task created");

    xTaskCreatePinnedToCore(
        disyn::ui::Task,
        "DisynUI",
        4096,
        nullptr,
        2,
        &uiHandle,
        0);
    Serial.println("BOOT: UI task created");
}

void loop()
{
    delay(1000);
}
