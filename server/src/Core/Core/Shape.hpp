#pragma once

#include "SLInclude.hpp"
#include "StdInclude.hpp"

namespace game
{
	template <class T>
	class Shape
	{
	private:
		using Data = std::variant<sl2::geometry2d::Rect<T>, sl2::geometry2d::Circle<T>>;
		Data m_Data;

		template <class TShape>
		friend class BiAlgorithmVisitor;

		template <class TShape>
		class BiAlgorithmVisitor
		{
		private:
			const TShape& m_Shape;

		public:
			BiAlgorithmVisitor(const TShape& _shape) :
				m_Shape(_shape)
			{}

			template <class Other, typename BinaryAlgorithm>
			auto rExecute(const Other& _obj, BinaryAlgorithm _algorithm)
			{
				return _algorithm(_obj, m_Shape);
			}

			template <class Other, typename BinaryAlgorithm>
			auto execute(const Other& _obj, BinaryAlgorithm _algorithm)
			{
				return _algorithm(m_Shape, _obj);
			}

			template <class T2, typename BinaryAlgorithm>
			auto execute(const Shape<T2>& _obj, BinaryAlgorithm _algorithm)
			{
				return _obj._exec<TShape, BinaryAlgorithm, true>(m_Shape, _algorithm);
			}
		};

		template <typename Algorithm>
		auto _exec(Algorithm _algorithm) const
		{
			using namespace sl2::geometry2d;
			if (std::holds_alternative<Rect<T>>(m_Data))
				return _algorithm(std::get<Rect<T>>(m_Data));
			if (std::holds_alternative<Circle<T>>(m_Data))
				return _algorithm(std::get<Circle<T>>(m_Data));
			else
				throw std::runtime_error("Shape::_exec: Invalid type");
		}

		template <class Other, typename BinaryAlgorithm, bool Reverse = false>
		auto _exec(const Other& _rhs, BinaryAlgorithm _algorithm) const
		{
			return _exec([&_rhs, _algorithm](const auto& _shape) {
				BiAlgorithmVisitor<std::decay_t<decltype(_shape)>> vis(_shape);
				if constexpr (Reverse)
					return vis.rExecute(_rhs, _algorithm);
				return vis.execute(_rhs, _algorithm);
			});
		}

	public:
		Shape() = default;

		template <class TShape, typename = std::enable_if_t<std::is_convertible_v<TShape, Data>>>
		Shape(TShape _shape) :
			m_Data(std::move(_shape))
		{
		}

		template <class TShape, typename = std::enable_if_t<std::is_convertible_v<TShape, Data>>>
		void setShape(TShape _shape)
		{
			m_Data = std::move(_shape);
		}

		template <class Other>
		bool overlaps(const Other& _other) const
		{
			return _exec(_other, [](const auto& _lhs, const auto& _rhs) {
				return sl2::geometry2d::overlaps(_lhs, _rhs);
			});
		}

		template <class Other>
		bool intersects(const Other& _other) const
		{
			return _exec(_other, [](const auto& _lhs, const auto& _rhs) {
				return sl2::geometry2d::intersects(_lhs, _rhs);
			});
		}

		template <class Other>
		bool contains(const Other& _other) const
		{
			return _exec(_other, [](const auto& _lhs, const auto& _rhs) {
				return sl2::geometry2d::contains(_lhs, _rhs);
			});
		}

		template <class Other>
		bool containedBy(const Other& _other) const
		{
			return _exec(_other, [](const auto& _lhs, const auto& _rhs) {
				return sl2::geometry2d::contains(_rhs, _lhs);
			});
		}

		std::optional<T> calculateTouchingDistance(const Shape& _other, const sl2::geometry2d::Vector<T>& _vec) const
		{
			using namespace sl2::geometry2d;
			return _exec(_other, [_vec](const auto& _lhs, const auto& _rhs) -> std::optional<T> {
				if constexpr (std::is_convertible_v<decltype(_lhs), Circle<T>> &&
					std::is_convertible_v<decltype(_rhs), Circle<T>>)
				{

					assert(!_lhs.isNull() && !_rhs.isNull());
					Line<T> lTmp(_lhs.getCenter(), _rhs.getCenter() + _vec, LineType::ray);
					Circle<T> cTmp(_rhs.getCenter(), _lhs.getRadius() + _rhs.getRadius());
					std::optional<T> minDistance;
					auto intersector = createIntersector(lTmp, cTmp);
					while (auto intersection = intersector.next())
					{
						auto newDist = sl2::calculateDistanceSquared(_lhs.getCenter(), *intersection);
						if (!minDistance || newDist < *minDistance)
							minDistance = newDist;
					}

					// we stored the distance squared, if its set, we have to calculate sqrt
					if (minDistance)
						minDistance = std::sqrt(*minDistance);
					return minDistance;
				}
				throw std::runtime_error("Shape::calculateTouchingDistance: Invalid type");
			});
			return {};
		}

		void setCenter(const sl2::geometry2d::Vector<T>& _pos)
		{
			using namespace sl2::geometry2d;
			if (std::holds_alternative<Rect<T>>(m_Data))
			{
				auto& rect = std::get<Rect<T>>(m_Data);
				assert(!rect.isNull());
				rect.setTopLeft(_pos - *rect.getSize() / 2);
			}
			else if (std::holds_alternative<Circle<T>>(m_Data))
			{
				auto& circle = std::get<Circle<T>>(m_Data);
				assert(!circle.isNull());
				circle.setCenter(_pos);
			}
			else
				throw std::runtime_error("Shape::setCenter: Invalid type");
		}

		sl2::geometry2d::Vector<T> getCenter() const
		{
			return _exec([](const auto& _shape) {
				return sl2::geometry2d::calculateCenter(_shape);
			});
		}

		sl2::geometry2d::Rect<T> createBoundingRect() const
		{
			return _exec([](const auto& _shape) {
				return sl2::geometry2d::generateBoundingRect(_shape);
			});
		}

		//sl2::geometry2d::Vector<T> calculateNearestPoint(const sl2::geometry2d::Vector<T>& _p) const
		//{
		//	throw std::runtime_error("Shape::CalculateNearestPoint: not implemented.");
		//}

		//T calculateDistanceSquared(const sl2::geometry2d::Vector<T>& _p) const
		//{
		//	return calculateNearestPoint(_p).calculateDistanceSquared(_p);
		//}

		//T calculateDistance(const sl2::geometry2d::Vector<T>& _p) const
		//{
		//	return calculateNearestPoint(_p).calculateDistance(_p);
		//}
	};
} // namespace game
