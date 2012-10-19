#include "stdafx.h"

#include "../Misc.h"

#include "FilterList.h"
#include "ElemHideFilters.h"
#include "HttpRequestFilters.h"

namespace adblockplus
{
	std::vector<std::string> FilterList::blacklist;
	std::vector<std::string> FilterList::whitelist;
	std::set<std::string> FilterList::disabled_blacklist;
	std::set<std::string> FilterList::disabled_whitelist;
	std::map<std::string, Filter *> FilterList::knownFilters;

	void FilterList::reset()
	{
		blacklist.clear();
		whitelist.clear();
		disabled_blacklist.clear();
		disabled_whitelist.clear();

		// 为了提高性能，预先为 vector 分配一段空间
		static const size_t INIT_CAP_BLACKLIST = 1000;
		static const size_t INIT_CAP_WHITELIST = 500;
		blacklist.reserve(INIT_CAP_BLACKLIST);
		whitelist.reserve(INIT_CAP_WHITELIST);
	}

	void FilterList::addBlacklist( const std::string & filter, bool disabled )
	{
		if ( ! disabled )
		{
			FilterList::blacklist.push_back(filter);
		}
		else
		{
			FilterList::disabled_blacklist.insert(filter);
		}
	}

	void FilterList::addWhitelist( const std::string & filter, bool disabled )
	{
		if ( ! disabled )
		{
			FilterList::whitelist.push_back(filter);
		}
		else
		{
			FilterList::disabled_whitelist.insert(filter);
		}
	}

	void FilterList::convert()
	{
		std::vector<RegExpFilter *> regexpBlacklist;
		std::vector<RegExpFilter *> regexpWhitelist;
		std::vector<ElemHideSelector *> elemHideBlacklist;
		std::vector<ElemHideSelector *> elemHideWhitelist;

		// 考虑到多线程的情况，在改变 vector 内容的时候必须要加锁, 为了提高性能，我们要尽可能缩短加锁的时间
		// 所以这里进行了优化，因为 convert() 很慢，所以把它做成 thread safe 的，而必须加锁的地方单独剥离
		// 出来
		FilterList::convert(FilterList::blacklist, FilterList::disabled_blacklist, regexpBlacklist, elemHideBlacklist);
		FilterList::convert(FilterList::whitelist, FilterList::disabled_whitelist, regexpWhitelist, elemHideWhitelist);

		HttpRequestFilters::setBlacklist(regexpBlacklist);
		HttpRequestFilters::setWhitelist(regexpWhitelist);
		ElemHideFilters::setBlacklist(elemHideBlacklist);
		ElemHideFilters::setWhitelist(elemHideWhitelist);
	}

	void FilterList::convert(const std::vector<std::string> & ruleList, const std::set<std::string> & disabled_list, std::vector<RegExpFilter *> & regexpList, std::vector<ElemHideSelector *> & elemHideFilter)
	{
		static const char * ELEMHIDE_PATTERN = "^([^\\/\\*\\|\\@\"]*?)#(?:([\\w\\-]+|\\*)((?:\\([\\w\\-]+(?:[$^*]?=[^\\(\\)\"]*)?\\))*)|#([^{}]+))$";
		static const char * REGEXPR_PATTERN = "^\\/(.*)\\/$";
		static const char * OPTIONS_PATTERN = "\\$(~?[\\w\\-]+(?:=[^,\\s]+)?(?:,~?[\\w\\-]+(?:=[^,\\s]+)?)*)$";

		for ( auto iter = ruleList.begin(); iter != ruleList.end(); iter++ )
		{
			std::string rule( * iter );

			if ( disabled_list.find(rule) != disabled_list.end() )
			{
				// 被禁用的规则，不做任何处理
				continue;
			}

			auto pos = knownFilters.find(rule);
			if ( pos != knownFilters.end() )
			{
				Filter * p = pos->second;
				if ( p )
				{
					RegExpFilter * regexpFilter = dynamic_cast<RegExpFilter *>(p);
					if ( regexpFilter )
					{
						regexpList.push_back(regexpFilter);
						// 加引用计数, 以免被释放
						regexpFilter->AddRef();
					}
					else
					{
						ElemHideSelector * selector = dynamic_cast<ElemHideSelector *>(p);
						if ( selector )
						{
							elemHideFilter.push_back(selector);
							// 加引用计数, 以免被释放
							selector->AddRef();
						}
						else
						{
							// 不应该走到这里, 但是如果真到这里了, 把 p 从 map 里面去掉
							knownFilters.erase(pos);
							p->Release();
						}
					}

					continue;
				}
			}

			// 元素隐藏规则
			std::string results[4];
			if ( regexpr_match(ELEMHIDE_PATTERN, rule.c_str(), FALSE, results, 4) )
			{
				const std::string & domain = results[0];
				const std::string & tagName = results[1];
				const std::string & attrRules = results[2];
				const std::string & selector = results[3];
				if ( ElemHideSelector * filter = ElemHideSelector::fromText(domain, tagName, attrRules, selector) )
				{
					elemHideFilter.push_back(filter);

					knownFilters[rule] = filter;
				}
			}
			else
			{
				std::string re_rule(rule);

				// 访问过滤规则
				std::string options_str;
				if ( regexpr_match(OPTIONS_PATTERN, re_rule.c_str(), FALSE, &options_str, 1) )
				{
					// 把参数部分去掉
					regexpr_replace(OPTIONS_PATTERN, re_rule.c_str(), "", re_rule);
				}
				
				if ( regexpr_match(REGEXPR_PATTERN, re_rule.c_str(), FALSE, results, 1) )
				{
					re_rule = results[0];
				}
				else
				{
					// 将 adblockplus 规则转换成正则表达式规则
					regexpr_replace("\\*+", re_rule.c_str(), "*", re_rule, TRUE);
					regexpr_replace("\\^\\|$", re_rule.c_str(), "^", re_rule, FALSE);
					regexpr_replace("(\\W)", re_rule.c_str(), "\r\\1", re_rule, TRUE);
					string_replace(re_rule, "\r", "\\");
					regexpr_replace("\\\\\\*", re_rule.c_str(), ".*", re_rule, TRUE);
					regexpr_replace("\\\\\\^", re_rule.c_str(), "(?:[^\rw\r-.%\ru0080-\ruFFFF]|$)", re_rule, TRUE);
					regexpr_replace("^\\\\\\|\\\\\\|", re_rule.c_str(), "^[\rw\r-]+:\r/+(?!\r/)(?:[^\r/]+\r.)?", re_rule, FALSE);
					string_replace(re_rule, "\r", "\\");
					regexpr_replace("^\\\\\\|", re_rule.c_str(), "^", re_rule, FALSE);
					regexpr_replace("\\\\\\|$", re_rule.c_str(), "$", re_rule, FALSE);
					regexpr_replace("^(\\.\\*)", re_rule.c_str(), "", re_rule, FALSE);
					regexpr_replace("(\\.\\*)$", re_rule.c_str(), "", re_rule, FALSE);
				}

				if ( RegExpFilter * filter = RegExpFilter::fromText(re_rule, options_str) )
				{
					regexpList.push_back(filter);

					knownFilters[rule] = filter;
				}
			}
		}
	}
}
