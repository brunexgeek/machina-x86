//
// pci.c
//
// PCI bus driver
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

struct {
  int classcode;
  char *name;
} pci_classnames[] = {
  {0x000000, "Non-VGA unclassified device"},
  {0x000100, "VGA compatible unclassified device"},
  {0x010000, "SCSI storage controller"},
  {0x010100, "IDE interface"},
  {0x010200, "Floppy disk controller"},
  {0x010300, "IPI bus controller"},
  {0x010400, "RAID bus controller"},
  {0x018000, "Unknown mass storage controller"},
  {0x020000, "Ethernet controller"},
  {0x020100, "Token ring network controller"},
  {0x020200, "FDDI network controller"},
  {0x020300, "ATM network controller"},
  {0x020400, "ISDN controller"},
  {0x028000, "Network controller"},
  {0x030000, "VGA controller"},
  {0x030100, "XGA controller"},
  {0x030200, "3D controller"},
  {0x038000, "Display controller"},
  {0x040000, "Multimedia video controller"},
  {0x040100, "Multimedia audio controller"},
  {0x040200, "Computer telephony device"},
  {0x048000, "Multimedia controller"},
  {0x050000, "RAM memory"},
  {0x050100, "FLASH memory"},
  {0x058000, "Memory controller"},
  {0x060000, "Host bridge"},
  {0x060100, "ISA bridge"},
  {0x060200, "EISA bridge"},
  {0x060300, "MicroChannel bridge"},
  {0x060400, "PCI bridge"},
  {0x060500, "PCMCIA bridge"},
  {0x060600, "NuBus bridge"},
  {0x060700, "CardBus bridge"},
  {0x060800, "RACEway bridge"},
  {0x060900, "Semi-transparent PCI-to-PCI bridge"},
  {0x060A00, "InfiniBand to PCI host bridge"},
  {0x068000, "Bridge"},
  {0x070000, "Serial controller"},
  {0x070100, "Parallel controller"},
  {0x070200, "Multiport serial controller"},
  {0x070300, "Modem"},
  {0x078000, "Communication controller"},
  {0x080000, "PIC"},
  {0x080100, "DMA controller"},
  {0x080200, "Timer"},
  {0x080300, "RTC"},
  {0x080400, "PCI Hot-plug controller"},
  {0x088000, "System peripheral"},
  {0x090000, "Keyboard controller"},
  {0x090100, "Digitizer Pen"},
  {0x090200, "Mouse controller"},
  {0x090300, "Scanner controller"},
  {0x090400, "Gameport controller"},
  {0x098000, "Input device controller"},
  {0x0A0000, "Generic Docking Station"},
  {0x0A8000, "Docking Station"},
  {0x0B0000, "386"},
  {0x0B0100, "486"},
  {0x0B0200, "Pentium"},
  {0x0B1000, "Alpha"},
  {0x0B2000, "Power PC"},
  {0x0B3000, "MIPS"},
  {0x0B4000, "Co-processor"},
  {0x0C0000, "FireWire (IEEE 1394)"},
  {0x0C0100, "ACCESS Bus"},
  {0x0C0200, "SSA"},
  {0x0C0300, "USB Controller"},
  {0x0C0400, "Fiber Channel"},
  {0x0C0500, "SMBus"},
  {0x0C0600, "InfiniBand"},
  {0x0D0000, "IRDA controller"},
  {0x0D0100, "Consumer IR controller"},
  {0x0D1000, "RF controller"},
  {0x0D8000, "Wireless controller"},
  {0x0E0000, "I2O"},
  {0x0F0000, "Satellite TV controller"},
  {0x0F0100, "Satellite audio communication controller"},
  {0x0F0300, "Satellite voice communication controller"},
  {0x0F0400, "Satellite data communication controller"},
  {0x100000, "Network and computing encryption device"},
  {0x101000, "Entertainment encryption device"},
  {0x108000, "Encryption controller"},
  {0x110000, "DPIO module"},
  {0x110100, "Performance counters"},
  {0x111000, "Communication synchronizer"},
  {0x118000, "Signal processing controller"},
  {0x000000, NULL}
};

__inline unsigned char pci_read_byte(int busno, int devno, int funcno, int addr) {
  outpd(PCI_CONFIG_ADDR, ((unsigned long) 0x80000000 | (busno << 16) | (devno << 11) | (funcno << 8) | addr));
  return inp(PCI_CONFIG_DATA);
}

__inline unsigned short pci_read_word(int busno, int devno, int funcno, int addr) {
  outpd(PCI_CONFIG_ADDR, ((unsigned long) 0x80000000 | (busno << 16) | (devno << 11) | (funcno << 8) | addr));
  return inpw(PCI_CONFIG_DATA);
}

__inline unsigned long pci_read_dword(int busno, int devno, int funcno, int addr) {
  outpd(PCI_CONFIG_ADDR, ((unsigned long) 0x80000000 | (busno << 16) | (devno << 11) | (funcno << 8) | addr));
  return inpd(PCI_CONFIG_DATA);
}

