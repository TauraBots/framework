/***************************************************************************
 *   Copyright 2015 Michael Eischer, Philipp Nordhus                       *
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

#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "protobuf/command.h"
#include "protobuf/status.h"
#include <QList>
#include <QMap>
#include <QPair>
#include <QQueue>
#include <QByteArray>

// higher values break the rolling friction of the ball
const float SIMULATOR_SCALE = 10.0f;
const float SUB_TIMESTEP = 1/200.f;
const float COLLISION_MARGIN = 0.04f;
const unsigned FOCAL_LENGTH = 390;

class QByteArray;
class QTimer;
class SimRobot;
struct SimulatorData;
class Timer;
class SSL_GeometryFieldSize;

class Simulator : public QObject
{
    Q_OBJECT

public:
    typedef QMap<QPair<unsigned int, unsigned int>, SimRobot *> RobotMap;

    explicit Simulator(const Timer *timer, amun::CommandSimulator::RuleVersion ruleVersion);
    ~Simulator() override;
    void handleSimulatorTick(double timeStep);

signals:
    void gotPacket(const QByteArray &data, qint64 time, QString sender);
    void sendStatus(const Status &status);
    void sendRadioResponses(const QList<robot::RadioResponse> &responses);

public slots:
    void handleCommand(const Command &command);
    void handleRadioCommands(const QList<robot::RadioCommand> &commands, qint64 processingDelay);
    void setScaling(double scaling);

private slots:
    void process();
    void sendVisionPacket();

private:
    void resetFlipped(RobotMap &robots, float side);
    QByteArray createVisionPacket();
    void resetVisionPackets();
    void setTeam(RobotMap &list, float side, const robot::Team &team);
    void moveBall(const amun::SimulatorMoveBall &ball);
    void moveRobot(const RobotMap &list, const amun::SimulatorMoveRobot &robot);
    void fieldAddLine(SSL_GeometryFieldSize *field, std::string name, float x1, float y1, float x2, float y2) const;
    void fieldAddCircularArc(SSL_GeometryFieldSize *field, std::string name, float x, float y, float radius, float a1, float a2) const;

private:
    typedef QPair<QList<robot::RadioCommand>, qint64> RadioCommand;
    SimulatorData *m_data;
    QQueue<RadioCommand> m_radioCommands;
    QQueue<QByteArray> m_visionPackets;
    QQueue<QTimer *> m_visionTimers;
    const Timer *m_timer;
    QTimer *m_trigger;
    qint64 m_time;
    qint64 m_lastSentStatusTime;
    double m_timeScaling;
    bool m_enabled;
    bool m_charge;
    // systemDelay + visionProcessingTime = visionDelay
    qint64 m_visionDelay;
    qint64 m_visionProcessingTime;
    amun::CommandSimulator::RuleVersion m_currentRuleVersion;
};

#endif // SIMULATOR_H
