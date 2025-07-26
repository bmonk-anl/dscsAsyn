// TODO: 
// data callback functions:
// 	DSCS_setDataCallback()
// 	DSCS_setDataOutputEnabled()
// 
// controller/operation
// 	DSCS_resetSetpointModulationPhase()
// 	DSCS_resetPIController()
// 	DSCS_startTrajectory()
//
// transfer some int to float variables
// make writeFloat64 function
//
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

#define INT_MAX 2147483647

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

// uses following functions from dscs.h:
// DSCS_getOSA_PS
// DSCS_getBS_PS
// DSCS_getNFO_PS
// DSCS_getSAM_PS
// DSCS_getNFO_SG
// DSCS_getSAM_CP_D
// DSCS_getXZ_ZX
// DSCS_getNFO
// DSCS_getSAM
//
// DSCS_API int WINCC DSCS_getAUX_DAC(const unsigned int devNo,
//                                    const DSCS_AUX_ADC aux,
//                                    int               *value);
// DSCS_API int WINCC DSCS_getAUX_ADC(const unsigned int devNo,
//                                    const DSCS_AUX_ADC aux,
//                                    int               *value);


// TODO: implement multi axis
inline void dscsAsyn::pollAnalogIn()
{

}

// This needs to come before the dscs constructor to avoid compiler errors
static void pollerThreadC(void * pPvt)
{
  dscsAsyn *pdscsAsyn = (dscsAsyn*)pPvt;
  pdscsAsyn->pollerThread();
}

