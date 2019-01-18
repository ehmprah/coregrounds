#pragma once

#include "Data/Spell.hpp"

namespace game::spell
{
	class TargetFinder;
	class Spell;

	using Destination = class AbstractDestination;
	using DestinationPtr = std::unique_ptr<Destination>;

	class AbstractDestination :
		sl2::NonCopyable
	{
	protected:
		const TargetFinder& m_TargetFinder;
		using TargetDefinition = data::Spell::Target;

		void reduceTargetsIfNeccessary(const TargetDefinition& _targetInfo, std::vector<Castable*>& _targets) const;

	public:
		AbstractDestination(const TargetFinder& _targetFinder);
		virtual ~AbstractDestination() = default;

		virtual DestinationPtr copy() const = 0;

		virtual bool nextDestination(const TargetDefinition& _targetInfo, const AbsShape& _area, bool _unique) = 0;
		virtual std::optional<AbsPosition> getPosition() const = 0;
		virtual CastableHandle getDestinationEntity() const = 0;
		virtual std::vector<Castable*> findTargets(const Spell& _spell) const = 0;
	};

	class SelfDestination :
		public AbstractDestination
	{
	private:
		using super = AbstractDestination;
		Castable& m_Caster;

	public:
		SelfDestination(const TargetFinder& _targetFinder, Castable& _caster);
		~SelfDestination();

		DestinationPtr copy() const override;

		bool nextDestination(const TargetDefinition& _targetInfo, const AbsShape& _area, bool _unique) override;
		std::optional<AbsPosition> getPosition() const override;
		CastableHandle getDestinationEntity() const override;
		std::vector<Castable*> findTargets(const Spell& _spell) const override;
	};

	class CastableDestination :
		public AbstractDestination
	{
	private:
		using super = AbstractDestination;

		std::vector<CastableHandle> m_Targets;

		void _switchTarget(CastableHandle _newHandle);

	public:
		CastableDestination(const TargetFinder& _targetFinder, CastableHandle _handle);
		~CastableDestination();

		DestinationPtr copy() const override;

		bool nextDestination(const TargetDefinition& _targetInfo, const AbsShape& _area, bool _unique) override;
		std::optional<AbsPosition> getPosition() const override;
		CastableHandle getDestinationEntity() const override;
		std::vector<Castable*> findTargets(const Spell& _spell) const override;
	};

	class PositionDestination :
		public AbstractDestination
	{
	private:
		using super = AbstractDestination;

		AbsPosition m_Position;

	public:
		PositionDestination(const TargetFinder& _targetFinder, AbsPosition _pos);

		DestinationPtr copy() const override;

		bool nextDestination(const TargetDefinition& _targetInfo, const AbsShape& _area, bool _unique) override;
		std::optional<AbsPosition> getPosition() const override;
		CastableHandle getDestinationEntity() const override;
		std::vector<Castable*> findTargets(const Spell& _spell) const override;
	};

	template <class Destination, typename... Args>
	DestinationPtr createDestination(const TargetFinder& _targetFinder, Args&&... _args)
	{
		return std::make_unique<Destination>(_targetFinder, std::forward<Args>(_args)...);
	}
} // namespace game::spell
