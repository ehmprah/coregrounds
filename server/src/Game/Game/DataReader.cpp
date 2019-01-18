#include "stdafx.h"
#include "DataReader.hpp"
#include "Core/HelperFunctions.hpp"
#include "Core/log.hpp"
#include "Data/Others.hpp"
#include "Data/DataStringHelper.hpp"
#include "Data/Spell.hpp"
#include "Data/Aura.hpp"
#include "Data/Ability.hpp"
#include "Data/Factory.hpp"
#include "Data/Unit.hpp"
#include "PropertyHelper.hpp"

namespace game {
namespace data {

	ReaderPtr createReaderForFile(std::string_view _pathName)
	{
		std::ifstream in(_pathName.data(), std::ifstream::in);
		if (in.is_open() && in.good())
			return std::make_unique<JsonReader>(std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()));
		throw std::runtime_error("DataReader: file invalid or not found: " + std::string(_pathName));
	}

	Shape _createArea(const json::Object& _obj)
	{
		Shape area;
		if (auto areaObj = json::getElementPtr(_obj, "area"))
		{
			prop::assign(area.x, *areaObj);
			prop::assign(area.y, *areaObj);
			if (auto obj = json::getElementPtr(*areaObj, "circle"))
			{
				Shape::Circle data;
				prop::assign(data.radius, *obj);
				area.setData(std::move(data));
			}
			else if (auto obj = json::getElementPtr(*areaObj, "rect"))
			{
				Shape::Rect data;
				prop::assign(data.width, *obj);
				prop::assign(data.height, *obj);
				area.setData(std::move(data));
			}
		}
		return area;
	}

	std::optional<AreaModifier> _createAreaModifier(const json::Object& _obj)
	{
		if (auto areaObj = json::getElementPtr(_obj, "area"))
		{
			AreaModifier mod;
			if (auto modeString = json::getPtr<std::string>(*areaObj, "mode"))
			{
				if (*modeString == "set")
					mod.type = AreaModifier::Type::set;
				else if (*modeString == "mod")
					mod.type = AreaModifier::Type::mod;
				else
					LOG_CHANNEL_ERR(dataLogger, "AreaModifier: Invalid mode. Use default (set).");
			}
			mod.data = _createArea(*areaObj);
			return mod;
		}
		return {};
	}

	TargetPreference _createTargetPreference(const json::Object& _obj)
	{
		TargetPreference targetPreference;
		if (auto preferenceObj = json::getElementPtr(_obj, "default_target_preference"))
		{
			targetPreference.mode = parseTargetPreferenceMode(json::get<std::string>(*preferenceObj, "mode"));
			targetPreference.target.set_mask(0);
			if (auto array = json::getArrayPtr(*preferenceObj, "targets"))
			{
				for (auto& target : *array)
					targetPreference.target.apply(parseTargetPreferenceTarget(target.get_ref<const std::string&>()));
			}

			if (auto array = json::getArrayPtr(*preferenceObj, "flags"))
			{
				for (auto& target : *array)
					targetPreference.flags.apply(parseTargetPreferenceFlag(target.get_ref<const std::string&>()));
			}
		}
		return targetPreference;
	}

	std::vector<ID> _createInitAuras(const json::Object& _obj)
	{
		std::vector<ID> ids;
		if (auto auraObj = json::getArrayPtr(_obj, "auras"))
		{
			for (auto& aura : *auraObj)
				ids.emplace_back(aura.get<ID>());
		}
		return ids;
	}

	/*#####
	# GameProperty stuff
	#####*/
	UpgradeDurations _createUpgradeDurations(const json::Object& _root)
	{
		if (auto array = json::getArrayPtr(_root, "upgrade_durations"))
		{
			UpgradeDurations upgradeDurations;
			for (auto& element : *array)
				upgradeDurations.emplace_back(UpgradeDuration{ element.get<float>() });
			return upgradeDurations;
		}
		throw Error("JsonReader: \"upgrade_durations\" invalid or not found.");
	}

	XPCurve _createXPCurve(const json::Object& _root)
	{
		if (auto array = json::getArrayPtr(_root, "xp_curve"))
		{
			XPCurve xpCurve;
			for (auto& element : *array)
				xpCurve.emplace_back(element.get<Stat>());
			return xpCurve;
		}
		throw Error("JsonReader: \"xp_curve\" invalid or not found.");
	}

	template <class StatType>
	XpBoostModifier<StatType> _createXpBoostModifier(const json::Object& _root, std::string_view _nodeName, const XpBoostModifier<StatType>* _default = nullptr)
	{
		auto modifiers(_default ? *_default : XpBoostModifier<StatType>());
		if (auto obj = json::getElementPtr(_root, _nodeName))
		{
			for (std::size_t i = 1; i < static_cast<std::size_t>(StatType::max); ++i)
			{
				auto type = static_cast<StatType>(i);
				modifiers.modifiers[i - 1] = json::getValue(*obj, parseStatTypeToString(type), modifiers.getModifier(type));
			}
		}
		return modifiers;
	}

	template <class StatType>
	StatContainer<StatType> _createStats(const json::Object& _object, std::string_view _nodeName, const StatContainer<StatType>* _default = nullptr)
	{
		auto stats(_default ? *_default : StatContainer<StatType>());
		if (auto statObj = json::getElementPtr(_object, _nodeName))
		{
			const auto max = static_cast<std::size_t>(StatType::max);
			for (std::size_t i = 1; i < max; ++i)
			{
				auto type = static_cast<StatType>(i);
				stats.setStat(type, json::getValue(*statObj, parseStatTypeToString(type), stats.getStat(type)));
			}
		}
		return stats;
	}

