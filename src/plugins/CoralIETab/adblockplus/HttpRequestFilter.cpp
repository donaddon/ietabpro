#include "stdafx.h"

#include "HttpRequestFilters.h"

namespace adblockplus
{
	std::vector<RegExpFilter *> HttpRequestFilters::m_blacklist;
	std::vector<RegExpFilter *> HttpRequestFilters::m_whitelist;

	HANDLE HttpRequestFilters::m_hWriteEvent = NULL;
	volatile LONG HttpRequestFilters::m_nConsumerCounter = 0;

	void HttpRequestFilters::init()
	{
		if ( NULL == m_hWriteEvent )
		{
			TCHAR szEventName[MAX_PATH];
			_sntprintf_s(szEventName, MAX_PATH, _T("SetRequestFilter_%.4X"), GetCurrentProcessId());
			m_hWriteEvent = CreateEvent( NULL, TRUE, TRUE, szEventName );
		}
	}
	
	bool HttpRequestFilters::test( const HttpRequest * request )
	{
		enum { UNDETERMINED, BLOCK, ALLOW } action = UNDETERMINED;

		static const DWORD WAIT_TIME = 100;
		if ( WaitForSingleObject( m_hWriteEvent, WAIT_TIME ) == WAIT_OBJECT_0 )		// 等待写操作完成
		{
			InterlockedIncrement( & m_nConsumerCounter );						// 开始读队列, 增加计数, 使得写操作知道现在还有读操作没有结束

			// 首先检查白名单
			for (auto iter = m_whitelist.begin(); iter != m_whitelist.end(); iter++)
			{
				RegExpFilter * filter = * iter;
				if ( filter )
				{
					if ( filter->match(request) )
					{
						action = ALLOW;
						break;
					}
				}
			}

			if ( UNDETERMINED == action )
			{
				// 然后测试黑名单
				for (auto iter = m_blacklist.begin(); iter != m_blacklist.end(); iter++)
				{
					RegExpFilter * filter = * iter;
					if ( filter )
					{
						if ( filter->match(request) )
						{
							action = BLOCK;
							break;
						}
					}
				}
			}

			InterlockedDecrement( & m_nConsumerCounter );						// 读操作完毕
		}

		return BLOCK == action;
	}

	void HttpRequestFilters::setBlacklist(const std::vector<RegExpFilter *> & blacklist)
	{
		LockedCopy(blacklist, m_blacklist);
	}
	
	void HttpRequestFilters::setWhitelist(const std::vector<RegExpFilter *> & whitelist)
	{
		LockedCopy(whitelist, m_whitelist);
	}

	bool HttpRequestFilters::LockedCopy(const std::vector<RegExpFilter *> & source, std::vector<RegExpFilter *> & target)
	{
		bool b = false;

		// 这是一个典型的 Producer/Consumer 的线程同步模型
		// 
		if ( ::ResetEvent(m_hWriteEvent) )	// 阻塞所有的读操作
		{
			if ( waitForRead() )			// 可能还有读操作没有结束, 等待
			{
				resetFilterList<RegExpFilter>(target);
				target = source;

				b = true;
			}

			::SetEvent( m_hWriteEvent );	// 释放读操作
		}

		return b;
	}

	bool HttpRequestFilters::waitForRead()
	{
		static const DWORD WAIT_TIME = 1000;		// 超时: 1s
		static const DWORD INTERVAL = 100;

		for ( int i = 0; i < WAIT_TIME / INTERVAL; i++ )		
		{
			if ( m_nConsumerCounter > 0 )
			{
				Sleep(INTERVAL);
			}
			else
			{
				break;
			}
		}

		return m_nConsumerCounter == 0;
	}
}