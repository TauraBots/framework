/***************************************************************************
 *   Copyright 2021 Andreas Wendler                                        *
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

#ifndef TRACKINGREPLAY_H
#define TRACKINGREPLAY_H

#include <QObject>

#include "protobuf/status.h"
#include "processor.h"

class TrackingReplay : public QObject
{
    Q_OBJECT
public:
    explicit TrackingReplay(Timer *timer);

signals:
    void gotStatus(const Status &status);

public slots:
    // this function will set the replay timer itself
    void handleStatus(const Status &status);

private slots:
    void ammendStatus(const Status &status);

private:
    Timer *m_timer;
    Processor m_replayProcessor;
    Status m_lastTrackingReplayGameState;
};

#endif // TRACKINGREPLAY_H
