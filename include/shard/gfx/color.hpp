#pragma once

namespace shard{
    namespace gfx{
        struct Color{
            Color():
                r{0.0f},
                g{0.0f},
                b{0.0f},
                a{255.0f}
            {}
            Color(float v):
                Color(v, v, v)
            {}
            Color(float _r, float _g, float _b):
                Color(_r, _g, _b, 255.0f)
            {}
            Color(float _r, float _g, float _b, float _a):
                r{_r},
                g{_g},
                b{_b},
                a{_a}
            {}

            bool operator == (const Color& c) const {
                return r == c.r &&
                       g == c.g &&
                       b == c.b &&
                       a == c.a;
            }

            float r, g, b, a;
        };
    }
}