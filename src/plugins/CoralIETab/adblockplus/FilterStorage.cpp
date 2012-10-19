#include "StdAfx.h"

#include <stdio.h>
#include <WinInet.h>

#include "../nsConfigManager.h"

#include "FilterList.h"
#include "FilterStorage.h"

namespace adblockplus
{
	std::string FilterStorage::m_patternFilename;

	bool FilterStorage::loadFromDisk(const std::string & patternsFile)
	{
		bool b = false;

		do
		{
			const char * fileName = patternsFile.c_str();
			if ( ( !fileName ) || !fileName[0] ) break;

			FILE * fp = NULL;
			if ( fopen_s(&fp, fileName, "r+") != 0 ) break;		// r+ ģʽ��֤���̣߳����̣�֮��Ļ���, ��������� FilterList �Ĳ����ǰ�ȫ��

			// �������֮ǰ��
			FilterList::reset();

			// ��ȡ�����ڴ���
			FilterStorage::readFile(fp);

			// ת�����ڲ��������
			FilterList::convert();

			fclose(fp);

			b = true;

		} while(false);

		return b;
	}

	void FilterStorage::readFile(FILE * fp)
	{
		static const char * SECTION_FILTER = "[Filter]";
		static const size_t SECTION_FILTER_LENGTH = strlen(SECTION_FILTER);
		static const char * SECTION_SUBSCRIPTION_FILTERS = "[Subscription filters]";
		static const size_t SECTION_SUBSCRIPTION_FILTERS_LENGTH = strlen(SECTION_SUBSCRIPTION_FILTERS);

		// �����ǰ��ж���, ��һ��״̬������¼�����ĸ�����
		typedef enum { STATUS_UNSUPPORTED, STATUS_FILTER_SECTION, STATUS_SUBSCRIPTION_FILTERS_SECTION } ReadStatus;

		ReadStatus readStatus = STATUS_UNSUPPORTED;

		std::string last_filter;

		char line[INTERNET_MAX_URL_LENGTH];
		while ( fgets(line, ARRAYSIZE(line), fp) != NULL )
		{
			switch (line[0])
			{
			case '[':
				{
					if ( strncmp(line, SECTION_FILTER, SECTION_FILTER_LENGTH) == 0 )
					{
						readStatus = STATUS_FILTER_SECTION;
					}
					else
					{
						if ( strncmp(line, SECTION_SUBSCRIPTION_FILTERS, SECTION_SUBSCRIPTION_FILTERS_LENGTH) == 0 )
						{
							readStatus = STATUS_SUBSCRIPTION_FILTERS_SECTION;
						}
						else
						{
							readStatus = STATUS_UNSUPPORTED;
						}
					}

					continue;
				}
			case '\r':
			case '\n':		// ����
			case '!':		// ע��
				{
					continue;
				}
			}

			switch ( readStatus )
			{
			case STATUS_UNSUPPORTED:
				{
					break;
				}
			case STATUS_SUBSCRIPTION_FILTERS_SECTION:
				{
					// ȥ��β���Ļ��з�
					size_t tail_pos = strlen(line);
					if ( '\n' == line[tail_pos-1] ) line[tail_pos-1] = 0;

					if ( '@' == line[0] )
					{
						// ������
						FilterList::addWhitelist(line+2, false);
					}
					else
					{
						FilterList::addBlacklist(line, false);
					}

					break;
				}
			case STATUS_FILTER_SECTION:
				{
					if ( strncmp(line, "text=", 5) == 0 )
					{
						// ȥ��β���Ļ��з�
						size_t tail_pos = strlen(line);
						if ( '\n' == line[tail_pos-1] ) line[tail_pos-1] = 0;

						if ( tail_pos > 5 )
						{
							last_filter = line + 5;
						}
						else
						{
							last_filter.clear();
						}
					}
					else
					{
						if ( strncmp(line, "disabled=true", 13) == 0 )
						{
							// �ѽ��õĴӹ����б�����ȥ�� 
							if ( '@' == last_filter[0] )
							{
								// ������
								FilterList::addWhitelist(last_filter.substr(2), true);
							}
							else
							{
								FilterList::addBlacklist(last_filter, true);
							}
						}
					}

					break;
				}
			}
		}
	}
}