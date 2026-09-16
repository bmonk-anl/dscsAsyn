/*
 * 
 * 
 * 
 */



#include <asynPortDriver.h>
#include <vector>

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
    virtual asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value);

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
				
				 
				 
	int LissFreq_[3];
	int LissFreq_rbv_[3];

	int LissPhase_[3];
	int LissPhase_rbv_[3];

	int LissAmp_[3];
	int LissAmp_rbv_[3];

	int LissOff_[3];
	int LissOff_rbv_[3];
				
	int ResetLissPhase_;     // for all 3 axes
				 
				 
	
	
	int ExtADCShift_;        // single value; full: ExternalADCShift; DSCS_setExternalADCShift
	int ExtADCShift_rbv_;    // single value; full: ExternalADCShift; DSCS_getExternalADCShift
	
	int PIEnNFO_[3];         // x, y, z axis; full: PIControllerEnabledNFO; DSCS_setPIControllerEnabledNFO
	int PIEnNFO_rbv_[3];     // x, y, z axis; full: PIControllerEnabledNFO; DSCS_getPIControllerEnabledNFO
				 


				 
	
	// int PIIValNFO_[3];       // x, y, z axis; full: PIControllerIValueNFO; DSCS_setPIControllerIValueNFO
	// int PIIValNFO_rbv_[3];   // x, y, z axis; full: PIControllerIValueNFO; DSCS_getPIControllerIValueNFO
	// 
	// int PIPValNFO_[3];       // x, y, z axis; full: PIControllerPValueNFO; DSCS_setPIControllerPValueNFO
	// int PIPValNFO_rbv_[3];   // x, y, z axis; full: PIControllerPValueNFO; DSCS_getPIControllerPValueNFO
	// 
	// int PILimNFO_;           // single value; full: PIControllerLimitNFO; DSCS_setPIControllerLimitNFO
	// int PILimNFO_rbv_;       // single value; full: PIControllerLimitNFO; DSCS_getPIControllerLimitNFO
	// 
	// int PIAvgNFO_;           // single value; full: PIControllerAverageNFO; DSCS_setPIControllerAverageNFO
	// int PIAvgNFO_rbv_;       // single value; full: PIControllerAverageNFO; DSCS_getPIControllerAverageNFO
	// 
	// int PIEnSAM_[3];         // x, y, z axis; full: PIControllerEnabledSAM; DSCS_setPIControllerEnabledSAM
	// int PIEnSAM_rbv_[3];     // x, y, z axis; full: PIControllerEnabledSAM; DSCS_getPIControllerEnabledSAM
	// 
	// int PIIValSAM_[3];       // x, y, z axis; full: PIControllerIValueSAM; DSCS_setPIControllerIValueSAM
	// int PIIValSAM_rbv_[3];   // x, y, z axis; full: PIControllerIValueSAM; DSCS_getPIControllerIValueSAM
	// 
	// int PIPValSAM_[3];       // x, y, z axis; full: PIControllerPValueSAM; DSCS_setPIControllerPValueSAM
	// int PIPValSAM_rbv_[3];   // x, y, z axis; full: PIControllerPValueSAM; DSCS_getPIControllerPValueSAM
	// 
	// int PILimSAM_;           // single value; full: PIControllerLimitSAM; DSCS_setPIControllerLimitSAM
	// int PILimSAM_rbv_;       // single value; full: PIControllerLimitSAM; DSCS_getPIControllerLimitSAM
	// 
	// int PITargPos_[3];       // x, y, z axis; full: PIControllerTargetPosition; DSCS_setPIControllerTargetPosition
	// int PITargPos_rbv_[3];   // x, y, z axis; full: PIControllerTargetPosition; DSCS_getPIControllerTargetPosition
	// 
	// int PITargMode_;         // single value; full: PIControllerTargetMode; DSCS_setPIControllerTargetMode
	// int PITargMode_rbv_;     // single value; full: PIControllerTargetMode; DSCS_getPIControllerTargetMode
	// 
	// int PINFOOut_rbv_[3];    // x, y, z axis; full: PIControllerNFOOutput; DSCS_getPIControllerNFOOutput
	// int PISAMOut_rbv_[3];    // x, y, z axis; full: PIControllerSAMOutput; DSCS_getPIControllerSAMOutput
	// 
	// int NFOADCLimMin_;       // single value; full: NFOADCLimitsMin; DSCS_setNFOADCLimits
	// int NFOADCLimMax_;       // single value; full: NFOADCLimitsMax; DSCS_setNFOADCLimits
	// int NFOADCLimMin_rbv_;   // single value; full: NFOADCLimitsMin; DSCS_getNFOADCLimits
	// int NFOADCLimMax_rbv_;   // single value; full: NFOADCLimitsMax; DSCS_getNFOADCLimits
	// 
	// int NFOSlewLim_;         // single value; full: NFOSlewRateLimit; DSCS_setNFOSlewRateLimit
	// int NFOSlewLim_rbv_;     // single value; full: NFOSlewRateLimit; DSCS_getNFOSlewRateLimit
	// 
	// int SAMADCLimMin_;       // single value; full: SAMADCLimitsMin; DSCS_setSAMADCLimits
	// int SAMADCLimMax_;       // single value; full: SAMADCLimitsMax; DSCS_setSAMADCLimits
	// int SAMADCLimMin_rbv_;   // single value; full: SAMADCLimitsMin; DSCS_getSAMADCLimits
	// int SAMADCLimMax_rbv_;   // single value; full: SAMADCLimitsMax; DSCS_getSAMADCLimits
	// 
	// int SAMSlewLim_;         // single value; full: SAMSlewRateLimit; DSCS_setSAMSlewRateLimit
	// int SAMSlewLim_rbv_;     // single value; full: SAMSlewRateLimit; DSCS_getSAMSlewRateLimit
	// 
	// int LimState_rbv_;       // single value; full: LimiterState; DSCS_getLimiterState
	







	// int InpTransMatRow; // internal values with current values of row and col
	// int InpTransMatCol;
	// int InpTransMatSetRow_;
	// int InpTransMatSetCol_;
	// int InpTransMatCoeff_;
	
	int InpTransMatCoeff_[3][15];
	
	int InpTransMatSave_; // DSCS_saveInputTransformationMatrix
	int InpTransRes_rbv_[3]; // x, y, z axis; full: InputTransformationResult; DSCS_getInputTransformationResult
	int InpTransAvg_rbv_;    // single value; full: InputTransformationAverage; DSCS_getInputTransformationAverage
	int InpTransState_rbv_;  // single value; full: InputTransformationState; DSCS_getInputTransformationState
	
	// int OutTransMatRow; // internal values with current values of row and col
	// int OutTransMatCol;
	// int OutTransMatSetRow_;
	// int OutTransMatSetCol_;
	// int OutTransMatCoeff_;
	// int OutTransMat_[42];  // matrix 6x7; full: OutputTransformationMatrix; DSCS_setOutputTransformationMatrix
	
	int OutTransMatCoeff_[6][7];
	int OutTransMatSave_; // DSCS_saveOutputTransformationMatrix
	int OutTransNFORes_rbv_[3]; // x, y, z axis; full: OutputTransformationNFOResult; DSCS_getOutputTransformationResult
	int OutTransSAMRes_rbv_[3]; // x, y, z axis; full: OutputTransformationSAMResult; DSCS_getOutputTransformationResult
	
	int ScanStartX_;         
	int ScanStartX_rbv_;     
	
	int ScanEndX_;           
	int ScanEndX_rbv_;       
	
	int ScanSpeedX_;         
	int ScanSpeedX_rbv_;     
	
	int ScanStartY_;         
	int ScanStartY_rbv_;     
	
	int ScanDistY_;          
	int ScanDistY_rbv_;      
	
	int ScanCountY_;         
	int ScanCountY_rbv_;     
	
	int ScanTurnTime_;       
	int ScanTurnTime_rbv_;   
	
	int ScanPosTime_;        
	int ScanPosTime_rbv_;    
	
	
	int ScanSettings_;       
	int ScanSettings_rbv_;   

	int StartScan_;       
	int StopScan_;       

	int ShutterState_rbv_;

	int ShutterActLow_rbv_[2];
	int ShutterActLow_[2];
	int ShutterActHigh_rbv_[2];
	int ShutterActHigh_[2];

	int ShutterHyst_rbv_;
	int ShutterHyst_;

	int AreaDetectorCount_rbv_;

	int XRayIntensity_rbv_;

	int XRFDeadTimeStatus_rbv_;

	int PiezoModelFit_rbv_[5][2];
	int PiezoModelFit_[5][2];

	int TrajMode_rbv_;
	int TrajMode_;

	// x, y, z
	int TrajDirTarget_rbv_[3];
	int TrajDirTarget_[3];

	int TrajState_rbv_;

	int CtrlMeasVal_rbv_[3];

	int CtrlTrajVal_rbv_[3];

	int CtrlSettings_rbv_;
	int CtrlSettings_;

	int PixelRateTriggerFactor_;
	int PixelRateTriggerFactor_rbv_;
	int AreaDetectorTriggerDivisor_;
	int AreaDetectorTriggerDivisor_rbv_;
	int ZygoReceptionActive_rbv_;
	int ZygoReceptionRateError_rbv_;
	int ZygoReceptionError_rbv_;
	int ClearZygoReceptionError_;
	int PixelTriggerOutputState_;
	int PixelTriggerOutputState_rbv_;
	int ResetZygoValues_;
	int ResetZygoValueOffsets_;
	

    asynUser* pasynUserdscsAsyn_;

