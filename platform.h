#ifndef VL53L7CX_PLATFORM_H
#define VL53L7CX_PLATFORM_H

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VL53L7CX_Platform
{
    /* ST ULD address format: 8-bit write address, for example 0x52. */
    uint16_t address;
    /* Optional pointer available to the MCU-specific port functions. */
    void *user_data;
} VL53L7CX_Platform;

#ifndef VL53L7CX_NB_TARGET_PER_ZONE
#define VL53L7CX_NB_TARGET_PER_ZONE 1U
#endif

#if (VL53L7CX_NB_TARGET_PER_ZONE < 1U) || \
    (VL53L7CX_NB_TARGET_PER_ZONE > 4U)
#error "VL53L7CX_NB_TARGET_PER_ZONE must be between 1 and 4"
#endif

/*
 * Maximum complete host-to-breakout frame. The breakout firmware accepts at
 * most 256 bytes, including one metadata byte and the 16-bit register address.
 * Reduce this value when the host I2C driver has a smaller transmit buffer.
 */
#ifndef VL53L7CX_BRIDGE_MAX_FRAME_SIZE
#define VL53L7CX_BRIDGE_MAX_FRAME_SIZE 31U
#endif

#if (VL53L7CX_BRIDGE_MAX_FRAME_SIZE < 4U) || \
    (VL53L7CX_BRIDGE_MAX_FRAME_SIZE > 256U)
#error "VL53L7CX_BRIDGE_MAX_FRAME_SIZE must be between 4 and 256"
#endif

/* Maximum payload returned by one breakout read request (one-byte length). */
#ifndef VL53L7CX_BRIDGE_MAX_READ_SIZE
#define VL53L7CX_BRIDGE_MAX_READ_SIZE 31U
#endif

#if (VL53L7CX_BRIDGE_MAX_READ_SIZE < 1U) || \
    (VL53L7CX_BRIDGE_MAX_READ_SIZE > 255U)
#error "VL53L7CX_BRIDGE_MAX_READ_SIZE must be between 1 and 255"
#endif

/* Tested conservative defaults; tune only after testing the target host MCU. */
#ifndef VL53L7CX_BRIDGE_WRITE_DELAY_BASE_US
#define VL53L7CX_BRIDGE_WRITE_DELAY_BASE_US 0U
#endif

#ifndef VL53L7CX_BRIDGE_WRITE_DELAY_PER_BYTE_US
#define VL53L7CX_BRIDGE_WRITE_DELAY_PER_BYTE_US 10U
#endif

#ifndef VL53L7CX_BRIDGE_READ_PREFETCH_BASE_US
#define VL53L7CX_BRIDGE_READ_PREFETCH_BASE_US 300U
#endif

#ifndef VL53L7CX_BRIDGE_READ_PREFETCH_PER_BYTE_US
#define VL53L7CX_BRIDGE_READ_PREFETCH_PER_BYTE_US 20U
#endif

#ifndef VL53L7CX_BRIDGE_ADDRESS_SAVE_DELAY_MS
#define VL53L7CX_BRIDGE_ADDRESS_SAVE_DELAY_MS 25U
#endif

#ifndef VL53L7CX_BRIDGE_RESET_LOW_TIME_MS
#define VL53L7CX_BRIDGE_RESET_LOW_TIME_MS 100U
#endif

#ifndef VL53L7CX_BRIDGE_RESET_BOOT_TIME_MS
#define VL53L7CX_BRIDGE_RESET_BOOT_TIME_MS 100U
#endif

#define VL53L7CX_PLATFORM_OK    ((uint8_t)0U)
#define VL53L7CX_PLATFORM_ERROR ((uint8_t)255U)

uint8_t VL53L7CX_RdByte(VL53L7CX_Platform *p_platform,
                        uint16_t RegisterAddress,
                        uint8_t *p_value);
uint8_t VL53L7CX_WrByte(VL53L7CX_Platform *p_platform,
                        uint16_t RegisterAddress,
                        uint8_t value);
uint8_t VL53L7CX_RdMulti(VL53L7CX_Platform *p_platform,
                         uint16_t RegisterAddress,
                         uint8_t *p_values,
                         uint32_t size);
uint8_t VL53L7CX_WrMulti(VL53L7CX_Platform *p_platform,
                         uint16_t RegisterAddress,
                         uint8_t *p_values,
                         uint32_t size);
uint8_t VL53L7CX_Reset_Sensor(VL53L7CX_Platform *p_platform);
void VL53L7CX_SwapBuffer(uint8_t *buffer, uint16_t size);
uint8_t VL53L7CX_WaitMs(VL53L7CX_Platform *p_platform, uint32_t TimeMs);
uint8_t VL53L7CX_WaitUs(VL53L7CX_Platform *p_platform, uint32_t TimeUs);

#ifdef __cplusplus
}
#endif

#endif /* VL53L7CX_PLATFORM_H */
