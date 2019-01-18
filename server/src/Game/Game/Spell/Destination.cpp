#include "stdafx.h"
#include "Destination.hpp"
#include "TargetFinder.hpp"
#include "Game/Entity/Unit.hpp"

namespace game::spell
{
	void AbstractDestination::reduceTargetsIfNeccessary(const TargetDefinition& _targetInfo, std::vector<Castable*>& _targets) const
	{
		/*if we have a target count and more possible targets in our list,
		select random targets and remove the others*/
		if (_targetInfo.count > 0 && std::size(_targets) > _targetInfo.count)
		{
			std::vector<Castable*> rndEntities;
			rndEntities.reserve(*_targetInfo.count);
			while (rndEntities.size() < *_targetInfo.count)
			{
				auto itr = std::begin(_targets);
				itr += randomInt<std::size_t>(0, _targets.size() - 1);
				assert(itr != std::end(_targets));
				rndEntities.emplace_back(*itr);
				// swap with last element to avoid moving all elements in the vector
				std::swap(*itr, _targets.back());
				_targets.pop_back();
			}
			_targets = std::move(rndEntities);
		}
		else if (_targetInfo.count < 0)
		{
			if (std::abs(*_targetInfo.count) < std::size(_targets))
			{
				for (auto newSize = std::size(_targets) + *_targetInfo.count;
					newSize < std::size(_targets);)
				{
					auto itr = std::begin(_targets) + randomInt<std::size_t>(0, _targets.size() - 1);
					assert(itr != std::end(_targets));
					// swap with last element to avoid moving all elements in the vector
					std::swap(*itr, _targets.back());
					_targets.pop_back();
				}
			}
			else
				_targets.clear();
		}
	}

	/*#####
	# AbstractTarget
	#####*/
	AbstractDestination::AbstractDestination(const TargetFinder& _targetFinder) :
		m_TargetFinder(_targetFinder)
	{
	}

	/*#####
	# SelfTarget
	#####*/
	SelfDestination::SelfDestination(const TargetFinder& _targetFinder, Castable& _caster) :
		super(_targetFinder),
		m_Caster(_caster)
	{
		++m_Caster.m_IncomingSpellCounter;
	}

	SelfDestination::~SelfDestination()
	{
		--m_Caster.m_IncomingSpellCounter;
	}

	bool SelfDestination::nextDestination(const TargetDefinition& _targetInfo, const AbsShape& _area, bool _unique)
	{
		return false;
	}

	std::optional<AbsPosition> SelfDestination::getPosition() const
	{
		if (m_Caster.isUnit())
			return m_Caster.toUnit().getPosition();
		return {};
	}

	CastableHandle SelfDestination::getDestinationEntity() const
	{
		return m_Caster.getCastableHandle();
	}

	std::vector<Castable*> SelfDestination::findTargets(const Spell& _spell) const
	{
		return { &m_Caster };
	}

	DestinationPtr SelfDestination::copy() const
	{
		return std::make_unique<SelfDestination>(m_TargetFinder, m_Caster);
	}

	/*#####
	# CastableTarget
	#####*/
	CastableDestination::CastableDestination(const TargetFinder& _targetFinder, CastableHandle _handle) :
		super(_targetFinder)
	{
		assert(_handle);
		_switchTarget(_handle);
	}

	CastableDestination::~CastableDestination()
	{
		_switchTarget(CastableHandle());
	}

	DestinationPtr CastableDestination::copy() const
	{
		return std::make_unique<CastableDestination>(m_TargetFinder, getDestinationEntity());
	}

	void CastableDestination::_switchTarget(CastableHandle _newHandle)
	{
		if (!m_Targets.empty() && m_Targets.back())
			--m_Targets.back()->m_IncomingSpellCounter;
		if (_newHandle)
		{
			++_newHandle->m_IncomingSpellCounter;
			m_Targets.emplace_back(_newHandle);
		}
	}