	UnitModifiers _createUnitModifiers(const json::Object& _obj, std::string_view _nodeName, const UnitModifiers* _default = nullptr)
	{
		auto modifiers(_default ? *_default : UnitModifiers());
		if (auto modObj = json::getElementPtr(_obj, _nodeName))
		{
			auto& obj = *modObj;
			using Mod = UnitModifiers::Modifier;
			for (auto i = static_cast<std::size_t>(Mod::none) + 1; i < static_cast<std::size_t>(Mod::max); ++i)
			{
				auto mod = static_cast<Mod>(i);
				modifiers.setModifier(mod, json::getValue(obj, parseUnitModifierToString(mod), modifiers.getModifier(mod)));
			}
		}
		return modifiers;
	}

#ifndef NDEBUG
	std::optional<GameProperties::Game::Debug> _readGameDebugProperties(const json::Object& _obj)
	{
		if (json::get(_obj, "active", false))
		{
			GameProperties::Game::Debug debug;
			prop::assign(debug.emptyAI, _obj);

			if (auto playerArray = json::getArrayPtr(_obj, "players"))
			{
				std::size_t iPlayer = 0;
				for (auto& el : *playerArray)
				{
					if (!el.is_object() || iPlayer >= debug.players.size())
						break;
					auto& player = debug.players.at(iPlayer);
					if (auto wallLocArray = json::getArrayPtr(el, "wall_locations"))
					{
						for (auto& el : *wallLocArray)
						{
							if (el.is_array() && el.size() >= 2)
							{
								player.wallLocations.emplace_back(el[0].get<int>(), el[1].get<int>());
							}
						}
					}
					++iPlayer;
				}
				if (iPlayer != debug.players.size())
					throw std::runtime_error("GameProperties: debug settings is active; player error.");
			}
			return debug;
		}
		return std::nullopt;
	}

	std::optional<GameProperties::Pick::Debug> _readPickDebugProperties(const json::Object& _obj)
	{
		if (json::get(_obj, "active", false))
		{
			GameProperties::Pick::Debug debug;
			if (auto playerArray = json::getArrayPtr(_obj, "players"))
			{
				std::size_t iPlayer = 0;
				for (auto& el : *playerArray)
				{
					if (!el.is_object() || iPlayer >= debug.players.size())
						break;
					auto& player = debug.players.at(iPlayer);
					for (auto& factory : player.factories)
					{
						prop::assign(factory, el);
						if (*factory == 0)
							throw std::runtime_error("GameProperties: debug settings is active, but at least one factory is missing or invalid.");
					}

					for (auto& ability : player.abilities)
					{
						prop::assign(ability, el);
						if (*ability == 0)
							throw std::runtime_error("GameProperties: debug settings is active, but at least one ability is missing or invalid.");
					}
					++iPlayer;
				}
				if (iPlayer != debug.players.size())
					throw std::runtime_error("GameProperties: debug settings is active; player error.");
			}
			return debug;
		}
		return std::nullopt;
	}
#endif

	GameProperties JsonReader::readGameProperties()
	{
		LOG_CHANNEL_INFO(dataLogger, "Begin reading game properties");
		GameProperties properties;
		if (auto propertyObj = json::getElementPtr(_getRootObject(), "properties"))
		{
			LOG_CHANNEL_INFO(dataLogger, "Begin reading game properties: section game");
			if (auto gameObj = json::getElementPtr(*propertyObj, "game"))
			{
				auto& gProp = properties.game;
				auto gObj = *gameObj;
				prop::assign(gProp.minionLimit, gObj);
				prop::assign(gProp.minionLimitExplodeSpellID, gObj);
				prop::assign(gProp.minionCoreSuicideSpellID, gObj);
				prop::assign(gProp.xpPerLevel, gObj);
				prop::assign(gProp.xpHandicapPerUpgrade, gObj);
				prop::assign(gProp.maxLevel, gObj);
				prop::assign(gProp.wallChainArmorBonus, gObj);
				prop::assign(gProp.maxWallChain, gObj);
				prop::assign(gProp.initialWalls, gObj);
				prop::assign(gProp.unitRegenerationInterval, gObj);
				prop::assign(gProp.xpPerSecond, gObj);
				prop::assign(gProp.warumUpTime, gObj);
				prop::assign(gProp.towerDeathCooldown, gObj);

				//gProp.upgradeDurations = _createUpgradeDurations(gObj);
				gProp.xpCurve = _createXPCurve(gObj);
				gProp.deviceXpBoostModifier = _createXpBoostModifier<device::StatType>(gObj, "device_xp_boost_modifiers");
				gProp.unitXpBoostModifier = _createXpBoostModifier<unit::StatType>(gObj, "unit_xp_boost_modifiers");
				gProp.unitModifiers = _createUnitModifiers(gObj, "unit_modifiers");
				gProp.unitStats = _createStats<unit::StatType>(gObj, "unit_stats");
				gProp.deviceStats = _createStats<device::StatType>(gObj, "device_stats");

#ifndef NDEBUG
				if (auto debugObj = json::getElementPtr(gObj, "debug"))
					gProp.debug = _readGameDebugProperties(*debugObj);
#endif
			}
			LOG_CHANNEL_INFO(dataLogger, "Finished reading game properties: section game");

			LOG_CHANNEL_INFO(dataLogger, "Begin reading game properties: section pick");
			if (auto pickObj = json::getElementPtr(*propertyObj, "pick"))
			{
				auto& pProp = properties.pick;
				auto pObj = *pickObj;
				prop::assign(pProp.maxTime, pObj);
				prop::assign(pProp.modPageSelectionTime, pObj);
				prop::assign(pProp.maxAbilityBans, pObj);
				prop::assign(pProp.maxFactoryBans, pObj);

#ifndef NDEBUG
				if (auto debugObj = json::getElementPtr(pObj, "debug"))
					pProp.debug = _readPickDebugProperties(*debugObj);
#endif
			}
			LOG_CHANNEL_INFO(dataLogger, "Finished reading game properties: section pick");

			LOG_CHANNEL_INFO(dataLogger, "Begin reading game properties: section finalize");
			if (auto finalizeObj = json::getElementPtr(*propertyObj, "finalize"))
			{
				auto& fProp = properties.finalize;
				auto fObj = *finalizeObj;
				prop::assign(fProp.phaseTime, fObj);
			}
			LOG_CHANNEL_INFO(dataLogger, "Finished reading game properties: section finalize");
		}
		return properties;
	}

