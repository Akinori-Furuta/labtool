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

#include <cstddef>
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
    setupLabels();
    setTitle("Analog Measurements");

    mNumSignals = 0;
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
    QLabel *labelLevel;
    QLabel *labelDiff;
    QLabel *labelPk;

    (void)active;
    for (int i = 0; i < mNumSignals; i++) {
        labelLevel = mMeasureLevel[i];

        if (i < level.size()) {

            labelLevel->setText(QString("%1 V").arg(level.at(i)));
            labelLevel->setVisible(true);
            if ((i % 2) == 1) {
                double diff = level.at(i-1)-level.at(i);
                if (diff < 0) diff = -diff;
                labelDiff = mMeasureLevelDiff[i/2];
                labelDiff->setText(QString("%1 V").arg(diff));
                labelDiff->setVisible(true);
            }

        }
        else {
            labelLevel->setVisible(false);
            if ((i % 2) == 1) {
                labelDiff = mMeasureLevelDiff[i/2];
                labelDiff->setVisible(false);
            }
        }
    }

    for (int i = 0; i < mNumSignals; i++) {
        labelPk = mMeasurePk[i];
        if (i < pk.size()) {
            labelPk->setText(QString("%1 V").arg(pk.at(i)));
            labelPk->setVisible(true);
        }
        else {
            labelPk->setVisible(false);
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

#define ELEMENTS_OF(array) (sizeof(array) / sizeof((array)[0]))

/*!
    Create needed labels.
*/
void UiAnalogGroup::setupLabels()
{
    QLabel *level;
    QLabel *label;

    /* first, clean pointers to widgets. */
    for (size_t i = 0; i < ELEMENTS_OF(mMeasureLevelLbl); i++) {
        mMeasureLevelLbl[i] = NULL;
    }
    for (size_t i = 0; i < ELEMENTS_OF(mMeasureLevel); i++) {
        mMeasureLevel[i] = NULL;
    }
    for (size_t i = 0; i < ELEMENTS_OF(mMeasurePk); i++) {
        mMeasurePk[i] = NULL;
    }
    for (size_t i = 0; i < ELEMENTS_OF(mMeasureLevelDiffLbl); i++) {
        mMeasureLevelDiffLbl[i] = NULL;
    }
    for (size_t i = 0; i < ELEMENTS_OF(mMeasureLevelDiff); i++) {
        mMeasureLevelDiff[i] = NULL;
    }

    for (int i = 0; i < UiAnalogSignal::MaxNumSignals; i++) {
        // Level

        // Deallocation: "Qt Object trees" (See UiMainWindow)
        mMeasureLevelLbl[i] = label = new QLabel(this);
        label->setText(QString("A%1:").arg(i));
        label->setVisible(false);
        // Deallocation: "Qt Object trees" (See UiMainWindow)
        mMeasureLevel[i] = level = new QLabel(this);
        level->setText("");
        level->setVisible(false);

        // Peak-to-Peak

        // Deallocation: "Qt Object trees" (See UiMainWindow)
        mMeasurePkLbl[i] = label = new QLabel(this);
        label->setText(QString("Pk-Pk%1:").arg(i));
        label->setVisible(false);
        // Deallocation: "Qt Object trees" (See UiMainWindow)
        mMeasurePk[i] = level = new QLabel(this);
        level->setText("");
        level->setVisible(false);

        // Level Diff (only every other idx; 1, 3, ...)
        if ((i % 2) == 1) {

            // Deallocation: "Qt Object trees" (See UiMainWindow)
            mMeasureLevelDiffLbl[i/2] = label = new QLabel(this);
            label->setText(QString("|A%1-A%2|:").arg(i-1).arg(i));
            label->setVisible(false);
            // Deallocation: "Qt Object trees" (See UiMainWindow)
            mMeasureLevelDiff[i/2] = level = new QLabel(this);
            level->setText("");
            level->setVisible(false);

        }

    }

}

/*!
    Position all child widgets.
*/
void UiAnalogGroup::doLayout()
{
    QMargins boxMargins = contentsMargins();
    QSize savedMinSize = mMinSize;

    /* Note: Assume that label and level use same font. */
    QFontMetrics fm(mMeasureLevelLbl[0]->font());
    int wLabel = fm.width("|A5-A6|:");
    int wValueMax = 0;
    //
    //    make sure all labels are resized to their minimum size
    //

    for (int i = 0; i < mNumSignals; i++) {
        QLabel *label = mMeasureLevelLbl[i];
        QLabel *level = mMeasureLevel[i];
        QString text;
        text = (level ? level->text() : "W");
        int    wValue = fm.width(text);
        if (label) (label->resize(wLabel, fm.height()));
        if (level) (level->resize(wValue, fm.height()));
        wValueMax = qMax(wValueMax, wValue);

        label = mMeasurePkLbl[i];
        level = mMeasurePk[i];
        text = (level ? level->text() : "W");
        wValue = fm.width(text);

        if (label) (label->resize(wLabel, fm.height()));
        if (level) (level->resize(wValue, fm.height()));
        wValueMax = qMax(wValueMax, wValue);

        if ((i % 2) == 1) {

            label = mMeasureLevelDiffLbl[i/2];
            level = mMeasureLevelDiff[i/2];
            text = (level ? level->text() : "W");
            wValue = fm.width(text);
            if (label) (label->resize(wLabel, fm.height()));
            if (level) (level->resize(wValue, fm.height()));
            wValueMax = qMax(wValueMax, wValue);

        }

    }


    //
    //    position the labels
    //

    int yPos = MarginTop + boxMargins.top();
    int xPos = MarginLeft + boxMargins.left();
    int xPosRight = xPos + wLabel + HoriDistBetweenRelated;

    for (int i = 0; i < mNumSignals; i++) {
        QLabel *label = mMeasureLevelLbl[i];
        QLabel *level = mMeasureLevel[i];
        QString text;
        text = (level ? level->text() : "W");
        int    wValue = fm.width(text);

        if (label) (label->move(xPos, yPos));
        if (level) (level->move(xPosRight, yPos));
        wValueMax = qMax(wValueMax, wValue);

        yPos += fm.height() + VertDistBetweenRelated;

    }

    if (mNumSignals/2 > 0) {
        yPos += VertDistBetweenUnrelated;

        for (int i = 0; i < mNumSignals/2; i++) {
            QLabel *label = mMeasureLevelDiffLbl[i/2];
            QLabel *level = mMeasureLevelDiff[i/2];
            QString text;

            text = (level ? level->text() : "W");
            int    wValue = fm.width(text);

            if (label) (label->move(xPos, yPos));
            if (level) (level->move(xPosRight, yPos));
            wValueMax = qMax(wValueMax, wValue);

            yPos += fm.height() + VertDistBetweenRelated;

        }
    }


    yPos += VertDistBetweenUnrelated;

    for (int i = 0; i < mNumSignals; i++) {
        QLabel *label = mMeasurePkLbl[i];
        QLabel *level = mMeasurePk[i];
        QString text;
        text = (level ? level->text() : "W");
        int    wValue = fm.width(text);

        if (label) (label->move(xPos, yPos));
        if (level) (level->move(xPosRight, yPos));
        wValueMax = qMax(wValueMax, wValue);

        yPos += fm.height() + VertDistBetweenRelated;

    }
    int xSize = xPosRight + wValueMax + MarginRight + boxMargins.right();
    int ySize = yPos + MarginBottom + boxMargins.bottom();
    xSize = (xSize + 0xf) & (~0xf);
    ySize = (ySize + 0xf) & (~0xf);
    setMinimumSize(xSize, yPos);
    mMinSize = QSize(xSize, ySize);
    if (savedMinSize != mMinSize) {
        updateGeometry();
    }
}

void UiAnalogGroup::updateUi()
{
}