dscsAsyn::dscsAsyn(const char *portName, const char *dscsAsynPortName, int dscsId) : asynPortDriver(portName, MAX_CONTROLLERS,
		asynInt32Mask | asynFloat64Mask | asynDrvUserMask | asynOctetMask | asynFloat64ArrayMask | asynInt32ArrayMask,
		asynInt32Mask | asynFloat64Mask | asynOctetMask | asynFloat64ArrayMask | asynInt32ArrayMask,
		ASYN_MULTIDEVICE | ASYN_CANBLOCK, 1, /* ASYN_CANBLOCK=0, ASYN_MULTIDEVICE=1, autoConnect=1 */
		0, 0), /* Default priority and stack size */
    pollTime_(DEFAULT_POLL_TIME)
{
	static const char *functionName = "dscsAsyn";
    asynStatus status;

    	this->deviceId = dscsId;

	// OSA_PS (x and y)
	createParam("OSA_PS_X",         asynParamInt32, &OSA_PS_[0]);
	createParam("OSA_PS_Y",         asynParamInt32, &OSA_PS_[1]);
	createParam("OSA_PS_RBV_X",     asynParamInt32, &OSA_PS_rbv_[0]);
	createParam("OSA_PS_RBV_Y",     asynParamInt32, &OSA_PS_rbv_[1]);
	
	// BS_PS (x and y)
	createParam("BS_PS_X",          asynParamInt32, &BS_PS_[0]);
	createParam("BS_PS_Y",          asynParamInt32, &BS_PS_[1]);
	createParam("BS_PS_RBV_X",      asynParamInt32, &BS_PS_rbv_[0]);
	createParam("BS_PS_RBV_Y",      asynParamInt32, &BS_PS_rbv_[1]);
	
	// AUX_DAC (0-3)
	createParam("AUX_DAC_0",        asynParamInt32, &AUX_DAC_[0]);
	createParam("AUX_DAC_1",        asynParamInt32, &AUX_DAC_[1]);
	createParam("AUX_DAC_2",        asynParamInt32, &AUX_DAC_[2]);
	createParam("AUX_DAC_3",        asynParamInt32, &AUX_DAC_[3]);
	createParam("AUX_DAC_RBV_0",    asynParamInt32, &AUX_DAC_rbv_[0]);
	createParam("AUX_DAC_RBV_1",    asynParamInt32, &AUX_DAC_rbv_[1]);
	createParam("AUX_DAC_RBV_2",    asynParamInt32, &AUX_DAC_rbv_[2]);
	createParam("AUX_DAC_RBV_3",    asynParamInt32, &AUX_DAC_rbv_[3]);
	
	// NFO_PS (x, y, z)
	createParam("NFO_PS_X",         asynParamInt32, &NFO_PS_[0]);
	createParam("NFO_PS_Y",         asynParamInt32, &NFO_PS_[1]);
	createParam("NFO_PS_Z",         asynParamInt32, &NFO_PS_[2]);
	createParam("NFO_PS_RBV_X",     asynParamInt32, &NFO_PS_rbv_[0]);
	createParam("NFO_PS_RBV_Y",     asynParamInt32, &NFO_PS_rbv_[1]);
	createParam("NFO_PS_RBV_Z",     asynParamInt32, &NFO_PS_rbv_[2]);
	
	// SAM_PS (x, y, z)
	createParam("SAM_PS_X",         asynParamInt32, &SAM_PS_[0]);
	createParam("SAM_PS_Y",         asynParamInt32, &SAM_PS_[1]);
	createParam("SAM_PS_Z",         asynParamInt32, &SAM_PS_[2]);
	createParam("SAM_PS_RBV_X",     asynParamInt32, &SAM_PS_rbv_[0]);
	createParam("SAM_PS_RBV_Y",     asynParamInt32, &SAM_PS_rbv_[1]);
	createParam("SAM_PS_RBV_Z",     asynParamInt32, &SAM_PS_rbv_[2]);
	
	// NFO_SG_rbv (x, y, z)
	createParam("NFO_SG_RBV_X",     asynParamInt32, &NFO_SG_rbv_[0]);
	createParam("NFO_SG_RBV_Y",     asynParamInt32, &NFO_SG_rbv_[1]);
	createParam("NFO_SG_RBV_Z",     asynParamInt32, &NFO_SG_rbv_[2]);
	
	// SAM_CP_D_rbv (x, y, z)
	createParam("SAM_CP_D_RBV_X",   asynParamInt32, &SAM_CP_D_rbv_[0]);
	createParam("SAM_CP_D_RBV_Y",   asynParamInt32, &SAM_CP_D_rbv_[1]);
	createParam("SAM_CP_D_RBV_Z",   asynParamInt32, &SAM_CP_D_rbv_[2]);
	
	// XZ_ZX_rbv (0-1)
	createParam("XZ_ZX_RBV_0",      asynParamInt32, &XZ_ZX_rbv_[0]);
	createParam("XZ_ZX_RBV_1",      asynParamInt32, &XZ_ZX_rbv_[1]);
	
	// AUX_ADC_rbv (0-2)
	createParam("AUX_ADC_RBV_0",    asynParamInt32, &AUX_ADC_rbv_[0]);
	createParam("AUX_ADC_RBV_1",    asynParamInt32, &AUX_ADC_rbv_[1]);
	createParam("AUX_ADC_RBV_2",    asynParamInt32, &AUX_ADC_rbv_[2]);
	
	// NFO_rbv (x, y, z)
	createParam("NFO_RBV_X",        asynParamInt32, &NFO_rbv_[0]);
	createParam("NFO_RBV_Y",        asynParamInt32, &NFO_rbv_[1]);
	createParam("NFO_RBV_Z",        asynParamInt32, &NFO_rbv_[2]);
	
	// SAM_rbv (x, y, z)
	createParam("SAM_RBV_X",        asynParamInt32, &SAM_rbv_[0]);
	createParam("SAM_RBV_Y",        asynParamInt32, &SAM_rbv_[1]);
	createParam("SAM_RBV_Z",        asynParamInt32, &SAM_rbv_[2]);
	
	// SetptFreq (x, y, z)
	createParam("SETPT_FREQ_X",         asynParamInt32, &SetptFreq_[0]);
	createParam("SETPT_FREQ_Y",         asynParamInt32, &SetptFreq_[1]);
	createParam("SETPT_FREQ_Z",         asynParamInt32, &SetptFreq_[2]);
	createParam("SETPT_FREQ_RBV_X",     asynParamInt32, &SetptFreq_rbv_[0]);
	createParam("SETPT_FREQ_RBV_Y",     asynParamInt32, &SetptFreq_rbv_[1]);
	createParam("SETPT_FREQ_RBV_Z",     asynParamInt32, &SetptFreq_rbv_[2]);
	
	// SetptPhase (x, y, z)
	createParam("SETPT_PHASE_X",        asynParamInt32, &SetptPhase_[0]);
	createParam("SETPT_PHASE_Y",        asynParamInt32, &SetptPhase_[1]);
	createParam("SETPT_PHASE_Z",        asynParamInt32, &SetptPhase_[2]);
	createParam("SETPT_PHASE_RBV_X",    asynParamInt32, &SetptPhase_rbv_[0]);
	createParam("SETPT_PHASE_RBV_Y",    asynParamInt32, &SetptPhase_rbv_[1]);
	createParam("SETPT_PHASE_RBV_Z",    asynParamInt32, &SetptPhase_rbv_[2]);
	
	// SetptAmp (x, y, z)
	createParam("SETPT_AMP_X",          asynParamInt32, &SetptAmp_[0]);
	createParam("SETPT_AMP_Y",          asynParamInt32, &SetptAmp_[1]);
	createParam("SETPT_AMP_Z",          asynParamInt32, &SetptAmp_[2]);
	createParam("SETPT_AMP_RBV_X",      asynParamInt32, &SetptAmp_rbv_[0]);
	createParam("SETPT_AMP_RBV_Y",      asynParamInt32, &SetptAmp_rbv_[1]);
	createParam("SETPT_AMP_RBV_Z",      asynParamInt32, &SetptAmp_rbv_[2]);
	
	// ExtADCShift (single value)
	createParam("EXT_ADC_SHIFT",        asynParamInt32, &ExtADCShift_);
	createParam("EXT_ADC_SHIFT_RBV",    asynParamInt32, &ExtADCShift_rbv_);
	
	// PIEnNFO (x, y, z)
	createParam("PI_EN_NFO_X",          asynParamInt32, &PIEnNFO_[0]);
	createParam("PI_EN_NFO_Y",          asynParamInt32, &PIEnNFO_[1]);
	createParam("PI_EN_NFO_Z",          asynParamInt32, &PIEnNFO_[2]);
	createParam("PI_EN_NFO_RBV_X",      asynParamInt32, &PIEnNFO_rbv_[0]);
	createParam("PI_EN_NFO_RBV_Y",      asynParamInt32, &PIEnNFO_rbv_[1]);
	createParam("PI_EN_NFO_RBV_Z",      asynParamInt32, &PIEnNFO_rbv_[2]);
	
	// PIIValNFO (x, y, z)
	// TODO: make float write function
	createParam("PI_I_VAL_NFO_X",       asynParamFloat64, &PIIValNFO_[0]);
	createParam("PI_I_VAL_NFO_Y",       asynParamFloat64, &PIIValNFO_[1]);
	createParam("PI_I_VAL_NFO_Z",       asynParamFloat64, &PIIValNFO_[2]);
	createParam("PI_I_VAL_NFO_RBV_X",   asynParamFloat64, &PIIValNFO_rbv_[0]);
	createParam("PI_I_VAL_NFO_RBV_Y",   asynParamFloat64, &PIIValNFO_rbv_[1]);
	createParam("PI_I_VAL_NFO_RBV_Z",   asynParamFloat64, &PIIValNFO_rbv_[2]);
	
	// PIPValNFO (x, y, z)
	createParam("PI_P_VAL_NFO_X",       asynParamInt32, &PIPValNFO_[0]);
	createParam("PI_P_VAL_NFO_Y",       asynParamInt32, &PIPValNFO_[1]);
	createParam("PI_P_VAL_NFO_Z",       asynParamInt32, &PIPValNFO_[2]);
	createParam("PI_P_VAL_NFO_RBV_X",   asynParamInt32, &PIPValNFO_rbv_[0]);
	createParam("PI_P_VAL_NFO_RBV_Y",   asynParamInt32, &PIPValNFO_rbv_[1]);
	createParam("PI_P_VAL_NFO_RBV_Z",   asynParamInt32, &PIPValNFO_rbv_[2]);
	
	// PILimNFO (single value)
	createParam("PI_LIM_NFO",           asynParamInt32, &PILimNFO_);
	createParam("PI_LIM_NFO_RBV",       asynParamInt32, &PILimNFO_rbv_);
	
	// PIAvgNFO (single value)
	createParam("PI_AVG_NFO",           asynParamInt32, &PIAvgNFO_);
	createParam("PI_AVG_NFO_RBV",       asynParamInt32, &PIAvgNFO_rbv_);
	
	// PIEnSAM (x, y, z)
	createParam("PI_EN_SAM_X",          asynParamInt32, &PIEnSAM_[0]);
	createParam("PI_EN_SAM_Y",          asynParamInt32, &PIEnSAM_[1]);
	createParam("PI_EN_SAM_Z",          asynParamInt32, &PIEnSAM_[2]);
	createParam("PI_EN_SAM_RBV_X",      asynParamInt32, &PIEnSAM_rbv_[0]);
	createParam("PI_EN_SAM_RBV_Y",      asynParamInt32, &PIEnSAM_rbv_[1]);
	createParam("PI_EN_SAM_RBV_Z",      asynParamInt32, &PIEnSAM_rbv_[2]);
	
	// PIIValSAM (x, y, z)
	createParam("PI_I_VAL_SAM_X",       asynParamFloat64, &PIIValSAM_[0]);
	createParam("PI_I_VAL_SAM_Y",       asynParamFloat64, &PIIValSAM_[1]);
	createParam("PI_I_VAL_SAM_Z",       asynParamFloat64, &PIIValSAM_[2]);
	createParam("PI_I_VAL_SAM_RBV_X",   asynParamFloat64, &PIIValSAM_rbv_[0]);
	createParam("PI_I_VAL_SAM_RBV_Y",   asynParamFloat64, &PIIValSAM_rbv_[1]);
	createParam("PI_I_VAL_SAM_RBV_Z",   asynParamFloat64, &PIIValSAM_rbv_[2]);
	
	// PIPValSAM (x, y, z)
	createParam("PI_P_VAL_SAM_X",       asynParamInt32, &PIPValSAM_[0]);
	createParam("PI_P_VAL_SAM_Y",       asynParamInt32, &PIPValSAM_[1]);
	createParam("PI_P_VAL_SAM_Z",       asynParamInt32, &PIPValSAM_[2]);
	createParam("PI_P_VAL_SAM_RBV_X",   asynParamInt32, &PIPValSAM_rbv_[0]);
	createParam("PI_P_VAL_SAM_RBV_Y",   asynParamInt32, &PIPValSAM_rbv_[1]);
	createParam("PI_P_VAL_SAM_RBV_Z",   asynParamInt32, &PIPValSAM_rbv_[2]);
	
	// PILimSAM (single value)
	createParam("PI_LIM_SAM",           asynParamInt32, &PILimSAM_);
	createParam("PI_LIM_SAM_RBV",       asynParamInt32, &PILimSAM_rbv_);
	
	// PITargPos (x, y, z)
	createParam("PI_TARG_POS_X",        asynParamInt32, &PITargPos_[0]);
	createParam("PI_TARG_POS_Y",        asynParamInt32, &PITargPos_[1]);
	createParam("PI_TARG_POS_Z",        asynParamInt32, &PITargPos_[2]);
	createParam("PI_TARG_POS_RBV_X",    asynParamInt32, &PITargPos_rbv_[0]);
	createParam("PI_TARG_POS_RBV_Y",    asynParamInt32, &PITargPos_rbv_[1]);
	createParam("PI_TARG_POS_RBV_Z",    asynParamInt32, &PITargPos_rbv_[2]);
	
	// PITargMode (single value)
	createParam("PI_TARG_MODE",         asynParamInt32, &PITargMode_);
	createParam("PI_TARG_MODE_RBV",     asynParamInt32, &PITargMode_rbv_);
	
	// PINFOOut_rbv (x, y, z)
	createParam("PI_NFO_OUT_RBV_X",     asynParamInt32, &PINFOOut_rbv_[0]);
	createParam("PI_NFO_OUT_RBV_Y",     asynParamInt32, &PINFOOut_rbv_[1]);
	createParam("PI_NFO_OUT_RBV_Z",     asynParamInt32, &PINFOOut_rbv_[2]);
	
	// PISAMOut_rbv (x, y, z)
	createParam("PI_SAM_OUT_RBV_X",     asynParamInt32, &PISAMOut_rbv_[0]);
	createParam("PI_SAM_OUT_RBV_Y",     asynParamInt32, &PISAMOut_rbv_[1]);
	createParam("PI_SAM_OUT_RBV_Z",     asynParamInt32, &PISAMOut_rbv_[2]);
	
	// NFOADCLimMin/Max (single value)
	createParam("NFO_ADC_LIM_MIN",      asynParamInt32, &NFOADCLimMin_);
	createParam("NFO_ADC_LIM_MAX",      asynParamInt32, &NFOADCLimMax_);
	createParam("NFO_ADC_LIM_MIN_RBV",  asynParamInt32, &NFOADCLimMin_rbv_);
	createParam("NFO_ADC_LIM_MAX_RBV",  asynParamInt32, &NFOADCLimMax_rbv_);
	
	// NFOSlewLim (single value)
	createParam("NFO_SLEW_LIM",         asynParamInt32, &NFOSlewLim_);
	createParam("NFO_SLEW_LIM_RBV",     asynParamInt32, &NFOSlewLim_rbv_);
	
	// SAMADCLimMin/Max (single value)
	createParam("SAM_ADC_LIM_MIN",      asynParamInt32, &SAMADCLimMin_);
	createParam("SAM_ADC_LIM_MAX",      asynParamInt32, &SAMADCLimMax_);
	createParam("SAM_ADC_LIM_MIN_RBV",  asynParamInt32, &SAMADCLimMin_rbv_);
	createParam("SAM_ADC_LIM_MAX_RBV",  asynParamInt32, &SAMADCLimMax_rbv_);
	
	// SAMSlewLim (single value)
	createParam("SAM_SLEW_LIM",         asynParamInt32, &SAMSlewLim_);
	createParam("SAM_SLEW_LIM_RBV",     asynParamInt32, &SAMSlewLim_rbv_);
	
	// LimState_rbv (single value)
	createParam("LIM_STATE_RBV",        asynParamInt32, &LimState_rbv_);
	
	// InpTransMat (matrix: one createParam, float64 array, 3x15 = 45 elements)
	createParam("INP_TRANS_MAT",        asynParamFloat64Array, InpTransMat_);   // 45 elements
	
	// InpTransRes_rbv (x, y, z)
	createParam("INP_TRANS_RES_RBV_X",  asynParamInt32, &InpTransRes_rbv_[0]);
	createParam("INP_TRANS_RES_RBV_Y",  asynParamInt32, &InpTransRes_rbv_[1]);
	createParam("INP_TRANS_RES_RBV_Z",  asynParamInt32, &InpTransRes_rbv_[2]);
	
	// InpTransAvg_rbv (single value)
	createParam("INP_TRANS_AVG_RBV",    asynParamInt32, &InpTransAvg_rbv_);
	

	// InpTransState_rbv (single value)
	createParam("INP_TRANS_STATE_RBV",  asynParamInt32, &InpTransState_rbv_);
	
	// OutTransMat (matrix: one createParam, float64 array, 6x7 = 42 elements)
	createParam("OUT_TRANS_MAT",        asynParamFloat64Array, OutTransMat_);   // 42 elements
	
	// OutTransNFORes_rbv (x, y, z)
	createParam("OUT_TRANS_NFO_RES_RBV_X", asynParamInt32, &OutTransNFORes_rbv_[0]);
	createParam("OUT_TRANS_NFO_RES_RBV_Y", asynParamInt32, &OutTransNFORes_rbv_[1]);
	createParam("OUT_TRANS_NFO_RES_RBV_Z", asynParamInt32, &OutTransNFORes_rbv_[2]);
	
	// OutTransSAMRes_rbv (x, y, z)
	createParam("OUT_TRANS_SAM_RES_RBV_X", asynParamInt32, &OutTransSAMRes_rbv_[0]);
	createParam("OUT_TRANS_SAM_RES_RBV_Y", asynParamInt32, &OutTransSAMRes_rbv_[1]);
	createParam("OUT_TRANS_SAM_RES_RBV_Z", asynParamInt32, &OutTransSAMRes_rbv_[2]);
	
	// Trajectory
	createParam("TRAJ_START_X",         asynParamInt32, &TrajStartX_);
	createParam("TRAJ_START_X_RBV",     asynParamInt32, &TrajStartX_rbv_);
	createParam("TRAJ_END_X",           asynParamInt32, &TrajEndX_);
	createParam("TRAJ_END_X_RBV",       asynParamInt32, &TrajEndX_rbv_);
	createParam("TRAJ_SPEED_X",         asynParamInt32, &TrajSpeedX_);
	createParam("TRAJ_SPEED_X_RBV",     asynParamInt32, &TrajSpeedX_rbv_);
	createParam("TRAJ_START_Y",         asynParamInt32, &TrajStartY_);
	createParam("TRAJ_START_Y_RBV",     asynParamInt32, &TrajStartY_rbv_);
	createParam("TRAJ_DIST_Y",          asynParamInt32, &TrajDistY_);
	createParam("TRAJ_DIST_Y_RBV",      asynParamInt32, &TrajDistY_rbv_);
	createParam("TRAJ_COUNT_Y",         asynParamInt32, &TrajCountY_);
	createParam("TRAJ_COUNT_Y_RBV",     asynParamInt32, &TrajCountY_rbv_);
	createParam("TRAJ_TURN_TIME",       asynParamInt32, &TrajTurnTime_);
	createParam("TRAJ_TURN_TIME_RBV",   asynParamInt32, &TrajTurnTime_rbv_);
	createParam("TRAJ_POS_TIME",        asynParamInt32, &TrajPosTime_);
	createParam("TRAJ_POS_TIME_RBV",    asynParamInt32, &TrajPosTime_rbv_);
	createParam("TRAJ_ANTI_HYST",       asynParamInt32, &TrajAntiHyst_);
	createParam("TRAJ_ANTI_HYST_RBV",   asynParamInt32, &TrajAntiHyst_rbv_);
	createParam("TRAJ_SETTINGS",        asynParamInt32, &TrajSettings_);
	createParam("TRAJ_SETTINGS_RBV",    asynParamInt32, &TrajSettings_rbv_);

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

    int value, errorCode;
    unsigned int uvalue;
    double dvalue;
    bln32 bvalue;
    DSCS_TargetMode targetMode;
    DSCS_LimiterState limiterState;
    DSCS_InputTransformationState inTransState;

    // Axis enum mapping
    const DSCS_Axis axes[3] = {DSCS_AxisX, DSCS_AxisY, DSCS_AxisZ};

    // AUX channel enum mapping
    const DSCS_AUX_ADC auxChans[4] = {DSCS_AUX_0, DSCS_AUX_1, DSCS_AUX_2, DSCS_AUX_3};

    // XZ/ZX enum mapping
    const DSCS_XZ_ZX xz_zx[2] = {DSCS_XZ, DSCS_ZX};

    // --- OSA_PS_rbv_[2] ---
    for (int i = 0; i < 2; ++i) {
        errorCode = DSCS_getOSA_PS(deviceNo, axes[i], &value);
        checkError("DSCS_getOSA_PS", errorCode);
        setIntegerParam(OSA_PS_rbv_[i], value);
    }

    // --- BS_PS_rbv_[2] ---
    for (int i = 0; i < 2; ++i) {
        errorCode = DSCS_getBS_PS(deviceNo, axes[i], &value);
        checkError("DSCS_getBS_PS", errorCode);
        setIntegerParam(BS_PS_rbv_[i], value);
    }

    // --- AUX_DAC_rbv_[4] ---
    for (int i = 0; i < 4; ++i) {
        errorCode = DSCS_getAUX_DAC(deviceNo, auxChans[i], &value);
        checkError("DSCS_getAUX_DAC", errorCode);
        setIntegerParam(AUX_DAC_rbv_[i], value);
    }

    // --- NFO_PS_rbv_[3] ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getNFO_PS(deviceNo, axes[i], &value);
        checkError("DSCS_getNFO_PS", errorCode);
        setIntegerParam(NFO_PS_rbv_[i], value);
    }

    // --- SAM_PS_rbv_[3] ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getSAM_PS(deviceNo, axes[i], &value);
        checkError("DSCS_getSAM_PS", errorCode);
        setIntegerParam(SAM_PS_rbv_[i], value);
    }

    // --- NFO_SG_rbv_[3] ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getNFO_SG(deviceNo, axes[i], &value);
        checkError("DSCS_getNFO_SG", errorCode);
        setIntegerParam(NFO_SG_rbv_[i], value);
    }

    // --- SAM_CP_D_rbv_[3] ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getSAM_CP_D(deviceNo, axes[i], &value);
        checkError("DSCS_getSAM_CP_D", errorCode);
        setIntegerParam(SAM_CP_D_rbv_[i], value);
    }

    // --- XZ_ZX_rbv_[2] ---
    for (int i = 0; i < 2; ++i) {
        errorCode = DSCS_getXZ_ZX(deviceNo, xz_zx[i], &value);
        checkError("DSCS_getXZ_ZX", errorCode);
        setIntegerParam(XZ_ZX_rbv_[i], value);
    }

    // --- AUX_ADC_rbv_[3] ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getAUX_ADC(deviceNo, auxChans[i], &value);
        checkError("DSCS_getAUX_ADC", errorCode);
        setIntegerParam(AUX_ADC_rbv_[i], value);
    }

    // --- NFO_rbv_[3] ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getNFO(deviceNo, axes[i], &value);
        checkError("DSCS_getNFO", errorCode);
        setIntegerParam(NFO_rbv_[i], value);
    }

    // --- SAM_rbv_[3] ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getSAM(deviceNo, axes[i], &value);
        checkError("DSCS_getSAM", errorCode);
        setIntegerParam(SAM_rbv_[i], value);
    }

    // --- SetptFreq_rbv_[3] ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getSetpointModulationFrequency(deviceNo, axes[i], &value);
        checkError("DSCS_getSetpointModulationFrequency", errorCode);
        setIntegerParam(SetptFreq_rbv_[i], value);
    }

    // --- SetptPhase_rbv_[3] ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getSetpointModulationPhase(deviceNo, axes[i], &value);
        checkError("DSCS_getSetpointModulationPhase", errorCode);
        setIntegerParam(SetptPhase_rbv_[i], value);
    }

    // --- SetptAmp_rbv_[3] ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getSetpointModulationAmplitude(deviceNo, axes[i], &value);
        checkError("DSCS_getSetpointModulationAmplitude", errorCode);
        setIntegerParam(SetptAmp_rbv_[i], value);
    }

    // --- ExtADCShift_rbv_ ---
    errorCode = DSCS_getExternalADCShift(deviceNo, &value);
    checkError("DSCS_getExternalADCShift", errorCode);
    setIntegerParam(ExtADCShift_rbv_, value);

    // --- PIEnNFO_rbv_[3] (bln32) ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getPIControllerEnabledNFO(deviceNo, axes[i], &bvalue);
        checkError("DSCS_getPIControllerEnabledNFO", errorCode);
        setIntegerParam(PIEnNFO_rbv_[i], bvalue);
    }

    // --- PIIValNFO_rbv_[3] ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getPIControllerIValueNFO(deviceNo, axes[i], &dvalue);
        checkError("DSCS_getPIControllerIValueNFO", errorCode);
        setDoubleParam(PIIValNFO_rbv_[i], dvalue);
    }

    // --- PIPValNFO_rbv_[3] ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getPIControllerPValueNFO(deviceNo, axes[i], &value);
        checkError("DSCS_getPIControllerPValueNFO", errorCode);
        setIntegerParam(PIPValNFO_rbv_[i], value);
    }

    // --- PILimNFO_rbv_ ---
    errorCode = DSCS_getPIControllerLimitNFO(deviceNo, &value);
    checkError("DSCS_getPIControllerLimitNFO", errorCode);
    setIntegerParam(PILimNFO_rbv_, value);

    // --- PIAvgNFO_rbv_ ---
    errorCode = DSCS_getPIControllerAverageNFO(deviceNo, (unsigned short*)&value);
    value = value & 0xFFFF; // mask to range of unsigned short
    checkError("DSCS_getPIControllerAverageNFO", errorCode);
    setIntegerParam(PIAvgNFO_rbv_, value);

    // --- PIEnSAM_rbv_[3] (bln32) ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getPIControllerEnabledSAM(deviceNo, axes[i], &bvalue);
        checkError("DSCS_getPIControllerEnabledSAM", errorCode);
        setIntegerParam(PIEnSAM_rbv_[i], bvalue);
    }

    // --- PIIValSAM_rbv_[3] ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getPIControllerIValueSAM(deviceNo, axes[i], &dvalue);
        checkError("DSCS_getPIControllerIValueSAM", errorCode);
        setDoubleParam(PIIValSAM_rbv_[i], dvalue);
    }

    // --- PIPValSAM_rbv_[3] ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getPIControllerPValueSAM(deviceNo, axes[i], &value);
        checkError("DSCS_getPIControllerPValueSAM", errorCode);
        setIntegerParam(PIPValSAM_rbv_[i], value);
    }

    // --- PILimSAM_rbv_ ---
    errorCode = DSCS_getPIControllerLimitSAM(deviceNo, &value);
    checkError("DSCS_getPIControllerLimitSAM", errorCode);
    setIntegerParam(PILimSAM_rbv_, value);

    // --- PITargPos_rbv_[3] ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getPIControllerTargetPosition(deviceNo, axes[i], &value);
        checkError("DSCS_getPIControllerTargetPosition", errorCode);
        setIntegerParam(PITargPos_rbv_[i], value);
    }

    // --- PITargMode_rbv_ (enum) ---
    errorCode = DSCS_getPIControllerTargetMode(deviceNo, &targetMode); // targetMode is typedef
    checkError("DSCS_getPIControllerTargetMode", errorCode);
    setIntegerParam(PITargMode_rbv_, targetMode);

    // --- PINFOOut_rbv_[3] ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getPIControllerNFOOutput(deviceNo, axes[i], &value);
        checkError("DSCS_getPIControllerNFOOutput", errorCode);
        setIntegerParam(PINFOOut_rbv_[i], value);
    }

    // --- PISAMOut_rbv_[3] ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getPIControllerSAMOutput(deviceNo, axes[i], &value);
        checkError("DSCS_getPIControllerSAMOutput", errorCode);
        setIntegerParam(PISAMOut_rbv_[i], value);
    }

    // --- NFOADCLimMin_rbv_ & NFOADCLimMax_rbv_ ---
    {
        int minValue, maxValue;
        errorCode = DSCS_getNFOADCLimits(deviceNo, &minValue, &maxValue);
        checkError("DSCS_getNFOADCLimits", errorCode);
        setIntegerParam(NFOADCLimMin_rbv_, minValue);
        setIntegerParam(NFOADCLimMax_rbv_, maxValue);
    }

    // --- NFOSlewLim_rbv_ ---
    errorCode = DSCS_getNFOSlewRateLimit(deviceNo, &value);
    checkError("DSCS_getNFOSlewRateLimit", errorCode);
    setIntegerParam(NFOSlewLim_rbv_, value);

    // --- SAMADCLimMin_rbv_ & SAMADCLimMax_rbv_ ---
    {
        int minValue, maxValue;
        errorCode = DSCS_getSAMADCLimits(deviceNo, &minValue, &maxValue);
        checkError("DSCS_getSAMADCLimits", errorCode);
        setIntegerParam(SAMADCLimMin_rbv_, minValue);
        setIntegerParam(SAMADCLimMax_rbv_, maxValue);
    }

    // --- SAMSlewLim_rbv_ ---
    errorCode = DSCS_getSAMSlewRateLimit(deviceNo, &value);
    checkError("DSCS_getSAMSlewRateLimit", errorCode);
    setIntegerParam(SAMSlewLim_rbv_, value);

    // --- LimState_rbv_ (enum) ---
    errorCode = DSCS_getLimiterState(deviceNo, &limiterState);
    checkError("DSCS_getLimiterState", errorCode);
    setIntegerParam(LimState_rbv_, limiterState);

    // --- InpTransRes_rbv_[3] ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getInputTransformationResult(deviceNo, axes[i], &value);
        checkError("DSCS_getInputTransformationResult", errorCode);
        setIntegerParam(InpTransRes_rbv_[i], value);
    }

    // --- InpTransAvg_rbv_ ---
    errorCode = DSCS_getInputTransformationAverage(deviceNo, &value);
    checkError("DSCS_getInputTransformationAverage", errorCode);
    setIntegerParam(InpTransAvg_rbv_, value);

    // --- InpTransState_rbv_ (enum) ---
    errorCode = DSCS_getInputTransformationState(deviceNo, &inTransState);
    checkError("DSCS_getInputTransformationState", errorCode);
    setIntegerParam(InpTransState_rbv_, inTransState);

    // --- OutTransNFORes_rbv_[3] ---
    for (int i = 0; i < 3; ++i) {
    	int dummy;
        errorCode = DSCS_getOutputTransformationResult(deviceNo, axes[i], &value, &dummy);
        checkError("DSCS_getOutputTransformationResult", errorCode);
        setIntegerParam(OutTransNFORes_rbv_[i], value);
    }

    // --- OutTransSAMRes_rbv_[3] ---
    for (int i = 0; i < 3; ++i) {
    	int dummy;
        errorCode = DSCS_getOutputTransformationResult(deviceNo, axes[i], &dummy, &value);
        checkError("DSCS_getOutputTransformationResult", errorCode);
        setIntegerParam(OutTransSAMRes_rbv_[i], value);
    }

    // --- Trajectory parameters (all single value) ---
    errorCode = DSCS_getTrajectoryLineStartX(deviceNo, &value);
    checkError("DSCS_getTrajectoryLineStartX", errorCode);
    setIntegerParam(TrajStartX_rbv_, value);

    errorCode = DSCS_getTrajectoryLineEndX(deviceNo, &value);
    checkError("DSCS_getTrajectoryLineEndX", errorCode);
    setIntegerParam(TrajEndX_rbv_, value);

    errorCode = DSCS_getTrajectoryLineSpeedX(deviceNo, &value);
    checkError("DSCS_getTrajectoryLineSpeedX", errorCode);
    setIntegerParam(TrajSpeedX_rbv_, value);

    errorCode = DSCS_getTrajectoryLineStartY(deviceNo, &value);
    checkError("DSCS_getTrajectoryLineStartY", errorCode);
    setIntegerParam(TrajStartY_rbv_, value);

    errorCode = DSCS_getTrajectoryLineDistY(deviceNo, &value);
    checkError("DSCS_getTrajectoryLineDistY", errorCode);
    setIntegerParam(TrajDistY_rbv_, value);

    errorCode = DSCS_getTrajectoryLineCountY(deviceNo, (unsigned short*)&value);
    value = value & 0xFFFF; // mask to range of unsigned short
    checkError("DSCS_getTrajectoryLineCountY", errorCode);
    setIntegerParam(TrajCountY_rbv_, value);

    // unsigned int
    errorCode = DSCS_getTrajectoryTurnTime(deviceNo, &uvalue);
    checkError("DSCS_getTrajectoryTurnTime", errorCode);
    if (uvalue <= INT_MAX) {
    	setIntegerParam(TrajTurnTime_rbv_, value);
    }
    else {
    	printf("error converting unsigned int to int for TrajTurnTime_rbv_, uvalue=%u", uvalue);
    }

    // unsigned int
    errorCode = DSCS_getTrajectoryPosTime(deviceNo, &uvalue);
    checkError("DSCS_getTrajectoryPosTime", errorCode);
    if (uvalue <= INT_MAX) {
    	setIntegerParam(TrajPosTime_rbv_, value);
    }
    else {
    	printf("error converting unsigned int to int for TrajPosTime_rbv_, uvalue=%u", uvalue);
    }

    errorCode = DSCS_getTrajectoryAntiHyst(deviceNo, &value);
    checkError("DSCS_getTrajectoryAntiHyst", errorCode);
    setIntegerParam(TrajAntiHyst_rbv_, value);

    // unsigned int
    errorCode = DSCS_getTrajectorySettings(deviceNo, &uvalue);
    checkError("DSCS_getTrajectorySettings", errorCode);
    if (uvalue <= INT_MAX) {
    	setIntegerParam(TrajSettings_rbv_, value);
    }
    else {
    	printf("error converting unsigned int to int for TrajSettings_rbv_, uvalue=%u", uvalue);
    }


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
asynStatus dscsAsyn::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
	int function = pasynUser->reason;
	asynStatus status = asynSuccess;
	static const char *functionName = "writeInt32";

    asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, 
			"%s:%s, port %s, function = %d\n",
			driverName, functionName, this->portName, function);

	setIntegerParam(function, value);

	if (function == OSA_PS_[0]) status = setOSA_PS(DSCS_AxisX, value);
    	else if (function == OSA_PS_[1]) status = setOSA_PS(DSCS_AxisY, value);

    	else if (function == BS_PS_[0]) status = setBS_PS(DSCS_AxisX, value);
    	else if (function == BS_PS_[1]) status = setBS_PS(DSCS_AxisY, value);

    	else if (function == AUX_DAC_[0]) status = setAUX_DAC(DSCS_AUX_0, value);
    	else if (function == AUX_DAC_[1]) status = setAUX_DAC(DSCS_AUX_1, value);
    	else if (function == AUX_DAC_[2]) status = setAUX_DAC(DSCS_AUX_2, value);
    	else if (function == AUX_DAC_[3]) status = setAUX_DAC(DSCS_AUX_3, value);

    	else if (function == NFO_PS_[0]) status = setNFO_PS(DSCS_AxisX, value);
    	else if (function == NFO_PS_[1]) status = setNFO_PS(DSCS_AxisY, value);
    	else if (function == NFO_PS_[2]) status = setNFO_PS(DSCS_AxisZ, value);

    	else if (function == SAM_PS_[0]) status = setSAM_PS(DSCS_AxisX, value);
    	else if (function == SAM_PS_[1]) status = setSAM_PS(DSCS_AxisY, value);
    	else if (function == SAM_PS_[2]) status = setSAM_PS(DSCS_AxisZ, value);

    	else if (function == SetptFreq_[0]) status = setSetpointModulationFrequency(DSCS_AxisX, value);
    	else if (function == SetptFreq_[1]) status = setSetpointModulationFrequency(DSCS_AxisY, value);
    	else if (function == SetptFreq_[2]) status = setSetpointModulationFrequency(DSCS_AxisZ, value);

    	else if (function == SetptPhase_[0]) status = setSetpointModulationPhase(DSCS_AxisX, value);
    	else if (function == SetptPhase_[1]) status = setSetpointModulationPhase(DSCS_AxisY, value);
    	else if (function == SetptPhase_[2]) status = setSetpointModulationPhase(DSCS_AxisZ, value);

    	else if (function == SetptAmp_[0]) status = setSetpointModulationAmplitude(DSCS_AxisX, value);
    	else if (function == SetptAmp_[1]) status = setSetpointModulationAmplitude(DSCS_AxisY, value);
    	else if (function == SetptAmp_[2]) status = setSetpointModulationAmplitude(DSCS_AxisZ, value);

    	else if (function == ExtADCShift_) status = setExternalADCShift(value);

    	else if (function == PIEnNFO_[0]) status = setPIControllerEnabledNFO(DSCS_AxisX, value);
    	else if (function == PIEnNFO_[1]) status = setPIControllerEnabledNFO(DSCS_AxisY, value);
    	else if (function == PIEnNFO_[2]) status = setPIControllerEnabledNFO(DSCS_AxisZ, value);

    	// else if (function == PIIValNFO_[0]) status = setPIControllerIValueNFO(DSCS_AxisX, value);
    	// else if (function == PIIValNFO_[1]) status = setPIControllerIValueNFO(DSCS_AxisY, value);
    	// else if (function == PIIValNFO_[2]) status = setPIControllerIValueNFO(DSCS_AxisZ, value);

    	else if (function == PIPValNFO_[0]) status = setPIControllerPValueNFO(DSCS_AxisX, value);
    	else if (function == PIPValNFO_[1]) status = setPIControllerPValueNFO(DSCS_AxisY, value);
    	else if (function == PIPValNFO_[2]) status = setPIControllerPValueNFO(DSCS_AxisZ, value);

    	else if (function == PILimNFO_) status = setPIControllerLimitNFO(value);
    	else if (function == PIAvgNFO_) status = setPIControllerAverageNFO(value);

    	else if (function == PIEnSAM_[0]) status = setPIControllerEnabledSAM(DSCS_AxisX, value);
    	else if (function == PIEnSAM_[1]) status = setPIControllerEnabledSAM(DSCS_AxisY, value);
    	else if (function == PIEnSAM_[2]) status = setPIControllerEnabledSAM(DSCS_AxisZ, value);

	// move to float64
    	// else if (function == PIIValSAM_[0]) status = setPIControllerIValueSAM(DSCS_AxisX, value);
    	// else if (function == PIIValSAM_[1]) status = setPIControllerIValueSAM(DSCS_AxisY, value);
    	// else if (function == PIIValSAM_[2]) status = setPIControllerIValueSAM(DSCS_AxisZ, value);

    	else if (function == PIPValSAM_[0]) status = setPIControllerPValueSAM(DSCS_AxisX, value);
    	else if (function == PIPValSAM_[1]) status = setPIControllerPValueSAM(DSCS_AxisY, value);
    	else if (function == PIPValSAM_[2]) status = setPIControllerPValueSAM(DSCS_AxisZ, value);

    	else if (function == PILimSAM_) status = setPIControllerLimitSAM(value);

    	else if (function == PITargPos_[0]) status = setPIControllerTargetPosition(DSCS_AxisX, value);
    	else if (function == PITargPos_[1]) status = setPIControllerTargetPosition(DSCS_AxisY, value);
    	else if (function == PITargPos_[2]) status = setPIControllerTargetPosition(DSCS_AxisZ, value);

    	else if (function == PITargMode_) status = setPIControllerTargetMode(value);

    	else if (function == NFOADCLimMin_) status = setNFOADCLimMin(value);
    	else if (function == NFOADCLimMax_) status = setNFOADCLimMax(value);

    	else if (function == NFOSlewLim_) status = setNFOSlewRateLimit(value);

    	else if (function == SAMADCLimMin_) status = setSAMADCLimMin(value);
    	else if (function == SAMADCLimMax_) status = setSAMADCLimMax(value);

    	else if (function == SAMSlewLim_) status = setSAMSlewRateLimit(value);

    	else if (function == TrajStartX_) status = setTrajectoryLineStartX(value);
    	else if (function == TrajEndX_) status = setTrajectoryLineEndX(value);
    	else if (function == TrajSpeedX_) status = setTrajectoryLineSpeedX(value);
    	else if (function == TrajStartY_) status = setTrajectoryLineStartY(value);
    	else if (function == TrajDistY_) status = setTrajectoryLineDistY(value);
    	else if (function == TrajCountY_) status = setTrajectoryLineCountY(value);
    	else if (function == TrajTurnTime_) status = setTrajectoryTurnTime(value);
    	else if (function == TrajPosTime_) status = setTrajectoryPosTime(value);
    	else if (function == TrajAntiHyst_) status = setTrajectoryAntiHyst(value);
    	else if (function == TrajSettings_) status = setTrajectorySettings(value);

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

