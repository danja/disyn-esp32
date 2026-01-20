#include <Arduino.h>
#include <esp_system.h>

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
    delay(10);
    Serial.println("BOOT: setup start");
    Serial.flush();
    Serial.print("BOOT: reset reason ");
    Serial.println(static_cast<int>(esp_reset_reason()));
    Serial.flush();

    disyn::gParamQueue = xQueueCreate(1, sizeof(disyn::ParamMessage));
    disyn::gStatusQueue = xQueueCreate(1, sizeof(disyn::StatusMessage));
    Serial.print("BOOT: queues ");
    Serial.print(disyn::gParamQueue != nullptr ? "ok" : "fail");
    Serial.print(" / ");
    Serial.println(disyn::gStatusQueue != nullptr ? "ok" : "fail");
    Serial.flush();

    Serial.println("BOOT: before UI task");
    Serial.flush();
    BaseType_t uiCreated = xTaskCreatePinnedToCore(
        disyn::ui::Task,
        "DisynUI",
        8192,
        nullptr,
        2,
        &uiHandle,
        0);
    Serial.print("BOOT: UI task created ");
    Serial.println(uiCreated == pdPASS ? "ok" : "fail");
    Serial.flush();

    Serial.println("BOOT: before DSP task");
    Serial.flush();
    BaseType_t dspCreated = xTaskCreatePinnedToCore(
        disyn::dsp::Task,
        "DisynDSP",
        8192,
        nullptr,
        2,
        &dspHandle,
        1);
    Serial.print("BOOT: DSP task created ");
    Serial.println(dspCreated == pdPASS ? "ok" : "fail");
    Serial.flush();
}

void loop()
{
    delay(1000);
}
