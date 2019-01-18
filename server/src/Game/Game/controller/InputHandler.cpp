#include "stdafx.h"
#include "InputHandler.hpp"
#include "Game/Entity/Participant.hpp"
#include "Game/Phase/AbstractGamePhase.hpp"
#include "Game/Phase/PickPhase.hpp"
#include "Game/Phase/FinalizePhase.hpp"

namespace game::controller
{
	template <class Message, class Func>
	void _tryExec(const Message& _msg, Func _func)
	{
		for (auto& msg : _msg)
		{
			try
			{
				_func(msg);
			}
			catch (const std::exception& _e)
			{
				LOG_ERR("Command: caught exception: \"" << _e.what() << "\"");
			}
			catch (...)
			{
				LOG_ERR("Command: caught unspecified exception.");
			}
		}
	}

	void _execFactoryActivations(Participant& _participant, const protobuf::in::Match::Game& _msg, phase::AbstractGame& _gamePhase)
	{
		if (isRunning(_gamePhase))
		{
			_tryExec(_msg.factoryactivations(), [&_participant, &_gamePhase](const auto& _msg) {
				auto mask = static_cast<std::uint32_t>(std::pow(2, 16) - 1);
				auto& factory = _participant.getFactories().at(static_cast<std::size_t>(_msg.index()));
				assert(factory);
				factory->activate(AbsPosition(_msg.position() & mask, (_msg.position() >> 16) & mask) / 1000);
			});
		}
	}

	void _execFactoryUpgrades(Participant& _participant, const protobuf::in::Match::Game& _msg, phase::AbstractGame& _gamePhase)
	{
		if (isRunning(_gamePhase))
		{
			_tryExec(_msg.factoryupgrades(), [&_participant, &_gamePhase](const auto& _msg) {
				auto& factory = _participant.getFactories().at(static_cast<std::size_t>(_msg.index()));
				assert(factory);
				factory->upgrade(_msg.upgrade());
			});
		}
	}

	void _execAbilityActivations(Participant& _participant, const protobuf::in::Match::Game& _msg, phase::AbstractGame& _gamePhase)
	{
		if (isRunning(_gamePhase))
		{
			_tryExec(_msg.abilityactivations(), [&_participant, &_gamePhase](const auto& _msg) {
				auto mask = static_cast<std::uint32_t>(std::pow(2, 16) - 1);
				auto& ability = _participant.getAbilities().at(static_cast<std::size_t>(_msg.index()));
				assert(ability);
				ability->activate(AbsPosition(_msg.position() & mask, (_msg.position() >> 16) & mask) / 1000);
			});
		}
	}

	void _execTargetPreferenceChanges(Participant& _participant, const protobuf::in::Match::Game& _msg, phase::AbstractGame& _gamePhase)
	{
		if (isRunning(_gamePhase))
		{
			_tryExec(_msg.targetpreferencechanges(), [&_participant, &_gamePhase](const auto& _msg) {
				auto mask = static_cast<std::uint32_t>(std::pow(2, 15) - 1);
				TargetPreference preference;
				preference.target.set_mask(_msg.targetpreference() & mask);
				preference.mode = fromInt<unit::targetPreference::Mode>((_msg.targetpreference() >> 15) & mask);

				auto& factory = _participant.getFactories().at(static_cast<std::size_t>(_msg.index()));
				assert(factory);
				factory->setTargetPreference(preference);
			});
		}
	}

	void _execGlobalTargetChanges(Participant& _participant, const protobuf::in::Match::Game& _msg, phase::AbstractGame& _gamePhase)
	{
		if (isRunning(_gamePhase))
		{
			_tryExec(_msg.globaltargetchanges(), [&_participant](const auto& _msg) {
				_participant.setGlobalTarget(_msg.guid());
			});
		}
	}

	void _execPlayEmotes(Participant& _participant, const protobuf::in::Match::Game& _msg, phase::AbstractGame& _gamePhase)
	{
		_tryExec(_msg.emoteplays(), [&_participant](const auto& _msg) {
			_participant.playEmote(_msg.id());
		});
	}

	void _execMessage(int _participantIndex, const protobuf::in::Match::Game& _msg, phase::AbstractGame& _gamePhase)
	{
		auto& participant = *_gamePhase.getParticipants().at(_participantIndex);
		_execFactoryActivations(participant, _msg, _gamePhase);
		_execFactoryUpgrades(participant, _msg, _gamePhase);
		_execAbilityActivations(participant, _msg, _gamePhase);
		_execTargetPreferenceChanges(participant, _msg, _gamePhase);
		_execGlobalTargetChanges(participant, _msg, _gamePhase);
		_execPlayEmotes(participant, _msg, _gamePhase);

		if (_msg.surrender())
			participant.surrender();
	}