	/*#####
	# spell stuff
	#####*/
	std::optional<sl2::Bitmask<spell::SpellFlags>> _createSpellFlags(const json::Object& _obj)
	{
		if (auto flagArray = json::getArrayPtr(_obj, "flags"))
		{
			sl2::Bitmask<spell::SpellFlags> flags;
			for (auto& value : *flagArray)
			{
				if (value.is_string())
					flags.apply(parseSpellFlag(value.get_ref<const std::string&>()));
			}
			return flags;
		}
		return{};
	}

	std::optional<Spell::Effect> _createSpellEffect(const json::Object& _obj)
	{
		Spell::Effect effect;
		using game::spell::EffectType;
		// setup effect type
		auto& effectString = json::get<std::string>(_obj, "type");
		effect.type = parseSpellEffectType(effectString);

		using Effect = data::Spell::Effect;
		switch (effect.type)
		{
		case EffectType::applyAura:
		case EffectType::spawnMinion:
		case EffectType::spawnTrigger:
		{
			Effect::Id data;
			prop::assign(data.value, _obj);
			effect.setData(std::move(data));
			break;
		}
		case EffectType::healing:
		case EffectType::damage:
		{
			Effect::Value data;
			prop::assign(data.value, _obj);
			effect.setData(std::move(data));
			break;
		}
		case EffectType::healthAsDamage:
		{
			Effect::Percentage data;
			prop::assign(data.value, _obj);
			effect.setData(std::move(data));
			break;
		}
		case EffectType::triggerSpell:
		{
			Effect::TriggerSpell data;
			prop::assign(data.id, _obj);
			prop::assign(data.forward, _obj);
			effect.setData(std::move(data));
			break;
		}
		}
		return effect;
	}

	std::vector<Spell::Effect> _createSpellEffectData(const json::Object& _obj)
	{
		std::vector<game::data::Spell::Effect> effects;
		if (auto array = json::getArrayPtr(_obj, "effects"))
		{
			for (auto& effectRef : *array)
			{
				if (effectRef.is_object())
				{
					if (auto effect = _createSpellEffect(effectRef))
						effects.emplace_back(std::move(*effect));
				}
			}
		}
		return effects;
	}

	std::optional<sl2::Bitmask<spell::TargetFlags>> _createTargetMask(const json::Object& _obj, const std::string& _name)
	{
		if (auto array = json::getArrayPtr(_obj, _name))
		{
			using TargetFlags = spell::TargetFlags;
			sl2::Bitmask<TargetFlags> mask;
			for (auto& value : *array)
			{
				if (value.is_string())
					mask.apply(parseSpellTargetFlag(value.get_ref<const std::string&>()));
			}
			return mask;
		}
		return{};
	}

	std::optional<Spell::Target> _createTargetData(const json::Object& _obj)
	{
		if (auto targetObj = json::getElementPtr(_obj, "targets"))
		{
			Spell::Target target;
			prop::assign(target.count, *targetObj);
			target.friendlyMask.set_mask(0);
			target.hostileMask.set_mask(0);

			if (auto maskObj = json::getElementPtr(*targetObj, "mask"))
			{
				if (auto mask = _createTargetMask(*maskObj, "friendly"))
					target.friendlyMask = *mask;
				if (auto mask = _createTargetMask(*maskObj, "hostile"))
					target.hostileMask = *mask;
			}
			return target;
		}
		return{};
	}

	Spell _createSpell(const json::Object& _obj, const GameProperties& _gameProp)
	{
		Spell spell;
		prop::assign(spell.id, _obj);
		prop::assign(spell.name, _obj);
		if (spell.name->empty())
			LOG_WARN("Spell id: " << *spell.id << " " << spell.name.getName() << " is empty.");
		prop::assign(spell.delay, _obj);
		prop::assign(spell.effectivity, _obj);
		prop::assign(spell.projectileId, _obj);
		prop::assign(spell.visualId, _obj);
		spell.setShape(_createArea(_obj));

		if (auto optional = _createSpellFlags(_obj))
			spell.setFlags(*optional);
		if (auto optional = _createTargetData(_obj))
			spell.setTarget(*optional);

		spell.setEffects(_createSpellEffectData(_obj));
		return spell;
	}

	/*#####
	# Aura stuff
	#####*/
	struct _AuraEffectReader
	{
		inline static const char* UnitModString{ "unit/" };
		inline static const char* DeviceModString{ "device/" };

		static bool startsWith(std::string_view _str, std::string_view _val)
		{
			if (_str.size() < _val.size())
				return false;

			return std::equal(std::begin(_str), std::begin(_str) + _val.size(), std::begin(_val), std::end(_val));
		}

