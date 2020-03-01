/*
 *  Copyright 2013 Embedded Artists AB
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
#ifndef LABTOOLCALIBRATIONDATA_H
#define LABTOOLCALIBRATIONDATA_H

#include "device/labtool/labtooldevicespec.h"
#include <qglobal.h>
#include <QString>

class LabToolCalibrationData
{
private:

    /*! \brief Raw calibration data.
     * This is part of the \ref calib_result_t structure that is read from
     * the LabTool Hardware.
     *
     * \private
     */
    struct calib_result
    {
      quint32 cmd;            /*!< Marker used by the protocol */
      quint32 checksum;       /*!< Checksum to assure correct read/write to EEPROM */
      quint32 version;        /*!< Future proof the data by adding a version number */
      quint32 dacValOut[LabToolDeviceSpec::ANALOG_IN_CAL_NUMS];   /*!< DAC values in 10-bit format used for calibration of analog out */
      int     userOut[LabToolDeviceSpec::ANALOG_IN_CHANNELS][LabToolDeviceSpec::ANALOG_IN_CAL_NUMS];  /*!< User's measured analog output in mV for dacValOut's values */

      /* Both voltsInLow and voltsInHigh are target value, there are not actual values. */
      int     voltsInLow[LabToolDeviceSpec::ANALOG_IN_RANGES];  /*!< Analog output values in mV used for calibration of analog in for each V/div */
      int     voltsInHigh[LabToolDeviceSpec::ANALOG_IN_RANGES]; /*!< Analog output values in mV used for calibration of analog in for each V/div */
      quint32 inLow[LabToolDeviceSpec::ANALOG_IN_CHANNELS][LabToolDeviceSpec::ANALOG_IN_RANGES];    /*!< Measured analog in for each channel and V/div combo at low output*/
      quint32 inHigh[LabToolDeviceSpec::ANALOG_IN_CHANNELS][LabToolDeviceSpec::ANALOG_IN_RANGES];   /*!< Measured analog in for each channel and V/div combo at high output*/
    };

    double mCalibA[LabToolDeviceSpec::ANALOG_IN_CHANNELS][LabToolDeviceSpec::ANALOG_IN_RANGES];
    double mCalibB[LabToolDeviceSpec::ANALOG_IN_CHANNELS][LabToolDeviceSpec::ANALOG_IN_RANGES];
    calib_result mRawResult;
    bool mReasonableData;

private:
    static bool isInfiniteOrNan(double d);
    double estimateActualDacVoltage(int ch, int targetMv);

public:
    LabToolCalibrationData(const quint8* data);

    static int rawDataByteSize() { return sizeof(calib_result); }

    double analogFactorA(int ch, int voltsPerDivIndex) { return mCalibA[ch][voltsPerDivIndex]; }
    double analogFactorB(int ch, int voltsPerDivIndex) { return mCalibB[ch][voltsPerDivIndex]; }

    const quint8* rawCalibrationData() { return (const quint8*)&mRawResult; }

    bool isDefaultData() { return (mRawResult.checksum == 0x00dead00 || mRawResult.version == 0x00dead00); }
    bool isDataReasonable() { return mReasonableData; }

    void printRawInfo();
    void printCalibrationInfo();
};

#endif // LABTOOLCALIBRATIONDATA_H
