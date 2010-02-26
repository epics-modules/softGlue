/* drvIp1k125.cc

    Author: Marty Smith

    This is the driver for the Acromag IP1K125 series of reconfigurable
	digital I/O IP modules.

    Modifications:
    12-Aug-2008  MLS  Initial release based on the IpUnidig driver
*/

/* System includes */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* EPICS includes */
#include <drvIpac.h>
#include <errlog.h>
#include <ellLib.h>
#include <devLib.h>
#include <cantProceed.h>
#include <epicsTypes.h>
#include <epicsThread.h>
#include <epicsMutex.h>
#include <epicsString.h>
#include <epicsExit.h>
#include <epicsMessageQueue.h>
#include <epicsExport.h>
#include <iocsh.h>
#include <asynDriver.h>
#include <asynUInt32Digital.h>
#include <asynInt32.h>
#include <epicsInterrupt.h>

#define APS_ID	0x53

#define MAX_MESSAGES 1000

typedef struct ip1k125regs {
    epicsUInt16 controlRegister;    	/* control register            */
    epicsUInt16 writeDataRegister;  	/* 16-bit data write/read      */
    epicsUInt16 readDataRegister;   	/* Input Data Read register    */
    epicsUInt16 risingIntStatus;   	/* Rising Int Status Register  */
    epicsUInt16 risingIntEnable;   	/* Rising Int Enable Reg       */
    epicsUInt16 fallingIntStatus;  	/* Falling Int Status Register */
    epicsUInt16 fallingIntEnable;  	/* Falling Int Enable Register */
} ip1k125Registers;

typedef struct {
    epicsUInt32 bits;
    epicsUInt32 interruptMask;
} ip1k125Message;

typedef struct {
    unsigned char manufacturer;
    unsigned char model;
    char *portName;
    asynUser *pasynUser;
    epicsUInt32 oldBits;
    epicsUInt32 risingMask;
    epicsUInt32 fallingMask;
    volatile ip1k125Registers *regs;
    double pollTime;
    epicsMessageQueueId msgQId;
    int messagesSent;
    int messagesFailed;
    asynInterface common;
    asynInterface uint32D;
    asynInterface int32;
    void *interruptPvt;
    int intVector;
    int key;
} drvIp1k125Pvt;

/*
 * asynCommon interface
 */
static void report                 	(void *drvPvt, FILE *fp, int details);
static asynStatus connect          	(void *drvPvt, asynUser *pasynUser);
static asynStatus disconnect       	(void *drvPvt, asynUser *pasynUser);

static const struct asynCommon ip1k125Common = {
    report,
    connect,
    disconnect
};

/*
 * asynUInt32Digital interface - we only implement part of this interface.
 */
static asynStatus readUInt32D      	(void *drvPvt, asynUser *pasynUser,
                                    	epicsUInt32 *value, epicsUInt32 mask);
static asynStatus writeUInt32D     	(void *drvPvt, asynUser *pasynUser,
                                    	epicsUInt32 value, epicsUInt32 mask);

/* default implementations are provided by asynUInt32DigitalBase. */
static struct asynUInt32Digital ip1k125UInt32D = {
    writeUInt32D, /* write */
    readUInt32D,  /* read */
    NULL,         /* setInterrupt: default does nothing */
    NULL,         /* clearInterrupt: default does nothing */
    NULL,         /* getInterrupt: default does nothing */
    NULL,         /* registerInterruptUser: default adds user to pollerThread's clientList. */
    NULL          /* cancelInterruptUser: default removes user from pollerThread's clientList. */
};


/*
 * asynInt32 interface - we don't implement any of this interface.
 * We just accept the default implementations provided by asynInt32Base.
 */

/* These are private functions */
static void pollerThread           	(drvIp1k125Pvt *pPvt);
static void intFunc                	(void *); /* Interrupt function */
static void rebootCallback         	(void *);



