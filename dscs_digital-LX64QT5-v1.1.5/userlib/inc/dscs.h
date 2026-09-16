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
 * @brief Reads the frequencies of the Lissajous scan
 *
 * @details
 *  Reads the frequencies of the Lissajous scan for the x, y and z
 * axis. The values are in 20.21/2^32 kHz as unsigned 32 bit integer.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of the Lissajous scan (@sa DSCS_Axis).
 * @param value[out] Output: The frequency of the Lissajous scan.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_getLissajousFrequency(const unsigned int devNo,
                                              const DSCS_Axis    axis,
                                              int               *value);


/*!
 * @brief Sets the frequencies of the Lissajous scan
 *
 * @details
 *  Sets the frequencies of the Lissajous scan for the x, y and z
 * axis. The values are in 20.21/2^32 kHz as unsigned 32 bit integer.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of the Lissajous scan (@sa DSCS_Axis).
 * @param value      The frequency of the Lissajous scan.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setLissajousFrequency(const unsigned int devNo,
                                              const DSCS_Axis    axis,
                                              const int          value);


/*!
 * @brief Reads the phase of the Lissajous scan
 *
 * @details
 *  Reads the phase of the Lissajous scan for the x, y and z axis.
 * The values are in 360/2^32 deg as unsigned 32 bit integer.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of the Lissajous scan (@sa DSCS_Axis).
 * @param value[out] Output: The phase of the Lissajous scan.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_getLissajousPhase(const unsigned int devNo,
                                          const DSCS_Axis    axis,
                                          int               *value);


/*!
 * @brief Sets the phase of the Lissajous scan
 *
 * @details
 *  Sets the phase of the Lissajous scan for the x, y and z axis.
 * The values are in 360/2^32 deg as unsigned 32 bit integer.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of the Lissajous scan (@sa DSCS_Axis).
 * @param value      The phase of the Lissajous scan.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setLissajousPhase(const unsigned int devNo,
                                          const DSCS_Axis    axis,
                                          const int          value);


/*!
 * @brief Reads the amplitudes of the Lissajous scan
 *
 * @details
 *  Reads the amplitudes of the Lissajous scan for the x, y and z
 * axis. The values are in 632.991/4096 nm as unsigned 32 bit integer.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of the Lissajous scan (@sa DSCS_Axis).
 * @param value[out] Output: The amplitude of the Lissajous scan.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_getLissajousAmplitude(const unsigned int devNo,
                                              const DSCS_Axis    axis,
                                              int               *value);


/*!
 * @brief Sets the amplitudes of the Lissajous scan
 *
 * @details
 *  Sets the amplitudes of the Lissajous scan for the x, y and z axis.
 * The values are in 632.991/4096 nm as unsigned 32 bit integer.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of the Lissajous scan (@sa DSCS_Axis).
 * @param value      The amplitude of the Lissajous scan.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setLissajousAmplitude(const unsigned int devNo,
                                              const DSCS_Axis    axis,
                                              const int          value);


/*!
 * @brief Reads the offset of the Lissajous scan
 *
 * @details
 *  Reads the offset of the Lissajous scan for the x, y and z axis.
 * The values are in 632.991/4096 nm as signed 32 bit integer.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of the Lissajous scan (@sa DSCS_Axis).
 * @param value[out] Output: The offset of the Lissajous scan.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_getLissajousOffset(const unsigned int devNo,
                                           const DSCS_Axis    axis,
                                           int               *value);


/*!
 * @brief Sets the offset of the Lissajous scan
 *
 * @details
 *  Sets the offset of the Lissajous scan for the x, y and z axis.
 * The values are in 632.991/4096 nm as signed 32 bit integer.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of the Lissajous scan (@sa DSCS_Axis).
 * @param value      The offset of the Lissajous scan.
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setLissajousOffset(const unsigned int devNo,
                                           const DSCS_Axis    axis,
                                           const int          value);


/*!
 * @brief Resets the phase of the Lissajous scan
 *
 * @details
 *  Resets the phase of the Lissajous scan for all three axis at
 * once.
 *
 * @param devNo      Sequence number of the device.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_resetLissajousPhase(const unsigned int devNo);


/*!
 * @brief  Sets a shift on external ADC values.
 *
 * @param devNo      Sequence number of the device.
 * @param shift      The shift value to be set [0,16]
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setExternalADCShift(const unsigned int devNo,
                                            const int          shift);


/*!
 * @brief Reads the shift on external ADC values.
 *
 * @param devNo      Sequence number of the device.
 * @param shift[out]  Output: The shift value [0,16]
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_getExternalADCShift(const unsigned int devNo,
                                            int               *shift);


/*!
 * @brief Reads if the PI controller is enabled for NFO.
 *
 * @param devNo        Sequence number of the device.
 * @param axis         Axis number of the PI controller (@sa DSCS_Axis).
 * @param enabled[out] Output: The value of the PI controller.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_getPIControllerEnabledNFO(const unsigned int devNo,
                                                  const DSCS_Axis    axis,
                                                  bln32             *enabled);


/*!
 * @brief Sets if the PI controller is enabled for NFO.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of the PI controller (@sa DSCS_Axis).
 * @param enable     The value of the PI controller.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setPIControllerEnabledNFO(const unsigned int devNo,
                                                  const DSCS_Axis    axis,
                                                  const bln32        enable);


/*!
 * @brief Reads the I-Value of the PI controller for the NFO signal.
 *
 * @param devNo        Sequence number of the device.
 * @param axis         Axis number of the PI controller (@sa DSCS_Axis).
 * @param value[out]   Output: The I-Value of the PI controller.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_getPIControllerIValueNFO(const unsigned int devNo,
                                                 const DSCS_Axis    axis,
                                                 double            *value);


/*!
 * @brief Sets the I-Value of the PI controller for the NFO signal.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of the PI controller (@sa DSCS_Axis).
 * @param value      The I-Value of the PI controller.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setPIControllerIValueNFO(const unsigned int devNo,
                                                 const DSCS_Axis    axis,
                                                 const double       value);


/*!
 * @brief Reads the P-Value of the PI controller for the NFO signal.
 *
 * @param devNo        Sequence number of the device.
 * @param axis         Axis number of the PI controller (@sa DSCS_Axis).
 * @param value[out]   Output: The P-Value of the PI controller.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_getPIControllerPValueNFO(const unsigned int devNo,
                                                 const DSCS_Axis    axis,
                                                 int               *value);


/*!
 * @brief Sets the P-Value of the PI controller for the NFO signal.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of the PI controller (@sa DSCS_Axis).
 * @param value      The P-Value of the PI controller.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setPIControllerPValueNFO(const unsigned int devNo,
                                                 const DSCS_Axis    axis,
                                                 const int          value);


/*!
 * @brief Reads the limit of the NFO PI controller output values.
 *
 * @param devNo        Sequence number of the device.
 * @param value[out]   Output: The limit of the NFO PI controller (in 632.991 /
 * 4096 nm).
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getPIControllerLimitNFO(const unsigned int devNo,
                                                int               *value);


/*!
 * @brief Sets the limit of the NFO PI controller output values.
 *
 *
 * @param devNo      Sequence number of the device.
 * @param value      The limit of the NFO PI controller (in 632.991 / 4096 nm).
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setPIControllerLimitNFO(const unsigned int devNo,
                                                const int          value);


/*!
 * @brief Reads if the PI controller is enabled for SAM.
 *
 * @param devNo        Sequence number of the device.
 * @param axis         Axis number of the PI controller (@sa DSCS_Axis).
 * @param enabled[out] Output: The value of the PI controller.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_getPIControllerEnabledSAM(const unsigned int devNo,
                                                  const DSCS_Axis    axis,
                                                  bln32             *enabled);


/*!
 * @brief Sets if the PI controller is enabled for SAM.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of the PI controller (@sa DSCS_Axis).
 * @param enable     The value of the PI controller.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setPIControllerEnabledSAM(const unsigned int devNo,
                                                  const DSCS_Axis    axis,
                                                  const bln32        enable);


/*!
 * @brief Reads the I-Value of the PI controller for the SAM signal.
 *
 * @param devNo        Sequence number of the device.
 * @param axis         Axis number of the PI controller (@sa DSCS_Axis).
 * @param value[out]   Output: The I-Value of the PI controller.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_getPIControllerIValueSAM(const unsigned int devNo,
                                                 const DSCS_Axis    axis,
                                                 double            *value);


/*!
 * @brief Sets the I-Value of the PI controller for the SAM signal.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of the PI controller (@sa DSCS_Axis).
 * @param value      The I-Value of the PI controller.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setPIControllerIValueSAM(const unsigned int devNo,
                                                 const DSCS_Axis    axis,
                                                 const double       value);


/*!
 * @brief Reads the P-Value of the PI controller for the SAM signal.
 *
 * @param devNo        Sequence number of the device.
 * @param axis         Axis number of the PI controller (@sa DSCS_Axis).
 * @param value[out]   Output: The P-Value of the PI controller.
 *
 */
