#pragma once

#include <string>
#include <vector>
#include <set>

namespace adblockplus
{
	/** HTTP ����, ������ RegExpFilter ���ݲ��� */
	struct HttpRequest
	{
		/** URL */
		const char * url;
		/** Referer */
		const char * referer;
		/** Content Type, nsIContentPolicy ���������� */
		long contentType;
		/** �Ƿ��������� */
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
		 * ���캯��
		 *
		 * @param regexp	����. adblockplus ���� |http://mat1.$object,domain=qq.com ��Ӧ���� |http://mat1.
		 * @param options	����. adblockplus ���� |http://mat1.$object,domain=qq.com ��Ӧ���� object,domain=qq.com
		 */
		static RegExpFilter * fromText(const std::string & regexp, const std::string & options);

		/** ���Ը��� HTTP �����Ƿ��뱾����ƥ�� */
		bool match(const HttpRequest * request);

	private:

		/** ���캯�� */
		RegExpFilter();

	private:

		/** ���� Content-Type �Ƿ���� */
		bool matchContentType(long aContentType);
		/** �����Ƿ��ǵ��������� */
		bool matchThirdParty(const HttpRequest * request);
		/** ���� domain */
		bool matchDomains(const HttpRequest * request );

	private:

		/** ���������б� */
		void parseOptions(const std::string & options);

		/** ���� Content Type ����, ���� |http://gg.$subdocument,image,script,object */
		bool _parseContentType(const char * option, bool versa);
		/** ���� third-party ����, ���� cop.my.xunlei.com$third-party */
		bool _parseThirdParty(const char * option, bool versa);
		/** ���� domain ����, ���� |http://mat1.$object,domain=qq.com */
		bool _parseDomain(const char * option);

	private:

		/** ������ʽ����. adblockplus ���� |http://mat1.$object,domain=qq.com ��Ӧ���� ^http:\/\/mat1\. */
		std::string m_RegExp;

		/** ������Ч�� domain. adblockplus ���� |http://mat1.$object,domain=qq.com ��Ӧ���� qq.com */
		std::vector<std::string> m_ActiveDomains;

		/** ����Ҫ�ų��� domain. adblockplus ���� images.sohu.com/cs/$domain=~wenda.sogou.com ��Ӧ���� wenda.sogou.com */
		std::vector<std::string> m_DisabledDomains;

		/** ������Ч�� contentType. adblockplus ���� |http://gg.$subdocument,image,script,object ��Ӧ���� subdocument,image,script,object */
		std::set<int> m_ActiveContentTypes;

		/** ����Ҫ�ų��� contentType. adblockplus ���� @@||adsense.easylife.tw$~script ��Ӧ���� script */
		std::set<int> m_DisabledContentTypes;

		/** �����Ƿ�ֻ�� third-party ��Ч. ���� cop.my.xunlei.com$third-party */
		bool m_bThirdParty;

		/** �� m_bThirdParty �෴, �Ƿ�ֻ������ԭҳ���������Ч. @@google.com$~third-party */
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

		/** ������Ч�� domain �б�. adblockplus ���� 163.com##*\[class^="ad"] ��Ӧ���� 163.com */
		std::vector<std::string> m_selectorDomains;

		/** CSS Selector. adblockplus ���� 163.com##*\[class^="ad"] ��Ӧ���� *\[class^="ad"] */
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