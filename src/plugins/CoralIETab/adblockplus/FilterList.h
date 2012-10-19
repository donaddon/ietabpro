#pragma once

#include <string>
#include <vector>
#include <set>
#include <map>

#include "Filter.h"

namespace adblockplus
{
	class FilterList
	{
	private:

		friend class FilterStorage;

		/** ��չ����б�, һ������׼���ؽ������б��ʱ�� */
		static void reset();

		/** ����һ�������� */
		static void addBlacklist(const std::string & filter, bool disabled);

		/** ����һ�������� */
		static void addWhitelist(const std::string & filter, bool disabled);

		/** �������б�� adblockplus �﷨ת�� Filter ���� */
		static void convert();
		static void convert(const std::vector<std::string> & ruleList, const std::set<std::string> & disabled_list, std::vector<RegExpFilter *> & regexpList, std::vector<ElemHideSelector *> & elemHideFilter);

		/** ������ */
		static std::vector<std::string> blacklist;

		/** ������ */
		static std::vector<std::string> whitelist;

		/** �����õĺ����� */
		static std::set<std::string> disabled_blacklist;

		/** �����õİ����� */
		static std::set<std::string> disabled_whitelist;

		/** ����������ʽת������, ������ǻ�����һ���Ѿ�ת���˵Ĺ��� */
		static std::map<std::string, Filter *> knownFilters;
	};
}