		static Aura::Effect::ModStat _createModStatData(const json::Object& _obj)
		{
			Aura::Effect::ModStat data;
			using Target = Aura::Effect::Target;
			// determine stat type (device, unit, ...)
			if (auto statString = json::getPtr<std::string>(_obj, "stat"))
			{
				if (startsWith(*statString, UnitModString))
				{
					data.target = Target::unit;
					auto stat = parseUnitStatType(statString->c_str() + std::string_view(UnitModString).size());
					data.statType = toInt(stat);
				}
				else if (startsWith(*statString, DeviceModString))
				{
					data.target = Target::device;
					auto stat = parseDeviceStatType(std::string_view(statString->c_str() + std::string_view(DeviceModString).size()));
					data.statType = toInt(stat);
				}
			}
			prop::assign(data.value, _obj);
			data.group = parseEntityStatModifierGroupe(json::get<std::string>(_obj, "group", ""));
			return data;
		}

		static Aura::Effect::TriggerSpell _createTriggerSpell(const json::Object& _obj)
		{
			Aura::Effect::TriggerSpell data;
			auto& triggerString = json::get<std::string>(_obj, "trigger_type");
			data.triggerType = parseAuraTriggerType(triggerString);
			prop::assign(data.id, _obj);
			return data;
		}

		static Aura::Effect::PeriodicTrigger _createPeriodicTrigger(const json::Object& _obj)
		{
			Aura::Effect::PeriodicTrigger data;
			prop::assign(data.interval, _obj);
			prop::assign(data.id, _obj);
			prop::assign(data.perma, _obj);
			return data;
		}

		static Aura::Effect::ModIncSpellEffectivity _createModIncSpellEffectivity(const json::Object& _obj)
		{
			Aura::Effect::ModIncSpellEffectivity data;
			prop::assign(data.bonus, _obj);
			prop::assign(data.spellId, _obj);
			return data;
		}

		static data::SpellModifier _createSpellModifier(const json::Object& _obj)
		{
			data::SpellModifier data;
			prop::assign(data.spellId, _obj);
			if (auto dataObj = json::getElementPtr(_obj, "mod"))
			{
				prop::assign(data.delay, *dataObj);
				prop::assign(data.chainEffectivity, *dataObj);
				prop::assign(data.projectileId, *dataObj);
				prop::assign(data.visualId, *dataObj);

				if (auto obj = json::getElementPtr(*dataObj, "flags"))
					data.flags = _createSpellFlags(*obj);

				if (auto obj = json::getElementPtr(*dataObj, "targets"))
					data.target = _createTargetData(*obj);

				data.area = _createAreaModifier(*dataObj);

				// effect stuff
				if (auto obj = json::getElementPtr(*dataObj, "effect"))
				{
					auto index = std::max<std::size_t>(json::getValue(*obj, "index", 0), 0);
					Spell::Effect effect;
					if (auto dataObj = json::getElementPtr(*obj, "data"))
					{
						if (auto effect = _createSpellEffect(*dataObj))
							data.effect = std::make_pair(index, std::move(*effect));
					}
				}
			}
			return data;
		}

		static Aura::Effect::Offender _createOffender(const json::Object& _obj)
		{
			Aura::Effect::Offender data;
			prop::assign(data.percent, _obj);
			return data;
		}

		static Aura::Effect::PeriodicDealing _createPeriodicDealing(const json::Object& _obj)
		{
			Aura::Effect::PeriodicDealing data;
			prop::assign(data.amount, _obj);
			prop::assign(data.interval, _obj);
			return data;
		}

		static Aura::Effect::ApplyCondition _createApplyCondition(const json::Object& _obj)
		{
			Aura::Effect::ApplyCondition data;
			auto& conditionString = json::get<std::string>(_obj, "condition");
			if (startsWith(conditionString, UnitModString))
			{
				data.target = data::Aura::Effect::Target::unit;
				auto conditionType = parseUnitCondition(conditionString.c_str() + std::string_view(UnitModString).size());
				data.condition = toInt(conditionType);
			}
			return data;
		}
	};

	Aura::Effects _createAuraEffects(const json::Object& _obj)
	{
		Aura::Effects effects;
		if (auto array = json::getArrayPtr(_obj, "effects"))
		{
			for (auto& effectObj : *array)
			{
				if (!effectObj.is_object())
					continue;
				Aura::Effect effect;
				using AuraType = game::data::Aura::Effect::Type;
				effect.type = parseAuraEffectType(json::get<std::string>(effectObj, "type", ""));
				switch (effect.type)
				{
				case AuraType::modStat:
				case AuraType::modStatPercent:
					effect.setData(_AuraEffectReader::_createModStatData(effectObj));
					break;
				case AuraType::triggerSpell:
					effect.setData(_AuraEffectReader::_createTriggerSpell(effectObj));
					break;
				case AuraType::periodicTrigger:
					effect.setData(_AuraEffectReader::_createPeriodicTrigger(effectObj));
					break;
				case AuraType::modIncSpell:
					effect.setData(_AuraEffectReader::_createModIncSpellEffectivity(effectObj));
					break;
				case AuraType::modSpell:
					effect.setData(_AuraEffectReader::_createSpellModifier(effectObj));
					break;
				case AuraType::offender:
					effect.setData(_AuraEffectReader::_createOffender(effectObj));
					break;
				case AuraType::periodicDamage:
				case AuraType::periodicDamagePercent:
				case AuraType::periodicHealing:
				case AuraType::periodicHealingPercent:
					effect.setData(_AuraEffectReader::_createPeriodicDealing(effectObj));
					break;
				case AuraType::condition:
					effect.setData(_AuraEffectReader::_createApplyCondition(effectObj));
					break;
				}
				effects.push_back(std::move(effect));
			}
		}
		return effects;
	}

