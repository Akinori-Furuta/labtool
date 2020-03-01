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
#include "labtoolcalibrationdata.h"
#include <string.h>
#include "math.h"
#include "labtooldevicespec.h"

/*!
    \class LabToolCalibrationData
    \brief Holds the calibration data loaded from the LabTool Hardware.

    \ingroup Device

    The LabToolCalibrationData class calculates the scaling factors based
    on the raw calibration data from the LabTool Hardware. The scaling factors
    are used to convert the captured data samples into correctly calibrated
    floating point values in Volts.
*/

/*!
    Constructs a new set of calibration data based on the raw \a data.
*/
LabToolCalibrationData::LabToolCalibrationData(const quint8 *data)
{
    memcpy(&mRawResult, data, sizeof(calib_result));
    mReasonableData = true;

    // Calculate calibration factors
    //
    //   B = (Vin1 - Vin2)/(hex1 - hex2)
    //   A = Vin1 - B*hex1
    //
    // Both A and B must be calculated for each channel and for each V/div setting

    for (int i = 0; i < LabToolDeviceSpec::ANALOG_IN_RANGES; i++)
    {

        for (int ch = 0; ch < LabToolDeviceSpec::ANALOG_IN_CHANNELS; ch++)
        {

            double vin1 = estimateActualDacVoltage(ch, mRawResult.voltsInLow[i]);
            double vin2 = estimateActualDacVoltage(ch, mRawResult.voltsInHigh[i]);
            double a;
            double b;

            b = (vin1 - vin2) / (mRawResult.inLow[ch][i] - mRawResult.inHigh[ch][i]);
            mCalibB[ch][i] = b;
            a = vin1 - b * mRawResult.inLow[ch][i];
            mCalibA[ch][i] = a;

            if (mReasonableData)
            {
                if (isInfiniteOrNan(a))
                {
                    mReasonableData = false;
                }
                else if ((a < -1000) || (a > 1000))
                {
                    mReasonableData = false;
                }
                else if (isInfiniteOrNan(b))
                {
                    mReasonableData = false;
                }
                else if ((b < -1000) || (b > 1000))
                {
                    mReasonableData = false;
                }
            }
        }
    }
}

/*! Estimate actual DAC output voltage from target output voltage.
 *
 * @param ch DAC channel number 0..1
 * @param targetMv target DAC output voltage in mV.
 *
 * @retval double estimated actual DAC output voltage.
 */
double LabToolCalibrationData::estimateActualDacVoltage(int ch, int targetMv)
{   /* Note: Calculate in float as LabTool device does. */
    float vL; /* Measured DAC output voltage at Low Position */
    float vH; /* Measured DAC output voltage at High Position */
    float dL; /* DACin value at Low position. */
    float dH; /* DACin value at High position. */
    float a;  /* Estimated DAC output voltage at DACin value = 0 */
    float b;  /* mVolts / DACin */
    float vActual;
    int    dacIn;

    dL = mRawResult.dacValOut[LabToolDeviceSpec::ANALOG_IN_CAL_LOW];
    dH = mRawResult.dacValOut[LabToolDeviceSpec::ANALOG_IN_CAL_HIGH];
    vL = mRawResult.userOut[ch][LabToolDeviceSpec::ANALOG_IN_CAL_LOW];
    vH = mRawResult.userOut[ch][LabToolDeviceSpec::ANALOG_IN_CAL_HIGH];

    /* Note: We calculate almost formulas in mV scale. */

    b = (vH - vL) / (dH - dL);
    a = vL - b * dL;

    dacIn = (targetMv - a) / b;
    dacIn = LabToolDeviceSpec::spiDacClipValue(dacIn);

    vActual = a + b * dacIn;
    /* Scale mV to V. */
    return vActual / 1000.0f;
}

