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
		if ( WaitForSingleObject( m_hWriteEvent, WAIT_TIME ) == WAIT_OBJECT_0 )		// �ȴ�д�������
		{
			InterlockedIncrement( & m_nConsumerCounter );						// ��ʼ������, ���Ӽ���, ʹ��д����֪�����ڻ��ж�����û�н���

			// ���ȼ�������
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
				// Ȼ����Ժ�����
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

			InterlockedDecrement( & m_nConsumerCounter );						// ���������
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

		// ����һ�����͵� Producer/Consumer ���߳�ͬ��ģ��
		// 
		if ( ::ResetEvent(m_hWriteEvent) )	// �������еĶ�����
		{
			if ( waitForRead() )			// ���ܻ��ж�����û�н���, �ȴ�
			{
				resetFilterList<RegExpFilter>(target);
				target = source;

				b = true;
			}

			::SetEvent( m_hWriteEvent );	// �ͷŶ�����
		}

		return b;
	}

	bool HttpRequestFilters::waitForRead()
	{
		static const DWORD WAIT_TIME = 1000;		// ��ʱ: 1s
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