// OSA_PS
asynStatus dscsAsyn::setOSA_PS(DSCS_Axis axis, epicsInt32 value) {
    static const char *functionName = "setOSA_PS";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, axis = %d, value = %d\n", driverName, functionName, this->portName, axis, value);
    return (DSCS_setOSA_PS(deviceNo, axis, value) == 0) ? asynSuccess : asynError;
}

// BS_PS
asynStatus dscsAsyn::setBS_PS(DSCS_Axis axis, epicsInt32 value) {
    static const char *functionName = "setBS_PS";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, axis = %d, value = %d\n", driverName, functionName, this->portName, axis, value);
    return (DSCS_setBS_PS(deviceNo, axis, value) == 0) ? asynSuccess : asynError;
}

// AUX_DAC
asynStatus dscsAsyn::setAUX_DAC(DSCS_AUX_ADC aux, epicsInt32 value) {
    static const char *functionName = "setAUX_DAC";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, aux = %d, value = %d\n", driverName, functionName, this->portName, aux, value);
    return (DSCS_setAUX_DAC(deviceNo, aux, value) == 0) ? asynSuccess : asynError;
}

// NFO_PS
asynStatus dscsAsyn::setNFO_PS(DSCS_Axis axis, epicsInt32 value) {
    static const char *functionName = "setNFO_PS";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, axis = %d, value = %d\n", driverName, functionName, this->portName, axis, value);
    return (DSCS_setNFO_PS(deviceNo, axis, value) == 0) ? asynSuccess : asynError;
}