private:
	// OSA_PS
	asynStatus setOSA_PS(DSCS_Axis axis, epicsFloat64 volts);
	
	// BS_PS
	asynStatus setBS_PS(DSCS_Axis axis, epicsFloat64 volts);
	
	// AUX_DAC
	asynStatus setAUX_DAC(DSCS_AUX_ADC aux, epicsFloat64 volts);
	
	// NFO_PS
	asynStatus setNFO_PS(DSCS_Axis axis, epicsFloat64 volts);
	
	// SAM_PS
	asynStatus setSAM_PS(DSCS_Axis axis, epicsFloat64 volts);

	// Lissajous
	asynStatus setLissFreq(DSCS_Axis axis, epicsFloat64 khz);
	asynStatus setLissPhase(DSCS_Axis axis, epicsFloat64 deg);
	asynStatus setLissAmp(DSCS_Axis axis, epicsFloat64 nm);
	asynStatus setLissOff(DSCS_Axis axis, epicsFloat64 nm);

	
	// // SetpointModulationFrequency
	// asynStatus setSetpointModulationFrequency(DSCS_Axis axis, epicsInt32 value);
	// 
	// // SetpointModulationPhase
	// asynStatus setSetpointModulationPhase(DSCS_Axis axis, epicsInt32 value);
	// 
	// // SetpointModulationAmplitude
	// asynStatus setSetpointModulationAmplitude(DSCS_Axis axis, epicsInt32 value);
	
	// ExternalADCShift
	asynStatus setExternalADCShift(epicsInt32 value);
	
	// PI Controller NFO
	asynStatus setPIControllerEnabledNFO(DSCS_Axis axis, epicsInt32 value);

	// // asynStatus setPIControllerIValueNFO(DSCS_Axis axis, epicsInt32 value);
	// asynStatus setPIControllerPValueNFO(DSCS_Axis axis, epicsInt32 value);
	// asynStatus setPIControllerLimitNFO(epicsInt32 value);
	// // asynStatus setPIControllerAverageNFO(epicsInt32 value);
	// 
	// // PI Controller SAM
	// asynStatus setPIControllerEnabledSAM(DSCS_Axis axis, epicsInt32 value);
	// asynStatus setPIControllerIValueSAM(DSCS_Axis axis, epicsInt32 value);
	// asynStatus setPIControllerPValueSAM(DSCS_Axis axis, epicsInt32 value);
	// asynStatus setPIControllerLimitSAM(epicsInt32 value);
	// 
	// // // PI Controller Target
	// // asynStatus setPIControllerTargetPosition(DSCS_Axis axis, epicsInt32 value);
	// // asynStatus setPIControllerTargetMode(epicsInt32 value);
	// 
	// // NFOADCLimits (min/max)
	// asynStatus setNFOADCLimMin(epicsInt32 value);
	// asynStatus setNFOADCLimMax(epicsInt32 value);
	// 
	// // NFOSlewRateLimit
	// asynStatus setNFOSlewRateLimit(epicsInt32 value);
	// 
	// // SAMADCLimits (min/max)
	// asynStatus setSAMADCLimMin(epicsInt32 value);
	// asynStatus setSAMADCLimMax(epicsInt32 value);
	// 
	// // SAMSlewRateLimit
	// asynStatus setSAMSlewRateLimit(epicsInt32 value);

	// asynStatus setInputTransformationRow(epicsInt32 value);
	// asynStatus setInputTransformationCol(epicsInt32 value);
	asynStatus setInputTransformationCoefficient(int row, int col, epicsFloat64 value);
	asynStatus saveInputTransformationMatrix(void);

	// asynStatus setOutputTransformationRow(epicsInt32 value);
	// asynStatus setOutputTransformationCol(epicsInt32 value);
	asynStatus setOutputTransformationCoefficient(int row, int col, epicsFloat64 value);
	asynStatus saveOutputTransformationMatrix(void);

	
	// Scan line parameters
	asynStatus setScanLineStartX(epicsFloat64 nm);
	asynStatus setScanLineEndX(epicsFloat64 nm);
	asynStatus setScanLineSpeedX(epicsInt32 value);
	asynStatus setScanLineStartY(epicsFloat64 nm);
	asynStatus setScanLineDistY(epicsFloat64 nm);
	asynStatus setScanLineCountY(epicsInt32 value);
	asynStatus setScanTurnTime(epicsInt32 value);
	asynStatus setScanPosTime(epicsInt32 value);
	asynStatus setScanSettings(epicsInt32 value);
	asynStatus startScan(void);
	asynStatus stopScan(void);

	asynStatus setShutterActivationLow(DSCS_Axis axis, epicsFloat64 lower_nm);
	asynStatus setShutterActivationHigh(DSCS_Axis axis, epicsFloat64 upper_nm);
	asynStatus setShutterHysteresis(epicsFloat64 nm);

	asynStatus setPiezoFitParameter(DSCS_PiezoModelParameters type,
                                         DSCS_Direction dir,
                                         epicsInt32 value);

	
	asynStatus setTrajectoryMode(epicsInt32 value);
	asynStatus setTrajectoryDirectTarget(DSCS_Axis axis, epicsFloat64 nm);
	
	asynStatus setControllerSettings(epicsInt32 value);

	asynStatus setPixelRateTriggerFactor(epicsInt32 value);
	asynStatus setAreaDetectorTriggerDivisor(epicsInt32 value);
	asynStatus clearZygoDataReceptionError(void);
	asynStatus setPixelTriggerOutputState(epicsInt32 value);
	asynStatus resetZygoValues(void);
	asynStatus resetZygoValueOffsets(void);
	

	void report(FILE *fp, int details);

	double pollTime_;

	int deviceId = -2;
	unsigned int deviceNo = 0;
	bool connected_ = false;

	struct PollUpdate {
		const char *context;
		int code;
		int param;
		bool isDouble;
		epicsInt32 integerValue;
		epicsFloat64 doubleValue;
	};

	void checkError(const char * context, int code);
	void setAllParamStatus(asynStatus status);
	void queueIntegerUpdate(std::vector<PollUpdate>& updates, const char *context,
	                        int code, int param, epicsInt32 value);
	void queueDoubleUpdate(std::vector<PollUpdate>& updates, const char *context,
	                       int code, int param, epicsFloat64 value);
	void applyPollUpdates(const std::vector<PollUpdate>& updates);

  
};

// #define NUM_PARAMS ((int)(&LAST_QUDIS_PARAM - &FIRST_QUDIS_PARAM + 1))
#define NUM_PARAMS 9


