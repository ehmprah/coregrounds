#pragma once

namespace game::controller
{
	class Player;

	class Handler
	{
	public:
		Handler(ParticipantData _participantData);
		virtual ~Handler() = default;

		void setController(Player& _controller);
		const ParticipantData& getParticipantData() const;

		virtual void sendMessage(const protobuf::out::Message& _msg) = 0;

	protected:
		void handleEvent(protobuf::in::Message _msg);

		void connected();
		void disconnected();

	private:
		std::mutex m_ControllerMx;
		Player* m_Controller = nullptr;
		ParticipantData m_ParticipantData;
	};
} // namespace game::controller

