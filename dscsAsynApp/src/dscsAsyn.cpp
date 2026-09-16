// TODO: 
// data callback functions:
// 	DSCS_setDataCallback()
// 	DSCS_setDataOutputEnabled()
// 
// transfer some int to float variables
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

#include <cstdint>
#include <cmath>

static constexpr double POW2_32  = 4294967296.0;
static constexpr double UV_LSB_V = 1e-6;                 // 1 uV
static constexpr double LISSA_FREQ_LSB_KHZ  = 20.21 / POW2_32;
static constexpr double LISSA_PHASE_LSB_DEG = 360.0 / POW2_32;
static constexpr double NM_LSB = 632.991 / 4096.0;       // nm per count
static constexpr double OUT_TRANS_LSB_V = 20.0 / POW2_32; // V per count

static inline int32_t clamp_i32(long long v) {
    if (v > INT32_MAX) return INT32_MAX;
    if (v < INT32_MIN) return INT32_MIN;
    return (int32_t)v;
}

static inline uint32_t clamp_u32(long long v) {
    if (v < 0) return 0u;
    if (v > 0xFFFFFFFFll) return 0xFFFFFFFFu;
    return (uint32_t)v;
}

// -10V..+10V in 1uV steps (signed)
static inline int32_t volts_to_raw_uV(double volts) {
    return clamp_i32((long long)llround(volts / UV_LSB_V));
}
static inline double raw_uV_to_volts(int32_t raw) {
    return (double)raw * UV_LSB_V;
}

// 632.991/4096 nm (signed unless noted)
static inline int32_t nm_to_raw_steps(double nm) {
    return clamp_i32((long long)llround(nm / NM_LSB));
}
static inline uint32_t nm_to_raw_steps_u32(double nm) {
    return clamp_u32((long long)llround(nm / NM_LSB));
}
static inline double raw_steps_to_nm(int32_t raw) {
    return (double)raw * NM_LSB;
}
static inline double raw_steps_u32_to_nm(uint32_t raw) {
    return (double)raw * NM_LSB;
}

// Lissajous frequency/phase are unsigned 32-bit counts (passed through int)
static inline uint32_t liss_freq_khz_to_raw(double khz) {
    return clamp_u32((long long)llround(khz / LISSA_FREQ_LSB_KHZ));
}
static inline double raw_to_liss_freq_khz(uint32_t raw) {
    return (double)raw * LISSA_FREQ_LSB_KHZ;
}

static inline uint32_t liss_phase_deg_to_raw(double deg) {
    return clamp_u32((long long)llround(deg / LISSA_PHASE_LSB_DEG));
}
static inline double raw_to_liss_phase_deg(uint32_t raw) {
    return (double)raw * LISSA_PHASE_LSB_DEG;
}

// Output transformation results: 20V/2^32
static inline uint32_t out_trans_v_to_raw(double volts) {
    return clamp_u32((long long)llround(volts / OUT_TRANS_LSB_V));
}
static inline double raw_to_out_trans_v(uint32_t raw) {
    return (double)raw * OUT_TRANS_LSB_V;
}

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
    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR, "%s: %s\n", context, getMessage(code));
  }
}

void dscsAsyn::setAllParamStatus(asynStatus status)
{
  int count = 0;
  getNumParams(&count);
  for (int param = 0; param < count; ++param) setParamStatus(param, status);
}

void dscsAsyn::updateIntegerParam(const char *context, int code, int param, epicsInt32 value)
{
  checkError(context, code);
  setParamStatus(param, code == DSCS_Ok ? asynSuccess : asynError);
  if (code == DSCS_Ok) setIntegerParam(param, value);
  else if (code == DSCS_NotConnected) connected_ = false;
}

