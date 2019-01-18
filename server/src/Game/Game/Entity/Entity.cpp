#include "stdafx.h"
#include "Entity.hpp"

namespace game
{
	Entity::Entity(const  EntityInitializer& _initializer) :
		super(_initializer.castableInit),
		m_Owner(&_initializer.owner)
	{
	}

	void Entity::startPlay()
	{
		derivedStartPlay();
	}

	void Entity::finalizeTick()
	{
		derivedFinalizeTick();
	}

	void Entity::setupCastableSnapshot(CastableSnapshot& _info) const
	{
		super::setupCastableSnapshot(_info);

		_info.participant = &getOwner();
	}

	void Entity::update(Time _diff)
	{
		updateCastable(_diff);
	}

	Participant& Entity::getOwner() const
	{
		assert(m_Owner);
		return *m_Owner;
	}

	void Entity::setOwner(Participant& _owner)
	{
		m_Owner = &_owner;
	}
} // namespace game
