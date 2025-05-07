/*****************************************************************************
 *
 *  Project:        DSCS Digital - Control Library
 *
 *  Filename:       dscs_digital.h
 *
 *  Author:         NHands GmbH & Co KG
 */
/*****************************************************************************/
/** @mainpage Custom Programming Library for DSCS
 *
 *  @ref dscs.h "DSCS Control Library" is a library that allows
 *  custom programming for the DSCS device.
 *  It can manage multiple devices that are connected to the PC via USB.
 *
 *  Use \ref DSCS_discover to discover devices on USB.
 *  Inspect them with \ref DSCS_getDeviceInfo and connect to selected devices
 *  using \ref DSCS_connect. After connecting, all the other functions for
 *  configuration and position can be called.
 *
 *  Documentation for the functions can be found \ref dscs.h "here".
 */
/*****************************************************************************/
/** @file dscs.h
 *
 *  @brief Control and acquisition functions for DSCS
 *
 *  Defines functions for connecting and controlling the DSCS device.
 *  The functions are not thread safe!
 */
/******************************************************************/

#ifndef DSCS_H__
#define DSCS_H__

#include "dscs_defines.h"


/*!
 * @brief Reads the version of the DSCS library
 *
 * @param version[out] version string
 * @param len[out]     length of the version string
 */
DSCS_API const char *WINCC DSCS_getVersion();


/*!
 * @brief Discover devices
 *
 * @details The function searches for connected DSCS devices on USB and
 * initializes internal data structures per device. Devices that are in use by
 * another application or PC are not found. The function must be called before
 * connecting to a device and must not be called as long as any devices are
 * connected.
 *
 * The number of devices found is returned. In subsequent functions, devices
 * are identified by a sequence number that must be less than the number
 * returned.
 *
 * @param   ifaces    Interfaces where devices are to be searched
 * @param   devCount  Output: number of devices found
 *
 * @returns           Error code
 */
DSCS_API int WINCC DSCS_discover(const DSCS_InterfaceType ifaces,
                                 unsigned int            *devCount);


/*!
 * @brief Device information
 *
 * @details Returns available information on a device. The function can not be
 * called before @ref DSCS_discover but the devices don't have to be @ref
 * DSCS_connect "connected" . All Pointers to output parameters may be zero to
 * ignore the respective value.
 *
 * @param   devNo     Sequence number of the device
 * @param   id        Output: programmed hardware ID of the device
 * @param   serialNo  Output: The device's serial number. The string buffer
 * should be NULL or at least 16 bytes long.i
 * @param   address   Output: The device's interface address if applicable.
 * Returns the IP address in dotted-decimal notation or the string "USB",
 * respectively. The string buffer should be NULL or at least 16 bytes long.
 *
 * @returns           Error code
 */
DSCS_API int WINCC DSCS_getDeviceInfo(const unsigned int devNo,
                                      int               *id,
                                      char              *serialNo,
                                      char              *address);


/*!
 * @brief Checks connection type
 *
 * @details There are two ways to connect to a device (via usb):
 *  1. Directly to the device for control and configuration.
 *  2. Secondary connection to the device for data acquisition.
 * This function can be used to identify the connection type.
 */
DSCS_API DSCS_ConnectionType WINCC
DSCS_getConnectionType(const unsigned int devNo);


/*!
 * @brief Connect device
 *
 * @details Initializes and connects the selected device. This has to be done
 * before any access to control variables or measured data.
 *
 * @param  devNo      Sequence number of the device
 *
 * @returns           Error code
 */
DSCS_API int WINCC DSCS_connect(const unsigned int devNo);


/*!
 * @brief Disconnect device
 *
 * @details Closes the connection to the device.
 *
 * @param  devNo      Sequence number of the device
 *
 * @returns           Error code
 */
DSCS_API int WINCC DSCS_disconnect(const unsigned int devNo);


