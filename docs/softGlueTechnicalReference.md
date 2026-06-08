---
layout: default
title: Technical Reference
nav_order: 4
---

# softGlue Technical Reference
{: .no_toc}

## Table of contents
{: .no_toc .text-delta }

- TOC
{:toc}

## FPGA-content files

This version of softGlue contains several FPGA-content files,
only one of which can be loaded at a time.

- `SoftGlue_2_2.hex` -- Contains all standard softGlue
  components.
- `SoftGlue_2_2_0_Octupole_0_0.hex` -- Contains everything
  in `SoftGlue_2_2.hex`, plus two 32-bit shift registers.
- `SoftGlue_2_2_Encoder.hex` -- Contains everything in
  `SoftGlue_2_2.hex`, plus two 32-bit up/down counters,
  plus two quadrature decoders.
- `SoftGlue_2_2_encoder_ave.hex` -- Contains everything in
  `SoftGlue_2_2.hex`, plus four 32-bit up/down counters,
  two quadrature decoders, one 32-bit magnitude comparator
  that is hardwired to compare counters 1 and 2, and one
  eight-bit up/down counter.
- `SoftGlue_2_2_1ID_Vgate_0_1.hex` -- Contains everything
  in `SoftGlue_2_2.hex`, plus two 32-bit up/down counters.

## Database files

`softGlue_FPGAContent.substitutions`
- Run through `msi` at build time to produce
  `softGlue_FPGAContent.db`. Loads records matching most
  of the FPGA content loaded at cold-boot time from
  `SoftGlue_2_2.hex`.

`softGlue_FPGAContent_octupole.substitutions`
- Run through `msi` to produce
  `softGlue_FPGAContent_octupole.db`. Loads records for the
  two 32-bit shift registers in
  `SoftGlue_2_2_Octupole_0_0.hex`.

`softGlue_FPGAContent_Encoder.substitutions`
- Run through `msi` to produce
  `softGlue_FPGAContent_Encoder.db`. Loads records for the
  two 32-bit up/down counters in
  `SoftGlue_2_2_Encoder.hex`.

`softGlue_FPGAContent_EncoderAvg.substitutions`
- Run through `msi` to produce
  `softGlue_FPGAContent_EncoderAvg.db`. Loads records for
  the additional content in
  `SoftGlue_2_2_encoder_ave.hex`.

`softGlue_FPGAContent_s1ID_Vgate.substitutions`
- Run through `msi` to produce
  `softGlue_FPGAContent_s1ID_Vgate.db`. Loads records for
  the two 32-bit up/down counters in
  `SoftGlue_2_2_1ID_Vgate_0_1.hex`.

`softGlue_FPGAInt.substitutions`
- Run through `msi` to produce `softGlue_FPGAInt.db`.
  Loads records matching the field-I/O register components.
  These components control the field I/O registers which
  softGlue uses to connect with external wiring. Also loads
  records for user control of the driver's poller thread
  period.

`softGlue_Input.db`, `softGlue_Output.db`
- Each supports a single softGlue circuit-element I/O bit.

`softGlue_FieldInput.db`, `softGlue_FieldOutput.db`
- Each supports a single field I/O bit.

`softGlue_InRegister.db`
- Supports reads from a 16-bit register. Currently unused
  by any softGlue circuit element.

`softGlue_InRegister32.db`
- Supports reads from a 32-bit register, implemented in the
  FPGA as two 16-bit registers.

`softGlue_IntBit.db`
- Supports a single input bit, with an interrupt-driven bi
  record to read the bit value, and a forward-linked bo
  record to write that value to a user-specified EPICS PV.

`softGlue_IntEdge.db`
- Controls the interrupt-enable mask for a single input bit.

`softGlue_PollTime.db`
- Controls the polling period for a field I/O register set.
  This period is the rate at which I/O bits that do not
  generate interrupts are read.

`softGlue_Register.db`
- Supports writes to a 16-bit register. Currently unused by
  any softGlue circuit element.

`softGlue_Register32.db`
- Supports writes to a 32-bit register, such as the `N`
  value for a divide-by-N circuit element.

