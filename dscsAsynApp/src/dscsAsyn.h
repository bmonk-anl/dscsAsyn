/*
 * 
 * 
 * 
 */



#include <asynPortDriver.h>

static const char *driverName = "dscsAsyn";

#define MAX_CONTROLLERS	1
#define DEFAULT_POLL_TIME 1

#define DEFAULT_CONTROLLER_TIMEOUT 2.0

/*
 * Class definition for the dscsAsyn class
 */
class dscsAsyn: public asynPortDriver {
public:
    dscsAsyn(const char *portName, const char *dscsAsynPortName, int dscsId);
    virtual ~dscsAsyn();
    
    /* These are the methods that we override from asynPortDriver */
    // virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    // virtual asynStatus readInt32(asynUser *pasynUser, epicsInt32 *value);
    // virtual asynStatus readFloat64(asynUser *pasynUser, epicsFloat64 *value);
    // virtual asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value);
    // value - pointer to string
    // maxChars - max num of characters to read
    //
    // virtual asynStatus readOctet(asynUser *pasynUser, char* value, size_t maxChars, 
    //    size_t* nActual, int* eomReason);
    // virtual asynStatus disconnect(asynUser *pasynUser);
    // virtual asynStatus connect(asynUser *pasynUser);
    // These should be private but are called from C

    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

    virtual asynStatus connect(asynUser *pasynUser);
    virtual asynStatus disconnect(asynUser *pasynUser);
    virtual void pollerThread(void);

	void pollAnalogIn();

protected:

	int OSA_PS_[2];          // x and y axis; DSCS_setOSA_PS
	int OSA_PS_rbv_[2];      // x and y axis; DSCS_getOSA_PS
	
	int BS_PS_[2];           // x and y axis; DSCS_setBS_PS
	int BS_PS_rbv_[2];       // x and y axis; DSCS_getBS_PS
	
	int AUX_DAC_[4];         // index 0-3; DSCS_setAUX_DAC
	int AUX_DAC_rbv_[4];     // index 0-3; DSCS_getAUX_DAC
	
	int NFO_PS_[3];          // x, y, z axis; DSCS_setNFO_PS
	int NFO_PS_rbv_[3];      // x, y, z axis; DSCS_getNFO_PS
	
	int SAM_PS_[3];          // x, y, z axis; DSCS_setSAM_PS
	int SAM_PS_rbv_[3];      // x, y, z axis; DSCS_getSAM_PS
	
	int NFO_SG_rbv_[3];      // x, y, z axis; DSCS_getNFO_SG
	int SAM_CP_D_rbv_[3];    // x, y, z axis; DSCS_getSAM_CP_D
	int XZ_ZX_rbv_[2];       // index 0-1; DSCS_getXZ_ZX
	int AUX_ADC_rbv_[3];     // index 0-2; DSCS_getAUX_ADC
	int NFO_rbv_[3];         // x, y, z axis; DSCS_getNFO
	int SAM_rbv_[3];         // x, y, z axis; DSCS_getSAM
	
	int SetptFreq_[3];       // x, y, z axis; full: SetpointModulationFrequency; DSCS_setSetpointModulationFrequency
	int SetptFreq_rbv_[3];   // x, y, z axis; full: SetpointModulationFrequency; DSCS_getSetpointModulationFrequency
	
	int SetptPhase_[3];      // x, y, z axis; full: SetpointModulationPhase; DSCS_setSetpointModulationPhase
	int SetptPhase_rbv_[3];  // x, y, z axis; full: SetpointModulationPhase; DSCS_getSetpointModulationPhase
	
	int SetptAmp_[3];        // x, y, z axis; full: SetpointModulationAmplitude; DSCS_setSetpointModulationAmplitude
	int SetptAmp_rbv_[3];    // x, y, z axis; full: SetpointModulationAmplitude; DSCS_getSetpointModulationAmplitude
	
	int ExtADCShift_;        // single value; full: ExternalADCShift; DSCS_setExternalADCShift
	int ExtADCShift_rbv_;    // single value; full: ExternalADCShift; DSCS_getExternalADCShift
	