void dscsAsyn::updateDoubleParam(const char *context, int code, int param, epicsFloat64 value)
{
  checkError(context, code);
  setParamStatus(param, code == DSCS_Ok ? asynSuccess : asynError);
  if (code == DSCS_Ok) setDoubleParam(param, value);
  else if (code == DSCS_NotConnected) connected_ = false;
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

	// "//" after line means tested

	// OSA_PS (x and y)
	createParam("OSA_PS_X",         asynParamFloat64, &OSA_PS_[0]); 
	createParam("OSA_PS_Y",         asynParamFloat64, &OSA_PS_[1]); 
	createParam("OSA_PS_RBV_X",     asynParamFloat64, &OSA_PS_rbv_[0]); 
	createParam("OSA_PS_RBV_Y",     asynParamFloat64, &OSA_PS_rbv_[1]); 
	
	// BS_PS (x and y)
	createParam("BS_PS_X",          asynParamFloat64, &BS_PS_[0]); 
	createParam("BS_PS_Y",          asynParamFloat64, &BS_PS_[1]); 
	createParam("BS_PS_RBV_X",      asynParamFloat64, &BS_PS_rbv_[0]); 
	createParam("BS_PS_RBV_Y",      asynParamFloat64, &BS_PS_rbv_[1]); 
	
	// AUX_DAC (0-3)
	createParam("AUX_DAC_0",        asynParamFloat64, &AUX_DAC_[0]); 
	createParam("AUX_DAC_1",        asynParamFloat64, &AUX_DAC_[1]); 
	createParam("AUX_DAC_2",        asynParamFloat64, &AUX_DAC_[2]); 
	createParam("AUX_DAC_3",        asynParamFloat64, &AUX_DAC_[3]); 
	createParam("AUX_DAC_RBV_0",    asynParamFloat64, &AUX_DAC_rbv_[0]); 
	createParam("AUX_DAC_RBV_1",    asynParamFloat64, &AUX_DAC_rbv_[1]); 
	createParam("AUX_DAC_RBV_2",    asynParamFloat64, &AUX_DAC_rbv_[2]); 
	createParam("AUX_DAC_RBV_3",    asynParamFloat64, &AUX_DAC_rbv_[3]); 
	
	// NFO_PS (x, y, z)
	createParam("NFO_PS_X",         asynParamFloat64, &NFO_PS_[0]); 
	createParam("NFO_PS_Y",         asynParamFloat64, &NFO_PS_[1]); 
	createParam("NFO_PS_Z",         asynParamFloat64, &NFO_PS_[2]); 
	createParam("NFO_PS_RBV_X",     asynParamFloat64, &NFO_PS_rbv_[0]); 
	createParam("NFO_PS_RBV_Y",     asynParamFloat64, &NFO_PS_rbv_[1]); 
	createParam("NFO_PS_RBV_Z",     asynParamFloat64, &NFO_PS_rbv_[2]); 
	
	// SAM_PS (x, y, z)
	createParam("SAM_PS_X",         asynParamFloat64, &SAM_PS_[0]); 
	createParam("SAM_PS_Y",         asynParamFloat64, &SAM_PS_[1]); 
	createParam("SAM_PS_Z",         asynParamFloat64, &SAM_PS_[2]); 
	createParam("SAM_PS_RBV_X",     asynParamFloat64, &SAM_PS_rbv_[0]); 
	createParam("SAM_PS_RBV_Y",     asynParamFloat64, &SAM_PS_rbv_[1]); 
	createParam("SAM_PS_RBV_Z",     asynParamFloat64, &SAM_PS_rbv_[2]); 
	
	// NFO_SG_rbv (x, y, z)
	createParam("NFO_SG_RBV_X",     asynParamFloat64, &NFO_SG_rbv_[0]); 
	createParam("NFO_SG_RBV_Y",     asynParamFloat64, &NFO_SG_rbv_[1]); 
	createParam("NFO_SG_RBV_Z",     asynParamFloat64, &NFO_SG_rbv_[2]); 
	
	// SAM_CP_D_rbv (x, y, z)
	createParam("SAM_CP_D_RBV_X",   asynParamFloat64, &SAM_CP_D_rbv_[0]); 
	createParam("SAM_CP_D_RBV_Y",   asynParamFloat64, &SAM_CP_D_rbv_[1]); 
	createParam("SAM_CP_D_RBV_Z",   asynParamFloat64, &SAM_CP_D_rbv_[2]); 
	
	// XZ_ZX_rbv (0-1)
	createParam("XZ_ZX_RBV_0",      asynParamFloat64, &XZ_ZX_rbv_[0]); 
	createParam("XZ_ZX_RBV_1",      asynParamFloat64, &XZ_ZX_rbv_[1]); 
	
	// AUX_ADC_rbv (0-2)
	createParam("AUX_ADC_RBV_0",    asynParamFloat64, &AUX_ADC_rbv_[0]); 
	createParam("AUX_ADC_RBV_1",    asynParamFloat64, &AUX_ADC_rbv_[1]); 
	createParam("AUX_ADC_RBV_2",    asynParamFloat64, &AUX_ADC_rbv_[2]); 
	
	// NFO_rbv (x, y, z)
	createParam("NFO_RBV_X",        asynParamFloat64, &NFO_rbv_[0]); 
	createParam("NFO_RBV_Y",        asynParamFloat64, &NFO_rbv_[1]); 
	createParam("NFO_RBV_Z",        asynParamFloat64, &NFO_rbv_[2]); 
	
	// SAM_rbv (x, y, z)
	createParam("SAM_RBV_X",        asynParamFloat64, &SAM_rbv_[0]); 
	createParam("SAM_RBV_Y",        asynParamFloat64, &SAM_rbv_[1]); 
	createParam("SAM_RBV_Z",        asynParamFloat64, &SAM_rbv_[2]); 
	
	// Lissajous params (x, y, z)
	createParam("LISS_FREQ_X",      asynParamFloat64, &LissFreq_[0]);
	createParam("LISS_FREQ_Y",      asynParamFloat64, &LissFreq_[1]);
	createParam("LISS_FREQ_Z",      asynParamFloat64, &LissFreq_[2]);
	createParam("LISS_FREQ_RBV_X",  asynParamFloat64, &LissFreq_rbv_[0]);
	createParam("LISS_FREQ_RBV_Y",  asynParamFloat64, &LissFreq_rbv_[1]);
	createParam("LISS_FREQ_RBV_Z",  asynParamFloat64, &LissFreq_rbv_[2]);

	createParam("LISS_PHASE_X",      asynParamFloat64, &LissPhase_[0]);
	createParam("LISS_PHASE_Y",      asynParamFloat64, &LissPhase_[1]);
	createParam("LISS_PHASE_Z",      asynParamFloat64, &LissPhase_[2]);
	createParam("LISS_PHASE_RBV_X",  asynParamFloat64, &LissPhase_rbv_[0]);
	createParam("LISS_PHASE_RBV_Y",  asynParamFloat64, &LissPhase_rbv_[1]);
	createParam("LISS_PHASE_RBV_Z",  asynParamFloat64, &LissPhase_rbv_[2]);

	createParam("LISS_AMP_X",        asynParamFloat64, &LissAmp_[0]);
	createParam("LISS_AMP_Y",        asynParamFloat64, &LissAmp_[1]);
	createParam("LISS_AMP_Z",        asynParamFloat64, &LissAmp_[2]);
	createParam("LISS_AMP_RBV_X",    asynParamFloat64, &LissAmp_rbv_[0]);
	createParam("LISS_AMP_RBV_Y",    asynParamFloat64, &LissAmp_rbv_[1]);
	createParam("LISS_AMP_RBV_Z",    asynParamFloat64, &LissAmp_rbv_[2]);

	createParam("LISS_OFF_X",        asynParamFloat64, &LissOff_[0]);
	createParam("LISS_OFF_Y",        asynParamFloat64, &LissOff_[1]);
	createParam("LISS_OFF_Z",        asynParamFloat64, &LissOff_[2]);
	createParam("LISS_OFF_RBV_X",    asynParamFloat64, &LissOff_rbv_[0]);
	createParam("LISS_OFF_RBV_Y",    asynParamFloat64, &LissOff_rbv_[1]);
	createParam("LISS_OFF_RBV_Z",    asynParamFloat64, &LissOff_rbv_[2]);

	createParam("RESET_LISS_PHASE",  asynParamInt32, &ResetLissPhase_);
	
	
	
	
	
	// // SetptFreq (x, y, z)
	// createParam("SETPT_FREQ_X",         asynParamInt32, &SetptFreq_[0]); //
	// createParam("SETPT_FREQ_Y",         asynParamInt32, &SetptFreq_[1]); //
	// createParam("SETPT_FREQ_Z",         asynParamInt32, &SetptFreq_[2]); //
	// createParam("SETPT_FREQ_RBV_X",     asynParamInt32, &SetptFreq_rbv_[0]); //
	// createParam("SETPT_FREQ_RBV_Y",     asynParamInt32, &SetptFreq_rbv_[1]); //
	// createParam("SETPT_FREQ_RBV_Z",     asynParamInt32, &SetptFreq_rbv_[2]); //
	// 
	// // SetptPhase (x, y, z)
	// createParam("SETPT_PHASE_X",        asynParamInt32, &SetptPhase_[0]); //
	// createParam("SETPT_PHASE_Y",        asynParamInt32, &SetptPhase_[1]); //
	// createParam("SETPT_PHASE_Z",        asynParamInt32, &SetptPhase_[2]); //
	// createParam("SETPT_PHASE_RBV_X",    asynParamInt32, &SetptPhase_rbv_[0]); //
	// createParam("SETPT_PHASE_RBV_Y",    asynParamInt32, &SetptPhase_rbv_[1]); //
	// createParam("SETPT_PHASE_RBV_Z",    asynParamInt32, &SetptPhase_rbv_[2]); //
	// 
	// // SetptAmp (x, y, z)
	// createParam("SETPT_AMP_X",          asynParamInt32, &SetptAmp_[0]); //
	// createParam("SETPT_AMP_Y",          asynParamInt32, &SetptAmp_[1]); //
	// createParam("SETPT_AMP_Z",          asynParamInt32, &SetptAmp_[2]); //
	// createParam("SETPT_AMP_RBV_X",      asynParamInt32, &SetptAmp_rbv_[0]); //
	// createParam("SETPT_AMP_RBV_Y",      asynParamInt32, &SetptAmp_rbv_[1]); //
	// createParam("SETPT_AMP_RBV_Z",      asynParamInt32, &SetptAmp_rbv_[2]); //

	/////////////////////////////////////////////////////////////////////////
	
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
	
	// // PIIValNFO (x, y, z)
	// // TODO: make float write function
	// createParam("PI_I_VAL_NFO_X",       asynParamFloat64, &PIIValNFO_[0]);
	// createParam("PI_I_VAL_NFO_Y",       asynParamFloat64, &PIIValNFO_[1]);
	// createParam("PI_I_VAL_NFO_Z",       asynParamFloat64, &PIIValNFO_[2]);
	// createParam("PI_I_VAL_NFO_RBV_X",   asynParamFloat64, &PIIValNFO_rbv_[0]);
	// createParam("PI_I_VAL_NFO_RBV_Y",   asynParamFloat64, &PIIValNFO_rbv_[1]);
	// createParam("PI_I_VAL_NFO_RBV_Z",   asynParamFloat64, &PIIValNFO_rbv_[2]);
	// 
	// // PIPValNFO (x, y, z)
	// createParam("PI_P_VAL_NFO_X",       asynParamInt32, &PIPValNFO_[0]);
	// createParam("PI_P_VAL_NFO_Y",       asynParamInt32, &PIPValNFO_[1]);
	// createParam("PI_P_VAL_NFO_Z",       asynParamInt32, &PIPValNFO_[2]);
	// createParam("PI_P_VAL_NFO_RBV_X",   asynParamInt32, &PIPValNFO_rbv_[0]);
	// createParam("PI_P_VAL_NFO_RBV_Y",   asynParamInt32, &PIPValNFO_rbv_[1]);
	// createParam("PI_P_VAL_NFO_RBV_Z",   asynParamInt32, &PIPValNFO_rbv_[2]);
	// 
	// // PILimNFO (single value)
	// createParam("PI_LIM_NFO",           asynParamInt32, &PILimNFO_);
	// createParam("PI_LIM_NFO_RBV",       asynParamInt32, &PILimNFO_rbv_);
	// 
	// // PIAvgNFO (single value)
	// createParam("PI_AVG_NFO",           asynParamInt32, &PIAvgNFO_);
	// createParam("PI_AVG_NFO_RBV",       asynParamInt32, &PIAvgNFO_rbv_);
	// 
	// // PIEnSAM (x, y, z)
	// createParam("PI_EN_SAM_X",          asynParamInt32, &PIEnSAM_[0]);
	// createParam("PI_EN_SAM_Y",          asynParamInt32, &PIEnSAM_[1]);
	// createParam("PI_EN_SAM_Z",          asynParamInt32, &PIEnSAM_[2]);
	// createParam("PI_EN_SAM_RBV_X",      asynParamInt32, &PIEnSAM_rbv_[0]);
	// createParam("PI_EN_SAM_RBV_Y",      asynParamInt32, &PIEnSAM_rbv_[1]);
	// createParam("PI_EN_SAM_RBV_Z",      asynParamInt32, &PIEnSAM_rbv_[2]);
	// 
	// // PIIValSAM (x, y, z)
	// createParam("PI_I_VAL_SAM_X",       asynParamFloat64, &PIIValSAM_[0]);
	// createParam("PI_I_VAL_SAM_Y",       asynParamFloat64, &PIIValSAM_[1]);
	// createParam("PI_I_VAL_SAM_Z",       asynParamFloat64, &PIIValSAM_[2]);
	// createParam("PI_I_VAL_SAM_RBV_X",   asynParamFloat64, &PIIValSAM_rbv_[0]);
	// createParam("PI_I_VAL_SAM_RBV_Y",   asynParamFloat64, &PIIValSAM_rbv_[1]);
	// createParam("PI_I_VAL_SAM_RBV_Z",   asynParamFloat64, &PIIValSAM_rbv_[2]);
	// 
	// // PIPValSAM (x, y, z)
	// createParam("PI_P_VAL_SAM_X",       asynParamInt32, &PIPValSAM_[0]);
	// createParam("PI_P_VAL_SAM_Y",       asynParamInt32, &PIPValSAM_[1]);
	// createParam("PI_P_VAL_SAM_Z",       asynParamInt32, &PIPValSAM_[2]);
	// createParam("PI_P_VAL_SAM_RBV_X",   asynParamInt32, &PIPValSAM_rbv_[0]);
	// createParam("PI_P_VAL_SAM_RBV_Y",   asynParamInt32, &PIPValSAM_rbv_[1]);
	// createParam("PI_P_VAL_SAM_RBV_Z",   asynParamInt32, &PIPValSAM_rbv_[2]);
	// 
	// // PILimSAM (single value)
	// createParam("PI_LIM_SAM",           asynParamInt32, &PILimSAM_);
	// createParam("PI_LIM_SAM_RBV",       asynParamInt32, &PILimSAM_rbv_);
	// 
	// // PITargPos (x, y, z)
	// createParam("PI_TARG_POS_X",        asynParamInt32, &PITargPos_[0]);
	// createParam("PI_TARG_POS_Y",        asynParamInt32, &PITargPos_[1]);
	// createParam("PI_TARG_POS_Z",        asynParamInt32, &PITargPos_[2]);
	// createParam("PI_TARG_POS_RBV_X",    asynParamInt32, &PITargPos_rbv_[0]);
	// createParam("PI_TARG_POS_RBV_Y",    asynParamInt32, &PITargPos_rbv_[1]);
	// createParam("PI_TARG_POS_RBV_Z",    asynParamInt32, &PITargPos_rbv_[2]);
	// 
	// // PITargMode (single value)
	// createParam("PI_TARG_MODE",         asynParamInt32, &PITargMode_);
	// createParam("PI_TARG_MODE_RBV",     asynParamInt32, &PITargMode_rbv_);
	// 
	// // PINFOOut_rbv (x, y, z)
	// createParam("PI_NFO_OUT_RBV_X",     asynParamInt32, &PINFOOut_rbv_[0]);
	// createParam("PI_NFO_OUT_RBV_Y",     asynParamInt32, &PINFOOut_rbv_[1]);
	// createParam("PI_NFO_OUT_RBV_Z",     asynParamInt32, &PINFOOut_rbv_[2]);
	// 
	// // PISAMOut_rbv (x, y, z)
	// createParam("PI_SAM_OUT_RBV_X",     asynParamInt32, &PISAMOut_rbv_[0]);
	// createParam("PI_SAM_OUT_RBV_Y",     asynParamInt32, &PISAMOut_rbv_[1]);
	// createParam("PI_SAM_OUT_RBV_Z",     asynParamInt32, &PISAMOut_rbv_[2]);
	// 
	// // NFOADCLimMin/Max (single value)
	// createParam("NFO_ADC_LIM_MIN",      asynParamInt32, &NFOADCLimMin_);
	// createParam("NFO_ADC_LIM_MAX",      asynParamInt32, &NFOADCLimMax_);
	// createParam("NFO_ADC_LIM_MIN_RBV",  asynParamInt32, &NFOADCLimMin_rbv_);
	// createParam("NFO_ADC_LIM_MAX_RBV",  asynParamInt32, &NFOADCLimMax_rbv_);
	// 
	// // NFOSlewLim (single value)
	// createParam("NFO_SLEW_LIM",         asynParamInt32, &NFOSlewLim_);
	// createParam("NFO_SLEW_LIM_RBV",     asynParamInt32, &NFOSlewLim_rbv_);
	// 
	// // SAMADCLimMin/Max (single value)
	// createParam("SAM_ADC_LIM_MIN",      asynParamInt32, &SAMADCLimMin_);
	// createParam("SAM_ADC_LIM_MAX",      asynParamInt32, &SAMADCLimMax_);
	// createParam("SAM_ADC_LIM_MIN_RBV",  asynParamInt32, &SAMADCLimMin_rbv_);
	// createParam("SAM_ADC_LIM_MAX_RBV",  asynParamInt32, &SAMADCLimMax_rbv_);
	// 
	// // SAMSlewLim (single value)
	// createParam("SAM_SLEW_LIM",         asynParamInt32, &SAMSlewLim_);
	// createParam("SAM_SLEW_LIM_RBV",     asynParamInt32, &SAMSlewLim_rbv_);
	// 
	// // LimState_rbv (single value)
	// createParam("LIM_STATE_RBV",        asynParamInt32, &LimState_rbv_);
	
	// // InpTransMat (matrix: one createParam, float64 array, 3x15 = 45 elements)
	// createParam("INP_TRANS_MAT",        asynParamFloat64Array, InpTransMat_);   // 45 elements
	
	// createParam("INP_TRANS_MAT_ROW",    asynParamInt32, &InpTransMatSetRow_);
	// createParam("INP_TRANS_MAT_COL",    asynParamInt32, &InpTransMatSetCol_);
	// createParam("INP_TRANS_MAT_COEFF",  asynParamFloat64, &InpTransMatCoeff_);

	// Input transformation matrix coeffs
	char name[64];
	// Input transform coeffs: INP_TRANS_MAT_R{0..2}_C{0..14}
	for (int r = 0; r < 3; r++) {
	    for (int c = 0; c < 15; c++) {
	        epicsSnprintf(name, sizeof(name), "INP_TRANS_MAT_R%d_C%d", r, c);
	        createParam(name, asynParamFloat64, &InpTransMatCoeff_[r][c]);
	    }
	}
	
	// InpTransMatSave
	createParam("INP_TRANS_MAT_SAVE",  asynParamInt32, &InpTransMatSave_);
	
	// InpTransRes_rbv (x, y, z)
	createParam("INP_TRANS_RES_RBV_X",  asynParamFloat64, &InpTransRes_rbv_[0]);
	createParam("INP_TRANS_RES_RBV_Y",  asynParamFloat64, &InpTransRes_rbv_[1]);
	createParam("INP_TRANS_RES_RBV_Z",  asynParamFloat64, &InpTransRes_rbv_[2]);
	
	// InpTransAvg_rbv (single value)
	createParam("INP_TRANS_AVG_RBV",    asynParamInt32, &InpTransAvg_rbv_);
	

	// InpTransState_rbv (single value)
	createParam("INP_TRANS_STATE_RBV",  asynParamInt32, &InpTransState_rbv_);
	
	// // OutTransMat (matrix: one createParam, float64 array, 6x7 = 42 elements)
	// createParam("OUT_TRANS_MAT",        asynParamFloat64Array, OutTransMat_);   // 42 elements
	
	// createParam("OUT_TRANS_MAT_ROW",    asynParamInt32, &OutTransMatSetRow_);
	// createParam("OUT_TRANS_MAT_COL",    asynParamInt32, &OutTransMatSetCol_);
	// createParam("OUT_TRANS_MAT_COEFF",  asynParamFloat64, &OutTransMatCoeff_);
										    
	// Output transform coeffs: OUT_TRANS_MAT_R{0..5}_C{0..6}
	for (int r = 0; r < 6; r++) {
	    for (int c = 0; c < 7; c++) {
	        epicsSnprintf(name, sizeof(name), "OUT_TRANS_MAT_R%d_C%d", r, c);
	        createParam(name, asynParamFloat64, &OutTransMatCoeff_[r][c]);
	    }
	}
	// OutTransMatSave
	createParam("OUT_TRANS_MAT_SAVE",  asynParamInt32, &OutTransMatSave_);
	
	// OutTransNFORes_rbv (x, y, z)
	createParam("OUT_TRANS_NFO_RES_RBV_X", asynParamFloat64, &OutTransNFORes_rbv_[0]);
	createParam("OUT_TRANS_NFO_RES_RBV_Y", asynParamFloat64, &OutTransNFORes_rbv_[1]);
	createParam("OUT_TRANS_NFO_RES_RBV_Z", asynParamFloat64, &OutTransNFORes_rbv_[2]);
	
	// OutTransSAMRes_rbv (x, y, z)
	createParam("OUT_TRANS_SAM_RES_RBV_X", asynParamFloat64, &OutTransSAMRes_rbv_[0]);
	createParam("OUT_TRANS_SAM_RES_RBV_Y", asynParamFloat64, &OutTransSAMRes_rbv_[1]);
	createParam("OUT_TRANS_SAM_RES_RBV_Z", asynParamFloat64, &OutTransSAMRes_rbv_[2]);

	
	// Scan Params
	createParam("SCAN_START_X",         asynParamFloat64, &ScanStartX_);
	createParam("SCAN_START_X_RBV",     asynParamFloat64, &ScanStartX_rbv_);
	createParam("SCAN_END_X",           asynParamFloat64, &ScanEndX_);
	createParam("SCAN_END_X_RBV",       asynParamFloat64, &ScanEndX_rbv_);
	createParam("SCAN_SPEED_X",         asynParamInt32, &ScanSpeedX_);
	createParam("SCAN_SPEED_X_RBV",     asynParamInt32, &ScanSpeedX_rbv_);
	createParam("SCAN_START_Y",         asynParamFloat64, &ScanStartY_);
	createParam("SCAN_START_Y_RBV",     asynParamFloat64, &ScanStartY_rbv_);
	createParam("SCAN_DIST_Y",          asynParamFloat64, &ScanDistY_);
	createParam("SCAN_DIST_Y_RBV",      asynParamFloat64, &ScanDistY_rbv_);
	createParam("SCAN_COUNT_Y",         asynParamInt32, &ScanCountY_);
	createParam("SCAN_COUNT_Y_RBV",     asynParamInt32, &ScanCountY_rbv_);
	createParam("SCAN_TURN_TIME",       asynParamInt32, &ScanTurnTime_);
	createParam("SCAN_TURN_TIME_RBV",   asynParamInt32, &ScanTurnTime_rbv_);
	createParam("SCAN_POS_TIME",        asynParamInt32, &ScanPosTime_);
	createParam("SCAN_POS_TIME_RBV",    asynParamInt32, &ScanPosTime_rbv_);
	createParam("SCAN_SETTINGS",        asynParamInt32, &ScanSettings_);
	createParam("SCAN_SETTINGS_RBV",    asynParamInt32, &ScanSettings_rbv_);
	createParam("START_SCAN",    	    asynParamInt32, &StartScan_);
	createParam("STOP_SCAN",    	    asynParamInt32, &StopScan_);

	createParam("SHUTTER_STATE_RBV",    asynParamInt32, &ShutterState_rbv_);
        // shutter activation window, has upper and lower params for each axis
        createParam("SHUTTER_ACT_LOW_X_RBV",  asynParamFloat64, &ShutterActLow_rbv_[0]);
        createParam("SHUTTER_ACT_LOW_X",      asynParamFloat64, &ShutterActLow_[0]);
        createParam("SHUTTER_ACT_HIGH_X_RBV", asynParamFloat64, &ShutterActHigh_rbv_[0]);
        createParam("SHUTTER_ACT_HIGH_X",     asynParamFloat64, &ShutterActHigh_[0]);
        createParam("SHUTTER_ACT_LOW_Y_RBV",  asynParamFloat64, &ShutterActLow_rbv_[1]);
        createParam("SHUTTER_ACT_LOW_Y",      asynParamFloat64, &ShutterActLow_[1]);
        createParam("SHUTTER_ACT_HIGH_Y_RBV", asynParamFloat64, &ShutterActHigh_rbv_[1]);
        createParam("SHUTTER_ACT_HIGH_Y",     asynParamFloat64, &ShutterActHigh_[1]);

        createParam("SHUTTER_HYST_RBV", asynParamFloat64, &ShutterHyst_rbv_);
        createParam("SHUTTER_HYST",     asynParamFloat64, &ShutterHyst_);

        createParam("AREADETECTOR_COUNT_RBV", asynParamInt32, &AreaDetectorCount_rbv_);

        createParam("XRAY_INTENSITY_RBV", asynParamInt32, &XRayIntensity_rbv_);

        createParam("XRF_DEADTIME_STATUS_RBV", asynParamInt32, &XRFDeadTimeStatus_rbv_);

        // TODO: piezo model fit params -- not sure on approach yet
	// --- PIEZO_MODEL_FIT_* (type index: 0..4, dir index: 0..1) ---
	createParam("PIEZO_FIT_X0_FW_RBV", asynParamInt32, &PiezoModelFit_rbv_[0][0]);
	createParam("PIEZO_FIT_X0_FW",     asynParamInt32, &PiezoModelFit_[0][0]);
	createParam("PIEZO_FIT_X0_BW_RBV", asynParamInt32, &PiezoModelFit_rbv_[0][1]);
	createParam("PIEZO_FIT_X0_BW",     asynParamInt32, &PiezoModelFit_[0][1]);
	
	createParam("PIEZO_FIT_A1_FW_RBV", asynParamInt32, &PiezoModelFit_rbv_[1][0]);
	createParam("PIEZO_FIT_A1_FW",     asynParamInt32, &PiezoModelFit_[1][0]);
	createParam("PIEZO_FIT_A1_BW_RBV", asynParamInt32, &PiezoModelFit_rbv_[1][1]);
	createParam("PIEZO_FIT_A1_BW",     asynParamInt32, &PiezoModelFit_[1][1]);
	
	createParam("PIEZO_FIT_A2_FW_RBV", asynParamInt32, &PiezoModelFit_rbv_[2][0]);
	createParam("PIEZO_FIT_A2_FW",     asynParamInt32, &PiezoModelFit_[2][0]);
	createParam("PIEZO_FIT_A2_BW_RBV", asynParamInt32, &PiezoModelFit_rbv_[2][1]);
	createParam("PIEZO_FIT_A2_BW",     asynParamInt32, &PiezoModelFit_[2][1]);
	
	createParam("PIEZO_FIT_A3_FW_RBV", asynParamInt32, &PiezoModelFit_rbv_[3][0]);
	createParam("PIEZO_FIT_A3_FW",     asynParamInt32, &PiezoModelFit_[3][0]);
	createParam("PIEZO_FIT_A3_BW_RBV", asynParamInt32, &PiezoModelFit_rbv_[3][1]);
	createParam("PIEZO_FIT_A3_BW",     asynParamInt32, &PiezoModelFit_[3][1]);
	
	createParam("PIEZO_FIT_A4_FW_RBV", asynParamInt32, &PiezoModelFit_rbv_[4][0]);
	createParam("PIEZO_FIT_A4_FW",     asynParamInt32, &PiezoModelFit_[4][0]);
	createParam("PIEZO_FIT_A4_BW_RBV", asynParamInt32, &PiezoModelFit_rbv_[4][1]);
	createParam("PIEZO_FIT_A4_BW",     asynParamInt32, &PiezoModelFit_[4][1]);


        // trajectory mode
        createParam("TRAJ_MODE_RBV",    asynParamInt32, &TrajMode_rbv_);
        createParam("TRAJ_MODE",        asynParamInt32, &TrajMode_);

        // trajectory direct target
        createParam("TRAJ_DIR_TARGET_X_RBV", asynParamFloat64, &TrajDirTarget_rbv_[0]);
        createParam("TRAJ_DIR_TARGET_X",     asynParamFloat64, &TrajDirTarget_[0]);
        createParam("TRAJ_DIR_TARGET_Y_RBV", asynParamFloat64, &TrajDirTarget_rbv_[1]);
        createParam("TRAJ_DIR_TARGET_Y",     asynParamFloat64, &TrajDirTarget_[1]);
        createParam("TRAJ_DIR_TARGET_Z_RBV", asynParamFloat64, &TrajDirTarget_rbv_[2]);
        createParam("TRAJ_DIR_TARGET_Z",     asynParamFloat64, &TrajDirTarget_[2]);

        createParam("TRAJ_STATE_RBV", asynParamInt32, &TrajState_rbv_);

        createParam("CTRL_MEAS_VAL_X_RBV", asynParamFloat64, &CtrlMeasVal_rbv_[0]);
        createParam("CTRL_MEAS_VAL_Y_RBV", asynParamFloat64, &CtrlMeasVal_rbv_[1]);
        createParam("CTRL_MEAS_VAL_Z_RBV", asynParamFloat64, &CtrlMeasVal_rbv_[2]);

        createParam("CTRL_TRAJ_VAL_X_RBV", asynParamFloat64, &CtrlTrajVal_rbv_[0]);
        createParam("CTRL_TRAJ_VAL_Y_RBV", asynParamFloat64, &CtrlTrajVal_rbv_[1]);
        createParam("CTRL_TRAJ_VAL_Z_RBV", asynParamFloat64, &CtrlTrajVal_rbv_[2]);

        createParam("CTRL_SETTINGS_RBV", asynParamInt32, &CtrlSettings_rbv_);
        createParam("CTRL_SETTINGS",     asynParamInt32, &CtrlSettings_);

        createParam("PIXEL_RATE_TRIGGER_FACTOR", asynParamInt32, &PixelRateTriggerFactor_);
        createParam("PIXEL_RATE_TRIGGER_FACTOR_RBV", asynParamInt32, &PixelRateTriggerFactor_rbv_);
        createParam("AREA_DETECTOR_TRIGGER_DIVISOR", asynParamInt32, &AreaDetectorTriggerDivisor_);
        createParam("AREA_DETECTOR_TRIGGER_DIVISOR_RBV", asynParamInt32, &AreaDetectorTriggerDivisor_rbv_);
        createParam("ZYGO_RECEPTION_ACTIVE_RBV", asynParamInt32, &ZygoReceptionActive_rbv_);
        createParam("ZYGO_RECEPTION_RATE_ERROR_RBV", asynParamInt32, &ZygoReceptionRateError_rbv_);
        createParam("ZYGO_RECEPTION_ERROR_RBV", asynParamInt32, &ZygoReceptionError_rbv_);
        createParam("CLEAR_ZYGO_RECEPTION_ERROR", asynParamInt32, &ClearZygoReceptionError_);
        createParam("PIXEL_TRIGGER_OUTPUT_STATE", asynParamInt32, &PixelTriggerOutputState_);
        createParam("PIXEL_TRIGGER_OUTPUT_STATE_RBV", asynParamInt32, &PixelTriggerOutputState_rbv_);
        createParam("RESET_ZYGO_VALUES", asynParamInt32, &ResetZygoValues_);
        createParam("RESET_ZYGO_VALUE_OFFSETS", asynParamInt32, &ResetZygoValueOffsets_);


	// Force the device to connect now
	if (connect(this->pasynUserSelf) != asynSuccess) {
		setAllParamStatus(asynDisconnected);
	}

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

	this->lock();
	const bool alreadyConnected = connected_;
	this->unlock();
	if (alreadyConnected) return asynSuccess;
	unsigned int devCount = 0; // number of dscs devices available
	unsigned int devNo;

    	asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
        	"%s:%s: Connecting...\n", driverName, functionName);

	// discover available devices. IfAll - both usb and ethernet
  	errorCode = DSCS_discover(IfAll, &devCount);
	if (errorCode != DSCS_Ok) {
		checkError("DSCS_discover", errorCode);
		return asynError;
	}

	if (devCount == 0) {
		printf( "No devices found\n" );
		return asynError;
	}

	// search through available devices for desired ID
	bool found = false;
  	for (devNo = 0; devNo < devCount; devNo++) {
		int id = 0;
		char addr[20], serialNo[20];
    		errorCode = DSCS_getDeviceInfo(devNo, &id, serialNo, addr);
		if (errorCode != DSCS_Ok) {
			checkError("DSCS_getDeviceInfo", errorCode);
			continue;
		}
    		printf( "Device found: No=%d Id=%d SN=%s Addr=%s\n", devNo, id, serialNo, addr );   
		if (id == this->deviceId) {
			printf("connecting to device with ID %d\n", id);
			this->deviceNo = devNo;
			found = true;
			break;
		}
	}
	if (!found) {
		printf("ID not found\n");
		return asynError;
	}
	

	this->lock();
	DSCS_disconnect(this->deviceNo); // disconnect first
	errorCode = DSCS_connect(this->deviceNo);
	this->unlock();

	if (errorCode != DSCS_Ok) {
		checkError("DSCS_connect", errorCode);
		return asynError;
	}

    /* We found the controller and everything is OK.  Signal to asynManager that we are connected. */
    status = pasynManager->exceptionConnect(this->pasynUserSelf);
    if (status) {
        asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
            "%s:%s: error calling pasynManager->exceptionConnect, error=%s\n",
            driverName, functionName, pasynUserSelf->errorMessage);
        DSCS_disconnect(this->deviceNo);
        return asynError;
    }

	this->lock();
	connected_ = true;
	this->unlock();
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
	connected_ = false;
	setAllParamStatus(asynDisconnected);
  	errorCode = DSCS_disconnect(this->deviceNo);
	this->unlock();

	if (errorCode != DSCS_Ok && errorCode != DSCS_NotConnected) {
		checkError("DSCS_disconnect", errorCode);
	}

    status = pasynManager->exceptionDisconnect(this->pasynUserSelf);
    if (status) {
        asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
            "%s:%s: error calling pasynManager->exceptionDisconnect, error=%s\n",
            driverName, functionName, pasynUserSelf->errorMessage);
        return asynError;
    }

	return (errorCode == DSCS_Ok || errorCode == DSCS_NotConnected) ? asynSuccess : asynError;
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
    if (!connected_) {
      unlock();
      epicsThreadSleep(pollTime_);
      continue;
    }

    int value = 0, value2 = 0, errorCode;
    unsigned int uvalue = 0;
    bln32 bvalue = 0;
    DSCS_InputTransformationState inTransState = TransformationError;
    DSCS_ScanSettings scanSettings = None;

    // Axis enum mapping
    const DSCS_Axis axes[3] = {DSCS_AxisX, DSCS_AxisY, DSCS_AxisZ};

    // AUX channel enum mapping
    const DSCS_AUX_ADC auxChans[4] = {DSCS_AUX_0, DSCS_AUX_1, DSCS_AUX_2, DSCS_AUX_3};

    // XZ/ZX enum mapping
    const DSCS_XZ_ZX xz_zx[2] = {DSCS_XZ, DSCS_ZX};

    // --- OSA_PS_rbv_[2] ---
    for (int i = 0; i < 2; ++i) {
        errorCode = DSCS_getOSA_PS(deviceNo, axes[i], &value);
        updateDoubleParam("DSCS_getOSA_PS", errorCode, OSA_PS_rbv_[i], raw_uV_to_volts((int32_t)value));
    }

    // --- BS_PS_rbv_[2] ---
    for (int i = 0; i < 2; ++i) {
        errorCode = DSCS_getBS_PS(deviceNo, axes[i], &value);
        updateDoubleParam("DSCS_getBS_PS", errorCode, BS_PS_rbv_[i], raw_uV_to_volts((int32_t)value));
    }

    // --- AUX_DAC_rbv_[4] ---
    for (int i = 0; i < 4; ++i) {
        errorCode = DSCS_getAUX_DAC(deviceNo, auxChans[i], &value);
        updateDoubleParam("DSCS_getAUX_DAC", errorCode, AUX_DAC_rbv_[i], raw_uV_to_volts((int32_t)value));
    }

    // --- NFO_PS_rbv_[3] ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getNFO_PS(deviceNo, axes[i], &value);
        updateDoubleParam("DSCS_getNFO_PS", errorCode, NFO_PS_rbv_[i], raw_uV_to_volts((int32_t)value));
    }

    // --- SAM_PS_rbv_[3] ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getSAM_PS(deviceNo, axes[i], &value);
        updateDoubleParam("DSCS_getSAM_PS", errorCode, SAM_PS_rbv_[i], raw_uV_to_volts((int32_t)value));
    }

    // --- NFO_SG_rbv_[3] ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getNFO_SG(deviceNo, axes[i], &value);
        updateDoubleParam("DSCS_getNFO_SG", errorCode, NFO_SG_rbv_[i], raw_uV_to_volts((int32_t)value));
    }

    // --- SAM_CP_D_rbv_[3] ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getSAM_CP_D(deviceNo, axes[i], &value);
        updateDoubleParam("DSCS_getSAM_CP_D", errorCode, SAM_CP_D_rbv_[i], raw_uV_to_volts((int32_t)value));
    }

    // --- XZ_ZX_rbv_[2] ---
    for (int i = 0; i < 2; ++i) {
        errorCode = DSCS_getXZ_ZX(deviceNo, xz_zx[i], &value);
        updateDoubleParam("DSCS_getXZ_ZX", errorCode, XZ_ZX_rbv_[i], raw_uV_to_volts((int32_t)value));
    }

    // --- AUX_ADC_rbv_[3] ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getAUX_ADC(deviceNo, auxChans[i], &value);
        updateDoubleParam("DSCS_getAUX_ADC", errorCode, AUX_ADC_rbv_[i], raw_uV_to_volts((int32_t)value));
    }

    // --- NFO_rbv_[3] ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getNFO(deviceNo, axes[i], &value);
        updateDoubleParam("DSCS_getNFO", errorCode, NFO_rbv_[i], raw_uV_to_volts((int32_t)value));
    }

    // --- SAM_rbv_[3] ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getSAM(deviceNo, axes[i], &value);
        updateDoubleParam("DSCS_getSAM", errorCode, SAM_rbv_[i], raw_uV_to_volts((int32_t)value));
    }

    // --- LissFreq_rbv_[3] ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getLissajousFrequency(deviceNo, axes[i], &value);
        updateDoubleParam("DSCS_getLissajousFrequency", errorCode, LissFreq_rbv_[i], raw_to_liss_freq_khz((uint32_t)value));
    }

    // --- LissPhase_rbv_[3] ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getLissajousPhase(deviceNo, axes[i], &value);
        updateDoubleParam("DSCS_getLissajousPhase", errorCode, LissPhase_rbv_[i], raw_to_liss_phase_deg((uint32_t)value));
    }

    // --- LissAmp_rbv_[3] ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getLissajousAmplitude(deviceNo, axes[i], &value);
        updateDoubleParam("DSCS_getLissajousAmplitude", errorCode, LissAmp_rbv_[i], raw_steps_u32_to_nm((uint32_t)value));
    }

    // --- LissOff_rbv_[3] ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getLissajousOffset(deviceNo, axes[i], &value);
        updateDoubleParam("DSCS_getLissajousOffset", errorCode, LissOff_rbv_[i], raw_steps_to_nm((int32_t)value));
    }

    // // --- SetptFreq_rbv_[3] ---
    // for (int i = 0; i < 3; ++i) {
    //     errorCode = DSCS_getSetpointModulationFrequency(deviceNo, axes[i], &value);
    //     checkError("DSCS_getSetpointModulationFrequency", errorCode);
    //     setIntegerParam(SetptFreq_rbv_[i], value);
    // }

    // // --- SetptPhase_rbv_[3] ---
    // for (int i = 0; i < 3; ++i) {
    //     errorCode = DSCS_getSetpointModulationPhase(deviceNo, axes[i], &value);
    //     checkError("DSCS_getSetpointModulationPhase", errorCode);
    //     setIntegerParam(SetptPhase_rbv_[i], value);
    // }

    // // --- SetptAmp_rbv_[3] ---
    //         printf("getting setptamp");
    // for (int i = 0; i < 3; ++i) {
    //     errorCode = DSCS_getSetpointModulationAmplitude(deviceNo, axes[i], &value);
    //     checkError("DSCS_getSetpointModulationAmplitude", errorCode);
    //     setIntegerParam(SetptAmp_rbv_[i], value);
    // }

    // --- ExtADCShift_rbv_ ---
    errorCode = DSCS_getExternalADCShift(deviceNo, &value);
    updateIntegerParam("DSCS_getExternalADCShift", errorCode, ExtADCShift_rbv_, value);

    // --- PIEnNFO_rbv_[3] (bln32) ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getPIControllerEnabledNFO(deviceNo, axes[i], &bvalue);
        updateIntegerParam("DSCS_getPIControllerEnabledNFO", errorCode, PIEnNFO_rbv_[i], bvalue);
    }

    // // --- PIIValNFO_rbv_[3] ---
    // for (int i = 0; i < 3; ++i) {
    //         printf("before get");
    //     errorCode = DSCS_getPIControllerIValueNFO(deviceNo, axes[i], &dvalue);
    //         printf("after get");
    //     checkError("DSCS_getPIControllerIValueNFO", errorCode);
    //     setDoubleParam(PIIValNFO_rbv_[i], dvalue);
    // }

    // // --- PIPValNFO_rbv_[3] ---
    // for (int i = 0; i < 3; ++i) {
    //     errorCode = DSCS_getPIControllerPValueNFO(deviceNo, axes[i], &value);
    //     checkError("DSCS_getPIControllerPValueNFO", errorCode);
    //     setIntegerParam(PIPValNFO_rbv_[i], value);
    // }

    // // --- PILimNFO_rbv_ ---
    // errorCode = DSCS_getPIControllerLimitNFO(deviceNo, &value);
    // checkError("DSCS_getPIControllerLimitNFO", errorCode);
    // setIntegerParam(PILimNFO_rbv_, value);

    // // --- PIAvgNFO_rbv_ ---
    // errorCode = DSCS_getPIControllerAverageNFO(deviceNo, (unsigned short*)&value);
    // value = value & 0xFFFF; // mask to range of unsigned short
    // checkError("DSCS_getPIControllerAverageNFO", errorCode);
    // setIntegerParam(PIAvgNFO_rbv_, value);

    // // --- PIEnSAM_rbv_[3] (bln32) ---
    // for (int i = 0; i < 3; ++i) {
    //     errorCode = DSCS_getPIControllerEnabledSAM(deviceNo, axes[i], &bvalue);
    //     checkError("DSCS_getPIControllerEnabledSAM", errorCode);
    //     setIntegerParam(PIEnSAM_rbv_[i], bvalue);
    // }

    // // --- PIIValSAM_rbv_[3] ---
    // for (int i = 0; i < 3; ++i) {
    //     errorCode = DSCS_getPIControllerIValueSAM(deviceNo, axes[i], &dvalue);
    //     checkError("DSCS_getPIControllerIValueSAM", errorCode);
    //     setDoubleParam(PIIValSAM_rbv_[i], dvalue);
    // }

    // // --- PIPValSAM_rbv_[3] ---
    // for (int i = 0; i < 3; ++i) {
    //     errorCode = DSCS_getPIControllerPValueSAM(deviceNo, axes[i], &value);
    //     checkError("DSCS_getPIControllerPValueSAM", errorCode);
    //     setIntegerParam(PIPValSAM_rbv_[i], value);
    // }

    // // --- PILimSAM_rbv_ ---
    // errorCode = DSCS_getPIControllerLimitSAM(deviceNo, &value);
    // checkError("DSCS_getPIControllerLimitSAM", errorCode);
    // setIntegerParam(PILimSAM_rbv_, value);

    // // --- PITargPos_rbv_[3] ---
    // for (int i = 0; i < 3; ++i) {
    //     errorCode = DSCS_getPIControllerTargetPosition(deviceNo, axes[i], &value);
    //     checkError("DSCS_getPIControllerTargetPosition", errorCode);
    //     setIntegerParam(PITargPos_rbv_[i], value);
    // }

    // // --- PITargMode_rbv_ (enum) ---
    // errorCode = DSCS_getPIControllerTargetMode(deviceNo, &targetMode); // targetMode is typedef
    // checkError("DSCS_getPIControllerTargetMode", errorCode);
    // setIntegerParam(PITargMode_rbv_, targetMode);

    // // --- PINFOOut_rbv_[3] ---
    // for (int i = 0; i < 3; ++i) {
    //     errorCode = DSCS_getPIControllerNFOOutput(deviceNo, axes[i], &value);
    //     checkError("DSCS_getPIControllerNFOOutput", errorCode);
    //     setIntegerParam(PINFOOut_rbv_[i], value);
    // }

    // // --- PISAMOut_rbv_[3] ---
    // for (int i = 0; i < 3; ++i) {
    //     errorCode = DSCS_getPIControllerSAMOutput(deviceNo, axes[i], &value);
    //     checkError("DSCS_getPIControllerSAMOutput", errorCode);
    //     setIntegerParam(PISAMOut_rbv_[i], value);
    // }

    // // --- NFOADCLimMin_rbv_ & NFOADCLimMax_rbv_ ---
    // {
    //     int minValue, maxValue;
    //     errorCode = DSCS_getNFOADCLimits(deviceNo, &minValue, &maxValue);
    //     checkError("DSCS_getNFOADCLimits", errorCode);
    //     setIntegerParam(NFOADCLimMin_rbv_, minValue);
    //     setIntegerParam(NFOADCLimMax_rbv_, maxValue);
    // }

    // // --- NFOSlewLim_rbv_ ---
    // errorCode = DSCS_getNFOSlewRateLimit(deviceNo, &value);
    // checkError("DSCS_getNFOSlewRateLimit", errorCode);
    // setIntegerParam(NFOSlewLim_rbv_, value);

    // // --- SAMADCLimMin_rbv_ & SAMADCLimMax_rbv_ ---
    // {
    //     int minValue, maxValue;
    //     errorCode = DSCS_getSAMADCLimits(deviceNo, &minValue, &maxValue);
    //     checkError("DSCS_getSAMADCLimits", errorCode);
    //     setIntegerParam(SAMADCLimMin_rbv_, minValue);
    //     setIntegerParam(SAMADCLimMax_rbv_, maxValue);
    // }

    // // --- SAMSlewLim_rbv_ ---
    // errorCode = DSCS_getSAMSlewRateLimit(deviceNo, &value);
    // checkError("DSCS_getSAMSlewRateLimit", errorCode);
    // setIntegerParam(SAMSlewLim_rbv_, value);

    // // --- LimState_rbv_ (enum) ---
    // errorCode = DSCS_getLimiterState(deviceNo, &limiterState);
    // checkError("DSCS_getLimiterState", errorCode);
    // setIntegerParam(LimState_rbv_, limiterState);

    // --- InpTransRes_rbv_[3] ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getInputTransformationResult(deviceNo, axes[i], &value);
        updateDoubleParam("DSCS_getInputTransformationResult", errorCode, InpTransRes_rbv_[i], raw_steps_to_nm((int32_t)value));
    }

    // NOT FOUND IN LIB YET
    // --- InpTransAvg_rbv_ ---
    errorCode = DSCS_getInputTransformationAverage(deviceNo, &value);
    updateIntegerParam("DSCS_getInputTransformationAverage", errorCode, InpTransAvg_rbv_, value);

    // --- InpTransState_rbv_ (enum) ---
    errorCode = DSCS_getInputTransformationState(deviceNo, &inTransState);
    updateIntegerParam("DSCS_getInputTransformationState", errorCode, InpTransState_rbv_, inTransState);

    // --- OutTransNFORes_rbv_[3] and OutTransSAMRes_rbv_[3] ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getOutputTransformationResult(deviceNo, axes[i], &value, &value2);
        updateDoubleParam("DSCS_getOutputTransformationResult", errorCode, OutTransNFORes_rbv_[i], raw_to_out_trans_v((uint32_t)value));
        updateDoubleParam("DSCS_getOutputTransformationResult", errorCode, OutTransSAMRes_rbv_[i], raw_to_out_trans_v((uint32_t)value2));
    }

    // // --- OutTransSAMRes_rbv_[3] ---
    // for (int i = 0; i < 3; ++i) {
    // 	int dummy;
    //     errorCode = DSCS_getOutputTransformationResult(deviceNo, axes[i], &dummy, &value);
    //     checkError("DSCS_getOutputTransformationResult", errorCode);
    //     setIntegerParam(OutTransSAMRes_rbv_[i], value);
    // }

    // --- Line scan parameters (all single value) ---
    errorCode = DSCS_getScanLineStartX(deviceNo, &value);
    updateDoubleParam("DSCS_getScanLineStartX", errorCode, ScanStartX_rbv_, raw_steps_to_nm((int32_t)value));

    errorCode = DSCS_getScanLineEndX(deviceNo, &value);
    updateDoubleParam("DSCS_getScanLineEndX", errorCode, ScanEndX_rbv_, raw_steps_to_nm((int32_t)value));

    errorCode = DSCS_getScanLineSpeedX(deviceNo, &value);
    updateIntegerParam("DSCS_getScanLineSpeedX", errorCode, ScanSpeedX_rbv_, value);

    errorCode = DSCS_getScanLineStartY(deviceNo, &value);
    updateDoubleParam("DSCS_getScanLineStartY", errorCode, ScanStartY_rbv_, raw_steps_to_nm((int32_t)value));

    errorCode = DSCS_getScanLineDistY(deviceNo, &value);
    updateDoubleParam("DSCS_getScanLineDistY", errorCode, ScanDistY_rbv_, raw_steps_to_nm((int32_t)value));

    unsigned short lineCount = 0;
    errorCode = DSCS_getScanLineCountY(deviceNo, &lineCount);
    updateIntegerParam("DSCS_getScanLineCountY", errorCode, ScanCountY_rbv_, lineCount);

    // unsigned int
    errorCode = DSCS_getScanTurnTime(deviceNo, &uvalue);
    if (errorCode == DSCS_Ok && uvalue > INT_MAX) errorCode = DSCS_ParamOutOfRg;
    updateIntegerParam("DSCS_getScanTurnTime", errorCode, ScanTurnTime_rbv_, (epicsInt32)uvalue);

    // unsigned int
    errorCode = DSCS_getScanPosTime(deviceNo, &uvalue);
    if (errorCode == DSCS_Ok && uvalue > INT_MAX) errorCode = DSCS_ParamOutOfRg;
    updateIntegerParam("DSCS_getScanPosTime", errorCode, ScanPosTime_rbv_, (epicsInt32)uvalue);

    errorCode = DSCS_getScanSettings(deviceNo, &scanSettings);
    updateIntegerParam("DSCS_getScanSettings", errorCode, ScanSettings_rbv_, scanSettings);


    // --- SHUTTER_STATE_RBV ---
    DSCS_ShutterState shutterState = (DSCS_ShutterState)0;
    errorCode = DSCS_getShutterState(deviceNo, &shutterState);
    updateIntegerParam("DSCS_getShutterState", errorCode, ShutterState_rbv_, (int)shutterState);

    // --- SHUTTER_ACT_LOW/HIGH_*_RBV [3] ---
    for (int i = 0; i < 2; ++i) {
        errorCode = DSCS_getShutterActivationWindow(deviceNo, axes[i], &value, &value2);
        updateDoubleParam("DSCS_getShutterActivationWindow", errorCode, ShutterActLow_rbv_[i], raw_steps_to_nm((int32_t)value));
        updateDoubleParam("DSCS_getShutterActivationWindow", errorCode, ShutterActHigh_rbv_[i], raw_steps_to_nm((int32_t)value2));
    }

    // --- SHUTTER_HYST_RBV ---
    errorCode = DSCS_getShutterHysteresis(deviceNo, &value);
    updateDoubleParam("DSCS_getShutterHysteresis", errorCode, ShutterHyst_rbv_, raw_steps_to_nm((int32_t)value));

    // --- AREADETECTOR_COUNT_RBV (unsigned int) ---
    errorCode = DSCS_getAreaDetectorCounter(deviceNo, &uvalue);
    if (errorCode == DSCS_Ok && uvalue > INT_MAX) errorCode = DSCS_ParamOutOfRg;
    updateIntegerParam("DSCS_getAreaDetectorCounter", errorCode, AreaDetectorCount_rbv_, (epicsInt32)uvalue);

    // --- XRAY_INTENSITY_RBV (unsigned int) ---
    errorCode = DSCS_getXRayIntensityCounter(deviceNo, &uvalue);
    if (errorCode == DSCS_Ok && uvalue > INT_MAX) errorCode = DSCS_ParamOutOfRg;
    updateIntegerParam("DSCS_getXRayIntensityCounter", errorCode, XRayIntensity_rbv_, (epicsInt32)uvalue);

    // --- XRF_DEADTIME_STATUS_RBV (unsigned int) ---
    errorCode = DSCS_getXRFDeadTimeStatus(deviceNo, &uvalue);
    if (errorCode == DSCS_Ok && uvalue > INT_MAX) errorCode = DSCS_ParamOutOfRg;
    updateIntegerParam("DSCS_getXRFDeadTimeStatus", errorCode, XRFDeadTimeStatus_rbv_, (epicsInt32)uvalue);

    // --- PIEZO_FIT_*_RBV [5][2] ---
    for (int type = 0; type < 5; ++type) {
        for (int dir = 0; dir < 2; ++dir) {
            errorCode = DSCS_getPiezoModelFitParameters(deviceNo,
                                                       (DSCS_PiezoModelParameters)type,
                                                       (DSCS_Direction)dir,
                                                       &value);
            updateIntegerParam("DSCS_getPiezoModelFitParameters", errorCode, PiezoModelFit_rbv_[type][dir], value);
        }
    }


    // --- TRAJ_MODE_RBV ---
    DSCS_TrajectoryMode trajMode = TrajectoryMode_DirectTarget;
    errorCode = DSCS_getTrajectoryMode(deviceNo, &trajMode);
    updateIntegerParam("DSCS_getTrajectoryMode", errorCode, TrajMode_rbv_, (int)trajMode);

    // --- TRAJ_DIR_TARGET_*_RBV [3] ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getTrajectoryDirectTarget(deviceNo, axes[i], &value);
        updateDoubleParam("DSCS_getTrajectoryDirectTarget", errorCode, TrajDirTarget_rbv_[i], raw_steps_to_nm((int32_t)value));
    }

    // --- TRAJ_STATE_RBV ---
    DSCS_TrajectoryState trajState = (DSCS_TrajectoryState)0;
    errorCode = DSCS_getTrajectoryState(deviceNo, &trajState);
    updateIntegerParam("DSCS_getTrajectoryState", errorCode, TrajState_rbv_, (int)trajState);

    // --- CTRL_MEAS_VAL_*_RBV [3] ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getControllerMeasureValue(deviceNo, axes[i], &value);
        updateDoubleParam("DSCS_getControllerMeasureValue", errorCode, CtrlMeasVal_rbv_[i], raw_steps_to_nm((int32_t)value));
    }

    // --- CTRL_TRAJ_VAL_*_RBV [3] ---
    for (int i = 0; i < 3; ++i) {
        errorCode = DSCS_getControllerTrajectoryValue(deviceNo, axes[i], &value);
        updateDoubleParam("DSCS_getControllerTrajectoryValue", errorCode, CtrlTrajVal_rbv_[i], raw_steps_to_nm((int32_t)value));
    }

    // --- CTRL_SETTINGS_RBV ---
    DSCS_ControllerSettings ctrlSettings = (DSCS_ControllerSettings)0;
    errorCode = DSCS_getControllerSettings(deviceNo, &ctrlSettings);
    updateIntegerParam("DSCS_getControllerSettings", errorCode, CtrlSettings_rbv_, (int)ctrlSettings);

    errorCode = DSCS_getPixelRateTriggerFactor(deviceNo, &value);
    updateIntegerParam("DSCS_getPixelRateTriggerFactor", errorCode, PixelRateTriggerFactor_rbv_, value);

    errorCode = DSCS_getAreaDetectorTriggerDivisor(deviceNo, &value);
    updateIntegerParam("DSCS_getAreaDetectorTriggerDivisor", errorCode, AreaDetectorTriggerDivisor_rbv_, value);

    unsigned int active = 0, rateError = 0, zygoError = 0;
    errorCode = DSCS_getZygoDataReceptionState(deviceNo, &active, &rateError, &zygoError);
    updateIntegerParam("DSCS_getZygoDataReceptionState", errorCode, ZygoReceptionActive_rbv_, (epicsInt32)active);
    updateIntegerParam("DSCS_getZygoDataReceptionState", errorCode, ZygoReceptionRateError_rbv_, (epicsInt32)rateError);
    updateIntegerParam("DSCS_getZygoDataReceptionState", errorCode, ZygoReceptionError_rbv_, (epicsInt32)zygoError);

    errorCode = DSCS_getPixelTriggerOutputState(deviceNo, &bvalue);
    updateIntegerParam("DSCS_getPixelTriggerOutputState", errorCode, PixelTriggerOutputState_rbv_, bvalue);



    const bool connectionLost = !connected_;
    if (connectionLost) setAllParamStatus(asynDisconnected);
    unlock();

    callParamCallbacks();
    if (connectionLost) pasynManager->exceptionDisconnect(pasynUserSelf);
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

	if (!connected_) return asynDisconnected;

    asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, 
			"%s:%s, port %s, function = %d\n",
			driverName, functionName, this->portName, function);

	if ((function == PixelRateTriggerFactor_ && (value < 1 || value > 8)) ||
	    (function == AreaDetectorTriggerDivisor_ && (value < 1 || value > 65535)) ||
	    (function == PixelTriggerOutputState_ && value != 0 && value != 1)) {
		return asynError;
	}

	setIntegerParam(function, value);










    	// else if (function == SetptFreq_[0]) status = setSetpointModulationFrequency(DSCS_AxisX, value);
    	// else if (function == SetptFreq_[1]) status = setSetpointModulationFrequency(DSCS_AxisY, value);
    	// else if (function == SetptFreq_[2]) status = setSetpointModulationFrequency(DSCS_AxisZ, value);

    	// else if (function == SetptPhase_[0]) status = setSetpointModulationPhase(DSCS_AxisX, value);
    	// else if (function == SetptPhase_[1]) status = setSetpointModulationPhase(DSCS_AxisY, value);
    	// else if (function == SetptPhase_[2]) status = setSetpointModulationPhase(DSCS_AxisZ, value);

    	// else if (function == SetptAmp_[0]) status = setSetpointModulationAmplitude(DSCS_AxisX, value);
    	// else if (function == SetptAmp_[1]) status = setSetpointModulationAmplitude(DSCS_AxisY, value);
    	// else if (function == SetptAmp_[2]) status = setSetpointModulationAmplitude(DSCS_AxisZ, value);

    	if (function == ExtADCShift_) status = setExternalADCShift(value);

    	else if (function == PIEnNFO_[0]) status = setPIControllerEnabledNFO(DSCS_AxisX, value);
    	else if (function == PIEnNFO_[1]) status = setPIControllerEnabledNFO(DSCS_AxisY, value);
    	else if (function == PIEnNFO_[2]) status = setPIControllerEnabledNFO(DSCS_AxisZ, value);

    	// else if (function == PIIValNFO_[0]) status = setPIControllerIValueNFO(DSCS_AxisX, value);
    	// else if (function == PIIValNFO_[1]) status = setPIControllerIValueNFO(DSCS_AxisY, value);
    	// else if (function == PIIValNFO_[2]) status = setPIControllerIValueNFO(DSCS_AxisZ, value);

    	// else if (function == PIPValNFO_[0]) status = setPIControllerPValueNFO(DSCS_AxisX, value);
    	// else if (function == PIPValNFO_[1]) status = setPIControllerPValueNFO(DSCS_AxisY, value);
    	// else if (function == PIPValNFO_[2]) status = setPIControllerPValueNFO(DSCS_AxisZ, value);

    	// else if (function == PILimNFO_) status = setPIControllerLimitNFO(value);
    	// else if (function == PIAvgNFO_) status = setPIControllerAverageNFO(value);

    	// else if (function == PIEnSAM_[0]) status = setPIControllerEnabledSAM(DSCS_AxisX, value);
    	// else if (function == PIEnSAM_[1]) status = setPIControllerEnabledSAM(DSCS_AxisY, value);
    	// else if (function == PIEnSAM_[2]) status = setPIControllerEnabledSAM(DSCS_AxisZ, value);

	// move to float64
    	// else if (function == PIIValSAM_[0]) status = setPIControllerIValueSAM(DSCS_AxisX, value);
    	// else if (function == PIIValSAM_[1]) status = setPIControllerIValueSAM(DSCS_AxisY, value);
    	// else if (function == PIIValSAM_[2]) status = setPIControllerIValueSAM(DSCS_AxisZ, value);

    	// else if (function == PIPValSAM_[0]) status = setPIControllerPValueSAM(DSCS_AxisX, value);
    	// else if (function == PIPValSAM_[1]) status = setPIControllerPValueSAM(DSCS_AxisY, value);
    	// else if (function == PIPValSAM_[2]) status = setPIControllerPValueSAM(DSCS_AxisZ, value);

    	// else if (function == PILimSAM_) status = setPIControllerLimitSAM(value);

    	// else if (function == PITargPos_[0]) status = setPIControllerTargetPosition(DSCS_AxisX, value);
    	// else if (function == PITargPos_[1]) status = setPIControllerTargetPosition(DSCS_AxisY, value);
    	// else if (function == PITargPos_[2]) status = setPIControllerTargetPosition(DSCS_AxisZ, value);

    	// else if (function == PITargMode_) status = setPIControllerTargetMode(value);

    	// else if (function == NFOADCLimMin_) status = setNFOADCLimMin(value);
    	// else if (function == NFOADCLimMax_) status = setNFOADCLimMax(value);

    	// else if (function == NFOSlewLim_) status = setNFOSlewRateLimit(value);

    	// else if (function == SAMADCLimMin_) status = setSAMADCLimMin(value);
    	// else if (function == SAMADCLimMax_) status = setSAMADCLimMax(value);

    	// else if (function == SAMSlewLim_) status = setSAMSlewRateLimit(value);

    	// else if (function == InpTransMatSetRow_) InpTransMatRow = value;
    	// else if (function == InpTransMatSetCol_) InpTransMatCol = value;
    	else if (function == InpTransMatSave_) status = saveInputTransformationMatrix();

    	// else if (function == OutTransMatSetRow_) OutTransMatRow = value;
    	// else if (function == OutTransMatSetCol_) OutTransMatCol = value;
    	else if (function == OutTransMatSave_) status = saveOutputTransformationMatrix();

    	else if (function == ScanSpeedX_) status = setScanLineSpeedX(value);
    	else if (function == ScanCountY_) status = setScanLineCountY(value);
    	else if (function == ScanTurnTime_) status = setScanTurnTime(value);
    	else if (function == ScanPosTime_) status = setScanPosTime(value);
    	else if (function == ScanSettings_) status = setScanSettings(value);
    	else if (function == StartScan_) status = startScan();
    	else if (function == StopScan_) status = stopScan();

	// --- SHUTTER_ACT_LOW/HIGH_* [3] ---
	
	// --- SHUTTER_HYST ---

	// --- PIEZO_FIT_* [5][2] ---
	else if (function == PiezoModelFit_[0][0]) status = setPiezoFitParameter((DSCS_PiezoModelParameters)0, (DSCS_Direction)0, value);
	else if (function == PiezoModelFit_[0][1]) status = setPiezoFitParameter((DSCS_PiezoModelParameters)0, (DSCS_Direction)1, value);
	
	else if (function == PiezoModelFit_[1][0]) status = setPiezoFitParameter((DSCS_PiezoModelParameters)1, (DSCS_Direction)0, value);
	else if (function == PiezoModelFit_[1][1]) status = setPiezoFitParameter((DSCS_PiezoModelParameters)1, (DSCS_Direction)1, value);
	
	else if (function == PiezoModelFit_[2][0]) status = setPiezoFitParameter((DSCS_PiezoModelParameters)2, (DSCS_Direction)0, value);
	else if (function == PiezoModelFit_[2][1]) status = setPiezoFitParameter((DSCS_PiezoModelParameters)2, (DSCS_Direction)1, value);
	
	else if (function == PiezoModelFit_[3][0]) status = setPiezoFitParameter((DSCS_PiezoModelParameters)3, (DSCS_Direction)0, value);
	else if (function == PiezoModelFit_[3][1]) status = setPiezoFitParameter((DSCS_PiezoModelParameters)3, (DSCS_Direction)1, value);
	
	else if (function == PiezoModelFit_[4][0]) status = setPiezoFitParameter((DSCS_PiezoModelParameters)4, (DSCS_Direction)0, value);
	else if (function == PiezoModelFit_[4][1]) status = setPiezoFitParameter((DSCS_PiezoModelParameters)4, (DSCS_Direction)1, value);

	
	// --- TRAJ_MODE ---
	else if (function == TrajMode_) status = setTrajectoryMode(value);
	
	// --- TRAJ_DIR_TARGET_* [3] ---
	
	// --- CTRL_SETTINGS ---
	else if (function == CtrlSettings_) status = setControllerSettings(value);

	else if (function == PixelRateTriggerFactor_) status = setPixelRateTriggerFactor(value);
	else if (function == AreaDetectorTriggerDivisor_) status = setAreaDetectorTriggerDivisor(value);
	else if (function == ClearZygoReceptionError_ && value) status = clearZygoDataReceptionError();
	else if (function == PixelTriggerOutputState_) status = setPixelTriggerOutputState(value);
	else if (function == ResetZygoValues_ && value) status = resetZygoValues();
	else if (function == ResetZygoValueOffsets_ && value) status = resetZygoValueOffsets();


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