`softGlue_SignalShow.db`
- Implements part of softGlue's mechanism for showing users
  which signals are connected together. When a user presses
  a signal's "= button", the signal's name is written to a
  PV in this database, against which all softGlue signals
  compare their own signal names to determine whether or
  not to display their "connected" boxes.

`softGlue_convenience.db`
- Contains two busy records for signaling completion to
  EPICS, two software pulse generators, and two software
  clock generators.

`softGlue_FPGAContentDev.substitutions`, `softGlue_FPGAIntDev.substitutions`
- Not used in standard deployment. Provided for developers
  and deployers of custom FPGA content. Essentially the
  same as `softGlue_FPGAContent.substitutions` and
  `softGlue_FPGAInt.substitutions`, but intended for use
  directly in `dbLoadTemplate()` commands rather than
  through `msi`.

## Autosave-request files

`softGlue_FPGAContent_settings.req`,
`softGlue_FPGAContent_octupole_settings.req`,
`softGlue_FPGAContent_Encoder_settings.req`,
`softGlue_FPGAContent_s1ID_Vgate_settings.req`,
`softGlue_FPGAInt_settings.req`,
`softGlue_SignalShow_settings.req`,
`softGlue_convenience_settings.req`

These autosave request files correspond with similarly named
database or substitutions files, and take the same macro
definitions.

`softGlue_settings.req` includes
`softGlue_SignalShow_settings.req`,
`softGlue_FPGAContent_settings.req`,
`softGlue_FPGAInt_settings.req`, and
`softGlue_convenience_settings.req`. For standard softGlue,
this is the only autosave-request file an IOC needs.

## Display files

There are too many display files to describe individually,
and many are similar, so this section describes classes of
display files and the overall implementation strategy.

Displays whose names begin with `softGlue_` support
individual circuit elements. Displays whose names are of the
form `softGlueXxx` (no underscore after `softGlue`) support
collections of circuit elements, either by implementing menus
for calling up other displays, or by including several
`softGlue_` displays. "Include", in this context, means
specified as a "Composite File", with macro arguments, in the
definition of an MEDM grouped item, or specified as a
"linkgroup" in CSS-BOY.

In the rest of this display-file documentation, MEDM examples
are given. For CSS-BOY, substitute `.opi` for `.adl`. For
caQtDM, substitute `.ui`.

`softGlueMenu.adl`, `softGlueTop.adl`
- `softGlueMenu.adl` contains related-display menus for
  everything in softGlue. `softGlueTop.adl` is an example
  of how `softGlueMenu.adl` can be called up.

`softGlue_Input.adl`, `softGlue_Output.adl`
- Support a single 16-bit register component in the FPGA.
  Most circuit-element displays include several instances.
  For example, `softGlue_AND_bare.adl` includes two
  instances of `softGlue_Input.adl` and one instance of
  `softGlue_Output.adl`.

`softGlue_*.adl`, `softGlue_*_bare.adl`
- Where `*` is one of AND, BUFFER, DEMUX2, DFF, DivByN,
  DnCntr, MUX2, MUX4, OR, Shift32, UpCntr, XOR. Each
  supports a single circuit element. The `_bare.adl`
  displays are intended to be included in other displays.

`softGlue_Field*.adl`, `softGlue_Field*_bare.adl`
- Where `*` is one of FieldInput, FieldOutput, IntBit.
  Support a single field-I/O bit or its interrupt control
  and dispatch records. The `_bare.adl` displays are
  intended to be included in other displays.

The display of a circuit element is built in layers, from
instances of `softGlue_Input.adl` and `softGlue_Output.adl`,
which are included in `softGlue_<element name>_bare.adl`,
which is in turn included in one of the user displays (for
example, `softGlueAll.adl`, `softGlue_AND.adl`, etc.).

## Driver

softGlue's driver implements four asyn ports to connect
EPICS records with registers implemented in the IP-EP20x
module's FPGA. Three ports connect with `fieldIO_registerSet`
components, which provide comprehensive control over digital
I/O bits implemented in the module, including data direction
(i.e., read or write), interrupt enable, and status. The
fourth asyn port connects with single 16-bit register
components, with which all softGlue signal connections are
implemented.

