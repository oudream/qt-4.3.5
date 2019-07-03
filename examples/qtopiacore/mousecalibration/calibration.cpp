/****************************************************************************
**
** Copyright (C) 2006-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the example classes of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License versions 2.0 or 3.0 as published by the Free Software
** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file.  Alternatively you may (at
** your option) use any later version of the GNU General Public
** License if such license has been publicly approved by Trolltech ASA
** (or its successors, if any) and the KDE Free Qt Foundation. In
** addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.2, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** Please review the following information to ensure GNU General
** Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/. If
** you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech, as the sole
** copyright holder for Qt Designer, grants users of the Qt/Eclipse
** Integration plug-in the right for the Qt/Eclipse Integration to
** link to functionality provided by Qt Designer and its related
** libraries.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not expressly
** granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "calibration.h"

#include <QWSPointerCalibrationData>
#include <QPainter>
#include <QFile>
#include <QTimer>
#include <QApplication>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QScreen>
#include <QWSServer>

Calibration::Calibration()
{
    QRect desktop = QApplication::desktop()->geometry();
    desktop.moveTo(QPoint(0, 0));
    setGeometry(desktop);

    setFocusPolicy(Qt::StrongFocus);
    setFocus();
    setModal(true);

    int width = qt_screen->deviceWidth();
    int height = qt_screen->deviceHeight();

    int dx = width / 10;
    int dy = height / 10;

    QPoint *points = data.screenPoints;
    points[QWSPointerCalibrationData::TopLeft] = QPoint(dx, dy);
    points[QWSPointerCalibrationData::BottomLeft] = QPoint(dx, height - dy);
    points[QWSPointerCalibrationData::BottomRight] = QPoint(width - dx, height - dy);
    points[QWSPointerCalibrationData::TopRight] = QPoint(width - dx, dy);
    points[QWSPointerCalibrationData::Center] = QPoint(width / 2, height / 2);

    pressCount = 0;
}

Calibration::~Calibration()
{
}

int Calibration::exec()
{
    QWSServer::mouseHandler()->clearCalibration();
    grabMouse();
    activateWindow();
    int ret = QDialog::exec();
    releaseMouse();
    return ret;
}

void Calibration::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.fillRect(rect(), Qt::white);

    QPoint point = data.screenPoints[pressCount];

    // Map to logical coordinates in case the screen is transformed
    QSize screenSize(qt_screen->deviceWidth(), qt_screen->deviceHeight());
    point = qt_screen->mapFromDevice(point, screenSize);

    p.fillRect(point.x() - 6, point.y() - 1, 13, 3, Qt::black);
    p.fillRect(point.x() - 1, point.y() - 6, 3, 13, Qt::black);
}

void Calibration::mouseReleaseEvent(QMouseEvent *event)
{
    // Map from device coordinates in case the screen is transformed
    QSize screenSize(qt_screen->width(), qt_screen->height());
    QPoint p = qt_screen->mapToDevice(event->pos(), screenSize);

    data.devPoints[pressCount] = p;

    if (++pressCount < 5)
        repaint();
    else
        accept();
}

void Calibration::accept()
{
    Q_ASSERT(pressCount == 5);
    QWSServer::mouseHandler()->calibrate(&data);
    QDialog::accept();
}

