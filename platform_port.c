#include "platform_port.h"

/*
 * Replace these stubs with the host MCU implementation. p_platform->user_data
 * can point to an I2C handle, Arduino object wrapper, RTOS context, or board
 * descriptor. Keep each I2C operation as one transaction ending with STOP.
 */

uint8_t VL53L7CX_PlatformI2CWrite(VL53L7CX_Platform *p_platform,
                                  uint8_t address_7bit,
                                  const uint8_t *data,
                                  uint32_t size)
{
    (void)p_platform;
    (void)address_7bit;
    (void)data;
    (void)size;
    return VL53L7CX_PLATFORM_ERROR;
}

uint8_t VL53L7CX_PlatformI2CRead(VL53L7CX_Platform *p_platform,
                                 uint8_t address_7bit,
                                 uint8_t *data,
                                 uint32_t size)
{
    (void)p_platform;
    (void)address_7bit;
    (void)data;
    (void)size;
    return VL53L7CX_PLATFORM_ERROR;
}

uint8_t VL53L7CX_PlatformDelayUs(VL53L7CX_Platform *p_platform,
                                 uint32_t delay_us)
{
    (void)p_platform;
    (void)delay_us;
    return VL53L7CX_PLATFORM_ERROR;
}

uint8_t VL53L7CX_PlatformSetLpn(VL53L7CX_Platform *p_platform,
                                uint8_t level)
{
    (void)p_platform;
    (void)level;
    return VL53L7CX_PLATFORM_ERROR;
}
