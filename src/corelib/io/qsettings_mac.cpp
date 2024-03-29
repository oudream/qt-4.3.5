/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the QtCore module of the Qt Toolkit.
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

#include "qsettings.h"
#include "qsettings_p.h"
#include "qdatetime.h"
#include "qdir.h"
#include "qvarlengtharray.h"
#include "private/qcore_mac_p.h"

static const CFStringRef hostNames[2] = { kCFPreferencesCurrentHost, kCFPreferencesAnyHost };
static const int numHostNames = 2;

/*
    On the Mac, it is more natural to use '.' as the key separator
    than '/'. Therefore, it makes sense to replace '/' with '.' in
    keys. Then we replace '.' with middle dots (which we can't show
    here) and middle dots with '/'. A key like "4.0/BrowserCommand"
    becomes "4<middot>0.BrowserCommand".
*/

enum RotateShift { Macify = 1, Qtify = 2 };

static QString rotateSlashesDotsAndMiddots(const QString &key, int shift)
{
    static const int NumKnights = 3;
    static char knightsOfTheRoundTable[NumKnights] = { '/', '.', '\xb7' };
    QString result = key;

    for (int i = 0; i < result.size(); ++i) {
        for (int j = 0; j < NumKnights; ++j) {
            if (result.at(i) == QLatin1Char(knightsOfTheRoundTable[j])) {
                result[i] = QLatin1Char(knightsOfTheRoundTable[(j + shift) % NumKnights]).unicode();
                break;
            }
        }
    }
    return result;
}

static QCFType<CFStringRef> macKey(const QString &key)
{
    return QCFString::toCFStringRef(rotateSlashesDotsAndMiddots(key, Macify));
}

static QString qtKey(CFStringRef cfkey)
{
    return rotateSlashesDotsAndMiddots(QCFString::toQString(cfkey), Qtify);
}

static QCFType<CFPropertyListRef> macValue(const QVariant &value);

static CFArrayRef macList(const QList<QVariant> &list)
{
    int n = list.size();
    QVarLengthArray<QCFType<CFPropertyListRef> > cfvalues(n);
    for (int i = 0; i < n; ++i)
        cfvalues[i] = macValue(list.at(i));
    return CFArrayCreate(kCFAllocatorDefault, reinterpret_cast<const void **>(cfvalues.data()),
                         CFIndex(n), &kCFTypeArrayCallBacks);
}

static QCFType<CFPropertyListRef> macValue(const QVariant &value)
{
    CFPropertyListRef result = 0;

    switch (value.type()) {
    case QVariant::ByteArray:
        {
            QByteArray ba = value.toByteArray();
            result = CFDataCreate(kCFAllocatorDefault, reinterpret_cast<const UInt8 *>(ba.data()),
                                  CFIndex(ba.size()));
        }
        break;
    case QVariant::List:
    case QVariant::StringList:
    case QVariant::Polygon:
        result = macList(value.toList());
        break;
    case QVariant::Map:
        {
            /*
                QMap<QString, QVariant> is potentially a multimap,
                whereas CFDictionary is a single-valued map. To allow
                for multiple values with the same key, we store
                each CFDictonary value as a CFArray.
            */
            QMap<QString, QVariant> map = value.toMap();
            QMap<QString, QVariant>::const_iterator i = map.constBegin();

            int maxUniqueKeys = map.size();
            int numUniqueKeys = 0;
            QVarLengthArray<QCFType<CFPropertyListRef> > cfkeys(maxUniqueKeys);
            QVarLengthArray<QCFType<CFPropertyListRef> > cfvalues(maxUniqueKeys);

            while (i != map.constEnd()) {
                const QString &key = i.key();
                QList<QVariant> values;

                do {
                    values << i.value();
                    ++i;
                } while (i != map.constEnd() && i.key() == key);

                cfkeys[numUniqueKeys] = QCFString::toCFStringRef(key);
                cfvalues[numUniqueKeys] = macList(values);
                ++numUniqueKeys;
            }

            result = CFDictionaryCreate(kCFAllocatorDefault,
                                        reinterpret_cast<const void **>(cfkeys.data()),
                                        reinterpret_cast<const void **>(cfvalues.data()),
                                        CFIndex(numUniqueKeys),
                                        &kCFTypeDictionaryKeyCallBacks,
                                        &kCFTypeDictionaryValueCallBacks);
        }
        break;
    case QVariant::DateTime:
        {
            /*
                CFDate, unlike QDateTime, doesn't store timezone information.
            */
            QDateTime dt = value.toDateTime();
            if (dt.timeSpec() == Qt::LocalTime) {
                QDateTime reference;
                reference.setTime_t((uint)kCFAbsoluteTimeIntervalSince1970);
                result = CFDateCreate(kCFAllocatorDefault, CFAbsoluteTime(reference.secsTo(dt)));
            } else {
                goto string_case;
            }
        }
        break;
    case QVariant::Bool:
        result = value.toBool() ? kCFBooleanTrue : kCFBooleanFalse;
        break;
    case QVariant::Int:
    case QVariant::UInt:
        {
            int n = value.toInt();
            result = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &n);
        }
        break;
    case QVariant::Double:
        {
            double n = value.toDouble();
            result = CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &n);
        }
        break;
    case QVariant::LongLong:
    case QVariant::ULongLong:
        {
            qint64 n = value.toLongLong();
            result = CFNumberCreate(0, kCFNumberLongLongType, &n);
        }
        break;
    case QVariant::String:
    string_case:
    default:
        result = QCFString::toCFStringRef(QSettingsPrivate::variantToString(value));
    }
    return result;
}

