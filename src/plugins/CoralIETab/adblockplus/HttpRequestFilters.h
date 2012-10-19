#pragma once

#include <vector>

#include <Windows.h>

#include "Filter.h"

namespace adblockplus
{
	class HttpRequestFilters
	{
	public:

		/** 初始化 */
		static void init();

		/** 测试一个 HTTP 请求是否要被过滤 */
		static bool test(const HttpRequest * request);

		/** 设置规则 */
		static void setBlacklist(const std::vector<RegExpFilter *> & blacklist);
		static void setWhitelist(const std::vector<RegExpFilter *> & whitelist);

	private:

		static bool LockedCopy(const std::vector<RegExpFilter *> & source, std::vector<RegExpFilter *> & target);

		/** 仅供 LockedCopy 调用, 等待 test() 方法读 vector 结束 */
		static bool waitForRead();

	private:

		/** 黑名单 */
		static std::vector<RegExpFilter *> m_blacklist;
		/** 白名单 */
		static std::vector<RegExpFilter *> m_whitelist;

		/** 线程安全锁 */
		static HANDLE m_hWriteEvent;

		/** 计数器: 当前有多少个读操作正在进行 */
		static volatile LONG m_nConsumerCounter;
	};
}