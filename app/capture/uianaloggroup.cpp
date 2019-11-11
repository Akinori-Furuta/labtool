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

#include <QtGlobal>
#include "uianaloggroup.h"

/*!
    \class UiAnalogGroup
    \brief UI widget that show analog signal measurements.

    \ingroup Capture

*/


/*!
    Constructs an UiAnalogGroup with the given \a parent.
*/
UiAnalogGroup::UiAnalogGroup(QWidget *parent) :
    QGroupBox(parent),
    mMinSize(0, 0)
{
    setTitle("Analog Measurements");

    mNumSignals = 0;
    setupLabels();
}

/*!
    Sets the number of analog signals that are used by the application.
*/
void UiAnalogGroup::setNumSignals(int numSignals)
{
    if (numSignals > UiAnalogSignal::MaxNumSignals) return;

    mNumSignals = numSignals;

    // enable/disable labels
    for (int i = 0; i < UiAnalogSignal::MaxNumSignals; i++) {
        mMeasureLevelLbl[i]->setVisible((i<mNumSignals));
        mMeasureLevel[i]->setVisible((i<mNumSignals));
        mMeasurePkLbl[i]->setVisible((i<mNumSignals));
        mMeasurePk[i]->setVisible((i<mNumSignals));

        if ((i % 2) == 1) {
            mMeasureLevelDiffLbl[i/2]->setVisible((i<mNumSignals));
            mMeasureLevelDiff[i/2]->setVisible((i<mNumSignals));
        }

    }

    doLayout();
}

/*!
    Sets the latest measurement data. The parameter \a level contains the
    analog voltage level for each signal at current mouse cursor. The parameter
    \a pk contains peak-to-peak values for each signal. The parameter \a active
    indicates if the measurement is active or not.
*/
void UiAnalogGroup::setMeasurementData(QList<double>level, QList<double>pk,
                                       bool active)
{
    (void)active;
    for (int i = 0; i < mNumSignals; i++) {
        if (i < level.size()) {

            mMeasureLevel[i]->setText(QString("%1 V").arg(level.at(i)));
            if ((i % 2) == 1) {
                double diff = level.at(i-1)-level.at(i);
                if (diff < 0) diff = -diff;
                mMeasureLevelDiff[i/2]->setText(QString("%1 V").arg(diff));
            }

        }
        else {
            mMeasureLevel[i]->setText("");
            if ((i % 2) == 1) {
                mMeasureLevelDiff[i/2]->setText("");
            }
        }
    }

    for (int i = 0; i < mNumSignals; i++) {
        if (i < pk.size()) {
            mMeasurePk[i]->setText(QString("%1 V").arg(pk.at(i)));
        }
        else {
            mMeasurePk[i]->setText("");

        }
    }

    doLayout();
}

/*!
    This event handler is called when the widget is first made visible.
*/
void UiAnalogGroup::showEvent(QShowEvent* event)
{
    (void)event;
    doLayout();
}

void UiAnalogGroup::changeEvent(QEvent *event)
{
    doLayout();
    QGroupBox::changeEvent(event);
}

/*!
    Returns the minimum size of this widget.
*/
QSize UiAnalogGroup::minimumSizeHint() const
{
    return mMinSize;
}

/*!
    Returns the recommended size of this widget.
*/
QSize UiAnalogGroup::sizeHint() const
{
    return minimumSizeHint();
}

/*!
    Create needed labels.
*/
void UiAnalogGroup::setupLabels()
{
    for (int i = 0; i < UiAnalogSignal::MaxNumSignals; i++) {
        // Level

        // Deallocation: "Qt Object trees" (See UiMainWindow)
        mMeasureLevelLbl[i] = new QLabel(this);
        mMeasureLevelLbl[i]->setText(QString("A%1:").arg(i));
        mMeasureLevelLbl[i]->setVisible(false);
        // Deallocation: "Qt Object trees" (See UiMainWindow)
        mMeasureLevel[i] = new QLabel(this);
        mMeasureLevel[i]->setVisible(false);

        // Peak-to-Peak

        // Deallocation: "Qt Object trees" (See UiMainWindow)
        mMeasurePkLbl[i] = new QLabel(this);
        mMeasurePkLbl[i]->setText(QString("Pk-Pk%1:").arg(i));
        mMeasurePkLbl[i]->setVisible(false);
        // Deallocation: "Qt Object trees" (See UiMainWindow)
        mMeasurePk[i] = new QLabel(this);
        mMeasurePk[i]->setVisible(false);

        // Level Diff (only every other idx; 1, 3, ...)
        if ((i % 2) == 1) {

            // Deallocation: "Qt Object trees" (See UiMainWindow)
            mMeasureLevelDiffLbl[i/2] = new QLabel(this);
            mMeasureLevelDiffLbl[i/2]
                    ->setText(QString("|A%1-A%2|:").arg(i-1).arg(i));
            mMeasureLevelDiffLbl[i/2]->setVisible(false);
            // Deallocation: "Qt Object trees" (See UiMainWindow)
            mMeasureLevelDiff[i/2] = new QLabel(this);
            mMeasureLevelDiff[i/2]->setVisible(false);

        }

    }

}

/*!
    Position all child widgets.
*/
void UiAnalogGroup::doLayout()
{
    QMargins boxMargins = contentsMargins();

    /* Note: Assume that label and level use same font. */
    QFontMetrics fm(mMeasureLevelDiffLbl[0]->font());
    int wLabel = fm.width("|A5-A6|:");
    //
    //    make sure all labels are resized to their minimum size
    //

    for (int i = 0; i < mNumSignals; i++) {
        QLabel	*label = mMeasureLevelLbl[i];
        QLabel	*level = mMeasureLevel[i];

        label->resize(wLabel, fm.height());
        level->resize(fm.width(level->text()), fm.height());

        label = mMeasurePkLbl[i];
        level = mMeasurePk[i];

        label->resize(wLabel, fm.height());
        level->resize(fm.width(level->text()), fm.height());

        if ((i % 2) == 1) {

            label = mMeasureLevelDiffLbl[i/2];
            level = mMeasureLevelDiff[i/2];

            label->resize(wLabel, fm.height());
            level->resize(fm.width(level->text()), fm.height());

        }

    }


    //
    //    position the labels
    //

    int yPos = MarginTop + boxMargins.top();
    int xPos = MarginLeft + boxMargins.left();
    int xPosRight = xPos + wLabel + HoriDistBetweenRelated;

    for (int i = 0; i < mNumSignals; i++) {
        QLabel	*label = mMeasureLevelLbl[i];
        QLabel	*level = mMeasureLevel[i];

        label->move(xPos, yPos);
        level->move(xPosRight, yPos);

        yPos += fm.height() + VertDistBetweenRelated;

    }

    if (mNumSignals/2 > 0) {
        yPos += VertDistBetweenUnrelated;

        for (int i = 0; i < mNumSignals/2; i++) {
            QLabel *label = mMeasureLevelDiffLbl[i/2];
            QLabel *level = mMeasureLevelDiff[i/2];

            label->move(xPos, yPos);
            level->move(xPosRight, yPos);

            yPos += fm.height() + VertDistBetweenRelated;

        }
    }


    yPos += VertDistBetweenUnrelated;

    for (int i = 0; i < mNumSignals; i++) {
        QLabel	*label = mMeasurePkLbl[i];
        QLabel	*level = mMeasurePk[i];

        label->move(xPos, yPos);
        level->move(xPosRight, yPos);

        yPos += fm.height() + VertDistBetweenRelated;

    }


}
