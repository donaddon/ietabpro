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

		/** 清空规则列表, 一般用于准备重建规则列表的时候 */
		static void reset();

		/** 增加一条黑名单 */
		static void addBlacklist(const std::string & filter, bool disabled);

		/** 增加一条白名单 */
		static void addWhitelist(const std::string & filter, bool disabled);

		/** 将规则列表从 adblockplus 语法转成 Filter 对象 */
		static void convert();
		static void convert(const std::vector<std::string> & ruleList, const std::set<std::string> & disabled_list, std::vector<RegExpFilter *> & regexpList, std::vector<ElemHideSelector *> & elemHideFilter);

		/** 黑名单 */
		static std::vector<std::string> blacklist;

		/** 白名单 */
		static std::vector<std::string> whitelist;

		/** 被禁用的黑名单 */
		static std::set<std::string> disabled_blacklist;

		/** 被禁用的白名单 */
		static std::set<std::string> disabled_whitelist;

		/** 由于正则表达式转换很慢, 因此我们缓存了一份已经转换了的规则 */
		static std::map<std::string, Filter *> knownFilters;
	};
}