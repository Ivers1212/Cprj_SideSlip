/* 文件名: <module_name>.c
 * 描述: <简要描述该模块的功能和用途>
 *
 * 边界:
 * 1. 该模块实现了 <模块功能描述>。
 * 2. 仅限于在当前模块内部使用的功能通过私有函数和变量进行封装。
 * 3. 对外提供公共API接口供其他模块调用。
 */

/* Private includes
 * **************************************************************/
#include "ch395cmd.h" // 包含与本文件相关的头文件，暴露公共接口
#include "bsp_delay.h"
#include "ch395.h"
#include "ch395hw.h"
#include <stdbool.h> // 布尔类型
#include <stdint.h>  // 标准整数类型

/* Private define
 * **************************************************************/
/* 宏定义 - 根据需求设置 */

/* Private typedef
 * **************************************************************/
/* 可用于内部的结构体或类型定义 */

/* Private variable
 * **************************************************************/
/* 文件内私有变量，使用 static 限制作用域 */

/* Private function prototypes
 * **************************************************************/
/* 声明本模块内部函数，不对外公开 */

/* Private function definitions
 * **************************************************************/
/* 私有函数定义，模块内部使用 */

/* Public APIs
 * **************************************************************/
/* 对外提供的公共API接口函数 */

/********************************************************************************
 * Function Name  : CH395CMDGetVer
 * Description    : Obtain the chip and firmware version number
 * Input          : None
 * Output         : None
 * Return         : Version number
 *******************************************************************************/
uint8_t CH395CMDGetVer(void) {
  uint8_t i = 0;
  xWriteCH395Cmd(CMD01_GET_IC_VER);
  i = xReadCH395Data();
  xEndCH395Cmd();
  return i;
}

/********************************************************************************
 * Function Name  : CH395CMDSetUartBaudRate
 * Description    : Set the baudrate for serial port communication
 * Input          : baudrate
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395CMDSetUartBaudRate(uint32_t baudrate) {
  xWriteCH395Cmd(CMD31_SET_BAUDRATE);
  xWriteCH395Data((uint8_t)baudrate);
  xWriteCH395Data((uint8_t)((uint16_t)baudrate >> 8));
  xWriteCH395Data((uint8_t)(baudrate >> 16));
  xEndCH395Cmd();
}

/********************************************************************************
 * Function Name  : CH395CMDEnterSleep
 * Description    : Enter low-power sleep suspended state
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395CMDEnterSleep(void) {
  xWriteCH395Cmd(CMD00_ENTER_SLEEP);
  xEndCH395Cmd();
}

/********************************************************************************
 * Function Name  : CH395CMDReset
 * Description    : Reset the CH395
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395CMDReset(void) {
  xWriteCH395Cmd(CMD00_RESET_ALL);
  xEndCH395Cmd();
}

/********************************************************************************
 * Function Name  : CH395CMDCheckExist
 * Description    : Test communication interface and working condition
 * Input          : testdata
 * Output         : None
 * Return         : bitwise inverse of input data
 *******************************************************************************/
uint8_t CH395CMDCheckExist(uint8_t testdata) {
  uint8_t i;
  xWriteCH395Cmd(CMD11_CHECK_EXIST);
  xWriteCH395Data(testdata);
  i = xReadCH395Data();
  xEndCH395Cmd();
  return i;
}

/*******************************************************************************
 * Function Name  : CH395CMDGetGlobIntStatus
 * Description    : Gets the CH395 global interrupt status
 * Input          : None
 * Output         : None
 * Return         : Global interrupt status
 *******************************************************************************/
uint16_t CH395CMDGetGlobIntStatus(void) {
  uint16_t init_status;
  xWriteCH395Cmd(CMD02_GET_GLOB_INT_STATUS_ALL);
  init_status = xReadCH395Data();
  init_status |= (uint16_t)xReadCH395Data() << 8;
  xEndCH395Cmd();
  return init_status;
}

