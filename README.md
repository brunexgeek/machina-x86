Machina
=======

A lightweight kernel and operating system for x86 machines (32bits), built to
be fast, simple and extensible.

Machina is under development and is not yet useable, but intends to provide a graphic mode CLI with
support for GUI.


Features
--------

- Operate in 32 bits protected mode
- Multi-threading support
- Kernel output through serial port
- Virtual memory and memory protection (read/write)
- Kernel API (syscalls) for user mode applications
- Bootable CD-ROM image
- Self configuring (PCI & DHCP support)
- TCP/IP networking stack
- Provided under [BSD 3-Clause](http://opensource.org/licenses/BSD-3-Clause) open-source license (check COPYING file for details)


Required tools
--------------

- GNU/Linux enviroment
- GNU C Compiler 2.7 or above (not tested with other versions and compilers)
- genisoimage


Building
--------

Clone the repository in your machine. In the root directory, type the following commands to create the ISO image:

```
# make iso
```

Now, just execute the following script to run the operating system with qemu:

```
# ./run.sh
```


Working in progress
-------------------

Currently I'm working on the following features:

- Support for ELF32 binaries
- Support for APIC
- ext2 filesystem

Roadmap
-------

Some features for future developement (no specific order):

- Standard C library
- Support for multiprocessing in user mode
- Graphic mode CLI
- Port to ARMv6 machines (e.g. Raspberry Pi)
- Support for PAE (Physical Address Extension)
- Support for PSE (Page Size Extension)
- Support for UEFI
- Support for multi-core CPUs
- OS API for GPU parallel computing


Supported hardware
------------------

 - Standard PC architecture
 - Minimum 2MB RAM (Maximum 4GB RAM)
 - Intel IA-32 and compatible processors (Pentium II or later)
 - IDE disks (PIO and UDMA mode)
 - IDE cdrom (PIO mode)
 - Floppy disks (3½" 1.44MB)
 - Keyboard
 - Text mode video
 - Serial ports (8250, 16450, 16550 and 16550A)


Credits
-------

This OS is based on [Sanos](http://www.jbox.dk) by Michael Ringgaard.
