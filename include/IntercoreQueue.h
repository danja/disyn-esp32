#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

namespace disyn {

extern QueueHandle_t gParamQueue;
extern QueueHandle_t gStatusQueue;

} // namespace disyn
