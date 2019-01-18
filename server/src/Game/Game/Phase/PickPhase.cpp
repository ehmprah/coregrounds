#include "stdafx.h"
#include "PickPhase.hpp"
#include "Data/Game.hpp"
#include "Game/Entity/Participant.hpp"

namespace game::phase
{
	IDs uniqueIdPrepare(const IDs& _src, const IDs& _dest)
	{
		assert(std::is_sorted(std::begin(_src), std::end(_src)) &&
			std::is_sorted(std::begin(_dest), std::end(_dest)));
		IDs result;
		result.reserve(std::size(_dest));
		std::set_difference(std::begin(_dest), std::end(_dest), std::begin(_src), std::end(_src), std::back_inserter(result));
		return result;
	}

	template <class Container>
	bool _isValidId(const Container& _container, ID _id)
	{
		assert(std::is_sorted(std::begin(_container), std::end(_container)));
		return std::binary_search(std::begin(_container), std::end(_container), _id);
	}

	template <class Table>
	IDs _extractPickableIdsFromTable(const Table& _ids)
	{
		IDs ids;
		ids.reserve(std::size(_ids));
		for (auto& obj : _ids)
		{
			if (*obj.pickable)
				ids.emplace_back(*obj.id);
		}
		std::sort(std::begin(ids), std::end(ids));
		return ids;
	}

	template <class Table, class Preparer>
	IDs _extractBanableIds(const IDs& _bans, const Table& _ids, Preparer _preparer)
	{
		auto ids = _preparer(_bans, _extractPickableIdsFromTable(_ids));
		ids.shrink_to_fit();
		return ids;
	}

	template <class Table, class Preparer>
	IDs _extractPickableIds(const IDs& _bans, const IDs& _picks, const IDs& _unlocks, const Table& _ids, Preparer _preparer)
	{
		auto pickableIds = _extractPickableIdsFromTable(_ids);
		IDs ids;
		ids.reserve(std::size(_unlocks));
		std::set_intersection(std::begin(_unlocks), std::end(_unlocks), std::begin(pickableIds), std::end(pickableIds), std::back_inserter(ids));

		ids = _preparer(_bans, std::move(ids));
		ids = _preparer(_picks, std::move(ids));
		ids.shrink_to_fit();
		return ids;
	}

	void _insertSorted(IDs& _dest, ID _value)
	{
		auto itr = std::lower_bound(std::begin(_dest), std::end(_dest), _value);
		assert(itr == std::end(_dest) || *itr != _value);
		_dest.insert(itr, _value);
	}

	Pick::Pick(const std::vector<Team>& _teams, std::shared_ptr<const data::Game> _gameData) :
		super(),
		m_Teams(_teams),
		m_GameData(_gameData),
		m_PickProperties(_gameData->gameProperties.pick),
		m_PickIdPreparer(&uniqueIdPrepare),
		m_BanIdPreparer(&uniqueIdPrepare)
	{
		assert(m_GameData);
	}

	bool Pick::pickFactory(PickParticipant& _participant, ID _id)
	{
		if (&getActiveParticipant() == &_participant && m_State == State::pickFactory)
		{
			if (_isValidId(getPickableFactories(_participant), _id))
			{
				_participant.pickFactory(_id);
				_insertSorted(m_PickedFactories, _id);
				_next();
				return true;
			}
		}
		return false;
	}

	bool Pick::pickAbility(PickParticipant& _participant, ID _id)
	{
		if (&getActiveParticipant() == &_participant && m_State == State::pickAbility)
		{
			if (_isValidId(getPickableAbilities(_participant), _id))
			{
				_participant.pickAbility(_id);
				_insertSorted(m_PickedAbilities, _id);
				_next();
				return true;
			}
		}
		return false;
	}

	bool Pick::banFactory(PickParticipant& _participant, ID _id)
	{
		if (&getActiveParticipant() == &_participant && m_State == State::banFactory)
		{
			assert(_participant.getFactoryBans().size() < *m_PickProperties.maxFactoryBans);
			
			if (_isValidId(getBanableFactories(_participant), _id))
			{
				_participant.banFactory(_id);
				_insertSorted(m_BannedFactories, _id);
				_next();
				return true;
			}
		}
		return false;
	}

