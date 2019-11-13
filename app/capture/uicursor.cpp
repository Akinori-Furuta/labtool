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
#include "uicursor.h"

#include <limits>
#include <QtGlobal>
#include <QDebug>
#include <QPainter>
#include <QPainterPath>
#include <qmath.h>

#include "common/configuration.h"
#include "device/devicemanager.h"


/*!
    \class UiCursor
    \brief UI area responsible for drawing cursors

    \ingroup Capture

    Cursors are used when analyzing captured signals to decide, for example,
    time distance between certain points in the signal. A capture is typically
    stopped/finished whan a trigger condition occurs. The trigger position is
    visualized using a cursor.
*/


/*!
    \enum UiCursor::Constants

    This enum describes constants used in UiCursor.

    \var UiCursor::Constants UiCursor::CursorBarHeight
    Height of the cursor bar where cursor symbols are painted.
*/

/*!
    \enum UiCursor::CursorId

    This enum defines the valid cursor IDs to use when accessing a cursor.

    \var UiCursor::CursorId UiCursor::Trigger
    The ID of the trigger cursor

    \var UiCursor::CursorId UiCursor::Cursor1
    The ID of the trigger 1

    \var UiCursor::CursorId UiCursor::Cursor2
    The ID of the trigger 2

    \var UiCursor::CursorId UiCursor::Cursor3
    The ID of the trigger 3

    \var UiCursor::CursorId UiCursor::Cursor4
    The ID of the trigger 4

    \var UiCursor::CursorId UiCursor::NoCursor
    This ID indicates that no cursor is selected
*/


/*!
    Constructs the UiCursor with the given \a parent. The signal manager
    given by \a signalManager is used to keep track of signal widgets. The
    time axis as specified by \a axis is needed as a pixel to time reference
    when drawing cursors.
*/
UiCursor::UiCursor(SignalManager *signalManager, UiTimeAxis *axis, QWidget *parent) :
    UiAbstractPlotItem(parent)
{
    /*

        Event Propagation Problem
        -------------------------

        I'm not getting mouse events to propagate to siblings in the widget
        stack and is therefore turning off reception of mouse events.

        The cursor widget is on top of all other widgets and I would assume
        that an event could propagate down the stack to e.g. a signal widget
        (which is underneath the cursor widget), but since I'm not getting
        this to work, reception is turned off and instead functions
        (mousePressed, mouseReleased, mouseMoved) have been added that are
        explicitly called from UiPlot (the parent).

     */
    setAttribute(Qt::WA_TransparentForMouseEvents);

    mSignalManager = signalManager;
    mTimeAxis = axis;


    for (int i = 0; i < NumCursors; i++) {
        mCursorOn[i] = false;

        mCursor[i] = i*0.0005*3;
    }

    //mCursorOn[0] = true;
    mCursorDrag = NoCursor;
    mPressXPos = -1;
    mPressYPos = -1;
    mCursorLabelWidth = CursorClickBand;

    mMinWidthSet = false;

    // default to begin with. Updated in paintEvent
    setMinimumInfoWidth(50);
}

/*!
    Mouse press event. Selects a cursor to move, enable, or disable.
*/
bool UiCursor::mousePressed(Qt::MouseButton button, const QPoint &pos)
{
    if (pos.x() < infoWidth()) return false;

    UiCursor::CursorId cursor = findCursor(pos);

    if (cursor != NoCursor && button == Qt::LeftButton) {
        mCursorDrag = cursor;
        mPressXPos = pos.x();
        mPressYPos = pos.y();
        return true;
    }

    return false;
}

