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

#include "robotfilter.h"
#include "core/timer.h"

const qint64 PROCESSOR_TICK_DURATION = 10 * 1000 * 1000;
const float MAX_LINEAR_ACCELERATION = 10.;
const float MAX_ROTATION_ACCELERATION = 60.;
const float OMEGA_MAX = 10 * 2 * M_PI;

RobotFilter::RobotFilter(const SSL_DetectionRobot &robot, qint64 last_time) :
    Filter(last_time),
    m_id(robot.robot_id()),
    m_futureTime(0)
{
    // translate from sslvision coordinate system
    Kalman::Vector x;
    x(0) = -robot.y() / 1000.0;
    x(1) = robot.x() / 1000.0;
    x(2) = robot.orientation() + M_PI_2;
    x(3) = 0.0;
    x(4) = 0.0;
    x(5) = 0.0;
    m_kalman = new Kalman(x);

    // we can only observe the position
    m_kalman->H(0, 0) = 1.0;
    m_kalman->H(1, 1) = 1.0;
    m_kalman->H(2, 2) = 1.0;

    m_futureKalman = new Kalman(x);
    resetFutureKalman();
}

RobotFilter::~RobotFilter()
{
    delete m_kalman;
    delete m_futureKalman;
}

void RobotFilter::resetFutureKalman()
{
    *m_futureKalman = *m_kalman;
    m_futureTime = m_lastTime;

    m_futureKalman->H = Kalman::MatrixM::Zero();
    m_futureKalman->H(0, 3) = 1.0;
    m_futureKalman->H(1, 4) = 1.0;
    m_futureKalman->H(2, 5) = 1.0;
}

// updates the filter to the best possible prediction for the given time
// vision frames are applies permanently that is their timestamps must
// increase monotonically. The same is true for the robot speed estimates,
// with the exception that these are only applied temporarily if they are
// newer than the newest vision frame
void RobotFilter::update(qint64 time)
{
    // apply new vision frames
    bool isVisionUpdated = false;
    while (!m_visionFrames.isEmpty()) {
        VisionFrame &frame = m_visionFrames.first();
        if (frame.time > time) {
            break;
        }

        // only apply radio commands that have reached the robot yet
        foreach (const RadioCommand &command, m_radioCommands) {
            const qint64 commandTime = command.second;
            if (commandTime > frame.time) {
                break;
            }
            predict(commandTime, false, true, false, m_lastRadioCommand);
            m_lastRadioCommand = command;
        }
        invalidateRobotCommand(frame.time);

        // switch to the new camera if the primary camera data is too old
        bool cameraSwitched = checkCamera(frame.cameraId, frame.time);
        predict(frame.time, false, true, cameraSwitched, m_lastRadioCommand);
        applyVisionFrame(frame);

        isVisionUpdated = true;
        m_visionFrames.removeFirst();
    }
    if (isVisionUpdated || time < m_futureTime) {
        // prediction is rebased on latest vision frame
        resetFutureKalman();
        m_futureRadioCommand = m_lastRadioCommand;
    }

    // only apply radio commands that have reached the robot yet
    foreach (const RadioCommand &command, m_radioCommands) {
        const qint64 commandTime = command.second;
        if (commandTime > time) {
            break;
        }
        // only apply radio commands not used yet
        if (commandTime > m_futureTime) {
            // updates m_futureKalman
            predict(commandTime, true, true, false, m_futureRadioCommand);
            m_futureRadioCommand = command;
        }
    }

    // predict to requested timestep
    predict(time, true, false, false, m_futureRadioCommand);
}

void RobotFilter::invalidateRobotCommand(qint64 time)
{
    // cleanup outdated radio commands
    while (!m_radioCommands.isEmpty()) {
        const RadioCommand &command = m_radioCommands.first();
        if (command.second > time) {
            break;
        }
        m_radioCommands.removeFirst();
    }
}

