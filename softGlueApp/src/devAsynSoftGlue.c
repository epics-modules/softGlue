/* derived from devAsynOctet.c */

/*

	This file provides device support for the stringout record, and uses the
	record's VAL field to determine a number to be written to a register
	implemented in the firmware of an FPGA, which we write to using some other
	asyn driver (drvIp1k125 for now).

    asynSoftGlue
        OUT contains <drvParams> which is passed to asynDrvUser.create (however,
			drvIp1k125 doesn't support this)
        VAL is used to derive the value to be sent.
*/

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <alarm.h>
#include <recGbl.h>
#include <dbAccess.h>
#include <dbDefs.h>
#include <link.h>
#include <epicsPrint.h>
#include <epicsMutex.h>
#include <epicsString.h>
#include <cantProceed.h>
#include <dbCommon.h>
#include <dbScan.h>
#include <callback.h>
#include <stringoutRecord.h>
#include <menuFtype.h>
#include <recSup.h>
#include <devSup.h>

#include <epicsExport.h>
#include "asynDriver.h"
#include "asynDrvUser.h"
#include "asynUInt32Digital.h"
/* Ideally, we would use ...SyncIO, but drvIp1k125 doesn't implement this interface. */
/* #include "asynUInt32DigitalSyncIO.h" */
#include "asynEpicsUtils.h"
#include <epicsExport.h>

typedef struct devPvt {
	dbCommon	*precord;
	asynUser	*pasynUser;
	char		*portName;
	int			addr;
	epicsUInt32	mask;
	asynUInt32Digital *pUInt32Digital;
	void		*UInt32DigitalPvt;
	int			canBlock;
	char		*drvParams;
	CALLBACK	callback;
} devPvt;

static long initCommon(dbCommon *precord, DBLINK *plink, userCallback callback);
static void initDrvUser(devPvt *pdevPvt);
static asynStatus writeIt(asynUser *pasynUser, epicsUInt32 value, epicsUInt32 mask);
/*static asynStatus readIt(asynUser *pasynUser, epicsUInt32 *value);*/
static long processCommon(dbCommon *precord);
static void finish(dbCommon *precord);

static long initSoWrite(stringoutRecord *pso);
static void callbackSoWrite(asynUser *pasynUser);


typedef struct commonDset {
	long		number;
	DEVSUPFUN	dev_report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
	DEVSUPFUN	get_ioint_info;
	DEVSUPFUN	process;
} commonDset;


commonDset asynSoftGlue = {5,0,0,initSoWrite, 0, processCommon};


epicsExportAddress(dset, asynSoftGlue);