/*!
    Mouse release event.
*/
bool UiCursor::mouseReleased(Qt::MouseButton button, const QPoint &pos)
{
    mCursorDrag = NoCursor;

    int diffX = abs(mPressXPos - pos.x());
    int diffY = abs(mPressYPos - pos.y());
    UiCursor::CursorId cursor = findCursor(pos);

    int cursorX = calcCursorXPosition(cursor);

    // clicked (not dragged...)
    if (diffX < 2 && diffY < 2 && cursor != NoCursor && button == Qt::LeftButton) {

        //
        //    Within viewing area. Enable/disable a cursor (except trigger)
        //

        if (cursorX >= infoWidth() && cursorX < width()) {
            if (cursor != Trigger) {
                mCursorOn[cursor] = !mCursorOn[cursor];

                emit cursorChanged(cursor, mCursorOn[cursor], mCursor[cursor]);

                update();
            }
        }

        //
        //    Outside of viewing area. Move plot to cursor in case
        //    the cursor is enabled.
        //

        else if (mCursorOn[cursor]){
            mTimeAxis->setReference(mCursor[cursor]);
            update();
        }

    }


    return false;
}

/*!
    Mouse move event.
*/
bool UiCursor::mouseMoved(Qt::MouseButton, const QPoint &pos)
{

    // not allowed to move Trigger cursor by mouse
    if(mCursorDrag != NoCursor && mCursorDrag != Trigger) {
        double t = mTimeAxis->pixelToTimeRelativeRef(pos.x());

        // snap to signal transition
        if (mCursorOn[mCursorDrag] && t >= 0) {

            double transTime = mSignalManager->closestDigitalTransition(t);

            if (transTime != -1) {
                double pxDiff = mTimeAxis->timeToPixel(t) - mTimeAxis->timeToPixel(transTime);
                pxDiff = qAbs(pxDiff);

                if (pxDiff < 6) {
                    t = transTime;
                }
            }
        }

        mCursor[mCursorDrag] = t;

        if (mCursorOn[mCursorDrag]) {
            emit cursorChanged(mCursorDrag, true, mCursor[mCursorDrag]);
        }

        update();
        return true;
    }


    return false;
}

/*!
    Set the trigger cursor to time position given by \a t.
*/
void UiCursor::setTrigger(double t)
{
    mCursor[Trigger] = t;
    mCursorOn[Trigger] = true;
}

/*!
   Get the time position for the cursor given by \a id.
*/
double UiCursor::cursorPosition(UiCursor::CursorId id)
{
    if (id >= UiCursor::NumCursors) return 0;

    return mCursor[id];
}

/*!
    Set the time position \a t for the cursor given by \a id.
*/
void UiCursor::setCursorPosition(UiCursor::CursorId id, double t)
{
    if (id >= UiCursor::NumCursors) return;

    mCursor[id] = t;
    emit cursorChanged(id, mCursorOn[id], mCursor[id]);
}

/*!
    Returns true if the cursor with ID \a id is enabled; otherwise false
    is returned.
*/
bool UiCursor::isCursorOn(UiCursor::CursorId id)
{
    if (id >= UiCursor::NumCursors) return false;

    return mCursorOn[id];
}

/*!
    Set the enabled state of cursor \a id to \a enable.
*/
void UiCursor::enableCursor(UiCursor::CursorId id, bool enable)
{
    if (id >= UiCursor::NumCursors) return;

    mCursorOn[id] = enable;
    emit cursorChanged(id, mCursorOn[id], mCursor[id]);
}

/*!
    Returns a map with cursor IDs and cursor names. Only enabled cursors are
    returned.
*/
QMap<UiCursor::CursorId, QString> UiCursor::activeCursors()
{
    QMap<UiCursor::CursorId, QString> map;

    for (int i = 0; i < NumCursors; i++) {

        if (!mCursorOn[i]) continue;

        if (i == Trigger) {
            map.insert((UiCursor::CursorId)i, "Trigger");
        }
        else {
            map.insert((UiCursor::CursorId)i, QString("C%1").arg(i));
        }
    }

    return map;
}

/*!
    \fn void UiCursor::cursorChanged(UiCursor::CursorId, bool, double)
    This signal is emitted when a cursor has been changed (moved, enabled,
    or disabled)
*/

int UiCursor::estimateCursorBarHeight()
{
    QFontMetrics fm(this->font());
    int textHeight = fm.height();
    int result = CursorHeight + CursorLabelSpace + textHeight + CursorBottomSpace;
    if (result < CursorBarMinHeight) {
        result = CursorBarMinHeight;
    }
    return result;
}

