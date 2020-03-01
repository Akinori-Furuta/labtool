/*
 *  Copyright 2020 Akinori Furuta
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
#ifndef LABTOOLDEVICESPEC_H
#define LABTOOLDEVICESPEC_H

#include <QObject>

#include "device/device.h"

/*! LabTool USB Device specification
 *  This class doesn't create any instance.
 *  Provides specifications by constant.
 */
class LabToolDeviceSpec : public QObject
{
    Q_OBJECT
public:
    enum DeviceSpec {
        ANALOG_IN_CHANNELS = 2,
        ANALOG_IN_RANGES = 8,
        ANALOG_IN_CAL_LOW = 0,
        ANALOG_IN_CAL_MIDDLE = 1,
        ANALOG_IN_CAL_HIGH = 2,
        ANALOG_IN_CAL_NUMS = 3,
        SPI_DAC_BITS = 10,
        SPI_DAC_MAX = (0x1 << SPI_DAC_BITS) - 1,
        SPI_DAC_MIN = 0,
        SPI_DAC_MASK = (0x1 << SPI_DAC_BITS) - 1,
    };

     LabToolDeviceSpec(QObject *parent) {(void)parent;};
    ~LabToolDeviceSpec() {};
    static int spiDacClipValue(int val) {
        if (val < SPI_DAC_MIN) {
            return SPI_DAC_MIN;
        }
        if (val > SPI_DAC_MAX) {
            return SPI_DAC_MAX;
        }
        return val;
    }
};

#endif // LABTOOLDEVICEDPEC_H