/*!
 * @brief Register a data callback function
 *
 * @details
 *  Register a data callback function for a device that will be called when new
 * data values are available. A callback function registered previously is
 * unregistered. Both connections (control and data) are available to register
 * a data callback function.
 *
 * @param devNo      Sequence number of the device
 * @param callback   Callback function for data values. Use NULL to unregister.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setDataCallback(const unsigned int devNo,
                                        DSCS_DataCallback  callback);


/*!
 * @brief En-/Disables the data output on seconday connection.
 *
 * @param devNo      Sequence number of the device
 * @param enable     Enable or disable the data output
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setDataOutputEnabled(const unsigned int devNo,
                                             const bln32        enable);


/************ Application function *************/

/*!
 * @brief Reads the values of analog output OSA_PS
 *
 * @details
 *  Values returned are in range of -10 V to +10 V in steps of 1 uV for the x
 * and y axis.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of OSA_PS out (@sa DSCS_Axis).
 * @param value[out] Output: The value of the analog output OSA_PS.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_getOSA_PS(const unsigned int devNo,
                                  const DSCS_Axis    axis,
                                  int               *value);


/*!
 * @brief Sets the values of analog output OSA_PS
 *
 * @details
 *  Values are in range of -10 V to +10 V in steps of 1 uV for the x and y
 * axis.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of OSA_PS out (@sa DSCS_Axis).
 * @param value      The value of the analog output OSA_PS.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setOSA_PS(const unsigned int devNo,
                                  const DSCS_Axis    axis,
                                  const int          value);


/*!
 * @brief Reads the values of analog output BS_PS
 *
 * @details
 *  Values returned are in range of -10 V to +10 V in steps of 1 uV for the x
 * and y axis.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of BS_PS out (@sa DSCS_Axis).
 * @param value[out] Output: The value of the analog output BS_PS.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_getBS_PS(const unsigned int devNo,
                                 const DSCS_Axis    axis,
                                 int               *value);


/*!
 * @brief Sets the values of analog output BS_PS
 *
 * @details
 *  Values are in range of -10 V to +10 V in steps of 1 uV for the x and y
 * axis.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of BS_PS out (@sa DSCS_Axis).
 * @param value      The value of the analog output BS_PS.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setBS_PS(const unsigned int devNo,
                                 const DSCS_Axis    axis,
                                 const int          value);


/*!
 * @brief Reads the values of analog output AUX_DAC
 *
 * @details
 *  Values returned are in range of -10 V to +10 V in steps of 1 uV.
 *
 * @param devNo      Sequence number of the device.
 * @param aux        Aux index [0-3] (@sa DSCS_AUX_ADC).
 * @param value[out] Output: The value of the analog output AUX_DAC.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_getAUX_DAC(const unsigned int devNo,
                                   const DSCS_AUX_ADC aux,
                                   int               *value);


/*!
 * @brief Sets the values of analog output AUX_DAC
 *
 * @details
 *  Values are in range of -10 V to +10 V in steps of 1 uV.
 *
 * @param devNo      Sequence number of the device.
 * @param aux        Aux index [0-3] (@sa DSCS_AUX_ADC).
 * @param value      The value of the analog output AUX_DAC.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setAUX_DAC(const unsigned int devNo,
                                   const DSCS_AUX_ADC aux,
                                   const int          value);


/*!
 * @brief Reads the values of analog output NFO_PS
 *
 * @details
 *  Values returned are in range of -10 V to +10 V in steps of 1 uV for the x,
 * y and z axis.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of NFO_PS out (@sa DSCS_Axis).
 * @param value[out] Output: The value of the analog output NFO_PS.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_getNFO_PS(const unsigned int devNo,
                                  const DSCS_Axis    axis,
                                  int               *value);


/*!
 * @brief Sets the values of analog output NFO_PS
 *
 * @details
 *  Values are in range of -10 V to +10 V in steps of 1 uV for the x, y and z
 * axis.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of NFO_PS out (@sa DSCS_Axis).
 * @param value      The value of the analog output NFO_PS.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setNFO_PS(const unsigned int devNo,
                                  const DSCS_Axis    axis,
                                  const int          value);


/*!
 * @brief Reads the values of analog output SAM_PS
 *
 * @details
 *  Values returned are in range of -10 V to +10 V in steps of 1 uV for the x,
 * y and z axis.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of SAM_PS out (@sa DSCS_Axis).
 * @param value[out] Output: The value of the analog output SAM_PS.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_getSAM_PS(const unsigned int devNo,
                                  const DSCS_Axis    axis,
                                  int               *value);


/*!
 * @brief Sets the values of analog output SAM_PS
 *
 * @details
 *  Values are in range of -10 V to +10 V in steps of 1 uV for the x, y and z
 * axis.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of SAM_PS out (@sa DSCS_Axis).
 * @param value      The value of the analog output SAM_PS.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setSAM_PS(const unsigned int devNo,
                                  const DSCS_Axis    axis,
                                  const int          value);


/*!
 * @brief Reads the values of analog input NFO_SG
 *
 * @details
 *  Values returned are in range of -10 V to +10 V in steps of 1 uV for the x,
 * y and z axis.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of NFO_SG in (@sa DSCS_Axis).
 * @param value[out] Output: The value of the analog input NFO_SG.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_getNFO_SG(const unsigned int devNo,
                                  const DSCS_Axis    axis,
                                  int               *value);


/*!
 * @brief Reads the values of analog input SAM_CP_D
 *
 * @details
 *  Values returned are in range of -10 V to +10 V in steps of 1 uV for the x,
 * y and z axis.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of SAM_CP_D in (@sa DSCS_Axis).
 * @param value[out] Output: The value of the analog input SAM_CP_D.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_getSAM_CP_D(const unsigned int devNo,
                                    const DSCS_Axis    axis,
                                    int               *value);


/*!
 * @brief Reads the values of analog input XZ_ZX
 *
 * @details
 *  Values returned are in range of -10 V to +10 V in steps of 1 uV.
 *
 * @param devNo      Sequence number of the device.
 * @param index      Index of the analog input XZ_ZX (@sa DSCS_XZ_ZX).
 * @param value[out] Output: The value of the analog input XZ_ZX.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_getXZ_ZX(const unsigned int devNo,
                                 const DSCS_XZ_ZX   index,
                                 int               *value);


/*!
 * @brief Reads the values of analog input AUX_ADC
 *
 * @details
 *  Values returned are in range of -10 V to +10 V in steps of 1 uV.
 *
 * @param devNo      Sequence number of the device.
 * @param aux        Aux index [0-2] (@sa DSCS_AUX_ADC).
 * @param value[out] Output: The value of the analog input AUX_ADC.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_getAUX_ADC(const unsigned int devNo,
                                   const DSCS_AUX_ADC aux,
                                   int               *value);


/*!
 * @brief Reads the values of analog input NFO
 *
 * @details
 *  Values returned are in range of -10 V to +10 V in steps of 1 uV for the x,
 * y and z axis.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of NFO in (@sa DSCS_Axis).
 * @param value[out] Output: The value of the analog input NFO.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_getNFO(const unsigned int devNo,
                               const DSCS_Axis    axis,
                               int               *value);


/*!
 * @brief Reads the values of analog input SAM
 *
 * @details
 *  Values returned are in range of -10 V to +10 V in steps of 1 uV for the x,
 * y and z axis.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of SAM in (@sa DSCS_Axis).
 * @param value[out] Output: The value of the analog input SAM.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_getSAM(const unsigned int devNo,
                               const DSCS_Axis    axis,
                               int               *value);


/*!
 * @brief Reads the frequencies of the setpoint modulation
 *
 * @details
 *  Reads the frequencies of the sinus setpoint modulation for the x, y and z
 * axis. The values are in 1/2^32 MHz as unsigned 32 bit integer.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of the setpoint modulation (@sa DSCS_Axis).
 * @param value[out] Output: The frequency of the setpoint modulation.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_getSetpointModulationFrequency(const unsigned int devNo,
                                                       const DSCS_Axis    axis,
                                                       int *value);


/*!
 * @brief Sets the frequencies of the setpoint modulation
 *
 * @details
 *  Sets the frequencies of the sinus setpoint modulation for the x, y and z
 * axis. The values are in 1/2^32 MHz as unsigned 32 bit integer.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of the setpoint modulation (@sa DSCS_Axis).
 * @param value      The frequency of the setpoint modulation.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setSetpointModulationFrequency(const unsigned int devNo,
                                                       const DSCS_Axis    axis,
                                                       const int value);


/*!
 * @brief Reads the phase of the setpoint modulation
 *
 * @details
 *  Reads the phase of the sinus setpoint modulation for the x, y and z axis.
 * The values are in 360/2^32 deg as unsigned 32 bit integer.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of the setpoint modulation (@sa DSCS_Axis).
 * @param value[out] Output: The phase of the setpoint modulation.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_getSetpointModulationPhase(const unsigned int devNo,
                                                   const DSCS_Axis    axis,
                                                   int               *value);


/*!
 * @brief Sets the phase of the setpoint modulation
 *
 * @details
 *  Sets the phase of the sinus setpoint modulation for the x, y and z axis.
 * The values are in 360/2^32 deg as unsigned 32 bit integer.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of the setpoint modulation (@sa DSCS_Axis).
 * @param value      The phase of the setpoint modulation.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setSetpointModulationPhase(const unsigned int devNo,
                                                   const DSCS_Axis    axis,
                                                   const int          value);


/*!
 * @brief Reads the amplitudes of the setpoint modulation
 *
 * @details
 *  Reads the amplitudes of the sinus setpoint modulation for the x, y and z
 * axis. The values are in 632.991/4096 nm as unsigned 32 bit integer.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of the setpoint modulation (@sa DSCS_Axis).
 * @param value[out] Output: The amplitude of the setpoint modulation.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_getSetpointModulationAmplitude(const unsigned int devNo,
                                                       const DSCS_Axis    axis,
                                                       int *value);


/*!
 * @brief Sets the amplitudes of the setpoint modulation
 *
 * @details
 *  Sets the amplitudes of the sinus setpoint modulation for the x, y and z
 * axis. The values are in 632.991/4096 nm as unsigned 32 bit integer.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of the setpoint modulation (@sa DSCS_Axis).
 * @param value      The amplitude of the setpoint modulation.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setSetpointModulationAmplitude(const unsigned int devNo,
                                                       const DSCS_Axis    axis,
                                                       const int value);


/*!
 * @brief Resets the phase of the setpoint modulation
 *
 * @details
 *  Resets the phase of the sinus setpoint modulation for all three axis at
 * once.
 *
 * @param devNo      Sequence number of the device.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_resetSetpointModulationPhase(const unsigned int devNo);


/*!
 * @brief Reads if the PI controller is enabled.
 *
 * @param devNo        Sequence number of the device.
 * @param axis         Axis number of the PI controller (@sa DSCS_Axis).
 * @param enabled[out] Output: The value of the PI controller.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_getPIControllerEnabled(const unsigned int devNo,
                                               const DSCS_Axis    axis,
                                               bln32             *enabled);


/*!
 * @brief Sets if the PI controller is enabled.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of the PI controller (@sa DSCS_Axis).
 * @param enable     The value of the PI controller.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setPIControllerEnabled(const unsigned int devNo,
                                               const DSCS_Axis    axis,
                                               const bln32        enable);


/*!
 * @brief Reads if the sign is inverted for the PI controller.
 *
 * @param devNo        Sequence number of the device.
 * @param axis         Axis number of the PI controller (@sa DSCS_Axis).
 * @param inverted[out] Output: The value of the PI controller.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_getPIControllerInverted(const unsigned int devNo,
                                                const DSCS_Axis    axis,
                                                bln32             *inverted);


/*!
 * @brief Sets if the sign is inverted for the PI controller.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of the PI controller (@sa DSCS_Axis).
 * @param inverted   The value of the PI controller.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setPIControllerInverted(const unsigned int devNo,
                                                const DSCS_Axis    axis,
                                                const bln32        inverted);


/*!
 * @brief Reads the I-Value of the PI controller for the NFO signal.
 *
 * @param devNo        Sequence number of the device.
 * @param value[out]   Output: The I-Value of the PI controller.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_getPIControllerIValueNFO(const unsigned int devNo,
                                                 double            *value);


/*!
 * @brief Sets the I-Value of the PI controller for the NFO signal.
 *
 * @param devNo      Sequence number of the device.
 * @param value      The I-Value of the PI controller.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setPIControllerIValueNFO(const unsigned int devNo,
                                                 const double       value);


/*!
 * @brief Reads the P-Value of the PI controller for the NFO signal.
 *
 * @param devNo        Sequence number of the device.
 * @param value[out]   Output: The P-Value of the PI controller.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_getPIControllerPValueNFO(const unsigned int devNo,
                                                 int               *value);


/*!
 * @brief Sets the P-Value of the PI controller for the NFO signal.
 *
 * @param devNo      Sequence number of the device.
 * @param value      The P-Value of the PI controller.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setPIControllerPValueNFO(const unsigned int devNo,
                                                 const int          value);


/*!
 * @brief Reads the I-Value of the PI controller for the SAM signal.
 *
 * @param devNo        Sequence number of the device.
 * @param value[out]   Output: The I-Value of the PI controller.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_getPIControllerIValueSAM(const unsigned int devNo,
                                                 double            *value);


/*!
 * @brief Sets the I-Value of the PI controller for the SAM signal.
 *
 * @param devNo      Sequence number of the device.
 * @param value      The I-Value of the PI controller.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setPIControllerIValueSAM(const unsigned int devNo,
                                                 const double       value);


/*!
 * @brief Reads the P-Value of the PI controller for the SAM signal.
 *
 * @param devNo        Sequence number of the device.
 * @param value[out]   Output: The P-Value of the PI controller.
 *
 */