	bool CastableDestination::nextDestination(const TargetDefinition& _targetInfo, const AbsShape& _area, bool _unique)
	{
		auto lastTarget = m_Targets.back();
		// if last target is unit target, we query for units; otherwise return false
		if (lastTarget && lastTarget->isUnit())
		{
			auto units = m_TargetFinder.findUnits(_targetInfo, _area);
			auto removeTargetIfUsed = [](std::vector<unit::Unit*>& _units, Castable* _target)
			{
				if (auto itr = std::find_if(std::begin(_units), std::end(_units),
					[_target](const unit::Unit* _other) { return _target == _other; });
					itr != std::end(_units)
				)
				{
					std::swap(*itr, _units.back());
					_units.pop_back();
				}
			};

			if (!units.empty())
			{
				// if unique, remove every used target from list
				if (_unique)
				{
					for (auto& target : m_Targets)
					{
						if (target)
							removeTargetIfUsed(units, target.get());
					}
				}
				// if not unique, remove only the last target
				else
				{
					if (m_Targets.back())
						removeTargetIfUsed(units, m_Targets.back().get());
				}

				if (!units.empty())
				{
					// ToDo: implement different modes (random, nearest, etc.)
					_switchTarget(units[randomInt<std::size_t>(0, units.size() - 1)]->getCastableHandle());
					return true;
				}
			}
		}
		return false;
	}

	std::optional<AbsPosition> CastableDestination::getPosition() const
	{
		auto lastTarget = m_Targets.back();
		if (lastTarget && lastTarget->isUnit())
			return lastTarget->toUnit().getPosition();
		return {};
	}

	CastableHandle CastableDestination::getDestinationEntity() const
	{
		return m_Targets.back();
	}

	std::vector<Castable*> CastableDestination::findTargets(const Spell& _spell) const
	{
		std::vector<Castable*> targets;
		auto lastTarget = m_Targets.back();
		if (lastTarget)
		{
			auto& target = *lastTarget;
			/* If target is an unit, query for other unit targets. We need an unit target to get a valid position. Otherwise
			we skip the unit query. */
			if (target.isUnit())
			{
				auto& unitTarget = target.toUnit();
				auto& spellShape = _spell.getShape();
				if (spellShape.isNull())	// unit single target
				{
					if (m_TargetFinder.hasCorrectTargetFlags(_spell.getSpellInfo().getTarget(), unitTarget))
						targets.emplace_back(&unitTarget);
				}
				else
				{
					auto pos = _spell.getDestinationPosition();
					assert(pos);
					auto units = m_TargetFinder.findUnits(_spell.getSpellInfo().getTarget(), *pos, spellShape);
					targets.insert(std::end(targets), std::begin(units), std::end(units));
				}
			}
			auto devices = m_TargetFinder.findDevices(_spell.getSpellInfo().getTarget());
			if (!devices.empty())
				targets.insert(std::end(targets), std::begin(devices), std::end(devices));
			reduceTargetsIfNeccessary(_spell.getSpellInfo().getTarget(), targets);
		}
		else
			LOG_ERR("CastableTarget: CastableHandle is invalid.");
		return targets;
	}

	/*#####
	# PositionTarget
	#####*/
	PositionDestination::PositionDestination(const TargetFinder& _targetFinder, AbsPosition _pos) :
		super(_targetFinder),
		m_Position(std::move(_pos))
	{
	}

	DestinationPtr PositionDestination::copy() const
	{
		return std::make_unique<PositionDestination>(m_TargetFinder, m_Position);
	}

	bool PositionDestination::nextDestination(const TargetDefinition& _targetInfo, const AbsShape& _area, bool _unique)
	{
		// ToDo: possibly random point in circle?
		return false;
	}

	std::optional<AbsPosition> PositionDestination::getPosition() const
	{
		return m_Position;
	}

	CastableHandle PositionDestination::getDestinationEntity() const
	{
		return CastableHandle();
	}

	std::vector<Castable*> PositionDestination::findTargets(const Spell& _spell) const
	{
		auto pos = _spell.getDestinationPosition();
		assert(pos);
		auto& spellArea = _spell.getShape();
		auto targets = m_TargetFinder.findUnits(_spell.getSpellInfo().getTarget(), *pos, spellArea);
		auto devices = m_TargetFinder.findDevices(_spell.getSpellInfo().getTarget());
		if (!targets.empty())
			devices.insert(std::end(devices), std::begin(targets), std::end(targets));
		reduceTargetsIfNeccessary(_spell.getSpellInfo().getTarget(), devices);
		return devices;
	}
} // namespace game::spell
