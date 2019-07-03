/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#ifndef QPOLYGON_H
#define QPOLYGON_H

#include <QtCore/qvector.h>
#include <QtCore/qpoint.h>
#include <QtCore/qrect.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QMatrix;
class QTransform;
class QRect;
class QVariant;

class Q_GUI_EXPORT QPolygon : public QVector<QPoint>
{
public:
    inline QPolygon() {}
    inline ~QPolygon() {}
    inline QPolygon(int size);
    inline QPolygon(const QPolygon &a) : QVector<QPoint>(a) {}
    inline QPolygon(const QVector<QPoint> &v) : QVector<QPoint>(v) {}
    QPolygon(const QRect &r, bool closed=false);
    QPolygon(int nPoints, const int *points);
    operator QVariant() const;

    void translate(int dx, int dy);
    void translate(const QPoint &offset);
    QRect boundingRect() const;

    void point(int i, int *x, int *y) const;
    QPoint point(int i) const;
    void setPoint(int index, int x, int y);
    void setPoint(int index, const QPoint &p);
    void setPoints(int nPoints, const int *points);
    void setPoints(int nPoints, int firstx, int firsty, ...);
    void putPoints(int index, int nPoints, const int *points);
    void putPoints(int index, int nPoints, int firstx, int firsty, ...);
    void putPoints(int index, int nPoints, const QPolygon & from, int fromIndex=0);

    bool containsPoint(const QPoint &pt, Qt::FillRule fillRule) const;

    QPolygon united(const QPolygon &r) const;
    QPolygon intersected(const QPolygon &r) const;
    QPolygon subtracted(const QPolygon &r) const;
};

inline QPolygon::QPolygon(int asize) : QVector<QPoint>(asize) {}

#ifndef QT_NO_DEBUG_STREAM
Q_GUI_EXPORT QDebug operator<<(QDebug, const QPolygon &);
#endif

/*****************************************************************************
  Misc. QPolygon functions
 *****************************************************************************/

inline void QPolygon::setPoint(int index, const QPoint &pt)
{ (*this)[index] = pt; }

inline void QPolygon::setPoint(int index, int x, int y)
{ (*this)[index] = QPoint(x, y); }

inline QPoint QPolygon::point(int index) const
{ return at(index); }

inline void QPolygon::translate(const QPoint &offset)
{ translate(offset.x(), offset.y()); }

class QRectF;

class Q_GUI_EXPORT QPolygonF : public QVector<QPointF>
{
public:
    inline QPolygonF() {}
    inline ~QPolygonF() {}
    inline QPolygonF(int size);
    inline QPolygonF(const QPolygonF &a) : QVector<QPointF>(a) {}
    inline QPolygonF(const QVector<QPointF> &v) : QVector<QPointF>(v) {}
    QPolygonF(const QRectF &r);
    QPolygonF(const QPolygon &a);

    inline void translate(qreal dx, qreal dy);
    void translate(const QPointF &offset);

    QPolygon toPolygon() const;

    bool isClosed() const { return !isEmpty() && first() == last(); }

    QRectF boundingRect() const;

    bool containsPoint(const QPointF &pt, Qt::FillRule fillRule) const;

    QPolygonF united(const QPolygonF &r) const;
    QPolygonF intersected(const QPolygonF &r) const;
    QPolygonF subtracted(const QPolygonF &r) const;
};

inline QPolygonF::QPolygonF(int asize) : QVector<QPointF>(asize) {}

#ifndef QT_NO_DEBUG_STREAM
Q_GUI_EXPORT QDebug operator<<(QDebug, const QPolygonF &);
#endif

/*****************************************************************************
  QPolygonF stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &stream, const QPolygonF &array);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &stream, QPolygonF &array);
#endif

inline void QPolygonF::translate(qreal dx, qreal dy)
{ translate(QPointF(dx, dy)); }

QT_END_HEADER

#endif // QPOLYGON_H