The following is copied from `drvIP_EP201.c`:

```
    This driver cooperates with specific FPGA firmware loaded into the Acromag
    IP-EP201 (and probably other IP-EP200-series modules).  The loaded FPGA
    firmware includes Eric Norum's IndustryPack Bridge, which is an interface
    between the IndustryPack bus and the Altera FPGA's Avalon bus.  The
    IndustryPack Bridge does not define anything we can write to in the FPGA. 
    It's job is to support additional firmware loaded into the FPGA.  The
    additional firmware defines registers that we can read and write, and it can
    take one of the two forms (sopc components) supported by this driver:

        1) fieldIO_registerSet component

           A set of seven 16-bit registers defined by 'fieldIO_registerSet'
           below.  This register set provides bit-level I/O direction and
           interrupt-generation support, and is intended primarily to
           implement field I/O registers.

        2) single 16-bit register component

           a single 16-bit register, which has no interrupt service or bit-level
           I/O direction.  This type of sopc component is just a plain 16-bit
           register, which can be written to or read.  This driver doesn't know
           or care what the register might be connected to inside the FPGA.

    Each fieldIO_registerSet component must be initialized by a separate call to
    initIP_EP201(), because the component's sopc address must be specified at
    init time, so that the interrupt-service routine associated with the
    component can use the sopc address.  Currently, each call to initIP_EP201()
    defines a new asyn port, connects an interrupt-service routine, creates a
    message queue, and launches a thread.

    Single 16-bit register components, on the other hand, need not have their
    sopc addresses known at init time, because they are not associated with an
    interrupt service routine.  As a consequence, many such single-register
    components can be served by a single asyn port.  Users of this port must
    specify the sopc address of the register they want to read or write in
    their asynUser structure. Records do this by including the address in the
    definition of the record's OUT or INP field.  For example, the ADDR macro in
    the following field definition should be set to the register's sopc address:
    field(OUT,"@asynMask($(PORT),$(ADDR),0x2f)")

    The addressing of sopc components requires some explanation.  When a
    component is loaded into the FPGA, it is given an sopc address, which is a
    number in one of two regions of the Avalon address space.  These regions of
    Avalon memory space are mapped by the IndustryPack Bridge to specific ranges
    of the IndustryPack module's IO and MEM spaces.  The IO and MEM spaces, in
    turn, are mapped by the IndustryPack carrier, and by the ipac-module
    software, to corresponding ranges in a VME address space.  The lowest
    address in the IndustryPack module's IO space is mapped to the VME A16
    address given by ipmBaseAddr(carrier, slot, ipac_addrIO), which I'll call
    IOBASE in the following table.  The lowest address in the IndustryPack
    module's MEM space is mapped to the VME A32 address given by
    ipmBaseAddr(carrier, slot, ipac_addrMem), which I'll call MEMBASE in the
    following table.  (The module's MEM space could also have been mapped to the
    VME A24 space.  This code doesn't know or care, because it just gets the VME
    address by making a function call to code provided by the ipac module.)
    
    Note that IOBASE and MEMBASE depend on the IndustryPack carrier and slot
    into which the IP-EP200 module has been placed.

    Avalon_address | IP_space  IP_address  | VME_space   VME_address
    (sopc address) |                       |                 
    ---------------|-----------------------|-----------------------------
    0x000000       | IO        0x000000    | A16         IOBASE+0x000000     
    ...            | IO        ...         | A16         ...             
    0x00007f       | IO        0x00007f    | A16         IOBASE+0x00007f     
                   |                       |                                        
    0x800000       | MEM       0x000000    | A32         MEMBASE+0x000000
    ...            | MEM       ...         | A32         ...
    0xffffff       | MEM       0x7fffff    | A32         MEMBASE+0x7fffff
    
    Thus, if a component is created with the sopc address 0x000010, it can be
    accessed at the IO-space address 0x000010, which is mapped to the VME
    address IOBASE+0x000010.  If a component is created with the sopc address
    0x800003, it can be accessed at MEM-space address 0x000003, which is
    mapped to the VME address MEMBASE+0x000003.

    Note that users of this code are not expected to know anything about this
    address-mapping business.  The only address users ever specify is the sopc
    address, exactly as it was specified to Quartus.	
```