/*!
    The paint event handler requesting to repaint this widget.
*/
void UiCursor::paintEvent(QPaintEvent *event)
{
    (void)event;
    QPainter painter(this);
    int hBarHeight = estimateCursorBarHeight();
    int barStart = height() - hBarHeight;


    //
    //    Paint cursor bar background
    //
    painter.fillRect(0, barStart, width(), hBarHeight,
                     Configuration::instance().outsidePlotColor());


    //
    //    Paint cursor bar label
    //
    QString lbl = QString("Cursors");
    // is this the correct way to check the height of the painted character?
    int textHeight = painter.fontMetrics().height();

    if (!mMinWidthSet) {
        int textWidth = painter.fontMetrics().width(lbl);
        setMinimumInfoWidth(10+textWidth+10);
        mMinWidthSet = true;
    }

    painter.save();
    QPen pen = painter.pen();
    pen.setColor(Qt::darkGray);
    painter.setPen(pen);
    painter.drawText(10, barStart + (hBarHeight + textHeight) / 2, lbl);
    painter.restore();


    /*
        Paint cursors
     */
    paintCursors(&painter);

}

/*!
    Paint the cursor symbol for the cursor with ID \a cursorId
*/
void UiCursor::paintCursorSymbol(QPainter* painter, int cursorId)
{


    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    QPen pen = painter->pen();
    pen.setColor(Configuration::instance().cursorColor(cursorId));
    painter->setPen(pen);


    int cursorX = calcCursorXPosition(cursorId);

    //
    //    Create a path for a triangle which points upwards
    //

    QPainterPath path;
    path.moveTo(0, 0);
    path.lineTo((CursorWidth/2), CursorHeight);
    path.lineTo(-(CursorWidth/2), CursorHeight);
    path.lineTo(0, 0);



    //    Position the cursor symbol (triangle) in the cursor bar. When the
    //    cursor is visible in the viewing area the point of the triangle is
    //    pointing upwards.
    //
    //    If the cursor is outside of the viewing area the point of the triangle
    //    is pointing in the direction (left or right) of where it is located
    //    and the triangle is positioned at the outer edges of the viewing area.


    int yCursorPosition = calcCursorYPosition(cursorId);
    // cursor is to the left of the viewing area
    if (cursorX < infoWidth()) {
        painter->translate(infoWidth()+1, yCursorPosition);
        painter->rotate(-90);
    }

    // cursor is to the right of the viewing area
    else if (cursorX > width()) {
        painter->translate(width()-1, yCursorPosition);
        painter->rotate(90);
    }

    // cursor is within the viewing area
    else {
        painter->translate(cursorX, yCursorPosition);
    }

    painter->drawPath(path);

    // if the cursor is on -> fill the triangle
    if (mCursorOn[cursorId]) {
        painter->fillPath(path, Configuration::instance().cursorColor(cursorId));
    }
    mCursorLabelWidth = CursorClickBand;
    // cursor name/ID
    if (cursorX >= infoWidth() && cursorX < width()) {
        QString cNum = QString("C%1").arg(cursorId);
        if (cursorId == Trigger) {
            cNum = QString("T");
        }
        int textWidth = painter->fontMetrics().width(cNum);
        mCursorLabelWidth = qMax(mCursorLabelWidth, textWidth);

        // is this the correct way to check the height of the painted character?
        int textHeight = painter->fontMetrics().height();

        painter->drawText(-textWidth/2, CursorHeight + CursorLabelSpace + textHeight - CursorBottomSpace, cNum);
    }

    painter->restore();
}

