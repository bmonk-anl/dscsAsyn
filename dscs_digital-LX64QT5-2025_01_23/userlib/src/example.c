/******************************************************************************
 *
 *  Project:        DSCS Control Library
 *
 *  Filename:       example.c
 *
 *  Purpose:        Simple example
 *
 *  Author:         N-Hands GmbH & Co KG
 */
/*****************************************************************************/

#include "dscs.h"

#include <stdio.h>
#include <stdlib.h>

static const char *
getMessage(int code)
{
  switch (code)
  {
    case DSCS_Ok:
      return "";
    case DSCS_Error:
      return "Unspecified error";
    case DSCS_Timeout:
      return "Communication timeout";
    case DSCS_NotConnected:
      return "No active connection to device";
    case DSCS_DriverError:
      return "Error in comunication with driver";
    case DSCS_DeviceLocked:
      return "Device is already in use by other";
    case DSCS_Unknown:
      return "Unknown error";
    case DSCS_NoDevice:
      return "Invalid device number in function call";
    case DSCS_ParamOutOfRg:
      return "A parameter exceeds the allowed range";
    default:
      return "Unknown error code";
  }
}

static void
checkError(const char *context, int code)
{
  if (code != DSCS_Ok)
  {
    printf("Error calling %s: %s\n", context, getMessage(code));
    /* It's not clean to exit here, should first close connections */
    exit(code);
  }
}

static int
selectDevice()
{
  unsigned int devCount = 0, devNo = 0;
  int          rc = DSCS_discover(IfAll, &devCount);
  checkError("DSCS_discover", rc);
  if (devCount <= 0)
  {
    printf("No devices found\n");
    exit(0);
  }

  for (devNo = 0; devNo < devCount; devNo++)
  {
    int  id = 0;
    char addr[20], serialNo[20];
    rc = DSCS_getDeviceInfo(devNo, &id, serialNo, addr);
    checkError("DSCS_getDeviceInfo", rc);
    printf("Device found: No=%d Id=%d SN=%s Addr=%s\n", devNo, id, serialNo,
           addr);
  }

  devNo = 0;
  if (devCount > 1)
  {
    printf("Select device: ");
    devNo = getchar();
    devNo = devNo >= devCount ? 0 : devNo;
    printf("\n");
  }

  return devNo;
}


int
main(int argc, char **argv)
{
  unsigned int devNo = 0;
  int          rc    = DSCS_Ok;
  int          value;


  printf("DSCS example program\n");
  printf("Using %s\n", DSCS_getVersion());

  devNo = selectDevice();
  rc    = DSCS_connect(devNo);
  checkError("DSCS_connect", rc);

  // The used values are just to demonstrate the usage of the functions
  // and might not be suitable for your application
  rc = DSCS_setAUX_DAC(devNo, DSCS_AUX_0, 1000);
  checkError("DSCS_setAUX_DAC", rc);

  rc = DSCS_getAUX_DAC(devNo, DSCS_AUX_0, &value);
  checkError("DSCS_getAUX_DAC", rc);

  rc = DSCS_disconnect(devNo);
  checkError("DSCS_disconnect", rc);
  return 0;
}
