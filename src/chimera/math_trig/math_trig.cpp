// SPDX-License-Identifier: GPL-3.0-only

#include <cmath>

#include "math_trig.hpp"

namespace Chimera {
    ColorRGB::ColorRGB(float r, float g, float b) noexcept :
        red(r),
        green(g),
        blue(b) {}
    ColorRGB::ColorRGB(const ColorByte &other) noexcept :
        red(static_cast<float>(other.red) / 255),
        green(static_cast<float>(other.green) / 255),
        blue(static_cast<float>(other.blue) / 255) {}
    ColorRGB::ColorRGB(const ColorARGB &other) noexcept :
        red(other.red),
        green(other.green),
        blue(other.blue) {}

    ColorARGB::ColorARGB(float a, float r, float g, float b) noexcept :
        alpha(a),
        red(r),
        green(g),
        blue(b) {}
    ColorARGB::ColorARGB(const ColorByte &other) noexcept :
        alpha(static_cast<float>(other.alpha) / 255),
        red(static_cast<float>(other.red) / 255),
        green(static_cast<float>(other.green) / 255),
        blue(static_cast<float>(other.blue) / 255) {}
    ColorARGB::ColorARGB(const ColorRGB &other) noexcept :
        red(other.red),
        green(other.green),
        blue(other.blue) {}

    ColorByte::ColorByte(float a, float r, float g, float b) noexcept :
        blue(static_cast<unsigned char>(b * 255)),
        green(static_cast<unsigned char>(g * 255)),
        red(static_cast<unsigned char>(r * 255)),
        alpha(static_cast<unsigned char>(a * 255)) {}
    ColorByte::ColorByte(unsigned char a, unsigned char r, unsigned char g, unsigned char b) noexcept :
        blue(b),
        green(g),
        red(r),
        alpha(a) {}
    ColorByte::ColorByte(const ColorRGB &other) noexcept :
        ColorByte::ColorByte(1.0f, other.red, other.green, other.blue) {}
    ColorByte::ColorByte(const ColorARGB &other) noexcept :
        ColorByte::ColorByte(other.alpha, other.red, other.green, other.blue) {}

    // http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/index.htm
    Quaternion::Quaternion(const RotationMatrix &matrix) noexcept {
        float tr = matrix.v[0].x + matrix.v[1].y + matrix.v[2].z;
        if(tr > 0) {
            float S = std::sqrt(tr+1.0f) * 2.0f; // S=4*qw
            this->w = 0.25f * S;
            this->x = (matrix.v[2].y - matrix.v[1].z) / S;
            this->y = (matrix.v[0].z - matrix.v[2].x) / S;
            this->z = (matrix.v[1].x - matrix.v[0].y) / S;
        }
        else if((matrix.v[0].x > matrix.v[1].y) & (matrix.v[0].x > matrix.v[2].z)) {
            float S = std::sqrt(1.0f + matrix.v[0].x - matrix.v[1].y - matrix.v[2].z) * 2.0f; // S=4*qx
            this->w = (matrix.v[2].y - matrix.v[1].z) / S;
            this->x = 0.25f * S;
            this->y = (matrix.v[0].y + matrix.v[1].x) / S;
            this->z = (matrix.v[0].z + matrix.v[2].x) / S;
        } else if(matrix.v[1].y > matrix.v[2].z) {
            float S = std::sqrt(1.0f + matrix.v[1].y - matrix.v[0].x - matrix.v[2].z) * 2.0f; // S=4*qy
            this->w = (matrix.v[0].z - matrix.v[2].x) / S;
            this->x = (matrix.v[0].y + matrix.v[1].x) / S;
            this->y = 0.25f * S;
            this->z = (matrix.v[1].z + matrix.v[2].y) / S;
        } else {
            float S = std::sqrt(1.0f + matrix.v[2].z - matrix.v[0].x - matrix.v[1].y) * 2.0f; // S=4*qz
            this->w = (matrix.v[1].x - matrix.v[0].y) / S;
            this->x = (matrix.v[0].z + matrix.v[2].x) / S;
            this->y = (matrix.v[1].z + matrix.v[2].y) / S;
            this->z = 0.25f * S;
        }
    }

    RotationMatrix::RotationMatrix() noexcept {}