// SAM_PS
asynStatus dscsAsyn::setSAM_PS(DSCS_Axis axis, epicsInt32 value) {
    static const char *functionName = "setSAM_PS";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, axis = %d, value = %d\n", driverName, functionName, this->portName, axis, value);
    return (DSCS_setSAM_PS(deviceNo, axis, value) == 0) ? asynSuccess : asynError;
}

// SetpointModulationFrequency
asynStatus dscsAsyn::setSetpointModulationFrequency(DSCS_Axis axis, epicsInt32 value) {
    static const char *functionName = "setSetpointModulationFrequency";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, axis = %d, value = %d\n", driverName, functionName, this->portName, axis, value);
    return (DSCS_setSetpointModulationFrequency(deviceNo, axis, value) == 0) ? asynSuccess : asynError;
}

// SetpointModulationPhase
asynStatus dscsAsyn::setSetpointModulationPhase(DSCS_Axis axis, epicsInt32 value) {
    static const char *functionName = "setSetpointModulationPhase";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, axis = %d, value = %d\n", driverName, functionName, this->portName, axis, value);
    return (DSCS_setSetpointModulationPhase(deviceNo, axis, value) == 0) ? asynSuccess : asynError;
}

// SetpointModulationAmplitude
asynStatus dscsAsyn::setSetpointModulationAmplitude(DSCS_Axis axis, epicsInt32 value) {
    static const char *functionName = "setSetpointModulationAmplitude";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, axis = %d, value = %d\n", driverName, functionName, this->portName, axis, value);
    return (DSCS_setSetpointModulationAmplitude(deviceNo, axis, value) == 0) ? asynSuccess : asynError;
}