asynStatus dscsAsyn::writeFloat64(asynUser *pasynUser, epicsFloat64 value){
	int function = pasynUser->reason;
	const char *functionName = "writeFloat64";
	asynStatus status = asynSuccess;

	if (!connected_) return asynDisconnected;

	// if (function == InpTransMatCoeff_) status = setInputTransformationCoefficient(value);
	// else if (function == OutTransMatCoeff_) status = setOutputTransformationCoefficient(value);
	
	// normal params handled first
	// matrix coeffs handled last (TODO: ADD ELSE)
	
	bool handled = false;

	setDoubleParam(function, value);

	// Converted Int32->Float64 params (engineering units)
	if (function == OSA_PS_[0]) { status = setOSA_PS(DSCS_AxisX, value); handled = true; }
	else if (function == OSA_PS_[1]) { status = setOSA_PS(DSCS_AxisY, value); handled = true; }

	else if (function == BS_PS_[0]) { status = setBS_PS(DSCS_AxisX, value); handled = true; }
	else if (function == BS_PS_[1]) { status = setBS_PS(DSCS_AxisY, value); handled = true; }

	else if (function == AUX_DAC_[0]) { status = setAUX_DAC(DSCS_AUX_0, value); handled = true; }
	else if (function == AUX_DAC_[1]) { status = setAUX_DAC(DSCS_AUX_1, value); handled = true; }
	else if (function == AUX_DAC_[2]) { status = setAUX_DAC(DSCS_AUX_2, value); handled = true; }
	else if (function == AUX_DAC_[3]) { status = setAUX_DAC(DSCS_AUX_3, value); handled = true; }

	else if (function == NFO_PS_[0]) { status = setNFO_PS(DSCS_AxisX, value); handled = true; }
	else if (function == NFO_PS_[1]) { status = setNFO_PS(DSCS_AxisY, value); handled = true; }
	else if (function == NFO_PS_[2]) { status = setNFO_PS(DSCS_AxisZ, value); handled = true; }

	else if (function == SAM_PS_[0]) { status = setSAM_PS(DSCS_AxisX, value); handled = true; }
	else if (function == SAM_PS_[1]) { status = setSAM_PS(DSCS_AxisY, value); handled = true; }
	else if (function == SAM_PS_[2]) { status = setSAM_PS(DSCS_AxisZ, value); handled = true; }

	else if (function == LissFreq_[0]) { status = setLissFreq(DSCS_AxisX, value); handled = true; }
	else if (function == LissFreq_[1]) { status = setLissFreq(DSCS_AxisY, value); handled = true; }
	else if (function == LissFreq_[2]) { status = setLissFreq(DSCS_AxisZ, value); handled = true; }

	else if (function == LissPhase_[0]) { status = setLissPhase(DSCS_AxisX, value); handled = true; }
	else if (function == LissPhase_[1]) { status = setLissPhase(DSCS_AxisY, value); handled = true; }
	else if (function == LissPhase_[2]) { status = setLissPhase(DSCS_AxisZ, value); handled = true; }

	else if (function == LissAmp_[0]) { status = setLissAmp(DSCS_AxisX, value); handled = true; }
	else if (function == LissAmp_[1]) { status = setLissAmp(DSCS_AxisY, value); handled = true; }
	else if (function == LissAmp_[2]) { status = setLissAmp(DSCS_AxisZ, value); handled = true; }

	else if (function == LissOff_[0]) { status = setLissOff(DSCS_AxisX, value); handled = true; }
	else if (function == LissOff_[1]) { status = setLissOff(DSCS_AxisY, value); handled = true; }
	else if (function == LissOff_[2]) { status = setLissOff(DSCS_AxisZ, value); handled = true; }

	else if (function == ScanStartX_) { status = setScanLineStartX(value); handled = true; }
	else if (function == ScanEndX_) { status = setScanLineEndX(value); handled = true; }
	else if (function == ScanStartY_) { status = setScanLineStartY(value); handled = true; }
	else if (function == ScanDistY_) { status = setScanLineDistY(value); handled = true; }

	else if (function == ShutterActLow_[0])  { status = setShutterActivationLow(DSCS_AxisX, value); handled = true; }
	else if (function == ShutterActLow_[1])  { status = setShutterActivationLow(DSCS_AxisY, value); handled = true; }
	else if (function == ShutterActHigh_[0]) { status = setShutterActivationHigh(DSCS_AxisX, value); handled = true; }
	else if (function == ShutterActHigh_[1]) { status = setShutterActivationHigh(DSCS_AxisY, value); handled = true; }

	else if (function == ShutterHyst_) { status = setShutterHysteresis(value); handled = true; }

	else if (function == TrajDirTarget_[0]) { status = setTrajectoryDirectTarget(DSCS_AxisX, value); handled = true; }
	else if (function == TrajDirTarget_[1]) { status = setTrajectoryDirectTarget(DSCS_AxisY, value); handled = true; }
	else if (function == TrajDirTarget_[2]) { status = setTrajectoryDirectTarget(DSCS_AxisZ, value); handled = true; }


	// Input matrix: 3x15
        for (int r = 0; r < 3 && !handled; r++) {
            for (int c = 0; c < 15; c++) {
                if (function == InpTransMatCoeff_[r][c]) {
                    status = setInputTransformationCoefficient(r, c, value);
                    handled = true;
                    break;
                }
            }
        }

	// Input matrix: 6x7
	for (int r = 0; r < 6 && !handled; r++) {
            for (int c = 0; c < 7; c++) {
                if (function == OutTransMatCoeff_[r][c]) {
                    status = setOutputTransformationCoefficient(r, c, value);
                    handled = true;
                    break;
                }
            }
        }

	callParamCallbacks();

	if (status)
		epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize,
			"%s:%s: status=%d, function=%d",
			driverName, functionName, status, function);
	else
		asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
			"%s:%s: function=%d\n",
			driverName, functionName, function);

	return status;
}