/* Initialize IP module */
int initIp1k125(const char *portName, ushort_t carrier, ushort_t slot,
                 int msecPoll, int dataDir, int sopcOffset, int sopcVector,
    	    	    int risingMask, int fallingMask)
{
    drvIp1k125Pvt *pPvt;
    ipac_idProm_t *id;
    unsigned char manufacturer;
    unsigned char model;
    int status;

    pPvt = callocMustSucceed(1, sizeof(*pPvt), "initIp1k125");
    pPvt->portName = epicsStrDup(portName);

    /* Default of 100 msec */
    if (msecPoll == 0) msecPoll = 100;
    pPvt->pollTime = msecPoll / 1000.;
    pPvt->msgQId = epicsMessageQueueCreate(MAX_MESSAGES, 
                                           sizeof(ip1k125Message));

    if (ipmCheck(carrier, slot)) {
       errlogPrintf("initIp1k125: bad carrier or slot\n");
       return(-1);
    }
    
	/* Set up ID and I/O space addresses for IP module */
    id = (ipac_idProm_t *) ipmBaseAddr(carrier, slot, ipac_addrID);
    pPvt->regs = (ip1k125Registers *) (ipmBaseAddr(carrier, slot, ipac_addrIO)+sopcOffset);
    pPvt->intVector = sopcVector;

    manufacturer = id->manufacturerId & 0xff;
    model = id->modelId & 0xff;

    pPvt->manufacturer = manufacturer;
    pPvt->model = model;

    /* Link with higher level routines */
    pPvt->common.interfaceType = asynCommonType;
    pPvt->common.pinterface  = (void *)&ip1k125Common;
    pPvt->common.drvPvt = pPvt;
    pPvt->uint32D.interfaceType = asynUInt32DigitalType;
    pPvt->uint32D.pinterface  = (void *)&ip1k125UInt32D;
    pPvt->uint32D.drvPvt = pPvt;
    status = pasynManager->registerPort(pPvt->portName,
                                        1, /* multiDevice, cannot block */
                                        1, /* autoconnect */
                                        0, /* medium priority */
                                        0);/* default stack size */
    if (status != asynSuccess) {
        errlogPrintf("initIp1k125 ERROR: Can't register port\n");
        return(-1);
    }
    status = pasynManager->registerInterface(pPvt->portName,&pPvt->common);
    if (status != asynSuccess) {
        errlogPrintf("initIp1k125 ERROR: Can't register common.\n");
        return(-1);
    }
    status = pasynUInt32DigitalBase->initialize(pPvt->portName, &pPvt->uint32D);
    if (status != asynSuccess) {
        errlogPrintf("initIp1k125 ERROR: Can't register UInt32Digital.\n");
        return(-1);
    }
    pasynManager->registerInterruptSource(pPvt->portName, &pPvt->uint32D,
                                          &pPvt->interruptPvt);

    status = pasynInt32Base->initialize(pPvt->portName,&pPvt->int32);
    if (status != asynSuccess) {
        errlogPrintf("initIp1k125 ERROR: Can't register Int32.\n");
        return(-1);
    }

     /* Create asynUser for asynTrace */
    pPvt->pasynUser = pasynManager->createAsynUser(0, 0);
    pPvt->pasynUser->userPvt = pPvt;

    /* Connect to device */
    status = pasynManager->connectDevice(pPvt->pasynUser, pPvt->portName, 0);
    if (status != asynSuccess) {
        errlogPrintf("initIp1k125, connectDevice failed %s\n",
                     pPvt->pasynUser->errorMessage);
        return(-1);
    }

    /* Set up the control register */	
    pPvt->regs->controlRegister = dataDir;  /* Set Data Direction Reg IP1k125*/  
    pPvt->risingMask = risingMask;
    pPvt->fallingMask = fallingMask;
	
    /* Start the thread to poll and handle interrupt callbacks to device support */
    epicsThreadCreate("ip1k125",
                      epicsThreadPriorityHigh,
                      epicsThreadGetStackSize(epicsThreadStackBig),
                      (EPICSTHREADFUNC)pollerThread, 
					  pPvt);
					  
    /* Interrupt support */
    /* If the risingMask and the fallingMask are zero, don't bother with interrupts, 
     * since the user probably didn't pass this parameter to Ip1k125::init()
     */
    if (pPvt->risingMask || pPvt->fallingMask) {
	
	/* For COS interrupts both the rising and falling masks must be the same 
		indicating COS for desired bit
	*/
	pPvt->regs->risingIntStatus = risingMask;
	pPvt->regs->fallingIntStatus = fallingMask;
		
	/* Rising Edge Triggered interrupts */
	if (pPvt->risingMask) { 
		pPvt->regs->risingIntEnable = pPvt->risingMask;
	}
	/* Falling Edge Triggered interrupts */
	if (pPvt->fallingMask) {  
		pPvt->regs->fallingIntEnable = pPvt->fallingMask;
	}
	if (devConnectInterruptVME(pPvt->intVector, intFunc, (void *)pPvt)) {
		errlogPrintf("ip1k125 interrupt connect failure\n");
		return(-1);
	}
	/* Enable IPAC module interrupts and set module status. */
	ipmIrqCmd(carrier, slot, 0, ipac_irqEnable);
	ipmIrqCmd(carrier, slot, 0, ipac_statActive);
    }
    epicsAtExit(rebootCallback, pPvt);
    return(0);
}

