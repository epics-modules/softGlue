softGlue Release Notes
----------------------

Release 2-8-3
-------------

- Added bob files, updated ui and edl files.

Release 2-8-2
-------------

- Req files now installed to top level db folder.

Release 2-8 
------------

- Custom interrupt service routines registered with softGlue now receive 16 bit masks wentLow and wentHigh, describing which bit went low or high to trigger the interrupt

Release 2-7 
------------

- Added 32-bit magnitude comparator.
- Add-on component to produce time-averaged value of an encoder (SoftGlue\_2\_2\_encoder\_ave.hex)

Release 2-6 
------------

- Added four-output demultiplexer (SoftGlue\_2\_2\_demux4.hex).
- Added four-output demultiplexer with output lines hardwired to last 16 field I/O pins (hardwired to last 16 field I/O pins).

Release 2-5 
------------

- Reworked displays for caQtDM.
- label all displays with $(P)$(H), so users know which instance of softGlue they're looking at.
- Fix for Windows build.
- Added powerpoint presentation.
- Previously, the number of signals in use wasn't updated at boot time.

Release 2-4-3 
--------------

- For compatibility with base 3.15, softGlueApp/src now specifies "DBD += softGlueSupport.dbd", instead of "DBD\_INSTALLS += softGlueSupport.dbd"

Release 2-4-2 
--------------

- Beginning with asyn-4-23, asynMask applies the mask to longout records, so softGlue\_PollTime.db now defines the mask.
- Build is no longer restricted to vxWorks.

Release 2-4-1 
--------------

- pollTimeMS was not being acted on at init time. Also, pollTimeMS was improperly converted to double, so that pollTimeMS &lt; 1000 yielded zero, which took all of the cpy cycles. Now, pollTimeMS is not honored if less than 50.

Release 2-4
-----------

- Added support for registering and calling a custom interrupt handler.
- Added support for calculating the VME address of a softGlue register.
- Added example custom interrupt handler.
- Added docs for softGlue\_FPGAContent\_Encoder
- Added field-wiring docs and pictures.

Release 2-3-1
-------------

- Added docs describing how to do the EPICS support for a new sopc component (documentation/AddComponent.txt).
- Added quadrature decoder, up/down counter.
- Added docs for shift register.
- Added label PVs: one for circuit as a whole, one each for each component. For now, labels are stringout records. The plan is to switch to lso records (in base 3.15), which can be longer than 40 characters.
- Support user modified field I/O connector descriptions.
- Added docs for using autosave's configMenu with softGlue.

Release 2-3
-----------

- macLib didn't like softGlue\_FPGAInt.db's use of PORT1 (etc.) to tell softGlue\_PollTime.db what value to give it's PORT1 macro (recursive). Modified softGlue\_PollTime.db to use PORTA.
- Improved docs.
- Added display files for CSS-BOY and caQtDM.
- Added configMenu support for saving and restoring circuits (requires autosave R5-1).

Release 2-2 (Aug 26, 2011)
--------------------------

- Significant rewrite, including new configuration functions, to support IP\_EP20x modules other than the IP\_EP201.
- The number of used and available signal names (bus lines) is now displayed.
- macro arguments for some database files have changed, so old softGlue.cmd files will not work without modification
- display of signal connections has been moved from the database to device support, saving around 1MB of IOC memory, and removing softGlue's dependence on the calc module.
- Added displays and FPGA add-on content ("octupole") for shift registers.
- Added displays for IP\_EP202 and IP\_EP203 field I/O
- Ring buffer size for interrupt dispatch can now be specified as the macro "FIFO", supplied to softGlue\_IntBit.db. The default value (10) is used if FIFO is not specified.
- The SOPC address for field I/O register set components need no longer be supplied as macros to softGlue\_FPGAInt.db.
- The Divide-By-N's RESET signal now works.
- In addition to MEDM displays, softGlue now includes CSS-BOY displays.
- Added displays and FPGA add-on content ("Vgate") for up/down counters.

Release 2-1 (Nov 11, 2010)
--------------------------

- The FPGA sometimes generates an interrupt while it is being loaded, so the interrupt level is disabled before the load, and enabled after.
- softGlue was using the interrupt vectors 0x80, 0x81, and 0x82, but these are already is use by the synApps IP-Octal support, so the vectors 0x90, 0x91, and 0x92 are now used instead. These vectors currently are hard-coded in the FPGA content, so even though initIP\_EP201() has an argument for them, they mustn't be changed.

Release 2-0 (Sept 24, 2010)
---------------------------

- Inverted outputs have been deleted from all FPGA components. Instead signal inversion is accomplished by appending an asterisk to the signal name specified for an input (or for a field I/O output, which behaves as an input).
- All 48 I/O bits are supported by both field inputs and field outputs, and the I/O directions are specified at boot time. If a bit is set for output, its digital value is routed to the field input, so that the bit can generate interrupts. With this change, all IP-EP20x modules should be supported, though this hasn't been tested, and databases and MEDM displays are provided only for the IP-EP201 (48 TTL bits).
- Added an 8 MHz clock component.
- Added more gates, added buffers (primarily so users don't have to use a logic gate simply to get an EPICS-written value fanned to more than one input).
- Added the secondary module, Octupole, which contains two 32-bit shift registers.
- Down counters and divide-by-N components are now 32 bit.
- User can control the polling period for field I/O bits.
- No longer maintain separate read rates for connected and unconnected signals, and cause signal-show display to be driven by a software event whose number is specified at boot time, rather than being periodically scanned. These and other changes simplify signal-name processing and speeds it up significantly. (Previously, two sCalcout records were processed every time a signal name was changed. Now none are.)
- 

Release 1-1 (June 14, 2010)
---------------------------

- Added the ability to load FPGA content at boot time, from a file, via the IP bus.
- Modified the recommended method of loading EPICS support. Now, user is expected to load the database directly from the softGlue module, so it's more likely to agree with FPGA content also loaded from softGlue.
- Renamed ip1k125 to IP\_EP200 or IP\_EP201.
- Now supporting 16 field-input bits, and 16-field output bits.
- Added 32-bit up counters.
- Added limited support for using Drag-And-Drop to connect signals. If the string written to a signal is the PVname of some other signal pertaining to the same IP-EP201 module, device support will replace the PVname with the string value of the PV. This connects signals only if the Drag-And-Drop source PV was not numeric, and not an empty string.
- Tighten up notion of legal signal names: leading spaces removed; for outputs, leading decimal digits alse removed. Added PINI=YES to \*Signal PV's, so rules are enforced also boot time.
- Input signal names ending with '\*' are allowed, denote the same signal as the same signal name without a trailing '\*', and indicate that the signal value is to be run through an inverter before being applied to the circuit element.
- use msi to build partially resolved databases, so user can supply their own P and H macros, without having to supply everything else.
- interrupt handling worked only when both edges were enabled
- user can modify interrupt enables, and poll time for non-interrupting field I/O signals, at run time.
- interrupt handler throttles interrupts
- included records for interrupts to drive; implemented GUI for users to manage this.
- added BURT request file to save all of softGlue
- added 8 MHz clock
- decrement user's down-counter N value before writing to FPGA
- Documentation and examples

Release 1-0 (Mar. 30, 2010)
---------------------------

- Initial release

Suggestions and Comments to:   
[Tim Mooney ](mailto:mooney@aps.anl.gov): (mooney@aps.anl.gov)
