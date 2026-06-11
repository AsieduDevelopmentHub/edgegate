#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

struct TaskMessage {
    uint8_t type;
    char data[64];
};

class TaskManager {
public:
    static constexpr int QUEUE_SIZE = 32;

    bool begin() {
        queue_ = xQueueCreate(QUEUE_SIZE, sizeof(TaskMessage));
        return queue_ != nullptr;
    }

    bool send(uint8_t type, const char* data = "") {
        TaskMessage msg = {};
        msg.type = type;
        if (data) strncpy(msg.data, data, 63);
        return xQueueSend(queue_, &msg, 0) == pdTRUE;
    }

    bool receive(TaskMessage& msg, TickType_t timeout = 0) {
        return xQueueReceive(queue_, &msg, timeout) == pdTRUE;
    }

    static void createTask(TaskFunction_t fn, const char* name, uint32_t stack,
                           UBaseType_t priority, BaseType_t core) {
        xTaskCreatePinnedToCore(fn, name, stack, nullptr, priority, nullptr, core);
    }

private:
    QueueHandle_t queue_ = nullptr;
};
