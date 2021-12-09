/*----------------------------------------------------------------------------
 * Name:    Serial.c
 * Purpose: MCB1700 Low level serial functions
 * Note(s):
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2008 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include <LPC17xx.H>
#include "Serial.h"

#define IER_RBR 0x01
#define IER_THRE 0x02
#define IER_RLS 0x04

#define IIR_PEND 0x01
#define IIR_RLS 0x03
#define IIR_RDA 0x02
#define IIR_CTI 0x06
#define IIR_THRE 0x01

#define LSR_RDR 0x01
#define LSR_OE 0x02
#define LSR_PE 0x04
#define LSR_FE 0x08
#define LSR_BI 0x10
#define LSR_THRE 0x20
#define LSR_TEMT 0x40
#define LSR_RXFE 0x80

volatile uint32_t UART2_Status;
volatile uint8_t UART2_TxEmpty = 1;
volatile uint8_t UART2_Buffer[BUFSIZE];
volatile uint32_t UART2_Count = 0;

/*----------------------------------------------------------------------------
  Initialize UART pins, Baudrate
 *----------------------------------------------------------------------------*/
void SER_init(int uart_port, int baudrate)
{
  uint32_t Fdiv;
  uint32_t pclkdiv, pclk;
  LPC_UART_TypeDef *pUart;

  // (1)
  if (uart_port == 0)
  {                                           // UART0
    LPC_PINCON->PINSEL0 |= (1 << 4);          // Pin P0.2 used as TXD0 (Com0)
    LPC_PINCON->PINSEL0 |= (1 << 6);          // Pin P0.3 used as RXD0 (Com0)
    pclkdiv = (LPC_SC->PCLKSEL0 >> 6) & 0x03; // Bits 7:6 are for UART0; see page 57 of the user manual!
    pUart = (LPC_UART_TypeDef *)LPC_UART0;
  }
  else if (uart_port == 2)
  { // UART2
    // power up the UART2 peripheral;
    // Note: this is the only difference compared to the other branch of
    // this if, i.e., the case of UART0; here we need to power-up the UART2
    // because (as described on page 307 of the user manual) only UART0 is
    // powered by default! so, to be able to use UART2, we need to set to
    // "1" the bit index 24 of the PCONP register, as discussed on page
    // 64 of the user manual!
    LPC_SC->PCONP |= (1 << 24);
    // LPC_PINCON->PINSEL0 |= (1 << 20);          // Pin P0.10 used as TXD1 (Com2)
    // LPC_PINCON->PINSEL0 |= (1 << 22);          // Pin P0.11 used as RXD1 (Com2)
    LPC_PINCON->PINSEL4 |= (1 << 17);          // Pin P2.8 used as TXD1 (Com2)
    LPC_PINCON->PINSEL4 |= (1 << 19);          // Pin P2.9 used as RXD1 (Com2)
    pclkdiv = (LPC_SC->PCLKSEL1 >> 16) & 0x03; // Bits 17:16 of PCLKSEL1 are for UART2; see page 58 of the user manual!
    pUart = (LPC_UART_TypeDef *)LPC_UART2;

    // attach interrupt for UART2
    NVIC_EnableIRQ(UART2_IRQn);
    LPC_UART2->IER = IER_RBR | IER_THRE | IER_RLS; /* Enable UART2 interrupt */
  }
  else
  { // UART3
    // power up the UART3 peripheral;
    // Note: looking at the schematic diagram of the LandTiger board we see that
    // UART3 RX and TX are available as pin functions at the pins P0.0, P0.1
    // and pins P4.29, P4.28; also note that P0.0, P0.1 are wired on the board's
    // PCB to the pins CAN1_RX, CAN1_TX that go thru a small IC to the output connector
    // CAN 1; on the other hand, P4.29, P4.28 are wired on the board's PCB to the pins
    // 485_RX, 485_TX that go thru a small IC to the output connector RS485;
    // I guess, we could  use any of these two sets of pins to make direct connections
    // to UART3; let's use P4.29, P4.28;
    LPC_SC->PCONP |= (1 << 25);
    // LPC_PINCON->PINSEL9 |= (3 << 24);          // Pin P4.28 used as TXD3 (Com3); see page 120 of the user manual!
    // LPC_PINCON->PINSEL9 |= (3 << 26);          // Pin P4.29 used as RXD3 (Com3)
    LPC_PINCON->PINSEL0 |= (1 << 1);           // Pin P0.0 used as TXD3 (Com3)
    LPC_PINCON->PINSEL0 |= (1 << 3);           // Pin P0.1 used as RXD3 (Com3)
    pclkdiv = (LPC_SC->PCLKSEL1 >> 18) & 0x03; // Bits 19:18 of PCLKSEL1 are for UART3; see page 58 of the user manual!
    pUart = (LPC_UART_TypeDef *)LPC_UART3;
  }

  // (2)
  switch (pclkdiv)
  {
  case 0x00:
  default:
    pclk = SystemCoreClock / 4;
    break;
  case 0x01:
    pclk = SystemCoreClock;
    break;
  case 0x02:
    pclk = SystemCoreClock / 2;
    break;
  case 0x03:
    pclk = SystemCoreClock / 8;
    break;
  }

  // (3)
  pUart->LCR = 0x83;             // 8 bits, no Parity, 1 Stop bit
  Fdiv = (pclk / 16) / baudrate; // baud rate
  pUart->DLM = Fdiv / 256;
  pUart->DLL = Fdiv % 256;
  pUart->LCR = 0x03; // DLAB = 0
  pUart->FCR = 0x07; // Enable and reset TX and RX FIFO
}