/*******************************************************************************
 * Function Name  : CH395CMDSetPHY
 * Description    : Set the PHY mode
 * Input          : PHY mode
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395CMDSetPHY(uint8_t PhyMode) {
  xWriteCH395Cmd(CMD10_SET_PHY);
  xWriteCH395Data(PhyMode);
  xEndCH395Cmd();
}

/********************************************************************************
 * Function Name  : CH395CMDSetMACAddr
 * Description    : Set MAC address
 * Input          : macaddr
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395CMDSetMACAddr(uint8_t *macaddr) {
  uint8_t i;
  xWriteCH395Cmd(CMD60_SET_MAC_ADDR);
  for (i = 0; i < 6; i++)
    xWriteCH395Data(*macaddr++);
  xEndCH395Cmd();
}

/********************************************************************************
 * Function Name  : CH395CMDSetIPAddr
 * Description    : Set IP address
 * Input          : ipaddr
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395CMDSetIPAddr(uint8_t *ipaddr) {
  uint8_t i;
  xWriteCH395Cmd(CMD40_SET_IP_ADDR);
  for (i = 0; i < 4; i++)
    xWriteCH395Data(*ipaddr++);
  xEndCH395Cmd();
}

/********************************************************************************
 * Function Name  : CH395CMDSetGWIPAddr
 * Description    : Set GWIP address
 * Input          : ipaddr
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395CMDSetGWIPAddr(uint8_t *gwipaddr) {
  uint8_t i;
  xWriteCH395Cmd(CMD40_SET_GWIP_ADDR);
  for (i = 0; i < 4; i++)
    xWriteCH395Data(*gwipaddr++);
  xEndCH395Cmd();
}

/********************************************************************************
 * Function Name  : CH395CMDSetMASKAddr
 * Description    : Set MASK address ,The default is 255.255.255.0
 * Input          : maskaddr
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395CMDSetMASKAddr(uint8_t *maskaddr) {
  uint8_t i;
  xWriteCH395Cmd(CMD40_SET_MASK_ADDR);
  for (i = 0; i < 4; i++)
    xWriteCH395Data(*maskaddr++);
  xEndCH395Cmd();
}

/*******************************************************************************
 * Function Name  : CH395CMDSetMACFilt
 * Description    : Set MAC filtering
 * Input          : filtype     - Preference of Filtering
                    table0      - Hash0
                    table1      - Hash1
 * Output         : None
 * Return         : None
*******************************************************************************/
void CH395CMDSetMACFilt(uint8_t filtype, uint32_t table0, uint32_t table1) {
  xWriteCH395Cmd(CMD90_SET_MAC_FILT);
  xWriteCH395Data(filtype);
  xWriteCH395Data((uint8_t)table0);
  xWriteCH395Data((uint8_t)((uint16_t)table0 >> 8));
  xWriteCH395Data((uint8_t)(table0 >> 16));
  xWriteCH395Data((uint8_t)(table0 >> 24));

  xWriteCH395Data((uint8_t)table1);
  xWriteCH395Data((uint8_t)((uint16_t)table1 >> 8));
  xWriteCH395Data((uint8_t)(table1 >> 16));
  xWriteCH395Data((uint8_t)(table1 >> 24));
  xEndCH395Cmd();
}

/*******************************************************************************
 * Function Name  : CH395CMDGetPHYStatus
 * Description    : Gets the current PHY status
 * Input          : None
 * Output         : None
 * Return         : Current PHY status. See PHY Parameter definition for the
 * status definition
 *******************************************************************************/
uint8_t CH395CMDGetPHYStatus(void) {
  uint8_t i;
  xWriteCH395Cmd(CMD01_GET_PHY_STATUS);
  i = xReadCH395Data();
  xEndCH395Cmd();
  return i;
}

/********************************************************************************
 * Function Name  : CH395CMDInitCH395
 * Description    : Initialize CH395
 * Input          : None
 * Output         : None
 * Return         : uint8_t s
 *******************************************************************************/
uint8_t CH395CMDInitCH395(void) {
  uint8_t i = 0;
  uint8_t s = 0;
  xWriteCH395Cmd(CMD0W_INIT_CH395);
  xEndCH395Cmd();
  while (1) {
    delay_ms(20);               /* Delay query, more than 20MS is recommended*/
    s = CH395CMDGetCmdStatus(); /* Do not query too frequently*/
    if (s != CH395_ERR_BUSY)
      break;
    if (i++ > 200)
      return CH395_ERR_UNKNOW; /* Timeout exits. This function needs more than
                                  400MS to complete */
  }
  return s;
}

/********************************************************************************
 * Function Name  : CH395CMDGetUnreachIPPT
 * Description    : Get unreachable information (IP,Port,Protocol Type)
 * Input          : list
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395CMDGetUnreachIPPT(uint8_t *list) {
  uint8_t i;
  xWriteCH395Cmd(CMD08_GET_UNREACH_IPPORT);
  for (i = 0; i < 8; i++) {
    *list++ = xReadCH395Data();
  }
  xEndCH395Cmd();
}

/********************************************************************************
 * Function Name  : CH395CMDSetRetranCount
 * Description    : Set retry times
 * Input          : retry times
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395CMDSetRetranCount(uint8_t time) {
  xWriteCH395Cmd(CMD10_SET_RETRAN_COUNT);
  xWriteCH395Data(time);
  xEndCH395Cmd();
}

/********************************************************************************
 * Function Name  : CH395CMDSetRetranPeriod
 * Description    : Set retry period
 * Input          : retry period
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395CMDSetRetranPeriod(uint16_t period) {
  xWriteCH395Cmd(CMD20_SET_RETRAN_PERIOD);
  xWriteCH395Data((uint8_t)period);
  xWriteCH395Data((uint8_t)(period >> 8));
  xEndCH395Cmd();
}

/*******************************************************************************
 * Function Name  : CH395GetCmdStatus
 * Description    : Obtain the command execution status
 * Input          : None
 * Output         : None
 * Return         : Command execution status
 *******************************************************************************/
