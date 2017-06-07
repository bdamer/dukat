#pragma once

namespace dukat
{
    // TODO: review - I don't like having to recalculate delta every frame
    // also, for more complex functions this could become very slow

    // Computes linear transition between y0 and y1 at time xt.
    template<class T>
    void linear_transition(float xt, float x1, const T& y0, const T& y1, T& yt)
    {
        const auto yd = y1 - y0;
        yt = y0 + yd * xt / x1;
    }

    // Quadratic transition - starts slow and changes rapidly towards the end
    template<class T>
    void quadratic_transition(float xt, float x1, const T& y0, const T& y1, T& yt)
    {
        const auto yd = y1 - y0;
        const auto xf = xt / x1;
        yt = y0 + yd * (xf * xf);
    }

    // Square root transition - starts fast and slows down towards the end
    template<class T>
    void sqrt_transition(float xt, float x1, const T& y0, const T& y1, T& yt)
    {
        const auto yd = y1 - y0;
        const auto xf = xt / x1;
        const auto root = std::sqrt(xf);
        yt = y0 + yd * root;
    }

    // E function - starts fast and slows down towards the end
    template<class T>
    void log_transition(float xt, float x1, const T& y0, const T& y1, T& yt)
    {
        const auto yd = y1 - y0;
        const auto xf = xt / x1;
        const auto xlog = (10.0f + std::log(xf)) / 10.0f;
        yt = y0 + yd * std::max(xlog, 0.0f);
    }

    template<class T>
    void sigmoid_transition(float xt, float x1, const T& y0, const T& y1, T& yt)
    {
        const auto yd = y1 - y0;
        // compute xs as where xt falls between x0..x1 in range [-100..100] 
        const auto xs = 100 * (xt - 0.5f * x1) / x1;
        yt = y0 + yd * xs / (1.0f + std::abs(xs));
    }
}