DSCS_API int WINCC DSCS_getPIControllerPValueSAM(const unsigned int devNo,
                                                 const DSCS_Axis    axis,
                                                 int               *value);


/*!
 * @brief Sets the P-Value of the PI controller for the SAM signal.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of the PI controller (@sa DSCS_Axis).
 * @param value      The P-Value of the PI controller.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setPIControllerPValueSAM(const unsigned int devNo,
                                                 const DSCS_Axis    axis,
                                                 const int          value);


/*!
 * @brief Reads the limit of the SAM PI controller output values.
 *
 * @param devNo        Sequence number of the device.
 * @param value[out]   Output: The limit of the SAM PI controller (in 632.991 /
 * 4096 nm).
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getPIControllerLimitSAM(const unsigned int devNo,
                                                int               *value);


/*!
 * @brief Sets the limit of the SAM PI controller output values.
 *
 * @param devNo      Sequence number of the device.
 * @param value      The limit of the SAM PI controller (in 632.991 / 4096 nm).
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setPIControllerLimitSAM(const unsigned int devNo,
                                                const int          value);


/*!
 * @brief Resets the PI controller.
 *
 * @param devNo      Sequence number of the device.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_resetPIController(const unsigned int devNo);


/*!
 * @brief Reads the controller NFO output value.
 *
 * @param devNo        Sequence number of the device.
 * @param axis         Axis number of the controller (@sa DSCS_Axis).
 * @param value[out]   Output: The NFO value of the controller.
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getControllerNFOOutput(const unsigned int devNo,
                                               const DSCS_Axis    axis,
                                               int               *value);


/*!
 * @brief Reads the controller SAM output value.
 *
 * @param devNo        Sequence number of the device.
 * @param axis         Axis number of the  controller (@sa DSCS_Axis).
 * @param value[out]   Output: The SAM value of the controller.
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getControllerSAMOutput(const unsigned int devNo,
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
 *  The coefficients of the input transformation matrix is one double value of a
 * 3x15 matrix.
 *
 * @param devNo      Sequence number of the device.
 * @param row        Row number of the matrix [0-2].
 * @param column     Column number of the matrix [0-14].
 * @param coeff      Coefficient of the matrix [0-256].
 *
 * @returns          Error code
 */
