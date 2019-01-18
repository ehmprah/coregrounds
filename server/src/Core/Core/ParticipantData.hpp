#pragma once

#include "Core/SharedDefines.hpp"

namespace game
{
	class ParticipantData
	{
	public:
		struct Human
		{
			ID uid = 0;
		};

		struct Bot
		{
			int difficulty = 0;
		};
		using TypeData = std::variant<Human, Bot>;

		ParticipantData(TypeData _typeData, std::string _name, ID _icon, IDs _skins, Unlocks _unlocks, ID _colorId,
			std::vector<ModPage> _modPages, std::optional<ID> _lastModPageId);

		const TypeData& getTypeData() const;
		ID getColorId() const;
		ID getIconId() const;
		std::string_view getName() const;
		const IDs& getSkins() const;
		const IDs& getUnlocks(UnlockType _type) const;
		const std::vector<ModPage>& getModPages() const;
		const std::optional<ID>& getLastModPageId() const;

	private:
		std::string m_Name;
		TypeData m_TypeData;
		Unlocks m_Unlocks;
		ID m_ColorId;
		ID m_IconId;
		IDs m_Skins;
		std::vector<ModPage> m_ModPages;
		std::optional<ID> m_LastModPageId;
	};
} // namespace game
