#pragma once

#include <vector>

#include <Windows.h>

#include "Filter.h"

namespace adblockplus
{
	class HttpRequestFilters
	{
	public:

		/** ��ʼ�� */
		static void init();

		/** ����һ�� HTTP �����Ƿ�Ҫ������ */
		static bool test(const HttpRequest * request);

		/** ���ù��� */
		static void setBlacklist(const std::vector<RegExpFilter *> & blacklist);
		static void setWhitelist(const std::vector<RegExpFilter *> & whitelist);

	private:

		static bool LockedCopy(const std::vector<RegExpFilter *> & source, std::vector<RegExpFilter *> & target);

		/** ���� LockedCopy ����, �ȴ� test() ������ vector ���� */
		static bool waitForRead();

	private:

		/** ������ */
		static std::vector<RegExpFilter *> m_blacklist;
		/** ������ */
		static std::vector<RegExpFilter *> m_whitelist;

		/** �̰߳�ȫ�� */
		static HANDLE m_hWriteEvent;

		/** ������: ��ǰ�ж��ٸ����������ڽ��� */
		static volatile LONG m_nConsumerCounter;
	};
}