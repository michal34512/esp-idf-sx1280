#include <inttypes.h>
#include <stdio.h>

#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include "lora.h"
#define MESS_SIZE  200
#define MESS_COUNT 100

uint16_t RxIrqMask = IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT;
uint16_t TxIrqMask = IRQ_TX_DONE | IRQ_RX_TX_TIMEOUT;
uint8_t buffer[MESS_SIZE];
uint8_t payloadSize = 0;

RadioLoRaSpreadingFactors_t spreading_factors[]
  = {LORA_SF5, LORA_SF6, LORA_SF7, LORA_SF8, LORA_SF9, LORA_SF10, LORA_SF11, LORA_SF12};
RadioLoRaBandwidths_t bandwidths[] = {LORA_BW_0200, LORA_BW_0400, LORA_BW_0800, LORA_BW_1600};
ModulationParams_t modulationParams;
uint8_t current_spreading = 0;
uint8_t current_bandwidth = 0;
uint32_t sent_message     = 0;

uint64_t start_time = 0;
uint64_t end_time   = 0;
EventGroupHandle_t xCreatedEventGroup;
EventBits_t eventBits;

const uint8_t message[]
  = "Szanowny Panie Prezesie Ty chamie nieskrobany. Jest Pan nedznym wyrzutkiem spoleczenstwa. Ludzkosc powinna Pana "
    "wykreslic z listy obecnosci, chamie nieskrobany. Wywalil mnie Pan po dziewieciu latach uczciwej i nienagannej "
    "pracy, wiec z pelna odpowiedzialnoscia oswiadczam, chamie nieskrobany, ze jest Pan nedznym wyrzutkiem "
    "spoleczenstwa Ty chamie nieskrobany.";

void txDone();
void rxDone();
void txTimeout();
void rxTimeout();

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

void next_config() {
    current_spreading++;
    if (current_spreading >= sizeof(spreading_factors) / sizeof(spreading_factors[0])) {
        current_spreading = 0;
        current_bandwidth++;
        if (current_bandwidth >= sizeof(bandwidths) / sizeof(bandwidths[0]))
            current_bandwidth = 0;
    }
    modulationParams.PacketType                  = PACKET_TYPE_LORA;
    modulationParams.Params.LoRa.SpreadingFactor = spreading_factors[current_spreading];
    modulationParams.Params.LoRa.Bandwidth       = bandwidths[current_bandwidth];
    modulationParams.Params.LoRa.CodingRate      = LORA_CR_LI_4_5;
    Radio.SetModulationParams(&modulationParams);
}

void rxDone() {
    printf("receiving done \n");
}

void txDone() {
    printf("transmiting done \n");
    sent_message++;
    if (sent_message >= MESS_COUNT) {
        // All messages sent, calculating datarate
        end_time     = esp_timer_get_time();
        float time_s = (float)(end_time - start_time) * 0.000001f;
        printf("Spreading: %d Bandwidth: %d DataRate: %f Time (s): %f \n",
               current_spreading,
               current_bandwidth,
               (float)(MESS_SIZE * MESS_COUNT * 8.f) / time_s,
               time_s);
        next_config();
        vTaskDelay(100);
        sent_message = 0;
        xEventGroupSetBits(xCreatedEventGroup, BIT0);  // next messages
    }
}

void rxTimeout() {
    printf("receiving timeout \n");
    Radio.SetRx((TickTime_t){RADIO_TICK_SIZE_1000_US, UINT16_MAX});
}

void txTimeout() {
    printf("transmission timeout \n");
}

void app_main(void) {
    xCreatedEventGroup   = xEventGroupCreate();
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
    if (LoraHwInit(&cfg) != ESP_OK)
        printf("ERROR\n");

    Radio.Init(&Callbacks);
    modulationParams.PacketType                  = PACKET_TYPE_LORA;
    modulationParams.Params.LoRa.SpreadingFactor = LORA_SF5;      ///
    modulationParams.Params.LoRa.Bandwidth       = LORA_BW_0200;  ///
    modulationParams.Params.LoRa.CodingRate      = LORA_CR_LI_4_5;

    PacketParams_t packetParams;
    packetParams.PacketType                 = PACKET_TYPE_LORA;
    packetParams.Params.LoRa.PreambleLength = 12;
    packetParams.Params.LoRa.HeaderType     = LORA_PACKET_VARIABLE_LENGTH;
    packetParams.Params.LoRa.PayloadLength  = MESS_SIZE;
    packetParams.Params.LoRa.CrcMode        = LORA_CRC_ON;
    packetParams.Params.LoRa.InvertIQ       = LORA_IQ_NORMAL;

    Radio.SetTxParams(0, RADIO_RAMP_02_US);  ///
    Radio.SetStandby(STDBY_RC);
    Radio.SetPacketType(modulationParams.PacketType);
    Radio.SetModulationParams(&modulationParams);
    Radio.SetPacketParams(&packetParams);
    Radio.SetRfFrequency(2450000000);
    Radio.SetBufferBaseAddresses(0x00, 0x00);

    printf("Version: %x\n", Radio.GetFirmwareVersion());
    while (true) {
        start_time = esp_timer_get_time();
        for (int i = 0; i < MESS_COUNT; i++) {
            // Transmit
            Radio.SetDioIrqParams(TxIrqMask, TxIrqMask, IRQ_RADIO_NONE, IRQ_RADIO_NONE);
            Radio.SendPayload(message, MESS_SIZE, (TickTime_t){RADIO_TICK_SIZE_1000_US, UINT16_MAX});
            printf("Next\n");
        }
        eventBits = xEventGroupWaitBits(
          xCreatedEventGroup, BIT0, pdTRUE, pdFALSE, portMAX_DELAY);  // Waiting for all message to be sent
    }
}