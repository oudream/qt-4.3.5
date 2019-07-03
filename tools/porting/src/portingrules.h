/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the qt3to4 porting application of the Qt Toolkit.
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

#ifndef PORTINGRULES_H
#define PORTINGRULES_H

#include "qtsimplexml.h"
#include "tokenreplacements.h"
#include <QList>
#include <QPair>
#include <QHash>
#include <QSet>
#include <QStringList>

class RuleDescription
{
public:
    explicit RuleDescription(QtSimpleXml &replacementRule) {
        qt3 = replacementRule[QLatin1String("Qt3")].text();
        qt4 = replacementRule[QLatin1String("Qt4")].text();
        ruleType = replacementRule.attribute(QLatin1String("Type"));
    }
    QString qt3;
    QString qt4;
    QString ruleType;
    bool operator==(const RuleDescription &other) const
    {
        return (qt3 == other.qt3 && qt4 == other.qt4 && ruleType == other.ruleType);
    }
};

class PortingRules
{
public:
    static void createInstance(QString xmlFilePath);
    static PortingRules *instance();
    static void deleteInstance();

    enum QtVersion{Qt3, Qt4};
    PortingRules(QString xmlFilePath);
    QList<TokenReplacement*> getTokenReplacementRules();
    QStringList getHeaderList(QtVersion qtVersion);
    QHash<QByteArray, QByteArray> getNeededHeaders();
    QStringList getInheritsQt();
    QHash<QByteArray, QByteArray> getClassLibraryList();
    QHash<QByteArray, QByteArray> getHeaderReplacements();
private:
    static PortingRules *theInstance;

    QList<TokenReplacement*> tokenRules;
    QStringList qt3Headers;
    QStringList qt4Headers;
    QHash<QByteArray, QByteArray> neededHeaders;
    QStringList inheritsQtClass;
    QList<RuleDescription> disabledRules;
    QHash<QByteArray, QByteArray> classLibraryList;
    QHash<QByteArray, QByteArray> headerReplacements;


    void parseXml(const QString fileName);
    void checkScopeAddRule(/*const */QtSimpleXml &currentRule);
    QtSimpleXml *loadXml(const QString fileName) const ;
    QString resolveFileName(const QString currentFileName,
                            const QString includeFileName) const;
    bool isReplacementRule(const QString ruleType) const;
    void disableRule(QtSimpleXml &replacementRule);
    bool isRuleDisabled(QtSimpleXml &replacementRule) const;
    void addLogWarning(const QString text) const;
    void addLogError(const QString text) const;
};

#endif // PORTINGRULES_H
