/***************************************************************************
 *   Copyright 2021 Tobias Heineken                                        *
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
#ifndef COMMANDCONVERTER_H
#define COMMANDCONVERTER_H

#include <QObject>
#include "protobuf/sslsim.h"
#include "protobuf/status.h"
#include "protobuf/command.h"

class CommandConverter: public QObject {
    Q_OBJECT
public:
    explicit CommandConverter(QObject* parent = nullptr) : QObject(parent) {}

signals:
    void sendSSLSim(SSLSimRobotControl control, bool blue);

public slots:
    void handleRadioCommands(const QList<robot::RadioCommand> &commands, qint64 processingStart);
    void handleCommand(Command c);

private:
    bool m_charge = false;

};

#endif
