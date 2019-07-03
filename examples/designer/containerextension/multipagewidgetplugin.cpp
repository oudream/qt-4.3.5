/****************************************************************************
**
** Copyright (C) 2005-2008 Trolltech ASA. All rights reserved.
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

#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerContainerExtension>
#include <QtDesigner/QDesignerPropertySheetExtension>

#include <QIcon>
#include <QtPlugin>

#include "multipagewidget.h"
#include "multipagewidgetplugin.h"
#include "multipagewidgetextensionfactory.h"

MultiPageWidgetPlugin::MultiPageWidgetPlugin(QObject *parent)
    :QObject(parent)
{
    initialized = false;
}

QString MultiPageWidgetPlugin::name() const
{
    return QLatin1String("MultiPageWidget");
}

QString MultiPageWidgetPlugin::group() const
{
    return QLatin1String("Display Widgets [Examples]");
}

QString MultiPageWidgetPlugin::toolTip() const
{
    return QString();
}

QString MultiPageWidgetPlugin::whatsThis() const
{
    return QString();
}

QString MultiPageWidgetPlugin::includeFile() const
{
    return QLatin1String("multipagewidget.h");
}

QIcon MultiPageWidgetPlugin::icon() const
{
    return QIcon();
}

bool MultiPageWidgetPlugin::isContainer() const
{
    return true;
}

QWidget *MultiPageWidgetPlugin::createWidget(QWidget *parent)
{
    MultiPageWidget *widget = new MultiPageWidget(parent);
    connect(widget, SIGNAL(currentIndexChanged(int)),
            this, SLOT(currentIndexChanged(int)));
    connect(widget, SIGNAL(pageTitleChanged(const QString &)),
            this, SLOT(pageTitleChanged(const QString &)));
    return widget;
}

bool MultiPageWidgetPlugin::isInitialized() const
{
    return initialized;
}

void MultiPageWidgetPlugin::initialize(QDesignerFormEditorInterface *formEditor)
{
    if (initialized)
        return;

    QExtensionManager *manager = formEditor->extensionManager();
    QExtensionFactory *factory = new MultiPageWidgetExtensionFactory(manager);

    Q_ASSERT(manager != 0);
    manager->registerExtensions(factory, Q_TYPEID(QDesignerContainerExtension));

    initialized = true;
}

QString MultiPageWidgetPlugin::domXml() const
{
    return QLatin1String("\
    <widget class=\"MultiPageWidget\" name=\"multipagewidget\">\
        <widget class=\"QWidget\" name=\"page\" />\
    </widget>\
    ");
}

void MultiPageWidgetPlugin::currentIndexChanged(int index)
{
    Q_UNUSED(index);
    MultiPageWidget *widget = qobject_cast<MultiPageWidget*>(sender());
    if (widget) {
        QDesignerFormWindowInterface *form = QDesignerFormWindowInterface::findFormWindow(widget);
        if (form)
            form->emitSelectionChanged();
    }
}

void MultiPageWidgetPlugin::pageTitleChanged(const QString &title)
{
    Q_UNUSED(title);
    MultiPageWidget *widget = qobject_cast<MultiPageWidget*>(sender());
    if (widget) {
        QWidget *page = widget->widget(widget->currentIndex());
        QDesignerFormWindowInterface *form;
        form = QDesignerFormWindowInterface::findFormWindow(widget);
        if (form) {
            QDesignerFormEditorInterface *editor = form->core();
            QExtensionManager *manager = editor->extensionManager();
            QDesignerPropertySheetExtension *sheet;
            sheet = qt_extension<QDesignerPropertySheetExtension*>(manager, page);
            const int propertyIndex = sheet->indexOf(QLatin1String("windowTitle"));
            sheet->setChanged(propertyIndex, true);
        }
    }
}

Q_EXPORT_PLUGIN2(containerextension, MultiPageWidgetPlugin)
