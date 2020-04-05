/*!
 * @file
 * @brief     Handles calibration of analog signals (both in and out)
 *
 * @copyright Copyright 2013 Embedded Artists AB
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef __CALIBRATE_H
#define __CALIBRATE_H

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "lpc_types.h"
#include "error_codes.h"
#include "circbuff.h"

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/

#define ANALOG_IN_CHANNELS	(2)
#define ANALOG_IN_RANGES	(8)

#define ANALOG_OUT_LO_DAC       (256)
#define ANALOG_OUT_HI_DAC       (768)
#define ANALOG_OUT_HL_DAC_DELTA (ANALOG_OUT_HI_DAC - ANALOG_OUT_LO_DAC)

#define ANALOG_OUT_LO_MV ( 2750)
#define ANALOG_OUT_HI_MV (-2750)
#define ANALOG_OUT_HL_DELTA_MV (ANALOG_OUT_HI_MV - ANALOG_OUT_LO_MV)

#define ANALOG_OUT_LO_MAX_MV ( 5800)
#define ANALOG_OUT_LO_MIN_MV ( 2000)
#define ANALOG_OUT_HI_MAX_MV (-2000)
#define ANALOG_OUT_HI_MIN_MV (-5800)

#define ANALOG_OUT_DMV_DC (((float)ANALOG_OUT_HI_MV - (float)ANALOG_OUT_LO_MV) / \
        ((float)ANALOG_OUT_HI_DAC - (float)ANALOG_OUT_LO_DAC))

#define ANALOG_OUT_DMV_MIN_DC (ANALOG_OUT_DMV_DC - (float)10.0f)
#define ANALOG_OUT_DMV_MAX_DC (ANALOG_OUT_DMV_DC + (float)10.0f)

typedef enum {
  ANALOG_IN_CAL_LOW = 0,
  ANALOG_IN_CAL_MIDDLE = 1,
  ANALOG_IN_CAL_HIGH = 2,
  ANALOG_IN_CAL_NUMS = 3,
} calib_levels;

/*! @brief The calibration data stored in EEPROM. */
typedef struct
{
  uint32_t checksum;       /*!< Checksum to assure correct read/write to EEPROM */
  uint32_t version;        /*!< Future proof the data by adding a version number */
  uint32_t dacValOut[ANALOG_IN_CAL_NUMS];   /*!< DAC values in 10-bit format used for calibration of analog out */
  int      userOut[ANALOG_IN_CHANNELS][ANALOG_IN_CAL_NUMS];  /*!< User's measured analog output in mV for dacValOut's values */

  int      voltsInLow[ANALOG_IN_RANGES];  /*!< Analog output values in mV used for calibration of analog in for each V/div */
  int      voltsInHigh[ANALOG_IN_RANGES]; /*!< Analog output values in mV used for calibration of analog in for each V/div */
  uint32_t inLow[ANALOG_IN_CHANNELS][ANALOG_IN_RANGES];    /*!< Measured analog in for each channel and V/div combo at low output*/
  uint32_t inHigh[ANALOG_IN_CHANNELS][ANALOG_IN_RANGES];   /*!< Measured analog in for each channel and V/div combo at high output*/
} calib_result_t;

/*! @brief Statemachine for the calibration process. */
typedef enum
{
  CALIB_STATE_STOPPED,        /*!< No calibration ongoing */
  CALIB_STATE_AOUT_FIRST,     /*!< Calibrating analog outputs, first request. */
  CALIB_STATE_AOUT_AGAIN,     /*!< Calibrating analog outputs, request again. */
  CALIB_STATE_AIN_SETUP_LOW,  /*!< Setting up analog outputs to low level for analog input calibration */
  CALIB_STATE_AIN_SETUP_HIGH, /*!< Setting up analog outputs to high level for analog input calibration */
  CALIB_STATE_AIN_PROCESS,    /*!< Processing captured analog input samples looking for average levels */
  CALIB_STATE_AIN_WAIT,       /*!< Waiting for capturing of analog input samples to complete */
  CALIB_STATE_SLEEP,          /*!< Delaying before next operation */
  CALIB_STATE_STOPPING,       /*!< Calibration about to be stopped. Waiting for everything to finish. */
} calib_states_t;

/******************************************************************************
 * Global Variables
 *****************************************************************************/

extern calib_states_t calibrationState;

/******************************************************************************
 * Functions
 *****************************************************************************/

cmd_status_t calibrate_Init(void);
cmd_status_t calibrate_AnalogOut(uint8_t* cfg, uint32_t size);
cmd_status_t calibrate_AnalogIn(uint8_t* cfg, uint32_t size);
void calibrate_Stop(void);
void calibrate_Feed(void);
void calibrate_ProcessResult(cmd_status_t status, circbuff_t* buff);

cmd_status_t calibrate_LoadCalibrationData(calib_result_t* data);
cmd_status_t calibrate_StoreCalibrationData(const calib_result_t * const data);
cmd_status_t calibrate_EraseCalibrationData(void);
const calib_result_t* calibrate_GetActiveCalibrationData(void);

void calibrate_GetFactorsForDAC(int ch, float* a, float* b);
#endif /* end __CALIBRATE_H */

