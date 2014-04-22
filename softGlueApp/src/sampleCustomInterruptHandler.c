/* Demonstrate softGlue custom interrupt routine to write new values to DivByN-3_N
 */

#include "drvIP_EP201.h"
#include <epicsExport.h>
#include <iocsh.h>

epicsUInt16 *addrMSW, *addrLSW;

#define MAX_VALUES 1000
int sampleCustomInterruptValues[MAX_VALUES];
int thisValue = 0;

/* When an interrupt matching conditions specified by sampleCustomInterruptPrepare() occurs,
 * we'll get called with the bitmask that generated the interrupt.
 */
void sampleCustomInterruptRoutine(ushort mask) {
	*addrMSW = (sampleCustomInterruptValues[thisValue]-1)/65536;
	*addrLSW = (sampleCustomInterruptValues[thisValue]-1)%65536;
	if (++thisValue >= MAX_VALUES) thisValue = 0;
	/*logMsg("sampleCustomInterruptRoutine(0x%x)\n", mask);*/
}

/* int sampleCustomInterruptPrepare(int carrier, int slot, int sopcAddress, int mask)
 * carrier: 	IP carrier (e.g., 0)
 * slot:		IP slot (e.g., 0)
 * sopcAddress: address of fieldIO_registerSet (I/O register) of the interrupt bit to which
 *				we want to respond (e.g., 0x800000, or 0x800010, or 0x800020)
 * mask:		bit(s) to which we want to respond (e.g., 0x1)
 */
int sampleCustomInterruptPrepare(int carrier, int slot, int sopcAddress, int mask) {
	int i;
	
	/* Get the address of the divByN N register.
	 * The sopc addresses 0x800720 and 0x800718 are from softGlue_FPGAContent.substitutions.
	 * They specify the DivByN-3_N register to which we want to write.  It's a 32-bit register,
	 * so we'll need to write to the MSW and LSW parts separately.
	 */
	addrMSW = softGlueCalcSpecifiedRegisterAddress((ushort_t) carrier, (ushort_t) slot, 0x800720);
	addrLSW = softGlueCalcSpecifiedRegisterAddress((ushort_t) carrier, (ushort_t) slot, 0x800718);

	/* Tell softGlue to call sampleCustomInterruptRoutine() when its interrupt-service routine handles an interrupt
	 * from the specified carrier, slot, sopcAddress (I/O register), and bit (mask).  This also
	 * tells softGlue to not execute any output links that might also have been programmed to
	 * execute in response to this interrupt.
	 */
	softGlueRegisterInterruptRoutine(carrier, slot, sopcAddress, mask, sampleCustomInterruptRoutine);

	/* prepare example data for sampleCustomInterruptRoutine() */
	for (i=0; i<MAX_VALUES; i++) {
		sampleCustomInterruptValues[i] = 10+10*(i%2);
	}
	return(0);
}

/* int sampleCustomInterruptPrepare(int carrier, int slot, int sopcAddress, int mask) */
static const iocshArg myArg1 = { "Carrier Number",  iocshArgInt};
static const iocshArg myArg2 = { "Slot Number", 	iocshArgInt};
static const iocshArg myArg3 = { "sopcAddress", iocshArgInt};
static const iocshArg myArg4 = { "mask",	iocshArgInt};
static const iocshArg * const myArgs[4] = {&myArg1, &myArg2, &myArg3, &myArg4};
static const iocshFuncDef myFuncDef = {"sampleCustomInterruptPrepare",4,myArgs};
static void myCallFunc(const iocshArgBuf *args) {
	sampleCustomInterruptPrepare(args[0].ival, args[1].ival, args[2].ival, args[3].ival);
}

void sampleCustomInterruptRegistrar(void)
{
	iocshRegister(&myFuncDef, myCallFunc);
}

epicsExportRegistrar(sampleCustomInterruptRegistrar);