uint8_t CH395CMDGetCmdStatus(void) {
  uint8_t i;
  xWriteCH395Cmd(CMD01_GET_CMD_STATUS);
  i = xReadCH395Data();
  xEndCH395Cmd();
  return i;
}

/********************************************************************************
 * Function Name  : CH395CMDGetRemoteIPP
 * Description    : Obtain the port and IP address of the remote end
 * Input          : sockindex      - sockindex
                    list           - Save the IP address and port number
 * Output         : None
 * Return         : None
*******************************************************************************/
void CH395CMDGetRemoteIPP(uint8_t sockindex, uint8_t *list) {
  uint8_t i;

  xWriteCH395Cmd(CMD06_GET_REMOT_IPP_SN);
  xWriteCH395Data(sockindex);
  for (i = 0; i < 6; i++) {
    *list++ = xReadCH395Data();
  }
  xEndCH395Cmd();
}

/*******************************************************************************
 * Function Name  : CH395ClearRecvBuf
 * Description    : Clear receive buffer
 * Input          : sockindex
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395CMDClearRecvBuf(uint8_t sockindex) {
  xWriteCH395Cmd(CMD10_CLEAR_RECV_BUF_SN);
  xWriteCH395Data((uint8_t)sockindex);
  xEndCH395Cmd();
}

/********************************************************************************
 * Function Name  : CH395CMDGetSocketStatus
 * Description    : Obtain the socket n status
 * Input          : sockindex
 * Output         : None
 * Return         : First byte     - socket n open or closed
                    Second byte    - TCP state, meaningful only if TCP mode and
the first byte is open
*******************************************************************************/
uint16_t CH395CMDGetSocketStatus(uint8_t sockindex) {
  uint16_t status;
  xWriteCH395Cmd(CMD12_GET_SOCKET_STATUS_SN);
  xWriteCH395Data(sockindex);
  status = xReadCH395Data() << 8;
  status |= xReadCH395Data();
  xEndCH395Cmd();
  return status;
}

/*******************************************************************************
 * Function Name  : CH395GetSocketInt
 * Description    : Gets the interrupt status of socket n
 * Input          : sockindex
 * Output         : None
 * Return         : socket interrupt status
 *******************************************************************************/
uint8_t CH395CMDGetSocketInt(uint8_t sockindex) {
  uint8_t intstatus;
  xWriteCH395Cmd(CMD11_GET_INT_STATUS_SN);
  xWriteCH395Data(sockindex);
  /*In between sending and receiving bytes, a TSC time delay is required.*/
  delay_us(1);
  intstatus = xReadCH395Data();
  xEndCH395Cmd();
  return intstatus;
}

/*******************************************************************************
 * Function Name  : CH395SetSocketDesIP
 * Description    : Set the destination IP address of socket n
 * Input          : sockindex     - socket index
                    ipaddr        - destination IP address
 * Output         : None
 * Return         : None
*******************************************************************************/
void CH395CMDSetSocketDesIP(uint8_t sockindex, uint8_t *ipaddr) {
  xWriteCH395Cmd(CMD50_SET_IP_ADDR_SN);
  xWriteCH395Data(sockindex);
  xWriteCH395Data(*ipaddr++);
  xWriteCH395Data(*ipaddr++);
  xWriteCH395Data(*ipaddr++);
  xWriteCH395Data(*ipaddr++);
  xEndCH395Cmd();
}

/*******************************************************************************
 * Function Name  : CH395SetSocketDesPort
 * Description    : Set the destination port of socket n
 * Input          : sockindex   - socket index
                    desport     - destination port
 * Output         : None
 * Return         : None
*******************************************************************************/
void CH395CMDSetSocketDesPort(uint8_t sockindex, uint16_t desport) {
  xWriteCH395Cmd(CMD30_SET_DES_PORT_SN);
  xWriteCH395Data(sockindex);
  xWriteCH395Data((uint8_t)desport);
  xWriteCH395Data((uint8_t)(desport >> 8));
  xEndCH395Cmd();
}

/*******************************************************************************
 * Function Name  : CH395SetSocketSourPort
 * Description    : Set the source port of socket n
 * Input          : sockindex     - socket index
                    sorport       - source port
 * Output         : None
 * Return         : None
*******************************************************************************/
void CH395CMDSetSocketSourPort(uint8_t sockindex, uint16_t sorport) {
  xWriteCH395Cmd(CMD30_SET_SOUR_PORT_SN);
  xWriteCH395Data(sockindex);
  xWriteCH395Data((uint8_t)sorport);
  xWriteCH395Data((uint8_t)(sorport >> 8));
  xEndCH395Cmd();
}

