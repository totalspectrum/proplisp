//
// automatically generated by spin2cpp v3.0.0 on Sat Apr 16 20:30:46 2016
// spin2cpp --ccode FullDuplexSerial.spin 
//

// ********************************************
// *  Full-Duplex Serial Driver v1.2          *
// *  Author: Chip Gracey, Jeff Martin        *
// *  Copyright (c) 2006-2009 Parallax, Inc.  *
// *  See end of file for terms of use.       *
// ********************************************
// modified by Eric Smith to strip it down to just
// rx and tx functions (C will handle the rest)
/* -----------------REVISION HISTORY-----------------
 v1.2 - 5/7/2009 fixed bug in dec method causing largest negative value (-2,147,483,648) to be output as -0.
 v1.1 - 3/1/2006 first official release.
 */
#include <propeller.h>
#include "FullDuplexSerial.h"

#ifdef __GNUC__
#define INLINE__ static inline
#define Yield__() __asm__ volatile( "" ::: "memory" )
#else
#define INLINE__ static
#define Yield__()
#define waitcnt(n) _waitcnt(n)
#define locknew() _locknew()
#define lockret(i) _lockret(i)
#define lockset(i) _lockset(i)
#define lockclr(i) _lockclr(i)
#define coginit(id, code, par) _coginit((unsigned)(par)>>2, (unsigned)(code)>>2, id)
#define cognew(code, par) coginit(0x8, (code), (par))
#define cogstop(i) _cogstop(i)
#endif

static uint8_t dat[] = {
  0xf0, 0xad, 0xbc, 0xa0, 0x10, 0xac, 0xfc, 0x80, 0x56, 0xae, 0xbc, 0x08, 0x01, 0xb6, 0xfc, 0xa0, 
  0x57, 0xb6, 0xbc, 0x2c, 0x04, 0xac, 0xfc, 0x80, 0x56, 0xae, 0xbc, 0x08, 0x01, 0xc2, 0xfc, 0xa0, 
  0x57, 0xc2, 0xbc, 0x2c, 0x04, 0xac, 0xfc, 0x80, 0x56, 0xb2, 0xbc, 0x08, 0x04, 0xac, 0xfc, 0x80, 
  0x56, 0xb4, 0xbc, 0x08, 0x04, 0xac, 0xfc, 0x80, 0x56, 0xb8, 0xbc, 0x08, 0x5c, 0xc4, 0xbc, 0xa0, 
  0x10, 0xc4, 0xfc, 0x80, 0x04, 0xb2, 0x7c, 0x62, 0x02, 0xb2, 0x7c, 0x61, 0x61, 0xe8, 0x9b, 0x68, 
  0x61, 0xec, 0xab, 0x68, 0x34, 0xcc, 0xfc, 0xa0, 0xf0, 0xab, 0x3c, 0x08, 0x66, 0xc0, 0xbc, 0x5c, 
  0x01, 0xb2, 0x7c, 0x62, 0xf2, 0xb7, 0x3c, 0x61, 0x17, 0x00, 0x64, 0x5c, 0x09, 0xbc, 0xfc, 0xa0, 
  0x5a, 0xbe, 0xbc, 0xa0, 0x01, 0xbe, 0xfc, 0x28, 0xf1, 0xbf, 0xbc, 0x80, 0x5a, 0xbe, 0xbc, 0x80, 
  0x66, 0xc0, 0xbc, 0x5c, 0x5f, 0xac, 0xbc, 0xa0, 0xf1, 0xad, 0xbc, 0x84, 0x00, 0xac, 0x7c, 0xc1, 
  0x20, 0x00, 0x4c, 0x5c, 0xf2, 0xb7, 0x3c, 0x61, 0x01, 0xba, 0xfc, 0x30, 0x1f, 0xbc, 0xfc, 0xe4, 
  0x17, 0xba, 0xfc, 0x28, 0xff, 0xba, 0xfc, 0x60, 0x01, 0xb2, 0x7c, 0x62, 0xff, 0xba, 0xd4, 0x6c, 
  0xf0, 0xaf, 0xbc, 0x08, 0x5c, 0xae, 0xbc, 0x80, 0x57, 0xba, 0x3c, 0x00, 0x5c, 0xae, 0xbc, 0x84, 
  0x01, 0xae, 0xfc, 0x80, 0x0f, 0xae, 0xfc, 0x60, 0xf0, 0xaf, 0x3c, 0x08, 0x17, 0x00, 0x7c, 0x5c, 
  0x60, 0xcc, 0xbc, 0x5c, 0xf0, 0xad, 0xbc, 0xa0, 0x08, 0xac, 0xfc, 0x80, 0x56, 0xae, 0xbc, 0x08, 
  0x04, 0xac, 0xfc, 0x80, 0x56, 0xb0, 0xbc, 0x08, 0x58, 0xae, 0x3c, 0x86, 0x34, 0x00, 0x68, 0x5c, 
  0x62, 0xb0, 0xbc, 0x80, 0x58, 0xc6, 0xbc, 0x00, 0x62, 0xb0, 0xbc, 0x84, 0x01, 0xb0, 0xfc, 0x80, 
  0x0f, 0xb0, 0xfc, 0x60, 0x56, 0xb0, 0x3c, 0x08, 0x00, 0xc7, 0xfc, 0x68, 0x02, 0xc6, 0xfc, 0x2c, 
  0x01, 0xc6, 0xfc, 0x68, 0x0b, 0xc8, 0xfc, 0xa0, 0xf1, 0xcb, 0xbc, 0xa0, 0x04, 0xb2, 0x7c, 0x62, 
  0x02, 0xb2, 0x7c, 0x61, 0x01, 0xc6, 0xe0, 0x6c, 0x01, 0xc6, 0xfc, 0x29, 0x61, 0xe8, 0xab, 0x70, 
  0x61, 0xec, 0x97, 0x74, 0x5a, 0xca, 0xbc, 0x80, 0x60, 0xcc, 0xbc, 0x5c, 0x65, 0xac, 0xbc, 0xa0, 
  0xf1, 0xad, 0xbc, 0x84, 0x00, 0xac, 0x7c, 0xc1, 0x4e, 0x00, 0x4c, 0x5c, 0x47, 0xc8, 0xfc, 0xe4, 
  0x34, 0x00, 0x7c, 0x5c, 0x00, 0x00, 0x00, 0x00, 
};
int32_t FullDuplexSerial_start(FullDuplexSerial *self, int32_t rxpin, int32_t txpin, int32_t mode, int32_t baudrate)
{
  int32_t okay = 0;
  // Start serial driver - starts a cog
  // returns false if no cog available
  //
  // mode bit 0 = invert rx
  // mode bit 1 = invert tx
  // mode bit 2 = open-drain/source tx
  // mode bit 3 = ignore tx echo on rx
  FullDuplexSerial_stop(self);
  self->txlock = locknew();
  self->strlock = locknew();
  memset( (void *)&self->rx_head, 0, sizeof(int32_t)*4);
  self->rx_pin = rxpin;
  self->tx_pin = txpin;
  self->rxtx_mode = mode;
  self->bit_ticks = CLKFREQ / baudrate;
  self->buffer_ptr = (int32_t)(&self->rx_buffer[0]);
  okay = self->cog = cognew((int32_t)(&(*(int32_t *)&dat[0])), (int32_t)(&self->rx_head)) + 1;
  return okay;
}