/*!
    Prints a table with the raw calibration data for each of the Volts/div levels.
*/
void LabToolCalibrationData::printRawInfo()
{
    qDebug("Got result:");
    qDebug("userOut { { %d, %d, %d }, { %d, %d, %d }",
           mRawResult.userOut[0][0],
           mRawResult.userOut[0][1],
           mRawResult.userOut[0][2],
           mRawResult.userOut[1][0],
           mRawResult.userOut[1][1],
           mRawResult.userOut[1][2]);

    qDebug("               Low               High");
    qDebug(" V/div     mV    A0   A1      mV    A0   A1");
    qDebug("-------  ------ ---- ----   ------ ---- ----");int i = 0;
    qDebug("   20mV   %5d %4u %4u    %5d %4u %4u", mRawResult.voltsInLow[i], mRawResult.inLow[0][i], mRawResult.inLow[1][i], mRawResult.voltsInHigh[i], mRawResult.inHigh[0][i], mRawResult.inHigh[1][i]);i++;
    qDebug("   50mV   %5d %4u %4u    %5d %4u %4u", mRawResult.voltsInLow[i], mRawResult.inLow[0][i], mRawResult.inLow[1][i], mRawResult.voltsInHigh[i], mRawResult.inHigh[0][i], mRawResult.inHigh[1][i]);i++;
    qDebug("  100mV   %5d %4u %4u    %5d %4u %4u", mRawResult.voltsInLow[i], mRawResult.inLow[0][i], mRawResult.inLow[1][i], mRawResult.voltsInHigh[i], mRawResult.inHigh[0][i], mRawResult.inHigh[1][i]);i++;
    qDebug("  200mV   %5d %4u %4u    %5d %4u %4u", mRawResult.voltsInLow[i], mRawResult.inLow[0][i], mRawResult.inLow[1][i], mRawResult.voltsInHigh[i], mRawResult.inHigh[0][i], mRawResult.inHigh[1][i]);i++;
    qDebug("  500mV   %5d %4u %4u    %5d %4u %4u", mRawResult.voltsInLow[i], mRawResult.inLow[0][i], mRawResult.inLow[1][i], mRawResult.voltsInHigh[i], mRawResult.inHigh[0][i], mRawResult.inHigh[1][i]);i++;
    qDebug(" 1000mV   %5d %4u %4u    %5d %4u %4u", mRawResult.voltsInLow[i], mRawResult.inLow[0][i], mRawResult.inLow[1][i], mRawResult.voltsInHigh[i], mRawResult.inHigh[0][i], mRawResult.inHigh[1][i]);i++;
    qDebug(" 2000mV   %5d %4u %4u    %5d %4u %4u", mRawResult.voltsInLow[i], mRawResult.inLow[0][i], mRawResult.inLow[1][i], mRawResult.voltsInHigh[i], mRawResult.inHigh[0][i], mRawResult.inHigh[1][i]);i++;
    qDebug(" 5000mV   %5d %4u %4u    %5d %4u %4u", mRawResult.voltsInLow[i], mRawResult.inLow[0][i], mRawResult.inLow[1][i], mRawResult.voltsInHigh[i], mRawResult.inHigh[0][i], mRawResult.inHigh[1][i]);i++;
}

/*!
    Prints a table with the calculated calibration factors for each of the Volts/div levels.
*/
void LabToolCalibrationData::printCalibrationInfo()
{
    int i = 0;
    qDebug("Calibration data:");
    if (isDefaultData()) {
        qDebug("USING DEFAULT DATA - The EEPROM is either empty or contains invalid data!");
    }
    if (!isDataReasonable()) {
        qDebug("Data seems to contain strange values, consider recalibrating!");
    }

    qDebug(" V/div     A0  A      A0  B       A1  A      A1  B   ");
    qDebug("-------  ---------- ----------  ---------- ----------");
    qDebug("   20mV  %10.7f %10.7f  %10.7f %10.7f", mCalibA[0][i], mCalibB[0][i], mCalibA[1][i], mCalibB[1][i]);i++;
    qDebug("   50mV  %10.7f %10.7f  %10.7f %10.7f", mCalibA[0][i], mCalibB[0][i], mCalibA[1][i], mCalibB[1][i]);i++;
    qDebug("  100mV  %10.7f %10.7f  %10.7f %10.7f", mCalibA[0][i], mCalibB[0][i], mCalibA[1][i], mCalibB[1][i]);i++;
    qDebug("  200mV  %10.7f %10.7f  %10.7f %10.7f", mCalibA[0][i], mCalibB[0][i], mCalibA[1][i], mCalibB[1][i]);i++;
    qDebug("  500mV  %10.7f %10.7f  %10.7f %10.7f", mCalibA[0][i], mCalibB[0][i], mCalibA[1][i], mCalibB[1][i]);i++;
    qDebug(" 1000mV  %10.7f %10.7f  %10.7f %10.7f", mCalibA[0][i], mCalibB[0][i], mCalibA[1][i], mCalibB[1][i]);i++;
    qDebug(" 2000mV  %10.7f %10.7f  %10.7f %10.7f", mCalibA[0][i], mCalibB[0][i], mCalibA[1][i], mCalibB[1][i]);i++;
    qDebug(" 5000mV  %10.7f %10.7f  %10.7f %10.7f", mCalibA[0][i], mCalibB[0][i], mCalibA[1][i], mCalibB[1][i]);i++;
}

bool LabToolCalibrationData::isInfiniteOrNan(double d)
{
    // Check whether the compiler is c++11 conformant.
    // With c++11 the isfinite() and isnan() macros became functions.
#if (__cplusplus == 201103L)
    return !std::isfinite(d) || std::isnan(d);
#else
    return !isfinite(d) || isnan(d);
#endif
}

/*!
    \fn static bool LabToolCalibrationData::isInfiniteOrNan()

    Returns true if \a d is INFINITE or NaN, false otherwise.
*/

/*!
    \fn static int LabToolCalibrationData::rawDataByteSize()

    Returns the size of the raw data structure
*/

/*!
    \fn double LabToolCalibrationData::analogFactorA()

    Returns the A factor for analog input based on which analog channel \a ch and
    it's Volt/div setting \a voltsPerDivIndex
*/

/*!
    \fn double LabToolCalibrationData::analogFactorB()

    Returns the B factor for analog input based on which analog channel \a ch and
    it's Volt/div setting \a voltsPerDivIndex
*/

/*!
    \fn const quint8* LabToolCalibrationData::rawCalibrationData()

    Returns a pointer to the raw calibration data. Used when saving the data in
    the LabTool Hardware's persistant memory.
*/

/*!
    \fn bool LabToolCalibrationData::isDefaultData()

    Returns true if the raw data represents the default settings and not the
    data specific to the connected hardware.
*/

/*!
    \fn bool LabToolCalibrationData::isDataReasonable()

    Does a simple validation of the calibration parameters. Returns true if the
    data passes the validation.
*/

