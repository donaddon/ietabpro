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
			if ( fopen_s(&fp, fileName, "r+") != 0 ) break;		// r+ 模式保证了线程（进程）之间的互斥, 所以下面对 FilterList 的操作是安全的

			// 先清除掉之前的
			FilterList::reset();

			// 读取规则到内存中
			FilterStorage::readFile(fp);

			// 转换成内部规则对象
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

		// 规则是按行读的, 用一个状态机来记录读到哪个段了
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
			case '\n':		// 空行
			case '!':		// 注释
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
					// 去掉尾部的换行符
					size_t tail_pos = strlen(line);
					if ( '\n' == line[tail_pos-1] ) line[tail_pos-1] = 0;

					if ( '@' == line[0] )
					{
						// 白名单
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
						// 去掉尾部的换行符
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
							// 把禁用的从规则列表里面去掉 
							if ( '@' == last_filter[0] )
							{
								// 白名单
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