## Device support

softGlue uses standard asyn device support for everything
except signal names, which are handled by
`devAsynSoftGlue.c`. The driver supports `asynInt32` and
`asynUInt32Digital` interfaces; `devAsynSoftGlue.c` uses
`asynUInt32Digital`.

`devAsynSoftGlue.c` maintains lists of signal names and
associates each signal name with one of 15 bus lines
implemented in the FPGA. When a new signal name is
encountered, it is assigned to an unused bus line (or
ignored if there are no unused bus lines). The bus line
number is written to a register, in the FPGA, which controls
a multiplexer (for input signals) or demultiplexer (for
output signals).

If an input signal name ends with `*`, a register bit is set
that causes the output of the multiplexer to be routed
through an inverter before being connected to the circuit
element. If an input signal is numeric, it is assigned to
multiplexer address 0, which is driven not by a bus line,
but by a register bit set according to the numeric value.
(If `*` is appended to a numeric signal, it is ignored.)

## Field I/O

Field inputs and outputs are supported by two independent
sets of binary input and output records:

**softGlue-supported records** -- These records are part of
the `softGlue_FPGAContent.db` database. They are connected
to hardware via softGlue's asyn port (the port initialized
by the function `initIP_EP201SingleRegisterPort()`), and no
interrupt support is provided for them. They are polled at a
rate determined by the `READ PERIOD` menu on the
`softGlueMenu.adl` display.

**Non-softGlue-supported records** -- These records are
loaded separately from the records described above (they are
loaded by the `softGlue_FPGAInt.db` database). They are
connected to hardware via the asyn port initialized by the
function `initIP_EP201()`, and they can be interrupt driven.
They are also polled periodically. The polling period's
initial value is specified as an argument to
`initIP_EP201()`, and it can be modified by the user via the
`POLL TIME (MS)` text entry on `softGlueFieldIO*.adl`
displays.

## FPGA content

### IndustryPack bus interface

All communication between EPICS and the FPGA goes through
Eric Norum's IP-bus interface, which is described in
[Industry Pack Bridge](IndustryPackBridge.html).

### Single-register component

Most of softGlue is implemented with single-register FPGA
components connecting to standard digital circuitry, such as
AND gates, counters, etc., through interface circuitry of the
following three types.

#### Input

An input is essentially a 16-input multiplexer controlled by
a register that softGlue can write to and read from. Inputs
1-15 of all input multiplexers are connected together to form
a 15-line bus, so that all inputs with the same multiplexer
address are connected together. Input 0 of the multiplexer is
special: it connects to the `U` bit of the signal's control
register, instead of to a bus line, and is the means by which
softGlue implements direct user control of the signal value.
When the user writes `0` or `1` as a signal name, softGlue
sets the multiplexer address to zero, and sets the `U` bit
to 0 or 1.

A second register bit, `N`, controls whether or not the
multiplexer output is routed through an inverter before
connecting to the payload digital circuit element input; this
bit is the means by which softGlue implements names like
`reset*`, which connects the input to an inverted copy of the
signal `reset`. The invert bit could be, but currently is
not, used with the `U` bit.

Input register bit map:

| bit | 15 | 14 | 13 | 12 | 11 | 10 | 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
| - | - | - | - | - | - | - | - | - | - | - | - | - | - | - | - | - |
| function | | | | | | | | | | N | R | U | A3 | A2 | A1 | A0 |

| N | invert-signal bit |
| - | - |
| R | read bit |
| U | user-write bit |
| An | bus-line address bit |

#### Output

An output is a signal routed via a demultiplexer to any of
15 bus lines. No connection is made to the demultiplexer
output selected by the address 0; this address is used to
implement unconnected output signals.

Output register bit map:

