/* 文件名: <header文件名>
 * 描述: <简短描述当前头文件功能>
 *
 * 边界:
 * 1. 该头文件仅供 <模块/库> 使用。
 * 2. 只包含外部需要的函数声明和宏定义。
 */

/* 头文件防护
 * **************************************************************/
#ifndef CH395CMD__H
#define CH395CMD__H

/* Private includes
 * **************************************************************/
/* 可以根据需要在此处包含必要的库或其他头文件 */
/* #include "some_other_header.h" */

#include "bsp_delay.h"
#include "ch395hw.h"
#include <stdint.h>

/* Public macros
 * **************************************************************/
/* 宏定义，用于模块的配置和常量 */

/* Public typedefs
 * **************************************************************/
/* 根据需要定义内部类型 */

/* Public variables
 * **************************************************************/
/* 仅在文件内使用的静态变量 */

/* Public function declarations
 * **************************************************************/
/* 对外提供的API接口声明 */

uint8_t CH395CMDGetVer(void); /* Obtain the chip and firmware version number */

void CH395CMDEnterSleep(void); /* Enter low-power sleep suspended state */

void CH395CMDSetUartBaudRate(
    uint32_t baudrate); /* Set the baudrate for serial port communication */

void CH395CMDReset(void); /* Reset */

uint8_t CH395CMDCheckExist(
    uint8_t testdata); /* Test communication interface and working condition */

uint16_t
CH395CMDGetGlobIntStatus(void); /* Gets the CH395 global interrupt status */

void CH395CMDSetPHY(uint8_t PhyMode); /* Set the PHY mode */

void CH395CMDSetMACAddr(uint8_t *macaddr); /* Set MAC address */

void CH395CMDSetIPAddr(uint8_t *ipaddr); /* Set IP address */

void CH395CMDSetGWIPAddr(uint8_t *gwipaddr); /* Set GWIP address */

void CH395CMDSetMASKAddr(uint8_t *maskaddr); /* Set MASK address */

void CH395CMDSetMACFilt(uint8_t filtype, uint32_t table0,
                        uint32_t table1); /* Set MAC filtering. */

uint8_t CH395CMDGetPHYStatus(void); /* Gets the current PHY status */

uint8_t CH395CMDInitCH395(void); /* Initialize CH395 */

void CH395CMDGetUnreachIPPT(
    uint8_t *list); /* Get unreachable information (IP,Port,Protocol Type) */

void CH395CMDSetRetranCount(uint8_t time); /* Set retry times */

void CH395CMDSetRetranPeriod(uint16_t period); /* Set retry period */

uint8_t CH395CMDGetCmdStatus(void); /* Obtain the command execution status */

void CH395CMDGetRemoteIPP(
    uint8_t sockindex,
    uint8_t *list); /*Obtain the port and IP address of the remote end */

void CH395CMDClearRecvBuf(uint8_t sockindex); /* Clear receive buffer  */

uint16_t
CH395CMDGetSocketStatus(uint8_t sockindex); /* Obtain the socket n status */

uint8_t CH395CMDGetSocketInt(
    uint8_t sockindex); /* Gets the interrupt status of socket n  */

void CH395CMDSetSocketDesIP(
    uint8_t sockindex,
    uint8_t *ipaddr); /* Set the destination IP address of socket n  */

void CH395CMDSetSocketDesPort(
    uint8_t sockindex,
    uint16_t desport); /* Set the destination port of socket n */

void CH395CMDSetSocketSourPort(
    uint8_t sockindex, uint16_t sorport); /* Set the source port of socket n */

void CH395CMDSetSocketProtType(
    uint8_t sockindex,
    uint8_t prottype); /* Set the protocol type of socket n  */

uint8_t CH395CMDOpenSocket(uint8_t sockindex); /* Open socket n */

uint8_t CH395CMDTCPListen(uint8_t sockindex); /* TCP Listen */

uint8_t CH395CMDTCPConnect(uint8_t sockindex); /* TCP Connect */

uint8_t CH395CMDTCPDisconnect(uint8_t sockindex); /* TCP Disconnect */

void CH395CMDSendData(uint8_t sockindex, uint8_t *databuf,
                      uint16_t len); /* Writes data to socket n buffer */

uint16_t CH395CMDGetRecvLength(
    uint8_t sockindex); /* Gets the length of data received by socket n */

void CH395CMDGetRecvData(uint8_t sockindex, uint16_t len,
                         uint8_t *pbuf); /* Gets socket n receive buffer data */

uint8_t CH395CMDCloseSocket(uint8_t sockindex); /* Close socket n*/

void CH395CMDSetSocketIPRAWProto(
    uint8_t sockindex,
    uint8_t
        prototype); /* In IP mode, configure the IP packet protocol field. */

void CH395CMDEnablePing(uint8_t enable); /* On/off PING*/

void CH395CMDGetMACAddr(uint8_t *macaddr); /* Gets the MAC address */

uint8_t CH395CMDDHCPEnable(uint8_t flag); /* Enable DHCP */

uint8_t CH395CMDGetDHCPStatus(void); /* Obtain DHCP status */

void CH395CMDGetIPInf(uint8_t *addr); /* Get IP, subnet mask, gateway */

void CH395CMDSetARP(
    uint8_t period,
    uint8_t cnt); /* Set ARP retransmission period and number of times */

void CH395CMDSetTCPMSS(uint16_t mss); /* Set TCP MSS */

void CH395CMDSetTTLNum(uint8_t sockindex, uint8_t TTLnum); /* Set the TTL */

void CH395CMDSetSocketRecvBuf(
    uint8_t sockindex, uint8_t startblk,
    uint8_t blknum); /* Sets the SOCKET receive buffer */

void CH395CMDSetSocketSendBuf(uint8_t sockindex, uint8_t startblk,
                              uint8_t blknum); /* Sets the SOCKET send buffer */

void CH395CMDSetFunPapr(uint8_t PARA1, uint8_t PARA2, uint8_t PARA3,
                        uint8_t PARA4); /*Setup parameters*/

void CH395CMDKeepLiveIDLE(uint32_t idle); /* Set KEEPLIVE idle time */

void CH395CMDKeepLiveINTVL(uint32_t intvl); /* Set KEEPLIVE interval time */

void CH395CMDKeepLiveCNT(uint8_t cnt); /* Set KEEPLIVE retries times */

void CH395CMDSetKeepLive(uint8_t sockindex,
                         uint8_t cmd); /* Set the socket n keeplive function */

uint8_t CH395CMDEEPROMErase(void); /* Erasure EEPROM */

uint8_t CH395CMDEEPROMWrite(uint16_t eepaddr, uint8_t *buf,
                            uint8_t len); /* Write EEPROM */

void CH395CMDEEPROMRead(uint16_t eepaddr, uint8_t *buf,
                        uint8_t len); /* Read EEPROM */

uint8_t CH395CMDReadGPIOAddr(uint8_t regadd); /* Read GPIO register */

void CH395CMDWriteGPIOAddr(uint8_t regadd,
                           uint8_t regval); /* Write GPIO register */

void CH395UDPSendTo(uint8_t *buf, uint32_t len, uint8_t *ip, uint16_t port,
                    uint8_t sockindex); /*UDP mode Send to assign IP */

// uint8_t CH395SetUartBaudRate(uint32_t baudrate); /* Set 395 Uart BaudRate */

/* 头文件结束
 * **************************************************************/
#endif /* <CH395CMD>_H */
