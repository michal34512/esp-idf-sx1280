/*
  ______                              _
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2016 Semtech

Description: Handling of the node configuration protocol

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis, Matthieu Verdy and Benjamin Boulet
*/
#include "sx1280-hal.h"
#include "esp_log.h"
#include "hw.h"
#include "sx1280-radio.h"
#include <string.h>

/*!
 * \brief Define the size of tx and rx hal buffers
 *
 * The Tx and Rx hal buffers are used for SPI communication to
 * store data to be sent/receive to/from the chip.
 *
 * \warning The application must ensure the maximal useful size to be much lower
 *          than the MAX_HAL_BUFFER_SIZE
 */
#define MAX_HAL_BUFFER_SIZE 0x3FF

#define IRQ_HIGH_PRIORITY 0

#define SET_IRQ(PIN, IRQ_HANDLER)                                            \
    {                                                                        \
        if (PIN != 0) {                                                      \
            if (GpioSetIrq(PIN, IRQ_HIGH_PRIORITY, IRQ_HANDLER) != ESP_OK) { \
                ESP_LOGW("LORA HAL", "Error setting IRQ on pin %d", PIN);    \
            }                                                                \
        }                                                                    \
    }

#define GET_IRQ_STATUS(PIN, STATUS)  \
    {                                \
        if (PIN != 0) {              \
            STATUS = STATUS << 1;    \
            STATUS |= GpioRead(PIN); \
        }                            \
    }
/*!
 * Radio driver structure initialization
 */
const struct Radio_s Radio = {
  SX1280Init,
  SX1280HalReset,
  SX1280GetStatus,
  SX1280HalWriteCommand,
  SX1280HalReadCommand,
  SX1280HalWriteRegisters,
  SX1280HalWriteRegister,
  SX1280HalReadRegisters,
  SX1280HalReadRegister,
  SX1280HalWriteBuffer,
  SX1280HalReadBuffer,
  SX1280HalGetDioStatus,
  SX1280GetFirmwareVersion,
  SX1280SetRegulatorMode,
  SX1280SetStandby,
  SX1280SetPacketType,
  SX1280SetModulationParams,
  SX1280SetPacketParams,
  SX1280SetRfFrequency,
  SX1280SetBufferBaseAddresses,
  SX1280SetTxParams,
  SX1280SetDioIrqParams,
  SX1280SetSyncWord,
  SX1280SetRx,
  SX1280GetPayload,
  SX1280SendPayload,
  SX1280SetRangingRole,
  SX1280SetPollingMode,
  SX1280SetInterruptMode,
  SX1280SetRegistersDefault,
  SX1280GetOpMode,
  SX1280SetSleep,
  SX1280SetFs,
  SX1280SetTx,
  SX1280SetRxDutyCycle,
  SX1280SetCad,
  SX1280SetTxContinuousWave,
  SX1280SetTxContinuousPreamble,
  SX1280GetPacketType,
  SX1280SetCadParams,
  SX1280GetRxBufferStatus,
  SX1280GetPacketStatus,
  SX1280GetRssiInst,
  SX1280GetIrqStatus,
  SX1280ClearIrqStatus,
  SX1280Calibrate,
  SX1280SetSaveContext,
  SX1280SetAutoTx,
  SX1280StopAutoTx,
  SX1280SetAutoFS,
  SX1280SetLongPreamble,
  SX1280SetPayload,
  SX1280SetSyncWordErrorTolerance,
  SX1280SetCrcSeed,
  SX1280SetBleAccessAddress,
  SX1280SetBleAdvertizerAccessAddress,
  SX1280SetCrcPolynomial,
  SX1280SetWhiteningSeed,
  SX1280EnableManualGain,
  SX1280DisableManualGain,
  SX1280SetManualGainValue,
  SX1280SetLNAGainSetting,
  SX1280SetRangingIdLength,
  SX1280SetDeviceRangingAddress,
  SX1280SetRangingRequestAddress,
  SX1280GetRangingResult,
  SX1280SetRangingCalibration,
  SX1280GetRangingPowerDeltaThresholdIndicator,
  SX1280RangingClearFilterResult,
  SX1280RangingSetFilterNumSamples,
  SX1280GetFrequencyError,
};

static uint8_t halTxBuffer[MAX_HAL_BUFFER_SIZE] = {0x00};
static uint8_t halRxBuffer[MAX_HAL_BUFFER_SIZE] = {0x00};

static lora_hw_config_t *hw_cfg;

void SX1280SetHwConfig(lora_hw_config_t *config) {
    hw_cfg = config;
}

/*!
 * \brief Used to block execution waiting for low state on radio busy pin.
 *        Essentially used in SPI communications
 */
void SX1280HalWaitOnBusy(void) {
    while (GpioRead(hw_cfg->busy_pin) == 1)
        ;
}

void SX1280HalInit(DioIrqHandler **irqHandlers) {
    SX1280HalReset();
    SX1280HalIoIrqInit(irqHandlers);
}

void SX1280HalIoIrqInit(DioIrqHandler **irqHandlers) {
    SET_IRQ(hw_cfg->dio1_pin, irqHandlers[0]);
    SET_IRQ(hw_cfg->dio2_pin, irqHandlers[0]);
    SET_IRQ(hw_cfg->dio3_pin, irqHandlers[0]);
}

void SX1280HalReset(void) {
    HAL_Delay(20);
    GpioWrite(hw_cfg->nreset_pin, 0);
    HAL_Delay(50);
    GpioWrite(hw_cfg->nreset_pin, 1);
    HAL_Delay(20);
}

