#include "stdafx.h"

#include <algorithm>
#include <WinInet.h>

#include "nsIContentPolicy.h"

#include "../Misc.h"
#include "Filter.h"

namespace adblockplus
{
	Filter::Filter()
	{
		// 注意这和 IUnknown 的 ref count 不一样, IUnkown 默认是 0, 以后每次引用计数加1
		// 这里只要对象创建出来就是1了, 这样在 new 的时候不用再手工执行一次 AddRef
		m_nRefCount = 1;
	}

	ULONG Filter::AddRef()
	{
		return InterlockedIncrement(&m_nRefCount) ;
	}

	ULONG Filter::Release()
	{
		LONG nRefCount = InterlockedDecrement(&m_nRefCount);

		if (nRefCount <= 0) delete this;

		return nRefCount;
	}

	RegExpFilter::RegExpFilter() : m_bThirdParty(false), m_bFirstParty(false)
	{
	}

	RegExpFilter * RegExpFilter::fromText( const std::string & regexp, const std::string & options )
{
		RegExpFilter * p = new RegExpFilter();

		p->m_RegExp = regexp;

		if ( ! options.empty() )
		{
			p->parseOptions(options);
		}
		
		return p;
	}

	inline std::string gethostname(const char * url)
	{
		std::string str;

		URL_COMPONENTSA uc;
		ZeroMemory( &uc, sizeof(uc) );
		uc.dwStructSize = sizeof(uc);

		CHAR szHostname[INTERNET_MAX_HOST_NAME_LENGTH];
		uc.lpszHostName = szHostname;
		uc.dwHostNameLength = INTERNET_MAX_HOST_NAME_LENGTH;
		if ( InternetCrackUrlA(url, 0, 0, & uc ) )
		{
			str = szHostname;
		}

		return str;
	}

	bool RegExpFilter::match(const HttpRequest * request)
	{
		if ( ! matchContentType(request->contentType) )
		{
			return false;
		}

		if ( ! matchThirdParty(request) )
		{
			return false;
		}

		if ( ! matchDomains(request) )
		{
			return false;
		}

		return regexpr_match( m_RegExp.c_str(), request->url, false );
	}

	bool RegExpFilter::matchContentType(long aContentType)
	{
		if ( m_DisabledContentTypes.find(aContentType) != m_DisabledContentTypes.end() )
		{
			return false;
		}

		if ( ! m_ActiveContentTypes.empty() )
		{
			if ( m_ActiveContentTypes.find(aContentType) == m_ActiveContentTypes.end() )
			{
				return false;
			}
		}

		return true;
	}

	bool RegExpFilter::matchThirdParty(const HttpRequest * request)
	{
		if ( m_bThirdParty || m_bFirstParty )
		{
			// 首先必须是子请求
			if ( ! request->subRequest ) return false;

			std::string requestHost = gethostname(request->url);
			std::string refererHost = gethostname(request->referer);
			
			// cop.my.xunlei.com$third-party: URL 和 Referer 必须不是同一个域
			if ( m_bThirdParty && ( requestHost == refererHost ) ) return false;
			// @@google.com$~third-party: URL 和 Referer 必须是同一个域
			if ( m_bFirstParty && ( requestHost != refererHost ) ) return false;
		}

		return true;
	}

	/**
	 * 比较两个域名是否相等.
	 * www.google.com 和 google.com 认为相等
	 * google.com.hacker.com 和 google.com 认为不相等
	 */
	inline bool hostcmp( const char * host1, const char * host2 )
	{
		const char * p1 = host1 + strlen(host1);
		const char * p2 = host2 + strlen(host2);

		while ( ( p1 >= host1 ) && ( p2 >= host2 ) )
		{
			if ( (*p1) != (*p2) ) return false;

			p1--;
			p2--;
		}

		return ( ( p1 >= host1 ) && ( '.' == *p1 ) ) || ( ( p2 >= host2 ) && ( '.' == *p2 ) ) || ( ( p1 < host1 ) && ( p2 < host2 ) );
	}