    // http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/index.htm
    RotationMatrix::RotationMatrix(const Quaternion &quaternion) noexcept {
        float sqw = quaternion.w*quaternion.w;
        float sqx = quaternion.x*quaternion.x;
        float sqy = quaternion.y*quaternion.y;
        float sqz = quaternion.z*quaternion.z;

        // invs (inverse square length) is only required if quaternion is not already normalised
        float invs = 1.0f / (sqx + sqy + sqz + sqw);
        this->v[0].x = ( sqx - sqy - sqz + sqw)*invs; // since sqw + sqx + sqy + sqz =1/invs*invs
        this->v[1].y = (-sqx + sqy - sqz + sqw)*invs;
        this->v[2].z = (-sqx - sqy + sqz + sqw)*invs;

        float tmp1 = quaternion.x*quaternion.y;
        float tmp2 = quaternion.z*quaternion.w;
        this->v[1].x = 2.0f * (tmp1 + tmp2)*invs;
        this->v[0].y = 2.0f * (tmp1 - tmp2)*invs;

        tmp1 = quaternion.x*quaternion.z;
        tmp2 = quaternion.y*quaternion.w;
        this->v[2].x = 2.0f * (tmp1 - tmp2)*invs;
        this->v[0].z = 2.0f * (tmp1 + tmp2)*invs;
        tmp1 = quaternion.y*quaternion.z;
        tmp2 = quaternion.x*quaternion.w;
        this->v[2].y = 2.0f * (tmp1 + tmp2)*invs;
        this->v[1].z = 2.0f * (tmp1 - tmp2)*invs;
    }

    // special thanks to MosesOfEgypt for the rotation interpolation stuff here
    void interpolate_quat(const Quaternion &in_before, const Quaternion &in_after, Quaternion &out, float scale) noexcept {
        auto &w1 = in_before.w;
        auto &x1 = in_before.x;
        auto &y1 = in_before.y;
        auto &z1 = in_before.z;
        auto w0 = in_after.w;
        auto x0 = in_after.x;
        auto y0 = in_after.y;
        auto z0 = in_after.z;
        float cos_half_theta = w0*w1 + x0*x1 + y0*y1 + z0*z1;
        if(cos_half_theta < 0) {
            w0*=-1;
            x0*=-1;
            y0*=-1;
            z0*=-1;
            cos_half_theta *= -1;
        }
        if(cos_half_theta < 0.01) return;

        float half_theta;
        if(std::fabs(cos_half_theta) >= 1.0) {
            half_theta = 0.0;
        }
        else {
            half_theta = std::acos(cos_half_theta);
        }

        float sin_half_theta = 0;
        float m = (1 - cos_half_theta*cos_half_theta);
        if(m > 0) sin_half_theta = m;

        float r0 = 1 - scale;
        float r1 = scale;
        if(sin_half_theta > 0.00001) {
            r0 = std::sin((1 - scale) * half_theta) / sin_half_theta;
            r1 = std::sin(scale * half_theta) / sin_half_theta;
        }

        out.w = w0*r1 + w1*r0;
        out.x = x0*r1 + x1*r0;
        out.y = y0*r1 + y1*r0;
        out.z = z0*r1 + z1*r0;
    }

    void interpolate_point(const Point3D &before, const Point3D &after, Point3D &output, float scale) noexcept {
        output.x = before.x + (after.x - before.x) * scale;
        output.y = before.y + (after.y - before.y) * scale;
        output.z = before.z + (after.z - before.z) * scale;
    }

    float distance_squared(float x1, float y1, float x2, float y2) noexcept {
        float x = x1 - x2;
        float y = y1 - y2;
        return x*x + y*y;
    }

    float distance_squared(float x1, float y1, float z1, float x2, float y2, float z2) noexcept {
        float x = x1 - x2;
        float y = y1 - y2;
        float z = z1 - z2;
        return x*x + y*y + z*z;
    }

    float distance_squared(const Point3D &a, const Point3D &b) noexcept {
        return distance_squared(a.x, a.y, a.z, b.x, b.y, b.z);
    }

    float distance_squared(const Point2D &a, const Point2D &b) noexcept {
        return distance_squared(a.x, a.y, b.x, b.y);
    }

    float distance(float x1, float y1, float x2, float y2) noexcept {
        return std::sqrt(distance_squared(x1, y1, x2, y2));
    }

    float distance(float x1, float y1, float z1, float x2, float y2, float z2) noexcept {
        return std::sqrt(distance_squared(x1, y1, z1, x2, y2, z2));
    }

    float distance(const Point3D &a, const Point3D &b) noexcept {
        return std::sqrt(distance_squared(a.x, a.y, a.z, b.x, b.y, b.z));
    }

    float distance(const Point2D &a, const Point3D &b) noexcept {
        return std::sqrt(distance_squared(a.x, a.y, b.x, b.y));
    }

    double counter_time_elapsed(const LARGE_INTEGER &before) noexcept {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        return counter_time_elapsed(before, now);
    }

    double counter_time_elapsed(const LARGE_INTEGER &before, const LARGE_INTEGER &after) noexcept {
        static LARGE_INTEGER performance_frequency = {};
        if(performance_frequency.QuadPart == 0) QueryPerformanceFrequency(&performance_frequency);
        return static_cast<double>(after.QuadPart - before.QuadPart) / performance_frequency.QuadPart;
    }
}
