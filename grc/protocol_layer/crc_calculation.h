#ifndef _CRC_CALCULATION_H_
#define _CRC_CALCULATION_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/*
  Name  : CRC-8
  Poly  : 0x31    x^8 + x^5 + x^4 + 1
  Init  : 0xFF
  Revert: false
  XorOut: 0x00
  Check : 0xF7 ("123456789")
  MaxLen: 15 bytes (127 bit) - detection of single, double, triple and all odd errors
*/
uint8_t Crc8(uint8_t* pcBlock, uint8_t len);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _CRC_CALCULATION_H_