void SX1280HalWakeup(void) {
    GpioWrite(hw_cfg->nss_pin, 0);

    uint16_t halSize = 2;
    halTxBuffer[0]   = RADIO_GET_STATUS;
    halTxBuffer[1]   = 0x00;
    SpiIn(halTxBuffer, halSize);

    GpioWrite(hw_cfg->nss_pin, 1);

    // Wait for chip to be ready.
    SX1280HalWaitOnBusy();
}

void SX1280HalWriteCommand(RadioCommands_t command, uint8_t *buffer, uint16_t size) {
    uint16_t halSize = size + 1;
    SX1280HalWaitOnBusy();

    GpioWrite(hw_cfg->nss_pin, 0);

    halTxBuffer[0] = command;
    memcpy(halTxBuffer + 1, (uint8_t *)buffer, size * sizeof(uint8_t));

    SpiIn(halTxBuffer, halSize);

    GpioWrite(hw_cfg->nss_pin, 1);

    if (command != RADIO_SET_SLEEP) {
        SX1280HalWaitOnBusy();
    }
}

void SX1280HalReadCommand(RadioCommands_t command, uint8_t *buffer, uint16_t size) {
    uint16_t halSize = 2 + size;
    halTxBuffer[0]   = command;
    halTxBuffer[1]   = 0x00;
    for (uint16_t index = 0; index < size; index++) {
        halTxBuffer[2 + index] = 0x00;
    }

    SX1280HalWaitOnBusy();

    GpioWrite(hw_cfg->nss_pin, 0);

    SpiInOut(halTxBuffer, halRxBuffer, halSize);

    memcpy(buffer, halRxBuffer + 2, size);

    GpioWrite(hw_cfg->nss_pin, 1);

    SX1280HalWaitOnBusy();
}

void SX1280HalWriteRegisters(uint16_t address, uint8_t *buffer, uint16_t size) {
    uint16_t halSize = size + 3;
    halTxBuffer[0]   = RADIO_WRITE_REGISTER;
    halTxBuffer[1]   = (address & 0xFF00) >> 8;
    halTxBuffer[2]   = address & 0x00FF;
    memcpy(halTxBuffer + 3, buffer, size);

    SX1280HalWaitOnBusy();

    GpioWrite(hw_cfg->nss_pin, 0);

    SpiIn(halTxBuffer, halSize);

    GpioWrite(hw_cfg->nss_pin, 1);

    SX1280HalWaitOnBusy();
}

void SX1280HalWriteRegister(uint16_t address, uint8_t value) {
    SX1280HalWriteRegisters(address, &value, 1);
}

void SX1280HalReadRegisters(uint16_t address, uint8_t *buffer, uint16_t size) {
    uint16_t halSize = 4 + size;
    halTxBuffer[0]   = RADIO_READ_REGISTER;
    halTxBuffer[1]   = (address & 0xFF00) >> 8;
    halTxBuffer[2]   = address & 0x00FF;
    halTxBuffer[3]   = 0x00;
    for (uint16_t index = 0; index < size; index++) {
        halTxBuffer[4 + index] = 0x00;
    }

    SX1280HalWaitOnBusy();

    GpioWrite(hw_cfg->nss_pin, 0);

    SpiInOut(halTxBuffer, halRxBuffer, halSize);

    memcpy(buffer, halRxBuffer + 4, size);

    GpioWrite(hw_cfg->nss_pin, 1);

    SX1280HalWaitOnBusy();
}

uint8_t SX1280HalReadRegister(uint16_t address) {
    uint8_t data;

    SX1280HalReadRegisters(address, &data, 1);

    return data;
}

void SX1280HalWriteBuffer(uint8_t offset, uint8_t *buffer, uint8_t size) {
    uint16_t halSize = size + 2;
    halTxBuffer[0]   = RADIO_WRITE_BUFFER;
    halTxBuffer[1]   = offset;
    memcpy(halTxBuffer + 2, buffer, size);

    SX1280HalWaitOnBusy();

    GpioWrite(hw_cfg->nss_pin, 0);

    SpiIn(halTxBuffer, halSize);

    GpioWrite(hw_cfg->nss_pin, 1);

    SX1280HalWaitOnBusy();
}

void SX1280HalReadBuffer(uint8_t offset, uint8_t *buffer, uint8_t size) {
    uint16_t halSize = size + 3;
    halTxBuffer[0]   = RADIO_READ_BUFFER;
    halTxBuffer[1]   = offset;
    halTxBuffer[2]   = 0x00;
    for (uint16_t index = 0; index < size; index++) {
        halTxBuffer[3 + index] = 0x00;
    }

    SX1280HalWaitOnBusy();

    GpioWrite(hw_cfg->nss_pin, 0);

    SpiInOut(halTxBuffer, halRxBuffer, halSize);

    memcpy(buffer, halRxBuffer + 3, size);

    GpioWrite(hw_cfg->nss_pin, 1);

    SX1280HalWaitOnBusy();
}

uint8_t SX1280HalGetDioStatus(void) {
    uint8_t Status = GpioRead(hw_cfg->busy_pin);
    GET_IRQ_STATUS(hw_cfg->dio3_pin, Status);
    GET_IRQ_STATUS(hw_cfg->dio2_pin, Status);
    GET_IRQ_STATUS(hw_cfg->dio1_pin, Status);
    return Status;
}