// asynStatus dscsAsyn::setInputTransformationCoefficient(epicsFloat64 value) {
//     static const char *functionName = "setInputTransformationCoefficient";
//     asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, value = %f\n", driverName, functionName, this->portName, value);
//     return (DSCS_setInputTransformationCoefficient(deviceNo, InpTransMatRow, InpTransMatCol, value) == 0) ? asynSuccess : asynError;
// }
// asynStatus dscsAsyn::setOutputTransformationCoefficient(epicsFloat64 value) {
//     static const char *functionName = "setOutputTransformationCoefficient";
//     asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, value = %f\n", driverName, functionName, this->portName, value);
//     return (DSCS_setOutputTransformationCoefficient(deviceNo, OutTransMatRow, OutTransMatCol, value) == 0) ? asynSuccess : asynError;
// }

asynStatus dscsAsyn::setInputTransformationCoefficient(int row, int col, epicsFloat64 value)
{
    static const char *functionName = "setInputTransformationCoefficient";
    asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
              "%s:%s, port %s, row=%d, col=%d, value=%f\n",
              driverName, functionName, portName, row, col, value);
    return (DSCS_setInputTransformationCoefficient(deviceNo, row, col, value) == 0)
           ? asynSuccess : asynError;
}

asynStatus dscsAsyn::setOutputTransformationCoefficient(int row, int col, epicsFloat64 value)
{
    static const char *functionName = "setOutputTransformationCoefficient";
    asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
              "%s:%s, port %s, row=%d, col=%d, value=%f\n",
              driverName, functionName, portName, row, col, value);
    return (DSCS_setOutputTransformationCoefficient(deviceNo, row, col, value) == 0)
           ? asynSuccess : asynError;
}