/*******************************************************************************
 * Function Name  : CH395SetSocketProtType
 * Description    : Set the protocol type of socket n
 * Input          : sockindex   - socket index
                    prottype    - protocol type
 * Output         : None
 * Return         : None
*******************************************************************************/
void CH395CMDSetSocketProtType(uint8_t sockindex, uint8_t prottype) {
  xWriteCH395Cmd(CMD20_SET_PROTO_TYPE_SN);
  xWriteCH395Data(sockindex);
  xWriteCH395Data(prottype);
  xEndCH395Cmd();
}

/*******************************************************************************
 * Function Name  : CH395OpenSocket
 * Description    : Open socket n
 * Input          : sockindex
 * Output         : None
 * Return         : s
 *******************************************************************************/
uint8_t CH395CMDOpenSocket(uint8_t sockindex) {
  uint8_t i = 0;
  uint8_t s = 0;
  xWriteCH395Cmd(CMD1W_OPEN_SOCKET_SN);
  xWriteCH395Data(sockindex);
  xEndCH395Cmd();
  while (1) {
    delay_ms(20);               /* Delay query, more than 20MS is recommended*/
    s = CH395CMDGetCmdStatus(); /* Do not query too frequently*/
    if (s != CH395_ERR_BUSY)
      break;
    if (i++ > 200)
      return CH395_ERR_UNKNOW;
  }
  return s;
}

/******************************************************************************
 * Function Name  : CH395TCPListen
 * Description    : socket n listens, After receiving this command, socket n
 * enters server mode, valid only for TCP mode Input          : sockindex Output
 *         : None Return         : s
 *******************************************************************************/
uint8_t CH395CMDTCPListen(uint8_t sockindex) {
  uint8_t i = 0;
  uint8_t s = 0;
  xWriteCH395Cmd(CMD1W_TCP_LISTEN_SN);
  xWriteCH395Data(sockindex);
  xEndCH395Cmd();
  while (1) {
    delay_ms(20);               /* Delay query, more than 20MS is recommended*/
    s = CH395CMDGetCmdStatus(); /* Do not query too frequently*/
    if (s != CH395_ERR_BUSY)
      break;
    if (i++ > 200)
      return CH395_ERR_UNKNOW;
  }
  return s;
}

/********************************************************************************
 * Function Name  : CH395TCPConnect
 * Description    : socket n connection. After receiving this command, socket n
 * enters the client mode, valid only for TCP mode Input          : sockindex
 * Output         : None
 * Return         : s
 *******************************************************************************/
uint8_t CH395CMDTCPConnect(uint8_t sockindex) {
  uint8_t i = 0;
  uint8_t s = 0;
  xWriteCH395Cmd(CMD1W_TCP_CONNECT_SN);
  xWriteCH395Data(sockindex);
  xEndCH395Cmd();
  while (1) {
    delay_ms(20);               /* Delay query, more than 20MS is recommended*/
    s = CH395CMDGetCmdStatus(); /* Do not query too frequently*/
    if (s != CH395_ERR_BUSY)
      break;
    if (i++ > 200)
      return CH395_ERR_UNKNOW;
  }
  return s;
}

/********************************************************************************
 * Function Name  : CH395CMDTCPDisconnect
 * Description    : socket n disconnection
 * Input          : sockindex
 * Output         : None
 * Return         : s
 *******************************************************************************/
uint8_t CH395CMDTCPDisconnect(uint8_t sockindex) {
  uint8_t i = 0;
  uint8_t s = 0;
  xWriteCH395Cmd(CMD1W_TCP_DISCONNECT_SN);
  xWriteCH395Data(sockindex);
  xEndCH395Cmd();
  while (1) {
    delay_ms(20);               /* Delay query, more than 20MS is recommended*/
    s = CH395CMDGetCmdStatus(); /* Do not query too frequently*/
    if (s != CH395_ERR_BUSY)
      break;
    if (i++ > 200)
      return CH395_ERR_UNKNOW;
  }
  return s;
}

