#include "platform.h"
#include "platform_port.h"

#include <stddef.h>
#include <string.h>

#define VL53L7CX_BRIDGE_HEADER_SIZE 3U
#define VL53L7CX_BRIDGE_READ_COMMAND_SIZE 3U
#define VL53L7CX_BRIDGE_PAGE_REGISTER ((uint16_t)0x7FFFU)
#define VL53L7CX_BRIDGE_ADDRESS_REGISTER ((uint16_t)0x0004U)
#define VL53L7CX_BRIDGE_ADDRESS_CONFIG_VALUE 0x00U
#define VL53L7CX_BRIDGE_ADDRESS_COMMIT_VALUE 0x02U
#define VL53L7CX_PLATFORM_DELAY_MAX_US 65535U

typedef enum
{
    VL53L7CX_ADDRESS_CHANGE_IDLE,
    VL53L7CX_ADDRESS_CHANGE_CONFIGURED,
    VL53L7CX_ADDRESS_CHANGE_ADDRESS_WRITTEN
} VL53L7CX_AddressChangeState;

static VL53L7CX_AddressChangeState address_change_state =
    VL53L7CX_ADDRESS_CHANGE_IDLE;

static uint8_t bridge_get_7bit_address(const VL53L7CX_Platform *p_platform,
                                       uint8_t *address_7bit)
{
    uint16_t address;

    if((p_platform == NULL) || (address_7bit == NULL))
    {
        return VL53L7CX_PLATFORM_ERROR;
    }

    address = p_platform->address;
    if((address > 0xFEU) || ((address & 1U) != 0U) ||
       (address < 0x10U))
    {
        return VL53L7CX_PLATFORM_ERROR;
    }

    *address_7bit = (uint8_t)(address >> 1U);
    return VL53L7CX_PLATFORM_OK;
}

static uint8_t bridge_write(VL53L7CX_Platform *p_platform,
                            const uint8_t *data,
                            uint32_t size)
{
    uint8_t address_7bit;

    if((data == NULL) || (size == 0U) ||
       (bridge_get_7bit_address(p_platform, &address_7bit) !=
        VL53L7CX_PLATFORM_OK))
    {
        return VL53L7CX_PLATFORM_ERROR;
    }

    return VL53L7CX_PlatformI2CWrite(p_platform, address_7bit, data, size);
}

static uint8_t bridge_read(VL53L7CX_Platform *p_platform,
                           uint8_t *data,
                           uint32_t size)
{
    uint8_t address_7bit;

    if((data == NULL) || (size == 0U) ||
       (bridge_get_7bit_address(p_platform, &address_7bit) !=
        VL53L7CX_PLATFORM_OK))
    {
        return VL53L7CX_PLATFORM_ERROR;
    }

    return VL53L7CX_PlatformI2CRead(p_platform, address_7bit, data, size);
}

static uint8_t bridge_wait_after_write(VL53L7CX_Platform *p_platform,
                                       uint32_t frame_size)
{
    uint32_t delay_us = VL53L7CX_BRIDGE_WRITE_DELAY_BASE_US +
                        (VL53L7CX_BRIDGE_WRITE_DELAY_PER_BYTE_US * frame_size);

    return VL53L7CX_WaitUs(p_platform, delay_us);
}

static uint8_t bridge_wait_for_read(VL53L7CX_Platform *p_platform,
                                    uint32_t read_size)
{
    uint32_t delay_us = VL53L7CX_BRIDGE_READ_PREFETCH_BASE_US +
                        (VL53L7CX_BRIDGE_READ_PREFETCH_PER_BYTE_US * read_size);

    return VL53L7CX_WaitUs(p_platform, delay_us);
}

uint8_t VL53L7CX_WaitUs(VL53L7CX_Platform *p_platform, uint32_t TimeUs)
{
    while(TimeUs != 0U)
    {
        uint32_t delay_us = TimeUs;
        uint8_t status;

        if(delay_us > VL53L7CX_PLATFORM_DELAY_MAX_US)
        {
            delay_us = VL53L7CX_PLATFORM_DELAY_MAX_US;
        }

        status = VL53L7CX_PlatformDelayUs(p_platform, delay_us);
        if(status != VL53L7CX_PLATFORM_OK)
        {
            address_change_state = VL53L7CX_ADDRESS_CHANGE_IDLE;
            return status;
        }
        TimeUs -= delay_us;
    }

    return VL53L7CX_PLATFORM_OK;
}

