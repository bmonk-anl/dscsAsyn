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
#include <unistd.h>


static void
dataCallback(int channel, int length, int index, const int *data)
{
  printf("Data callback: Channel=%d Length=%d Index=%d\n", channel, length,
         index);
  int package = 0;
  if (length % DSCS_TUPLE_SIZE)
  {
    fprintf(stderr, " (not a multiple of %d)\n", DSCS_TUPLE_SIZE);
    return;
  }
  printf("Package[%02d]: ", package);
  for (int i = 0; i < length; ++i)
  {
    printf(" 0x%08x", data[i]);
    if (i % DSCS_TUPLE_SIZE == (DSCS_TUPLE_SIZE - 1) && i < (length - 1))
    {
      printf("\nPackage[%02d]: ", ++package);
    }
  }
  printf("\n");
}


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
selectDataDevice(unsigned int devCount)
{
  unsigned int dataDevNo   = 0;
  int          dataDevices = 0;

  for (unsigned int devNo = 0; devNo < devCount; ++devNo)
  {
    int                 id = 0;
    char                addr[20], serialNo[20];
    int                 rc = DSCS_getDeviceInfo(devNo, &id, serialNo, addr);
    DSCS_ConnectionType connType = DSCS_getConnectionType(devNo);
    checkError("DSCS_getDeviceInfo", rc);
    if (connType == DataConnection)
    {
      ++dataDevices;
      dataDevices = devNo;
      printf("Device found: No=%d Id=%d SN=%s Addr=%s Connection Type=%d\n",
             devNo, id, serialNo, addr, connType);
    }
  }

  if (dataDevices > 1)
  {
    printf("Select device: ");
    dataDevNo = getchar();
    dataDevNo = dataDevNo >= devCount ? 0 : dataDevNo;
    printf("\n");
  }

  return dataDevNo;
}

static int
selectControllerDevice(unsigned int devCount)
{
  unsigned int ctrlDevNo   = 0;
  int          ctrlDevices = 0;

  for (unsigned int devNo = 0; devNo < devCount; ++devNo)
  {
    int                 id = 0;
    char                addr[20], serialNo[20];
    int                 rc = DSCS_getDeviceInfo(devNo, &id, serialNo, addr);
    DSCS_ConnectionType connType = DSCS_getConnectionType(devNo);
    checkError("DSCS_getDeviceInfo", rc);
    if (connType == ControllerConnection)
    {
      ++ctrlDevices;
      ctrlDevNo = devNo;
      printf("Device found: No=%d Id=%d SN=%s Addr=%s Connection Type=%d\n",
             devNo, id, serialNo, addr, connType);
    }
  }

  if (ctrlDevices > 1)
  {
    printf("Select device: ");
    ctrlDevNo = getchar();
    ctrlDevNo = ctrlDevNo >= devCount ? 0 : ctrlDevNo;
    printf("\n");
  }

  return ctrlDevNo;
}


int
main(int argc, char **argv)
{
  unsigned int ctrlDevNo = 0, dataDevNo = 0, devCount;
  int          rc = DSCS_Ok;
  int          value;


  printf("DSCS example program\n");
  printf("Using %s\n", DSCS_getVersion());

  rc = DSCS_discover(IfAll, &devCount);
  if (devCount <= 0)
  {
    printf("No devices found\n");
    exit(0);
  }
  printf("Found %d devices\n", devCount);

  ctrlDevNo = selectControllerDevice(devCount);
  dataDevNo = selectDataDevice(devCount);
  rc        = DSCS_connect(dataDevNo);
  rc        = DSCS_connect(ctrlDevNo);
  checkError("DSCS_connect", rc);

  DSCS_setDataOutputEnabled(ctrlDevNo, 1);
  DSCS_setDataCallback(dataDevNo, &dataCallback);

  // sleep a very short time to receive some data
  usleep(10);


  // The used values are just to demonstrate the usage of the functions
  // and might not be suitable for your application
  rc = DSCS_setAUX_DAC(ctrlDevNo, DSCS_AUX_0, 1000);
  checkError("DSCS_setAUX_DAC", rc);

  rc = DSCS_getAUX_DAC(ctrlDevNo, DSCS_AUX_0, &value);
  checkError("DSCS_getAUX_DAC", rc);

  rc = DSCS_disconnect(dataDevNo);
  checkError("DSCS_disconnect", rc);
  rc = DSCS_disconnect(ctrlDevNo);
  checkError("DSCS_disconnect", rc);
  return 0;
}
