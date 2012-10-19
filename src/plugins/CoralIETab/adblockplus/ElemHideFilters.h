#pragma once

#include <vector>

#include "../CriticalSection.h"

#include "Filter.h"

namespace adblockplus
{
	class ElemHideFilters
	{
	public:

		/** 设置规则 */
		static void setBlacklist(const std::vector<ElemHideSelector *> & blacklist);
		static void setWhitelist(const std::vector<ElemHideSelector *> & whitelist);

	private:

		/** 黑名单 */
		static std::vector<ElemHideSelector *> m_blacklist;

		/** 白名单(暂时不支持) */
		static std::vector<ElemHideSelector *> m_whitelist;
		
		/** 线程锁 */
		static CriticalSection m_cs;
	};
}