	sl2::Bitmask<aura::AuraFlags> _createAuraFlags(const json::Object& _obj)
	{
		sl2::Bitmask<aura::AuraFlags> flags;
		if (auto array = json::getArrayPtr(_obj, "flags"))
		{
			for (auto& value : *array)
			{
				if (value.is_string())
					flags.apply(parseAuraFlag(value.get_ref<const std::string&>()));
			}
		}
		return flags;
	}

	Aura _createAura(const json::Object& _obj, const GameProperties& _gameProp)
	{
		Aura aura;
		prop::assign(aura.id, _obj);
		prop::assign(aura.visualId, _obj);
		prop::assign(aura.name, _obj);
		prop::assign(aura.duration, _obj);
		prop::assign(aura.family, _obj);
		prop::assign(aura.maxStacks, _obj);
		aura.setEffects(_createAuraEffects(_obj));
		aura.flags = _createAuraFlags(_obj);
		return std::move(aura);
	}

	/*#####
	# Device stuff
	#####*/
	Device::Stats _createDeviceStats(const json::Object& _obj, const Device::Stats& _default)
	{
		return _createStats<device::StatType>(_obj, "stats", &_default);
	}

	Device::XpBoostModifier _createDeviceXpBoostModifiers(const json::Object& _obj, const Device::XpBoostModifier& _defaultXpBoostModifier)
	{
		return _createXpBoostModifier<device::StatType>(_obj, "xp_boost_modifiers", &_defaultXpBoostModifier);
	}

	void _setupDevice(const json::Object& _obj, const GameProperties& _gameProp, Device& _device)
	{
		prop::assign(_device.id, _obj);
		prop::assign(_device.name, _obj);
		prop::assign(_device.description, _obj);
		prop::assign(_device.pickable, _obj);
		_device.setStats(_createDeviceStats(_obj, _gameProp.game.deviceStats));
		_device.setXpBoostModifier(_createDeviceXpBoostModifiers(_obj, _gameProp.game.deviceXpBoostModifier));
	}

	/*#####
	# Ability stuff
	#####*/
	sl2::Bitmask<Ability::Flag> _createAbilityFlags(const json::Object& _obj)
	{
		sl2::Bitmask<Ability::Flag> flags;
		if (auto flagArray = json::getArrayPtr(_obj, "ability_flags"))
		{
			for (auto& value : *flagArray)
			{
				if (value.is_string())
					flags.apply(parseAbilityFlag(value.get_ref<const std::string&>()));
			}
		}
		return flags;
	}

	Ability _createAbility(const json::Object& _obj, const GameProperties& _gameProp)
	{
		Ability ability;
		_setupDevice(_obj, _gameProp, ability);

		prop::assign(ability.spellId, _obj);
		prop::assign(ability.cooldown, _obj);
		ability.flags = _createAbilityFlags(_obj);
		return ability;
	}

	/*#####
	# Factory stuff
	#####*/
	Factory::Upgrades _createUpgrades(const json::Object& _root)
	{
		Factory::Upgrades upgrades;
		std::size_t i = 0;
		if (auto upgradeArray = json::getArrayPtr(_root, "upgrades"))
		{
			for (auto& upgrade : *upgradeArray)
			{
				std::size_t iId = 0;
				if (upgrade.is_object())
				{
					upgrades.names.at(i) = json::get<std::string>(upgrade, "name", "");
					if (auto idArray = json::getArrayPtr(upgrade, "ids"))
					{
						for (auto& id : *idArray)
							upgrades.spellIDs.at(i).at(iId++) = id.get<ID>();
					}
				}
				if (iId != 4)
					throw std::runtime_error("upgrade index: " + std::to_string(i) + " is invalid.");
				++i;
			}
		}
		if (i != 3)
			throw std::runtime_error("upgrades invalid.");
		return upgrades;
	}

	Factory _createFactory(const json::Object& _obj, const GameProperties& _gameProp)
	{
		Factory factory;
		_setupDevice(_obj, _gameProp, factory);

		// determine factory type
		if (auto typeObj = json::getElementPtr(_obj, "minion"))
		{
			Factory::Minion minion;
			prop::assign(minion.id, *typeObj);
			minion.xpPerSecond = _gameProp.game.xpPerSecond;
			assign(minion.xpPerSecond, *typeObj);

			minion.setUpgrades(_createUpgrades(*typeObj));
			minion.setTargetPreference(_createTargetPreference(*typeObj));

			factory.setData(std::move(minion));
		}
		else if (auto typeObj = json::getElementPtr(_obj, "tower"))
		{
			Factory::Tower tower;
			prop::assign(tower.id, *typeObj);
			tower.xpPerSecond = _gameProp.game.xpPerSecond;
			assign(tower.xpPerSecond, *typeObj);

			tower.setUpgrades(_createUpgrades(*typeObj));
			tower.setTargetPreference(_createTargetPreference(*typeObj));
			
			factory.setData(std::move(tower));
		}
		else if (auto typeObj = json::getElementPtr(_obj, "wall"))
		{
			Factory::Wall wall;
			prop::assign(wall.id, *typeObj);
			factory.setData(std::move(wall));
		}
		if (factory.isEmpty())
			throw std::runtime_error("DataReader::createFactory: no factory type set.");
		return factory;
	}

	/*#####
	# Unit stuff
	#####*/
	UnitModifiers _createUnitModifiers(const json::Object& _obj, const UnitModifiers& _defaultUnitModifiers)
	{
		return _createUnitModifiers(_obj, "modifiers", &_defaultUnitModifiers);
	}

	Unit::XpBoostModifier _createUnitXpBoostModifiers(const json::Object& _obj, const Unit::XpBoostModifier& _defaultXpBoostModifier)
	{
		return _createXpBoostModifier<unit::StatType>(_obj, "xp_boost_modifiers", &_defaultXpBoostModifier);
	}