// OSA_PS
asynStatus dscsAsyn::setOSA_PS(DSCS_Axis axis, epicsFloat64 volts) {
    static const char *functionName = "setOSA_PS";
    const int32_t raw = volts_to_raw_uV(volts);
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, axis = %d, volts = %f (raw=%d)\n",
              driverName, functionName, this->portName, axis, volts, (int)raw);
    return (DSCS_setOSA_PS(deviceNo, axis, (int)raw) == 0) ? asynSuccess : asynError;
}

// BS_PS
asynStatus dscsAsyn::setBS_PS(DSCS_Axis axis, epicsFloat64 volts) {
    static const char *functionName = "setBS_PS";
    const int32_t raw = volts_to_raw_uV(volts);
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, axis = %d, volts = %f (raw=%d)\n",
              driverName, functionName, this->portName, axis, volts, (int)raw);
    return (DSCS_setBS_PS(deviceNo, axis, (int)raw) == 0) ? asynSuccess : asynError;
}

// AUX_DAC
asynStatus dscsAsyn::setAUX_DAC(DSCS_AUX_ADC aux, epicsFloat64 volts) {
    static const char *functionName = "setAUX_DAC";
    const int32_t raw = volts_to_raw_uV(volts);
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, aux = %d, volts = %f (raw=%d)\n",
              driverName, functionName, this->portName, aux, volts, (int)raw);
    return (DSCS_setAUX_DAC(deviceNo, aux, (int)raw) == 0) ? asynSuccess : asynError;
}

