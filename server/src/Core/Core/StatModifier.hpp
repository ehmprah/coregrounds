#pragma once

#include "Core/StatContainer.hpp"
#include "Core/HelperFunctions.hpp"
#include "Core/StdInclude.hpp"

namespace game
{
	template <class Type>
	class StatModifier :
		sl2::NonCopyable
	{
	private:
		class ListInterface
		{
		public:
			~ListInterface() = default;

			virtual bool hasNegativeMod(Type _type) = 0;
			virtual void add(Type _type, Stat _value) = 0;
			virtual void remove(Type _type, Stat _value) = 0;
			virtual Stat get(Type _type) = 0;
		};

		class ValueList
		{
		public:
			bool hasNegativeMod(Type _type)
			{
				auto& statList = m_List[detail::_mapToIndex(_type)];
				return statList.empty() ? false : statList.front() < 0;
			}

			void add(Type _type, Stat _value)
			{
				auto& statList = m_List[detail::_mapToIndex(_type)];
				statList.insert(std::lower_bound(std::begin(statList), std::end(statList), _value), _value);
			}

			void remove(Type _type, Stat _value)
			{
				auto& statList = m_List[detail::_mapToIndex(_type)];
				auto itr = std::find_if(std::begin(statList), std::end(statList),
					[_value](Stat _other) { return sl2::isAlmostEqual(_value, _other); }
				);
				assert(itr != std::end(statList));
				statList.erase(itr);
			}

			Stat getHighLow(Type _type)
			{
				auto& statList = m_List[detail::_mapToIndex(_type)];
				if (statList.empty())
					return 0;
				else if (statList.size() == 1)
					return statList.back();
				Stat value = 0;
				if (statList.front() < 0)
					value += statList.front();
				if (statList.back() > 0)
					value += statList.back();
				return value;
			}

			Stat getAccumulated(Type _type)
			{
				auto& statList = m_List[detail::_mapToIndex(_type)];
				return std::accumulate(std::begin(statList), std::end(statList), Stat(0));
			}

		private:
			using List = std::array<std::vector<Stat>, static_cast<std::size_t>(Type::max) - 1>;
			List m_List{};
		};

		class AccumulateList :
			public ListInterface
		{
		private:
			ValueList m_List{};
			StatContainer<Type> m_Cache;

		public:
			bool hasNegativeMod(Type _type) override
			{
				return m_List.hasNegativeMod(_type);
			}

			void add(Type _type, Stat _value) override
			{
				m_List.add(_type, _value);
				m_Cache.setStat(_type, m_List.getAccumulated(_type));
			}

			void remove(Type _type, Stat _value) override
			{
				m_List.remove(_type, _value);
				m_Cache.setStat(_type, m_List.getAccumulated(_type));
			}

			Stat get(Type _type) override
			{
				return m_Cache.getStat(_type);
			}
		};

		class HighLowList :
			public ListInterface
		{
		private:
			ValueList m_List;

		public:
			bool hasNegativeMod(Type _type) override
			{
				return m_List.hasNegativeMod(_type);
			}

			void add(Type _type, Stat _value) override
			{
				m_List.add(_type, _value);
			}

			void remove(Type _type, Stat _value) override
			{
				m_List.remove(_type, _value);
			}

			Stat get(Type _type) override
			{
				return m_List.getHighLow(_type);
			}
		};

		using Lists = std::array<std::unique_ptr<ListInterface>, static_cast<std::size_t>(StatModifierGroup::max) - 1>;
		Lists m_Flats
		{
			std::make_unique<AccumulateList>(),			// modifications
			std::make_unique<AccumulateList>(),			// upgrades
			std::make_unique<HighLowList>()				// buffs
		};

		Lists m_Percents
		{
			std::make_unique<AccumulateList>(),			// modifications
			std::make_unique<AccumulateList>(),			// upgrades
			std::make_unique<HighLowList>()				// buffs
		};

	public:
		StatModifier()
		{
			assert(std::none_of(std::begin(m_Flats), std::end(m_Flats), [](const auto& _list) { return !_list; }));
			assert(std::none_of(std::begin(m_Percents), std::end(m_Percents), [](const auto& _list) { return !_list; }));
		}

		Stat getModPercent(Type _stat) const
		{
			return std::accumulate(std::begin(m_Percents), std::end(m_Percents), Stat(0),
				[_stat](Stat _value, const auto& _list) { return _value + _list->get(_stat); }
			);
		}

		Stat getModFlat(Type _stat) const
		{
			return std::accumulate(std::begin(m_Flats), std::end(m_Flats), Stat(0),
				[_stat](Stat _value, const auto& _list) { return _value + _list->get(_stat); }
			);
		}

		// none as grp is allowed!
		bool hasNegativeMod(Type _stat, StatModifierGroup _grp) const
		{
			if (_grp == StatModifierGroup::none)
			{
				auto hasNegativeMod = [_stat](const auto& _list)
				{
					return std::any_of(std::begin(_list), std::end(_list),
						[_stat](const auto& _list)
						{
							assert(_list);
							return _list->hasNegativeMod(_stat);
						}
					);
				};
				return hasNegativeMod(m_Flats) || hasNegativeMod(m_Percents);
			}
			auto index = detail::_mapToIndex(_grp);
			return m_Flats[index]->hasNegativeMod(_stat) ||
				m_Percents[index]->hasNegativeMod(_stat);
		}

		Stat modifyValue(Type _stat, Stat _value) const
		{
			return modifyValue(_value, getModPercent(_stat), getModFlat(_stat));
		}

		void applyPercentMod(Type _stat, StatModifierGroup _grp, Stat _value)
		{
			auto& list = m_Percents[detail::_mapToIndex(_grp)];
			assert(list);
			list->add(_stat, _value);
		}

		void removePercentMod(Type _stat, StatModifierGroup _grp, Stat _value)
		{
			auto& list = m_Percents[detail::_mapToIndex(_grp)];
			assert(list);
			list->remove(_stat, _value);
		}

		void applyFlatMod(Type _stat, StatModifierGroup _grp, Stat _value)
		{
			auto& list = m_Flats[detail::_mapToIndex(_grp)];
			assert(list);
			list->add(_stat, _value);
		}

		void removeFlatMod(Type _stat, StatModifierGroup _grp, Stat _value)
		{
			auto& list = m_Flats[detail::_mapToIndex(_grp)];
			assert(list);
			list->remove(_stat, _value);
		}

		static Stat modifyValue(Stat _value, Stat _modPercent, Stat _modFlat)
		{
			auto mod = _value;
			mod += _value * _modPercent;
			mod += _modFlat;
			return std::max<Stat>(mod, 0);
		}
	};
} // namespace game