/********************************************************************************
 * Function Name  : CH395CMDSendData
 * Description    : Writes data to socket n buffer
 * Input          : sockindex    - socket index
                    data buf     - data buf
                    len          - data length
 * Output         : None
 * Return         : None
*******************************************************************************/
void CH395CMDSendData(uint8_t sockindex, uint8_t *databuf, uint16_t len) {
#if (CH395_SPI_DMA_ENABLE == DISABLE)
  uint16_t i;
  xWriteCH395Cmd(CMD30_WRITE_SEND_BUF_SN);
  xWriteCH395Data((uint8_t)sockindex);
  xWriteCH395Data((uint8_t)len);
  xWriteCH395Data((uint8_t)(len >> 8));
  for (i = 0; i < len; i++) {
    xWriteCH395Data(*databuf++);
  }
  xEndCH395Cmd();
#else
  if (!len)
    return;
  xWriteCH395Cmd(CMD30_WRITE_SEND_BUF_SN);
  xWriteCH395Data((u8)sockindex);
  xWriteCH395Data((u8)len);
  xWriteCH395Data((u8)(len >> 8));

  delay_us(1);

  SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
  SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, ENABLE);
  DMA_Tx_Init(DMA1_Channel3, (u32)&SPI1->DR, (u32)databuf, len);
  DMA_Rx_Init(DMA1_Channel2, (u32)&SPI1->DR, (u32)databuf, len);
  DMA_Cmd(DMA1_Channel2, ENABLE);
  DMA_Cmd(DMA1_Channel3, ENABLE);
  while (!DMA_GetFlagStatus(DMA1_FLAG_TC3) ||
         !DMA_GetFlagStatus(DMA1_FLAG_TC2)) {
    if (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == RESET)
      break;
  }
  DMA_ClearFlag(DMA1_FLAG_TC3);
  DMA_ClearFlag(DMA1_FLAG_TC2);

  SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, DISABLE);
  SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, DISABLE);
  DMA_Cmd(DMA1_Channel2, DISABLE);
  DMA_Cmd(DMA1_Channel3, DISABLE);
  xEndCH395Cmd();
#endif
}

/*******************************************************************************
 * Function Name  : CH395CMDGetRecvLength
 * Description    : Gets the length of data received by socket n
 * Input          : socket index
 * Output         : None
 * Return         : 2 bytes receive length
 *******************************************************************************/
uint16_t CH395CMDGetRecvLength(uint8_t sockindex) {
  uint16_t i;

  xWriteCH395Cmd(CMD12_GET_RECV_LEN_SN);
  xWriteCH395Data((uint8_t)sockindex);
  /*In between sending and receiving bytes, a TSC time delay is required.*/
  delay_us(1);
  i = xReadCH395Data();
  i = (uint16_t)(xReadCH395Data() << 8) + i;
  xEndCH395Cmd();
  return i;
}

/********************************************************************************
 * Function Name  : CH395CMDGetRecvData
 * Description    : Gets socket n receive buffer data
 * Input          : sockindex    - socket index
                    len          - data length
                    pbuf         - data buf
 * Output         : None
 * Return         : None
*******************************************************************************/
void CH395CMDGetRecvData(uint8_t sockindex, uint16_t len, uint8_t *pbuf) {
#if (CH395_SPI_DMA_ENABLE == 0)
  uint16_t i;
  if (!len)
    return;
  xWriteCH395Cmd(CMD30_READ_RECV_BUF_SN);
  xWriteCH395Data(sockindex);
  xWriteCH395Data((uint8_t)len);
  xWriteCH395Data((uint8_t)(len >> 8));
  for (i = 0; i < len; i++) {
    *pbuf = xReadCH395Data();
    pbuf++;
  }
  xEndCH395Cmd();

#else
  if (!len)
    return;
  xWriteCH395Cmd(CMD30_READ_RECV_BUF_SN);
  xWriteCH395Data(sockindex);
  xWriteCH395Data((u8)len);
  xWriteCH395Data((u8)(len >> 8));

  SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
  SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, ENABLE);
  DMA_Tx_Init(DMA1_Channel3, (u32)&SPI1->DR, (u32)pbuf, len);
  DMA_Rx_Init(DMA1_Channel2, (u32)&SPI1->DR, (u32)pbuf, len);
  DMA_Cmd(DMA1_Channel2, ENABLE);
  DMA_Cmd(DMA1_Channel3, ENABLE);
  while (!DMA_GetFlagStatus(DMA1_FLAG_TC3) ||
         !DMA_GetFlagStatus(DMA1_FLAG_TC2)) {
    if (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == RESET)
      break;
  }
  DMA_ClearFlag(DMA1_FLAG_TC3);
  DMA_ClearFlag(DMA1_FLAG_TC2);

  SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, DISABLE);
  SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, DISABLE);
  DMA_Cmd(DMA1_Channel2, DISABLE);
  DMA_Cmd(DMA1_Channel3, DISABLE);
  xEndCH395Cmd();
#endif
}

/*******************************************************************************
 * Function Name  : CH395CMDCloseSocket
 * Description    : Close socket n
 * Input          : sockindex
 * Output         : None
 * Return         : s
 *******************************************************************************/
uint8_t CH395CMDCloseSocket(uint8_t sockindex) {
  uint8_t i = 0;
  uint8_t s = 0;
  xWriteCH395Cmd(CMD1W_CLOSE_SOCKET_SN);
  xWriteCH395Data(sockindex);
  xEndCH395Cmd();
  while (1) {
    delay_ms(20);               /* Delay query, more than 20MS is recommended*/
    s = CH395CMDGetCmdStatus(); /* Do not query too frequently*/
    if (s != CH395_ERR_BUSY)
      break;
    if (i++ > 200)
      return CH395_ERR_UNKNOW;
  }
  return s;
}

