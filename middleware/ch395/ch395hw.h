/* 文件名: <header文件名>
 * 描述: <简短描述当前头文件功能>
 *
 * 边界:
 * 1. 该头文件仅供 <模块/库> 使用。
 * 2. 只包含外部需要的函数声明和宏定义。
 */

/* 头文件防护
 * **************************************************************/
#ifndef CH395HW__H
#define CH395HW__H

/* Private includes
 * **************************************************************/
/* 可以根据需要在此处包含必要的库或其他头文件 */
/* #include "some_other_header.h" */
#include <stdint.h>
/* Public macros
 * **************************************************************/
/* 宏定义，用于模块的配置和常量 */
#define PORT_NO 1

/* Public typedefs
 * **************************************************************/
/* 根据需要定义内部类型 */

/* Public variables
 * **************************************************************/
/* 仅在文件内使用的静态变量 */

/* Public function declarations
 * **************************************************************/
/* 对外提供的API接口声明 */

void ch395PinsInit(void);
void ch395Reset(void);
void xWriteCH395Cmd(uint8_t cmd);
void xWriteCH395Data(uint8_t mdata);
uint8_t xReadCH395Data(void);
void xEndCH395Cmd(void);
uint8_t Query395Interrupt(void);
void ch395_BRS(uint32_t baud);

/* 头文件结束
 * **************************************************************/
#endif /* <CH395HW>_H */