// NFO_PS
asynStatus dscsAsyn::setNFO_PS(DSCS_Axis axis, epicsFloat64 volts) {
    static const char *functionName = "setNFO_PS";
    const int32_t raw = volts_to_raw_uV(volts);
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, axis = %d, volts = %f (raw=%d)\n",
              driverName, functionName, this->portName, axis, volts, (int)raw);
    return (DSCS_setNFO_PS(deviceNo, axis, (int)raw) == 0) ? asynSuccess : asynError;
}

// SAM_PS
asynStatus dscsAsyn::setSAM_PS(DSCS_Axis axis, epicsFloat64 volts) {
    static const char *functionName = "setSAM_PS";
    const int32_t raw = volts_to_raw_uV(volts);
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, axis = %d, volts = %f (raw=%d)\n",
              driverName, functionName, this->portName, axis, volts, (int)raw);
    return (DSCS_setSAM_PS(deviceNo, axis, (int)raw) == 0) ? asynSuccess : asynError;
}

// Lissajous params
asynStatus dscsAsyn::setLissFreq(DSCS_Axis axis, epicsFloat64 khz) {
    static const char *functionName = "setLissFreq";
    const uint32_t raw_u = liss_freq_khz_to_raw(khz);
    const int32_t raw = (int32_t)raw_u;
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, axis = %d, khz = %f (raw_u=%u)\n",
              driverName, functionName, this->portName, axis, khz, (unsigned)raw_u);
    return (DSCS_setLissajousFrequency(deviceNo, axis, (int)raw) == 0) ? asynSuccess : asynError;
}
asynStatus dscsAsyn::setLissPhase(DSCS_Axis axis, epicsFloat64 deg) {
    static const char *functionName = "setLissPhase";
    const uint32_t raw_u = liss_phase_deg_to_raw(deg);
    const int32_t raw = (int32_t)raw_u;
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, axis = %d, deg = %f (raw_u=%u)\n",
              driverName, functionName, this->portName, axis, deg, (unsigned)raw_u);
    return (DSCS_setLissajousPhase(deviceNo, axis, (int)raw) == 0) ? asynSuccess : asynError;
}
asynStatus dscsAsyn::setLissAmp(DSCS_Axis axis, epicsFloat64 nm) {
    static const char *functionName = "setLissAmp";
    const uint32_t raw_u = nm_to_raw_steps_u32(nm);
    const int32_t raw = (int32_t)raw_u;
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, axis = %d, nm = %f (raw_u=%u)\n",
              driverName, functionName, this->portName, axis, nm, (unsigned)raw_u);
    return (DSCS_setLissajousAmplitude(deviceNo, axis, (int)raw) == 0) ? asynSuccess : asynError;
}
asynStatus dscsAsyn::setLissOff(DSCS_Axis axis, epicsFloat64 nm) {
    static const char *functionName = "setLissOff";
    const int32_t raw = nm_to_raw_steps(nm);
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, axis = %d, nm = %f (raw=%d)\n",
              driverName, functionName, this->portName, axis, nm, (int)raw);
    return (DSCS_setLissajousOffset(deviceNo, axis, (int)raw) == 0) ? asynSuccess : asynError;
}