void RobotFilter::predict(qint64 time, bool updateFuture, bool permanentUpdate, bool cameraSwitched, const RadioCommand &cmd)
{
    // just assume that the prediction step is the same for now and the future
    Kalman* kalman = (updateFuture) ? m_futureKalman : m_kalman;
    const qint64 lastTime = (updateFuture) ? m_futureTime : m_lastTime;
    const double timeDiff = (time - lastTime) * 1E-9;
    Q_ASSERT(timeDiff >= 0);

    // for the filter model see:
    // Browning, Brett; Bowling, Michael; and Veloso, Manuela M., "Improbability Filtering for Rejecting False Positives" (2002).
    // Robotics Institute. Paper 123. http://repository.cmu.edu/robotics/123

    // state vector description: (v_s and v_f are swapped in comparision to the paper)
    // (x y phi v_s v_f omega)
    // local and global coordinate system are rotated by 90 degree (see processor)
    const float phi = kalman->baseState()(2) - M_PI_2;
    const float v_s = kalman->baseState()(3);
    const float v_f = kalman->baseState()(4);
    const float omega = kalman->baseState()(5);

    // Process state transition: update position with the current speed
    kalman->F(0, 3) = std::cos(phi) * timeDiff;
    kalman->F(0, 4) = -std::sin(phi) * timeDiff;
    kalman->F(1, 3) = std::sin(phi) * timeDiff;
    kalman->F(1, 4) = std::cos(phi) * timeDiff;
    kalman->F(2, 5) = timeDiff;

    kalman->F(3, 3) = 1;
    kalman->F(4, 4) = 1;
    kalman->F(5, 5) = 1;
    // clear control input
    kalman->u = Kalman::Vector::Zero();
    if (time < cmd.second + 2 * PROCESSOR_TICK_DURATION) {
        // radio commands are intended to be applied over 10ms
        float cmd_interval = (float)std::max(PROCESSOR_TICK_DURATION*1E-9, timeDiff);

        float cmd_v_s = cmd.first.v_s();
        float cmd_v_f = cmd.first.v_f();
        float cmd_omega = cmd.first.omega();

        float accel_s = (cmd_v_s - v_s)/cmd_interval;
        float accel_f = (cmd_v_f - v_f)/cmd_interval;
        float accel_omega = (cmd_omega - omega)/cmd_interval;

        float bounded_a_s = qBound(-MAX_LINEAR_ACCELERATION, accel_s, MAX_LINEAR_ACCELERATION);
        float bounded_a_f = qBound(-MAX_LINEAR_ACCELERATION, accel_f, MAX_LINEAR_ACCELERATION);
        float bounded_a_omega = qBound(-MAX_ROTATION_ACCELERATION, accel_omega, MAX_ROTATION_ACCELERATION);

        kalman->u(0) = 0;
        kalman->u(1) = 0;
        kalman->u(2) = 0;
        kalman->u(3) = bounded_a_s * timeDiff;
        kalman->u(4) = bounded_a_f * timeDiff;
        kalman->u(5) = bounded_a_omega * timeDiff;
    }

    // prevent rotation speed windup
    if (omega > OMEGA_MAX) {
        kalman->u(5) = std::min<float>(kalman->u(5), OMEGA_MAX - omega);
    } else if (omega < -OMEGA_MAX) {
        kalman->u(5) = std::max<float>(kalman->u(5), -OMEGA_MAX + omega);
    }

    // update covariance jacobian
    kalman->B = kalman->F;
    kalman->B(0, 2) = -(v_s*std::sin(phi) + v_f*std::cos(phi)) * timeDiff;
    kalman->B(1, 2) = (v_s*std::cos(phi) - v_f*std::sin(phi)) * timeDiff;

    // Process noise: stddev for acceleration
    // guessed from the accelerations that are possible on average
    const float sigma_a_x = 4.0f;
    const float sigma_a_y = 4.0f;
    // a bit too low, but that speed is nearly impossible all the time
    const float sigma_a_phi = 10.0f;

    // using no position errors (in opposite to the CMDragons model)
    // seems to yield better results in the simulator
    // d = timediff
    // G = (d^2/2, d^2/2, d^2/2, d, d, d)
    // sigma = (x, y, phi, x, y, phi)  (using x = sigma_a_x, ...)
    // Q = GG^T*(diag(sigma)^2)
    Kalman::Vector G;
    G(0) = timeDiff * timeDiff / 2 * sigma_a_x;
    G(1) = timeDiff * timeDiff / 2 * sigma_a_y;
    G(2) = timeDiff * timeDiff / 2 * sigma_a_phi;
    G(3) = timeDiff * sigma_a_x;
    G(4) = timeDiff * sigma_a_y;
    G(5) = timeDiff * sigma_a_phi;

    if (cameraSwitched) {
        // handle small errors in camera alignment
        G(0) += 0.02;
        G(1) += 0.02;
        G(2) += 0.05;
    }

    kalman->Q(0, 0) = G(0) * G(0);
    kalman->Q(0, 3) = G(0) * G(3);
    kalman->Q(3, 0) = G(3) * G(0);
    kalman->Q(3, 3) = G(3) * G(3);

    kalman->Q(1, 1) = G(1) * G(1);
    kalman->Q(1, 4) = G(1) * G(4);
    kalman->Q(4, 1) = G(4) * G(1);
    kalman->Q(4, 4) = G(4) * G(4);

    kalman->Q(2, 2) = G(2) * G(2);
    kalman->Q(2, 5) = G(2) * G(5);
    kalman->Q(5, 2) = G(5) * G(2);
    kalman->Q(5, 5) = G(5) * G(5);

    kalman->predict(permanentUpdate);
    if (permanentUpdate) {
        if (updateFuture) {
            m_futureTime = time;
        } else {
            m_lastTime = time;
        }
    }
}