	Unit::Stats _createUnitStats(const json::Object& _obj, const Unit::Stats& _default)
	{
		return _createStats<unit::StatType>(_obj, "stats", &_default);
	}

	void _setupUnit(const json::Object& _obj, const GameProperties& _gameProp, Unit& _unit)
	{
		prop::assign(_unit.id, _obj);
		prop::assign(_unit.name, _obj);
		prop::assign(_unit.killingXp, _obj);
		prop::assign(_unit.layer, _obj);
		if (_unit.name->empty())
			LOG_WARN(_unit.getTypeName() << " id: " << *_unit.id << " empty name.");

		_unit.setStats(_createUnitStats(_obj, _gameProp.game.unitStats));
		_unit.setXpBoostModifier(_createUnitXpBoostModifiers(_obj, _gameProp.game.unitXpBoostModifier));
		_unit.setBody(_createArea(_obj));
		_unit.setModifiers(_createUnitModifiers(_obj, _gameProp.game.unitModifiers));
		_unit.setAuraIds(_createInitAuras(_obj));
	}

	/*#####
	# ActiveUnit stuff
	#####*/
	void _setupActiveUnit(const json::Object& _obj, const GameProperties& _gameProp, ActiveUnit& _unit)
	{
		_setupUnit(_obj, _gameProp, _unit);

		prop::assign(_unit.spellId, _obj);
	}

	/*#####
	# Core stuff
	#####*/
	Core _createCore(const json::Object& _obj, const GameProperties& _gameProp)
	{
		Core core;
		_setupUnit(_obj, _gameProp, core);
		return core;
	}

	/*#####
	# Wall stuff
	#####*/
	Wall _createWall(const json::Object& _obj, const GameProperties& _gameProp)
	{
		Wall wall;
		_setupUnit(_obj, _gameProp, wall);
		return wall;
	}

	/*#####
	# Minion stuff
	#####*/
	Minion::Flags _createMinionFlags(const json::Object& _obj)
	{
		Minion::Flags flags;
		if (auto flagArray = json::getArrayPtr(_obj, "minion_flags"))
		{
			for (auto& el : *flagArray)
			{
				if (el.is_string())
					flags.apply(parseMinionFlag(el.get_ref<const std::string&>()));
			}
		}
		return flags;
	}

	Minion _createMinion(const json::Object& _obj, const GameProperties& _gameProp)
	{
		Minion minion;
		_setupActiveUnit(_obj, _gameProp, minion);

		minion.setFlags(_createMinionFlags(_obj));
		return minion;
	}

	/*#####
	# Tower stuff
	#####*/
	Tower _createTower(const json::Object& _obj, const GameProperties& _gameProp)
	{
		Tower tower;
		_setupActiveUnit(_obj, _gameProp, tower);

		prop::assign(tower.movementRange, _obj);
		prop::assign(tower.movementWindup, _obj);
		prop::assign(tower.movementCooldown, _obj);
		return tower;
	}

	/*#####
	# Projectile stuff
	#####*/
	Projectile::Flags _createProjectileFlags(const json::Object& _obj)
	{
		Projectile::Flags flags;
		if (auto flagArray = json::getArrayPtr(_obj, "projectile_flags"))
		{
			for (auto& el : *flagArray)
			{
				if (el.is_string())
					flags.apply(parseProjectileFlag(el.get_ref<const std::string&>()));
			}
		}
		return flags;
	}

	Projectile _createProjectile(const json::Object& _obj, const GameProperties& _gameProp)
	{
		Projectile projectile;
		_setupUnit(_obj, _gameProp, projectile);
		prop::assign(projectile.bounces, _obj);
		prop::assign(projectile.bounceRange, _obj);
		projectile.setFlags(_createProjectileFlags(_obj));
		return projectile;
	}

	/*#####
	# Trigger stuff
	#####*/
	Trigger::Flags _createTriggerFlags(const json::Object& _obj)
	{
		Trigger::Flags flags;
		if (auto flagArray = json::getArrayPtr(_obj, "trigger_flags"))
		{
			for (auto& el : *flagArray)
			{
				if (el.is_string())
					flags.apply(parseTriggerFlag(el.get_ref<const std::string&>()));
			}
		}
		return flags;
	}

	Trigger _createTrigger(const json::Object& _obj, const GameProperties& _gameProp)
	{
		Trigger trigger;
		_setupUnit(_obj, _gameProp, trigger);
		prop::assign(trigger.lifeTime, _obj);
		trigger.setFlags(_createTriggerFlags(_obj));
		return trigger;
	}

	/*#####
	# other stuff
	#####*/
	template <class Obj>
	Obj _createIdObject(const json::Object& _obj, const GameProperties& _gameProp)
	{
		Obj obj;
		prop::assign(obj.id, _obj);
		return obj;
	}

	/*#####
	# Modification stuff
	#####*/
	Modification _createModification(const json::Object& _obj, const GameProperties& _gameProp)
	{
		Modification mod;
		prop::assign(mod.id, _obj);
		prop::assign(mod.spellId, _obj);
		prop::assign(mod.name, _obj);
		if (mod.name->empty())
			LOG_CHANNEL_WARN(dataLogger, "Modification id: " << *mod.id << " property name: " << mod.name.getName() << " is empty.");
		return mod;
	}

	/*#####
	# Emote stuff
	#####*/
	Emote _createEmote(const json::Object& _obj, const GameProperties& _gameProp)
	{
		Emote emote;
		prop::assign(emote.id, _obj);
		prop::assign(emote.emote, _obj);
		if (emote.emote->empty())
			LOG_CHANNEL_WARN(dataLogger, "Emote id: " << *emote.id << " property name: " << emote.emote.getName() << " is empty.");
		return emote;
	}