// // SetpointModulationFrequency
// asynStatus dscsAsyn::setSetpointModulationFrequency(DSCS_Axis axis, epicsInt32 value) {
//     static const char *functionName = "setSetpointModulationFrequency";
//     asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, axis = %d, value = %d\n", driverName, functionName, this->portName, axis, value);
//     return (DSCS_setSetpointModulationFrequency(deviceNo, axis, value) == 0) ? asynSuccess : asynError;
// }
// 
// // SetpointModulationPhase
// asynStatus dscsAsyn::setSetpointModulationPhase(DSCS_Axis axis, epicsInt32 value) {
//     static const char *functionName = "setSetpointModulationPhase";
//     asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, axis = %d, value = %d\n", driverName, functionName, this->portName, axis, value);
//     return (DSCS_setSetpointModulationPhase(deviceNo, axis, value) == 0) ? asynSuccess : asynError;
// }
// 
// // SetpointModulationAmplitude
// asynStatus dscsAsyn::setSetpointModulationAmplitude(DSCS_Axis axis, epicsInt32 value) {
//     static const char *functionName = "setSetpointModulationAmplitude";
//     asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, axis = %d, value = %d\n", driverName, functionName, this->portName, axis, value);
//     return (DSCS_setSetpointModulationAmplitude(deviceNo, axis, value) == 0) ? asynSuccess : asynError;
// }

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
// asynStatus dscsAsyn::setPIControllerPValueNFO(DSCS_Axis axis, epicsInt32 value) {
//     static const char *functionName = "setPIControllerPValueNFO";
//     asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, axis = %d, value = %d\n", driverName, functionName, this->portName, axis, value);
//     return (DSCS_setPIControllerPValueNFO(deviceNo, axis, value) == 0) ? asynSuccess : asynError;
// }
// 
// // NOT FOUND IN LIB YET
// asynStatus dscsAsyn::setPIControllerLimitNFO(epicsInt32 value) {
//     static const char *functionName = "setPIControllerLimitNFO";
//     asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, value = %d\n", driverName, functionName, this->portName, value);
//     // return (DSCS_setPIControllerLimitNFO(deviceNo, value) == 0) ? asynSuccess : asynError;
//     return asynSuccess;
// }
// // asynStatus dscsAsyn::setPIControllerAverageNFO(epicsInt32 value) {
// //     static const char *functionName = "setPIControllerAverageNFO";
// //     asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, value = %d\n", driverName, functionName, this->portName, value);
// //     return (DSCS_setPIControllerAverageNFO(deviceNo, (unsigned short)value) == 0) ? asynSuccess : asynError;
// // }
// 
// // PI Controller SAM
// asynStatus dscsAsyn::setPIControllerEnabledSAM(DSCS_Axis axis, epicsInt32 value) {
//     static const char *functionName = "setPIControllerEnabledSAM";
//     asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, axis = %d, value = %d\n", driverName, functionName, this->portName, axis, value);
//     return (DSCS_setPIControllerEnabledSAM(deviceNo, axis, value) == 0) ? asynSuccess : asynError;
// }
// // move to float64
// // asynStatus dscsAsyn::setPIControllerIValueSAM(DSCS_Axis axis, epicsInt32 value) {
// //     static const char *functionName = "setPIControllerIValueSAM";
// //     asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, axis = %d, value = %d\n", driverName, functionName, this->portName, axis, value);
// //     return (DSCS_setPIControllerIValueSAM(deviceNo, axis, value) == 0) ? asynSuccess : asynError;
// // }
// asynStatus dscsAsyn::setPIControllerPValueSAM(DSCS_Axis axis, epicsInt32 value) {
//     static const char *functionName = "setPIControllerPValueSAM";
//     asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, axis = %d, value = %d\n", driverName, functionName, this->portName, axis, value);
//     return (DSCS_setPIControllerPValueSAM(deviceNo, axis, value) == 0) ? asynSuccess : asynError;
// }
// asynStatus dscsAsyn::setPIControllerLimitSAM(epicsInt32 value) {
//     static const char *functionName = "setPIControllerLimitSAM";
//     asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, value = %d\n", driverName, functionName, this->portName, value);
//     return (DSCS_setPIControllerLimitSAM(deviceNo, value) == 0) ? asynSuccess : asynError;
// }
// 
// // PI Controller Target
// // asynStatus dscsAsyn::setPIControllerTargetPosition(DSCS_Axis axis, epicsInt32 value) {
// //     static const char *functionName = "setPIControllerTargetPosition";
// //     asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, axis = %d, value = %d\n", driverName, functionName, this->portName, axis, value);
// //     return (DSCS_setPIControllerTargetPosition(deviceNo, axis, value) == 0) ? asynSuccess : asynError;
// // }
// // asynStatus dscsAsyn::setPIControllerTargetMode(epicsInt32 value) {
// //     static const char *functionName = "setPIControllerTargetMode";
// //     asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, value = %d\n", driverName, functionName, this->portName, value);
// //     return (DSCS_setPIControllerTargetMode(deviceNo, (DSCS_TargetMode)value) == 0) ? asynSuccess : asynError; // cast to DSCS_TargetMode
// // }
// 
// // NFOADCLimits (min/max)
// asynStatus dscsAsyn::setNFOADCLimMin(epicsInt32 value) {
//     static const char *functionName = "setNFOADCLimMin";
//     asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, min = %d\n", driverName, functionName, this->portName, value);
//     int max = 0;
//     int err = DSCS_getNFOADCLimits(deviceNo, nullptr, &max);
//     if (err != 0) {
//         asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "%s:%s, port %s, ERROR reading max: %d\n", driverName, functionName, this->portName, err);
//         return asynError;
//     }
//     return (DSCS_setNFOADCLimits(deviceNo, value, max) == 0) ? asynSuccess : asynError;
// }
// asynStatus dscsAsyn::setNFOADCLimMax(epicsInt32 value) {
//     static const char *functionName = "setNFOADCLimMax";
//     asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, max = %d\n", driverName, functionName, this->portName, value);
//     int min = 0;
//     int err = DSCS_getNFOADCLimits(deviceNo, &min, nullptr);
//     if (err != 0) {
//         asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "%s:%s, port %s, ERROR reading min: %d\n", driverName, functionName, this->portName, err);
//         return asynError;
//     }
//     return (DSCS_setNFOADCLimits(deviceNo, min, value) == 0) ? asynSuccess : asynError;
// }
// 
// // NFOSlewRateLimit
// asynStatus dscsAsyn::setNFOSlewRateLimit(epicsInt32 value) {
//     static const char *functionName = "setNFOSlewRateLimit";
//     asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, value = %d\n", driverName, functionName, this->portName, value);
//     return (DSCS_setNFOSlewRateLimit(deviceNo, value) == 0) ? asynSuccess : asynError;
// }
// 
// // SAMADCLimits (min/max)
// asynStatus dscsAsyn::setSAMADCLimMin(epicsInt32 value) {
//     static const char *functionName = "setSAMADCLimMin";
//     asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, min = %d\n", driverName, functionName, this->portName, value);
//     int max = 0;
//     int err = DSCS_getSAMADCLimits(deviceNo, nullptr, &max);
//     if (err != 0) {
//         asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "%s:%s, port %s, ERROR reading max: %d\n", driverName, functionName, this->portName, err);
//         return asynError;
//     }
//     return (DSCS_setSAMADCLimits(deviceNo, value, max) == 0) ? asynSuccess : asynError;
// }
// asynStatus dscsAsyn::setSAMADCLimMax(epicsInt32 value) {
//     static const char *functionName = "setSAMADCLimMax";
//     asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, max = %d\n", driverName, functionName, this->portName, value);
//     int min = 0;
//     int err = DSCS_getSAMADCLimits(deviceNo, &min, nullptr);
//     if (err != 0) {
//         asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR, "%s:%s, port %s, ERROR reading min: %d\n", driverName, functionName, this->portName, err);
//         return asynError;
//     }
//     return (DSCS_setSAMADCLimits(deviceNo, min, value) == 0) ? asynSuccess : asynError;
// }
// 
// // SAMSlewRateLimit
// asynStatus dscsAsyn::setSAMSlewRateLimit(epicsInt32 value) {
//     static const char *functionName = "setSAMSlewRateLimit";
//     asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, value = %d\n", driverName, functionName, this->portName, value);
//     return (DSCS_setSAMSlewRateLimit(deviceNo, value) == 0) ? asynSuccess : asynError;
// }

