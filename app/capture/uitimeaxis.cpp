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
#include "uitimeaxis.h"

#include <QPainter>
#include <QDebug>

#include <qmath.h>

#include "common/stringutil.h"
#include "common/configuration.h"
#include "device/devicemanager.h"

/*!
    \class UiTimeAxis
    \brief UI widget that paints the time axis

    \ingroup Capture

    This class is also responsible for handling "time to pixel" and
    "pixel to time" conversions needed when painting signals.

*/



/*!
    \enum UiTimeAxis::Constants

    This enum describes integer constants used by this widget.

    \var UiTimeAxis::Constants UiTimeAxis::MajorStepPixelWidth
    Number of pixels between major steps

    \var UiTimeAxis::Constants UiTimeAxis::NumberOfMinorSteps
    Number of minor steps between major steps

    \var UiTimeAxis::Constants UiTimeAxis::ReferenceMajorStep
    Reference time starts at this major step

    \var UiTimeAxis::Constants UiTimeAxis::MinStepAsPowOf10
    Minimum step time as power of 10

    \var UiTimeAxis::Constants UiTimeAxis::MaxStepAsPowOf10
    Maximum step time as power of 10

    \var UiTimeAxis::Constants UiTimeAxis::MinRefTimeAsPowOf10
    Minimum Reference time as power of 10
*/


/*!
    Constructs an UiTimeAxis with the given \a parent.
*/
UiTimeAxis::UiTimeAxis(QWidget *parent) :
    UiAbstractPlotItem(parent)
{
    // the default reference time is 0
    mRefTime = 0.0;
    // 1 ms is the default time between major steps
    mMajorStepTime = 0.001;

    mRangeLower = 0;
    mRangeUpper = 1;


    // we don't want the background to be transparent since signals should
    // be put behind the time axis during vertical scroll
    setAutoFillBackground(true);
    doLayout();
}

/*!
    Returns the upper time value for the axis range.
*/
double UiTimeAxis::rangeUpper()
{
    return mRangeUpper;
}

/*!
    Returns the lower time value for the axis range.
*/
double UiTimeAxis::rangeLower()
{
    return mRangeLower;
}

/*!
    Handle Qt Change Event (ex. change appearance)
*/
void UiTimeAxis::changeEvent(QEvent *event)
{
    UiAbstractPlotItem::changeEvent(event);
    doLayout();
    updateRange();
}

/*!
    Returns the reference time. The time axis has one position as reference
    and other values are normally calulated relative to this reference.
*/
double UiTimeAxis::reference()
{
    return mRefTime;
}

/*!
    returns mejor step.
*/
double UiTimeAxis::majorStepTime()
{
    return mMajorStepTime;
}

/*!
    Sets the reference time/position.
*/
void  UiTimeAxis::setReference(double value)
{
    if (value < 0 && -value < qPow(10, MinRefTimeAsPowOf10)) {
        value = 0;
    }
    else if (value > 0 && value < qPow(10, MinRefTimeAsPowOf10)) {
        value = 0;
    }
    mRefTime = value;
    updateRange();
    update();
}

/*!
    Sets the major step time
*/

void UiTimeAxis::setMajorStepTime(double step)
{
    mMajorStepTime = step;
    updateRange();
    update();
}

/*!
    Returns the pixel position for given time \a value.
*/
double UiTimeAxis::timeToPixel(double value)
{
    return value*MajorStepPixelWidth/mMajorStepTime;
}

/*!
    Returns the time for at the given pixel postition \a value
*/
double UiTimeAxis::pixelToTime(double value)
{
    return value*mMajorStepTime/MajorStepPixelWidth;
}

/*!
    Returns the pixel position relative to the reference position for
    given time \a value
*/
double UiTimeAxis::timeToPixelRelativeRef(double value)
{
    return timeToPixel(value-mRangeLower) + infoWidth();
}

/*!
    Returns the time relative to the reference time for
    given pixel position (x-coordinate) \a xcoord
*/
double UiTimeAxis::pixelToTimeRelativeRef(double xcoord)
{
    // make xcoord relative to the plot area only
    xcoord -= infoWidth();

    return (xcoord*mMajorStepTime) / (MajorStepPixelWidth) + mRangeLower;
}