double RobotFilter::limitAngle(double angle) const
{
    while (angle > M_PI) {
        angle -= 2 * M_PI;
    }
    while (angle < -M_PI) {
        angle += 2 * M_PI;
    }
    return angle;
}

void RobotFilter::applyVisionFrame(const VisionFrame &frame)
{
    const float pRot = m_kalman->state()(2);
    const float pRotLimited = limitAngle(pRot);
    if (pRot != pRotLimited) {
        // prevent rotation windup
        m_kalman->modifyState(2, pRotLimited);
    }
    float rot = frame.detection.orientation() + M_PI_2;
    // prevent discontinuities
    float diff = limitAngle(rot - pRotLimited);

    // keep for debugging
    world::RobotPosition p;
    p.set_time(frame.time);
    p.set_p_x(-frame.detection.y() / 1000.0);
    p.set_p_y(frame.detection.x() / 1000.0);
    p.set_phi(pRotLimited + diff);
    p.set_camera_id(frame.cameraId);
    m_measurements.append(p);

    m_kalman->z(0) = p.p_x();
    m_kalman->z(1) = p.p_y();
    m_kalman->z(2) = p.phi();

    Kalman::MatrixMM R = Kalman::MatrixMM::Zero();
    if (frame.cameraId == m_primaryCamera) {
        // measurement covariance matrix
        // a good calibration should work with 0.002, 0.002, 0.006
        // however add some safety margin, except for orientation which is a perfect normal distribution
        // for moving robots the safety margin is required, probably to smooth out the robot vibrations
        R(0, 0) = 0.004;
        R(1, 1) = 0.004;
        R(2, 2) = 0.01;
    } else {
        // handle small errors in camera alignment
        // ensure that the measurements don't corrupt the results
        R(0, 0) = 0.02;
        R(1, 1) = 0.02;
        R(2, 2) = 0.03;
    }
    m_kalman->R = R.cwiseProduct(R);
    m_kalman->update();
}