static QVariant qtValue(CFPropertyListRef cfvalue)
{
    if (!cfvalue)
        return QVariant();

    CFTypeID typeId = CFGetTypeID(cfvalue);

    /*
        Sorted grossly from most to least frequent type.
    */
    if (typeId == CFStringGetTypeID()) {
        return QSettingsPrivate::stringToVariant(QCFString::toQString(static_cast<CFStringRef>(cfvalue)));
    } else if (typeId == CFNumberGetTypeID()) {
        CFNumberRef cfnumber = static_cast<CFNumberRef>(cfvalue);
        if (CFNumberIsFloatType(cfnumber)) {
            double d;
            CFNumberGetValue(cfnumber, kCFNumberDoubleType, &d);
            return d;
        } else {
            int i;
            qint64 ll;

            if (CFNumberGetValue(cfnumber, kCFNumberIntType, &i))
                return i;
            CFNumberGetValue(cfnumber, kCFNumberLongLongType, &ll);
            return ll;
        }
    } else if (typeId == CFArrayGetTypeID()) {
        CFArrayRef cfarray = static_cast<CFArrayRef>(cfvalue);
        QList<QVariant> list;
        CFIndex size = CFArrayGetCount(cfarray);
        bool metNonString = false;
        for (CFIndex i = 0; i < size; ++i) {
            QVariant value = qtValue(CFArrayGetValueAtIndex(cfarray, i));
            if (value.type() != QVariant::String)
                metNonString = true;
            list << value;
        }
        if (metNonString)
            return list;
        else
            return QVariant(list).toStringList();
    } else if (typeId == CFBooleanGetTypeID()) {
        return (bool)CFBooleanGetValue(static_cast<CFBooleanRef>(cfvalue));
    } else if (typeId == CFDataGetTypeID()) {
        CFDataRef cfdata = static_cast<CFDataRef>(cfvalue);
        return QByteArray(reinterpret_cast<const char *>(CFDataGetBytePtr(cfdata)),
                          CFDataGetLength(cfdata));
    } else if (typeId == CFDictionaryGetTypeID()) {
        CFDictionaryRef cfdict = static_cast<CFDictionaryRef>(cfvalue);
        CFTypeID arrayTypeId = CFArrayGetTypeID();
        int size = (int)CFDictionaryGetCount(cfdict);
        QVarLengthArray<CFPropertyListRef> keys(size);
        QVarLengthArray<CFPropertyListRef> values(size);
        CFDictionaryGetKeysAndValues(cfdict, keys.data(), values.data());

        QMultiMap<QString, QVariant> map;
        for (int i = 0; i < size; ++i) {
            QString key = QCFString::toQString(static_cast<CFStringRef>(keys[i]));

            if (CFGetTypeID(values[i]) == arrayTypeId) {
                CFArrayRef cfarray = static_cast<CFArrayRef>(values[i]);
                CFIndex arraySize = CFArrayGetCount(cfarray);
                for (CFIndex j = arraySize - 1; j >= 0; --j)
                    map.insert(key, qtValue(CFArrayGetValueAtIndex(cfarray, j)));
            } else {
                map.insert(key, qtValue(values[i]));
            }
        }
        return map;
    } else if (typeId == CFDateGetTypeID()) {
        QDateTime dt;
        dt.setTime_t((uint)kCFAbsoluteTimeIntervalSince1970);
        return dt.addSecs((int)CFDateGetAbsoluteTime(static_cast<CFDateRef>(cfvalue)));
    }
    return QVariant();
}