uint8_t VL53L7CX_WaitMs(VL53L7CX_Platform *p_platform, uint32_t TimeMs)
{
    uint8_t status;

    while(TimeMs >= 1000U)
    {
        status = VL53L7CX_WaitUs(p_platform, 1000000U);
        if(status != VL53L7CX_PLATFORM_OK)
        {
            return status;
        }
        TimeMs -= 1000U;
    }

    if(TimeMs != 0U)
    {
        return VL53L7CX_WaitUs(p_platform, TimeMs * 1000U);
    }

    return VL53L7CX_PLATFORM_OK;
}

uint8_t VL53L7CX_WrMulti(VL53L7CX_Platform *p_platform,
                         uint16_t RegisterAddress,
                         uint8_t *p_values,
                         uint32_t size)
{
    uint8_t frame[VL53L7CX_BRIDGE_MAX_FRAME_SIZE];
    uint8_t status = VL53L7CX_PLATFORM_OK;
    uint32_t offset = 0U;
    const uint32_t max_payload = VL53L7CX_BRIDGE_MAX_FRAME_SIZE -
                                 VL53L7CX_BRIDGE_HEADER_SIZE;

    if((size != 0U) && (p_values == NULL))
    {
        return VL53L7CX_PLATFORM_ERROR;
    }

    while(offset < size)
    {
        uint32_t chunk_size = size - offset;
        uint16_t current_register = (uint16_t)(RegisterAddress + offset);
        uint32_t frame_size;

        if(chunk_size > max_payload)
        {
            chunk_size = max_payload;
        }

        frame[0] = 0U;
        frame[1] = (uint8_t)(current_register >> 8U);
        frame[2] = (uint8_t)current_register;
        (void)memcpy(&frame[VL53L7CX_BRIDGE_HEADER_SIZE],
                     &p_values[offset], chunk_size);
        frame_size = chunk_size + VL53L7CX_BRIDGE_HEADER_SIZE;

        status = bridge_write(p_platform, frame, frame_size);
        if(status != VL53L7CX_PLATFORM_OK)
        {
            return status;
        }

        status = bridge_wait_after_write(p_platform, frame_size);
        if(status != VL53L7CX_PLATFORM_OK)
        {
            return status;
        }

        offset += chunk_size;
    }

    return status;
}

uint8_t VL53L7CX_RdMulti(VL53L7CX_Platform *p_platform,
                         uint16_t RegisterAddress,
                         uint8_t *p_values,
                         uint32_t size)
{
    uint8_t request[VL53L7CX_BRIDGE_READ_COMMAND_SIZE];
    uint8_t status = VL53L7CX_PLATFORM_OK;
    uint32_t offset = 0U;

    if((size != 0U) && (p_values == NULL))
    {
        return VL53L7CX_PLATFORM_ERROR;
    }

    while(offset < size)
    {
        uint32_t chunk_size = size - offset;
        uint16_t current_register = (uint16_t)(RegisterAddress + offset);

        if(chunk_size > VL53L7CX_BRIDGE_MAX_READ_SIZE)
        {
            chunk_size = VL53L7CX_BRIDGE_MAX_READ_SIZE;
        }

        request[0] = (uint8_t)chunk_size;
        request[1] = (uint8_t)(current_register >> 8U);
        request[2] = (uint8_t)current_register;

        status = bridge_write(p_platform, request,
                              VL53L7CX_BRIDGE_READ_COMMAND_SIZE);
        if(status != VL53L7CX_PLATFORM_OK)
        {
            return status;
        }

        status = bridge_wait_for_read(p_platform, chunk_size);
        if(status != VL53L7CX_PLATFORM_OK)
        {
            return status;
        }

        status = bridge_read(p_platform, &p_values[offset], chunk_size);
        if(status != VL53L7CX_PLATFORM_OK)
        {
            return status;
        }

        offset += chunk_size;
    }

    return status;
}

