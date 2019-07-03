/****************************************************************************
**
** Copyright (C) 2004-2008 Trolltech ASA. All rights reserved.
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

#include <QtNetwork>

#include "fortunethread.h"

FortuneThread::FortuneThread(QObject *parent)
    : QThread(parent), quit(false)
{
}

FortuneThread::~FortuneThread()
{
    quit = true;
    cond.wakeOne();
    wait();
}

void FortuneThread::requestNewFortune(const QString &hostName, quint16 port)
{
    QMutexLocker locker(&mutex);
    this->hostName = hostName;
    this->port = port;
    if (!isRunning())
        start();
    else
        cond.wakeOne();
}

void FortuneThread::run()
{
    mutex.lock();
    QString serverName = hostName;
    quint16 serverPort = port;
    mutex.unlock();

    while (!quit) {
        const int Timeout = 5 * 1000;

        QTcpSocket socket;
        socket.connectToHost(serverName, serverPort);

        if (!socket.waitForConnected(Timeout)) {
            emit error(socket.error(), socket.errorString());
            return;
        }

        while (socket.bytesAvailable() < (int)sizeof(quint16)) {
            if (!socket.waitForReadyRead(Timeout)) {
                emit error(socket.error(), socket.errorString());
                return;
            }
        }

        quint16 blockSize;
        QDataStream in(&socket);
        in.setVersion(QDataStream::Qt_4_0);
        in >> blockSize;

        while (socket.bytesAvailable() < blockSize) {
            if (!socket.waitForReadyRead(Timeout)) {
                emit error(socket.error(), socket.errorString());
                return;
            }
        }

        QMutexLocker locker(&mutex);

        QString fortune;
        in >> fortune;
        emit newFortune(fortune);

        cond.wait(&mutex);
        serverName = hostName;
        serverPort = port;
    }
}
