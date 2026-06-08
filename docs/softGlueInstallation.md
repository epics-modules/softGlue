---
layout: default
title: Installation
nav_order: 3
---

# softGlue Installation and Deployment
{: .no_toc}

## Table of contents
{: .no_toc .text-delta }

- TOC
{:toc}

## Overview

softGlue is a synApps module, so if you've used any other synApps
module, you probably already know how to install and deploy it.
The important thing is that softGlue is pure support: you are not
expected to run an IOC directly with it, but instead to draw from
the module into your own IOC application.

{: .note }
> Unlike most other synApps modules, softGlue publishes the text
> files needed to boot an IOC in its `db` directory (as an EPICS
> module really should), rather than in the `softGlueApp/Db`
> directory.

## How to get the software

softGlue is available as part of synApps 5.5 and higher, or
from the [softGlue GitHub repository](https://github.com/epics-modules/softGlue).

## Building softGlue

1. Edit `softGlue/configure/RELEASE` to specify the paths to
   `EPICS_BASE`, `ASYN`, and `IPAC`.

2. If you're using a version of ipac older than 2.11, edit
   `softGlueApp/src/drvIP_EP201.c` to change the definition of
   the macro `DO_IPMODULE_CHECK`:

   ```c
   #define DO_IPMODULE_CHECK 0
   ```

3. Run `make` in the top-level directory, using the same `make`
   executable used to build EPICS base.

{: .note }
> The build will issue a warning that it can't expand all macros
> in substitution files. This is not an error; unexpanded macros
> are intended to be defined at boot time. Note that version 1-4
> of `msi` returns an error, which causes the softGlue build to
> fail, after writing a database file that contains unexpanded
> macros.

## Deploying softGlue to an IOC

To configure an EPICS IOC application to use softGlue, you must
make modifications in the following directories, and then rebuild
the application.

**`configure/RELEASE`** -- Edit the `RELEASE` file to define the
following names:

```
SOFTGLUE=<path to the softGlue module>
ASYN=<path to the asyn module>
IPAC=<path to the ipac module>
BUSY=<path to the busy module>
```

`BUSY` is an optional add-on for use with softGlue. If the busy
module is available, you can arrange for EPICS database processing
to wait for a signal from softGlue hardware before declaring
itself to be finished. The `softGlue_convenience.db` database
loads busy records for this purpose, and the
`softGlueConvenience.adl` display file contains menu items to
display the records. Nothing else in softGlue depends on the
busy module.

**`xxxApp/src`** -- Edit `Makefile` or a `*Include.dbd` file so
that `softGlueSupport.dbd` is included in the `.dbd` file the
IOC loads at boot time. You'll also need files from asyn, ipac,
and busy, if you're not already including them.

For a Makefile:

```
iocxxx_DBD_vxWorks += softGlueSupport.dbd drvIpac.dbd asyn.dbd busySupport.dbd
```

For the `.dbd` file that will be loaded into a vxWorks IOC:

```
include "softGlueSupport.dbd"
include "drvIpac.dbd"
include "asyn.dbd"
include "busySupport.dbd"
```

Edit `Makefile` so that the IOC executable is linked with the
`softGlue` library. You'll also need libraries from the asyn,
ipac, and busy modules. The order in which libraries are named
is sometimes important. Example:

```
xxx_LIBS_vxWorks += asyn Ipac softGlue busy
```

**`iocBoot/iocxxx`** -- Copy
`softGlue/iocBoot/iocSoftGlue/softGlue.cmd` to your IOC
directory (`iocBoot/iocxxx`), and edit the `dbLoadRecords()`
commands in your copy of `softGlue.cmd` to define the macro
`P`, so that it's unique to your IOC.

{: .note }
> If you'll have more than one IP_EP20x module in an IOC, you'll
> also need to maintain separate definitions for the macro `H`,
> for asyn port names associated with the modules, and for the
> macro `READEVENT`. Port names are supplied as arguments to the
> functions `initIP_EP200()`, `initIP_EP201()`, and
> `initIP_EP201SingleRegisterPort()`, and they are supplied as
> the macro definitions `PORT`, `PORT1`, `PORT2`, and `PORT3` in
> `dbLoadRecords()` commands.

{: .important }
> If you specify the same value of `READEVENT` for N instances of
> softGlue in a single IOC, the software will still work, but it
> will use N times the CPU cycles, as each instance will post read
> events that will cause all other instances to read from the
> hardware.

Add the following line to `st.cmd`, before the call to
`iocInit()`:

```
< softGlue.cmd
```

If you use autosave, add the following line to
`save_restore.cmd`:

```
set_requestfile_path(softglue, "softGlueApp/Db")
```

{: .warning }
> The first argument `softglue` must be all lowercase, because
> this is how the path variable is defined in `cdCommands`.

Add the following line to `auto_settings.req` (or whatever you've
named the file used to save/restore PVs of arbitrary type):

```
file softGlue_settings.req  P=$(P) H=softGlue:
```

where the macros `P` and `H` agree with those in `softGlue.cmd`.

**`xxxApp/op/adl`** -- If you use MEDM, add a related-display
button to call up the `softGlueMenu.adl` display with the macros
`P` and `H`, as defined in `iocBoot/iocxxx/softGlue.cmd`. The
file `softGlueApp/op/adl/softGlueTop.adl` contains an example
button.

{: .note }
> If you figure out how to use softGlue with some other display
> manager, please tell us about it, so we can include your work
> in the next version of softGlue. The MEDM-display files
> included in softGlue make heavy use of MEDM's `composite file`,
> and other display managers may not have a comparable feature.
> softGlue's use of `composite file` is purely a
> display-development convenience.

If you use MEDM, give it access to the softGlue module's `.adl`
files. In csh, you could do this with the following command:

```
setenv EPICS_DISPLAY_PATH $EPICS_DISPLAY_PATH':'$SOFTGLUE/softGlueApp/op/adl
```

Don't forget to rebuild your application:

```
cd <applicationTop>
make rebuild
```

## Configuring hardware

The IP-EP20x board must be configured to permit programming the
FPGA via the IndustryPack bus, by moving the DIP jumper to
"IP BUS". This is the factory default setting.

## Configuring softGlue

softGlue is configured by editing `softGlue.cmd`. This file
contains calls to the user callable functions described below,
and `dbLoadRecords()` commands that load the databases matching
the FPGA content.

## User callable functions

The following functions are called from `softGlue.cmd` (or from
`st.cmd`) to initialize the IP-EP20x module and softGlue support.

### initIP_EP200_FPGA

```
initIP_EP200_FPGA(ushort carrier, ushort slot, char *filename)
    carrier:  IP-carrier number (numbering begins at 0)
    slot:     IP-slot number (numbering begins at 0)
    filename: Name of the FPGA-content hex file to load into
              the FPGA.

Example:
    initIP_EP200_FPGA(0, 2, "$(SOFTGLUE)/softGlueApp/Db/SoftGlue_2_2.hex")
```

Write content to the FPGA. This command will fail if the FPGA
already has content loaded, as it will after a soft reboot. When
the command fails for this reason, the already loaded FPGA content
will be left as it was, with no ill effect. To load new FPGA
content, you must power cycle the IOC.

### initIP_EP200

```
initIP_EP200(ushort carrier, ushort slot, char *portName1,
    char *portName2, char *portName3, int sopcBase)
    carrier:   IP-carrier number (numbering begins at 0)
    slot:      IP-slot number (numbering begins at 0)
    portName1: Name of asyn port for component at sopcBase
    portName2: Name of asyn port for component at sopcBase+0x10
    portName3: Name of asyn port for component at sopcBase+0x20
    sopcBase:  must agree with FPGA content (0x800000)

Example:
    initIP_EP200(0, 2, "SGIO_1", "SGIO_2", "SGIO_3", 0x800000)
```

Initialize basic field I/O.

### initIP_EP200_Int

```
initIP_EP200_Int(ushort carrier, ushort slot, int intVectorBase,
    int risingMaskMS, int risingMaskLS,
    int fallingMaskMS, int fallingMaskLS)
    carrier:       IP-carrier number (numbering begins at 0)
    slot:          IP-slot number (numbering begins at 0)
    intVectorBase: must agree with the FPGA content loaded
                   (0x90 for softGlue 2.1 and higher;
                    0x80 for softGlue 2.0 and lower).
                   softGlue uses three vectors, for example,
                   0x90, 0x91, 0x92.
    risingMaskMS:  interrupt on 0->1 for I/O pins 33-48
    risingMaskLS:  interrupt on 0->1 for I/O pins 1-32
    fallingMaskMS: interrupt on 1->0 for I/O pins 33-48
    fallingMaskLS: interrupt on 1->0 for I/O pins 1-32

Example:
    initIP_EP200_Int(0, 2, 0x90, 0x0, 0x0, 0x0, 0x0)
```

Initialize field-I/O interrupt support.

{: .important }
> Interrupt vectors are hardwired in the supplied FPGA content.
> Each IP-EP20x module uses three vectors (0x90, 0x91, 0x92),
> one for each set of 16 I/O bits. Depending on the
> interrupt-service mechanism supported by the operating system,
> multiple IP-EP20x boards may or may not all be able to generate
> interrupts. For PowerPC processors, interrupt-service routines
> (ISRs) are chained, so multiple IP-EP20x modules can all
> generate interrupts. But if the operating system doesn't permit
> multiple ISRs attached to a single interrupt vector, then only
> one IP-EP20x board will be able to generate interrupts. See
> "Field I/O Interrupt support" in the
> [User Guide](softGlueUserGuide.html) for a description of the
> `softGlueFieldIO_Intxx.adl` display.

### initIP_EP200_IO

```
initIP_EP200_IO(ushort carrier, ushort slot,
    ushort moduleType, ushort dataDir)
    carrier:    IP-carrier number (numbering begins at 0)
    slot:       IP-slot number (numbering begins at 0)
    moduleType: one of [201, 202, 203, 204]
    dataDir:    Bit mask, in which only the first 9 bits are
                significant.  If a bit is set, the corresponding
                field I/O pins are outputs.  Note that for the
                202 and 204 modules, all I/O is differential,
                and I/O pin N is paired with pin N+1.  For the
                203 module, pins 25/26 through 47/48 are
                differential pairs.
```

Correspondence between `dataDir` bits (0-8) and I/O pins (1-48):

| dataDir bit | 201 | 202 or 204 | 203 |
| - | - | - | - |
| bit 0 | pins 1-8 | pins 1, 3,25,27 | pins 25,27 |
| bit 1 | pins 9-16 | pins 5, 7,29,31 | pins 29,31 |
| bit 2 | pins 17-24 | pins 9,11,33,35 | pins 33,35 |
| bit 3 | pins 25-32 | pins 13,15,37,39 | pins 37,39 |
| bit 4 | pins 33-40 | pins 17,19,41,43 | pins 41,43 |
| bit 5 | pins 41-48 | pins 21,23,45,47 | pins 45,47 |
| bit 6 | x | x | pins 1-8 |
| bit 7 | x | x | pins 9-16 |
| bit 8 | x | x | pins 17-24 |

Examples:

1. For the IP-EP201, `moduleType` is 201, and `dataDir == 0x3c`
   would mean that I/O bits 17-48 are outputs.
2. For the IP-EP202 or IP-EP204, `moduleType` is 202 or 204, and
   `dataDir == 0x13` would mean that I/O bits
   1,3,25,27, 5,7,29,31, 17,19,41,43 are outputs.
3. For the IP-EP203, `moduleType` is 203, and `dataDir == 0x???`
   would mean that I/O bits 1-8, 25,27, 29,31, 33,35, 45,47 are
   outputs.

Set field-I/O data direction:

```
initIP_EP200_IO(0, 2, 201, 0x3c)
```

### initIP_EP201

For backward compatibility with softGlue 2.1 and earlier, the
following command can be used to initialize an IP_EP201 module,
instead of the above calls to `initIP_EP200()`,
`initIP_EP200_Int()`, and `initIP_EP200_IO()`. This won't work
for any other IP_EP200-series module.

```
initIP_EP201(char *portName, ushort carrier, ushort slot,
    int msecPoll, int dataDir, int sopcOffset,
    int interruptVector, int risingMask, int fallingMask)
    portName:        Name of asyn port for component at
                     sopcOffset
    carrier:         IP-carrier number (numbering begins at 0)
    slot:            IP-slot number (numbering begins at 0)
    msecPoll:        Time interval between driver polls of
                     field I/O bits
    dataDir:         Data direction for I/O bits, explained
                     below.
    sopcOffset:      SOPC offset (must be as in example below).
    interruptVector: Must agree with the FPGA content loaded
                     (0x90, 0x91, 0x92 for softGlue 2.1 and
                     higher; 0x80, 0x81, 0x82 for softGlue 2.0
                     and lower).
    risingMask:      16-bit mask: if a bit is 1, the
                     corresponding I/O bit will generate an
                     interrupt when its value goes from 0 to 1.
                     Bit 0 corresponds to field I/O pin 1,
                     bit 1 to pin 2, etc.
    fallingMask:     Similar to risingMask, but for 1-to-0
                     transitions.

                     Note that the user can overwrite risingMask
                     and fallingMask at run time, with menu
                     selections, and probably has those
                     selections autosaved.
```

`dataDir` is a bit mask in which only bits 0 and 8 are
significant:

- For `sopcOffset` 0x800000:
  - If bit 0 of `dataDir` is set, I/O bits 1-8 are outputs.
  - If bit 8 of `dataDir` is set, I/O bits 9-16 are outputs.
- For `sopcOffset` 0x800010:
  - If bit 0 of `dataDir` is set, I/O bits 17-24 are outputs.
  - If bit 8 of `dataDir` is set, I/O bits 25-32 are outputs.
- For `sopcOffset` 0x800020:
  - If bit 0 of `dataDir` is set, I/O bits 33-40 are outputs.
  - If bit 8 of `dataDir` is set, I/O bits 41-48 are outputs.

Example:

```
initIP_EP201("SGIO_1",0,2,1000000,0x101,0x800000,0x90,0x00,0x00)
initIP_EP201("SGIO_2",0,2,1000000,0x101,0x800010,0x91,0x00,0x00)
initIP_EP201("SGIO_3",0,2,1000000,0x101,0x800020,0x92,0x00,0x00)
```

{: .note }
> Interrupt vectors are currently hardwired in the supplied FPGA
> content. Thus, if you want to use two or more IP_EP20x modules,
> only one may be permitted to generate interrupts. Interrupt
> generation is entirely an end-user choice, and it occurs only
> for the purpose of causing some EPICS record to process on the
> change of state of a softGlue field I/O signal. See "Field I/O
> Interrupt support" in the [User Guide](softGlueUserGuide.html)
> for a description of the `softGlueFieldIO_Intxx.adl` display.

### initIP_EP201SingleRegisterPort

```
initIP_EP201SingleRegisterPort(char *portName,
    ushort carrier, ushort slot)
```

Initialize softGlue signal-name support.

Example:

```
initIP_EP201SingleRegisterPort("SOFTGLUE", 0, 2)
```
