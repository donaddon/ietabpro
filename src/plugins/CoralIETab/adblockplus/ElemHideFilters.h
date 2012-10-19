#pragma once

#include <vector>

#include "../CriticalSection.h"

#include "Filter.h"

namespace adblockplus
{
	class ElemHideFilters
	{
	public:

		/** ���ù��� */
		static void setBlacklist(const std::vector<ElemHideSelector *> & blacklist);
		static void setWhitelist(const std::vector<ElemHideSelector *> & whitelist);

	private:

		/** ������ */
		static std::vector<ElemHideSelector *> m_blacklist;

		/** ������(��ʱ��֧��) */
		static std::vector<ElemHideSelector *> m_whitelist;
		
		/** �߳��� */
		static CriticalSection m_cs;
	};
}