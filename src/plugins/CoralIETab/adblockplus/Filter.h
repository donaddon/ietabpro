#pragma once

#include <string>
#include <vector>
#include <set>

namespace adblockplus
{
	/** HTTP 请求, 用来向 RegExpFilter 传递参数 */
	struct HttpRequest
	{
		/** URL */
		const char * url;
		/** Referer */
		const char * referer;
		/** Content Type, nsIContentPolicy 的数据类型 */
		long contentType;
		/** 是否是子请求 */
		bool subRequest;
	};

	class Filter
	{
	public:
		
		Filter();

		virtual ULONG AddRef();

		virtual ULONG Release();

	protected:

		LONG m_nRefCount;
	};

	class RegExpFilter : public Filter
	{
	public:

		/**
		 * 构造函数
		 *
		 * @param regexp	规则. adblockplus 规则 |http://mat1.$object,domain=qq.com 对应的是 |http://mat1.
		 * @param options	参数. adblockplus 规则 |http://mat1.$object,domain=qq.com 对应的是 object,domain=qq.com
		 */
		static RegExpFilter * fromText(const std::string & regexp, const std::string & options);

		/** 测试给定 HTTP 请求是否与本规则匹配 */
		bool match(const HttpRequest * request);

	private:

		/** 构造函数 */
		RegExpFilter();

	private:

		/** 测试 Content-Type 是否相符 */
		bool matchContentType(long aContentType);
		/** 测试是否是第三方请求 */
		bool matchThirdParty(const HttpRequest * request);
		/** 测试 domain */
		bool matchDomains(const HttpRequest * request );

	private:

		/** 解析参数列表 */
		void parseOptions(const std::string & options);

		/** 解析 Content Type 规则, 例如 |http://gg.$subdocument,image,script,object */
		bool _parseContentType(const char * option, bool versa);
		/** 解析 third-party 规则, 例如 cop.my.xunlei.com$third-party */
		bool _parseThirdParty(const char * option, bool versa);
		/** 解析 domain 规则, 例如 |http://mat1.$object,domain=qq.com */
		bool _parseDomain(const char * option);

	private:

		/** 正则表达式规则. adblockplus 规则 |http://mat1.$object,domain=qq.com 对应的是 ^http:\/\/mat1\. */
		std::string m_RegExp;

		/** 规则生效的 domain. adblockplus 规则 |http://mat1.$object,domain=qq.com 对应的是 qq.com */
		std::vector<std::string> m_ActiveDomains;

		/** 规则要排除的 domain. adblockplus 规则 images.sohu.com/cs/$domain=~wenda.sogou.com 对应的是 wenda.sogou.com */
		std::vector<std::string> m_DisabledDomains;

		/** 规则生效的 contentType. adblockplus 规则 |http://gg.$subdocument,image,script,object 对应的是 subdocument,image,script,object */
		std::set<int> m_ActiveContentTypes;

		/** 规则要排除的 contentType. adblockplus 规则 @@||adsense.easylife.tw$~script 对应的是 script */
		std::set<int> m_DisabledContentTypes;

		/** 规则是否只对 third-party 生效. 例如 cop.my.xunlei.com$third-party */
		bool m_bThirdParty;

		/** 与 m_bThirdParty 相反, 是否只对来自原页面的请求生效. @@google.com$~third-party */
		bool m_bFirstParty;
	};

	class ElemHideSelector : public Filter
	{
	public:

		/**
		 * Creates an element hiding filter from a pre-parsed text representation
		 *
		 * @param {const std::string &} domain     domain part of the text representation (can be empty)
		 * @param {const std::string &} tagName    tag name part (can be empty)
		 * @param {const std::string &} attrRules  attribute matching rules (can be empty)
		 * @param {const std::string &} selector   raw CSS selector (can be empty)
		 * @return {ElemHiddingFilter or NULL}
		 */
		static ElemHideSelector * fromText(const std::string & domain, const std::string & tagName, const std::string & attrRules, const std::string & selector);

	private:

		/** 规则生效的 domain 列表. adblockplus 规则 163.com##*\[class^="ad"] 对应的是 163.com */
		std::vector<std::string> m_selectorDomains;

		/** CSS Selector. adblockplus 规则 163.com##*\[class^="ad"] 对应的是 *\[class^="ad"] */
		std::string m_selector;
	};

	template<typename T>
	void resetFilterList(std::vector<T *> & filterList)
	{
		try
		{
			for (auto iter = filterList.begin(); iter != filterList.end(); iter++)
			{
				T * filter = * iter;
				if ( filter )
				{
					filter->Release();

					* iter = NULL;
				}
			}
		}
		catch (...)	{}

		try
		{
			filterList.clear();
		}
		catch (...)	{}
	}
}