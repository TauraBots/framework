/***************************************************************************
 *   Copyright 2016 Alexander Danzer                                       *
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

#ifndef BALLFLYFILTER_H
#define BALLFLYFILTER_H

#include "abstractballfilter.h"
#include "quadraticleastsquaresfitter.h"
#include "protobuf/ssl_detection.pb.h"
#include "protobuf/world.pb.h"

class FlyFilter : public AbstractBallFilter
{
public:
    explicit FlyFilter(const VisionFrame& frame, CameraInfo* cameraInfo, const FieldTransform &transform);
    FlyFilter(const FlyFilter &filter) = default;

    void processVisionFrame(const VisionFrame& frame) override;
    bool acceptDetection(const VisionFrame& frame) override;
    void writeBallState(world::Ball *ball, qint64 predictionTime, const QVector<RobotInfo> &robots, qint64 lastCameraFrameTime) override;
    float distToStartPos() { return m_distToStartPos; }

    bool isActive() const;

private:
    struct PinvResult {
        float x0;
        float y0;
        float z0;
        float vx;
        float vy;
        float vz;
        float distStartPos;
        float vxControl;
        float vyControl;
        float refSpeed;
    };

    struct IntersectionResult {
        Eigen::Vector2f intersection;
        Eigen::Vector2f intersectionGroundSpeed;
        float intersectionZSpeed;
    };

    // stores the information from the chip reconstruction,
    // fully describes the current chip
    struct ChipReconstruction {
        Eigen::Vector2f chipStartPos;
        qint64 chipStartTime;
        Eigen::Vector2f groundSpeed;
        float zSpeed;
    };

    struct ChipDetection {
        ChipDetection(float s, float as, float t, Eigen::Vector2f bp, Eigen::Vector2f dp,  float a, Eigen::Vector2f r, quint32 cid, bool cc, bool lc, int rid)
            :  dribblerSpeed(s), absSpeed(as), time(t), ballPos(bp), dribblerPos(dp), robotPos(r), cameraId(cid), ballArea(a), chipCommand(cc), linearCommand(lc), robotId(rid)  {}
        ChipDetection(){} // make QVector happy

        float dribblerSpeed;
        float absSpeed;
        float time; // in ns, since init of filter
        Eigen::Vector2f ballPos;
        Eigen::Vector2f dribblerPos;
        Eigen::Vector2f robotPos;
        int robotId;
        quint32 cameraId;
        float ballArea;
        bool chipCommand;
        bool linearCommand;
    };

    ChipDetection createChipDetection(const VisionFrame& frame) const;

    bool detectionCurviness(const PinvResult& pinvRes) const;
    bool detectionHeight() const;
    bool detectionSpeed() const;
    bool detectionPinv(const PinvResult &pinvRes) const;
    bool detectChip(const PinvResult &pinvRes) const;

    bool checkIsShot();
    bool checkIsDribbling() const;
    bool collision();
    unsigned numMeasurementsWithOwnCamera() const;
    Eigen::Vector3f unproject(const ChipDetection& detection, float ballRadius) const;

    PinvResult calcPinv();
    IntersectionResult calcIntersection(const PinvResult &pinvRes) const;

    ChipReconstruction approachPinvApply(const PinvResult& pinvRes) const;
    ChipReconstruction approachIntersectApply(const IntersectionResult &intRes) const;
    ChipReconstruction approachAreaApply();

    bool approachPinvApplicable(const PinvResult& pinvRes) const;
    bool approachIntersectApplicable(const IntersectionResult &intRes) const;

    void parabolicFlightReconstruct(const PinvResult &pinvRes);
    void resetFlightReconstruction();

    struct Prediction {
        Prediction(Eigen::Vector2f pos2, float z, Eigen::Vector2f speed2, float vz) :
            pos(Eigen::Vector3f(pos2.x(),pos2.y(),z)), speed(Eigen::Vector3f(speed2.x(),speed2.y(),vz)) {}

        Eigen::Vector3f pos;
        Eigen::Vector3f speed;
    };

    Prediction predictTrajectory(qint64 time);

private:
    bool m_chipDetected;
    bool m_isActive;

    QVector<ChipDetection> m_shotDetectionWindow; // sliding window of size 5
    QVector<ChipDetection> m_kickFrames;

    ChipReconstruction m_chipReconstruction;

    Eigen::Vector2f m_touchdownPos;

    bool m_bouncing;
    qint64 m_bounceStartTime;
    float m_bounceZSpeed;
    Eigen::Vector2f m_bounceStartPos;
    Eigen::Vector2f m_bounceGroundSpeed;

    int m_shotStartFrame;

    float m_distToStartPos;

    qint64 m_initTime;

    QuadraticLeastSquaresFitter m_flyFitter;

    int m_pinvDataInserted;
    Eigen::VectorXf m_d_detailed;
    Eigen::MatrixXf m_D_detailed;
    Eigen::VectorXf m_d_coarseControl;
    Eigen::MatrixXf m_D_coarseControl;

    qint64 m_lastPredictionTime;

    float m_acceptDist = 0;
};

#endif // BALLFLYFILTER_H