__inline void pci_write_byte(int busno, int devno, int funcno, int addr, unsigned char value) {
  outpd(PCI_CONFIG_ADDR, ((unsigned long) 0x80000000 | (busno << 16) | (devno << 11) | (funcno << 8) | addr));
  outp(PCI_CONFIG_DATA, value);
}

__inline void pci_write_word(int busno, int devno, int funcno, int addr, unsigned short value) {
  outpd(PCI_CONFIG_ADDR, ((unsigned long) 0x80000000 | (busno << 16) | (devno << 11) | (funcno << 8) | addr));
  outpw(PCI_CONFIG_DATA, value);
}

__inline void pci_write_dword(int busno, int devno, int funcno, int addr, unsigned long value) {
  outpd(PCI_CONFIG_ADDR, ((unsigned long) 0x80000000 | (busno << 16) | (devno << 11) | (funcno << 8) | addr));
  outpd(PCI_CONFIG_DATA, value);
}

unsigned char pci_read_config_byte(struct unit *unit, int addr) {
  return pci_read_byte(unit->bus->busno, PCI_DEVNO(unit->unitno), PCI_FUNCNO(unit->unitno), addr);
}

unsigned short pci_read_config_word(struct unit *unit, int addr) {
  return pci_read_word(unit->bus->busno, PCI_DEVNO(unit->unitno), PCI_FUNCNO(unit->unitno), addr);
}

unsigned long pci_read_config_dword(struct unit *unit, int addr) {
  return pci_read_dword(unit->bus->busno, PCI_DEVNO(unit->unitno), PCI_FUNCNO(unit->unitno), addr);
}

void pci_write_config_byte(struct unit *unit, int addr, unsigned char value) {
  pci_write_byte(unit->bus->busno, PCI_DEVNO(unit->unitno), PCI_FUNCNO(unit->unitno), addr, value);
}

void pci_write_config_word(struct unit *unit, int addr, unsigned short value) {
  pci_write_word(unit->bus->busno, PCI_DEVNO(unit->unitno), PCI_FUNCNO(unit->unitno), addr, value);
}

void pci_write_config_dword(struct unit *unit, int addr, unsigned long value) {
  pci_write_dword(unit->bus->busno, PCI_DEVNO(unit->unitno), PCI_FUNCNO(unit->unitno), addr, value);
}

void pci_read_buffer(struct unit *unit, int addr, void *buffer, int len) {
  if ((addr & 3) == 0 && (len && 3) == 0) {
    unsigned long *buf = (unsigned long *) buffer;

    while (len > 0) {
      *buf++ = pci_read_dword(unit->bus->busno, PCI_DEVNO(unit->unitno), PCI_FUNCNO(unit->unitno), addr);
      addr += 4;
      len -= 4;
    }
  } else {
    unsigned char *buf = (unsigned char *) buffer;

    while (len > 0) {
      *buf++ = pci_read_byte(unit->bus->busno, PCI_DEVNO(unit->unitno), PCI_FUNCNO(unit->unitno), addr);
      addr++;
      len--;
    }
  }
}

void pci_write_buffer(struct unit *unit, int addr, void *buffer, int len) {
  if ((addr & 3) == 0 && (len && 3) == 0) {
    unsigned long *buf = (unsigned long *) buffer;

    while (len > 0) {
      pci_write_dword(unit->bus->busno, PCI_DEVNO(unit->unitno), PCI_FUNCNO(unit->unitno), addr, *buf++);
      addr += 4;
      len -= 4;
    }
  } else {
    unsigned char *buf = (unsigned char *) buffer;

    while (len > 0) {
      pci_write_byte(unit->bus->busno, PCI_DEVNO(unit->unitno), PCI_FUNCNO(unit->unitno), addr, *buf++);
      addr++;
      len--;
    }
  }
}

int pci_find_capability(struct unit *unit, int cap) {
  unsigned short status;
  unsigned char pos, id;
  int ttl = 48;

  status = pci_read_config_word(unit, PCI_CONFIG_STATUS);
  if (!(status & PCI_STATUS_CAP_LIST)) return 0;

  pos = pci_read_config_byte(unit, PCI_CONFIG_CAPABILITIES);

  while (ttl-- && pos >= 0x40) {
    pos &= ~3;
    id = pci_read_config_byte(unit, pos + PCI_CAP_LIST_ID);
    if (id == 0xff) break;
    if (id == cap) return pos;
    pos = pci_read_config_byte(unit, pos + PCI_CAP_LIST_NEXT);
  }

  return 0;
}

void pci_enable_busmastering(struct unit *unit) {
  unsigned long value;

  value = pci_read_config_dword(unit, PCI_CONFIG_CMD_STAT);
  value |= 0x00000004;
  pci_write_config_dword(unit, PCI_CONFIG_CMD_STAT, value);
}