	/*#####
	# ai stuff
	#####*/
	AiMazeLayout _createAiMazeLayout(const json::Object& _obj, const GameProperties& _gameProp)
	{
		AiMazeLayout layout;
		prop::assign(layout.id, _obj);
		prop::assign(layout.name, _obj);
		if (auto positions = json::getArrayPtr(_obj, "positions"))
		{
			std::vector<TilePosition> posList;
			for (auto& pos : *positions)
			{
				if (pos.is_array())
				{
					if (pos.size() >= 2)
						posList.emplace_back(pos[0].get<TileCoordinateType>(), pos[1].get<TileCoordinateType>());
				}
			}
			layout.positions = std::move(posList);
		}
		return layout;
	}

	std::vector<ModPage> _createModPages(const json::Object& _obj)
	{
		std::vector<ModPage> pages;
		if (auto array = json::getArrayPtr(_obj, "mod_pages"))
		{
			for (auto& node : *array)
			{
				auto pid = json::getValue<ID>(node, "pid");
				auto& mods = json::getArray(node, "mods");
				IDs ids;
				ids.reserve(mods.size());
				std::transform(std::begin(mods), std::end(mods), std::back_inserter(ids),
					[](const auto& _value) { return _value.get<ID>(); }
				);
				pages.emplace_back(pid, std::move(ids));
			}
		}
		return pages;
	}

	AiStrategy _createAiStrategy(const json::Object& _obj, const GameProperties& _gameProp)
	{
		AiStrategy strategy;
		prop::assign(strategy.id, _obj);
		prop::assign(strategy.name, _obj);
		prop::assign(strategy.botLevelMax, _obj);
		prop::assign(strategy.botLevelMin, _obj);
		assert(strategy.botLevelMin <= strategy.botLevelMax);

		auto readStringVector = [](const json::Object& _obj, std::string_view _nodeName)
		{
			std::vector<std::string> result;
			if (auto array = json::getArrayPtr(_obj, _nodeName))
			{
				result.reserve(array->size());
				std::transform(std::begin(*array), std::end(*array), std::back_inserter(result),
					[](const auto& _node) { return _node.get_ref<const std::string&>(); }
				);
			}
			return result;
		};

		strategy.flags = readStringVector(_obj, "flags");
		if (auto array = json::getArrayPtr(_obj, "skins"))
		{
			std::transform(std::begin(*array), std::end(*array),  std::back_inserter(strategy.skins),
				[](const auto& _value) { return _value.get<ID>(); }
			);
		}

		auto readIdList = [](std::string_view _name, const json::Object& _root)
		{
			std::vector<IDs> list;
			if (auto array = json::getArrayPtr(_root, _name))
			{
				for (auto& value : *array)
				{
					if (value.is_array())
					{
						IDs ids;
						for (auto& subVals : value)
							ids.emplace_back(subVals.get<ID>());
						list.emplace_back(std::move(ids));
					}
					else
						list.emplace_back(IDs{ value.get<ID>() });
				}
			}
			return list;
		};

		// read picks
		if (auto pickObj = json::getElementPtr(_obj, "picks"))
		{
			strategy.factoryPicks = readIdList("factories", *pickObj);
			strategy.abilityPicks = readIdList("abilities", *pickObj);
		}

		// read bans
		if (auto banObj = json::getElementPtr(_obj, "bans"))
		{
			strategy.factoryBans = readIdList("factories", *banObj);
			strategy.abilityBans = readIdList("abilities", *banObj);
		}

		// read maze layouts
		if (auto layoutArray = json::getArrayPtr(_obj, "layouts"))
		{
			IDs layouts;
			for (auto& el : *layoutArray)
				layouts.emplace_back(el.get<ID>());
			strategy.mazeLayouts = std::move(layouts);
		}

		strategy.modPages = _createModPages(_obj);
		return strategy;
	}

	/*#####
	# JsonReader
	#####*/
	template <class T>
	T readElement(const json::Object& _obj, const GameProperties& _gameProp)
	{
		// spell/aura
		if constexpr (std::is_same_v<T, Aura>)
			return _createAura(_obj, _gameProp);
		else if constexpr (std::is_same_v<T, Spell>)
			return _createSpell(_obj, _gameProp);

		// devices
		else if constexpr (std::is_same_v<T, Ability>)
			return _createAbility(_obj, _gameProp);
		else if constexpr (std::is_same_v<T, Factory>)
			return _createFactory(_obj, _gameProp);

		// units
		else if constexpr (std::is_same_v<T, Core>)
			return _createCore(_obj, _gameProp);
		else if constexpr (std::is_same_v<T, Wall>)
			return _createWall(_obj, _gameProp);
		else if constexpr (std::is_same_v<T, Minion>)
			return _createMinion(_obj, _gameProp);
		else if constexpr (std::is_same_v<T, Tower>)
			return _createTower(_obj, _gameProp);
		else if constexpr (std::is_same_v<T, Projectile>)
			return _createProjectile(_obj, _gameProp);
		else if constexpr (std::is_same_v<T, Trigger>)
			return _createTrigger(_obj, _gameProp);

		// others
		else if constexpr (std::is_same_v<T, Color>)
			return _createIdObject<Color>(_obj, _gameProp);
		else if constexpr (std::is_same_v<T, PlayerIcon>)
			return _createIdObject<PlayerIcon>(_obj, _gameProp);
		else if constexpr (std::is_same_v<T, Skin>)
			return _createIdObject<Skin>(_obj, _gameProp);
		else if constexpr (std::is_same_v<T, Background>)
			return _createIdObject<Background>(_obj, _gameProp);
		else if constexpr (std::is_same_v<T, Emote>)
			return _createEmote(_obj, _gameProp);
		else if constexpr (std::is_same_v<T, Modification>)
			return _createModification(_obj, _gameProp);

		// ai
		else if constexpr (std::is_same_v<T, AiMazeLayout>)
			return _createAiMazeLayout(_obj, _gameProp);
		else if constexpr (std::is_same_v<T, AiStrategy>)
			return _createAiStrategy(_obj, _gameProp);
		else
			static_assert(false, "Invalid element type.");
	}

