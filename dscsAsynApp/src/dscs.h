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


#endif  // DSCS_H__