static long initCommon(dbCommon *precord, DBLINK *plink, userCallback callback)
{
	devPvt			*pdevPvt;
	asynStatus		status;
	asynUser		*pasynUser;
	asynInterface	*pasynInterface;

	/*
	 * Allocate a devPvt structure for record specific information.  We'll attach
	 * this to the record, so we'll have it when record support calls us, and to
	 * the asynUser structure, so we'll have it when asyn calls us.
	 */
	pdevPvt = callocMustSucceed(1,sizeof(*pdevPvt),"devAsynSoftGlue:initCommon");
	precord->dpvt = pdevPvt;

	/* Create an asynUser, and keep a pointer to it in devPvt. */
	pasynUser = pasynManager->createAsynUser(callback, 0);
	pdevPvt->pasynUser = pasynUser;

	/*
	 * When asyn calls us back to do I/O, the call will go to callbackSoWrite().  The
	 * only context information we're going to get will be the asynUser we just created,
	 * so we'd better attach our devPvt structure to the asynUser structure, and we'll
	 * have to be able to get the record pointer from this, so attach it to devPvt.
	 */
	pasynUser->userPvt = pdevPvt;
	pdevPvt->precord = precord;

	/*
	 * Parse the record's output link to find out who we're supposed to
	 * talk to (and maybe what command we're supposed to send).
	 * This device support supports an INST_IO output link (this is specified
	 * in the .dbd file).  There are two preferred choices for an INST_IO link:
	 *
	 *       field(OUT,"@asyn(portName,addr,timeout) drvParams")
	 * e.g.: field(OUT,"@asyn(myPort,0,0.1) SETGAIN")
	 *
	 * or
	 *
	 *       field(OUT,"@asynMask(portName,addr,mask,timeout) drvParams")
	 * e.g.: field(OUT,"@asynMask(myPort,0,0xff,0.1) SET_ADDR_BITS")
	 *
	 * If we can use one of these syntaxes, we don't have to write parsing code,
	 * because asyn provides code for these two syntaxes in asynEpicsUtils.c,
	 * which we use via asynEpicsUtils.h.  Note that timeout is recorded in the
	 * asynUser for us, and is not returned to us.  drvParams is whatever
	 * follows the closing ')', which may be nothing.
	 */

	/*
	 * example of syntax with no mask value:
	 * status = pasynEpicsUtils->parseLink(pasynUser, plink,
	 *     &pdevPvt->portName, &pdevPvt->addr,&pdevPvt->drvParams);
	 */

	status = pasynEpicsUtils->parseLinkMask(pasynUser, plink, 
		&pdevPvt->portName, &pdevPvt->addr, &pdevPvt->mask,&pdevPvt->drvParams);

	if (status != asynSuccess) {
		printf("%s devAsynSoftGlue:initCommon: error in link %s\n",
			precord->name, pasynUser->errorMessage);
		goto bad;
	}

	/* Connect to the device specified in the link we just parsed */
	status = pasynManager->connectDevice(pasynUser,
		pdevPvt->portName, pdevPvt->addr);
	if (status != asynSuccess) {
		printf("%s devAsynSoftGlue:initCommon: connectDevice failed %s\n",
			precord->name, pasynUser->errorMessage);
		goto bad;
	}

	/*
	 * We're not going to talk directly to the device.  Instead, we're going
	 * to use an existing driver, and talk through the interface that the
	 * driver implements.  The driver will have registered all the interfaces
	 * it implements, so we tell asyn to go through its list of the registered
	 * interfaces for the device we want to talk to, and find one of the type
	 * we'd like to use.  In this case, we want to talk through an interface
	 * of type asynUInt32DigitalType.
	 */
	pasynInterface = pasynManager->findInterface(pasynUser,asynUInt32DigitalType,1);
	if (!pasynInterface) {
		printf("%s devAsynSoftGlue:initCommon: interface %s not found\n",
			precord->name,asynUInt32DigitalType);
		goto bad;
	}

	/*
	 * Attach copies of the interface pointer, and the driver-private pointer,
	 * to our per-record structure.
	 */
	pdevPvt->pUInt32Digital = pasynInterface->pinterface;
	pdevPvt->UInt32DigitalPvt = pasynInterface->drvPvt;
	/* Determine if device can block */
	pasynManager->canBlock(pasynUser, &pdevPvt->canBlock);
	return(0);

bad:
	/*
	 * If there was a problem during the initialization, disable the record
	 * by making it appear to be processing continuously.
	 */
	precord->pact=1;
	return(-1);
}




static void initDrvUser(devPvt *pdevPvt)
{
	asynUser		*pasynUser = pdevPvt->pasynUser;
	asynStatus		status;
	asynInterface	*pasynInterface;
	dbCommon		*precord = pdevPvt->precord;

	/*call drvUserCreate*/
	pasynInterface = pasynManager->findInterface(pasynUser,asynDrvUserType,1);
	if (pasynInterface && pdevPvt->drvParams) {
		asynDrvUser *pasynDrvUser;
		void *drvPvt;

		pasynDrvUser = (asynDrvUser *)pasynInterface->pinterface;
		drvPvt = pasynInterface->drvPvt;
		status = pasynDrvUser->create(drvPvt,pasynUser,pdevPvt->drvParams,0,0);
		if (status!=asynSuccess) {
			printf("%s devAsynSoftGlue drvUserCreate failed %s\n",
				precord->name, pasynUser->errorMessage);
		}
	}
}