// ExternalADCShift
asynStatus dscsAsyn::setExternalADCShift(epicsInt32 value) {
    static const char *functionName = "setExternalADCShift";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, value = %d\n", driverName, functionName, this->portName, value);
    return (DSCS_setExternalADCShift(deviceNo, value) == 0) ? asynSuccess : asynError;
}

// PI Controller NFO
asynStatus dscsAsyn::setPIControllerEnabledNFO(DSCS_Axis axis, epicsInt32 value) {
    static const char *functionName = "setPIControllerEnabledNFO";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, axis = %d, value = %d\n", driverName, functionName, this->portName, axis, value);
    return (DSCS_setPIControllerEnabledNFO(deviceNo, axis, value) == 0) ? asynSuccess : asynError;
}
// move to float64
// asynStatus dscsAsyn::setPIControllerIValueNFO(DSCS_Axis axis, epicsInt32 value) {
//     static const char *functionName = "setPIControllerIValueNFO";
//     asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, axis = %d, value = %d\n", driverName, functionName, this->portName, axis, value);
//     return (DSCS_setPIControllerIValueNFO(deviceNo, axis, value) == 0) ? asynSuccess : asynError;
// }
asynStatus dscsAsyn::setPIControllerPValueNFO(DSCS_Axis axis, epicsInt32 value) {
    static const char *functionName = "setPIControllerPValueNFO";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, axis = %d, value = %d\n", driverName, functionName, this->portName, axis, value);
    return (DSCS_setPIControllerPValueNFO(deviceNo, axis, value) == 0) ? asynSuccess : asynError;
}
asynStatus dscsAsyn::setPIControllerLimitNFO(epicsInt32 value) {
    static const char *functionName = "setPIControllerLimitNFO";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, value = %d\n", driverName, functionName, this->portName, value);
    return (DSCS_setPIControllerLimitNFO(deviceNo, value) == 0) ? asynSuccess : asynError;
}
asynStatus dscsAsyn::setPIControllerAverageNFO(epicsInt32 value) {
    static const char *functionName = "setPIControllerAverageNFO";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, value = %d\n", driverName, functionName, this->portName, value);
    return (DSCS_setPIControllerAverageNFO(deviceNo, (unsigned short)value) == 0) ? asynSuccess : asynError;
}