	int PIEnNFO_[3];         // x, y, z axis; full: PIControllerEnabledNFO; DSCS_setPIControllerEnabledNFO
	int PIEnNFO_rbv_[3];     // x, y, z axis; full: PIControllerEnabledNFO; DSCS_getPIControllerEnabledNFO
	
	int PIIValNFO_[3];       // x, y, z axis; full: PIControllerIValueNFO; DSCS_setPIControllerIValueNFO
	int PIIValNFO_rbv_[3];   // x, y, z axis; full: PIControllerIValueNFO; DSCS_getPIControllerIValueNFO
	
	int PIPValNFO_[3];       // x, y, z axis; full: PIControllerPValueNFO; DSCS_setPIControllerPValueNFO
	int PIPValNFO_rbv_[3];   // x, y, z axis; full: PIControllerPValueNFO; DSCS_getPIControllerPValueNFO
	
	int PILimNFO_;           // single value; full: PIControllerLimitNFO; DSCS_setPIControllerLimitNFO
	int PILimNFO_rbv_;       // single value; full: PIControllerLimitNFO; DSCS_getPIControllerLimitNFO
	
	int PIAvgNFO_;           // single value; full: PIControllerAverageNFO; DSCS_setPIControllerAverageNFO
	int PIAvgNFO_rbv_;       // single value; full: PIControllerAverageNFO; DSCS_getPIControllerAverageNFO
	
	int PIEnSAM_[3];         // x, y, z axis; full: PIControllerEnabledSAM; DSCS_setPIControllerEnabledSAM
	int PIEnSAM_rbv_[3];     // x, y, z axis; full: PIControllerEnabledSAM; DSCS_getPIControllerEnabledSAM
	
	int PIIValSAM_[3];       // x, y, z axis; full: PIControllerIValueSAM; DSCS_setPIControllerIValueSAM
	int PIIValSAM_rbv_[3];   // x, y, z axis; full: PIControllerIValueSAM; DSCS_getPIControllerIValueSAM
	
	int PIPValSAM_[3];       // x, y, z axis; full: PIControllerPValueSAM; DSCS_setPIControllerPValueSAM
	int PIPValSAM_rbv_[3];   // x, y, z axis; full: PIControllerPValueSAM; DSCS_getPIControllerPValueSAM
	
	int PILimSAM_;           // single value; full: PIControllerLimitSAM; DSCS_setPIControllerLimitSAM
	int PILimSAM_rbv_;       // single value; full: PIControllerLimitSAM; DSCS_getPIControllerLimitSAM
	
	int PITargPos_[3];       // x, y, z axis; full: PIControllerTargetPosition; DSCS_setPIControllerTargetPosition
	int PITargPos_rbv_[3];   // x, y, z axis; full: PIControllerTargetPosition; DSCS_getPIControllerTargetPosition
	
	int PITargMode_;         // single value; full: PIControllerTargetMode; DSCS_setPIControllerTargetMode
	int PITargMode_rbv_;     // single value; full: PIControllerTargetMode; DSCS_getPIControllerTargetMode
	
	int PINFOOut_rbv_[3];    // x, y, z axis; full: PIControllerNFOOutput; DSCS_getPIControllerNFOOutput
	int PISAMOut_rbv_[3];    // x, y, z axis; full: PIControllerSAMOutput; DSCS_getPIControllerSAMOutput
	
	int NFOADCLimMin_;       // single value; full: NFOADCLimitsMin; DSCS_setNFOADCLimits
	int NFOADCLimMax_;       // single value; full: NFOADCLimitsMax; DSCS_setNFOADCLimits
	int NFOADCLimMin_rbv_;   // single value; full: NFOADCLimitsMin; DSCS_getNFOADCLimits
	int NFOADCLimMax_rbv_;   // single value; full: NFOADCLimitsMax; DSCS_getNFOADCLimits
	
	int NFOSlewLim_;         // single value; full: NFOSlewRateLimit; DSCS_setNFOSlewRateLimit
	int NFOSlewLim_rbv_;     // single value; full: NFOSlewRateLimit; DSCS_getNFOSlewRateLimit
	
