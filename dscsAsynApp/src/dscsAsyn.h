/*
 * 
 * 
 * 
 */

#include <asynPortDriver.h>

static const char *driverName = "dscsAsyn";

#define MAX_CONTROLLERS	1
#define DEFAULT_POLL_TIME 0.1

#define DEFAULT_CONTROLLER_TIMEOUT 2.0

/* These are the drvInfo strings that are used to identify the parameters.
 * They are used by asyn clients, including standard asyn device support */

#define get_NFO_SG_string "NFO_SG_RBV"	    /* asynFloat64 r/o */

/*
 * Class definition for the dscsAsyn class
 */
class dscsAsyn: public asynPortDriver {
public:
    dscsAsyn(const char *portName, const char *dscsAsynPortName);
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
    // "float" index values
    int nfo_sg_rbv_;

	#define FIRST_QUDIS_PARAM pos1Val_;
	#define LAST_QUDIS_PARAM pos1Val_;

    asynUser* pasynUserdscsAsyn_;

private:

	void report(FILE *fp, int details);

	double pollTime_;

	unsigned int deviceId = -1;
	unsigned int deviceNo = 0;

	void checkError(const char * context, int code);

  
};

// #define NUM_PARAMS ((int)(&LAST_QUDIS_PARAM - &FIRST_QUDIS_PARAM + 1))
#define NUM_PARAMS 1