/******************************************************************************
 * Function Name  : CH395SetSocketIPRAWProto
 * Description    : In IP mode, configure the IP packet protocol field.
 * Input          : sockindex     - SocketIndex
                    prototype     - 1 byte protocol field
 * Output         : None
 * Return         : None
*******************************************************************************/
void CH395CMDSetSocketIPRAWProto(uint8_t sockindex, uint8_t prototype) {
  xWriteCH395Cmd(CMD20_SET_IPRAW_PRO_SN);
  xWriteCH395Data(sockindex);
  xWriteCH395Data(prototype);
  xEndCH395Cmd();
}

/********************************************************************************
 * Function Name  : CH395EnablePing
 * Description    : On/off PING
 * Input          : 1 Enable PING ; 0  Disable PING
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395CMDEnablePing(uint8_t enable) {
  xWriteCH395Cmd(CMD10_PING_ENABLE);
  xWriteCH395Data(enable);
  xEndCH395Cmd();
}

/********************************************************************************
 * Function Name  : CH395CMDGetMACAddr
 * Description    : Gets the MAC address
 * Input          : MAC address pointer
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395CMDGetMACAddr(uint8_t *macaddr) {
  uint8_t i;
  xWriteCH395Cmd(CMD06_GET_MAC_ADDR);
  for (i = 0; i < 6; i++)
    *macaddr++ = xReadCH395Data();
  xEndCH395Cmd();
}

/******************************************************************************
 * Function Name  : CH395DHCPEnable
 * Description    : Enable DHCP
 * Input          : flag : 1 enable DHCP, 0 disable DHCP
 * Output         : None
 * Return         : s
 *******************************************************************************/
uint8_t CH395CMDDHCPEnable(uint8_t flag) {
  uint8_t i = 0;
  uint8_t s;
  xWriteCH395Cmd(CMD10_DHCP_ENABLE);
  xWriteCH395Data(flag);
  xEndCH395Cmd();
  while (1) {
    delay_ms(20);               /* Delay query, more than 20MS is recommended*/
    s = CH395CMDGetCmdStatus(); /* Do not query too frequently*/
    if (s != CH395_ERR_BUSY)
      break;
    if (i++ > 200)
      return CH395_ERR_UNKNOW;
  }
  return s;
}

/******************************************************************************
 * Function Name  : CH395GetDHCPStatus
 * Description    : Obtain DHCP status
 * Input          : None
 * Output         : None
 * Return         : 1 byte status code, 0 indicates success, other values fail
 *******************************************************************************/
uint8_t CH395CMDGetDHCPStatus(void) {
  uint8_t status;
  xWriteCH395Cmd(CMD01_GET_DHCP_STATUS);
  status = xReadCH395Data();
  xEndCH395Cmd();
  return status;
}

/*******************************************************************************
 * Function Name  : CH395GetIPInf
 * Description    : Get IP, subnet mask, gateway
 * Input          : None
 * Output         : 20 bytes, respectively 4 bytes IP, 4 bytes gateway, 4 bytes
 * mask, 4 bytes DNS1, 4 bytes DNS2 Return         : None
 *******************************************************************************/
void CH395CMDGetIPInf(uint8_t *addr) {
  uint8_t i;
  xWriteCH395Cmd(CMD014_GET_IP_INF);
  for (i = 0; i < 20; i++) {
    *addr++ = xReadCH395Data();
  }
  xEndCH395Cmd();
}

/*******************************************************************************
 * Function Name  : CH395CMDSetARP
 * Description    : Set ARP retransmission period and number of times
 *  Input         : period   - 1 byte ARP retransmission period
                    cnt      - 1 byte ARP retransmission number
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395CMDSetARP(uint8_t period, uint8_t cnt) {
  xWriteCH395Cmd(CMD20_SET_ARP);
  xWriteCH395Data(period);
  xWriteCH395Data(cnt);
  xEndCH395Cmd();
}

/*******************************************************************************
 * Function Name  : CH395CMDSetTCPMSS
 * Description    : Set TCP MSS
 * Input          : mss
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395CMDSetTCPMSS(uint16_t mss) {
  xWriteCH395Cmd(CMD20_TCP_MSS);
  xWriteCH395Data((uint8_t)mss);
  xWriteCH395Data((uint8_t)(mss >> 8));
  xEndCH395Cmd();
}

/*******************************************************************************
 * Function Name  : CH395SetTTLNum
 * Description    : Set the TTL
 * Input          : sockindex   - SocketIndex
 *                  TTLnum      - 1 byte TTL
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395CMDSetTTLNum(uint8_t sockindex, uint8_t TTLnum) {
  xWriteCH395Cmd(CMD20_SET_TTL);
  xWriteCH395Data(sockindex);
  xWriteCH395Data(TTLnum);
  xEndCH395Cmd();
}

/*******************************************************************************
 * Function Name  : CH395SetSocketRecvBuf
 * Description    : Sets the SOCKET receive buffer
 * Input          : sockindex    - sockindex
                    startblk     - starting block index
                    blknum       - the number of blocks
 * Output         : None
 * Return         : None
*******************************************************************************/
void CH395CMDSetSocketRecvBuf(uint8_t sockindex, uint8_t startblk,
                              uint8_t blknum) {
  xWriteCH395Cmd(CMD30_SET_RECV_BUF);
  xWriteCH395Data(sockindex);
  xWriteCH395Data(startblk);
  xWriteCH395Data(blknum);
  xEndCH395Cmd();
}