	int SAMADCLimMin_;       // single value; full: SAMADCLimitsMin; DSCS_setSAMADCLimits
	int SAMADCLimMax_;       // single value; full: SAMADCLimitsMax; DSCS_setSAMADCLimits
	int SAMADCLimMin_rbv_;   // single value; full: SAMADCLimitsMin; DSCS_getSAMADCLimits
	int SAMADCLimMax_rbv_;   // single value; full: SAMADCLimitsMax; DSCS_getSAMADCLimits
	
	int SAMSlewLim_;         // single value; full: SAMSlewRateLimit; DSCS_setSAMSlewRateLimit
	int SAMSlewLim_rbv_;     // single value; full: SAMSlewRateLimit; DSCS_getSAMSlewRateLimit
	
	int LimState_rbv_;       // single value; full: LimiterState; DSCS_getLimiterState
	
	int InpTransMat_[45]; // matrix 3x15; full: InputTransformationMatrix; DSCS_setInputTransformationMatrix
	int InpTransRes_rbv_[3]; // x, y, z axis; full: InputTransformationResult; DSCS_getInputTransformationResult
	int InpTransAvg_rbv_;    // single value; full: InputTransformationAverage; DSCS_getInputTransformationAverage
	int InpTransState_rbv_;  // single value; full: InputTransformationState; DSCS_getInputTransformationState
	
	int OutTransMat_[42];  // matrix 6x7; full: OutputTransformationMatrix; DSCS_setOutputTransformationMatrix
	int OutTransNFORes_rbv_[3]; // x, y, z axis; full: OutputTransformationNFOResult; DSCS_getOutputTransformationResult
	int OutTransSAMRes_rbv_[3]; // x, y, z axis; full: OutputTransformationSAMResult; DSCS_getOutputTransformationResult
	
	int TrajStartX_;         // single value; full: TrajectoryLineStartX; DSCS_setTrajectoryLineStartX
	int TrajStartX_rbv_;     // single value; full: TrajectoryLineStartX; DSCS_getTrajectoryLineStartX
	
	int TrajEndX_;           // single value; full: TrajectoryLineEndX; DSCS_setTrajectoryLineEndX
	int TrajEndX_rbv_;       // single value; full: TrajectoryLineEndX; DSCS_getTrajectoryLineEndX
	
	int TrajSpeedX_;         // single value; full: TrajectoryLineSpeedX; DSCS_setTrajectoryLineSpeedX
	int TrajSpeedX_rbv_;     // single value; full: TrajectoryLineSpeedX; DSCS_getTrajectoryLineSpeedX
	
	int TrajStartY_;         // single value; full: TrajectoryLineStartY; DSCS_setTrajectoryLineStartY
	int TrajStartY_rbv_;     // single value; full: TrajectoryLineStartY; DSCS_getTrajectoryLineStartY
	
	int TrajDistY_;          // single value; full: TrajectoryLineDistY; DSCS_setTrajectoryLineDistY
	int TrajDistY_rbv_;      // single value; full: TrajectoryLineDistY; DSCS_getTrajectoryLineDistY
	
	int TrajCountY_;         // single value; full: TrajectoryLineCountY; DSCS_setTrajectoryLineCountY
	int TrajCountY_rbv_;     // single value; full: TrajectoryLineCountY; DSCS_getTrajectoryLineCountY
	
	int TrajTurnTime_;       // single value; full: TrajectoryTurnTime; DSCS_setTrajectoryTurnTime
	int TrajTurnTime_rbv_;   // single value; full: TrajectoryTurnTime; DSCS_getTrajectoryTurnTime
	
	int TrajPosTime_;        // single value; full: TrajectoryPosTime; DSCS_setTrajectoryPosTime
	int TrajPosTime_rbv_;    // single value; full: TrajectoryPosTime; DSCS_getTrajectoryPosTime
	
	int TrajAntiHyst_;       // single value; full: TrajectoryAntiHyst; DSCS_setTrajectoryAntiHyst
	int TrajAntiHyst_rbv_;   // single value; full: TrajectoryAntiHyst; DSCS_getTrajectoryAntiHyst
	
