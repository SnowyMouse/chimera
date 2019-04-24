#ifndef MATH_TRIG_HPP
#define MATH_TRIG_HPP

#include <windows.h>
#include "../version.hpp"

namespace Chimera {
    struct ColorARGB;
    struct ColorByte;

    struct ColorRGB {
        float red = 1.0;
        float green = 1.0;
        float blue = 1.0;

        ColorRGB() noexcept;
        ColorRGB(float r, float g, float b) noexcept;
        ColorRGB(const ColorByte &other) noexcept;
        ColorRGB(const ColorARGB &other) noexcept;
        ColorRGB(const ColorRGB &other) noexcept;
    };

    struct ColorARGB {
        float alpha = 1.0;
        float red = 1.0;
        float green = 1.0;
        float blue = 1.0;

        ColorARGB() noexcept;
        ColorARGB(float a, float r, float g, float b) noexcept;
        ColorARGB(const ColorByte &other) noexcept;
        ColorARGB(const ColorRGB &other) noexcept;
        ColorARGB(const ColorARGB &other) noexcept;
    };

    struct ColorByte {
        unsigned char blue;
        unsigned char green;
        unsigned char red;
        unsigned char alpha = 255;

        ColorByte() noexcept;
        ColorByte(float a, float r, float g, float b) noexcept;
        ColorByte(unsigned char a, unsigned char r, unsigned char g, unsigned char b) noexcept;
        ColorByte(const ColorRGB &other) noexcept;
        ColorByte(const ColorARGB &other) noexcept;
        ColorByte(const ColorByte &other) noexcept;
    };

    struct Point3D {
        float x;
        float y;
        float z;
    };

    struct Point2D {
        float x;
        float y;
    };

    struct Euler3DPYR {
        float pitch;
        float yaw;
        float roll;
    };

    struct Euler3DYPR {
        float yaw;
        float pitch;
        float roll;
    };

    struct RotationMatrix;
    struct Quaternion {
        float x;
        float y;
        float z;
        float w;
        Quaternion() noexcept;
        Quaternion(const RotationMatrix &matrix) noexcept;
        Quaternion(const Quaternion &copy) noexcept;
    };

    struct RotationMatrix {
        Point3D v[3];
        RotationMatrix() noexcept;
        RotationMatrix(const Quaternion &quaternion) noexcept;
        RotationMatrix(const RotationMatrix &copy) noexcept;
    };

    /**
     * Interpolate a quaternion.
     * @param in_before This is the quaternion to interpolate from.
     * @param in_after  This is the quaternion to interpolate to.
     * @param out       This is the quaternion to overwrite.
     * @param progress  This is how far in between each quaternion (0.0 - 1.0) to create an interpolated quaternion.
     */
    void interpolate_quat(const Quaternion &in_before, const Quaternion &in_after, Quaternion &out, float progress) noexcept;

    /**
     * Interpolate a 3D point.
     * @param in_before This is the 3D point to interpolate from.
     * @param in_after  This is the 3D point to interpolate to.
     * @param out       This is the 3D point to overwrite.
     * @param progress  This is how far in between each point (0.0 - 1.0) to create an interpolated 3D point.
     */
    void interpolate_point(const Point3D &before, const Point3D &after, Point3D &output, float scale) noexcept;

    /**
     * Calculate the distance between two 2D points without taking square roots. If the square root isn't necessary, then this is faster.
     * @param  x1 This is the X coordinate of the first point.
     * @param  y1 This is the Y coordinate of the first point.
     * @param  x2 This is the X coordinate of the second point.
     * @param  y2 This is the Y coordinate of the second point.
     * @return    Return the distance.
     */
    float distance_squared(float x1, float y1, float x2, float y2) noexcept;

    /**
     * Calculate the distance between two 2D points without taking square roots. If the square root isn't necessary, then this is faster.
     * @param  a This is the first point.
     * @param  b This is the second point.
     * @return   Return the distance squared.
     */
    float distance_squared(const Point2D &a, const Point2D &b) noexcept;

    /**
     * Calculate the distance between two 3D points without taking square roots. If the square root isn't necessary, then this is faster.
     * @param  x1 This is the X coordinate of the first point.
     * @param  y1 This is the Y coordinate of the first point.
     * @param  z1 This is the Z coordinate of the first point.
     * @param  x2 This is the X coordinate of the second point.
     * @param  y2 This is the Y coordinate of the second point.
     * @param  z2 This is the Z coordinate of the second point.
     * @return    Return the distance squared.
     */
    float distance_squared(float x1, float y1, float z1, float x2, float y2, float z2) noexcept;

    /**
     * Calculate the distance between two 3D points without taking square roots. If the square root isn't necessary, then this is faster.
     * @param  a This is the first point.
     * @param  b This is the second point.
     * @return   Return the distance squared.
     */
    float distance_squared(const Point3D &a, const Point3D &b) noexcept;

    /**
     * Calculate the distance between two 2D points.
     * @param  x1 This is the X coordinate of the first point.
     * @param  y1 This is the Y coordinate of the first point.
     * @param  x2 This is the X coordinate of the second point.
     * @param  y2 This is the Y coordinate of the second point.
     * @return    Return the distance.
     */
    float distance(float x1, float y1, float x2, float y2) noexcept;

    /**
     * Calculate the distance between two 2D points.
     * @param  a This is the first point.
     * @param  b This is the second point.
     * @return   Return the distance.
     */
    float distance(const Point2D &a, const Point2D &b) noexcept;

    /**
     * Calculate the distance between two 3D points.
     * @param  x1 This is the X coordinate of the first point.
     * @param  y1 This is the Y coordinate of the first point.
     * @param  z1 This is the Z coordinate of the first point.
     * @param  x2 This is the X coordinate of the second point.
     * @param  y2 This is the Y coordinate of the second point.
     * @param  z2 This is the Z coordinate of the second point.
     * @return    Return the distance.
     */
    float distance(float x1, float y1, float z1, float x2, float y2, float z2) noexcept;

    /**
     * Calculate the distance between two 3D points.
     * @param  a This is the first point.
     * @param  b This is the second point.
     * @return   Return the distance.
     */
    float distance(const Point3D &a, const Point3D &b) noexcept;
}

#endif