/*!
    Zoom by specified number of \a steps centered around the given
    x coordinate \a xCenter.
*/
void UiTimeAxis::zoom(int steps, double xCenter)
{
    double center = pixelToTimeRelativeRef(xCenter);
    double newValue = 0;

    double factor = 0.5;
    int majorUnitValue = closestUnitDigit(mMajorStepTime);
    if (steps < 0) {
        steps = -steps;
        factor = 2.0;
        if (majorUnitValue == 2)
            factor = 2.5;
    }
    else {
        factor = 0.5;
        if (majorUnitValue == 5)
            factor = 0.4;
    }
    factor = factor*steps;

    newValue = mMajorStepTime * factor;

    // lower and upper limit on the major step
    if (factor < 1 && newValue < qPow(10, MinStepAsPowOf10)) return;
    if (factor > 1 && newValue > qPow(10, MaxStepAsPowOf10)) return;

    // zoom around the center point
    mMajorStepTime = newValue;
    //mRefTime = center - (center-mRefTime)*factor;
    setReference(center - (center-mRefTime)*factor);

    updateRange();
}

/*!
    Zoom the plot until \a lowerTime and \a upperTime will be visible
    in the range.
*/
void UiTimeAxis::zoomAll(double lowerTime, double upperTime)
{
    double interval = upperTime - lowerTime;

    if (interval <= 0) return;

    setReference(mMajorStepTime);
    updateRange();
    while (upperTime < mRangeUpper) {
        zoom(1, 0);
        setReference(mMajorStepTime);
        updateRange();
    }

    while (upperTime > mRangeUpper) {
        zoom(-1, 0);
        setReference(mMajorStepTime);
        updateRange();
    }


    setReference(mMajorStepTime-(mRangeUpper-upperTime)/2);
    updateRange();

}

void UiTimeAxis::restoreAxis(double refTime, double majorTime, double lowerTime, double upperTime)
{
    if (lowerTime > upperTime) {
        double t;
        t = upperTime;
        lowerTime = upperTime;
        lowerTime = t;
    }
    int plotWidth = width() - mInfoWidth;
    double window = upperTime - lowerTime;
    double majorTicks = plotWidth / MajorStepPixelWidth;

    if (window < majorTime) {
        majorTime = window / majorTicks;
        double l10MajorTime = qFloor(qLn(majorTime) / qLn(10.0));
        double alignedMajorTime = qPow(10.0, l10MajorTime);
        double x5 = alignedMajorTime * 5.0;
        if (x5 < majorTime) {
            majorTime = x5;
        } else {
            double x2 = alignedMajorTime * 2.0;
            if (x2 < majorTime) {
                majorTime = x2;
            } else {
                majorTime = alignedMajorTime;
            }
        }
    }
    if ((lowerTime > refTime) || (refTime > upperTime)) {
        /* Range doesn't contain reference time */
        /* do as update range. */

        lowerTime = mRefTime - ReferenceMajorStep*majorTime;
        upperTime = lowerTime
            + (majorTime*plotWidth)/MajorStepPixelWidth;
    }
    mRefTime = refTime;
    mMajorStepTime = majorTime;
    mRangeLower = lowerTime;
    mRangeUpper = upperTime;
    update();
}

void UiTimeAxis::restoreAxis(double refTime, double majorTime)
{
    mRefTime = refTime;
    mMajorStepTime = majorTime;
    updateRange();
    update();
}

static const char projectGroupThis[] = "timeAxis";
static const char projectKeyRefTime[] = "refTime";
static const char projectKeyMajorStepTime[] = "majorStepTime";

void UiTimeAxis::saveProject(QSettings &project)
{
    project.beginGroup(projectGroupThis);
    project.setValue(projectKeyRefTime, mRefTime);
    project.setValue(projectKeyMajorStepTime, mMajorStepTime);
    project.endGroup();
}

