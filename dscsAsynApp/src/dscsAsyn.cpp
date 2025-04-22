#include <stdio.h>
#include <iocsh.h>
#include <epicsExport.h>
#include <epicsString.h>
#include <epicsThread.h>
#include <asynOctetSyncIO.h>
#include <string.h>

#include <stdlib.h>
#include <unistd.h>
#include "dscs.h" // vendor supplied library

#include "dscsAsyn.h"

using namespace std;

struct Context {
  const char * _tag;
  unsigned int _expectedIndex;
  double       _timestamp;
};

static struct Context contextRel = { "Rel", 0, -1 };
static struct Context contextAbs = { "Abs", 0, -1 };

static const char * getMessage( int code )
{
  switch( code ) {
  case DSCS_Ok:           return "";
  case DSCS_Error:        return "Unspecified error";
  case DSCS_Timeout:      return "Communication timeout";
  case DSCS_NotConnected: return "No active connection to device";
  case DSCS_DriverError:  return "Error in comunication with driver";
  case DSCS_DeviceLocked: return "Device is already in use by other";
  case DSCS_Unknown:      return "Unknown error";
  case DSCS_NoDevice:     return "Invalid device number in function call";
  case DSCS_ParamOutOfRg: return "A parameter exceeds the allowed range";
  default:               return "Unknown error code";
  }
}

void dscsAsyn::checkError(const char * context, int code)
{
  if ( code != DSCS_Ok ) {
    printf( "Error calling %s: %s\n", context, getMessage( code ) );
  }
}

void dscsAsyn::pollAnalogIn()
{

  int analog_in_nfo_sg;
  DSCS_Axis axis = DSCS_AxisX;

    int errorCode = DSCS_getNFO_SG(this->deviceNo, axis, &analog_in_nfo_sg);
    checkError( "DSCS_getNFO_SG", errorCode);

    printf( "%9.1d \n", analog_in_nfo_sg);

    setDoubleParam(nfo_sg_rbv_, (double)analog_in_nfo_sg);

}

// This needs to come before the dscs constructor to avoid compiler errors
static void pollerThreadC(void * pPvt)
{
  dscsAsyn *pdscsAsyn = (dscsAsyn*)pPvt;
  pdscsAsyn->pollerThread();
}

dscsAsyn::dscsAsyn(const char *portName, const char *dscsAsynPortName, int dscsId) : asynPortDriver(portName, MAX_CONTROLLERS,
		asynInt32Mask | asynFloat64Mask | asynDrvUserMask | asynOctetMask,
		asynInt32Mask | asynFloat64Mask | asynOctetMask,
		ASYN_MULTIDEVICE | ASYN_CANBLOCK, 1, /* ASYN_CANBLOCK=0, ASYN_MULTIDEVICE=1, autoConnect=1 */
		0, 0), /* Default priority and stack size */
    pollTime_(DEFAULT_POLL_TIME)
{
	static const char *functionName = "dscsAsyn";
    asynStatus status;

    	this->deviceId = dscsId;
	
	createParam(get_NFO_SG_string,   	asynParamFloat64, &nfo_sg_rbv_);

	// Force the device to connect now
	connect(this->pasynUserSelf);

	// status = pasynOctetSyncIO->connect(dscsAsynPortName, 0, &pasynUserdscsAsyn_, NULL);
	
  //setIntegerParam(variable_, 1);
  
	// Start the poller
  epicsThreadCreate("dscsAsynPoller", 
      epicsThreadPriorityLow,
      epicsThreadGetStackSize(epicsThreadStackMedium),
      (EPICSTHREADFUNC)pollerThreadC,
      this);
	
  //epicsThreadSleep(5.0);
}

