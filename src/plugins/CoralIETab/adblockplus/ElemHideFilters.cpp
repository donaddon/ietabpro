#include "stdafx.h"

#include "ElemHideFilters.h"

namespace adblockplus
{
	std::vector<ElemHideSelector *> ElemHideFilters::m_blacklist;
	std::vector<ElemHideSelector *> ElemHideFilters::m_whitelist;
	CriticalSection ElemHideFilters::m_cs;

	void ElemHideFilters::setBlacklist(const std::vector<ElemHideSelector *> & blacklist)
	{
		resetFilterList<ElemHideSelector>(m_blacklist);
		m_blacklist = blacklist;
	}

	void ElemHideFilters::setWhitelist(const std::vector<ElemHideSelector *> & whitelist)
	{
		resetFilterList<ElemHideSelector>(m_whitelist);
		m_whitelist = whitelist;
	}
}