// PI Controller SAM
asynStatus dscsAsyn::setPIControllerEnabledSAM(DSCS_Axis axis, epicsInt32 value) {
    static const char *functionName = "setPIControllerEnabledSAM";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, axis = %d, value = %d\n", driverName, functionName, this->portName, axis, value);
    return (DSCS_setPIControllerEnabledSAM(deviceNo, axis, value) == 0) ? asynSuccess : asynError;
}
// move to float64
// asynStatus dscsAsyn::setPIControllerIValueSAM(DSCS_Axis axis, epicsInt32 value) {
//     static const char *functionName = "setPIControllerIValueSAM";
//     asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, axis = %d, value = %d\n", driverName, functionName, this->portName, axis, value);
//     return (DSCS_setPIControllerIValueSAM(deviceNo, axis, value) == 0) ? asynSuccess : asynError;
// }
asynStatus dscsAsyn::setPIControllerPValueSAM(DSCS_Axis axis, epicsInt32 value) {
    static const char *functionName = "setPIControllerPValueSAM";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, axis = %d, value = %d\n", driverName, functionName, this->portName, axis, value);
    return (DSCS_setPIControllerPValueSAM(deviceNo, axis, value) == 0) ? asynSuccess : asynError;
}
asynStatus dscsAsyn::setPIControllerLimitSAM(epicsInt32 value) {
    static const char *functionName = "setPIControllerLimitSAM";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, value = %d\n", driverName, functionName, this->portName, value);
    return (DSCS_setPIControllerLimitSAM(deviceNo, value) == 0) ? asynSuccess : asynError;
}