/*----------------------------------------------------------------------------
  Write character to Serial Port
 *----------------------------------------------------------------------------*/
int SER_putChar(int uart, int c)
{
  LPC_UART_TypeDef *pUart;

  pUart = (uart == 0) ? (LPC_UART_TypeDef *)LPC_UART0 : (LPC_UART_TypeDef *)LPC_UART2;
  while (!(pUart->LSR & 0x20))
    ;
  return (pUart->THR = c);
}

/*----------------------------------------------------------------------------
  Read character from Serial Port   (blocking read)
 *----------------------------------------------------------------------------*/
int SER_getChar(int uart)
{
  LPC_UART_TypeDef *pUart;

  pUart = (uart == 0) ? (LPC_UART_TypeDef *)LPC_UART0 : (LPC_UART_TypeDef *)LPC_UART2;
  while (!(pUart->LSR & 0x01))
    ;
  return (pUart->RBR);
}

/*----------------------------------------------------------------------------
  Read character from Serial Port   (non blocking read)
 *----------------------------------------------------------------------------*/
int SER_getChar_nb(int uart)
{
  LPC_UART_TypeDef *pUart;

  pUart = (uart == 0) ? (LPC_UART_TypeDef *)LPC_UART0 : (LPC_UART_TypeDef *)LPC_UART2;
  if (pUart->LSR & 0x01)
    return (pUart->RBR);
  else
    return 0;
}

/*----------------------------------------------------------------------------
  Write character to Serial Port
 *----------------------------------------------------------------------------*/
void SER_putString(int uart, char *s)
{

  while (*s != 0)
  {
    SER_putChar(uart, *s++);
  }
}

void UART2_IRQHandler(void)
{
  uint8_t IIRValue, LSRValue;
  uint8_t Dummy = Dummy;

  IIRValue = LPC_UART2->IIR;

  IIRValue >>= 1;          /* skip pending bit in IIR */
  IIRValue &= 0x07;        /* check bit 1~3, interrupt identification */
  if (IIRValue == IIR_RLS) /* Receive Line Status */
  {
    LSRValue = LPC_UART2->LSR;
    /* Receive Line Status */
    if (LSRValue & (LSR_OE | LSR_PE | LSR_FE | LSR_RXFE | LSR_BI))
    {
      /* There are errors or break interrupt */
      /* Read LSR will clear the interrupt */
      UART2_Status = LSRValue;
      Dummy = LPC_UART2->RBR; /* Dummy read on RX to clear 
                                interrupt, then bail out */
      return;
    }
    if (LSRValue & LSR_RDR) /* Receive Data Ready */
    {
      /* If no error on RLS, normal ready, save into the data buffer. */
      /* Note: read RBR will clear the interrupt */
      UART2_Buffer[UART2_Count] = LPC_UART2->RBR;
      UART2_Count++;
      if (UART2_Count == BUFSIZE)
      {
        UART2_Count = 0; /* buffer overflow */
      }
    }
  }
  else if (IIRValue == IIR_RDA) /* Receive Data Available */
  {
    /* Receive Data Available */
    UART2_Buffer[UART2_Count] = LPC_UART2->RBR;
    UART2_Count++;
    if (UART2_Count == BUFSIZE)
    {
      UART2_Count = 0; /* buffer overflow */
    }
  }
  else if (IIRValue == IIR_CTI) /* Character timeout indicator */
  {
    /* Character Time-out indicator */
    UART2_Status |= 0x100; /* Bit 9 as the CTI error */
  }
  else if (IIRValue == IIR_THRE) /* THRE, transmit holding register empty */
  {
    /* THRE interrupt */
    LSRValue = LPC_UART2->LSR; /* Check status in the LSR to see if
                                valid data in U0THR or not */
    if (LSRValue & LSR_THRE)
    {
      UART2_TxEmpty = 1;
    }
    else
    {
      UART2_TxEmpty = 0;
    }
  }
}
