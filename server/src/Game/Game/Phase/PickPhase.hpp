#pragma once

#include "PhaseInterface.hpp"
#include "Game/Entity/Participant.hpp"
#include "Data/Game.hpp"
#include "Game/Entity/Team.hpp"

namespace game::phase
{
	IDs uniqueIdPrepare(const IDs& _src, const IDs& _dest);

	class Pick :
		public Interface
	{
		using super = Interface;

	public:
		using Participants = std::vector<std::unique_ptr<PickParticipant>>;

		enum class State
		{
			none,
			banFactory,
			banAbility,
			pickFactory,
			pickAbility,
			modSelection,
			finished
		};

		Pick(const std::vector<Team>& _teams, std::shared_ptr<const data::Game> _gameData);
		virtual ~Pick() = default;

		void setup(std::vector<const ParticipantData*> _participantData);

		bool pickFactory(PickParticipant& _participant, ID _id);
		bool pickAbility(PickParticipant& _participant, ID _id);
		bool banFactory(PickParticipant& _participant, ID _id);
		bool banAbility(PickParticipant& _participant, ID _id);
		bool switchModPage(PickParticipant& _participant, ID _pid);

		IDs getPickableFactories(const PickParticipant& _participant) const;
		IDs getPickableAbilities(const PickParticipant& _participant) const;
		IDs getBanableFactories(const PickParticipant& _participant) const;
		IDs getBanableAbilities(const PickParticipant& _participant) const;

		std::vector<ParticipantDef> generateParticipantDefinitions() const;
		int getActiveParticipantIndex() const;
		PickParticipant& getActiveParticipant() const;
		const Participants& getParticipants() const;
		State getState() const;
		Time getTime() const;
		const data::Game& getGameData() const;

		VisualCollector& getVisualCollector() override;
		const VisualCollector& getVisualCollector() const override;

		virtual void start() override;
		virtual void finalize() override;
		virtual Data update(Time _diff) override;

		virtual bool hasFinished() const override;
		bool isValid() const;
		virtual Type getType() const override;

	private:
		State m_State = State::none;
		std::shared_ptr<const data::Game> m_GameData;
		const data::GameProperties::Pick& m_PickProperties;
		Participants m_Participants;
		const std::vector<Team>& m_Teams;
		std::size_t m_CurrentIndex;
		Timer m_Timer;
		bool m_Forward = true;
		VisualCollector m_VisualCollector;
		using IdPreparer = std::function<IDs(const IDs&, const IDs&)>;
		IdPreparer m_BanIdPreparer;
		IdPreparer m_PickIdPreparer;

		IDs m_BannedFactories;
		IDs m_PickedFactories;
		IDs m_BannedAbilities;
		IDs m_PickedAbilities;

		void _switchParticipant();
		void _next();
		void _timerExpired();
	};
	using PickPtr = std::unique_ptr<Pick>;
} // namespace game::phase