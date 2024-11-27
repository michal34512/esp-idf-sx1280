
#ifndef __HW_GPIO_ISR_DAEMON_H__
#define __HW_GPIO_ISR_DAEMON_H__

typedef void(GpioIrqHandler)(void);

void HwGpioIsrDaemonInit();

void HwGpioIsrDaemonDeinit();

void HwGpioSetupInterruptHandler(GpioIrqHandler handler, int pin);

void HwGpioHandleInterruptFromIsr(gpio_num_t gpio_num);

#endif  // __HW_GPIO_ISR_DAEMON_H__