	template <class T>
	Table<T> _readTable(std::string_view _name, const json::Object& _root, const GameProperties& _gameProp)
	{
		LOG_CHANNEL_INFO(dataLogger, "Begin initializing table name: " << _name);
		if (auto array = json::getArrayPtr(_root, _name))
		{
			Table<T> table;
			for (auto& element : *array)
			{
				if (!element.is_object())
					continue;

				try
				{
					if (auto obj = readElement<T>(element, _gameProp); 0 < *obj.id)
					{
						if (!insertData(table, obj))
							LOG_ERR(_name << " id: " << *obj.id << " Skipped duplication.");
					}
					else
						LOG_ERR(_name << " element with id: 0 found.");
				}
				catch (const std::exception& _e)
				{
					LOG_ERR(_name << " caught exception: " << _e.what());
				}
			}
			if (table.empty())
				LOG_CHANNEL_WARN(dataLogger, "Finished initializing table name: " << _name << " is empty.");
			else
				LOG_CHANNEL_INFO(dataLogger, "Finished initializing table name: " << _name << " " << table.size() << " elements loaded.");
			return table;
		}
		else
			throw Error("JsonReader: \"" + std::string(_name) + "\" invalid or not found.");
	}

	JsonReader::JsonReader(const json::Object& _doc) :
		m_Doc(_doc)
	{
		if (m_Doc.is_null())
			throw Error("DataReader: invalid data");
	}

	const json::Object& JsonReader::_getRootObject()
	{
		if (m_Doc.is_null())
			throw Error("JsonReader: Invalid JSON document.");
		return m_Doc;
	}

	bool JsonReader::isOpen() const
	{
		return !m_Doc.is_null();
	}

	Table<AiMazeLayout> JsonReader::readAiMazeLayouts(const GameProperties& _gameProp)
	{
		return _readTable<AiMazeLayout>("ai_maze_layouts", _getRootObject(), _gameProp);
	}

	Table<AiStrategy> JsonReader::readAiStrategies(const GameProperties& _gameProp)
	{
		return _readTable<AiStrategy>("ai_strategies", _getRootObject(), _gameProp);
	}

	Table<Emote> JsonReader::readEmotes(const GameProperties& _gameProp)
	{
		return _readTable<Emote>("emotes", _getRootObject(), _gameProp);
	}

	Table<Modification> JsonReader::readModifications(const GameProperties& _gameProp)
	{
		return _readTable<Modification>("modifications", _getRootObject(), _gameProp);
	}

	Table<Color> JsonReader::readColors(const GameProperties& _gameProp)
	{
		return _readTable<Color>("colors", _getRootObject(), _gameProp);
	}

	Table<PlayerIcon> JsonReader::readPlayerIcons(const GameProperties& _gameProp)
	{
		return _readTable<PlayerIcon>("playerIcons", _getRootObject(), _gameProp);
	}

	Table<Skin> JsonReader::readSkins(const GameProperties& _gameProp)
	{
		return _readTable<Skin>("skins", _getRootObject(), _gameProp);
	}

	Table<Background> JsonReader::readBackgrounds(const GameProperties& _gameProp)
	{
		return _readTable<Background>("backgrounds", _getRootObject(), _gameProp);
	}

	Table<Trigger> JsonReader::readTriggers(const GameProperties& _gameProp)
	{
		return _readTable<Trigger>("triggers", _getRootObject(), _gameProp);
	}

	Table<Projectile> JsonReader::readProjectiles(const GameProperties& _gameProp)
	{
		return _readTable<Projectile>("projectiles", _getRootObject(), _gameProp);
	}

	Table<Factory> JsonReader::readFactories(const GameProperties& _gameProp)
	{
		return _readTable<Factory>("factories", _getRootObject(), _gameProp);
	}

	Table<Core> JsonReader::readCores(const GameProperties& _gameProp)
	{
		return _readTable<Core>("cores", _getRootObject(), _gameProp);
	}

	Table<Wall> JsonReader::readWalls(const GameProperties& _gameProp)
	{
		return _readTable<Wall>("walls", _getRootObject(), _gameProp);
	}

	Table<Minion> JsonReader::readMinions(const GameProperties& _gameProp)
	{
		return _readTable<Minion>("minions", _getRootObject(), _gameProp);
	}

	Table<Tower> JsonReader::readTowers(const GameProperties& _gameProp)
	{
		return _readTable<Tower>("towers", _getRootObject(), _gameProp);
	}

	Table<Spell> JsonReader::readSpells(const GameProperties& _gameProp)
	{
		return _readTable<Spell>("spells", _getRootObject(), _gameProp);
	}

	Table<Aura> JsonReader::readAuras(const GameProperties& _gameProp)
	{
		return _readTable<Aura>("auras", _getRootObject(), _gameProp);
	}

	Table<Ability> JsonReader::readAbilities(const GameProperties& _gameProp)
	{
		return _readTable<Ability>("abilities", _getRootObject(), _gameProp);
	}
} // namespace data
} // namespace game
