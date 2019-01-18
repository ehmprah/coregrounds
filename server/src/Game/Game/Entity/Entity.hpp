#pragma once

#include "Castable.hpp"

namespace game
{
	struct EntityInitializer
	{
		CastableInitializer castableInit;
		Participant& owner;
	};

	class Entity :
		public Castable
	{
		using super = Castable;

	public:
		Entity(const  EntityInitializer& _initializer);
		virtual ~Entity() = default;

		void startPlay();
		void finalizeTick();

		virtual void update(Time _diff);

		Participant& getOwner() const;
		virtual void setOwner(Participant& _owner);

	protected:
		virtual void derivedStartPlay() {}
		virtual void derivedFinalizeTick() {}

		void setupCastableSnapshot(CastableSnapshot& _info) const override;

	private:
		Participant* m_Owner = nullptr;
	};
} // namespace game