// Trigger save of input matrix
asynStatus dscsAsyn::saveInputTransformationMatrix(void) {
    static const char *functionName = "saveInputTransformationMatrix";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s\n", driverName, functionName, this->portName);
    return (DSCS_saveInputTransformationMatrix(deviceNo) == 0) ? asynSuccess : asynError;
}

// Trigger save of output matrix
asynStatus dscsAsyn::saveOutputTransformationMatrix(void) {
    static const char *functionName = "saveOutputTransformationMatrix";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s\n", driverName, functionName, this->portName);
    return (DSCS_saveOutputTransformationMatrix(deviceNo) == 0) ? asynSuccess : asynError;
}

// Scan line parameters
asynStatus dscsAsyn::setScanLineStartX(epicsFloat64 nm) {
    static const char *functionName = "setScanLineStartX";
    const int32_t raw = nm_to_raw_steps(nm);
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, nm = %f (raw=%d)\n",
              driverName, functionName, this->portName, nm, (int)raw);
    return (DSCS_setScanLineStartX(deviceNo, (int)raw) == 0) ? asynSuccess : asynError;
}
asynStatus dscsAsyn::setScanLineEndX(epicsFloat64 nm) {
    static const char *functionName = "setScanLineEndX";
    const int32_t raw = nm_to_raw_steps(nm);
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, nm = %f (raw=%d)\n",
              driverName, functionName, this->portName, nm, (int)raw);
    return (DSCS_setScanLineEndX(deviceNo, (int)raw) == 0) ? asynSuccess : asynError;
}
asynStatus dscsAsyn::setScanLineSpeedX(epicsInt32 value) {
    static const char *functionName = "setScanLineSpeedX";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, value = %d\n", driverName, functionName, this->portName, value);
    return (DSCS_setScanLineSpeedX(deviceNo, value) == 0) ? asynSuccess : asynError;
}
asynStatus dscsAsyn::setScanLineStartY(epicsFloat64 nm) {
    static const char *functionName = "setScanLineStartY";
    const int32_t raw = nm_to_raw_steps(nm);
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, nm = %f (raw=%d)\n",
              driverName, functionName, this->portName, nm, (int)raw);
    return (DSCS_setScanLineStartY(deviceNo, (int)raw) == 0) ? asynSuccess : asynError;
}
asynStatus dscsAsyn::setScanLineDistY(epicsFloat64 nm) {
    static const char *functionName = "setScanLineDistY";
    const int32_t raw = nm_to_raw_steps(nm);
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, nm = %f (raw=%d)\n",
              driverName, functionName, this->portName, nm, (int)raw);
    return (DSCS_setScanLineDistY(deviceNo, (int)raw) == 0) ? asynSuccess : asynError;
}
asynStatus dscsAsyn::setScanLineCountY(epicsInt32 value) {
    static const char *functionName = "setScanLineCountY";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, value = %d\n", driverName, functionName, this->portName, value);
    return (DSCS_setScanLineCountY(deviceNo, value) == 0) ? asynSuccess : asynError;
}
asynStatus dscsAsyn::setScanTurnTime(epicsInt32 value) {
    static const char *functionName = "setScanTurnTime";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, value = %d\n", driverName, functionName, this->portName, value);
    return (DSCS_setScanTurnTime(deviceNo, value) == 0) ? asynSuccess : asynError;
}
asynStatus dscsAsyn::setScanPosTime(epicsInt32 value) {
    static const char *functionName = "setScanPosTime";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, value = %d\n", driverName, functionName, this->portName, value);
    return (DSCS_setScanPosTime(deviceNo, value) == 0) ? asynSuccess : asynError;
}
asynStatus dscsAsyn::setScanSettings(epicsInt32 value) {
    static const char *functionName = "setScanSettings";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, value = %d\n", driverName, functionName, this->portName, value);
    return (DSCS_setScanSettings(deviceNo, (DSCS_ScanSettings)value) == 0) ? asynSuccess : asynError;
}
asynStatus dscsAsyn::startScan(void) {
    static const char *functionName = "startScan";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s\n", driverName, functionName, this->portName);
    return (DSCS_startScan(deviceNo) == 0) ? asynSuccess : asynError;
}
asynStatus dscsAsyn::stopScan(void) {
    static const char *functionName = "stopScan";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s\n", driverName, functionName, this->portName);
    return (DSCS_stopScan(deviceNo) == 0) ? asynSuccess : asynError;
}

asynStatus dscsAsyn::setShutterActivationLow(DSCS_Axis axis, epicsFloat64 lower_nm)
{
    static const char *functionName = "setShutterActivationLow";
    epicsFloat64 upper_nm = 0.0;

    // Use current HIGH setpoint for the same axis
    if (axis == DSCS_AxisX)      getDoubleParam(ShutterActHigh_[0], &upper_nm);
    else if (axis == DSCS_AxisY) getDoubleParam(ShutterActHigh_[1], &upper_nm);

    const int32_t lower_raw = nm_to_raw_steps(lower_nm);
    const int32_t upper_raw = nm_to_raw_steps(upper_nm);

    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER,
              "%s:%s, port %s, axis=%d, lower_nm=%f (raw=%d), upper_nm=%f (raw=%d)\n",
              driverName, functionName, this->portName, axis, lower_nm, (int)lower_raw, upper_nm, (int)upper_raw);

    return (DSCS_setShutterActivationWindow(deviceNo, axis, (int)lower_raw, (int)upper_raw) == 0)
             ? asynSuccess : asynError;
}

asynStatus dscsAsyn::setShutterActivationHigh(DSCS_Axis axis, epicsFloat64 upper_nm)
{
    static const char *functionName = "setShutterActivationHigh";
    epicsFloat64 lower_nm = 0.0;

    // Use current LOW setpoint for the same axis
    if (axis == DSCS_AxisX)      getDoubleParam(ShutterActLow_[0], &lower_nm);
    else if (axis == DSCS_AxisY) getDoubleParam(ShutterActLow_[1], &lower_nm);

    const int32_t lower_raw = nm_to_raw_steps(lower_nm);
    const int32_t upper_raw = nm_to_raw_steps(upper_nm);

    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER,
              "%s:%s, port %s, axis=%d, lower_nm=%f (raw=%d), upper_nm=%f (raw=%d)\n",
              driverName, functionName, this->portName, axis, lower_nm, (int)lower_raw, upper_nm, (int)upper_raw);

    return (DSCS_setShutterActivationWindow(deviceNo, axis, (int)lower_raw, (int)upper_raw) == 0)
             ? asynSuccess : asynError;
}

asynStatus dscsAsyn::setShutterHysteresis(epicsFloat64 nm)
{
    static const char *functionName = "setShutterHysteresis";
    const int32_t raw = nm_to_raw_steps(nm);

    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER,
              "%s:%s, port %s, nm=%f (raw=%d)\n",
              driverName, functionName, this->portName, nm, (int)raw);

    return (DSCS_setShutterHysteresis(deviceNo, (int)raw) == 0)
             ? asynSuccess : asynError;
}

asynStatus dscsAsyn::setPiezoFitParameter(DSCS_PiezoModelParameters type,
                                         DSCS_Direction dir,
                                         epicsInt32 value)
{
    static const char *functionName = "setPiezoFitParameter";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER,
              "%s:%s, port %s, type = %d, dir = %d, value = %d\n",
              driverName, functionName, this->portName, (int)type, (int)dir, (int)value);

    return (DSCS_setPiezoModelFitParameters(deviceNo, type, dir, (int)value) == 0) ? asynSuccess : asynError;
}


asynStatus dscsAsyn::setTrajectoryMode(epicsInt32 value)
{
    static const char *functionName = "setTrajectoryMode";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER,
              "%s:%s, port %s, value=%d\n",
              driverName, functionName, this->portName, (int)value);

    return (DSCS_setTrajectoryMode(deviceNo, (DSCS_TrajectoryMode)value) == 0)
             ? asynSuccess : asynError;
}

asynStatus dscsAsyn::setTrajectoryDirectTarget(DSCS_Axis axis, epicsFloat64 nm) {
    static const char *functionName = "setTrajectoryDirectTarget";
    const int32_t raw = nm_to_raw_steps(nm);
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER, "%s:%s, port %s, axis = %d, nm = %f (raw=%d)\n",
              driverName, functionName, this->portName, axis, nm, (int)raw);
    return (DSCS_setTrajectoryDirectTarget(deviceNo, axis, (int)raw) == 0) ? asynSuccess : asynError;
}

asynStatus dscsAsyn::setControllerSettings(epicsInt32 value)
{
    static const char *functionName = "setControllerSettings";
    asynPrint(this->pasynUserSelf, ASYN_TRACEIO_DRIVER,
              "%s:%s, port %s, value=%d\n",
              driverName, functionName, this->portName, (int)value);

    return (DSCS_setControllerSettings(deviceNo, (DSCS_ControllerSettings)value) == 0)
             ? asynSuccess : asynError;
}

asynStatus dscsAsyn::setPixelRateTriggerFactor(epicsInt32 value)
{
    if (value < 1 || value > 8) return asynError;
    return (DSCS_setPixelRateTriggerFactor(deviceNo, value) == DSCS_Ok)
             ? asynSuccess : asynError;
}

asynStatus dscsAsyn::setAreaDetectorTriggerDivisor(epicsInt32 value)
{
    if (value < 1 || value > 65535) return asynError;
    return (DSCS_setAreaDetectorTriggerDivisor(deviceNo, value) == DSCS_Ok)
             ? asynSuccess : asynError;
}

asynStatus dscsAsyn::clearZygoDataReceptionError(void)
{
    return (DSCS_clearZygoDataReceptionError(deviceNo) == DSCS_Ok)
             ? asynSuccess : asynError;
}

asynStatus dscsAsyn::setPixelTriggerOutputState(epicsInt32 value)
{
    if (value != 0 && value != 1) return asynError;
    return (DSCS_setPixelTriggerOutputState(deviceNo, value) == DSCS_Ok)
             ? asynSuccess : asynError;
}

asynStatus dscsAsyn::resetZygoValues(void)
{
    return (DSCS_resetZygoValues(deviceNo) == DSCS_Ok)
             ? asynSuccess : asynError;
}

asynStatus dscsAsyn::resetZygoValueOffsets(void)
{
    return (DSCS_resetZygoValueOffsets(deviceNo) == DSCS_Ok)
             ? asynSuccess : asynError;
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