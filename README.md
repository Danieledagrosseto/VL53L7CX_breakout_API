# VL53L7CX bridge platform layer

This folder replaces `platform.h` and `platform.c` in the ST VL53L7CX ULD when the host communicates through the programmable-address VL53L7CX bridge breakout.

## Files

* `platform.h`: ST ULD-compatible API, bridge limits, and timing configuration.
* `platform.c`: MCU-independent bridge framing, chunking, read prefetch, reset, and buffer conversion.
* `platform\_port.h`: contract for the MCU adapter.
* `platform\_port.c`: four stubs to replace with the target MCU implementation.

## Integration

1. Replace the ST ULD `platform.h` and `platform.c` with these files.
2. Add `platform\_port.h` and `platform\_port.c` to the build.
3. Implement the four functions in `platform\_port.c`.
4. Store the breakout address in ST's 8-bit format, for example `0x52`. Port functions receive the converted 7-bit address, for example `0x29`.
5. Set `p\_platform->user\_data` when the port needs an I2C handle or board context.

Each port I2C call must perform exactly one complete transaction ending with STOP. Return zero only after every requested byte has transferred successfully.

## Configuration

The defaults use 31-byte frames and reads to match the PSoC host driver's current chunk sizes. Delays larger than 65,535 us are split into smaller port calls for compatibility with delay APIs whose argument is 16-bit. Override macros before including `platform.h`, or through compiler definitions:

* `VL53L7CX\_BRIDGE\_MAX\_FRAME\_SIZE`: 4 to 256 bytes, limited further by the host driver's transmit buffer.
* `VL53L7CX\_BRIDGE\_MAX\_READ\_SIZE`: 1 to 255 bytes, limited further by the host driver's receive buffer.
* `VL53L7CX\_BRIDGE\_WRITE\_DELAY\_BASE\_US`
* `VL53L7CX\_BRIDGE\_WRITE\_DELAY\_PER\_BYTE\_US`
* `VL53L7CX\_BRIDGE\_READ\_PREFETCH\_BASE\_US`
* `VL53L7CX\_BRIDGE\_READ\_PREFETCH\_PER\_BYTE\_US`

Do not advertise or configure a transfer size larger than the MCU driver can send as one transaction. The PSoC host implementation currently uses 31-byte maximum write frames and 31-byte reads. Many classic Arduino `Wire` implementations are limited to 32 bytes, so configure a maximum complete frame of 32 bytes and a compatible read size on those targets.

The library does not retry failed writes because repeating a partially accepted register write may not be safe. Implement retries in the MCU adapter only when its driver can distinguish address NACK/no-transfer from a partially completed transaction.
