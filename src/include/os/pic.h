//
// pic.h
//
// Programmable Interrupt Controller (PIC i8259)
//
// Copyright (C) 2014 Bruno Ribeiro. All rights reserved.
// Copyright (C) 2002 Michael Ringgaard. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of the project nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE.
//

#ifndef PIC_H
#define PIC_H

//
// PIC I/O ports
//

#define PIC_MASTER_COMMAND      0x20
#define PIC_MASTER_DATA         0x21
#define PIC_SLAVE_COMMAND       0xA0
#define PIC_SLAVE_DATA          0xA1


#define ICW4_8086               0x01  // 8086/88 (MCS-80/85) mode
#define ICW4_AUTO               0x02  // Auto (normal) EOI
#define ICW4_BUF_SLAVE          0x08  // Buffered mode/slave
#define ICW4_BUF_MASTER         0x0C  // Buffered mode/master
#define ICW4_SFNM               0x10  // Special fully nested

//
// Master commands
//

#define PIC_MASTER_ICW1           0x11
#define PIC_MASTER_ICW2           0x20
#define PIC_MASTER_ICW3           0x04
#define PIC_MASTER_ICW4           ICW4_8086

//
// Slave commands
//

#define PIC_SLAVE_ICW1            0x11
#define PIC_SLAVE_ICW2            0x28
#define PIC_SLAVE_ICW3            0x02
#define PIC_SLAVE_ICW4            ICW4_8086

//
// End of interrupt commands
//

#define PIC_EOI_BASE            0x60

#define PIC_EOI_CAS             0x62
#define PIC_EOI_FD              0x66

void kpic_init();

KERNELAPI void kpic_enable_irq(unsigned int irq);
KERNELAPI void kpic_disable_irq(unsigned int irq);
KERNELAPI void kpic_eoi(unsigned int irq);

#endif