DSCS_API int WINCC
DSCS_setInputTransformationCoefficient(const unsigned int devNo,
                                       const int          row,
                                       const int          column,
                                       const double       coeff);


/*!
 * @brief Saves the input transformation matrix coefficients persistently.
 *
 * @details
 *  The coefficients can be saved as user settings persistently in non-volatile
 * memory.
 *
 * @param devNo      Sequence number of the device.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_saveInputTransformationMatrix(const unsigned int devNo);


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
 * @brief Sets a output transformation matrix coefficient.
 *
 * @details
 *  The coefficients of the output transformation matrix is one double value of
 * a 6x7 matrix.
 *
 * @param devNo      Sequence number of the device.
 * @param row        Row number of the matrix [0-5].
 * @param column     Column number of the matrix [0-6].
 * @param coeff      Coefficient of the matrix [-32768,32767].
 *
 * @returns          Error code
 */
DSCS_API int WINCC
DSCS_setOutputTransformationCoefficient(const unsigned int devNo,
                                        const int          row,
                                        const int          column,
                                        const double       coeff);


/*!
 * @brief Saves the output transformation matrix coefficients persistently.
 *
 * @details
 *  The coefficients can be saved as user settings persistently in non-volatile
 * memory.
 *
 * @param devNo      Sequence number of the device.
 *
 * @returns          Error code
 */
DSCS_API int WINCC
DSCS_saveOutputTransformationMatrix(const unsigned int devNo);


