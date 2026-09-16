/*******************************************************************************
 *
 *  Project:  DSCS Controller Library
 *
 *  Filename: dscs_defines.h
 *
 *  Purpose:  Constants and enums used with dscs.h
 *
 *  Author:   NHands GmbH & Co KG
 */
/******************************************************************************/
/*!
 * @file dscs.h
 *
 * @brief Constants and enums used with dscs.h
 *
 */
/******************************************************************************/

#ifndef DSCS_DEFINES_H_
#define DSCS_DEFINES_H_

#include "dscs_export.h"


#ifdef __cplusplus
#define EXTC extern "C" /**< Storage class for C++                */

#ifdef DSCS_DEPRECATED_MSG
#undef DSCS_DEPRECATED_MSG
#endif  // DSCS_DEPRECATED_MSG

#define DSCS_DEPRECATED_MSG(msg) [[deprecated(msg)]]

#else               // __cplusplus
#define EXTC extern /**< Storage class for C                  */

#ifdef DSCS_DEPRECATED_MSG
#undef DSCS_DEPRECATED_MSG
#endif  // DSCS_DEPRECATED_MSG:w

#define DSCS_DEPRECATED_MSG(msg) DSCS_DEPRECATED

#endif  // __cplusplus


#ifdef unix
#define WINCC           /**< Not required for Unix                */
#else                   // unix
#define WINCC __stdcall /**< Calling convention for Windows       */
#endif                  // unix

#ifndef DSCS_API
#define DSCS_API EXTC DSCS_EXPORT /**< For use with C++        */
#endif                            // DSCS_API


#ifdef _MSC_VER
typedef __int32 Int32; /*!< 32-Bit-Integer fuer MSVC      */
#else
#include <inttypes.h>
typedef int32_t Int32; /*!< 32-Bit-Integer fuer GCC       */
#endif

typedef int bln32; /**< Boolean; to avoid troubles with
                        incomplete C99 implementations         */


/*!
 * @name Return values of the functions
 *
 * @details
 *  All functions of this lib - as far as they can fail - return
 * one of these constants for success control.
 *
 *  @{
 */
#define DSCS_Ok           0  /**< No error                               */
#define DSCS_Error        -1 /**< Unspecified error                      */
#define DSCS_Timeout      1  /**< Communication timeout                  */
#define DSCS_NotConnected 2  /**< No active connection to device         */
#define DSCS_DriverError  3  /**< Error in communication with driver     */
#define DSCS_DeviceLocked 7  /**< Device is already in use by other      */
#define DSCS_Unknown      8  /**< Unknown error                          */
#define DSCS_NoDevice     9  /**< Invalid device number in function call */
#define DSCS_ParamOutOfRg 11 /**< A parameter exceeds the allowed range  */
/** @} */


/*!
 * @brief Data callback function
 *
 * @details
 *  A function of this type can be registered as callback function for new
 *  data values with @ref QDS_setDataCallback.
 *
 *  @param  chanenl      Data channel associated to the received data
 *  @param  length       Number of bytes in the data array
 *  @param  index        Sequence number of the first position of the packet
 *  @param  data         Buffer with the data. This buffer is deleted by the
 *                       library after the callback function returns.
 */
typedef void (*DSCS_DataCallback)(int          channel,
                                  int          length,
                                  int          index,
                                  const Int32 *data);


#define DSCS_TUPLE_SIZE 32


/*!
 * @brief Interface types
 */
typedef enum
{
  IfNone = 0x00, /**< Device invalid / not connected         */
  IfUsb3 = 0x10, /**< Device connected via USB               */
  IfAll  = 0x10  /**< All physical interfaces                */
} DSCS_InterfaceType;


/*!
 * @brief Axis for analog outputs
 */
typedef enum
{
  DSCS_AxisX, /**< X-Axis                                */
  DSCS_AxisY, /**< Y-Axis                                */
  DSCS_AxisZ, /**< Z-Axis                                */
} DSCS_Axis;


/*!
 * @brief Index types for XZ/ZX read back.
 */
typedef enum
{
  DSCS_XZ, /**< XZ read back                           */
  DSCS_ZX, /**< ZX read back                           */
} DSCS_XZ_ZX;


/*!
 * @brief Index of display ports.
 */
typedef enum
{
  DisplayPort_1, /**< Display port 1                        */
  DisplayPort_2, /**< Display port 2                        */
} DSCS_DisplayPort;


/*!
 * @brief Index types for AUX_ADC read back.
 */
