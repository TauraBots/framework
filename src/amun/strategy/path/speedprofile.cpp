/***************************************************************************
 *   Copyright 2020 Andreas Wendler                                        *
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

#include "speedprofile.h"

static float sign(float x)
{
    return x < 0.0f ? -1.0f : 1.0f;
}

static float dist(float v0, float v1, float acc)
{
    float time = std::abs(v0 - v1) / acc;
    return 0.5f * (v0 + v1) * time;
}

// exponential slowdown calculation
const float MIN_ACC_FACTOR = 0.3f;
float SpeedProfile1D::endOffsetSlowDown(float slowDownTime) const
{
    float pos = 0;
    float slowDownStartTime = profile[counter-1].t - slowDownTime;
    float endTime = profile[counter-1].t + SpeedProfile::SLOW_DOWN_TIME - slowDownTime;
    for (unsigned int i = 0;i<counter-1;i++) {
        if (profile[i+1].t < slowDownStartTime || profile[i].v == profile[i+1].v) {
            pos += (profile[i].v + profile[i+1].v) * 0.5f * (profile[i+1].t - profile[i].t);
        } else {
            float v0;
            float t0;
            if (profile[i].t < slowDownStartTime) {
                float diff = profile[i+1].t == profile[i].t ? 1 : (slowDownStartTime - profile[i].t) / (profile[i+1].t - profile[i].t);
                v0 = profile[i].v + diff * (profile[i+1].v - profile[i].v);
                t0 = slowDownStartTime;
                float partDist = (profile[i].v + v0) * 0.5f * (slowDownStartTime - profile[i].t);
                pos += partDist;
            } else {
                v0 = profile[i].v;
                t0 = profile[i].t;
            }
            float toEndTime0 = endTime - t0;
            float toEndTime1 = endTime - profile[i+1].t;
            float a0 = acc * (MIN_ACC_FACTOR + (1 - MIN_ACC_FACTOR) * toEndTime0 / SpeedProfile::SLOW_DOWN_TIME);
            float a1 = acc * (MIN_ACC_FACTOR + (1 - MIN_ACC_FACTOR) * toEndTime1 / SpeedProfile::SLOW_DOWN_TIME);

            float averageAcc = (a0 + a1) * 0.5f;
            float v1 = profile[i+1].v;
            float t = std::abs(v0 - v1) / averageAcc;
            // TODO: die zeit wird modifiziert, das muss aber auf beiden richtungen gleich sein?

            float d = t * v0 + 0.5f * t * t * sign(v1 - v0) * a0 + (1.0f / 6.0f) * t * t * sign(v1 - v0) * (a1 - a0);
            pos += d;
        }
    }
    return pos;
}

float SpeedProfile1D::endOffset() const
{
    float offset = 0;
    for (unsigned int i = 0;i<counter-1;i++) {
        offset += (profile[i].v + profile[i+1].v) * 0.5f * (profile[i+1].t - profile[i].t);
    }
    return offset;
}

float SpeedProfile1D::timeWithSlowDown(float slowDownTime) const
{
    float time = 0;
    float slowDownStartTime = profile[counter-1].t - slowDownTime;
    float endTime = profile[counter-1].t + SpeedProfile::SLOW_DOWN_TIME - slowDownTime;
    for (unsigned int i = 0;i<counter-1;i++) {
        if (profile[i+1].t < slowDownStartTime || profile[i].v == profile[i+1].v) {
            time += profile[i+1].t - profile[i].t;
        } else {
            float v0;
            float t0;
            if (profile[i].t < slowDownStartTime) {
                float diff = profile[i+1].t == profile[i].t ? 1 : (slowDownStartTime - profile[i].t) / (profile[i+1].t - profile[i].t);
                v0 = profile[i].v + diff * (profile[i+1].v - profile[i].v);
                t0 = slowDownStartTime;
                time += slowDownStartTime - profile[i].t;
            } else {
                v0 = profile[i].v;
                t0 = profile[i].t;
            }
            float toEndTime0 = endTime - t0;
            float toEndTime1 = endTime - profile[i+1].t;
            float a0 = acc * (MIN_ACC_FACTOR + (1 - MIN_ACC_FACTOR) * toEndTime0 / SpeedProfile::SLOW_DOWN_TIME);
            float a1 = acc * (MIN_ACC_FACTOR + (1 - MIN_ACC_FACTOR) * toEndTime1 / SpeedProfile::SLOW_DOWN_TIME);

            float averageAcc = (a0 + a1) * 0.5f;
            float v1 = profile[i+1].v;
            float t = std::abs(v0 - v1) / std::abs(averageAcc);

            time += t;
        }
    }
    return time;
}

std::pair<float, float> SpeedProfile1D::offsetAndSpeedForTime(float time) const
{
    float offset = 0;
    for (unsigned int i = 0;i<counter-1;i++) {
        if (profile[i+1].t >= time) {
            float diff = profile[i+1].t == profile[i].t ? 1 : (time - profile[i].t) / (profile[i+1].t - profile[i].t);
            float speed = profile[i].v + diff * (profile[i+1].v - profile[i].v);
            float partDist = (profile[i].v + speed) * 0.5f * (time - profile[i].t);
            return {offset + partDist, speed};
        }
        offset += (profile[i].v + profile[i+1].v) * 0.5f * (profile[i+1].t - profile[i].t);
    }
    return {offset, profile[counter-1].v};
}

std::pair<float, float> SpeedProfile1D::offsetAndSpeedForTimeSlowDown(float time, float slowDownTime) const
{
    float pos = 0;
    float slowDownStartTime = profile[counter-1].t - slowDownTime;
    unsigned int i = 0;
    float v0, t0 = slowDownStartTime;
    for (;i<counter-1;i++) {
        if (profile[i+1].t >= time || profile[i+1].t >= slowDownStartTime) {
            float td = std::min(time, slowDownStartTime);
            float diff = profile[i+1].t == profile[i].t ? 1 : (td - profile[i].t) / (profile[i+1].t - profile[i].t);
            float speed = profile[i].v + diff * (profile[i+1].v - profile[i].v);
            float partDist = (profile[i].v + speed) * 0.5f * (td - profile[i].t);
            if (time < slowDownStartTime) {
                return {pos + partDist, speed};
            } else {
                pos += partDist;
                v0 = speed;
                break;
            }
        }
        pos += (profile[i].v + profile[i+1].v) * 0.5f * (profile[i+1].t - profile[i].t);
    }

    float endTime = profile[counter-1].t + SpeedProfile::SLOW_DOWN_TIME - slowDownTime;
    float totalTime = t0;
    for (;i<counter-1;i++) {

        float toEndTime0 = endTime - t0;
        float toEndTime1 = endTime - profile[i+1].t;
        float a0 = acc * (MIN_ACC_FACTOR + (1 - MIN_ACC_FACTOR) * toEndTime0 / SpeedProfile::SLOW_DOWN_TIME);
        float a1 = acc * (MIN_ACC_FACTOR + (1 - MIN_ACC_FACTOR) * toEndTime1 / SpeedProfile::SLOW_DOWN_TIME);

        float averageAcc = (a0 + a1) * 0.5f;
        float v1 = profile[i+1].v;
        float t = std::abs(v0 - v1) / averageAcc;

        // a = a0 + t * (a1 - a0) / t1
        // v = v0 + t * a0 + t^2 * 0.5 * (a1 - a0) / t1
        // d = t * v0 + 0.5f * t * t * a0 +

        if (totalTime + t < time) {
            //float d = t * v0 + 0.5f * t * t * sign(v1 - v0) * a1;
            float d = t * v0 + 0.5f * t * t * sign(v1 - v0) * a0 + (1.0f / 6.0f) * t * t * sign(v1 - v0) * (a1 - a0);
            pos += d;

            v0 = profile[i+1].v;
            t0 = profile[i+1].t;
            totalTime += t;
        } else {
            float tm = time - totalTime;
            float speed = v0 + tm * sign(v1 - v0) * a0 + 0.5f * tm * tm * sign(v1 - v0) * (a1 - a0) / t;
            float d = tm * v0 + 0.5f * tm * tm * sign(v1 - v0) * a0 + (1.0f / 6.0f) * tm * tm * tm * sign(v1 - v0) * (a1 - a0) / t;
            return {pos + d, speed};
        }
    }
    return {pos, profile[counter-1].v};
}

void SpeedProfile1D::trajectoryPositions(std::vector<Vector> &outPoints, std::size_t outIndex, float timeInterval, float positionOffset, std::size_t desiredCount) const
{
    std::size_t segment = 0; // where in the profile we currently are
    float currentTime = 0; // the time for the next point to compute the offset of
    float currentOffset = positionOffset; // the offset up to the profile segment with index 'counter', not up to the current time!

    for (std::size_t i = 0;i<desiredCount;i++) {

        while (profile[segment+1].t < currentTime) { // over a profile segment boundary
            currentOffset += (profile[segment].v + profile[segment+1].v) * 0.5f * (profile[segment+1].t - profile[segment].t);
            segment++;

            // if we are at the end of the profile, add the current point the desired number of times
            if (segment == counter-1) {
                while (i < desiredCount) {
                    outPoints[i][outIndex] = currentOffset;
                    i++;
                }
                return;
            }
        }

        // the desired time is in the current segment
        float diff = profile[segment+1].t == profile[segment].t ? 1 : (currentTime - profile[segment].t) / (profile[segment+1].t - profile[segment].t);
        float speed = profile[segment].v + diff * (profile[segment+1].v - profile[segment].v);
        float partDist = (profile[segment].v + speed) * 0.5f * (currentTime - profile[segment].t);
        outPoints[i][outIndex] = currentOffset + partDist;

        currentTime += timeInterval;
    }
}

void SpeedProfile1D::trajectoryPositionsSlowDown(std::vector<Vector> &outPoints, std::size_t outIndex, float timeInterval,
                                                 float positionOffset, float slowDownTime) const
{
    float slowDownStartTime = profile[counter-1].t - slowDownTime;

    // the first points can be handled by the regular trajectory positions calculation
    std::size_t simplePositionCount = static_cast<std::size_t>(std::max(0.0f, slowDownStartTime / timeInterval));
    trajectoryPositions(outPoints, outIndex, timeInterval, positionOffset, simplePositionCount);

    // find current segment
    std::size_t segment = 0;
    float currentOffset = positionOffset;
    while (profile[segment+1].t < slowDownStartTime) {
        currentOffset += (profile[segment].v + profile[segment+1].v) * 0.5f * (profile[segment+1].t - profile[segment].t);
        segment++;
    }

    // compute start position and speed
    float t0 = slowDownStartTime;
    float diff = profile[segment+1].t == profile[segment].t ? 1 : (t0 - profile[segment].t) / (profile[segment+1].t - profile[segment].t);
    float v0 = profile[segment].v + diff * (profile[segment+1].v - profile[segment].v);
    currentOffset += (profile[segment].v + v0) * 0.5f * (t0 - profile[segment].t);

    // go through the remainder of the trajectory and compute the positions
    float endTime = profile[counter-1].t + SpeedProfile::SLOW_DOWN_TIME - slowDownTime;
    float totalTime = t0;
    float currentTime = simplePositionCount * timeInterval;
    for (std::size_t i = simplePositionCount;i<outPoints.size();i++) {

        float toEndTime0 = endTime - t0;
        float toEndTime1 = endTime - profile[segment+1].t;
        float a0 = acc * (MIN_ACC_FACTOR + (1 - MIN_ACC_FACTOR) * toEndTime0 / SpeedProfile::SLOW_DOWN_TIME);
        float a1 = acc * (MIN_ACC_FACTOR + (1 - MIN_ACC_FACTOR) * toEndTime1 / SpeedProfile::SLOW_DOWN_TIME);

        float averageAcc = (a0 + a1) * 0.5f;
        float v1 = profile[segment+1].v;
        float t = std::abs(v0 - v1) / averageAcc;

        if (totalTime + t < currentTime) { // over a profile segment boundary
            float d = t * v0 + 0.5f * t * t * sign(v1 - v0) * a0 + (1.0f / 6.0f) * t * t * sign(v1 - v0) * (a1 - a0);
            currentOffset += d;
            v0 = profile[segment+1].v;
            t0 = profile[segment+1].t;
            totalTime += t;
            segment++;

            // if we are at the end of the profile, add the current point the desired number of times
            if (segment == counter-1) {
                while (i < outPoints.size()) {
                    outPoints[i][outIndex] = currentOffset;
                    i++;
                }
                return;
            }
        }

        // the desired time is in the current segment
        float tm = currentTime - totalTime;
        float d = tm * v0 + 0.5f * tm * tm * sign(v1 - v0) * a0 + (1.0f / 6.0f) * tm * tm * tm * sign(v1 - v0) * (a1 - a0) / t;
        outPoints[i][outIndex] = currentOffset + d;

        currentTime += timeInterval;
    }
}

std::pair<float, float> SpeedProfile1D::calculateRange(float slowDownTime) const
{
    float minPos = 0;
    float maxPos = 0;

    float pos = 0;
    float slowDownStartTime = profile[counter-1].t - slowDownTime;
    unsigned int i = 0;
    float v0, t0 = slowDownStartTime;
    for (;i<counter-1;i++) {

        if (profile[i+1].t >= slowDownStartTime) {
            float td = slowDownStartTime;
            float diff = profile[i+1].t == profile[i].t ? 1 : (td - profile[i].t) / (profile[i+1].t - profile[i].t);
            float speed = profile[i].v + diff * (profile[i+1].v - profile[i].v);
            float partDist = (profile[i].v + speed) * 0.5f * (td - profile[i].t);

            pos += partDist;
            minPos = std::min(minPos, pos);
            maxPos = std::max(maxPos, pos);
            v0 = speed;
            break;
        }
        if ((profile[i].v > 0) != (profile[i+1].v > 0)) {
            float proportion = std::abs(profile[i].v) / (std::abs(profile[i].v) + std::abs(profile[i+1].v));
            float t = (profile[i+1].t - profile[i].t) * proportion;
            float zeroPos = pos + (profile[i].v + 0) * 0.5f * t;
            minPos = std::min(minPos, zeroPos);
            maxPos = std::max(maxPos, zeroPos);
        }
        pos += (profile[i].v + profile[i+1].v) * 0.5f * (profile[i+1].t - profile[i].t);
        minPos = std::min(minPos, pos);
        maxPos = std::max(maxPos, pos);
    }

    float endTime = profile[counter-1].t + SpeedProfile::SLOW_DOWN_TIME - slowDownTime;
    float totalTime = t0;
    for (;i<counter-1;i++) {

        float toEndTime0 = endTime - t0;
        float toEndTime1 = endTime - profile[i+1].t;
        float a0 = acc * (MIN_ACC_FACTOR + (1 - MIN_ACC_FACTOR) * toEndTime0 / SpeedProfile::SLOW_DOWN_TIME);
        float a1 = acc * (MIN_ACC_FACTOR + (1 - MIN_ACC_FACTOR) * toEndTime1 / SpeedProfile::SLOW_DOWN_TIME);

        float averageAcc = (a0 + a1) * 0.5f;
        float v1 = profile[i+1].v;
        float t = std::abs(v0 - v1) / averageAcc;

        // a = a0 + t * (a1 - a0) / t1
        // v = v0 + t * a0 + t^2 * 0.5 * (a1 - a0) / t1
        // d = t * v0 + 0.5f * t * t * a0 +

        //float d = t * v0 + 0.5f * t * t * sign(v1 - v0) * a1;
        float d = t * v0 + 0.5f * t * t * sign(v1 - v0) * a0 + (1.0f / 6.0f) * t * t * sign(v1 - v0) * (a1 - a0);
        pos += d;
        minPos = std::min(minPos, pos);
        maxPos = std::max(maxPos, pos);

        v0 = profile[i+1].v;
        t0 = profile[i+1].t;
        totalTime += t;
    }

    return {minPos, maxPos};
}

void SpeedProfile1D::limitToTime(float time)
{
    for (unsigned int i = 0;i<counter-1;i++) {
        if (profile[i+1].t >= time) {
            float diff = profile[i+1].t == profile[i].t ? 1 : (time - profile[i].t) / (profile[i+1].t - profile[i].t);
            float speed = profile[i].v + diff * (profile[i+1].v - profile[i].v);
            profile[i+1].v = speed;
            profile[i+1].t = time;
            counter = i+2;
            return;
        }
    }
}

std::vector<TrajectoryPoint> SpeedProfile::getTrajectoryPoints() const
{
    if (!isValid()) {
        return {};
    }

    // speed changes less than this time apart are grouped into one trajectory point
    const float SAME_POINT_EPSILON = 0.005f;

    std::vector<TrajectoryPoint> result;
    result.reserve(xProfile.counter + yProfile.counter);
    result.push_back({Vector(0, 0), Vector(xProfile.profile[0].v, yProfile.profile[0].v), 0});

    std::size_t xIndex = 0;
    std::size_t yIndex = 0;

    while (xIndex < xProfile.counter-1 && yIndex < yProfile.counter-1) {
        float xNext = xProfile.profile[xIndex + 1].t;
        float yNext = yProfile.profile[yIndex + 1].t;

        if (std::abs(xNext - yNext) < SAME_POINT_EPSILON) {
            float time = (xNext + yNext) / 2.0f;
            auto posSpeed = positionAndSpeedForTime(time);
            result.push_back({posSpeed.first, posSpeed.second, time});
            xIndex++;
            yIndex++;
        } else if (xNext < yNext) {
            auto posSpeed = positionAndSpeedForTime(xNext);
            result.push_back({posSpeed.first, posSpeed.second, xNext});
            xIndex++;
        } else {
            auto posSpeed = positionAndSpeedForTime(yNext);
            result.push_back({posSpeed.first, posSpeed.second, yNext});
            yIndex++;
        }
    }

    // compensate for the missing exponential slowdown by adding a segment with zero speed
    if (slowDownTime != 0.0f) {
        float endTime = time();
        result.push_back({positionAndSpeedForTime(endTime).first, result.back().speed, endTime});
    }

    return result;
}

// trajectory calculation
static float constantDistance(float v, float time)
{
    return v * time;
}

static float freeExtraTimeDistance(float v, float time, float acc, float vMax, float &outTopSpeed)
{
    vMax *= sign(time);
    time = std::abs(time);
    float toMaxTime = 2.0f * std::abs(vMax - v) / acc;
    if (toMaxTime < time) {
        outTopSpeed = vMax;
        return 2 * dist(v, vMax, acc) +
               constantDistance(vMax, time - toMaxTime);
    } else {
        float v1 = (v > vMax ? -1.0f : 1.0f) * acc * time / 2 + v;
        outTopSpeed = v1;
        return 2.0f * dist(v, v1, acc);
    }
}

SpeedProfile1D::TrajectoryPosInfo1D SpeedProfile1D::calculateEndPos1D(float v0, float v1, float hintDist, float acc, float vMax)
{
    // TODO: this can be optimized
    float topSpeed;
    if (hintDist == 0.0f) {
        return {dist(v0, v1, acc), std::max(v0, v1)};
    } else if (hintDist < 0 && v0 <= v1) {
        if (v0 >= -vMax) {
            return {freeExtraTimeDistance(v0, hintDist, acc, vMax, topSpeed) +
                        dist(v0, v1, acc), topSpeed};
        } else if (v0 < -vMax && v1 >= -vMax) {
            return {dist(v0, v1, acc) +
                        constantDistance(-vMax, -hintDist), -vMax};
        } else {
            return {dist(v0, v1, acc) +
                        freeExtraTimeDistance(v1, hintDist, acc, vMax, topSpeed), topSpeed};
        }
    } else if (hintDist < 0 && v0 > v1) {
        if (v1 >= -vMax) {
            return {dist(v0, v1, acc) +
                        freeExtraTimeDistance(v1, hintDist, acc, vMax, topSpeed), topSpeed};
        } else if (v1 < -vMax && v0 >= -vMax) {
            return {dist(v0, v1, acc) +
                        constantDistance(-vMax, -hintDist), -vMax};
        } else {
            return {freeExtraTimeDistance(v0, hintDist, acc, vMax, topSpeed) +
                        dist(v0, v1, acc), topSpeed};
        }
    } else if (hintDist > 0 && v0 <= v1) {
        if (v1 <= vMax) {
            return {dist(v0, v1, acc) +
                        freeExtraTimeDistance(v1, hintDist, acc, vMax, topSpeed), topSpeed};
        } else if (v1 > vMax && v0 <= vMax) {
            return {dist(v0, v1, acc) +
                        constantDistance(vMax, hintDist), vMax};
        } else {
            return {freeExtraTimeDistance(v0, hintDist, acc, vMax, topSpeed) +
                        dist(v0, v1, acc), topSpeed};
        }
    } else { // hintDist > 0, v0 > v1
        //assert(hintDist > 0 && v0 > v1);
        if (v0 <= vMax) {
            return {freeExtraTimeDistance(v0, hintDist, acc, vMax, topSpeed) +
                        dist(v0, v1, acc), topSpeed};
        } else if (v0 > vMax && v1 <= vMax) {
            return {dist(v0, v1, acc) +
                        constantDistance(vMax, hintDist), vMax};
        } else {
            return {dist(v0, v1, acc) +
                        freeExtraTimeDistance(v1, hintDist, acc, vMax, topSpeed), topSpeed};
        }
    }
}

static void adjustEndSpeed(float v0, const float v1, float time, bool directionPositive, float acc, float &outExtraTime, float &outV1)
{
    outExtraTime = 0.0f;
    outV1 = v1;

    if (directionPositive) {
        if (v0 < 0 && v1 < 0) {
            float toZeroTime = std::abs(v0) / acc;
            if (toZeroTime < time) {
                outV1 = 0;
                outExtraTime = time - toZeroTime;
            } else {
                outV1 = v0 + time * acc;
                // omit check for invalid case outV1 < v1
            }
        } else if (v0 < 0 && v1 >= 0) {
            float toV1Time = (v1 - v0) / acc;
            if (toV1Time < time) {
                // do nothing, everything is fine
                outExtraTime = time - toV1Time;
            } else {
                outV1 = v0 + time * acc;
                // omit check for invalid case outV1 < 0
            }
        } else if (v0 >= 0 && v1 < 0) {
            outV1 = 0;
            outExtraTime = time - std::abs(v0) / acc;
            // omit check for invalid case outRestTime < 0
        } else { // v0 >= 0, v1 >= 0
            float directTime = std::abs(v0 - v1) / acc;
            if (directTime < time) {
                outExtraTime = time - directTime;
            } else {
                // omit check for invalid case v0 > v1
                outV1 = v0 + time * acc;
            }
        }

    } else { // directionPositive == false
        // TODO: this block is basically the same as the above, just some sign changes
        if (v0 < 0 && v1 < 0) {
            float directTime = std::abs(v0 - v1) / acc;
            if (directTime < time) {
                outExtraTime = time - directTime;
            } else {
                // omit check for invalid case v0 < v1
                outV1 = v0 - time * acc;
            }
        } else if (v0 < 0 && v1 >= 0) {
            outV1 = 0;
            outExtraTime = time - std::abs(v0) / acc;
            // omit check for invalid case outRestTime < 0
        } else if (v0 >= 0 && v1 < 0) {
            float toV1Time = (v0 - v1) / acc;
            if (toV1Time < time) {
                // do nothing, everything is fine
                outExtraTime = time - toV1Time;
            } else {
                outV1 = v0 - time * acc;
                // omit check for invalid case outV1 > 0
            }
        } else { // v0 >= 0 && v1 >= 0
            float toZeroTime = std::abs(v0) / acc;
            if (toZeroTime < time) {
                outV1 = 0;
                outExtraTime = time - toZeroTime;
            } else {
                outV1 = v0 - time * acc;
                // omit check for invalid case outV1 > v1
            }
        }
    }
}

SpeedProfile1D::TrajectoryPosInfo1D SpeedProfile1D::calculateEndPos1DFastSpeed(float v0, float v1, float time, bool directionPositive, float acc, float vMax)
{
    // TODO: simple case with v1 = 0 seperately?
    float realV1, extraTime;
    adjustEndSpeed(v0, v1, time, directionPositive, acc, extraTime, realV1);

    if (extraTime == 0.0f) {
        return {(v0 + realV1) * 0.5f * time, directionPositive ? std::max(v0, v1) : std::min(v0, v1)};
    } else {
        // TODO: remove the negative time in calculateEndPos1D
        return calculateEndPos1D(v0, realV1, directionPositive ? extraTime : -extraTime, acc, vMax);
    }
}

void SpeedProfile1D::calculate1DTrajectoryFastEndSpeed(float v0, float v1, float time, bool directionPositive, float acc, float vMax)
{
    // TODO: simple case with v1 = 0 seperately?
    float realV1, extraTime;
    adjustEndSpeed(v0, v1, time, directionPositive, acc, extraTime, realV1);

    if (extraTime == 0.0f) {
        this->acc = acc;
        profile[0] = {v0, 0};
        profile[1] = {realV1, std::abs(realV1 - v0) / acc};
        counter = 2;
    } else {
        // TODO: remove the negative time in calculateEndPos1D
        calculate1DTrajectory(v0, realV1, directionPositive ? extraTime : -extraTime, acc, vMax);
    }
}

static void createFreeExtraTimeSegment(float beforeSpeed, float v, float nextSpeed, float time, float acc, float vMax, SpeedProfile1D &p)
{
    vMax *= sign(time);
    time = std::abs(time);
    float toMaxTime = 2.0f * std::abs(vMax - v) / acc;
    if (toMaxTime < time) {
        p.profile[1] = {vMax, std::abs(vMax - beforeSpeed) / acc};
        p.profile[2] = {vMax, time - toMaxTime};
        p.profile[3] = {nextSpeed, std::abs(vMax - nextSpeed) / acc};
        p.counter = 4;
    } else {
        float v1 = (v > vMax ? -1.0f : 1.0f) * acc * time / 2 + v;
        p.profile[1] = {v1, std::abs(beforeSpeed - v1) / acc};
        p.profile[2] = {nextSpeed, std::abs(nextSpeed - v1) / acc};
        p.counter = 3;
    }
}

void SpeedProfile1D::calculate1DTrajectory(float v0, float v1, float hintDist, float acc, float vMax)
{
    this->acc = acc;
    profile[0] = {v0, 0};

    if (hintDist == 0.0f) {
        profile[1] = {v1, std::abs(v0 - v1) / acc};
        counter = 2;
    } else if (hintDist < 0 && v0 <= v1) {
        if (v0 >= -vMax) {
            createFreeExtraTimeSegment(v0, v0, v1, hintDist, acc, vMax, *this);
        } else if (v0 < -vMax && v1 >= -vMax) {
            profile[1] = {-vMax, std::abs(v0 + vMax) / acc};
            profile[2] = {-vMax, -hintDist};
            profile[3] = {v1, std::abs(v1 + vMax) / acc};
            counter = 4;
        } else {
            createFreeExtraTimeSegment(v0, v1, v1, hintDist, acc, vMax, *this);
        }
    } else if (hintDist < 0 && v0 > v1) {
        if (v1 >= -vMax) {
            createFreeExtraTimeSegment(v0, v1, v1, hintDist, acc, vMax, *this);
        } else if (v1 < -vMax && v0 >= -vMax) {
            profile[1] = {-vMax, std::abs(v0 + vMax) / acc};
            profile[2] = {-vMax, -hintDist};
            profile[3] = {v1, std::abs(v1 + vMax) / acc};
            counter = 4;
        } else {
            createFreeExtraTimeSegment(v0, v0, v1, hintDist, acc, vMax, *this);
        }
    } else if (hintDist > 0 && v0 <= v1) {
        if (v1 <= vMax) {
            createFreeExtraTimeSegment(v0, v1, v1, hintDist, acc, vMax, *this);
        } else if (v1 > vMax && v0 <= vMax) {
            profile[1] = {vMax, std::abs(v0 - vMax) / acc};
            profile[2] = {vMax, hintDist};
            profile[3] = {v1, std::abs(v1 - vMax) / acc};
            counter = 4;
        } else {
            createFreeExtraTimeSegment(v0, v0, v1, hintDist, acc, vMax, *this);
        }
    } else { // hintDist > 0, v0 > v1
        //assert(hintDist > 0 && v0 > v1);
        if (v0 <= vMax) {
            createFreeExtraTimeSegment(v0, v0, v1, hintDist, acc, vMax, *this);
        } else if (v0 > vMax && v1 <= vMax) {
            profile[1] = {vMax, std::abs(v0 - vMax) / acc};
            profile[2] = {vMax, hintDist};
            profile[3] = {v1, std::abs(v1 - vMax) / acc};
            counter = 4;
        } else {
            createFreeExtraTimeSegment(v0, v1, v1, hintDist, acc, vMax, *this);
        }
    }
}