DSCS_API int WINCC DSCS_getPIControllerPValueSAM(const unsigned int devNo,
                                                 int               *value);


/*!
 * @brief Sets the P-Value of the PI controller for the SAM signal.
 *
 * @param devNo      Sequence number of the device.
 * @param value      The P-Value of the PI controller.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setPIControllerPValueSAM(const unsigned int devNo,
                                                 const int          value);


/*!
 * @brief Reads the target position of the PI controller.
 *
 * @param devNo        Sequence number of the device.
 * @param axis         Axis number of the PI controller (@sa DSCS_Axis).
 * @param value[out]   Output: The target position of the PI controller
 * [632.991/4096 nm].
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_getPIControllerTargetPosition(const unsigned int devNo,
                                                      const DSCS_Axis    axis,
                                                      int               *value);


/*!
 * @brief Sets the target position of the PI controller.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of the PI controller (@sa DSCS_Axis).
 * @param value      The target position of the PI controller [632.991/4096 nm].
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setPIControllerTargetPosition(const unsigned int devNo,
                                                      const DSCS_Axis    axis,
                                                      const int          value);


/*!
 * @brief Reads which target mode is used for the PI controller.
 *
 * @details
 *  Defines if either the P/I values or the setpoint parameters are used for
 * the PI controller.
 *
 * @param devNo        Sequence number of the device.
 * @param mode         The target mode of the PI controller (@sa
 * DSCS_PI_TargetMode).
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_getPIControllerTargetMode(const unsigned int devNo,
                                                  DSCS_TargetMode   *mode);


/*!
 * @brief Sets which target mode is used for the PI controller.
 *
 * @details
 *  Defines if either the P/I values or the setpoint parameters are used for
 * the PI controller.
 *
 * @param devNo      Sequence number of the device.
 * @param mode       The target mode of the PI controller (@sa
 * DSCS_PI_TargetMode).
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setPIControllerTargetMode(const unsigned int    devNo,
                                                  const DSCS_TargetMode mode);


/*!
 * @brief Resets the PI controller.
 *
 * @param devNo      Sequence number of the device.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_resetPIController(const unsigned int devNo);


/*!
 * @brief Reads the PI controller NFO output value.
 *
 * @param devNo        Sequence number of the device.
 * @param axis         Axis number of the PI controller (@sa DSCS_Axis).
 * @param value[out]   Output: The NFO value of the PI controller.
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getPIControllerNFOOutput(const unsigned int devNo,
                                                 const DSCS_Axis    axis,
                                                 int               *value);


/*!
 * @brief Reads the PI controller SAM output value.
 *
 * @param devNo        Sequence number of the device.
 * @param axis         Axis number of the PI controller (@sa DSCS_Axis).
 * @param value[out]   Output: The SAM value of the PI controller.
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getPIControllerSAMOutput(const unsigned int devNo,
                                                 const DSCS_Axis    axis,
                                                 int               *value);


/*!
 * @brief Reads the limits of the NFO ADC.
 *
 * @details
 *  The values are returned in steps of 19.07 µV. The range is [-10 V, +10 V].
 *
 * @param devNo        Sequence number of the device.
 * @param min[out]     Output: The lower limits of the NFO ADC.
 * @param max[out]     Output: The upper limits of the NFO ADC.
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getNFOADCLimits(const unsigned int devNo,
                                        int               *min,
                                        int               *max);


/*!
 * @brief Sets the limits of the NFO ADC.
 *
 * @details
 *  The values are set in steps of 19.07 µV. The range is [-10 V, +10 V].
 *
 * @param devNo      Sequence number of the device.
 * @param min        The lower limits of the NFO ADC.
 * @param max        The upper limits of the NFO ADC.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setNFOADCLimits(const unsigned int devNo,
                                        const int          min,
                                        const int          max);


/*!
 * @brief Read the limit of the NFO slew rate.
 *
 * @details
 *  The values are returned in steps of approx. 14.55 mV/s (1e9/2^36).
 *
 * @param devNo        Sequence number of the device.
 * @param limit[out]   Output: The limit of the NFO slew rate.
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getNFOSlewRateLimit(const unsigned int devNo,
                                            int               *limit);


/*!
 * @brief Set the limit of the NFO slew rate.
 *
 * @details
 *  The values are set in steps of approx. 14.55 mV/s (1e9/2^36). It is limited
 * to an unsigned 16 bit integer.
 *
 * @param devNo      Sequence number of the device.
 * @param limit      The limit of the NFO slew rate.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setNFOSlewRateLimit(const unsigned int devNo,
                                            const int          limit);


/*!
 * @brief Reads the limits of the SAM ADC.
 *
 * @details
 *  The values are returned in steps of 19.07 µV. The range is [-10 V, +10 V].
 *
 * @param devNo        Sequence number of the device.
 * @param min[out]     Output: The lower limits of the SAM ADC.
 * @param max[out]     Output: The upper limits of the SAM ADC.
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getSAMADCLimits(const unsigned int devNo,
                                        int               *min,
                                        int               *max);


/*!
 * @brief Sets the limits of the SAM ADC.
 *
 * @details
 *  The values are set in steps of 19.07 µV. The range is [-10 V, +10 V].
 *
 * @param devNo      Sequence number of the device.
 * @param min        The lower limits of the SAM ADC.
 * @param max        The upper limits of the SAM ADC.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setSAMADCLimits(const unsigned int devNo,
                                        const int          min,
                                        const int          max);


/*!
 * @brief Reads the limit of the SAM slew rate.
 *
 * @details
 *  The values are returned in steps of approx. 14.55 mV/s (1e9/2^36).
 *
 * @param devNo        Sequence number of the device.
 * @param limit[out]   Output: The limit of the SAM slew rate.
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getSAMSlewRateLimit(const unsigned int devNo,
                                            int               *limit);


/*!
 * @brief Set the limit of the SAM slew rate.
 *
 * @details
 *  The values are set in steps of approx. 14.55 mV/s (1e9/2^36). It is limited
 * to an unsigned 16 bit integer.
 *
 * @param devNo      Sequence number of the device.
 * @param limit      The limit of the SAM slew rate.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setSAMSlewRateLimit(const unsigned int devNo,
                                            const int          limit);


/*!
 * @brief Reads the limiter state.
 *
 * @param devNo        Sequence number of the device.
 * @param state[out]   Output: The state of the limiter (@sa DSCS_LimiterState).
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getLimiterState(const unsigned int devNo,
                                        DSCS_LimiterState *state);


/*!
 * @brief Sets a input transformation matrix coefficient.
 *
 * @details
 *  The coefficients of the input transformation matrix are composed of 3 16 bit
 * integers. These are combined to a 48 bit integer which is interpreted as a
 * fixed point number with 8 bits before the decimal point and 40 bits after the
 * decimal point. The matrix has a dimension of 3x15.
 *
 * @param devNo      Sequence number of the device.
 * @param row        Row number of the matrix [0-2].
 * @param column     Column number of the matrix [0-14].
 * @param coeff1     Coefficient 1 of the matrix.
 * @param coeff2     Coefficient 2 of the matrix.
 * @param coeff3     Coefficient 3 of the matrix.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setInputTransformationMatrix(const unsigned int devNo,
                                                     const int          row,
                                                     const int          column,
                                                     const int          coeff1,
                                                     const int          coeff2,
                                                     const int          coeff3);


/*!
 * @brief Reads the result of the input transformation.
 *
 * @details
 *  The result is defined in steps of approx. 89.20 pm (632.991 nm/4096).
 *
 * @param devNo        Sequence number of the device.
 * @param axis         Axis of the result (@sa DSCS_Axis).
 * @param result[out]  Output: The result of the input transformation.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_getInputTransformationResult(const unsigned int devNo,
                                                     const DSCS_Axis    axis,
                                                     int               *result);


/*!
 * @brief Reads the average of the calculated coordinates of the input
 * transformation.
 *
 * @param devNo        Sequence number of the device.
 * @param result[out]  Output: The average of the calculated coordinates of the
 * input transformation.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_getInputTransformationAverage(const unsigned int devNo,
                                                      int *result);


/*!
 * @brief Reads the state of the input transformation.
 *
 * @param devNo        Sequence number of the device.
 * @param state[out]   Output: The state of the input transformation (@sa
 * DSCS_InputTransformationState).
 *
 * @returns            Error code
 */
