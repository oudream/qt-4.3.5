/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
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

#include "svgalibscreen.h"
#include "svgalibsurface.h"

#include <QVector>
#include <QApplication>
#include <QColor>
#include <QWidget>

bool SvgalibScreen::connect(const QString &displaySpec)
{
    Q_UNUSED(displaySpec);

    int mode = vga_getdefaultmode();
    if (mode <= 0) {
        qCritical("SvgalibScreen::connect(): invalid vga mode");
        return false;
    }

    vga_modeinfo *modeinfo = vga_getmodeinfo(mode);

    QScreen::lstep = modeinfo->linewidth;
    QScreen::dw = QScreen::w = modeinfo->width;
    QScreen::dh = QScreen::h = modeinfo->height;
    QScreen::d = modeinfo->bytesperpixel * 8;
    QScreen::size = QScreen::lstep * dh;
    QScreen::data = 0;

    switch (depth()) {
    case 32:
        setPixelFormat(QImage::Format_ARGB32_Premultiplied);
        break;
    case 16:
        setPixelFormat(QImage::Format_RGB16);
        break;
    default:
        break;
    }

    const int dpi = 72;
    QScreen::physWidth = qRound(QScreen::dw * 25.4 / dpi);
    QScreen::physHeight = qRound(QScreen::dh * 25.4 / dpi);

    return true;
}

bool SvgalibScreen::initDevice()
{
    if (vga_init() != 0) {
        qCritical("SvgalibScreen::initDevice(): unable to initialize svgalib");
        return false;
    }

    int mode = vga_getdefaultmode();
    if (vga_setmode(mode) == -1) {
        qCritical("SvgalibScreen::initialize(): unable to set graphics mode");
        return false;
    }

    if (gl_setcontextvga(mode) != 0) {
        qCritical("SvgalibScreen::initDevice(): unable to set vga context");
        return false;
    }
    context = gl_allocatecontext();
    gl_getcontext(context);

    vga_modeinfo *modeinfo = vga_getmodeinfo(mode);
    if (modeinfo->flags & IS_LINEAR)
        QScreen::data = vga_getgraphmem();

    QScreenCursor::initSoftwareCursor();
    return true;
}

void SvgalibScreen::shutdownDevice()
{
    gl_freecontext(context);
    vga_setmode(TEXT);
}

void SvgalibScreen::disconnect()
{
}

void SvgalibScreen::exposeRegion(QRegion region, int changing)
{
    QScreen::exposeRegion(region, changing);
    // flip between buffers if implementing a double buffered driver
}

void SvgalibScreen::solidFill(const QColor &color, const QRegion &reg)
{
    if (depth() != 32 && depth() != 16) {
        if (base())
            QScreen::solidFill(color, reg);
        return;
    }

    const QVector<QRect> rects = (reg & region()).rects();
    for (int i = 0; i < rects.size(); ++i) {
        const QRect r = rects.at(i);
        gl_fillbox(r.left(), r.top(), r.width(), r.height(), color.rgba());
    }
}

void SvgalibScreen::blit(const QImage &img, const QPoint &topLeft,
                         const QRegion &reg)
{
    if (img.format() != pixelFormat()) {
        if (base())
            QScreen::blit(img, topLeft, reg);
        return;
    }

    const QVector<QRect> rects = (reg & region()).rects();

    for (int i = 0; i < rects.size(); ++i) {
        const QRect r = rects.at(i);
        gl_putboxpart(r.x(), r.y(), r.width(), r.height(),
                      img.width(), img.height(),
                      static_cast<void*>(const_cast<uchar*>(img.bits())),
                      r.x() - topLeft.x(), r.y() - topLeft.y());
    }
}

QWSWindowSurface* SvgalibScreen::createSurface(QWidget *widget) const
{
    if (base()) {
        static int onScreenPaint = -1;
        if (onScreenPaint == -1)
            onScreenPaint = qgetenv("QT_ONSCREEN_PAINT").toInt();

        if (onScreenPaint > 0 || widget->testAttribute(Qt::WA_PaintOnScreen))
            return new SvgalibSurface(widget);
    }
    return QScreen::createSurface(widget);
}

QWSWindowSurface* SvgalibScreen::createSurface(const QString &key) const
{
    if (key == QLatin1String("svgalib"))
        return new SvgalibSurface;
    return QScreen::createSurface(key);
}
