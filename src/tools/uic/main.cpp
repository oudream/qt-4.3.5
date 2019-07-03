/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the tools applications of the Qt Toolkit.
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

#include "uic.h"
#include "option.h"
#include "driver.h"
#include "../../corelib/global/qconfig.cpp"
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QTextCodec>

static const char *error = 0;

void showHelp(const char *appName)
{
    fprintf(stderr, "Qt User Interface Compiler version %s\n", QT_VERSION_STR);
    if (error)
        fprintf(stderr, "%s: %s\n", appName, error);

    fprintf(stderr, "Usage: %s [options] <uifile>\n\n"
            "  -h, -help                 display this help and exit\n"
            "  -v, -version              display version\n"
            "  -d, -dependencies         display the dependencies\n"
            "  -o <file>                 place the output into <file>\n"
            "  -tr <func>                use func() for i18n\n"
            "  -p, -no-protection        disable header protection\n"
            "  -n, -no-implicit-includes disable generation of #include-directives\n"
            "                            for forms generated by uic3\n"
            "  -g <name>                 change generator\n"
            "\n", appName);
}

int main(int argc, char *argv[])
{
    Driver driver;

    const char *fileName = 0;

    int arg = 1;
    while (arg < argc) {
        QString opt = QString::fromLocal8Bit(argv[arg]);
        if (opt == QLatin1String("-h") || opt == QLatin1String("-help")) {
            showHelp(argv[0]);
            return 0;
        } else if (opt == QLatin1String("-d") || opt == QLatin1String("-dependencies")) {
            driver.option().dependencies = true;
        } else if (opt == QLatin1String("-v") || opt == QLatin1String("-version")) {
            fprintf(stderr, "Qt User Interface Compiler version %s\n", QT_VERSION_STR);
            return 0;
        } else if (opt == QLatin1String("-o") || opt == QLatin1String("-output")) {
            ++arg;
            if (!argv[arg]) {
                showHelp(argv[0]);
                return 1;
            }
            driver.option().outputFile = QFile::decodeName(argv[arg]);
        } else if (opt == QLatin1String("-p") || opt == QLatin1String("-no-protection")) {
            driver.option().headerProtection = false;
        } else if (opt == QLatin1String("-n") || opt == QLatin1String("-no-implicit-includes")) {
            driver.option().implicitIncludes = false;
        } else if (opt == QLatin1String("-postfix")) {
            ++arg;
            if (!argv[arg]) {
                showHelp(argv[0]);
                return 1;
            }
            driver.option().postfix = QLatin1String(argv[arg]);
        } else if (opt == QLatin1String("-3")) {
            ++arg;
            if (!argv[arg]) {
                showHelp(argv[0]);
                return 1;
            }
            driver.option().uic3 = QFile::decodeName(argv[arg]);
        } else if (opt == QLatin1String("-tr") || opt == QLatin1String("-translate")) {
            ++arg;
            if (!argv[arg]) {
                showHelp(argv[0]);
                return 1;
            }
            driver.option().translateFunction = QLatin1String(argv[arg]);
        } else if (opt == QLatin1String("-g") || opt == QLatin1String("-generator")) {
            ++arg;
            if (!argv[arg]) {
                showHelp(argv[0]);
                return 1;
            }
            QString name = QString::fromLocal8Bit(argv[arg]).toLower ();
            driver.option().generator = (name == QLatin1String ("java")) ? Option::JavaGenerator : Option::CppGenerator;
        } else if (!fileName) {
            fileName = argv[arg];
        } else {
            showHelp(argv[0]);
            return 1;
        }

        ++arg;
    }

    // report Qt usage for commercial customers with a "metered license" (currently experimental)
#if QT_EDITION != QT_EDITION_OPENSOURCE
#ifdef QT_CONFIGURE_BINARIES_PATH
    const char *binariesPath = QT_CONFIGURE_BINARIES_PATH;
    QString reporterPath = QString::fromLocal8Bit(binariesPath) + QDir::separator()
                           + QLatin1String("qtusagereporter");
#if defined(Q_OS_WIN)
    reporterPath += QLatin1String(".exe");
#endif
    if (QFile::exists(reporterPath))
        system(qPrintable(reporterPath + QLatin1String(" uic")));
#endif
#endif

    QString inputFile;
    if (fileName)
        inputFile = QString::fromLocal8Bit(fileName);
    else
        driver.option().headerProtection = false;

    if (driver.option().dependencies) {
        return !driver.printDependencies(inputFile);
    }

    QTextStream *out = 0;
    QFile f;
    if (driver.option().outputFile.size()) {
        f.setFileName(driver.option().outputFile);
        if (!f.open(QIODevice::WriteOnly | QFile::Text)) {
            fprintf(stderr, "Could not create output file\n");
            return 1;
        }
        out = new QTextStream(&f);
        out->setCodec(QTextCodec::codecForName("UTF-8"));
    }

    bool rtn = driver.uic(inputFile, out);
    delete out;

    if (!rtn) {
        if (driver.option().outputFile.size()) {
            f.close();
            f.remove();
        }
        fprintf(stderr, "File '%s' is not valid\n", inputFile.isEmpty() ? "<stdin>" : inputFile.toLocal8Bit().constData());
    }

    return !rtn;
}