	bool Pick::banAbility(PickParticipant& _participant, ID _id)
	{
		if (&getActiveParticipant() == &_participant && m_State == State::banAbility)
		{
			assert(_participant.getAbilityBans().size() < *m_PickProperties.maxAbilityBans);

			if (_isValidId(getBanableAbilities(_participant), _id))
			{
				_participant.banAbility(_id);
				_insertSorted(m_BannedAbilities, _id);
				_next();
				return true;
			}
		}
		return false;
	}

	bool Pick::switchModPage(PickParticipant& _participant, ID _pid)
	{
		if (m_State == State::modSelection)
		{
			_participant.setModPage(_pid);
			return true;
		}
		return false;
	}

	std::vector<ParticipantDef> Pick::generateParticipantDefinitions() const
	{
		std::vector<ParticipantDef> result;
		result.reserve(std::size(m_Participants));
		for (auto& participant : m_Participants)
		{
			assert(participant);
			result.emplace_back(participant->generateDefinition());
		}
		return result;
	}

	int Pick::getActiveParticipantIndex() const
	{
		return m_Forward ? m_CurrentIndex : std::size(m_Participants) - 1 - m_CurrentIndex;
	}

	void Pick::setup(std::vector<const ParticipantData*> _participantData)
	{
		m_Participants.clear();
		m_Participants.reserve(std::size(_participantData));
		std::transform(std::begin(_participantData), std::end(_participantData), std::back_inserter(m_Participants),
			[&owner = *this, i = 0, teams = m_Teams](const auto& _data) mutable
			{
				assert(_data);
				auto team = findTeam(teams, i);
				assert(team);
				return std::make_unique<PickParticipant>(i++, *_data, *team, owner);
			}
		);
		m_State = State::banFactory;

		m_VisualCollector.resize(std::size(m_Participants));

#ifndef NDEBUG
		if (m_PickProperties.debug)
		{
			std::size_t iPlayer = 0;
			for (auto& player : m_PickProperties.debug->players)
			{
				auto& participant = m_Participants.at(iPlayer);
				for (auto factory : player.factories)
				{
					assert(*factory != 0);
					participant->pickFactory(*factory);
				}

				for (auto ability : player.abilities)
				{
					assert(*ability != 0);
					participant->pickAbility(*ability);
				}
				++iPlayer;
			}
			m_State = State::finished;
		}
#endif
	}

	void Pick::_switchParticipant()
	{
		if (++m_CurrentIndex >= std::size(m_Participants))
		{
			m_Forward = !m_Forward;
			m_CurrentIndex = 0;
		}
		m_Timer.start(*m_GameData->gameProperties.pick.maxTime);
	}

	void Pick::_next()
	{
		_switchParticipant();

		assert(m_State != State::none);
		switch (m_State)
		{
		case State::banFactory:
		{
			auto maxBans = *m_PickProperties.maxFactoryBans;
			if (std::all_of(std::begin(m_Participants), std::end(m_Participants),
				[maxBans](const auto& _participant) { return std::size(_participant->getFactoryBans()) >= maxBans; }
			))
			{
				m_State = State::banAbility;
			}
			break;
		}
		case State::banAbility:
		{
			auto maxBans = *m_PickProperties.maxAbilityBans;
			if (std::all_of(std::begin(m_Participants), std::end(m_Participants),
				[maxBans](const auto& _participant) { return std::size(_participant->getAbilityBans()) >= maxBans; }
			))
			{
				m_State = State::pickFactory;
			}
			break;
		}
		case State::pickFactory:
		{
			auto maxPicks = MAX_FACTORIES;
			if (std::all_of(std::begin(m_Participants), std::end(m_Participants),
				[maxPicks](const auto& _participant)
			{
				return std::size(_participant->getFactoryPicks()) >= maxPicks - 1;		// we exclude the wall factory here in counting
			}
			))
			{
				m_State = State::pickAbility;
			}
			break;
		}
		case State::pickAbility:
		{
			auto maxPicks = MAX_ABILITIES;
			if (std::all_of(std::begin(m_Participants), std::end(m_Participants),
				[maxPicks](const auto& _participant) { return std::size(_participant->getAbilityPicks()) >= maxPicks; }
			))
			{
				m_State = State::modSelection;
				m_Timer.start(*m_GameData->gameProperties.pick.modPageSelectionTime);
			}
			break;
		}
		case State::modSelection:
			m_State = State::finished;
			break;
		}
	}