/*!
 * @brief Reads the results of the output transformation.
 *
 * @details
 *  Reads the NFO and SAM results of the output transformation. The values are
 * defined in steps of 4.66 nV (20 V / 2^32).
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


/*!
 * @brief Reads the start of a scan line in X direction.
 *
 * @param devNo        Sequence number of the device.
 * @param start[out]   Output: The start of the scan line in X direction
 * (in 632.991/4096 nm).
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getScanLineStartX(const unsigned int devNo, int *start);


/*!
 * @brief Sets the start of a scan line in X direction.
 *
 * @param devNo      Sequence number of the device.
 * @param start      The start of the scan line in X direction (in
 * 632.991/4096 nm).
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setScanLineStartX(const unsigned int devNo,
                                          const int          start);


/*!
 * @brief Reads the end of a scan line in X direction.
 *
 * @param devNo        Sequence number of the device.
 * @param end[out]     Output: The end of the scan line in X direction (in
 * 632.991/4096 nm).
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getScanLineEndX(const unsigned int devNo, int *end);


/*!
 * @brief Sets the end of a scan line in X direction.
 *
 * @param devNo      Sequence number of the device.
 * @param end        The end of the scan line in X direction (in
 * 632.991/4096 nm).
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setScanLineEndX(const unsigned int devNo,
                                        const int          end);


/*!
 * @brief Reads the speed of a scan line in X direction.

 * @param devNo      Sequence number of the device.
 * @param value[out] Output: The speed of the scan line in X direction
 * (in 47.658 pm/s
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_getScanLineSpeedX(const unsigned int devNo, int *value);


/*!
 * @brief Sets the speed of a scan line in X direction.
 *
 * @param devNo      Sequence number of the device.
 * @param value      The speed of the scan line in X direction (in
 * 47.658 pm/s).
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setScanLineSpeedX(const unsigned int devNo,
                                          const int          value);


/*!
 * @brief Reads the start of a scan line in Y direction.
 *
 * @param devNo        Sequence number of the device.
 * @param value[out]   Output: The start of the scan line in Y direction
 * (in 632.991/4096 nm).
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getScanLineStartY(const unsigned int devNo, int *value);


/*!
 * @brief Sets the start of a scan line in Y direction.
 *
 * @param devNo      Sequence number of the device.
 * @param value      The start of the scan line in Y direction (in
 * 632.991/4096 nm).
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setScanLineStartY(const unsigned int devNo,
                                          const int          value);


/*!
 * @brief Reads the distance between lines in Y direction.
 *
 * @param devNo        Sequence number of the device.
 * @param value[out]   Output: The distance between lines in Y direction (in
 * 632.991/4096 nm).
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getScanLineDistY(const unsigned int devNo, int *value);


/*!
 * @brief Sets the distance between lines in Y direction.
 *
 * @param devNo      Sequence number of the device.
 * @param value      The distance between lines in Y direction (in 632.991/4096
 * nm).
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setScanLineDistY(const unsigned int devNo,
                                         const int          value);


/*!
 * @brief Reads number of scan lines in Y direction.
 *
 * @param devNo        Sequence number of the device.
 * @param value[out]   Output: The number of scan lines in Y direction.
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getScanLineCountY(const unsigned int devNo,
                                          unsigned short    *value);


/*!
 * @brief Sets number of scan lines in Y direction.
 *
 * @param devNo      Sequence number of the device.
 * @param value      The number of scan lines in Y direction.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setScanLineCountY(const unsigned int   devNo,
                                          const unsigned short value);


/*!
 * @brief Reads the divider of the scan turn time constant.
 *
 * @details
 *  The value is used as divider N in the formula 212.5e3 / N s. The resulting
 * time is the time used for turning directions in the scan.
 *
 * @param devNo        Sequence number of the device.
 * @param value[out]   Output: Divider of the scan turn time constant.
 * */
DSCS_API int WINCC DSCS_getScanTurnTime(const unsigned int devNo,
                                        unsigned int      *value);


/*!
 * @brief Sets the divider of the scan turn time constant.
 *
 * @details
 *  The value is used as divider N in the formula 212.5e3 / N s. The resulting
 * time is the time used for turning directions in the scan.
 *
 * @param devNo      Sequence number of the device.
 * @param value      Divider of the scan turn time constant.
 *
 * @return          Error code
 */
DSCS_API int WINCC DSCS_setScanTurnTime(const unsigned int devNo,
                                        const unsigned int value);


/*!
 * @brief Reads the divider of the scan position time constant.
 *
 * @details
 *  The value is used as divider in the formula 212.5e3 / N s. The resulting
 * time is the time used for initial positioning of the scan.
 *
 * @param devNo        Sequence number of the device.
 * @param value[out]   Output: Divider of the scan position time constant.
 *
 * @return          Error code
 */
DSCS_API int WINCC DSCS_getScanPosTime(const unsigned int devNo,
                                       unsigned int      *value);


/*!
 * @brief Sets the divider of the scan position time constant.
 *
 * @details
 * The value is used as divider in the formula 212.5e3 / N s. The resulting time
 * is the time used for initial positioning of the scan.
 *
 * @param devNo      Sequence number of the device.
 * @param value      Divider of the scan position time constant.
 *
 * @return          Error code
 */
DSCS_API int WINCC DSCS_setScanPosTime(const unsigned int devNo,
                                       const unsigned int value);


/*!
 * @brief Reads the additional distance in the Y direction for the
 * anti-hysteresis feature.
 *
 * @details
 *  When initially moving to the starting point of the scan, this point is
 * overshot by the specified value. This ensures that every point in the Y
 * direction along the scan is approached from the same side.
 *
 * @param devNo        Sequence number of the device.
 * @param value[out]   Output: Additional distance in the Y direction for
 *                     anti-hysteresis (in 632.991/4096 nm).
 *
 * @return             Error code
 */
