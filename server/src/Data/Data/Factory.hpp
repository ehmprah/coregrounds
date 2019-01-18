#pragma once

#include "Device.hpp"
#include "Core/TargetPreference.hpp"

namespace game::data
{
	struct Factory : public Device
	{
	public:
		struct Upgrades
		{
			std::array<std::array<ID, device::MaxFactoryUpgrades>, device::MaxFactoryUpgradeTypes> spellIDs;
			std::array<std::string, device::MaxFactoryUpgradeTypes> names;
		};

		struct Wall
		{
		public:
			prop::Property<ID> id { prop::Id };
		};

		struct Minion
		{
		private:
			Upgrades m_Upgrades;
			TargetPreference m_TargetPreference;

		public:
			prop::Property<ID> id { prop::Id };
			prop::Property<Stat> xpPerSecond { "xp_per_second", 0, prop::GreaterEqualZero };

			void setUpgrades(Upgrades _upgrades);
			const Upgrades& getUpgrades() const;

			void setTargetPreference(TargetPreference _pref);
			const TargetPreference& getTargetPreference() const;
		};

		struct Tower
		{
		private:
			Upgrades m_Upgrades;
			TargetPreference m_TargetPreference;

		public:
			prop::Property<ID> id { prop::Id };
			prop::Property<Stat> xpPerSecond { "xp_per_second", 0, prop::GreaterEqualZero };

			void setUpgrades(Upgrades _upgrades);
			const Upgrades& getUpgrades() const;

			void setTargetPreference(TargetPreference _pref);
			const TargetPreference& getTargetPreference() const;
		};
		using Data = std::variant<std::monostate, Wall, Minion, Tower>;

	private:
		using super = Device;

		template <class T>
		using _IsDataType = std::disjunction<std::is_same<T, Wall>, std::is_same<T, Minion>, std::is_same<T, Tower>>;
		template <class T>
		using IsDataType = _IsDataType<std::remove_cv_t<T>>;

		Data m_Data;

	public:
		template <class TData, typename = std::enable_if_t<IsDataType<TData>::value>>
		void setData(TData&& _data)
		{
			m_Data = std::forward<TData>(_data);
		}

		template <class TData, typename = std::enable_if_t<IsDataType<TData>::value>>
		const TData& getData() const
		{
			return std::get<TData>(m_Data);
		}

		template <class TData, typename = std::enable_if_t<IsDataType<TData>::value>>
		bool hasData() const
		{
			return std::holds_alternative<TData>(m_Data);
		}

		bool isEmpty() const;

		std::string_view getTypeName() const override;
	};
} // namespace game::data