/* readUInt32D
 *	Reads the data registers of the IP1k125 Acromag IP module
 *	Even though this is a 16-bit read.
 *	
 */
static asynStatus readUInt32D(void *drvPvt, asynUser *pasynUser, epicsUInt32 *value,
                       epicsUInt32 mask)
{
    drvIp1k125Pvt *pPvt = (drvIp1k125Pvt *)drvPvt;
	
    *value = 0;
    *value = (epicsUInt32) (pPvt->regs->readDataRegister & mask);
    asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
              "drvIp1k125::readUInt32D, value=0x%X, mask=0x%X\n", *value,mask);
    return(asynSuccess);
}

/* writeUInt32D
 *	This method allows write access to I/O registers based on the
 *	contents of the data direction register for each of the sopc
 *	components.
 *	This register determines which I/O bits are inputs and which 
 *	are	outputs. If the LSB of the byte = 1 then the byte is an output.	
 *	When the module is powered up all I/O registers default to input.
 */
static asynStatus writeUInt32D(void *drvPvt, asynUser *pasynUser, epicsUInt32 value,
                        epicsUInt32 mask)
{
    drvIp1k125Pvt *pPvt = (drvIp1k125Pvt *)drvPvt;

    pPvt->regs->writeDataRegister = (pPvt->regs->writeDataRegister & ~mask) | (value & mask);	
    asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
              "drvIp1k125::write, value=0x%X, mask=0x%X\n",value, mask);
    return(asynSuccess);
}

/*
 * This is the interrupt-service routine associated with the interrupt
 * vector pPvt->intVector supplied in the drvPvt structure.
 */
static void intFunc(void *drvPvt)
{
    drvIp1k125Pvt *pPvt = (drvIp1k125Pvt *)drvPvt;
    epicsUInt32 inputs=0, pendingLow, pendingHigh, pendingMask;
    ip1k125Message msg;

	/* Make sure interrupt is from this hardware otherwise just leave */
    if (pPvt->regs->risingIntStatus || pPvt->regs->fallingIntStatus) {
    	pendingLow = pPvt->regs->fallingIntStatus;
    	pendingHigh = pPvt->regs->risingIntStatus;
    	pendingMask = pendingLow | pendingHigh;

    /* Read the current input */
    	inputs = (epicsUInt16) pPvt->regs->readDataRegister;
    	msg.bits = inputs;
    	msg.interruptMask = pendingMask;

    	if (epicsMessageQueueTrySend(pPvt->msgQId, &msg, sizeof(msg)) == 0)
    	    pPvt->messagesSent++;
    	else
    	    pPvt->messagesFailed++;
    /* Clear the interrupts */
    	pPvt->regs->risingIntStatus = pendingHigh;
    	pPvt->regs->fallingIntStatus = pendingLow;
		
	/* Generate dummy read cycle for PPC */
    	pendingHigh = pPvt->regs->risingIntStatus;
    }
}


/* This function runs in a separate thread.  It waits for the poll
 * time, or an interrupt, whichever comes first.  If the bits read from
 * the ip1k125 have changed then it does callbacks to all clients that
 * have registered with registerInterruptUser */

static void pollerThread(drvIp1k125Pvt *pPvt)
{
    epicsUInt32 newBits, changedBits, interruptMask=0;
    ip1k125Message msg;
    ELLLIST *pclientList;
    interruptNode *pnode;
    asynUInt32DigitalInterrupt *pUInt32D;

    while(1) {      
        /*  Wait for an interrupt or for the poll time, whichever comes first */
        if (epicsMessageQueueReceiveWithTimeout(pPvt->msgQId, 
                                                &msg, sizeof(msg), 
                                                pPvt->pollTime) == -1) {
            /* The wait timed out, so there was no interrupt, so we need
             * to read the bits.  If there was an interrupt the bits got
             * set in the interrupt routine
			 */
    	    readUInt32D(pPvt, pPvt->pasynUser, &newBits, 0xffff);
        } else {
            newBits = msg.bits;
            interruptMask = msg.interruptMask;
            asynPrint(pPvt->pasynUser, ASYN_TRACE_FLOW,
                      "drvIp1k125::pollerThread, got interrupt\n");
        }
        asynPrint(pPvt->pasynUser, ASYN_TRACEIO_DRIVER,
                  "drvIp1k125::pollerThread, bits=%x\n", newBits);

        /* We detect change both from interruptMask (which only works for
         * hardware interrupts) and changedBits, which works for polling */
        changedBits = newBits ^ pPvt->oldBits;
        interruptMask = interruptMask | changedBits;
        if (interruptMask) {
            pPvt->oldBits = newBits;
            pasynManager->interruptStart(pPvt->interruptPvt, &pclientList);
            pnode = (interruptNode *)ellFirst(pclientList);
            while (pnode) {
                pUInt32D = pnode->drvPvt;
                if (pUInt32D->mask & interruptMask) {
                    asynPrint(pPvt->pasynUser, ASYN_TRACE_FLOW,
                              "drvIp1k125::pollerThread, calling client %p"
                              " mask=%x, callback=%p\n",
                              pUInt32D, pUInt32D->mask, pUInt32D->callback);
                    pUInt32D->callback(pUInt32D->userPvt, pUInt32D->pasynUser,
                                       pUInt32D->mask & newBits);
                }
                pnode = (interruptNode *)ellNext(&pnode->node);
            }
            pasynManager->interruptEnd(pPvt->interruptPvt);
        }
    }
}