	void _execPick(PickParticipant& _participant, const protobuf::in::Match::Pick& _msg, phase::Pick& _pickPhase)
	{
		using Target = protobuf::in::Match::Pick::Target;
		_tryExec(_msg.picks(), [&_participant, &_pickPhase](const auto& _msg) {
			switch (_msg.target())
			{
			case Target::Match_Pick_Target_factory: _pickPhase.pickFactory(_participant, _msg.id()); break;
			case Target::Match_Pick_Target_ability: _pickPhase.pickAbility(_participant, _msg.id()); break;
			}
		});
	}

	void _execBan(PickParticipant& _participant, const protobuf::in::Match::Pick& _msg, phase::Pick& _pickPhase)
	{
		using Target = protobuf::in::Match::Pick::Target;
		_tryExec(_msg.bans(), [&_participant, &_pickPhase](const auto& _msg) {
			switch (_msg.target())
			{
			case Target::Match_Pick_Target_factory: _pickPhase.banFactory(_participant, _msg.id()); break;
			case Target::Match_Pick_Target_ability: _pickPhase.banAbility(_participant, _msg.id()); break;
			}
		});
	}

	void _execModPageSelection(PickParticipant& _participant, const protobuf::in::Match::Pick& _msg, phase::Pick& _pickPhase)
	{
		_tryExec(_msg.modpageselections(), [&_participant, &_pickPhase](const auto& _msg) {
			_pickPhase.switchModPage(_participant, _msg.pid());
		});
	}

	void _execPlayEmotes(PickParticipant& _participant, const protobuf::in::Match::Pick& _msg, phase::Pick& _pickPhase)
	{
		_tryExec(_msg.emoteplays(), [&_participant](const auto& _msg) {
			_participant.playEmote(_msg.id());
		});
	}

	void _execMessage(int _participantIndex, const protobuf::in::Match::Pick& _msg, phase::Pick& _pickPhase)
	{
		auto& participant = *_pickPhase.getParticipants().at(_participantIndex);

		_execPick(participant, _msg, _pickPhase);
		_execBan(participant, _msg, _pickPhase);
		_execModPageSelection(participant, _msg, _pickPhase);
		_execPlayEmotes(participant, _msg, _pickPhase);
	}

	void _execPlayEmotes(FinalizeParticipant& _participant, const protobuf::in::Match::Finalize& _msg, phase::Finalize& _finalizePhase)
	{
		_tryExec(_msg.emoteplays(), [&_participant](const auto& _msg) {
			_participant.playEmote(_msg.id());
		});
	}

	void _execMessage(int _participantIndex, const protobuf::in::Match::Finalize& _msg, phase::Finalize& _finalizePhase)
	{
		auto& participant = *_finalizePhase.getParticipants().at(_participantIndex);

		_execPlayEmotes(participant, _msg, _finalizePhase);
	}

	/*#####
	# InputHandler
	#####*/
	InputHandler::InputHandler(phase::Pick& _pickPhase, phase::AbstractGame& _gamePhase, phase::Finalize& _finalizePhase) :
		m_PickPhase(_pickPhase),
		m_GamePhase(_gamePhase),
		m_FinalizePhase(_finalizePhase)
	{
	}

	void InputHandler::add(int _participantIndex, protobuf::in::Message _msg)
	{
		if (!_msg.has_match())
			return;

		assert(0 <= _participantIndex);
		std::scoped_lock<std::mutex> lock(m_MessagesMx);
		m_Messages.emplace_back(_participantIndex, std::move(_msg));
	}

	void InputHandler::exec(phase::Type _curPhase)
	{
		decltype(m_Messages) execMessages;
		{
			std::scoped_lock<std::mutex> lock(m_MessagesMx);
			execMessages = std::move(m_Messages);
		}

		_tryExec(execMessages,
			[_curPhase, &pickPhase = m_PickPhase, &gamePhase = m_GamePhase, &finalizePhase = m_FinalizePhase](const auto& _pair)
			{
				switch (_curPhase)
				{
				case phase::Type::pick:
					_execMessage(_pair.first, _pair.second.match().pick(), pickPhase);
					break;
				case phase::Type::game:
					_execMessage(_pair.first, _pair.second.match().game(), gamePhase);
					break;
				case phase::Type::finalize:
					_execMessage(_pair.first, _pair.second.match().finalize(), finalizePhase);
					break;
				}
			}
		);
	}
} // namespace game::controller
