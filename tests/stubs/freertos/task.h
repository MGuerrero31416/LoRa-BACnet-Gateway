#pragma once
typedef void *TaskHandle_t;
static inline TickType_t xTaskGetTickCount(void) { return 0; }
static inline BaseType_t xTaskCreate(
    void (*task)(void *), const char *name, uint32_t stack,
    void *argument, int priority, TaskHandle_t *handle)
{
    (void)task;
    (void)name;
    (void)stack;
    (void)argument;
    (void)priority;
    *handle = (void *)1;
    return pdPASS;
}