// PI Controller Target
asynStatus dscsAsyn::setPIControllerTargetPosition(DSCS_Axis axis, epicsInt32 value) {
    static const char *functionName = "setPIControllerTargetPosition";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, axis = %d, value = %d\n", driverName, functionName, this->portName, axis, value);
    return (DSCS_setPIControllerTargetPosition(deviceNo, axis, value) == 0) ? asynSuccess : asynError;
}
asynStatus dscsAsyn::setPIControllerTargetMode(epicsInt32 value) {
    static const char *functionName = "setPIControllerTargetMode";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, value = %d\n", driverName, functionName, this->portName, value);
    return (DSCS_setPIControllerTargetMode(deviceNo, (DSCS_TargetMode)value) == 0) ? asynSuccess : asynError; // cast to DSCS_TargetMode
}

// NFOADCLimits (min/max)
asynStatus dscsAsyn::setNFOADCLimMin(epicsInt32 value) {
    static const char *functionName = "setNFOADCLimMin";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, min = %d\n", driverName, functionName, this->portName, value);
    int max = 0;
    int err = DSCS_getNFOADCLimits(deviceNo, nullptr, &max);
    if (err != 0) {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "%s:%s, port %s, ERROR reading max: %d\n", driverName, functionName, this->portName, err);
        return asynError;
    }
    return (DSCS_setNFOADCLimits(deviceNo, value, max) == 0) ? asynSuccess : asynError;
}
asynStatus dscsAsyn::setNFOADCLimMax(epicsInt32 value) {
    static const char *functionName = "setNFOADCLimMax";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, max = %d\n", driverName, functionName, this->portName, value);
    int min = 0;
    int err = DSCS_getNFOADCLimits(deviceNo, &min, nullptr);
    if (err != 0) {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "%s:%s, port %s, ERROR reading min: %d\n", driverName, functionName, this->portName, err);
        return asynError;
    }
    return (DSCS_setNFOADCLimits(deviceNo, min, value) == 0) ? asynSuccess : asynError;
}

