#include <inttypes.h>
#include <stdio.h>

#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include "lora.h"
#define ESP32s3

uint16_t RxIrqMask = IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT;
uint16_t TxIrqMask = IRQ_TX_DONE | IRQ_RX_TX_TIMEOUT;
uint8_t buffer[100];
EventGroupHandle_t receivedMessageEvent;

static StackType_t pingPongTaskStack[4096];
static StaticTask_t pingPongTaskBuffer;

void rxDone() {
    uint8_t payloadSize = 0;
    Radio.GetPayload(buffer, &payloadSize, 100);
    printf("Received message: %.*s\n", payloadSize, buffer);
    xEventGroupSetBits(receivedMessageEvent, BIT0);
}

void txDone() {
    printf("transmission completed \n");
    xEventGroupSetBits(receivedMessageEvent, BIT1);
}

void rxTimeout() {
    printf("receiving timeout \n");
    Radio.SetRx((TickTime_t){RADIO_TICK_SIZE_1000_US, UINT16_MAX});
}

void txTimeout() {
    printf("transmission timeout \n");
}

const uint8_t message[] = "THIS IS MESSAGE";

static void ping_pong_task();

void app_main(void) {
#ifdef ESP32s3
    lora_hw_config_t cfg = {
      .nss_pin    = 10,
      .mosi_pin   = 11,
      .miso_pin   = 13,
      .sclk_pin   = 12,
      .nreset_pin = 4,
      .busy_pin   = 5,
      .dio1_pin   = 6,
      .dio2_pin   = 0,
      .dio3_pin   = 0,
      .spi_host   = SPI2_HOST,
    };
#else
    lora_hw_config_t cfg = {
      .nss_pin    = 5,
      .mosi_pin   = 23,
      .miso_pin   = 19,
      .sclk_pin   = 18,
      .nreset_pin = 22,
      .busy_pin   = 32,
      .dio1_pin   = 33,
      .dio2_pin   = 0,
      .dio3_pin   = 0,
      .spi_host   = SPI2_HOST,
    };
#endif
    if (LoraHwInit(&cfg) != ESP_OK)
        printf("ERROR\n");

    RadioCallbacks_t Callbacks = {
      txDone,     // txDone
      rxDone,     // rxDone
      NULL,       // syncWordDone
      NULL,       // headerDone
      txTimeout,  // txTimeout
      rxTimeout,  // rxTimeout
      NULL,       // rxError
      NULL,       // rangingDone
      NULL,       // cadDone
    };
    Radio.Init(&Callbacks);

    ModulationParams_t modulationParams;
    modulationParams.PacketType                  = PACKET_TYPE_LORA;
    modulationParams.Params.LoRa.SpreadingFactor = LORA_SF12;     ///
    modulationParams.Params.LoRa.Bandwidth       = LORA_BW_1600;  ///
    modulationParams.Params.LoRa.CodingRate      = LORA_CR_LI_4_7;

    PacketParams_t packetParams;
    packetParams.PacketType                 = PACKET_TYPE_LORA;
    packetParams.Params.LoRa.PreambleLength = 12;
    packetParams.Params.LoRa.HeaderType     = LORA_PACKET_VARIABLE_LENGTH;
    packetParams.Params.LoRa.PayloadLength  = 20;
    packetParams.Params.LoRa.CrcMode        = LORA_CRC_ON;
    packetParams.Params.LoRa.InvertIQ       = LORA_IQ_NORMAL;

    Radio.SetTxParams(0, RADIO_RAMP_02_US);  ///
    Radio.SetStandby(STDBY_RC);
    Radio.SetPacketType(modulationParams.PacketType);
    Radio.SetModulationParams(&modulationParams);
    Radio.SetPacketParams(&packetParams);
    Radio.SetRfFrequency(2404000000);
    Radio.SetBufferBaseAddresses(0x00, 0x00);

    printf("Version: %x\n", Radio.GetFirmwareVersion());
    xTaskCreateStaticPinnedToCore(
      ping_pong_task, "PING PONG TASK", 4096, NULL, 1, pingPongTaskStack, &pingPongTaskBuffer, 0);

    receivedMessageEvent = xEventGroupCreate();
    xEventGroupSetBits(receivedMessageEvent, BIT0);  // Start with transmit
    vTaskDelete(NULL);
}

static void ping_pong_task() {
    for (;;) {
        EventBits_t eventBits = xEventGroupWaitBits(receivedMessageEvent, BIT0 | BIT1, pdTRUE, pdFALSE, portMAX_DELAY);
        if ((eventBits & BIT0) == BIT0)  // Message received
        {
            // Transmit
            printf("transmitting message\n");
            Radio.SetDioIrqParams(TxIrqMask, TxIrqMask, IRQ_RADIO_NONE, IRQ_RADIO_NONE);
            Radio.SendPayload(message, 16, (TickTime_t){RADIO_TICK_SIZE_1000_US, 1000});
        } else if ((eventBits & BIT1) == BIT1)  // Message transmitted
        {
            // Receive
            printf("receiving message\n");
            Radio.SetDioIrqParams(RxIrqMask, RxIrqMask, IRQ_RADIO_NONE, IRQ_RADIO_NONE);
            Radio.SetRx((TickTime_t){RADIO_TICK_SIZE_1000_US, UINT16_MAX});
        }
    }
}