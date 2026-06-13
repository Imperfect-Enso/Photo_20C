#pragma once

#include <Arduino.h>

class KalmanFilter
{
private:
    float q;
    float r;
    float x;
    float p;

public:
    KalmanFilter(float q_, float r_, float init = 0.0f, float p_init = 1.0f)
        : q(q_), r(r_), x(init), p(p_init) {}

    float update(float measurement)
    {
        p += q;

        float k = p / (p + r);
        x += k * (measurement - x);
        p *= (1.0f - k);

        return x;
    }

    float value() const { return x; }

    void  setQ(float q_) { q = q_; }
    void  setR(float r_) { r = r_; }
    float getQ() const   { return q; }
    float getR() const   { return r; }
};