void UiTimeAxis::openProject(QSettings &project)
{
    double refTime;
    double majorTime;

    project.beginGroup(projectGroupThis);
    refTime = project.value(projectKeyRefTime, mRefTime).toDouble();
    majorTime = project.value(projectKeyMajorStepTime, mMajorStepTime).toDouble();
    project.endGroup();
    restoreAxis(refTime, majorTime);
}

/*!
    Move the time axis \a differenceInPixels number of pixels
*/
void UiTimeAxis::moveAxis(int differenceInPixels)
{

    setReference(mRefTime +
               (differenceInPixels*(mMajorStepTime/MajorStepPixelWidth)));


    updateRange();
    update();
}

/*!
    Paint event handler responsible for painting this widget.
*/
void UiTimeAxis::paintEvent(QPaintEvent *event)
{
    (void)event;
    QPainter painter(this);

    const int plotWidth = width()-infoWidth();
    const int plotSteps = MajorStepPixelWidth/NumberOfMinorSteps;
    const int numMinorSteps = plotWidth / plotSteps + 1;

    painter.save();
    QPen pen = painter.pen();
    pen.setColor(Configuration::instance().textColor());
    painter.setPen(pen);
    painter.translate(infoWidth(), 0);

    const int fontHeight = painter.fontMetrics().height();

    for (int i = 0; i < numMinorSteps; i++) {
        int stepHeight = MinorTickHeight;
        int xpos = plotSteps * i;
        if (/*i > 0 &&*/ (i % NumberOfMinorSteps) == 0) {
            stepHeight = MajorTickHeight;

            QString stepText = getTimeLabelForStep(i/NumberOfMinorSteps);

            // draw text centered over a major step
            int textWidth = painter.fontMetrics().width(stepText);
            painter.drawText(xpos - textWidth/2, fontHeight, stepText);

        }

        // draw minor/major step on the time axis
        painter.drawLine(xpos, height() - stepHeight,
                         xpos, height());
    }

    painter.restore();
}

/*!
    Paint event handler responsible for painting this widget.
*/
void UiTimeAxis::resizeEvent(QResizeEvent* event)
{
    (void)event;
    doLayout();
    updateRange();
}

/*!
    Called when the info width has changed.
*/
void UiTimeAxis::infoWidthChanged()
{
    doLayout();
    updateRange();
}

int UiTimeAxis::estimateHeight()
{
    QFontMetrics fm(parentWidget()->font());
    int fontHeight = fm.height();
    int hTimeScale = fontHeight + TimeTickSpace + MajorTickHeight;
    return hTimeScale;
}

/*!
    Layout Time value and ticks.
*/
void UiTimeAxis::doLayout()
{
    int hTimeScale = estimateHeight();
    resize(width(), hTimeScale);
    setMinimumHeight(hTimeScale);
    setMaximumHeight(hTimeScale);
    updateGeometry();
}

/*!
    Update the range based on widget width.
*/
void UiTimeAxis::updateRange()
{
    int plotWidth = width() - mInfoWidth;

    mRangeLower = mRefTime - ReferenceMajorStep*mMajorStepTime;
    mRangeUpper = mRangeLower
            + mMajorStepTime*plotWidth/MajorStepPixelWidth;
}

/*!
    Get time label for given step \a majorStep.
*/
QString UiTimeAxis::getTimeLabelForStep(int majorStep)
{
    double t = mMajorStepTime*(majorStep-ReferenceMajorStep);

    // get time relative to trigger
    CaptureDevice * device = DeviceManager::instance().activeDevice()
            ->captureDevice();
    double triggerTime = (double)device->digitalTriggerIndex()
            / device->usedSampleRate();
    t -= (triggerTime-mRefTime);

    QString result = StringUtil::timeInSecToString(t);

    if (t > 0) {
        result.prepend("+");
    }

    return result;
}

/*!
    Returns closest unit value for given double \a value.
    Example: 0.0021 -> 2, 30.076 -> 3
*/
int UiTimeAxis::closestUnitDigit(double value)
{

    while(value < 1) {
        value *= 10;
    }

    while(value > 10) {
        value /= 10;
    }

    return (int) value;
}


