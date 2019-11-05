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
#include "uigrid.h"
#include "common/configuration.h"

#include <QPainter>
#include <QDebug>


/*!
    \class UiGrid
    \brief UI widget that paints the grid ontop of the plot.

    \ingroup Capture

*/


/*!
    Constructs an UiGrid with the given time axis \a axis
    and \a parent.
*/
UiGrid::UiGrid(UiTimeAxis *axis, QWidget *parent) :
    UiAbstractPlotItem(parent)
{
    mTimeAxis = axis;
}

/*!
    Paint event handler responsible for painting this widget.
*/
void UiGrid::paintEvent(QPaintEvent *event)
{
    (void)event;
    QPainter painter(this);
    QPen pen = painter.pen();
    int time_reference = UiTimeAxis::ReferenceMajorStep;

    painter.save();
    painter.translate(infoWidth(), 0);

    // draw grid for each main step
    int numMajorSteps = width() / UiTimeAxis::MajorStepPixelWidth + 1;

    pen.setColor(Configuration::instance().gridColor());
    pen.setStyle(Qt::DotLine);
    painter.setPen(pen);

    for (int i = 0; i < numMajorSteps; i++) {

        if (i == time_reference) {
            pen.setColor(Configuration::instance().gridColorHighLight());
            pen.setStyle(Qt::DashLine);
            painter.setPen(pen);
        } else {
            if (i == time_reference + 1) {
                pen.setColor(Configuration::instance().gridColor());
                pen.setStyle(Qt::DotLine);
                painter.setPen(pen);
            }
        }
        int xpos = i*UiTimeAxis::MajorStepPixelWidth;

        painter.drawLine(xpos, 2, xpos, height());

    }

    painter.restore();
}