	bool RegExpFilter::matchDomains(const HttpRequest * request )
	{
		if ( (! m_ActiveDomains.empty()) || (! m_DisabledDomains.empty()) )
		{
			std::string hostname = gethostname( request->referer && request->referer[0] ? request->referer : request->url);
			if (!hostname.empty())
			{
				std::vector<std::string>::iterator iter;
				for ( iter = m_DisabledDomains.begin(); iter != m_DisabledDomains.end(); iter++ )
				{
					if ( hostcmp( hostname.c_str(), (* iter).c_str() ) )
					{
						// 如果与被排除的 domain 相符, 那么这条规则肯定不匹配了, 直接返回
						return false;
					}
				}

				if ( ! m_ActiveDomains.empty() )
				{
					bool b = false;
					for ( iter = m_ActiveDomains.begin(); iter != m_ActiveDomains.end(); iter++ )
					{
						if ( hostcmp( hostname.c_str(), (* iter).c_str() ) )
						{
							b = true;
							break;
						}
					}
					if ( ! b )
					{
						// 没找到, 说明与要求的 domain 不匹配, 也直接返回
						return false;
					}
				}
			}
		}

		return true;
	}

	void RegExpFilter::parseOptions( const std::string & options )
	{
		// 参考: /newhuagg/*$image,domain=onlinedown.net|newhua.com
		// 参数以“,”分隔
		std::vector<std::string> optionsList;
		tokenize_string(options, optionsList, ",");
		for ( auto iter = optionsList.begin(); iter != optionsList.end(); iter++ )
		{
			std::string str = *iter;
			str.erase(remove_if(str.begin(), str.end(), isspace), str.end());	// 去掉所有的空格

			const char * option = str.c_str();
			if ( ! option[0] ) break;

			bool versa = ( '~' == option[0] );
			if ( versa ) option++;

			if ( _parseContentType(option, versa) )
			{
				continue;
			}

			if ( _parseThirdParty(option, versa) )
			{
				continue;
			}

			if ( _parseDomain(option) )
			{
				continue;
			}
		}
	}

	bool RegExpFilter::_parseContentType( const char * option, bool versa )
{
		static const struct	{ const char * name;	const int value; } MAP [] = {
			{"image", nsIContentPolicy::TYPE_IMAGE},
			{"script", nsIContentPolicy::TYPE_SCRIPT},
			{"stylesheet", nsIContentPolicy::TYPE_STYLESHEET},
			{"object", nsIContentPolicy::TYPE_OBJECT},
			{"subdocument", nsIContentPolicy::TYPE_SUBDOCUMENT},
			{"document", nsIContentPolicy::TYPE_DOCUMENT}
		};

		for ( int i = 0; i < ARRAYSIZE(MAP); i++ )
		{
			if ( _stricmp(MAP[i].name, option) == 0 )
			{
				if ( ! versa )
				{
					m_ActiveContentTypes.insert(MAP[i].value);
				}
				else
				{
					m_DisabledContentTypes.insert(MAP[i].value);
				}

				return true;
			}
		}

		return false;
	}

	bool RegExpFilter::_parseThirdParty( const char * option, bool versa )
{
		if ( _stricmp("third-party", option) == 0 )
		{
			if ( ! versa )
			{
				m_bThirdParty = true;
				m_bFirstParty = false;
			}
			else
			{
				m_bThirdParty = false;
				m_bFirstParty = true;
			}

			return true;
		}
		else
		{
			return false;
		}
	}

	bool RegExpFilter::_parseDomain( const char * option )
	{
		// domain=value 的形式
		if ( _strnicmp("domain=", option, 7) == 0 )
		{
			const char * value = option + 7;
			if ( value )
			{
				std::vector<std::string> domains;
				tokenize_string( value, domains, "|");
				for ( auto domain_iter = domains.begin(); domain_iter != domains.end(); domain_iter++ )
				{
					// 参考: domain=example.com|~foo.example.com
					std::string domain = * domain_iter;
					if ( '~' == domain[0] )
					{
						m_DisabledDomains.push_back(domain.substr(1, domain.length()-1));
					}
					else
					{
						m_ActiveDomains.push_back(domain);
					}
				}

				return true;
			}
		}
			
		return false;
	}

	ElemHideSelector * ElemHideSelector::fromText(const std::string & domain, const std::string & tagName, const std::string & attrRules, const std::string & selector)
	{
		ElemHideSelector * p = new ElemHideSelector();

		p->m_selector = selector;

		return p;
	}
}