| bit | 15 | 14 | 13 | 12 | 11 | 10 | 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
| - | - | - | - | - | - | - | - | - | - | - | - | - | - | - | - | - |
| function | | | | | | | | | | | R | | A3 | A2 | A1 | A0 |

Where `R` and `An` are as defined above for the input
register.

#### 16-bit register

An integer value written by EPICS to a parameter register
with a fixed connection to a specific device instance.
softGlue uses two 16-bit registers to implement the 32-bit
up counter.

### Field I/O register set component (`IP_EP20x`)

This SOPC component supports 16 bits of field I/O.

#### Registers

| Address Offset | Function | Description | Read/Write |
| - | - | - | - |
| 0 | Control/Status | Field-I/O direction, IRQ status | Read/Write |
| 1 | Field I/O Write Data OR Diff/Dir register | Write Field I/O or differential/direction register, depending on the FPGA content and how `DIFF_DIR` is defined in `drvIP_EP201.c` | Read/Write |
| 2 | Field I/O Read Data | Read Field I/O ONLY when dir = 0 | Read |
| 3 | Rising IRQ Status Bits | Which bits are causing interrupt from field I/O on transition to 1 | Read/Write* |
| 4 | Rising IRQ Interrupt-Enable Bits | Which rising edge bits have IRQ enabled | Read/Write |
| 5 | Falling IRQ Status Bits | Which bits are causing interrupt from field I/O on transition to 0 | Read/Write* |
| 6 | Falling IRQ Interrupt-enable bits | Which bits have falling IRQ enabled | Read/Write |

\* A write to this register clears interrupts for nonzero
bits. For example, after servicing a rising-edge interrupt
from bit 0, the interrupt-service routine writes 0x01 to
register 3 before re-enabling interrupts.

#### Control/Status-register bits

| Bit | Function | Value | Description |
| - | - | - | - |
| 0 | Field I/O Direction Lower 8-Bits | 0=Input, 1=Output | Sets I/O direction for the lower 8 field-I/O bits (single-ended signals only) |
| 1 | | 0 | |
| 2 | | 0 | |
| 3 | | 0 | |
| 4 | | 0 | |
| 5 | Falling Edge IRQ Present | 1=Present | Falling Edge interrupt from bits 0-7 sets bit to 1 |
| 6 | Rising Edge IRQ Present | 1=Present | Rising Edge interrupt from bits 0-7 sets bit to 1 |
| 7 | Lower 8-bits IRQ Present | 1=Present | Interrupt from bits 0-7 sets bit to 1 |
| 8 | Field I/O Direction Upper 8-Bits | 0=Input, 1=Output | Sets I/O direction for the upper 8 field-I/O bits (single-ended signals only) |
| 9 | | 0 | |
| 10 | | 0 | |
| 11 | | 0 | |
| 12 | | 0 | |
| 13 | Falling Edge IRQ Present Upper 8-Bits | 1=Present | Falling Edge interrupt from bits 8-15 sets bit to 1 |
| 14 | Rising Edge IRQ Present Upper 8-Bits | 1=Present | Rising Edge interrupt from bits 8-15 sets bit to 1 |
| 15 | Upper 8-bits IRQ Present | 1=Present | Interrupt from bits 8-15 sets bit to 1 |

#### Diff/Dir-register bits

| Bit | Function | Value | Description |
| - | - | - | - |
| 0 | Field I/O Direction Bit 0 | 0=Input, 1=Output | Sets I/O direction for differential signals only |
| 1 | Field I/O Direction Bit 1 | 0=Input, 1=Output | Sets I/O direction for differential signals only |
| 2 | Field I/O Direction Bit 2 | 0=Input, 1=Output | Sets I/O direction for differential signals only |
| 3 | Field I/O Direction Bit 3 | 0=Input, 1=Output | Sets I/O direction for differential signals only |
| 4 | Field I/O Direction Bit 4 | 0=Input, 1=Output | Sets I/O direction for differential signals only |
| 5 | Field I/O Direction Bit 5 | 0=Input, 1=Output | Sets I/O direction for differential signals only |
| 6 | Differential | 1=lower eight bits represent four differential signals | See Diff/Dir note below |
| 7 | Differential | 1=upper eight bits represent four differential signals | See Diff/Dir note below |
| 8 | | 0 | |
| 9 | | 0 | |
| 10 | | 0 | |
| 11 | | 0 | |
| 12 | | 0 | |
| 13 | | 0 | |
| 14 | | 0 | |
| 15 | | 0 | |

