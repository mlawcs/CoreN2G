#ifndef SPI_H
#define SPI_H

#include "Core.h"
#include "spi_com.h"

constexpr uint16_t SPITimeoutMillis = 250;
#if 0
typedef enum
{
    SPI_ERROR = -1,
    SPI_OK = 0,
    SPI_ERROR_TIMEOUT = 1,
    SPI_ERROR_ARGUMENT,
    SPI_ERROR_OVERRUN,
    SPI_ERROR_MODE_FAULT,
    SPI_ERROR_OVERRUN_AND_MODE_FAULT
} spi_status_t;
#endif
typedef spi_status_e spi_status_t;
// SSP/SPI Channels - yes I know the names do not match the numbers...
enum SSPChannel : uint8_t
{
    //Hardware SPI
    SSP1 = 0,
    SSP2,
    SSP3,
    //Software SPI
    SWSPI0,
    SWSPI1,
    SWSPI2,
    //Additional Hardware devices
#if STM32H7
    SSP4,
    SSP5,
    SSP6,
#endif
    SSPMAX,
    // Hardware SDIO
    SSPSDIO = 0xef,
    // Not defined
    SSPNONE = 0xff
};
constexpr size_t NumSPIDevices = (uint32_t)SSPMAX;
constexpr size_t NumSoftwareSPIDevices = 3;

class SPI
{
public:
    virtual void configureDevice(uint32_t bits, uint32_t clockMode, uint32_t bitRate) noexcept;
    virtual spi_status_t transceivePacket(const uint8_t *tx_data, uint8_t *rx_data, size_t len, Pin cs = NoPin) noexcept;    
    virtual bool waitForTxEmpty() noexcept;
    virtual void initPins(Pin sck, Pin miso, Pin mosi, NvicPriority priority = -1) noexcept;
    static SPI *getSSPDevice(SSPChannel channel) noexcept;
};

#endif