uint8_t VL53L7CX_RdByte(VL53L7CX_Platform *p_platform,
                        uint16_t RegisterAddress,
                        uint8_t *p_value)
{
    return VL53L7CX_RdMulti(p_platform, RegisterAddress, p_value, 1U);
}

uint8_t VL53L7CX_WrByte(VL53L7CX_Platform *p_platform,
                        uint16_t RegisterAddress,
                        uint8_t value)
{
    uint8_t status = VL53L7CX_WrMulti(p_platform, RegisterAddress, &value, 1U);
    uint8_t address_change_committed = 0U;

    if(status != VL53L7CX_PLATFORM_OK)
    {
        return status;
    }

    switch(address_change_state)
    {
        case VL53L7CX_ADDRESS_CHANGE_IDLE:
            if((RegisterAddress == VL53L7CX_BRIDGE_PAGE_REGISTER) &&
               (value == VL53L7CX_BRIDGE_ADDRESS_CONFIG_VALUE))
            {
                address_change_state = VL53L7CX_ADDRESS_CHANGE_CONFIGURED;
            }
            break;

        case VL53L7CX_ADDRESS_CHANGE_CONFIGURED:
            if(RegisterAddress == VL53L7CX_BRIDGE_ADDRESS_REGISTER)
            {
                address_change_state =
                    VL53L7CX_ADDRESS_CHANGE_ADDRESS_WRITTEN;
            }
            else if((RegisterAddress == VL53L7CX_BRIDGE_PAGE_REGISTER) &&
                    (value == VL53L7CX_BRIDGE_ADDRESS_CONFIG_VALUE))
            {
                address_change_state = VL53L7CX_ADDRESS_CHANGE_CONFIGURED;
            }
            else
            {
                address_change_state = VL53L7CX_ADDRESS_CHANGE_IDLE;
            }
            break;

        case VL53L7CX_ADDRESS_CHANGE_ADDRESS_WRITTEN:
            if((RegisterAddress == VL53L7CX_BRIDGE_PAGE_REGISTER) &&
               (value == VL53L7CX_BRIDGE_ADDRESS_COMMIT_VALUE))
            {
                address_change_committed = 1U;
            }
            address_change_state = VL53L7CX_ADDRESS_CHANGE_IDLE;
            break;

        default:
            address_change_state = VL53L7CX_ADDRESS_CHANGE_IDLE;
            break;
    }

    if(address_change_committed != 0U)
    {
        status = VL53L7CX_WaitMs(p_platform,
                                 VL53L7CX_BRIDGE_ADDRESS_SAVE_DELAY_MS);
    }

    return status;
}

uint8_t VL53L7CX_Reset_Sensor(VL53L7CX_Platform *p_platform)
{
    uint8_t status = VL53L7CX_PlatformSetLpn(p_platform, 0U);

    if(status == VL53L7CX_PLATFORM_OK)
    {
        status = VL53L7CX_WaitMs(p_platform,
                                 VL53L7CX_BRIDGE_RESET_LOW_TIME_MS);
    }
    if(status == VL53L7CX_PLATFORM_OK)
    {
        status = VL53L7CX_PlatformSetLpn(p_platform, 1U);
    }
    if(status == VL53L7CX_PLATFORM_OK)
    {
        status = VL53L7CX_WaitMs(p_platform,
                                 VL53L7CX_BRIDGE_RESET_BOOT_TIME_MS);
    }

    return status;
}

void VL53L7CX_SwapBuffer(uint8_t *buffer, uint16_t size)
{
    uint16_t index;

    if(buffer == NULL)
    {
        return;
    }

    for(index = 0U; (uint16_t)(index + 3U) < size; index += 4U)
    {
        uint8_t first = buffer[index];
        uint8_t second = buffer[index + 1U];

        buffer[index] = buffer[index + 3U];
        buffer[index + 1U] = buffer[index + 2U];
        buffer[index + 2U] = second;
        buffer[index + 3U] = first;
    }
}