typedef enum
{
  DSCS_AUX_0,                /**< AUX 0 index                           */
  DSCS_AUX_1,                /**< AUX 1 index                           */
  DSCS_AUX_2,                /**< AUX 2 index                           */
  DSCS_AUX_3,                /**< AUX 3 index                           */
  DSCS_AUX_MIN = DSCS_AUX_0, /**< Minimum AUX index                     */
  DSCS_AUX_MAX = DSCS_AUX_3, /**< Maximum AUX index                     */
} DSCS_AUX_ADC;


typedef enum
{
  UnknownConnection,
  ControllerConnection, /**< Controller connection for configurations  */
  DataConnection,       /**< Secondary connection for data acquisition */
} DSCS_ConnectionType;


typedef enum
{
  Direct,      /**< Use the P/I parameters of the controller */
  Setpoint,    /**< Use the setpoint parameters              */
  ExternalAux, /**< Use the external AUX ADC values.         */
  Trajectory,  /**< Use the values of trajectory generator   */
} DSCS_TargetMode;


typedef enum
{
  OutputNull = 0x01, /**< Output set to 0 Volts */
} DSCS_LimiterState;


typedef enum
{
  TransformationError = 0x01, /**< Transformation error */
  ZygoInputActive     = 0x02, /**< Zygo input active   */
  AC1InputActive      = 0x04, /**< AC1 input active   */
} DSCS_InputTransformationState;


typedef enum
{
  None       = 0x00, /**< No special mode enabled */
  FWBW       = 0x01, /**< FWBW enabled */
  AntiHyst   = 0x02, /**< Anti-hysteresis enabled */
  Continuous = 0x04, /**< Continuous mode enabled */
} DSCS_ScanSettings;


typedef enum
{
  ShutterState_Open = 0x01, /**< Shutter is open */
  ShutterState_High = 0x02, /**< Area detector is high */
  ShutterState_XRay = 0x04, /**< X-Ray intensity is high */
} DSCS_ShutterState;


typedef enum
{
  Direction_FW = 0,
  Direction_BW = 1
} DSCS_Direction;

typedef enum
{
  PiezoModel_X0 = 0,
  PiezoModel_A1 = 1,
  PiezoModel_A2 = 2,
  PiezoModel_A3 = 3,
  PiezoModel_A4 = 4,
} DSCS_PiezoModelParameters;


typedef enum
{
  TrajectoryMode_DirectTarget = 0, /**< Direct target mode */
  TrajectoryMode_Lissajous    = 1, /**< Lissajous scan */
  TrajectoryMode_ExtAux       = 2, /**< External AUX ADC */
  TrajectoryMode_Scan         = 3, /**< Scan mode (FwBw or Snake) */
} DSCS_TrajectoryMode;


typedef enum
{
  TrajectoryState_ScanRunning = 0x01, /**< Scan is running */
} DSCS_TrajectoryState;


typedef enum
{
  ControllerSettings_NoiseCancellation =
      0x01,                                 /**< Noise cancellation enabled */
  ControllerSettings_InputDecimal   = 0x02, /**< Input decimal enabled */
  ControllerSettings_MeasureLowPass = 0x04, /**< Measure low pass enabled */
  ControllerSettings_PhaseShift     = 0x08, /**< Phase shift enabled */
  ControllerSettings_TrajectoryLowPass =
      0x10,                             /**< Trajectory low pass enabled */
  ControllerSettings_PiezoModel = 0x20, /**< Piezo model parameters enabled */
  ControllerSettings_NFO2SAM_PILowPass =
      0x40,                              /**< NFO2SAM PI low pass enabled */
  ControllerSettings_NFOLowPass = 0x80,  /**< NFO low pass enabled */
  ControllerSettings_SAMLowPass = 0x100, /**< SAM low pass enabled */
} DSCS_ControllerSettings;


typedef enum
{
  CommState_Error       = 0x01,  /**< Communication error        */
  CommState_ConnLost    = 0x02,  /**< Connection lost            */
  CommState_RoleUnknown = 0x04,  /**< Role of the connection is unknown */
  CommState_RoleSwapped = 0x08,  /**< Role of the connection swapped */
  CommState_RoleSame    = 0x10,  /**< Both sides have the same role */
  CommState_RemoteError = 0x20,  /**< Remote error               */
  CommState_RcvRateLow  = 0x40,  /**< Receive rate too low      (<100 kHz) */
  CommState_InvalidDACs = 0x80,  /**< Invalid DAC values        */
  CommState_TxRateUnexp = 0x100, /**< Unexpected transmit rate  (+-20 kHz)*/
} DSCS_CommState;



#endif  // DSCS_DEFINES_H_
