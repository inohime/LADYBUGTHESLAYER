/**
 * Created by: Inohime @ 11/6/2021
 */

#pragma once

#include <iostream>

namespace gmtk {
	template <typename T>
	class Vector2 {
	public:
		Vector2() : x(0), y(0) {}

		Vector2(T x, T y) : x(x), y(y) {}

		T DotProduct(const Vector2 &left, const Vector2 &right);

	public:
		T x, y;
	};

	template <typename T>
	inline T Vector2<T>::DotProduct(const Vector2 &left, const Vector2 &right) {
		return (left.x * right.x + left.y * right.y);
	}

	template <typename T>
	inline Vector2<T> operator+(const Vector2<T> &left, const Vector2<T> &right) noexcept {
		return Vector2<T>(left.x + right.x, left.y + right.y);
	}

	template <typename T>
	inline Vector2<T> operator-(const Vector2<T> &left, const Vector2<T> &right) noexcept {
		return Vector2<T>(left.x - right.x, left.y - right.y);
	}

	template <typename T>
	inline Vector2<T> operator*(const Vector2<T> &left, T right) noexcept {
		return Vector2<T>(left.x * right, left.y * right);
	}

	template <typename T>
	inline Vector2<T> operator/(const Vector2<T> &left, T right) noexcept {
		return Vector2<T>(left.x / right, left.y / right);
	}

   /**
	* Adds left's x / y with right value
    */
	template <typename T>
	inline Vector2<T> operator+=(Vector2<T> &left, const Vector2<T> &right) noexcept {
		return Vector2<T>(left.x += right.x, left.y += right.y);
	}

   /**
	* Subtracts left's x / y with right value
    */
	template <typename T>
	inline Vector2<T> operator-=(Vector2<T> &left, T right) noexcept {
		return Vector2<T>(left.x -= right, left.y -= right);
	}

   /**
	* Multiplies left's x / y with right value
    */
	template <typename T>
	inline Vector2<T> operator*=(Vector2<T> &left, T right) noexcept {
		return Vector2<T>(left.x *= right, left.y *= right);
	}

   /**
	* Divides left's x / y with right value
    */
	template <typename T>
	inline Vector2<T> operator/=(Vector2<T> &left, T right) noexcept {
		return Vector2<T>(left.x /= right, left.y /= right);
	}

	template <typename T>
	std::ostream &operator<<(std::ostream &os, const Vector2<T> &Vec2) {
		os << "[" << Vec2.x << ", " << Vec2.y << "]";
		return os;
	}

	using vec2i = Vector2<int>;
	using vec2d = Vector2<double>;
	using vec2f = Vector2<float>;
	using vec2u = Vector2<unsigned int>;
}