static QString comify(const QString &organization)
{
    for (int i = organization.size() - 1; i >= 0; --i) {
        QChar ch = organization.at(i);
        if (ch == QLatin1Char('.') || ch == QChar(0x3002) || ch == QChar(0xff0e)
                || ch == QChar(0xff61)) {
            QString suffix = organization.mid(i + 1).toLower();
            if (suffix.size() == 2 || suffix == QLatin1String("com")
                    || suffix == QLatin1String("org") || suffix == QLatin1String("net")
                    || suffix == QLatin1String("edu") || suffix == QLatin1String("gov")
                    || suffix == QLatin1String("mil") || suffix == QLatin1String("biz")
                    || suffix == QLatin1String("info") || suffix == QLatin1String("name")
                    || suffix == QLatin1String("pro") || suffix == QLatin1String("aero")
                    || suffix == QLatin1String("coop") || suffix == QLatin1String("museum")) {
                QString result = organization;
                result.replace(QLatin1Char('/'), QLatin1Char(' '));
                return result;
            }
            break;
        }
        int uc = ch.unicode();
        if ((uc < 'a' || uc > 'z') && (uc < 'A' || uc > 'Z'))
            break;
    }

    QString domain;
    for (int i = 0; i < organization.size(); ++i) {
        QChar ch = organization.at(i);
        int uc = ch.unicode();
        if ((uc >= 'a' && uc <= 'z') || (uc >= '0' && uc <= '9')) {
            domain += ch;
        } else if (uc >= 'A' && uc <= 'Z') {
            domain += ch.toLower();
        } else {
           domain += QLatin1Char(' ');
        }
    }
    domain = domain.simplified();
    domain.replace(QLatin1Char(' '), QLatin1Char('-'));
    if (!domain.isEmpty())
        domain.append(QLatin1String(".com"));
    return domain;
}

class QMacSettingsPrivate : public QSettingsPrivate
{
public:
    QMacSettingsPrivate(QSettings::Scope scope, const QString &organization,
                        const QString &application);
    ~QMacSettingsPrivate();

    void remove(const QString &key);
    void set(const QString &key, const QVariant &value);
    bool get(const QString &key, QVariant *value) const;
    QStringList children(const QString &prefix, ChildSpec spec) const;
    void clear();
    void sync();
    void flush();
    bool isWritable() const;
    QString fileName() const;

private:
    struct SearchDomain
    {
        CFStringRef userName;
        CFStringRef applicationOrSuiteId;
    };

    QCFString applicationId;
    QCFString suiteId;
    QCFString hostName;
    SearchDomain domains[6];
    int numDomains;
};

