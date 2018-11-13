/***************************************************************************
 *   Copyright 2018 Andreas Wendler                                        *
 *   Robotics Erlangen e.V.                                                *
 *   http://www.robotics-erlangen.de/                                      *
 *   info@robotics-erlangen.de                                             *
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   any later version.                                                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifndef LOGOPENER_H
#define LOGOPENER_H

#include <QObject>
#include <QList>
#include <QMap>

#include "protobuf/status.h"

// Disable for now in order to be able to compile
/*class QMenu;
class QAction;
class StatusSource;
namespace Ui {
    class MainWindow;
}

// see this class as more of a mixin for mainwindow for opening logfiles
// the MainWindow class has to provide a logmanager, an open file menu (with a actionLogCutter action)
// and an open file button btnOpen
class LogOpener : public QObject
{
    Q_OBJECT
public:
    explicit LogOpener(Ui::MainWindow *ui, QObject *parent = nullptr);
    void close();

signals:
    void logOpened();

public slots:
    void handleStatus(const Status &status);
    void openFile(const QString &filename);
    void openFile();
    void goToLastFilePosition();

private:
    void makeRecentFileMenu();

private:
    Ui::MainWindow *ui;

    std::shared_ptr<StatusSource> m_logFile;
    QString m_openFileName;

    QList<QString> m_recentFiles;
    QMap<QString, uint> m_lastFilePositions;
    uint m_packetsSinceOpened;
    QMenu *m_recentFilesMenu;
    QAction *m_recentFilesMenuAction;

    const int MAX_RECENT_FILE_COUNT = 10;
};*/

#endif // LOGOPENER_H