DSCS_API int WINCC DSCS_getScanAntiHyst(const unsigned int devNo, int *value);


/*!
 * @brief Sets the additional distance in the Y direction for the
 * anti-hysteresis feature.
 *
 * @details
 *  When initially moving to the starting point of the scan, this point is
 * overshot by the specified value. This ensures that every point in the Y
 * direction along the scan is approached from the same side.
 *
 * @param devNo      Sequence number of the device.
 * @param value      Additional distance in the Y direction for anti-hysteresis
 *                   (in 632.991/4096 nm).
 *
 * @return          Error code
 */
DSCS_API int WINCC DSCS_setScanAntiHyst(const unsigned int devNo,
                                        const int          value);


/*!
 * @brief Reads the scan settings.
 *
 * @details
 *  Die Settings können als Bitmap übergeben werden. Die einzelnen Bits sind in
 * @ref DSCS_ScanSettings definiert. Um mehrere Modi gleichzeitig
 * einzuschalten, müssen die entsprechenden Werte durch eine bitweise
 * ODER-Verknüpfung kombiniert werden.
 *
 * @param devNo        Sequence number of the device.
 * @param value[out]   Output: The scan settings as a bitmask.
 *
 * @return             Error code
 */
DSCS_API int WINCC DSCS_getScanSettings(const unsigned int devNo,
                                        DSCS_ScanSettings *value);


/*!
 * @brief Sets the scan settings.
 *
 * @details
 *  The settings can be passed as a bitmap. The individual bits are defined in
 * @ref DSCS_ScanSettings. To enable multiple modes at once, the
 * corresponding values must be combined using a bitwise OR operation.
 *
 * @param devNo      Sequence number of the device.
 * @param value      The scan settings as a bitmask.
 *
 * @return           Error code
 */
DSCS_API int WINCC DSCS_setScanSettings(const unsigned int      devNo,
                                        const DSCS_ScanSettings value);


/*!
 * @brief Starts the scan.
 *
 * @param devNo      Sequence number of the device.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_startScan(const unsigned int devNo);


/*!
 * @brief Stops the scan.
 *
 * @param devNo      Sequence number of the device.
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_stopScan(const unsigned int devNo);


/*!
 * @brief Reads the shutter state.
 *
 * @param devNo        Sequence number of the device.
 * @param state[out]   Output: The state of the shutter (@sa DSCS_ShutterState).
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getShutterState(const unsigned int devNo,
                                        DSCS_ShutterState *state);


/*!
 * @brief Reads the windows for the shutter activation.
 *
 * @details
 *  Configures the window in which the shutter is open.
 *
 * @param devNo        Sequence number of the device.
 * @param axis[in]     Axis number of the shutter activation window
 * @param lower[out]   Output: The lower limit of the shutter activation window
 *                     (in 632.991/4096 nm).
 * @param upper[out]   Output: The upper limit of the shutter activation window
 *                     (in 632.991/4096 nm).
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getShutterActivationWindow(const unsigned int devNo,
                                                   const DSCS_Axis    axis,
                                                   int               *lower,
                                                   int               *upper);


/*!
 * @brief Sets the windows for the shutter activation.
 *
 * @details
 *  Configures the window in which the shutter is open.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of the shutter activation window
 * @param lower      The lower limit of the shutter activation window (in
 *                   632.991/4096 nm).
 * @param upper      The upper limit of the shutter activation window (in
 *                   632.991/4096 nm).
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setShutterActivationWindow(const unsigned int devNo,
                                                   const DSCS_Axis    axis,
                                                   const int          lower,
                                                   const int          upper);


/*!
 * @brief Reads the artificial hysteresis constant of the shutter.
 *
 * @details
 *  Allows to configure an artificial hysteresis for the shutter. This
 * hysteresis is used to prevent the shutter from opening and closing too
 * frequently.
 *
 * @param devNo        Sequence number of the device.
 * @param value[out]   Output: The artificial hysteresis constant of the shutter
 *                     (in 632.991/4096 nm).
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getShutterHysteresis(const unsigned int devNo,
                                             int               *value);


/*!
 * @brief Sets the artificial hysteresis constant of the shutter.
 *
 * @details
 *  Allows to configure an artificial hysteresis for the shutter. This
 * hysteresis is used to prevent the shutter from opening and closing too
 * frequently.
 *
 * @param devNo      Sequence number of the device.
 * @param value      The artificial hysteresis constant of the shutter (in
 *                   632.991/4096 nm).
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setShutterHysteresis(const unsigned int devNo,
                                             const int          value);


/*!
 * @brief Reads the area detector counter.
 *
 * @details
 *  Reads the counter which is incremented when the area detector input is
 * triggered by a rising edge.
 *
 * @param devNo        Sequence number of the device.
 * @param value[out]   Output: The area detector counter.
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getAreaDetectorCounter(const unsigned int devNo,
                                               unsigned int      *value);


/*!
 * @brief Reads the X-Ray intensity counter.
 *
 * @details
 *  Reads the counter which is incremented when the X-Ray intensity input is
 * triggered by a rising edge.
 *
 * @param devNo        Sequence number of the device.
 * @param value[out]   Output: The X-Ray intensity counter.
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getXRayIntensityCounter(const unsigned int devNo,
                                                unsigned int      *value);


/*!
 * @brief Reads the status bits of the XRF dead time signal.
 *
 * @param devNo        Sequence number of the device.
 * @param status[out]  Output: The status bits of the XRF dead time signal.
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getXRFDeadTimeStatus(const unsigned int devNo,
                                             unsigned int      *status);


/*!
 * @brief Reads the piezo model fit parameters.
 *
 * @param devNo        Sequence number of the device.
 * @param type         The type of the piezo model parameter.
 * @param direction    The direction of the piezo model parameter.
 * @param params[out]  Output: The piezo model fit parameters [lsb].
 *
 * @returns            Error code
 */
