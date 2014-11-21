Machina
=======

A lightweight operating system for x86 machines (32bits). The OS is still in development,
but intends to provide a CLI in graphical mode and support for GUI.


Features
----------------

- Operate in 32bits protected mode
- Multi-thread support
- Kernel output through serial port
- Virtual memory and memory protection (read/write)
- Kernel API (syscalls) for user mode applications
- Bootable CD-ROM image


Working in progress
----------------

- Support for ELF32 binaries
- Support for multiprocessing in user mode


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