static asynStatus writeIt(asynUser *pasynUser, epicsUInt32 value, epicsUInt32 mask)
{
	devPvt		*pdevPvt = (devPvt *)pasynUser->userPvt;
	dbCommon	*precord = pdevPvt->precord;
	asynStatus	status;

	status = pdevPvt->pUInt32Digital->write(pdevPvt->UInt32DigitalPvt, pasynUser, value, mask);
	if (status!=asynSuccess) {
		asynPrint(pasynUser,ASYN_TRACE_ERROR,
			"%s devAsynSoftGlue: writeIt failed %s\n",
			precord->name,pasynUser->errorMessage);
		recGblSetSevr(precord, WRITE_ALARM, INVALID_ALARM);
		return status;
	}
	asynPrint(pasynUser,ASYN_TRACEIO_DEVICE,
		"%s devAsynSoftGlue:writeIt: value=%lu\n", precord->name, value);
	return status;
}


static long processCommon(dbCommon *precord)
{
	devPvt		*pdevPvt = (devPvt *)precord->dpvt;
	asynStatus	status;
	
	if (pdevPvt->canBlock) precord->pact = 1;
	status = pasynManager->queueRequest(
		pdevPvt->pasynUser, asynQueuePriorityMedium, 0.0);
	if ((status==asynSuccess) && pdevPvt->canBlock) return 0;
	if (pdevPvt->canBlock) precord->pact = 0;
	if (status != asynSuccess) {
		asynPrint(pdevPvt->pasynUser, ASYN_TRACE_ERROR,
			"%s devAsynSoftGlue:processCommon: error queuing request %s\n", 
			precord->name,pdevPvt->pasynUser->errorMessage);
		recGblSetSevr(precord,READ_ALARM,INVALID_ALARM);
	}
	return(0);
}

static void finish(dbCommon *pr)
{
	devPvt	*pPvt = (devPvt *)pr->dpvt;

	if (pr->pact) callbackRequestProcessCallback(&pPvt->callback,pr->prio,pr);
}



static long initSoWrite(stringoutRecord *pso)
{
	asynStatus	status;
	devPvt		*pdevPvt;

	status = initCommon((dbCommon *)pso,&pso->out,callbackSoWrite);
	if (status!=asynSuccess) return 0;
	pdevPvt = (devPvt *)pso->dpvt;
	initDrvUser((devPvt *)pso->dpvt);
	return 0;
}

static void callbackSoWrite(asynUser *pasynUser)
{
	devPvt			*pdevPvt = (devPvt *)pasynUser->userPvt;
	stringoutRecord	*pso = (stringoutRecord *)pdevPvt->precord;
	asynStatus		status;
	epicsUInt32		value=0, mask=0x2f;

	/* See if string value begins with a number or something else */
	if ((strlen(pso->val) > 0) && isdigit((int)pso->val[0])) {
		/*
		 *It's a number.  Cause FPGA register to connect user-write bit
		 * (which is wired to mux input 0) to the device, by setting the mux
		 * address to zero, and writing '0' or '1' to register bit 5.
		 */
		if (pso->val[0] == '0') {
			value = 0;
		} else {
			value = 0x20;
		}
	} else {
		/*
		 * It's not a number.  Interpret string as a signal name.  For now, signals
		 * are named 'B0' through 'B14'.
		 */
		if (strlen(pso->val) > 1) {
			/* e.g., the string "B13" produces the integer 13 */
			value = atoi(&pso->val[1]);
		} else {
			value = 0;
		}
	}
	/*printf("devAsynSoftGlue:callbackSoWrite: writing 0x%x 0x%x\n", value, mask);*/
	status = writeIt(pasynUser, value, mask);
	finish((dbCommon *)pso);
}