/*******************************************************************************
 * Function Name  : CH395CMDSetSocketSendBuf
 * Description    : Sets the SOCKET send buffer
 * Input          : sockindex    - sockindex
                    startblk     - starting block index
                    blknum       - the number of blocks
 * Output         : None
 * Return         : None
*******************************************************************************/
void CH395CMDSetSocketSendBuf(uint8_t sockindex, uint8_t startblk,
                              uint8_t blknum) {
  xWriteCH395Cmd(CMD30_SET_SEND_BUF);
  xWriteCH395Data(sockindex);
  xWriteCH395Data(startblk);
  xWriteCH395Data(blknum);
  xEndCH395Cmd();
}

/*******************************************************************************
 * Function Name  : CH395CMDSetFunPapr
 * Description    : Sets function parameter
 * Input          : Four-byte function parameter
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395CMDSetFunPapr(uint8_t PARA1, uint8_t PARA2, uint8_t PARA3,
                        uint8_t PARA4) {
  xWriteCH395Cmd(CMD40_SET_FUN_PARA);
  xWriteCH395Data(PARA1);
  xWriteCH395Data(PARA2);
  xWriteCH395Data(PARA3);
  xWriteCH395Data(PARA4);
  xEndCH395Cmd();
}

/*******************************************************************************
 * Function Name  : CH395KeepLiveIDLE
 * Description    : Set KEEPLIVE idle time
 * Input          : idle time (ms)
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395CMDKeepLiveIDLE(uint32_t idle) {
  xWriteCH395Cmd(CMD40_SET_KEEP_LIVE_IDLE);
  xWriteCH395Data((uint8_t)idle);
  xWriteCH395Data((uint8_t)((uint16_t)idle >> 8));
  xWriteCH395Data((uint8_t)(idle >> 16));
  xWriteCH395Data((uint8_t)(idle >> 24));
  xEndCH395Cmd();
}

/*******************************************************************************
 * Function Name  : CH395KeepLiveINTVL
 * Description    : Set KEEPLIVE interval time
 * Input          : timeout interval (ms)
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395CMDKeepLiveINTVL(uint32_t intvl) {
  xWriteCH395Cmd(CMD40_SET_KEEP_LIVE_INTVL);
  xWriteCH395Data((uint8_t)intvl);
  xWriteCH395Data((uint8_t)((uint16_t)intvl >> 8));
  xWriteCH395Data((uint8_t)(intvl >> 16));
  xWriteCH395Data((uint8_t)(intvl >> 24));
  xEndCH395Cmd();
}

/*******************************************************************************
 * Function Name  : CH395KeepLiveCNT
 * Description    : Set KEEPLIVE retries times
 * Input          : retry times
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395CMDKeepLiveCNT(uint8_t cnt) {
  xWriteCH395Cmd(CMD10_SET_KEEP_LIVE_CNT);
  xWriteCH395Data(cnt);
  xEndCH395Cmd();
}

/*******************************************************************************
 * Function Name  : CH395SetKeepLive
 * Description    : Set the socket n keeplive function
 * Input          : sockindex   - sockindex
 *                  cmd         - 0: close 1:open
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395CMDSetKeepLive(uint8_t sockindex, uint8_t cmd) {
  xWriteCH395Cmd(CMD20_SET_KEEP_LIVE_SN);
  xWriteCH395Data(sockindex);
  xWriteCH395Data(cmd);
  xEndCH395Cmd();
}

/*******************************************************************************
 * Function Name  : CH395CMDEEPROMErase
 * Description    : EEPROM erase
 * Input          : None
 * Output         : None
 * Return         : executing state
 *******************************************************************************/
uint8_t CH395CMDEEPROMErase(void) {
  uint8_t i = 0, s = 0;
  xWriteCH395Cmd(CMD0W_EEPROM_ERASE);
  xEndCH395Cmd();
  while (1) {
    delay_ms(20);               /* Delay query, more than 20MS is recommended*/
    s = CH395CMDGetCmdStatus(); /* Do not query too frequently*/
    if (s != CH395_ERR_BUSY)
      break;
    if (i++ > 200)
      return CH395_ERR_UNKNOW;
  }
  return i;
}

