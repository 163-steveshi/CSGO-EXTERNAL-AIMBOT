#pragma once
#include <numbers>
#include <cmath>
#include <iostream>
using namespace std;

struct Vector3
{ 
  //constructor of the vector3
  //an expression or function can be evaluated at compile time.
  constexpr Vector3(
    const float x = 0.f,
    const float y = 0.f,
    const float z = 0.f) noexcept : //expect no exceptation occurs
    x(x) , y(y), z(z) {}
  // do the operation overloading for the vector3
  constexpr const Vector3& operator-(const Vector3& v) const noexcept
  {
    return Vector3{x - v.x, y - v.y, z - v.z};

  }
  constexpr const Vector3& operator+(const Vector3& v) const noexcept
  {
    return Vector3{x + v.x, y + v.y, z + v.z};
  }

  constexpr const Vector3& operator*(const float factor) const noexcept
  {
    return Vector3{x *  factor, y * factor, z * factor};
  }
  constexpr const Vector3& operator/(const float factor) const noexcept
  {
    return Vector3{x /  factor, y / factor, z / factor};
  }

  //determine angle function
  constexpr const Vector3& ToAngle() const noexcept
  {

    return Vector3{
			atan2(-z, hypot(x, y)) * (180.0f / numbers::pi_v<float>),
			atan2(y, x) * (180.0f / numbers::pi_v<float>),
			0.0f };

  }

  //helper function that checks vector has changed or not
  constexpr const bool IsZero() const noexcept
  {

    return x == 0.f && y == 0.f && z == 0.f;
  }
  float x, y, z;
};