/*!
    Paint all the cursors
*/
void UiCursor::paintCursors(QPainter* painter)
{
    int hBarHeight = estimateCursorBarHeight();
    int hLine = height() - hBarHeight - 1;

    for (int i = 0; i < NumCursors; i++) {

        int cursorX = calcCursorXPosition(i);
        if (mCursorOn[i] && cursorX >= infoWidth() && cursorX < width()) {


            painter->save();


            QPen pen = painter->pen();
            pen.setColor(Configuration::instance().cursorColor(i));
            painter->setPen(pen);

            painter->setRenderHint(QPainter::Antialiasing);
            painter->drawLine(cursorX, 0,cursorX, hLine);

            painter->restore();

        }

        paintCursorSymbol(painter, i);

    }
}

/*!
    Find cursor at given position \pos.
*/
UiCursor::CursorId UiCursor::findCursor(const QPoint &pos)
{
    double time = mTimeAxis->pixelToTimeRelativeRef(pos.x());
    double diff = mTimeAxis->pixelToTimeRelativeRef(pos.x() + mCursorLabelWidth / 2)-time;
    QPainter painter(this);
    int hBarHeight = estimateCursorBarHeight();

    double diff_min = std::numeric_limits<double>::max();

    if (diff < 0) diff = -diff;

    UiCursor::CursorId cursor = NoCursor;

    // Choose nearlest cursor from pointer position.
    // if two or more cursors are nearlest,
    // choose more higher number of cursor.
    for (int i = NumCursors-1; i >= 0; i--) {
        double x1 = time - diff;
        double x2 = time + diff;
        double tCursor = mCursor[i];

        if ((x1 <= tCursor) && (tCursor <= x2)) {

            if ((mCursorOn[i]) ||
                ((pos.y() >= (height() - hBarHeight)) && (pos.y() < height()))) {
                /* "Cursor is on" or "Pointer on CursorBar" */
                double dt;

                dt = tCursor - x1;
                if (dt < diff_min) {
                    diff_min = dt;
                    cursor = (UiCursor::CursorId)i;
                }

                dt = x2 - tCursor;
                if (dt < diff_min) {
                    diff_min = dt;
                    cursor = (UiCursor::CursorId)i;
                }
            }
        }
    }

    //
    //    See if we are trying to move a cursor which is outside of the
    //    view area. This will only happen if we click within the cursor bar.
    //

    if (cursor == NoCursor &&
            pos.y() >= (height() - hBarHeight)
            && pos.y() < height())
    {
        UiCursor::CursorId outCursor = NoCursor;

        double distance = 123456789;

        for (int i = 0; i < NumCursors; i++) {
            int cursorX = calcCursorXPosition(i);

            // NOT outside the view area
            if (cursorX >= infoWidth() && cursorX < width()) continue;

            int xPos = infoWidth()+CursorHeight/2;
            if (cursorX > width()) {
                xPos = width()-CursorHeight/2;
            }

            QPoint mid(xPos, calcCursorYPosition(i));


            // calculate distance between press point and middle of cursor symbol
            double dist = qSqrt( qPow(pos.x()-mid.x(), 2) + qPow(pos.y()-mid.y(), 2) );
            if (dist < distance) {
                distance = dist;
                outCursor = (UiCursor::CursorId)i;
            }

        }


        if (outCursor != NoCursor) {
            cursor = outCursor;
        }

    }

    return cursor;
}

/*!
    Calculate the y position for a cursor with ID \a cursorId.
*/
int UiCursor::calcCursorYPosition(int cursorId)
{
    int hBar = estimateCursorBarHeight();
    int barStart = height() - hBar;
    int barOff = 2;
    int cursorX = calcCursorXPosition(cursorId);

    int result = barStart + barOff;

    if (cursorX < infoWidth() || cursorX > width()) {
        int hCursorHalf = CursorWidth / 2;
        double dyCursor = hBar - barOff - hCursorHalf;
        dyCursor /= NumCursors;
        if (dyCursor <= 0) {
            dyCursor = 1;
        }
        result += hCursorHalf + cursorId * dyCursor;
    }

    return result;
}

/*!
    Calculate the x position for a cursor with ID \a cursorId.
*/
int UiCursor::calcCursorXPosition(int cursorId)
{
    return mTimeAxis->timeToPixelRelativeRef(mCursor[cursorId]);
}


