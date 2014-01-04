//
// pic.c
//
// Programmable Interrupt Controller (PIC i8259)
//
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

#include <os/krnl.h>
#include <os/pic.h>

// All IRQs disabled initially except cascade

static unsigned int irq_mask = 0xFFFB;

//
// Set interrupt mask
//

static void set_interrupt_mask(unsigned long mask)
{
    outp(PIC_MASTER_DATA, (unsigned char) mask);
    outp(PIC_SLAVE_DATA, (unsigned char) (mask >> 8));
}

/**
 * Initialize the 8259 Programmable Interrupt Controller.
 */
void kpic_init()
{
    // start the PICs initialization
    outp(PIC_MASTER_COMMAND, PIC_MASTER_ICW1);
    outp(PIC_SLAVE_COMMAND, PIC_SLAVE_ICW1);
    // set the master vector offset
    outp(PIC_MASTER_DATA, PIC_MASTER_ICW2);
    // set the slave vector offset
    outp(PIC_SLAVE_DATA, PIC_SLAVE_ICW2);
    // tell master PIC that there is a slave PIC at IRQ2
    outp(PIC_MASTER_DATA, PIC_MASTER_ICW3);
    // tell slave PIC it is cascade identity
    outp(PIC_SLAVE_DATA, PIC_SLAVE_ICW3);
    // set the operation mode for master and slave PIC
    outp(PIC_MASTER_DATA, PIC_MASTER_ICW4);
    outp(PIC_SLAVE_DATA, PIC_SLAVE_ICW4);

    set_interrupt_mask(irq_mask);
}

//
// Enable IRQ
//

void kpic_enable_irq(unsigned int irq)
{
    irq_mask &= ~(1 << irq);
    if (irq >= 8) irq_mask &= ~(1 << 2);
    set_interrupt_mask(irq_mask);
}

//
// Disable IRQ
//

void kpic_disable_irq(unsigned int irq)
{
    irq_mask |= (1 << irq);
    if ((irq_mask & 0xFF00) == 0xFF00) irq_mask |= (1 << 2);
    set_interrupt_mask(irq_mask);
}


/**
 * Signal end of interrupt to PIC.
 */
void kpic_eoi(unsigned int irq)
{
    if (irq < 8)
    {
        outp(PIC_MASTER_COMMAND, irq + PIC_EOI_BASE);
    }
    else
    {
        outp(PIC_SLAVE_COMMAND, (irq - 8) + PIC_EOI_BASE);
        outp(PIC_MASTER_COMMAND, PIC_EOI_CAS);
    }
}
