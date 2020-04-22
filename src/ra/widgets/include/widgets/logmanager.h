/***************************************************************************
 *   Copyright 2017 Andreas Wendler                                        *
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

#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QWidget>
#include <QQueue>
#include <QTimer>
#include <memory>
#include "protobuf/status.h"
#include "core/timer.h"
#include "logfile/statussource.h"

class Timer;

namespace Ui {
class LogManager;
}

class TimedStatusSource;

namespace LogManagerInternal {
class SignalSource;
}

class LogManager : public QWidget
{
    Q_OBJECT

public:
    explicit LogManager(QWidget *parent = 0);
    ~LogManager();
    LogManager(const LogManager&) = delete;
    LogManager& operator=(const LogManager&) = delete;
    void setStatusSource(std::shared_ptr<StatusSource> source);
    void goToEnd();
    void setPaused(bool p);
    int getLastFrame();
    uint getFrame();

public slots:
    void seekPacket(int packet);

signals:
    void gotStatus(const Status &status);
    void disableSkipping(bool disable);
    void resetBacklog();
    void setSpeed(int speed);
    void stepBackward();
    void stepForward();
    void togglePaused();

private slots:
    void seekFrame(int frame);
    void previousFrame();
    void nextFrame();
    void handleStatus(const Status &status);

private:
    QString formatTime(qint64 time);
    void resetVariables();
    void initializeLabels(int64_t packetCount = 0);
    void connectStatusSource();

private:
    Ui::LogManager *ui;

    QThread *m_logthread;

    LogManagerInternal::SignalSource* m_signalSource;

    TimedStatusSource* m_statusSource = nullptr;

    qint64 m_startTime;
    qint64 m_duration;

    int m_exactSliderValue;
    bool m_scroll;
};

#endif // LOGMANAGER_H