/*******************************************************************************
 * Function Name  : CH395EEPROMWrite
 * Description    : Write EEPROM
 * Input          : eepaddr   - EEPROM addr
 *                ：buf       - buffer address
 *                ：len       - len
 * Output         : None
 * Return         : executing state
 *******************************************************************************/
uint8_t CH395CMDEEPROMWrite(uint16_t eepaddr, uint8_t *buf, uint8_t len) {
  uint8_t i = 0, s = 0;
  xWriteCH395Cmd(CMD30_EEPROM_WRITE);
  xWriteCH395Data((uint8_t)(eepaddr));
  xWriteCH395Data((uint8_t)(eepaddr >> 8));
  xWriteCH395Data(len);
  while (len--)
    xWriteCH395Data(*buf++);
  xEndCH395Cmd();
  while (1) {
    delay_ms(20);               /* Delay query, more than 20MS is recommended*/
    s = CH395CMDGetCmdStatus(); /* Do not query too frequently*/
    if (s != CH395_ERR_BUSY)
      break;
    if (i++ > 200)
      return CH395_ERR_UNKNOW;
  }
  return s;
}

/*******************************************************************************
 * Function Name  : CH395EEPROMRead
 * Description    : Read EEPROM
 * Input          : eepaddr   - EEPROM addr
 *                ：buf       - buffer address
 *                ：len       - len
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395CMDEEPROMRead(uint16_t eepaddr, uint8_t *buf, uint8_t len) {
  xWriteCH395Cmd(CMD30_EEPROM_READ);
  xWriteCH395Data((uint8_t)(eepaddr));
  xWriteCH395Data((uint8_t)(eepaddr >> 8));
  xWriteCH395Data(len);
  delay_us(30);
  while (len--)
    *buf++ = xReadCH395Data();
  xEndCH395Cmd();
}

/*******************************************************************************
 * Function Name  : CH395CMDReadGPIOAddr
 * Description    : Read GPIO register
 * Input          : register addr
 * Output         : None
 * Return         : register value
 *******************************************************************************/
uint8_t CH395CMDReadGPIOAddr(uint8_t regadd) {
  uint8_t i;
  xWriteCH395Cmd(CMD11_READ_GPIO_REG);
  xWriteCH395Data(regadd);
  i = xReadCH395Data();
  xEndCH395Cmd();
  return i;
}

/*******************************************************************************
 * Function Name  : CH395CMDWriteGPIOAddr
 * Description    : Write GPIO register
 * Input          : regadd    - register addr
 *                ：regval    - register value
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH395CMDWriteGPIOAddr(uint8_t regadd, uint8_t regval) {
  xWriteCH395Cmd(CMD20_WRITE_GPIO_REG);
  xWriteCH395Data(regadd);
  xWriteCH395Data(regval);
  xEndCH395Cmd();
}

/*******************************************************************************
 * Function Name  : CH395SetUartBaudRate
 * Description    : Set 395 Uart BaudRate
 * Input          : BaudRate
 * Output         : None
 * Return         : s
 *******************************************************************************/
// uint8_t CH395SetUartBaudRate(uint32_t baudrate) {
//   uint8_t s = CH395_ERR_UNKNOW;
// #if (CH395_OP_INTERFACE_MODE == CH395_UART_MODE)
//   CH395CMDSetUartBaudRate(baudrate); /* Set BaudRate */
//   delay_ms(1);
//   Set_MCU_BaudRate(baudrate);
//   s = xReadCH395Data(); /* If the setting is successful CH395 return
//                            CMD_ERR_SUCCESS */
//   if (s == CMD_ERR_SUCCESS)
//     printf("Set Success\r\n");
// #endif
//   return s;
// }

/*******************************************************************************
 * Function Name  : CH395UDPSendTo
 * Description    : UDP sends data to the specified IP address and port
 * Input          : buf     - Send data buffer
                    len     - Send data length
                    ip      - DES IP
                    port    - DES Port
                    sockeid - socket index
 * Output         : None
 * Return         : None
*******************************************************************************/
void CH395UDPSendTo(uint8_t *buf, uint32_t len, uint8_t *ip, uint16_t port,
                    uint8_t sockindex) {
  CH395CMDSetSocketDesIP(sockindex, ip);
  CH395CMDSetSocketDesPort(sockindex, port);
  CH395CMDSendData(sockindex, buf, len);
}
/* Weak hook override
 * **************************************************************/
/* 如果需要，用户可以在其他地方覆盖这些钩子函数 */
// __attribute__((weak)) void custom_callback(void) {
//   // 默认空实现，用户可以重写
// }
