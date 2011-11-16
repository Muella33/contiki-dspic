
/* 
 * Microchip does not include stdint.h in PIC30 includes, define what we need
 */
#ifndef __STDINT_H__
#define __STDINT_H__

typedef          char         int8_t;
typedef unsigned char        uint8_t;
typedef          int         int16_t;
typedef unsigned int        uint16_t;
typedef          long        int32_t;
typedef unsigned long       uint32_t;

#endif /* __STDINT_H__ */