DSCS_API int WINCC
DSCS_getInputTransformationState(const unsigned int             devNo,
                                 DSCS_InputTransformationState *state);


/*!
 * @brief Sets the output transformation matrix coefficient.
 *
 * @details
 *  The coefficients of the output transformation matrix are composed of 3 16
 * bit integers. These are combined to a 48 bit integer which is interpreted as
 * a fixed point number with 8 bits before the decimal point and 40 bits after
 * the decimal point. The matrix has a dimension of 6x7.
 *
 * @param devNo      Sequence number of the device.
 * @param row        Row number of the matrix [0-5].
 * @param column     Column number of the matrix [0-6].
 * @param coeff1     Coefficient 1 of the matrix.
 * @param coeff2     Coefficient 2 of the matrix.
 * @param coeff3     Coefficient 3 of the matrix.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setOutputTransformationMatrix(const unsigned int devNo,
                                                      const int          row,
                                                      const int          column,
                                                      const int          coeff1,
                                                      const int          coeff2,
                                                      const int coeff3);


/*!
 * @brief Reads the results of the output transformation.
 *
 * @details
 *  Reads the NFO and SAM results of the output transformation. The values are
 * defined in steps of 89.20 pm (632.991 nm/4096).
 *
 * @param devNo        Sequence number of the device.
 * @param axis         Axis of the result (@sa DSCS_Axis).
 * @param nfo[out]     Output: The NFO result of the output transformation.
 * @param sam[out]     Output: The SAM result of the output transformation.
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getOutputTransformationResult(const unsigned int devNo,
                                                      const DSCS_Axis    axis,
                                                      int               *nfo,
                                                      int               *sam);

#endif  // DSCS_H__