### Diff/Dir note

Field-I/O data direction for IP_EP200-series modules is
awkward and unfortunate, and is different for the different
module types. The module types differ in which I/O bits are
single ended and which are differential. Bit numbering can
be a little confusing in this context, because two single
ended signals from one module type use the same pins as a
single differential signal from another module type. In this
documentation, pin numbers refer to field I/O connector pins
1-48, which are the same for all module types.

- **IP_EP201** -- Pins 1-48 are single ended.
- **IP_EP202 & IP_EP204** -- Pins 1-48 are differential.
  Software drives only odd-numbered pins; even-numbered pins
  are their differential pairs.
- **IP_EP203** -- Pins 1-24 are single ended, pins 25-48 are
  differential. For differential pins, software drives only
  odd-numbered pins; even-numbered pins are their
  differential pairs.

For single-ended pins, data direction is controlled by the
Control/Status register of a `fieldIO_registerSet` component.
Each component controls 16 I/O pins. Bit 0 of
Control/Status controls the direction of the lower eight I/O
pins, bit 8 controls the direction of the upper eight pins.

For differential pins, data direction is controlled by bits
0-5 of the Diff/Dir register of the first (lowest SOPC
address) `fieldIO_registerSet` component.

FPGA content must be told which pins are single ended and
which are differential. Bits 6-7 of the Diff/Dir register of
a `fieldIO_registerSet` component are used for this purpose.
If bit 6 is 1, the lower eight I/O pins are differential.
If bit 7 is 1, the upper eight I/O pins are differential.

## Deployment considerations

Software dependence on FPGA content is of three kinds:

1. **Register set type** -- Dependence on the register set
   with which SOPC components are implemented. There are two
   different register sets currently in use:
   `fieldIO_registerSet` and `single 16-bit register`. This
   dependence is restricted to the driver code,
   `drvIP_EP201.c`.

2. **User circuits** -- Dependence on the user circuits
   attached to `single 16-bit register` SOPC components. For
   example, the version 2.0 implementation of softGlue has
   several AND gates, several OR gates, some counters, etc.,
   controlled by `single 16-bit register` components. This
   dependence is restricted to the database,
   autosave-request file, and display files, which should
   have an analog for each `single 16-bit register`
   component, and which should know which component address
   corresponds with which user circuit, and with which part
   of the user circuit.

3. **Field I/O direction settings** -- In softGlue 2.x,
   field I/O direction is not adjustable by the user, but
   is fixed at boot time. All databases and display files
   are compatible with any choice of field I/O direction
   settings, though displays do not give the user
   information about which bits are inputs and which are
   outputs.

Currently, the only strategy in use for ensuring that EPICS
support agrees with FPGA content is the inclusion of FPGA
content with softGlue. Deployers are expected to copy files
from `softGlue/iocBoot/iocSoftGlue` for initial deployment,
and to copy them again for any version upgrades, or else to
take responsibility themselves for agreement between software
and firmware.

## Programming the FPGA via the IP bus

The hex file to be loaded is included in the `softGlueApp/Db`
directory. The IP-EP20x board must be prepared for IP-bus
programming by moving the DIP jumper to "IP BUS" (this is the
factory default setting).

The hex file was prepared in Quartus with the following steps:

1. Under programming type select the Hexadecimal file format
   for Intel
2. Select your file name
3. Add your `.sof` file
4. Select the sof file and hit the properties button
5. Select the compression box
6. Make sure that you have selected 1-bit Passive Serial
   above for the mode
7. Select the options button under programming file type --
   make sure that you have a start address of 0x0 and that
   the count up radio button is selected
8. Generate your file