DSCS_API int WINCC
DSCS_getPiezoModelFitParameters(const unsigned int              devNo,
                                const DSCS_PiezoModelParameters type,
                                const DSCS_Direction            direction,
                                int                            *params);


/*!
 * @brief Sets the piezo model fit parameters.
 *
 * @param devNo        Sequence number of the device.
 * @param type         The type of the piezo model parameter.
 * @param direction    The direction of the piezo model parameter.
 * @param params       The piezo model fit parameters [lsb].
 *
 * @returns            Error code
 */
DSCS_API int WINCC
DSCS_setPiezoModelFitParameters(const unsigned int              devNo,
                                const DSCS_PiezoModelParameters type,
                                const DSCS_Direction            direction,
                                const int                       params);


/*!
 * @brief Reads the trajectory generator mode
 *
 * @param devNo        Sequence number of the device.
 * @param mode[out]    Output: The trajectory generator mode (@sa
 * DSCS_TrajectoryMode).
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getTrajectoryMode(const unsigned int   devNo,
                                          DSCS_TrajectoryMode *mode);


/*! @brief Sets the trajectory generator mode
 *
 * @param devNo      Sequence number of the device.
 * @param mode       The trajectory generator mode (@sa DSCS_TrajectoryMode).
 *
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setTrajectoryMode(const unsigned int        devNo,
                                          const DSCS_TrajectoryMode mode);


/*!
 * @brief Reads the direct target of the trajectory generator
 *
 * @details
 *  Only used if @ref DSCS_TrajectoryMode is set to TrajectoryMode_DirectTarget.
 *
 * @param devNo        Sequence number of the device.
 * @param axis         Axis number of the target (@sa DSCS_Axis).
 * @param val[out]     Output: The direct target (in 632.991/4096 nm).
 *
 * @returns            Error code
 *
 * @sa DSCS_getTrajectoryMode, DSCS_setTrajectoryMode
 */
DSCS_API int WINCC DSCS_getTrajectoryDirectTarget(const unsigned int devNo,
                                                  const DSCS_Axis    axis,
                                                  int               *val);


/*!
 * @brief Sets the direct target of the trajectory generator
 *
 * @details
 *  Only used if @ref DSCS_TrajectoryMode is set to TrajectoryMode_DirectTarget.
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of the target (@sa DSCS_Axis).
 * @param val        The direct target (in 632.991/4096 nm).
 *
 * @returns          Error code
 *
 * @sa DSCS_getTrajectoryMode, DSCS_setTrajectoryMode
 */
DSCS_API int WINCC DSCS_setTrajectoryDirectTarget(const unsigned int devNo,
                                                  const DSCS_Axis    axis,
                                                  const int          val);