void FullDuplexSerial_stop(FullDuplexSerial *self)
{
  int32_t	_tmp__0001;
  // Stop serial driver - frees a cog
  if (self->cog) {
    cogstop((( ( (_tmp__0001 = self->cog), (self->cog = 0) ), _tmp__0001 ) - 1));
    lockret(self->txlock);
    lockret(self->strlock);
  }
  memset( (void *)&self->rx_head, 0, sizeof(int32_t)*9);
}

void FullDuplexSerial_txflush(FullDuplexSerial *self)
{
  // Flush transmit buffer
  while (self->tx_tail != self->tx_head) {
    Yield__();
  }
}

void FullDuplexSerial_rxflush(FullDuplexSerial *self)
{
  // Flush receive buffer
  while (FullDuplexSerial_rx(self) >= 0) {
    Yield__();
  }
}

int32_t FullDuplexSerial_rx(FullDuplexSerial *self)
{
  int32_t rxbyte = 0;
  // Check if byte received (never waits)
  // returns -1 if no byte received, $00..$FF if byte
  rxbyte = -1;
  if (self->rx_tail != self->rx_head) {
    rxbyte = self->rx_buffer[self->rx_tail];
    self->rx_tail = (self->rx_tail + 1) & 0xf;
  }
  return rxbyte;
}

void FullDuplexSerial_tx(FullDuplexSerial *self, int32_t txbyte)
{
  // Send byte (may wait for room in buffer)
  while (lockset(self->txlock)) {
    Yield__();
  }
  while (!(self->tx_tail != ((self->tx_head + 1) & 0xf))) {
    Yield__();
  }
  self->tx_buffer[self->tx_head] = txbyte;
  self->tx_head = (self->tx_head + 1) & 0xf;
  lockclr(self->txlock);
  if (self->rxtx_mode & 0x8) {
    FullDuplexSerial_rx(self);
  }
}

void FullDuplexSerial_str(FullDuplexSerial *self, int32_t stringptr)
{
  int32_t	_idx__0002, _limit__0003;
  // Send string                    
  while (lockset(self->strlock)) {
    Yield__();
  }
  for(( (_idx__0002 = 0), (_limit__0003 = strlen((char *) stringptr)) ); _idx__0002 < _limit__0003; _idx__0002++) {
    FullDuplexSerial_tx(self, ((uint8_t *)(stringptr++))[0]);
  }
  lockclr(self->strlock);
}

/* 

┌──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐
│                                                   TERMS OF USE: MIT License                                                  │                                                            
├──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┤
│Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation    │ 
│files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,    │
│modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software│
│is furnished to do so, subject to the following conditions:                                                                   │
│                                                                                                                              │
│The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.│
│                                                                                                                              │
│THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE          │
│WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR         │
│COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,   │
│ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                         │
└──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘
 */