// NFOSlewRateLimit
asynStatus dscsAsyn::setNFOSlewRateLimit(epicsInt32 value) {
    static const char *functionName = "setNFOSlewRateLimit";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, value = %d\n", driverName, functionName, this->portName, value);
    return (DSCS_setNFOSlewRateLimit(deviceNo, value) == 0) ? asynSuccess : asynError;
}

// SAMADCLimits (min/max)
asynStatus dscsAsyn::setSAMADCLimMin(epicsInt32 value) {
    static const char *functionName = "setSAMADCLimMin";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, min = %d\n", driverName, functionName, this->portName, value);
    int max = 0;
    int err = DSCS_getSAMADCLimits(deviceNo, nullptr, &max);
    if (err != 0) {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "%s:%s, port %s, ERROR reading max: %d\n", driverName, functionName, this->portName, err);
        return asynError;
    }
    return (DSCS_setSAMADCLimits(deviceNo, value, max) == 0) ? asynSuccess : asynError;
}
asynStatus dscsAsyn::setSAMADCLimMax(epicsInt32 value) {
    static const char *functionName = "setSAMADCLimMax";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, max = %d\n", driverName, functionName, this->portName, value);
    int min = 0;
    int err = DSCS_getSAMADCLimits(deviceNo, &min, nullptr);
    if (err != 0) {
        asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "%s:%s, port %s, ERROR reading min: %d\n", driverName, functionName, this->portName, err);
        return asynError;
    }
    return (DSCS_setSAMADCLimits(deviceNo, min, value) == 0) ? asynSuccess : asynError;
}

// SAMSlewRateLimit
asynStatus dscsAsyn::setSAMSlewRateLimit(epicsInt32 value) {
    static const char *functionName = "setSAMSlewRateLimit";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, value = %d\n", driverName, functionName, this->portName, value);
    return (DSCS_setSAMSlewRateLimit(deviceNo, value) == 0) ? asynSuccess : asynError;
}

// Trajectory line parameters
asynStatus dscsAsyn::setTrajectoryLineStartX(epicsInt32 value) {
    static const char *functionName = "setTrajectoryLineStartX";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, value = %d\n", driverName, functionName, this->portName, value);
    return (DSCS_setTrajectoryLineStartX(deviceNo, value) == 0) ? asynSuccess : asynError;
}
asynStatus dscsAsyn::setTrajectoryLineEndX(epicsInt32 value) {
    static const char *functionName = "setTrajectoryLineEndX";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, value = %d\n", driverName, functionName, this->portName, value);
    return (DSCS_setTrajectoryLineEndX(deviceNo, value) == 0) ? asynSuccess : asynError;
}
asynStatus dscsAsyn::setTrajectoryLineSpeedX(epicsInt32 value) {
    static const char *functionName = "setTrajectoryLineSpeedX";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, value = %d\n", driverName, functionName, this->portName, value);
    return (DSCS_setTrajectoryLineSpeedX(deviceNo, value) == 0) ? asynSuccess : asynError;
}
asynStatus dscsAsyn::setTrajectoryLineStartY(epicsInt32 value) {
    static const char *functionName = "setTrajectoryLineStartY";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, value = %d\n", driverName, functionName, this->portName, value);
    return (DSCS_setTrajectoryLineStartY(deviceNo, value) == 0) ? asynSuccess : asynError;
}
asynStatus dscsAsyn::setTrajectoryLineDistY(epicsInt32 value) {
    static const char *functionName = "setTrajectoryLineDistY";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, value = %d\n", driverName, functionName, this->portName, value);
    return (DSCS_setTrajectoryLineDistY(deviceNo, value) == 0) ? asynSuccess : asynError;
}
asynStatus dscsAsyn::setTrajectoryLineCountY(epicsInt32 value) {
    static const char *functionName = "setTrajectoryLineCountY";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, value = %d\n", driverName, functionName, this->portName, value);
    return (DSCS_setTrajectoryLineCountY(deviceNo, value) == 0) ? asynSuccess : asynError;
}
asynStatus dscsAsyn::setTrajectoryTurnTime(epicsInt32 value) {
    static const char *functionName = "setTrajectoryTurnTime";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, value = %d\n", driverName, functionName, this->portName, value);
    return (DSCS_setTrajectoryTurnTime(deviceNo, value) == 0) ? asynSuccess : asynError;
}
asynStatus dscsAsyn::setTrajectoryPosTime(epicsInt32 value) {
    static const char *functionName = "setTrajectoryPosTime";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, value = %d\n", driverName, functionName, this->portName, value);
    return (DSCS_setTrajectoryPosTime(deviceNo, value) == 0) ? asynSuccess : asynError;
}
asynStatus dscsAsyn::setTrajectoryAntiHyst(epicsInt32 value) {
    static const char *functionName = "setTrajectoryAntiHyst";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, value = %d\n", driverName, functionName, this->portName, value);
    return (DSCS_setTrajectoryAntiHyst(deviceNo, value) == 0) ? asynSuccess : asynError;
}
asynStatus dscsAsyn::setTrajectorySettings(epicsInt32 value) {
    static const char *functionName = "setTrajectorySettings";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, value = %d\n", driverName, functionName, this->portName, value);
    return (DSCS_setTrajectorySettings(deviceNo, value) == 0) ? asynSuccess : asynError;
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
