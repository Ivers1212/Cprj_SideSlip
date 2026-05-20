/* cmsis_version.h - minimal stub for Cortex-M3 bare-metal project */

#ifndef CMSIS_VERSION_H
#define CMSIS_VERSION_H

#include <stdint.h>

#define __CMSIS_VERSION_MAIN (0x05U) /* [31:16] main version */
#define __CMSIS_VERSION_SUB (0x00U)  /* [15:0]  sub version  */
#define __CMSIS_VERSION ((__CMSIS_VERSION_MAIN << 16U) | __CMSIS_VERSION_SUB)

typedef struct {
  uint16_t main; /*!< [31:16] main version */
  uint16_t sub;  /*!< [15:0]  sub version  */
} CMSIS_VERSION_t;

#endif /* CMSIS_VERSION_H */
