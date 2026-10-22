#ifndef VL53L7CX_PLATFORM_PORT_H
#define VL53L7CX_PLATFORM_PORT_H

#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Implement these four functions for the host MCU.
 * All functions return VL53L7CX_PLATFORM_OK on success and a nonzero value on
 * failure. I2C addresses passed here are always 7-bit addresses.
 */
uint8_t VL53L7CX_PlatformI2CWrite(VL53L7CX_Platform *p_platform,
                                  uint8_t address_7bit,
                                  const uint8_t *data,
                                  uint32_t size);
uint8_t VL53L7CX_PlatformI2CRead(VL53L7CX_Platform *p_platform,
                                 uint8_t address_7bit,
                                 uint8_t *data,
                                 uint32_t size);
uint8_t VL53L7CX_PlatformDelayUs(VL53L7CX_Platform *p_platform,
                                 uint32_t delay_us);
uint8_t VL53L7CX_PlatformSetLpn(VL53L7CX_Platform *p_platform,
                                uint8_t level);

#ifdef __cplusplus
}
#endif

#endif /* VL53L7CX_PLATFORM_PORT_H */