/*!
 * @brief Reads the trajectory generator state.
 *
 * @param devNo        Sequence number of the device.
 * @param state[out]   Output: The trajectory generator state (@sa
 * DSCS_TrajectoryState).
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getTrajectoryState(const unsigned int    devNo,
                                           DSCS_TrajectoryState *state);


/*!
 * @brief Reads the measured input values for the controller.
 *
 * @param devNo        Sequence number of the device.
 * @param axis         Axis number of the input (@sa DSCS_Axis).
 * @param value[out]   Output: The input value (in 632.991/4096 nm).
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getControllerMeasureValue(const unsigned int devNo,
                                                  const DSCS_Axis    axis,
                                                  int               *value);


/*!
 * @brief Reads the trajectory input values for the controller.
 *
 * @param devNo        Sequence number of the device.
 * @param axis         Axis number of the input (@sa DSCS_Axis).
 * @param value[out]   Output: The input value (in 632.991/4096 nm).
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getControllerTrajectoryValue(const unsigned int devNo,
                                                     const DSCS_Axis    axis,
                                                     int               *value);


/*!
 * @brief Reads the enabled controller settings.
 *
 * @param devNo        Sequence number of the device.
 * @param value[out]   Output: The enabled controller settings as a bitmask
 * (@ref DSCS_ControllerSettings).
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getControllerSettings(const unsigned int       devNo,
                                              DSCS_ControllerSettings *value);


/*!
 * @brief Sets the enabled controller settings.
 *
 * @param devNo        Sequence number of the device.
 * @param value        The enabled controller settings as a bitmask (@ref
 * DSCS_ControllerSettings).
 *
 * @returns            Error code
 */
DSCS_API int WINCC
DSCS_setControllerSettings(const unsigned int            devNo,
                           const DSCS_ControllerSettings value);


/*!
 * @brief Reads the communication rate  for digital -> analog device.
 *
 * @param devNo        Sequence number of the device.
 * @param port         Display port number (@sa DSCS_DisplayPort).
 * @param rate[out]    Output: The communication rate (in kHz).
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getCommunicationRateD2A(const unsigned int     devNo,
                                                const DSCS_DisplayPort port,
                                                int                   *rate);


/*!
 * @brief Sets the communication rate for digital -> analog device.
 *
 * @param devNo        Sequence number of the device.
 * @param port         Display port number (@sa DSCS_DisplayPort).
 * @param rate         The communication rate (in kHz).
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_setCommunicationRateD2A(const unsigned int     devNo,
                                                const DSCS_DisplayPort port,
                                                const int              rate);


/*!
 * @brief Reads the communication rate for analog -> digital device.
 *
 * @param devNo        Sequence number of the device.
 * @param port         Display port number (@sa DSCS_DisplayPort).
 * @param rate[out]    Output: The communication rate (in kHz).
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getCommunicationRateA2D(const unsigned int     devNo,
                                                const DSCS_DisplayPort port,
                                                int                   *rate);


/*!
 * @brief Sets the communication rate for analog -> digital device.
 *
 * @param devNo        Sequence number of the device.
 * @param port         Display port number (@sa DSCS_DisplayPort).
 * @param rate         The communication rate (in kHz).
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_setCommunicationRateA2D(const unsigned int     devNo,
                                                const DSCS_DisplayPort port,
                                                const int              rate);


/*!
 * @brief Reads the communication state for the display ports.
 *
 * @param devNo        Sequence number of the device.
 * @param port         Display port number (@sa DSCS_DisplayPort).
 * @param state[out]   Output: The communication state (@sa DSCS_CommState).
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getCommunicationState(const unsigned int     devNo,
                                              const DSCS_DisplayPort port,
                                              DSCS_CommState        *state);


/*!
 * @brief Sets Zygo SCLK factor for pixel rate trigger output.
 *
 * @details
 *  The factor is used as multiplication factor for the pixel rate trigger
 * output. It is used to generate a trigger signal based on the Zygo SCLK (700
 * kHz).
 *
 * @param devNo        Sequence number of the device.
 * @param factor       SCLK multiplication factor for pixel rate trigger output
 * [1-8].
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_setPixelRateTriggerFactor(const unsigned int devNo,
                                                  const int          factor);


/*!
 * @brief Reads Zygo SCLK factor for pixel rate trigger output.
 *
 * @param devNo        Sequence number of the device.
 * @param factor[out]  Output: SCLK multiplication factor for pixel rate trigger
 * output.
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getPixelRateTriggerFactor(const unsigned int devNo,
                                                  int               *factor);


/*!
 * @brief Sets Zygo SCLK divisor for area detector trigger output.
 *
 * @details
 *  The divisor is used as division factor for the area detector trigger output.
 * It is used to generate a trigger signal based on the Zygo SCLK (700 kHz).
 *
 * @param devNo        Sequence number of the device.
 * @param divisor      SCLK division factor for area detector trigger output
 * [1-MAX_UINT16].
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_setAreaDetectorTriggerDivisor(const unsigned int devNo,
                                                      const int divisor);


/*!
 * @brief Reads Zygo SCLK divisor for area detector trigger output.
 *
 * @param devNo        Sequence number of the device.
 * @param divisor[out] Output: SCLK division factor for area detector trigger
 * output.
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getAreaDetectorTriggerDivisor(const unsigned int devNo,
                                                      int *divisor);


/*!
 * @brief Reads the Zygo data reception state.
 *
 * @details
 *  Reads the following states for the 12 channels of the Zygo card:
 * 1. Whether the Zygo data transmission is active.
 * 2. Whether the data transmission rate is outside the correct range
 *    (769-770 Hz). If it is outside the range, the second value is set. If the
 *    data transmission rate is within the range, this error value is reset.
 * 3. Whether the CRC validity of the data transmission or the data transmission
 *    rate was faulty. This is a "sticky" error value that can only be reset by
 *    the user.
 *
 * @param devNo          Sequence number of the device.
 * @param active[out]    Output: [Bits]: 0...11 = Channels 1...12, 1 = data
 *                       reception active
 * @param rateError[out] Output: [Bits]: 0...11 = Channels 1...12, 1 = Data
 *                       reception rate outside the limits (< 769 oder > 770)
 * @param error[out]     Output: [Bits]: 0...11 = Channels 1...12, 1 = Sticky
 *                       error bits
 * 
 * @returns              Error code
 */