QMacSettingsPrivate::QMacSettingsPrivate(QSettings::Scope scope, const QString &organization,
                                         const QString &application)
{
    QString javaPackageName;
    int curPos = 0;
    int nextDot;

    QString domainName = comify(organization);
    if (domainName.isEmpty()) {
        setStatus(QSettings::AccessError);
        domainName = QLatin1String("unknown-organization.trolltech.com");
    }

    while ((nextDot = domainName.indexOf(QLatin1Char('.'), curPos)) != -1) {
        javaPackageName.prepend(domainName.mid(curPos, nextDot - curPos));
        javaPackageName.prepend(QLatin1Char('.'));
        curPos = nextDot + 1;
    }
    javaPackageName.prepend(domainName.mid(curPos));
    javaPackageName = javaPackageName.toLower();
    if (curPos == 0)
        javaPackageName.prepend(QLatin1String("com."));
    suiteId = javaPackageName;

    if (scope == QSettings::SystemScope)
        spec |= F_System;

    if (application.isEmpty()) {
        spec |= F_Organization;
    } else {
        javaPackageName += QLatin1Char('.');
        javaPackageName += application;
        applicationId = javaPackageName;
    }

    numDomains = 0;
    for (int i = (spec & F_System) ? 1 : 0; i < 2; ++i) {
        for (int j = (spec & F_Organization) ? 1 : 0; j < 3; ++j) {
            SearchDomain &domain = domains[numDomains++];
            domain.userName = (i == 0) ? kCFPreferencesCurrentUser : kCFPreferencesAnyUser;
            if (j == 0)
                domain.applicationOrSuiteId = applicationId;
            else if (j == 1)
                domain.applicationOrSuiteId = suiteId;
            else
                domain.applicationOrSuiteId = kCFPreferencesAnyApplication;
        }
    }

    hostName = (scope == QSettings::SystemScope) ? kCFPreferencesCurrentHost : kCFPreferencesAnyHost;
    sync();
}

QMacSettingsPrivate::~QMacSettingsPrivate()
{
}

void QMacSettingsPrivate::remove(const QString &key)
{
    QStringList keys = children(key + QLatin1Char('/'), AllKeys);

    // If i == -1, then delete "key" itself.
    for (int i = -1; i < keys.size(); ++i) {
        QString subKey = key;
        if (i >= 0) {
            subKey += QLatin1Char('/');
            subKey += keys.at(i);
        }
        CFPreferencesSetValue(macKey(subKey), 0, domains[0].applicationOrSuiteId,
                              domains[0].userName, hostName);
    }
}

void QMacSettingsPrivate::set(const QString &key, const QVariant &value)
{
    CFPreferencesSetValue(macKey(key), macValue(value), domains[0].applicationOrSuiteId,
                          domains[0].userName, hostName);
}

bool QMacSettingsPrivate::get(const QString &key, QVariant *value) const
{
    QCFString k = macKey(key);
    for (int i = 0; i < numDomains; ++i) {
        for (int j = 0; j < numHostNames; ++j) {
            QCFType<CFPropertyListRef> ret =
                    CFPreferencesCopyValue(k, domains[i].applicationOrSuiteId, domains[i].userName,
                                           hostNames[j]);
            if (ret) {
                if (value)
                    *value = qtValue(ret);
                return true;
            }
        }

        if (!fallbacks)
            break;
    }
    return false;
}

QStringList QMacSettingsPrivate::children(const QString &prefix, ChildSpec spec) const
{
    QMap<QString, QString> result;
    int startPos = prefix.size();

    for (int i = 0; i < numDomains; ++i) {
        for (int j = 0; j < numHostNames; ++j) {
            QCFType<CFArrayRef> cfarray = CFPreferencesCopyKeyList(domains[i].applicationOrSuiteId,
                                                                   domains[i].userName,
                                                                   hostNames[j]);
            if (cfarray) {
                CFIndex size = CFArrayGetCount(cfarray);
                for (CFIndex k = 0; k < size; ++k) {
                    QString currentKey =
                            qtKey(static_cast<CFStringRef>(CFArrayGetValueAtIndex(cfarray, k)));
                    if (currentKey.startsWith(prefix))
                        processChild(currentKey.mid(startPos), spec, result);
                }
            }
        }

        if (!fallbacks)
            break;
    }
    return result.keys();
}

void QMacSettingsPrivate::clear()
{
    QCFType<CFArrayRef> cfarray = CFPreferencesCopyKeyList(domains[0].applicationOrSuiteId,
                                                           domains[0].userName, hostName);
    CFPreferencesSetMultiple(0, cfarray, domains[0].applicationOrSuiteId, domains[0].userName,
                             hostName);
}

void QMacSettingsPrivate::sync()
{
    for (int i = 0; i < numDomains; ++i) {
        for (int j = 0; j < numHostNames; ++j) {
            Boolean ok = CFPreferencesSynchronize(domains[i].applicationOrSuiteId,
                                                  domains[i].userName, hostNames[j]);
            if (!ok)
                setStatus(QSettings::AccessError);
        }
    }
}

