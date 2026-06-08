---
layout: default
title: Release Notes
nav_order: 2
---

# softGlue Release Notes
{: .no_toc}

## Table of contents
{: .no_toc .text-delta }

- TOC
{:toc}

## Release 2-8-4

- **Updated autoconversion** -- Updated autoconversion of bob and edl files.
- **Documentation** -- Documentation moved to GitHub Pages.

## Release 2-8-3

- **Display files** -- Added bob files, updated ui and edl files.

## Release 2-8-2

- **Req files** -- Req files now installed to top level db folder.

## Release 2-8

- **Interrupt masks** -- Custom interrupt service routines registered with softGlue now receive 16 bit masks `wentLow` and `wentHigh`, describing which bit went low or high to trigger the interrupt.

## Release 2-7

- **Magnitude comparator** -- Added 32-bit magnitude comparator.
- **Encoder average** -- Add-on component to produce time-averaged value of an encoder (`SoftGlue_2_2_encoder_ave.hex`).

## Release 2-6

- **Four-output demultiplexer** -- Added four-output demultiplexer (`SoftGlue_2_2_demux4.hex`).
- **Hardwired demultiplexer** -- Added four-output demultiplexer with output lines hardwired to last 16 field I/O pins.

## Release 2-5

- **caQtDM displays** -- Reworked displays for caQtDM.
- **Display labels** -- Label all displays with `$(P)$(H)`, so users know which instance of softGlue they're looking at.
- **Windows build** -- Fix for Windows build.
- **Presentation** -- Added PowerPoint presentation.
- **Signal count** -- Previously, the number of signals in use wasn't updated at boot time.

## Release 2-4-3

- **Base 3.15 compatibility** -- For compatibility with base 3.15, `softGlueApp/src` now specifies `DBD += softGlueSupport.dbd`, instead of `DBD_INSTALLS += softGlueSupport.dbd`.

## Release 2-4-2

- **asynMask change** -- Beginning with asyn-4-23, asynMask applies the mask to longout records, so `softGlue_PollTime.db` now defines the mask.
- **Build restriction** -- Build is no longer restricted to vxWorks.

## Release 2-4-1

- **pollTimeMS fix** -- `pollTimeMS` was not being acted on at init time. Also, `pollTimeMS` was improperly converted to double, so that `pollTimeMS` < 1000 yielded zero, which took all of the CPU cycles. Now, `pollTimeMS` is not honored if less than 50.

## Release 2-4

- **Custom interrupt handler** -- Added support for registering and calling a custom interrupt handler.
- **VME address calculation** -- Added support for calculating the VME address of a softGlue register.
- **Example handler** -- Added example custom interrupt handler.
- **Encoder docs** -- Added docs for `softGlue_FPGAContent_Encoder`.
- **Field-wiring docs** -- Added field-wiring docs and pictures.

## Release 2-3-1

- **AddComponent docs** -- Added docs describing how to do the EPICS support for a new SOPC component (`documentation/AddComponent.txt`).
- **New components** -- Added quadrature decoder, up/down counter.
- **Shift register docs** -- Added docs for shift register.
- **Label PVs** -- Added label PVs: one for circuit as a whole, one each for each component. For now, labels are stringout records. The plan is to switch to lso records (in base 3.15), which can be longer than 40 characters.
- **Connector descriptions** -- Support user modified field I/O connector descriptions.
- **configMenu docs** -- Added docs for using autosave's configMenu with softGlue.

## Release 2-3

- **macLib fix** -- macLib didn't like `softGlue_FPGAInt.db`'s use of `PORT1` (etc.) to tell `softGlue_PollTime.db` what value to give its `PORT1` macro (recursive). Modified `softGlue_PollTime.db` to use `PORTA`.
- **Improved docs** -- Improved docs.
- **New displays** -- Added display files for CSS-BOY and caQtDM.
- **configMenu support** -- Added configMenu support for saving and restoring circuits (requires autosave R5-1).

## Release 2-2

