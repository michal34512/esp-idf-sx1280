#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "driver/gpio.h"
#include "hw-gpio-isr-daemon.h"

#define GPIO_ISR_DAEMON_STACK_SIZE 2048

static StackType_t gpioIsrDaemonStack[GPIO_ISR_DAEMON_STACK_SIZE];
static StaticTask_t gpioIsrDaemonBuffer;
static GpioIrqHandler *GpioIrq[GPIO_NUM_MAX];

static TaskHandle_t taskHandle;
static uint32_t ulNotifiedValue = 0;

static void GpioIsrDaemon(void *);

void HwGpioIsrDaemonInit() {
    taskHandle = xTaskCreateStaticPinnedToCore(GpioIsrDaemon,
                                               "GPIO ISR DAEMON",
                                               GPIO_ISR_DAEMON_STACK_SIZE,
                                               NULL,
                                               1,
                                               gpioIsrDaemonStack,
                                               &gpioIsrDaemonBuffer,
                                               0);
}

void HwGpioIsrDaemonDeinit() {
    vTaskDelete(taskHandle);
}

void HwGpioHandleInterruptFromIsr(gpio_num_t gpio_num) {
    xTaskNotifyFromISR(taskHandle, (uint32_t)gpio_num, eSetValueWithOverwrite, NULL);
}

void HwGpioSetupInterruptHandler(GpioIrqHandler handler, int pin) {
    GpioIrq[pin] = handler;
}
static void GpioIsrDaemon(void *) {
    for (;;) {
        if (xTaskNotifyWait(0, ULONG_MAX, &ulNotifiedValue, portMAX_DELAY) == pdTRUE) {
            if (ulNotifiedValue < GPIO_NUM_MAX && GpioIrq[ulNotifiedValue] != NULL) {
                GpioIrq[ulNotifiedValue]();
            }
        }
    }
}
