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

#include "qscreenahigl_qws.h"

#include <QScreenDriverPlugin>
#include <QStringList>

class QAhiGLScreenPlugin : public QScreenDriverPlugin
{
public:
    QAhiGLScreenPlugin();

    QStringList keys() const;
    QScreen *create(const QString&, int displayId);
};

/*!
  \class QAhiGLScreenPlugin
  \brief The QAhiGLScreenPlugin class is the plugin for the ATI handheld device graphics driver.

  QAhiGLScreenPlugin inherits QScreenDriverPlugin. See
  \l{How to Create Qt Plugins} for details.
 */

/*!
  This is the default constructor.
 */
QAhiGLScreenPlugin::QAhiGLScreenPlugin()
    : QScreenDriverPlugin()
{
}

/*!
  Returns a string list containing the string "ahigl" which
  is the only screen driver supported by this plugin.
 */
QStringList QAhiGLScreenPlugin::keys() const
{
    return (QStringList() << "ahigl");
}

/*!
  Creates a screen driver of the kind specified by \a driver.
  The \a displayId identifies the Qtopia Core server to connect to.
 */
QScreen* QAhiGLScreenPlugin::create(const QString& driver, int displayId)
{
    if (driver.toLower() != "ahigl")
        return 0;

    return new QAhiGLScreen(displayId);
}

Q_EXPORT_PLUGIN2(qahiglscreen, QAhiGLScreenPlugin)