	ID _getRandomID(const IDs& _ids)
	{
		if (std::empty(_ids))
			throw std::logic_error("PickPhase: Participant has no pickable ids.");
		return _ids[randomInt<std::size_t>(0, std::size(_ids) - 1)];
	}

	void Pick::_timerExpired()
	{
		auto& activeParticipant = getActiveParticipant();
		switch (m_State)
		{
		case State::banFactory:
			banFactory(activeParticipant, _getRandomID(getBanableFactories(activeParticipant)));
			break;
		case State::banAbility:
			banAbility(activeParticipant, _getRandomID(getBanableAbilities(activeParticipant)));
			break;
		case State::pickFactory:
			pickFactory(activeParticipant, _getRandomID(getPickableFactories(activeParticipant)));
			break;
		case State::pickAbility:
			pickAbility(activeParticipant, _getRandomID(getPickableAbilities(activeParticipant)));
			break;
		case State::modSelection:
			_next();
			break;
		}
	}

	const Pick::Participants& Pick::getParticipants() const
	{
		return m_Participants;
	}

	PickParticipant& Pick::getActiveParticipant() const
	{
		return *m_Participants.at(getActiveParticipantIndex());
	}

	Pick::State Pick::getState() const
	{
		return m_State;
	}

	Time Pick::getTime() const
	{
		return m_Timer.getTimerValueInMsec();
	}

	const data::Game& Pick::getGameData() const
	{
		assert(m_GameData);
		return *m_GameData;
	}

	VisualCollector& Pick::getVisualCollector()
	{
		return m_VisualCollector;
	}

	const VisualCollector& Pick::getVisualCollector() const
	{
		return m_VisualCollector;
	}

	void Pick::start()
	{
		assert(!m_Participants.empty());
		m_CurrentIndex = 0;
		m_Forward = randomBool();
		m_Timer.start(*m_PickProperties.maxTime);
	}

	void Pick::finalize()
	{
	}

	Data Pick::update(Time _diff)
	{
		if (!hasFinished())
		{
			assert(isValid());

			m_Timer.update(_diff);
			if (m_Timer.getState() == Timer::State::justFinished)
				_timerExpired();

			m_VisualCollector.swap();
		}
		return PickState();
	}

	bool Pick::hasFinished() const
	{
		return m_State == State::finished;
	}

	bool Pick::isValid() const
	{
		return std::size(m_Participants) >= 2;
	}

	IDs Pick::getPickableFactories(const PickParticipant& _participant) const
	{
		return _extractPickableIds(m_BannedFactories, m_PickedFactories,
			_participant.getData().getUnlocks(UnlockType::factory), getGameData().factories, m_PickIdPreparer);
	}

	IDs Pick::getPickableAbilities(const PickParticipant& _participant) const
	{
		return _extractPickableIds(m_BannedAbilities, m_PickedAbilities,
			_participant.getData().getUnlocks(UnlockType::ability), getGameData().abilities, m_PickIdPreparer);
	}

	IDs Pick::getBanableFactories(const PickParticipant& _participant) const
	{
		return _extractBanableIds(m_BannedFactories, getGameData().factories, m_BanIdPreparer);
	}

	IDs Pick::getBanableAbilities(const PickParticipant& _participant) const
	{
		return _extractBanableIds(m_BannedAbilities, getGameData().abilities, m_BanIdPreparer);
	}

	Type Pick::getType() const
	{
		return Type::pick;
	}
} // namespace game::phase
