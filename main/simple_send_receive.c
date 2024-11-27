#include <inttypes.h>
#include <stdio.h>

#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include "lora.h"
//#define TRANS

#ifndef TRANS
uint8_t received[100] = {"_"};
#endif

void rxDone() {
    uint8_t payloadSize = 0;
    Radio.GetPayload(received, &payloadSize, 100);
    printf("Received: %.*s\n", payloadSize, received);
    printf("receive done \n");
    Radio.SetRx((TickTime_t){RADIO_TICK_SIZE_1000_US, UINT16_MAX});
}

void txDone() {
    printf("transmission done \n");
}

void rxTimeout() {
    printf("receive timeout \n");
    Radio.SetRx((TickTime_t){RADIO_TICK_SIZE_1000_US, UINT16_MAX});
}

void txTimeout() {
    printf("transmission timeout \n");
}

const uint8_t message[] = "THIS IS MESSAGE";

uint16_t RxIrqMask = IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT;

uint16_t TxIrqMask = IRQ_TX_DONE | IRQ_RX_TX_TIMEOUT;

void app_main(void) {
#ifdef TRANS
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
    modulationParams.Params.LoRa.SpreadingFactor = LORA_SF12;
    modulationParams.Params.LoRa.Bandwidth       = LORA_BW_1600;
    modulationParams.Params.LoRa.CodingRate      = LORA_CR_LI_4_7;

    PacketParams_t packetParams;
    packetParams.PacketType                 = PACKET_TYPE_LORA;
    packetParams.Params.LoRa.PreambleLength = 12;
    packetParams.Params.LoRa.HeaderType     = LORA_PACKET_VARIABLE_LENGTH;
    packetParams.Params.LoRa.PayloadLength  = 20;
    packetParams.Params.LoRa.CrcMode        = LORA_CRC_ON;
    packetParams.Params.LoRa.InvertIQ       = LORA_IQ_NORMAL;

    Radio.SetStandby(STDBY_RC);
    Radio.SetPacketType(modulationParams.PacketType);
    Radio.SetModulationParams(&modulationParams);
    Radio.SetPacketParams(&packetParams);
    Radio.SetRfFrequency(2404000000);
    Radio.SetBufferBaseAddresses(0x00, 0x00);

    printf("Version: %x\n", Radio.GetFirmwareVersion());

#ifdef TRANS
    Radio.SetDioIrqParams(TxIrqMask, TxIrqMask, IRQ_RADIO_NONE, IRQ_RADIO_NONE);
    Radio.SendPayload(message, 16, (TickTime_t){RADIO_TICK_SIZE_1000_US, 1000});
#else
    Radio.SetDioIrqParams(RxIrqMask, RxIrqMask, IRQ_RADIO_NONE, IRQ_RADIO_NONE);
    Radio.SetRx((TickTime_t){RADIO_TICK_SIZE_1000_US, UINT16_MAX});
    printf("Waiting....\n");
#endif
    vTaskDelay(10000);
}