asynStatus dscsAsyn::connect(asynUser *pasynUser)
{
	asynStatus status;
	static const char *functionName = "connect";
	int errorCode;
	unsigned int devCount; // number of dscs devices available
	unsigned int devNo; 
	
	printf("devCount before setting: %d\n", devCount);


    	asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
        	"%s:%s: Connecting...\n", driverName, functionName);

	// discover available devices. IfAll - both usb and ethernet
  	errorCode = DSCS_discover(IfAll, &devCount);

	printf("errorCode after discover: %d\n", devCount);

	printf("devCount after setting: %d\n", devCount);

  	if (devCount <= 0) {
		printf( "No devices found\n" );
		return asynError;
	}

	// search through available devices for desired ID
  	for (devNo = 0; devNo < devCount; devNo++) {
		int id = 0;
		char addr[20], serialNo[20];
    		errorCode = DSCS_getDeviceInfo(devNo, &id, serialNo, addr);
    		checkError("DSCS_getDeviceInfo", errorCode);
    		printf( "Device found: No=%d Id=%d SN=%s Addr=%s\n", devNo, id, serialNo, addr );   
		if (id == this->deviceId) {
			printf("connecting to device with ID %d\n", id);
			this->deviceNo = devNo;
			break;
		}
		// desired ID not found:
		if (devNo == devCount-1) {
			printf("ID not found\n");
			return asynError;
		}
	}
	

	this->lock();
	errorCode = DSCS_disconnect(this->deviceNo); // disconnect first
	errorCode = DSCS_connect(this->deviceNo);
	this->unlock();

  	checkError("DSCS_connect", errorCode);

    /* We found the controller and everything is OK.  Signal to asynManager that we are connected. */
    status = pasynManager->exceptionConnect(this->pasynUserSelf);
    if (status) {
        asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
            "%s:%s: error calling pasynManager->exceptionConnect, error=%s\n",
            driverName, functionName, pasynUserSelf->errorMessage);
        return asynError;
    }

 	return asynSuccess;
}

asynStatus dscsAsyn::disconnect(asynUser *pasynUser)
{
	asynStatus status;
	static const char *functionName = "disconnect";
	int errorCode;

    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
        "%s:%s: Disconnecting...\n", driverName, functionName);

	this->lock();
  	errorCode = DSCS_disconnect(this->deviceNo);
	this->unlock();

  	checkError("DSCS_disconnect", errorCode);

    /* We found the controller and everything is OK.  Signal to asynManager that we are connected. */
    status = pasynManager->exceptionDisconnect(this->pasynUserSelf);
    if (status) {
        asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
            "%s:%s: error calling pasynManager->exceptionDisonnect, error=%s\n",
            driverName, functionName, pasynUserSelf->errorMessage);
        return asynError;
    }

 	return asynSuccess;
}



dscsAsyn::~dscsAsyn()
{
	// Force the controller to disconnect
	disconnect(this->pasynUserSelf);
}

/*
 * 
 * poller
 * 
 */
void dscsAsyn::pollerThread()
{
  /* This function runs in a separate thread.  It waits for the poll time. */
  static const char *functionName = "pollerThread";

  // Other variable declarations
    asynStatus comStatus;
  
  while (1)
  {
    
    lock();

    // pollPositions();
    pollAnalogIn();

    unlock();
    callParamCallbacks();
    epicsThreadSleep(pollTime_);

  }
}

/*
 *
 * writeInt32
 *
 */
// this function not needed currently
asynStatus dscsAsyn::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
	int function = pasynUser->reason;
	asynStatus status = asynSuccess;
	static const char *functionName = "writeInt32";

    asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, 
			"%s:%s, port %s, function = %d\n",
			driverName, functionName, this->portName, function);

	setIntegerParam(function, value);

	callParamCallbacks();

	if (status == 0) {
		asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, 
             "%s:%s, port %s, wrote %d\n",
             driverName, functionName, this->portName, value);
	} else {
		asynPrint(pasynUser, ASYN_TRACE_ERROR, 
             "%s:%s, port %s, ERROR writing %d, status=%d\n",
             driverName, functionName, this->portName, value, status);
	}
	
	return (status==0) ? asynSuccess : asynError;
}

void dscsAsyn::report(FILE *fp, int details)
{
    asynPortDriver::report(fp, details);
    fprintf(fp, "* Port: %s\n", 
        this->portName);
    fprintf(fp, "\n");
}

extern "C" int dscsAsynConfig(const char *portName, const char *dscsAsynPortName, int dscsId)
{
    dscsAsyn *pdscsAsyn = new dscsAsyn(portName, dscsAsynPortName, dscsId);
    pdscsAsyn = NULL; /* This is just to avoid compiler warnings */
    return(asynSuccess);
}

static const iocshArg dscsAsynArg0 = { "Port name", iocshArgString};
static const iocshArg dscsAsynArg1 = { "dscsAsyn port name", iocshArgString};
static const iocshArg dscsAsynArg2 = { "Device ID", iocshArgInt};
static const iocshArg * const dscsAsynArgs[3] = {&dscsAsynArg0, &dscsAsynArg1, &dscsAsynArg2};
static const iocshFuncDef dscsAsynFuncDef = {"dscsAsynConfig", 3, dscsAsynArgs};
static void dscsAsynCallFunc(const iocshArgBuf *args)
{
    dscsAsynConfig(args[0].sval, args[1].sval, args[2].ival);
}

void drvdscsAsynRegister(void)
{
    iocshRegister(&dscsAsynFuncDef, dscsAsynCallFunc);
}

extern "C" {
    epicsExportRegistrar(drvdscsAsynRegister);
}
