#include "ParticipantData.hpp"

namespace game
{
	void _removeInvalidMods(const IDs& _modUnlocks, IDs& _mods)
	{
		assert(std::is_sorted(std::begin(_modUnlocks), std::end(_modUnlocks)));
		_mods.erase(std::remove_if(std::begin(_mods), std::end(_mods),
			[&_modUnlocks](auto _modId) { return !std::binary_search(std::begin(_modUnlocks), std::end(_modUnlocks), _modId); }
		), std::end(_mods));
	}

	ID _checkColorId(const IDs& _colorUnlocks, ID _colorId)
	{
		assert(!_colorUnlocks.empty());
		if (std::find(std::begin(_colorUnlocks), std::end(_colorUnlocks), _colorId) == std::end(_colorUnlocks))
		{
			auto min = std::min_element(std::begin(_colorUnlocks), std::end(_colorUnlocks));
			return *min;
		}
		return _colorId;
	}

	ParticipantData::ParticipantData(TypeData _typeData, std::string _name, ID _icon, IDs _skins, Unlocks _unlocks, ID _colorId,
		std::vector<ModPage> _modPages, std::optional<ID> _lastModPageId) :
		m_TypeData(_typeData),
		m_Name(std::move(_name)),
		m_IconId(_icon),
		m_Skins(std::move(_skins)),
		m_Unlocks(std::move(_unlocks)),
		m_ColorId(_checkColorId(getUnlocks(UnlockType::color), _colorId)),
		m_ModPages(std::move(_modPages)),
		m_LastModPageId(_lastModPageId)
	{
		assert(std::all_of(std::begin(m_Unlocks), std::end(m_Unlocks),
			[](const auto& _ids) { return std::is_sorted(std::begin(_ids), std::end(_ids)); }
		));

		for (auto& page : m_ModPages)
			_removeInvalidMods(getUnlocks(UnlockType::modification), std::get<IDs>(page));

		if (!m_LastModPageId && !std::empty(m_ModPages))
			m_LastModPageId = std::get<ID>(m_ModPages.front());
	}

	const ParticipantData::TypeData& ParticipantData::getTypeData() const
	{
		return m_TypeData;
	}

	ID ParticipantData::getColorId() const
	{
		assert(m_ColorId > 0);
		return m_ColorId;
	}

	ID ParticipantData::getIconId() const
	{
		return m_IconId;
	}

	std::string_view ParticipantData::getName() const
	{
		return m_Name;
	}

	const IDs& ParticipantData::getSkins() const
	{
		return m_Skins;
	}

	const IDs& ParticipantData::getUnlocks(UnlockType _type) const
	{
		auto index = static_cast<std::size_t>(_type) - 1;
		assert(index < m_Unlocks.size());
		return m_Unlocks[index];
	}

	const std::vector<ModPage>& ParticipantData::getModPages() const
	{
		return m_ModPages;
	}

	const std::optional<ID>& ParticipantData::getLastModPageId() const
	{
		return m_LastModPageId;
	}
} // namespace game