void RobotFilter::get(world::Robot *robot, bool flip, bool noRawData)
{
    float px = m_futureKalman->state()(0);
    float py = m_futureKalman->state()(1);
    float phi = m_futureKalman->state()(2);
    // convert to global coordinates
    const float v_s = m_futureKalman->state()(3);
    const float v_f = m_futureKalman->state()(4);
    const float tmpPhi = phi - M_PI_2;
    float vx = std::cos(tmpPhi)*v_s - std::sin(tmpPhi)*v_f;
    float vy = std::sin(tmpPhi)*v_s + std::cos(tmpPhi)*v_f;
    float omega = m_futureKalman->state()(5);

    if (flip) {
        phi += M_PI;
        px = -px;
        py = -py;
        vx = -vx;
        vy = -vy;
    }

    robot->set_id(m_id);
    robot->set_p_x(px);
    robot->set_p_y(py);
    robot->set_phi(limitAngle(phi));
    robot->set_v_x(vx);
    robot->set_v_y(vy);
    robot->set_omega(omega);

    if (noRawData) {
        return;
    }

    foreach (const world::RobotPosition &p, m_measurements) {
        world::RobotPosition *np = robot->add_raw();
        np->set_time(p.time());
        float rot;
        if (flip) {
            np->set_p_x(-p.p_x());
            np->set_p_y(-p.p_y());
            rot = p.phi() + M_PI;
        } else {
            np->set_p_x(p.p_x());
            np->set_p_y(p.p_y());
            rot = p.phi();
        }
        np->set_phi(limitAngle(rot));
        np->set_camera_id(p.camera_id());

        const world::RobotPosition &prevPos = m_lastRaw[np->camera_id()];

        if (prevPos.IsInitialized() && np->time() > prevPos.time()
                && prevPos.time() + 200*1000*1000 > np->time()) {
            np->set_v_x((np->p_x() - prevPos.p_x()) / ((np->time() - prevPos.time()) * 1E-9));
            np->set_v_y((np->p_y() - prevPos.p_y()) / ((np->time() - prevPos.time()) * 1E-9));
            np->set_omega(limitAngle(np->phi() - prevPos.phi()) / ((np->time() - prevPos.time()) * 1E-9));
            np->set_time_diff_scaled((np->time() - prevPos.time()) * 1E-7);
            np->set_system_delay((Timer::systemTime() - np->time()) * 1E-9);
        }
        m_lastRaw[np->camera_id()] = *np;
    }

    m_measurements.clear();
}

// uses the tracked position only based on vision data!!!
float RobotFilter::distanceTo(const SSL_DetectionRobot &robot) const
{
    Eigen::Vector2f b;
    b(0) = -robot.y() / 1000.0;
    b(1) = robot.x() / 1000.0;

    Eigen::Vector2f p;
    p(0) = m_kalman->state()(0);
    p(1) = m_kalman->state()(1);

    return (b - p).norm();
}

Eigen::Vector2f RobotFilter::dribblerPos() const
{
    Eigen::Vector2f robotMiddle(m_kalman->state()(0), m_kalman->state()(1));
    float phi = limitAngle(m_kalman->state()(2));
    return robotMiddle + 0.08*Eigen::Vector2f(cos(phi), sin(phi));
}

Eigen::Vector2f RobotFilter::robotPos() const
{
    return Eigen::Vector2f(m_kalman->state()(0), m_kalman->state()(1));
}


void RobotFilter::addVisionFrame(qint32 cameraId, const SSL_DetectionRobot &robot, qint64 time)
{
    m_visionFrames.append(VisionFrame(cameraId, robot, time));
    // only count frames for the primary camera
    if (m_primaryCamera == -1 || m_primaryCamera == cameraId) {
        m_frameCounter++;
    }
}

void RobotFilter::addRadioCommand(const robot::Command &radioCommand, qint64 time)
{
    m_radioCommands.append(qMakePair(radioCommand, time));
}