void QMacSettingsPrivate::flush()
{
    sync();
}

bool QMacSettingsPrivate::isWritable() const
{
    QMacSettingsPrivate *that = const_cast<QMacSettingsPrivate *>(this);
    QString impossibleKey(QLatin1String("qt_internal/"));

    QSettings::Status oldStatus = that->status;
    that->status = QSettings::NoError;

    that->set(impossibleKey, QVariant());
    that->sync();
    bool writable = (status == QSettings::NoError) && that->get(impossibleKey, 0);
    that->remove(impossibleKey);
    that->sync();

    that->status = oldStatus;
    return writable;
}

QString QMacSettingsPrivate::fileName() const
{
    QString result;
    if ((spec & F_System) == 0)
        result = QDir::homePath();
    result += QLatin1String("/Library/Preferences/");
    result += QCFString::toQString(domains[0].applicationOrSuiteId);
    result += QLatin1String(".plist");
    return result;
}

QSettingsPrivate *QSettingsPrivate::create(QSettings::Format format,
                                           QSettings::Scope scope,
                                           const QString &organization,
                                           const QString &application)
{
    if (format == QSettings::NativeFormat) {
        return new QMacSettingsPrivate(scope, organization, application);
    } else {
        return new QConfFileSettingsPrivate(format, scope, organization, application);
    }
}

static QCFType<CFURLRef> urlFromFileName(const QString &fileName)
{
    return CFURLCreateWithFileSystemPath(kCFAllocatorDefault, QCFString(fileName),
                                         kCFURLPOSIXPathStyle, false);
}

bool QConfFileSettingsPrivate::readPlistFile(const QString &fileName, ParsedSettingsMap *map) const
{
    QCFType<CFDataRef> resource;
    SInt32 code;
    if (!CFURLCreateDataAndPropertiesFromResource(kCFAllocatorDefault, urlFromFileName(fileName),
                                                  &resource, 0, 0, &code))
        return false;

    QCFString errorStr;
    QCFType<CFPropertyListRef> propertyList =
            CFPropertyListCreateFromXMLData(kCFAllocatorDefault, resource, kCFPropertyListImmutable,
                                            &errorStr);

    if (!propertyList)
        return true;
    if (CFGetTypeID(propertyList) != CFDictionaryGetTypeID())
        return false;

    CFDictionaryRef cfdict =
            static_cast<CFDictionaryRef>(static_cast<CFPropertyListRef>(propertyList));
    int size = (int)CFDictionaryGetCount(cfdict);
    QVarLengthArray<CFPropertyListRef> keys(size);
    QVarLengthArray<CFPropertyListRef> values(size);
    CFDictionaryGetKeysAndValues(cfdict, keys.data(), values.data());

    for (int i = 0; i < size; ++i) {
        QString key = qtKey(static_cast<CFStringRef>(keys[i]));
        map->insert(QSettingsKey(key, Qt::CaseSensitive), qtValue(values[i]));
    }
    return true;
}

bool QConfFileSettingsPrivate::writePlistFile(const QString &fileName,
                                              const ParsedSettingsMap &map) const
{
    QVarLengthArray<QCFType<CFStringRef> > cfkeys(map.size());
    QVarLengthArray<QCFType<CFPropertyListRef> > cfvalues(map.size());
    int i = 0;
    ParsedSettingsMap::const_iterator j;
    for (j = map.constBegin(); j != map.constEnd(); ++j) {
        cfkeys[i] = macKey(j.key());
        cfvalues[i] = macValue(j.value());
        ++i;
    }

    QCFType<CFDictionaryRef> propertyList =
            CFDictionaryCreate(kCFAllocatorDefault,
                               reinterpret_cast<const void **>(cfkeys.data()),
                               reinterpret_cast<const void **>(cfvalues.data()),
                               CFIndex(map.size()),
                               &kCFTypeDictionaryKeyCallBacks,
                               &kCFTypeDictionaryValueCallBacks);

    QCFType<CFDataRef> xmlData = CFPropertyListCreateXMLData(kCFAllocatorDefault, propertyList);

    SInt32 code;
    return CFURLWriteDataAndPropertiesToResource(urlFromFileName(fileName), xmlData, 0, &code);
}