DSCS_API int WINCC DSCS_getZygoDataReceptionState(const unsigned int devNo,
                                                  unsigned int      *active,
                                                  unsigned int      *rateError,
                                                  unsigned int      *error);


/*!
 * @brief Clears the sticky error bits of the Zygo data reception state.
 *
 * @param devNo        Sequence number of the device.
 *
 * @returns            Error code
 * 
 * @sa DSCS_getZygoDataReceptionState
 */
DSCS_API int WINCC DSCS_clearZygoDataReceptionError(const unsigned int devNo);


/*!
 * @brief Reads Zygo position
 *
 * @param devNo         Sequence number of the device.
 * @param axis          Axis number of Zygo position readback (@sa DSCS_Axis).
 * @param position[out] Reads the position from the selected axis (in 154.54
 * pm).
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getZygoPosition(const unsigned int devNo,
                                        const DSCS_Axis    axis,
                                        const int         *position);


/*!
 * @brief Reads Zygo NFO angle
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of Zygo NFO angle readback (@sa DSCS_Axis).
 * @param angle[out] Reads the NFO angle from the selected axis (in 15.454
 * nRad).
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getZygoAngleNFO(const unsigned int devNo,
                                        const DSCS_Axis    axis,
                                        const int         *angle);


/*!
 * @brief Reads Zygo SAM angle
 *
 * @param devNo      Sequence number of the device.
 * @param axis       Axis number of Zygo SAM angle readback (@sa DSCS_Axis).
 * @param angle[out] Reads the SAM angle from the selected axis (in 15.454
 * nRad).
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getZygoAngleSAM(const unsigned int devNo,
                                        const DSCS_Axis    axis,
                                        const int         *angle);


/*!
 * @brief Sets the state of the pixel trigger output.
 *
 * @details
 *  Allows to start (1) or stop (0) the pixel trigger output.
 * If the pixel trigger output is stopped its counter is reset to zero.
 * 
 * @param devNo      Sequence number of the device.
 * @param state      The state of the pixel trigger output (0 = stop, 1 = start).
 * 
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_setPixelTriggerOutputState(const unsigned int devNo,
                                                   const bln32        state);


/*!
 * @brief Reads the state of the pixel trigger output.
 *
 * @param devNo        Sequence number of the device.
 * @param state[out]   Output: The state of the pixel trigger output (0 = stop, 1 = start).
 * 
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_getPixelTriggerOutputState(const unsigned int devNo,
                                                   bln32             *state);


/*!
 * @brief Resets the internal Zygo position/angles to zero.
 *
 * @details
 *  Saves the current Zygo positions/angles as offsets which are then used to internally reset 
 * the Zygo positions/angles to zero.
 * 
 * @param devNo      Sequence number of the device.
 * 
 * @returns          Error code
 */
DSCS_API int WINCC DSCS_resetZygoValues(const unsigned int devNo);



/*!
 * @brief Resets the internal Zygo position/angles offsets to zero.
 *
 * @param devNo      Sequence number of the device.
 * 
 * @returns          Error code
 * 
 * @sa DSCS_resetZygoValues
 */
DSCS_API int WINCC DSCS_resetZygoValueOffsets(const unsigned int devNo);


#if (0)

/*!
 * @brief Sends Zygo clear commands.
 *
 * @param devNo        Sequence number of the device.
 * @param select       Selects what should be cleared (@sa DSCS_ClearSelect). 0
 * = Clear sticky error bits (@ref DSCS_getZygoState), 1 = Set outputs to zero,
 * 2 = Clear offsets.
 *
 * @returns            Error code
 */
DSCS_API int WINCC DSCS_setZygoClear(const unsigned int     devNo,
                                     const DSCS_ClearSelect select);





#endif
#endif  // DSCS_H__