static char *get_pci_class_name(int classcode) {
  int i = 0;

  while (pci_classnames[i].name != NULL) {
    if (pci_classnames[i].classcode == classcode) return pci_classnames[i].name;
    if (pci_classnames[i].classcode == (classcode & 0xFFFF00)) return pci_classnames[i].name;
    i++;
  }

  return "Unknown";
}

static unsigned long pci_size(unsigned long base, unsigned long mask) {
  // Find the significant bits
  unsigned long size = mask & base;

  // Get the lowest of them to find the decode size
  size = size & ~(size-1);

  return size;
}

void enum_pci_bus(struct bus *bus) {
  int devno;
  int funcno;
  int busno;
  int bar;
  int intrpin;
  int irq;
  unsigned long value;
  unsigned long len;
  unsigned long vendorid;
  unsigned long deviceid;
  unsigned long classcode;
  unsigned long revision;
  struct unit *unit;
  unsigned long prev_deviceid;

  for (devno = 0; devno < 32; devno++) {
    prev_deviceid = 0;

    for (funcno = 0; funcno < 8; funcno++) {
      // Vendor and device ids
      value = pci_read_dword(bus->busno, devno, funcno, PCI_CONFIG_VENDOR);
      vendorid = value & 0xFFFF;
      deviceid = value >> 16;

      if (vendorid == 0xFFFF || vendorid == 0) continue;
      if (deviceid == prev_deviceid) continue;
      prev_deviceid = deviceid;

      // Function class code
      value = pci_read_dword(bus->busno, devno, funcno, PCI_CONFIG_CLASS_REV);
      classcode = value >> 8;
      revision = value & 0xFF;

      // Register new unit, host bridge is a special case
      if (bus->busno == 0 && devno == 0 && funcno == 0 && bus->self) {
        unit = bus->self;
      } else {
        unit = kdev_add_unit(bus, classcode, PCI_UNITCODE(vendorid, deviceid), PCI_UNITNO(devno, funcno));
      }


      unit->classname = get_pci_class_name(classcode);
      unit->revision = revision;

      // Subsystem id
      value = pci_read_dword(bus->busno, devno, funcno, PCI_CONFIG_SUBSYSTEM);
      unit->subunitcode = PCI_UNITCODE(value & 0xFFFF, value >> 16);

      if (classcode == PCI_BRIDGE) {
        struct bus *bridge;

        // Get secondary bus number for bridge
        value = pci_read_dword(bus->busno, devno, funcno, PCI_CONFIG_BASE_ADDR_2);
        busno = (value >> 8) & 0xFF;

        // Allocate and initialize new PCI bus
        bridge = kdev_add_bus(unit, BUSTYPE_PCI, busno);

        // Scan for devices on secondary bus
        enum_pci_bus(bridge);
      } else {
        // Function I/O and memory base addresses
        for (bar = 0; bar < 6; bar++) {
          int reg = PCI_CONFIG_BASE_ADDR_0 + (bar << 2);

          value = pci_read_dword(bus->busno, devno, funcno, reg);
          pci_write_dword(bus->busno, devno, funcno, reg, 0xFFFFFFFF);
          len = pci_read_dword(bus->busno, devno, funcno, reg);
          pci_write_dword(bus->busno, devno, funcno, reg, value);

          if (len != 0 && len != 0xFFFFFFFF) {
            if (value == 0xFFFFFFFF) value = 0;

            if (value & 1) {
              kdev_add_resource(unit, RESOURCE_IO, 0, value & PCI_BASE_ADDRESS_IO_MASK, pci_size(len, PCI_BASE_ADDRESS_IO_MASK & 0xFFFF));
            } else {
              kdev_add_resource(unit, RESOURCE_MEM, 0, value & PCI_BASE_ADDRESS_MEM_MASK, pci_size(len, PCI_BASE_ADDRESS_MEM_MASK));
            }
          }
        }

        // Function interrupt line
        value = pci_read_dword(bus->busno, devno, funcno, PCI_CONFIG_INTR);
        if ((value & 0xFF) > 0 && (value & 0xFF) < 32) {
          intrpin = (value >> 8) & 0xFF;
          irq = value & 0xFF;
          kdev_add_resource(unit, RESOURCE_IRQ, 0, irq, 1);
        }
      }
    }
  }
}

unsigned long get_pci_hostbus_unitcode() {
  unsigned long value;
  unsigned long vendorid;
  unsigned long deviceid;

  // Try to read bus 0 device 0 function 0 vendor
  value = pci_read_dword(0, 0, 0, PCI_CONFIG_VENDOR);
  vendorid = value & 0xFFFF;
  deviceid = value >> 16;

  if (vendorid == 0 || vendorid == 0xFFFF) return 0;

  return PCI_UNITCODE(vendorid, deviceid);
}