- **Significant rewrite** -- Significant rewrite, including new configuration functions, to support IP_EP20x modules other than the IP_EP201.
- **Signal count display** -- The number of used and available signal names (bus lines) is now displayed.
- **Macro changes** -- Macro arguments for some database files have changed, so old `softGlue.cmd` files will not work without modification.
- **Memory savings** -- Display of signal connections has been moved from the database to device support, saving around 1MB of IOC memory, and removing softGlue's dependence on the calc module.
- **Shift registers** -- Added displays and FPGA add-on content ("octupole") for shift registers.
- **IP_EP202/203 displays** -- Added displays for IP_EP202 and IP_EP203 field I/O.
- **FIFO size** -- Ring buffer size for interrupt dispatch can now be specified as the macro `FIFO`, supplied to `softGlue_IntBit.db`. The default value (10) is used if `FIFO` is not specified.
- **SOPC address** -- The SOPC address for field I/O register set components need no longer be supplied as macros to `softGlue_FPGAInt.db`.
- **DivByN RESET** -- The Divide-By-N's `RESET` signal now works.
- **CSS-BOY displays** -- In addition to MEDM displays, softGlue now includes CSS-BOY displays.
- **Up/down counters** -- Added displays and FPGA add-on content ("Vgate") for up/down counters.

## Release 2-1

- **Interrupt during load** -- The FPGA sometimes generates an interrupt while it is being loaded, so the interrupt level is disabled before the load, and enabled after.
- **Interrupt vectors** -- softGlue was using the interrupt vectors 0x80, 0x81, and 0x82, but these are already in use by the synApps IP-Octal support, so the vectors 0x90, 0x91, and 0x92 are now used instead. These vectors currently are hard-coded in the FPGA content, so even though `initIP_EP201()` has an argument for them, they mustn't be changed.

## Release 2-0

- **Signal inversion** -- Inverted outputs have been deleted from all FPGA components. Instead signal inversion is accomplished by appending an asterisk to the signal name specified for an input (or for a field I/O output, which behaves as an input).
- **48-bit I/O** -- All 48 I/O bits are supported by both field inputs and field outputs, and the I/O directions are specified at boot time. If a bit is set for output, its digital value is routed to the field input, so that the bit can generate interrupts. With this change, all IP-EP20x modules should be supported, though this hasn't been tested, and databases and MEDM displays are provided only for the IP-EP201 (48 TTL bits).
- **8 MHz clock** -- Added an 8 MHz clock component.
- **More gates and buffers** -- Added more gates, added buffers (primarily so users don't have to use a logic gate simply to get an EPICS-written value fanned to more than one input).
- **Octupole module** -- Added the secondary module, Octupole, which contains two 32-bit shift registers.
- **32-bit counters** -- Down counters and divide-by-N components are now 32 bit.
- **Poll period control** -- User can control the polling period for field I/O bits.
- **Signal processing improvements** -- No longer maintain separate read rates for connected and unconnected signals, and cause signal-show display to be driven by a software event whose number is specified at boot time, rather than being periodically scanned. These and other changes simplify signal-name processing and speeds it up significantly. (Previously, two sCalcout records were processed every time a signal name was changed. Now none are.)

## Release 1-1

- **FPGA loading** -- Added the ability to load FPGA content at boot time, from a file, via the IP bus.
- **Database loading** -- Modified the recommended method of loading EPICS support. Now, user is expected to load the database directly from the softGlue module, so it's more likely to agree with FPGA content also loaded from softGlue.
- **Renamed** -- Renamed ip1k125 to IP_EP200 or IP_EP201.
- **16-bit field I/O** -- Now supporting 16 field-input bits, and 16 field-output bits.
- **32-bit counters** -- Added 32-bit up counters.
- **Drag-And-Drop** -- Added limited support for using Drag-And-Drop to connect signals. If the string written to a signal is the PV name of some other signal pertaining to the same IP-EP201 module, device support will replace the PV name with the string value of the PV. This connects signals only if the Drag-And-Drop source PV was not numeric, and not an empty string.
- **Signal name rules** -- Tighten up notion of legal signal names: leading spaces removed; for outputs, leading decimal digits also removed. Added `PINI=YES` to `*Signal` PVs, so rules are enforced also at boot time.
- **Signal inversion** -- Input signal names ending with `*` are allowed, denote the same signal as the same signal name without a trailing `*`, and indicate that the signal value is to be run through an inverter before being applied to the circuit element.
- **msi usage** -- Use msi to build partially resolved databases, so user can supply their own `P` and `H` macros, without having to supply everything else.
- **Interrupt fix** -- Interrupt handling worked only when both edges were enabled.
- **Runtime control** -- User can modify interrupt enables, and poll time for non-interrupting field I/O signals, at run time.
- **Interrupt throttling** -- Interrupt handler throttles interrupts.
- **Interrupt records** -- Included records for interrupts to drive; implemented GUI for users to manage this.
- **BURT request file** -- Added BURT request file to save all of softGlue.
- **8 MHz clock** -- Added 8 MHz clock.
- **Down counter fix** -- Decrement user's down-counter N value before writing to FPGA.
- **Documentation** -- Documentation and examples.

## Release 1-0

- **Initial release** -- Initial release.