static void rebootCallback(void *drvPvt)
{
    drvIp1k125Pvt *pPvt = (drvIp1k125Pvt *)drvPvt;
	
    int key = epicsInterruptLock();
    /* Disable interrupts first so no interrupts during reboot */
    pPvt->regs->risingIntEnable = 0;
    pPvt->regs->fallingIntEnable = 0;
    epicsInterruptUnlock(key);
}

/* asynCommon routines */

/* Report  parameters */
static void report(void *drvPvt, FILE *fp, int details)
{
    drvIp1k125Pvt *pPvt = (drvIp1k125Pvt *)drvPvt;
    interruptNode *pnode;
    ELLLIST *pclientList;

    fprintf(fp, "drvIp1k125 %s: connected at base address %p\n",
            pPvt->portName, pPvt->regs);
    if (details >= 1) {
        fprintf(fp, "  controlRegister=%x\n", pPvt->regs->controlRegister);
        fprintf(fp, "  risingMask=%x\n", pPvt->risingMask);
        fprintf(fp, "  risingIntEnable=%x\n", pPvt->regs->risingIntEnable);
        fprintf(fp, "  fallingMask=%x\n", pPvt->fallingMask);
        fprintf(fp, "  fallingIntEnable=%x\n", pPvt->regs->fallingIntEnable);
        fprintf(fp, "  messages sent OK=%d; send failed (queue full)=%d\n",
                pPvt->messagesSent, pPvt->messagesFailed);
    	pasynManager->interruptStart(pPvt->interruptPvt, &pclientList);
        pnode = (interruptNode *)ellFirst(pclientList);
        while (pnode) {
            asynUInt32DigitalInterrupt *pUInt32D = pnode->drvPvt;
            fprintf(fp, "  uint32Digital callback client address=%p, mask=%x\n",
                    pUInt32D->callback, pUInt32D->mask);
            pnode = (interruptNode *)ellNext(&pnode->node);
        }
        pasynManager->interruptEnd(pPvt->interruptPvt);
     }
}

/* Connect */
static asynStatus connect(void *drvPvt, asynUser *pasynUser)
{
    pasynManager->exceptionConnect(pasynUser);
    return(asynSuccess);
}

/* Connect */
static asynStatus disconnect(void *drvPvt, asynUser *pasynUser)
{
    pasynManager->exceptionDisconnect(pasynUser);
    return(asynSuccess);
}

/* iocsh functions */
static const iocshArg initArg0 = { "Server name",iocshArgString};
static const iocshArg initArg1 = { "Carrier",iocshArgInt};
static const iocshArg initArg2 = { "Slot",iocshArgInt};
static const iocshArg initArg3 = { "msecPoll",iocshArgInt};
static const iocshArg initArg4 = { "Data Dir Reg",iocshArgInt};
static const iocshArg initArg5 = { "sopc Offset Addr",iocshArgInt};
static const iocshArg initArg6 = { "sopc Vector Addr",iocshArgInt};
static const iocshArg initArg7 = { "Rising Edge Mask",iocshArgInt};
static const iocshArg initArg8 = { "Falling Edge Mask",iocshArgInt};
static const iocshArg * const initArgs[9] = {&initArg0,
                                             &initArg1,
                                             &initArg2,
                                             &initArg3,
                                             &initArg4,
                                             &initArg5,
                                             &initArg6,
                                             &initArg7,
                                             &initArg8};
static const iocshFuncDef initFuncDef = {"initIp1k125",9,initArgs};
static void initCallFunc(const iocshArgBuf *args)
{
    initIp1k125(args[0].sval, args[1].ival, args[2].ival,
                args[3].ival, args[4].ival, args[5].ival,
		args[6].ival, args[7].ival, args[8].ival);
}
void ip1k125Register(void)
{
    iocshRegister(&initFuncDef,initCallFunc);
}

epicsExportRegistrar(ip1k125Register);