	int TrajSettings_;       // single value; full: TrajectorySettings; DSCS_setTrajectorySettings
	int TrajSettings_rbv_;   // single value; full: TrajectorySettings; DSCS_getTrajectorySettings
	

    asynUser* pasynUserdscsAsyn_;

private:
	// OSA_PS
	asynStatus setOSA_PS(DSCS_Axis axis, epicsInt32 value);
	
	// BS_PS
	asynStatus setBS_PS(DSCS_Axis axis, epicsInt32 value);
	
	// AUX_DAC
	asynStatus setAUX_DAC(DSCS_AUX_ADC aux, epicsInt32 value);
	
	// NFO_PS
	asynStatus setNFO_PS(DSCS_Axis axis, epicsInt32 value);
	
	// SAM_PS
	asynStatus setSAM_PS(DSCS_Axis axis, epicsInt32 value);
	
	// SetpointModulationFrequency
	asynStatus setSetpointModulationFrequency(DSCS_Axis axis, epicsInt32 value);
	
	// SetpointModulationPhase
	asynStatus setSetpointModulationPhase(DSCS_Axis axis, epicsInt32 value);
	
	// SetpointModulationAmplitude
	asynStatus setSetpointModulationAmplitude(DSCS_Axis axis, epicsInt32 value);
	
	// ExternalADCShift
	asynStatus setExternalADCShift(epicsInt32 value);
	
	// PI Controller NFO
	asynStatus setPIControllerEnabledNFO(DSCS_Axis axis, epicsInt32 value);
	asynStatus setPIControllerIValueNFO(DSCS_Axis axis, epicsInt32 value);
	asynStatus setPIControllerPValueNFO(DSCS_Axis axis, epicsInt32 value);
	asynStatus setPIControllerLimitNFO(epicsInt32 value);
	asynStatus setPIControllerAverageNFO(epicsInt32 value);
	
	// PI Controller SAM
	asynStatus setPIControllerEnabledSAM(DSCS_Axis axis, epicsInt32 value);
	asynStatus setPIControllerIValueSAM(DSCS_Axis axis, epicsInt32 value);
	asynStatus setPIControllerPValueSAM(DSCS_Axis axis, epicsInt32 value);
	asynStatus setPIControllerLimitSAM(epicsInt32 value);
	
	// PI Controller Target
	asynStatus setPIControllerTargetPosition(DSCS_Axis axis, epicsInt32 value);
	asynStatus setPIControllerTargetMode(epicsInt32 value);
	
	// NFOADCLimits (min/max)
	asynStatus setNFOADCLimMin(epicsInt32 value);
	asynStatus setNFOADCLimMax(epicsInt32 value);
	
	// NFOSlewRateLimit
	asynStatus setNFOSlewRateLimit(epicsInt32 value);
	
	// SAMADCLimits (min/max)
	asynStatus setSAMADCLimMin(epicsInt32 value);
	asynStatus setSAMADCLimMax(epicsInt32 value);
	
	// SAMSlewRateLimit
	asynStatus setSAMSlewRateLimit(epicsInt32 value);
	
	// Trajectory line parameters
	asynStatus setTrajectoryLineStartX(epicsInt32 value);
	asynStatus setTrajectoryLineEndX(epicsInt32 value);
	asynStatus setTrajectoryLineSpeedX(epicsInt32 value);
	asynStatus setTrajectoryLineStartY(epicsInt32 value);
	asynStatus setTrajectoryLineDistY(epicsInt32 value);
	asynStatus setTrajectoryLineCountY(epicsInt32 value);
	asynStatus setTrajectoryTurnTime(epicsInt32 value);
	asynStatus setTrajectoryPosTime(epicsInt32 value);
	asynStatus setTrajectoryAntiHyst(epicsInt32 value);
	asynStatus setTrajectorySettings(epicsInt32 value);


	void report(FILE *fp, int details);

	double pollTime_;

	int deviceId = -2;
	unsigned int deviceNo = 0;

	void checkError(const char * context, int code);

  
};

// #define NUM_PARAMS ((int)(&LAST_QUDIS_PARAM - &FIRST_QUDIS_PARAM + 1))
#define NUM_PARAMS 9


