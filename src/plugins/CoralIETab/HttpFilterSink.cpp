/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* ***** BEGIN LICENSE BLOCK *****
* Version: NPL 1.1/GPL 2.0/LGPL 2.1
*
* The contents of this file are subject to the Netscape Public License
* Version 1.1 (the "License"); you may not use this file except in
* compliance with the License. You may obtain a copy of the License at
* http://www.mozilla.org/NPL/
*
* Software distributed under the License is distributed on an "AS IS" basis,
* WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
* for the specific language governing rights and limitations under the
* License.
*
* The Original Code is Coral IETab.
*
* The Initial Developer of the Original Code is quaful <quaful msn[dot]com>.
*
* Contributor(s): .
*
* Alternatively, the contents of this file may be used under the terms of
* either the GNU General Public License Version 2 or later (the "GPL"), or
* the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
* in which case the provisions of the GPL or the LGPL are applicable instead
* of those above. If you wish to allow use of your version of this file only
* under the terms of either the GPL or the LGPL, and not to allow others to
* use your version of this file under the terms of the NPL, indicate your
* decision by deleting the provisions above and replace them with the notice
* and other provisions required by the GPL or the LGPL. If you do not delete
* the provisions above, a recipient may use your version of this file under
* the terms of any one of the NPL, the GPL or the LGPL.
*
* ***** END LICENSE BLOCK ***** */

#include "StdAfx.h"

#include <set>

#include <atlutil.h>

#include <WinInet.h>
#include <WinCred.h>
#pragma comment(lib, "Credui.lib")

#include "../gecko-sdk/include/nsStringAPI.h"
#include "../gecko-sdk/internal/nsNetCID.h"
#include "../gecko-sdk/include/nsIContentPolicy.h"

#include "IEHostWindow.h"
#include "nsPluginInstance.h"
#include "nsConfigManager.h"
#include "CoralIETabNPObject.h"
#include "CriticalSection.h"
#include "Misc.h"

#include "adblockplus/adblockplus.h"

#include "HttpFilterSink.h"

/** 1x1 �Ŀհ�͸�� GIF �ļ�, ��������ͼƬ */
static const BYTE  TRANSPARENT_GIF_1x1 [] =
{
	0x47,0x49,0x46,0x38,0x39,0x61,0x01,0x00,/**/ 0x01,0x00,0x91,0x00,0x00,0x00,0x00,0x00,
	0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,/**/ 0x00,0x21,0xf9,0x04,0x05,0x14,0x00,0x02,
	0x00,0x2c,0x00,0x00,0x00,0x00,0x01,0x00,/**/ 0x01,0x00,0x00,0x02,0x02,0x54,0x01,0x00,
	0x3b
};
static const DWORD  TRANSPARENT_GIF_1x1_LENGTH = sizeof(TRANSPARENT_GIF_1x1);
/** �հ� HTML, ����������ҳ */
static const BYTE   BLANK_HTML []= "<HTML></HTML>";
static const DWORD  BLANK_HTML_LENGTH = sizeof(BLANK_HTML)-1;

VOID _HttpRawHeader2CrLfHeader(LPCSTR szRawHeader, CString & strCrLfHeader);
LPWSTR _ExtractFieldValue( LPCWSTR szHeader, LPCWSTR szFieldName, LPWSTR * pFieldValue, size_t * pSize );

//////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
// ActiveHandlers
//
// This class is here to workaround a crash in IE6SP2 when the browser process
// is exiting. In some circumstances, HttpHandlers are not terminated as they
// should be. Some time after our DLL is unloaded during shutdown, IE
// invokes methods on these orphaned handlers resulting in a crash. Note that
// WinCE does not suffer from this problem.
// See http://code.google.com/p/google-gears/issues/detail?id=182
//
// To avoid this problem, we maintain a collection of the active HttpHandlers
// when running in IE6 or earlier. When our DLL is unloaded, we explicitly
// Terminate() any orphaned handlers. This prevents the crash.
// TODO(michaeln): If and when we find and fix the source of this bug,
// remove this workaround code.
//------------------------------------------------------------------------------

#ifdef WINCE
class ActiveHandlers {
public:
	void Add(CHttpFilterAPP *handler) {}
	void Remove(CHttpFilterAPP *handler) {}
};
#else
class ActiveHandlers : public std::set<CHttpFilterAPP*> {
public:
	ActiveHandlers() {}

	virtual ~ActiveHandlers() {
		while (CHttpFilterAPP* handler = GetAndRemoveFirstHandler()) {
			handler->Terminate(0);
			handler->Release();
		}
	}

	void Add(CHttpFilterAPP *handler) {
		m_cs.Lock();
		insert(handler);
		m_cs.Unlock();
	}

	void Remove(CHttpFilterAPP *handler) {
		m_cs.Lock();
		erase(handler);
		m_cs.Unlock();
	}

private:
	CHttpFilterAPP *GetAndRemoveFirstHandler() {
		if (empty()) return NULL;
		
		m_cs.Lock();
		CHttpFilterAPP *handler = *begin();
		erase(handler);
		m_cs.Unlock();
		
		handler->AddRef();  // released by our caller
		return handler;
	}

private:

	CriticalSection m_cs;

};
#endif

static ActiveHandlers g_active_handlers;

//////////////////////////////////////////////////////////////////////////

CHttpFilterSink::CHttpFilterSink()
{
	pTargetBuffer = NULL;
	dwTargetBufSize = 0;

	m_pIEHostWindow = NULL;
	m_bIsSubRequest = true;
}

HRESULT WINAPI CHttpFilterSink::QueryIAuthenticate(void* pv, REFIID riid, LPVOID* ppv, DWORD dw)
{
	* ppv = NULL;

	if ( pv && InlineIsEqualGUID(riid, IID_IAuthenticate) )
	{
		CHttpFilterSink * pThis = (CHttpFilterSink *)pv;

		if ( nsConfigManager::isCookieSyncEnabled && pThis->m_pIEHostWindow && pThis->m_pIEHostWindow->SyncCookies && pThis->m_pIEHostWindow->m_pPluginInstance && ! pThis->m_strURL.IsEmpty() && pThis->m_spTargetProtocol )
		{
			do 
			{
				CComPtr<IWinInetHttpInfo> spWinInetHttpInfo;
				if ( FAILED(pThis->m_spTargetProtocol->QueryInterface(&spWinInetHttpInfo)) ) break;
				if ( ! spWinInetHttpInfo ) break;

				CHAR szRawHeader[8192];		// IWinInetHttpInfo::QueryInfo() ���ص� Raw Header ���� Unicode ��
				DWORD dwBuffSize = ARRAYSIZE(szRawHeader);

				if ( FAILED(spWinInetHttpInfo->QueryInfo(HTTP_QUERY_RAW_HEADERS, szRawHeader, &dwBuffSize, 0, NULL)) ) break;

				CString strHeader;
				_HttpRawHeader2CrLfHeader(szRawHeader, strHeader);

				static const WCHAR AUTH_HEAD [] = L"\r\nWWW-Authenticate:";

				LPWSTR lpAuth = NULL;
				size_t nAuthLen = 0;
				if ( ! _ExtractFieldValue( strHeader, AUTH_HEAD, & lpAuth, & nAuthLen ) ) break;
				if ( ! lpAuth ) break;

				nsCString aAuthScheme;
				nsCString aAuthRealm;

				// ���������¼��������
				// WWW-Authenticate: Basic realm="Secure Area"
				// WWW-Authenticate: Digest realm="testrealm@host.com", qop="auth,auth-int", nonce="dcd98b7102dd2f0e8b11d0f600bfb0c093", opaque="5ccc069c403ebaf9f0171e9517f40e41"
				// WWW-Authenticate: NTLM
				// WWW-Authenticate: NTLM <auth token>
				LPWSTR pPos = StrStrW( lpAuth, L" " );
				if ( pPos )
				{
					* pPos = L'\0';
					NS_UTF16ToCString(nsDependentString(lpAuth), NS_CSTRING_ENCODING_UTF8, aAuthScheme);

					do 
					{
						pPos = StrStrIW( pPos + 1, L"realm");
						if ( ! pPos ) break;
						pPos = StrChrW( pPos + 5, L'=');
						if ( ! pPos ) break;
						pPos = StrChrW( pPos + 1, L'"');
						if ( ! pPos ) break;
						LPWSTR lpRealm = pPos + 1;
						pPos = StrChrW( lpRealm, L'"');
						if ( ! pPos ) break;
						* pPos = L'\0';

						NS_UTF16ToCString(nsDependentString(lpRealm), NS_CSTRING_ENCODING_UTF8, aAuthRealm);

					} while (false);

				}
				else
				{
					NS_UTF16ToCString(nsDependentString(lpAuth), NS_CSTRING_ENCODING_UTF8, aAuthScheme);
				}
				
				VirtualFree( lpAuth, 0, MEM_RELEASE);

				// ���� NPN_GetAuthenticationInfo �ò��� NTLM �� domain��û�취����¼��ֻ�ò�֧����
				if ( _stricmp(aAuthScheme.get(), "NTLM") == 0 ) return E_NOINTERFACE;

				CUrl url;
				if ( url.CrackUrl(pThis->m_strURL) )
				{
					CW2A aScheme(url.GetSchemeName());
					CW2A aHost(url.GetHostName());
					int aPort = url.GetPortNumber();

					char* username = NULL;
					char* password = NULL;
					uint32_t ulen = 0, plen = 0;

					if ( NPN_GetAuthenticationInfo(pThis->m_pIEHostWindow->m_pPluginInstance->instance(), aScheme, aHost, aPort, aAuthScheme.get(), aAuthRealm.get(), &username, &ulen, &password, &plen ) != NPERR_NO_ERROR ) break;

					pThis->m_strUsername = username;
					pThis->m_strPassword = password;

					NPN_MemFree(username);
					NPN_MemFree(password);
				}

				* ppv = dynamic_cast<IAuthenticate *>(pThis);

				((IUnknown*)*ppv)->AddRef();

				return S_OK;

			} while (false);
		}
	}

	return E_NOINTERFACE;
}

STDMETHODIMP CHttpFilterSink::BeginningTransaction(
	/* [in] */ LPCWSTR szURL,
	/* [in] */ LPCWSTR szHeaders,
	/* [in] */ DWORD dwReserved,
	/* [out] */ LPWSTR *pszAdditionalHeaders)
{
	if (pszAdditionalHeaders)
	{
		*pszAdditionalHeaders = 0;
	}

	// �ȵ���Ĭ�ϵ� IHttpNegotiate ����ӿ�, ��Ϊ����֮�� pszAdditionalHeaders �Ż��� Referer ����Ϣ,
	// ��������� _SetUserAgent() �������д��������
	CComPtr<IHttpNegotiate> spHttpNegotiate;
	QueryServiceFromClient(&spHttpNegotiate);
	HRESULT hr = spHttpNegotiate ?
		spHttpNegotiate->BeginningTransaction(szURL, szHeaders,
		dwReserved, pszAdditionalHeaders) :	S_OK;

	// BeginningTransaction() �Ǳ������ʼ�����õķ���, ��������µ��õ� URL
	m_strURL = szURL;

	// ��ѯ��������Ӧ�� CIEHostWindow ����, ������ʱ���õ�
	_QueryIEHostWindow();

	// ������� User-Agent��Referer ���ӵ� pszAdditionalHeaders ��
	_SetCustomHeaders(pszAdditionalHeaders);

	return hr;
}

STDMETHODIMP CHttpFilterSink::OnResponse(
								   /* [in] */ DWORD dwResponseCode,
								   /* [in] */ LPCWSTR szResponseHeaders,
								   /* [in] */ LPCWSTR szRequestHeaders,
								   /* [out] */ LPWSTR *pszAdditionalRequestHeaders)
{
	if (pszAdditionalRequestHeaders)
	{
		* pszAdditionalRequestHeaders = NULL;
	}

	CComPtr<IHttpNegotiate> spHttpNegotiate;
	QueryServiceFromClient(&spHttpNegotiate);
	HRESULT hr = spHttpNegotiate ?
		spHttpNegotiate->OnResponse(dwResponseCode, szResponseHeaders,
		szRequestHeaders, pszAdditionalRequestHeaders) : S_OK;

	if ( ( dwResponseCode >= 200 ) && ( dwResponseCode < 300 ) )
	{
		bool bImportCookies = nsConfigManager::isCookieSyncEnabled;

		if ( nsConfigManager::isAdblockEnabled )
		{
			static const WCHAR CONTENT_TYPE_HEAD [] = L"Content-Type:";
			LPWSTR pContentType = NULL;
			size_t nLen = 0;
			if ( _ExtractFieldValue(szResponseHeaders, CONTENT_TYPE_HEAD, & pContentType, & nLen) )
			{
				PRUint32 aContentType = _ScanContentType(pContentType);

				if ( pContentType ) VirtualFree( pContentType, 0, MEM_RELEASE);

				if ( ( nsIContentPolicy::TYPE_DOCUMENT == aContentType ) && m_bIsSubRequest ) aContentType = nsIContentPolicy::TYPE_SUBDOCUMENT;

				if ( !_CanLoadContent(aContentType) )
				{
					// �������˾Ͳ��õ��� Cookie ��
					bImportCookies = false;

					switch ( aContentType )
					{
					case nsIContentPolicy::TYPE_IMAGE:
						{
							pTargetBuffer = TRANSPARENT_GIF_1x1;
							dwTargetBufSize = TRANSPARENT_GIF_1x1_LENGTH;

							break;
						}
					case nsIContentPolicy::TYPE_DOCUMENT:
					case nsIContentPolicy::TYPE_SUBDOCUMENT:
						{
							pTargetBuffer = BLANK_HTML;
							dwTargetBufSize = BLANK_HTML_LENGTH;

							break;
						}
					default:
						{
							// �����������͵��ļ�, ֱ����ֹ����
							hr = E_ABORT;
						}
					}

					if ( m_spInternetProtocolSink ) m_spInternetProtocolSink->ReportData(BSCF_FIRSTDATANOTIFICATION | BSCF_LASTDATANOTIFICATION |BSCF_DATAFULLYAVAILABLE, 0, 0);
					if ( m_spInternetProtocolSink ) m_spInternetProtocolSink->ReportResult(S_OK,S_OK,NULL);
				}
			}
		}

		// �����ﵼ�� Cookies, ���ܻ��а�ȫ������, ��һЩ������ Cookie Policy �� Cookie Ҳ�Ź�ȥ
		// ReportProgress() ���濴�ĵ��и� BINDSTATUS_SESSION_COOKIES_ALLOWED, �о�Ҫ����ȫһЩ, ����ʵ������ʱһֱû�е������״̬
		// Ҳ�� Firefox �Լ��ᴦ��
		if ( bImportCookies ) _ExportCookies( szResponseHeaders );
	}

	return hr;
}

STDMETHODIMP CHttpFilterSink::Authenticate( 
	/* [out] */ HWND *phwnd,
	/* [out] */ LPWSTR *pszUsername,
	/* [out] */ LPWSTR *pszPassword)
{
	if ( (! m_strUsername.IsEmpty()) && (! m_strPassword.IsEmpty()) )
	{
		size_t len = m_strUsername.GetLength()+1;
		* pszUsername = (LPWSTR)CoTaskMemAlloc(len*sizeof(WCHAR));
		wcscpy_s( * pszUsername, len, m_strUsername);
		len = m_strPassword.GetLength()+1;
		* pszPassword = (LPWSTR)CoTaskMemAlloc(len*sizeof(WCHAR));
		wcscpy_s( * pszPassword, len, m_strPassword);
	}

	return S_OK;
}

STDMETHODIMP CHttpFilterSink::ReportProgress(
									   /* [in] */ ULONG ulStatusCode,
									   /* [in] */ LPCWSTR szStatusText)
{
	ATLASSERT(m_spInternetProtocolSink != 0);

	HRESULT hr = m_spInternetProtocolSink ?	m_spInternetProtocolSink->ReportProgress(ulStatusCode, szStatusText) : S_OK;

	switch ( ulStatusCode )
	{
	case BINDSTATUS_REDIRECTING:
		{
			// �ض�����, ���¼�¼�� URL
			if ( m_pIEHostWindow )
			{
				if ( ! m_bIsSubRequest )
				{
					m_pIEHostWindow->m_strLoadingUrl = szStatusText;
					m_strURL = szStatusText;
				}

				// �ܶ���վ��¼��ʱ�����302��תʱ����Cookie, ����Gmail, ��������������ҲҪ���� Cookie
				if ( nsConfigManager::isCookieSyncEnabled && m_pIEHostWindow->SyncCookies )
				{
					CComPtr<IWinInetHttpInfo> spWinInetHttpInfo;
					if ( SUCCEEDED(m_spTargetProtocol->QueryInterface(&spWinInetHttpInfo)) && spWinInetHttpInfo )
					{
						CHAR szRawHeader[8192];		// IWinInetHttpInfo::QueryInfo() ���ص� Raw Header ���� Unicode ��
						DWORD dwBuffSize = ARRAYSIZE(szRawHeader);

						if ( SUCCEEDED(spWinInetHttpInfo->QueryInfo(HTTP_QUERY_RAW_HEADERS, szRawHeader, &dwBuffSize, 0, NULL)) )
						{
							// ע�� HTTP_QUERY_RAW_HEADERS ���ص� Raw Header �� \0 �ָ���, �� \0\0 ��Ϊ����, ��������Ҫ��ת��
							CString strHeader;
							_HttpRawHeader2CrLfHeader(szRawHeader, strHeader);

							_ExportCookies(strHeader);
						}
					}

				}
			}

			break;
		}
	/*
	case BINDSTATUS_MIMETYPEAVAILABLE:
		{
			break;
		}
	*/
	}

	return hr;
}

VOID CHttpFilterSink::_QueryIEHostWindow()
{
	// ��ѯ�����������ĸ� IE ����
	CComPtr<IWindowForBindingUI> spWindowForBindingUI;
	if ( SUCCEEDED(QueryServiceFromClient(&spWindowForBindingUI)) && spWindowForBindingUI )
	{
		HWND hwndIEServer = NULL;
		if ( SUCCEEDED(spWindowForBindingUI->GetWindow(IID_IHttpSecurity, &hwndIEServer)) && IsWindow(hwndIEServer))
		{
			// ����õ��� hwndIEServer ����ܸ���, �� Internet Explorer_Server ���ڻ�û�����ü�������ʱ��(�շ�����������ʱ��),
			// hwndIEServer �� Shell Embedding ���ڵľ��; ֮���������� Internet Explorer_Server ���ڵľ��, ��ʱ��Ҳ����
			// Shell DocObject View ���ڵľ��

			HWND hwndAtlHost = ::GetParent(hwndIEServer);

			// ������������, ����ʹ� hwndIEServer һֱ������, ֱ���ҵ��� CIEHostWindow �� ATL Host ����Ϊֹ. Ϊ�˰�ȫ���, ���
			// ������ 5 ��
			TCHAR szClassName[MAX_PATH];
			for ( int i = 0; i < 5; i++ )
			{
				if ( GetClassName(hwndAtlHost, szClassName, ARRAYSIZE(szClassName)) == 0 ) break;

				if ( _tcsncmp(szClassName, _T("ATL:"), 4) == 0 )
				{
					// �ҵ���
					m_pIEHostWindow = CIEHostWindow::FromHwnd(hwndAtlHost);
					break;
				}
				else
				{
					hwndAtlHost = ::GetParent(hwndAtlHost);
				}
			}
		}
	}

	// ���� URL ��ʶ���Ƿ���ҳ���ڵ�������
	m_bIsSubRequest = ! ( m_pIEHostWindow && FuzzyUrlCompare(m_pIEHostWindow->m_strLoadingUrl, m_strURL) );
}

VOID CHttpFilterSink::_SetCustomHeaders(LPWSTR *pszAdditionalHeaders)
{
	if ( pszAdditionalHeaders )
	{
		static const WCHAR REFERER [] = L"Referer:";

		CStringW strHeaders(*pszAdditionalHeaders);

		if ( m_pIEHostWindow )
		{
			if ( nsConfigManager::isCookieSyncEnabled )
			{
				if ( m_pIEHostWindow->SyncCookies && m_strURL[0] )
				{
					_ImportCookies();
				}

				if ( m_pIEHostWindow->SyncUserAgent )
				{
					// ���� User-Agent
					CString strUserAgent;
					strUserAgent.Format( _T("User-Agent: %s\r\n"), m_pIEHostWindow->nsUserAgent() );

					strHeaders += strUserAgent;
				}
			}

			// ����� m_strUrlContext, ˵�������´���, ��Ҫ�� IE ���� Referer
			if ( ! m_pIEHostWindow->m_strUrlContext.IsEmpty() )
			{
				if ( StrStrIW( *pszAdditionalHeaders, REFERER ) )
				{
					// �Ѿ��� Referer ��, �ǾͲ�����
					if ( ! m_bIsSubRequest )
					{
						m_pIEHostWindow->m_strUrlContext.Empty();
					}
				}
				else
				{
					CString strReferer;
					strReferer.Format( _T("%s %s\r\n"), REFERER, m_pIEHostWindow->m_strUrlContext );

					strHeaders += strReferer;
				}
			}

			size_t nLen = strHeaders.GetLength() + 2;
			if ( *pszAdditionalHeaders = (LPWSTR)CoTaskMemRealloc(*pszAdditionalHeaders, nLen*sizeof(WCHAR)) )
			{
				wcscpy_s( *pszAdditionalHeaders, nLen, strHeaders);
			}
		}

		LPWSTR lpReferer = NULL;
		size_t nRefererLen = 0;
		if ( _ExtractFieldValue(*pszAdditionalHeaders, REFERER, & lpReferer, & nRefererLen ) )
		{
			m_strReferer = lpReferer;

			VirtualFree( lpReferer, 0, MEM_RELEASE);
		}
	}
}

inline char * SplitCookies(char * cookies, std::string & cookie_name, std::string & cookie_value)
{
	char * p = cookies;
	// IE ���Լ����˵��ո���������Ĵ��벻��Ҫ��
	// while ( cookies && (*cookies == ' ') ) cookies++;			// �˵��ո�
	while ( p && (*p != 0) && (*p != '=') ) p++;
	if ( '=' == *p )
	{
		*p = 0;
		cookie_name = cookies;
		cookies = ++p;

		while ( (*p != 0) && (*p != ';') ) p++;
		if ( ';' == *p )
		{
			*p = 0;
		}
		cookie_value = cookies;

		return ++p;
	}

	return NULL;
}

VOID CHttpFilterSink::_ImportCookies()
{
	char * cookies = NULL;
	uint32_t len = 0;
	CT2A url(m_strURL);
	NPN_GetValueForURL(m_pIEHostWindow->m_pPluginInstance->instance(), NPNURLVCookie, url, &cookies, &len);
	if ( cookies )
	{
		char * p = cookies;

		// NPN_GetValueForURL() ���ص� cookie �ĸ�ʽ�� cookie1=value1;cookie2=value2;cookie3=value3
		// ������Ҫһ��һ���⿪
		std::string cookie_name;
		std::string cookie_value;
		while ( p = SplitCookies(p, cookie_name, cookie_value) )
		{
			InternetSetCookieA(url, cookie_name.c_str(), cookie_value.c_str());
		}

		NPN_MemFree(cookies);
	}

}

VOID CHttpFilterSink::_ExportCookies(LPCWSTR szResponseHeaders)
{
	static const WCHAR SET_COOKIE_HEAD [] = L"\r\nSet-Cookie:";

	do 
	{
		if ( ! m_pIEHostWindow ) break;
		if ( ! m_pIEHostWindow->SyncCookies ) break;
		if ( ! m_pIEHostWindow->m_pPluginInstance ) break;

		nsresult rv = NS_OK;

		nsCString url_utf8;
		rv = NS_UTF16ToCString(nsDependentString(m_strURL), NS_CSTRING_ENCODING_UTF8, url_utf8);
		if ( rv != NS_OK ) break;

		LPWSTR p = (LPWSTR)szResponseHeaders;
		LPWSTR lpCookies = NULL;
		size_t nCookieLen = 0;
		while ( p = _ExtractFieldValue( p, SET_COOKIE_HEAD, & lpCookies, & nCookieLen ) )
		{
			if ( lpCookies )
			{
				nsCString cookie_utf8;
				rv = NS_UTF16ToCString(nsDependentString(lpCookies), NS_CSTRING_ENCODING_UTF8, cookie_utf8);
				if ( rv != NS_OK ) continue;

				NPN_SetValueForURL( m_pIEHostWindow->m_pPluginInstance->instance(), NPNURLVCookie, url_utf8.get(), cookie_utf8.get(), cookie_utf8.Length());

				VirtualFree( lpCookies, 0, MEM_RELEASE);
				lpCookies = NULL;
				nCookieLen = 0;
			}
		}

	} while(false);
}

BOOL CHttpFilterSink::_CanLoadContent( PRUint32 aContentType )
{
	CT2A url(m_strURL);
	CT2A referer(m_strReferer);

	adblockplus::HttpRequest request;
	request.url = url.m_psz;
	request.referer = referer.m_psz;
	request.contentType = aContentType;
	request.subRequest = m_bIsSubRequest;
	if ( ! request.url ) return TRUE;
	bool block = adblockplus::test(&request);

	CIEHostWindow * pIEHostWindow = m_pIEHostWindow ? m_pIEHostWindow : CIEHostWindow::FromUrl(NULL);

	// �����Ƿ����, ��֪ͨ adblockplus, ���������ڡ��ɹ�����Ŀ��������ʾ����
	if ( pIEHostWindow )
	{
		static const DWORD ADBLOCK_NOTIFY_SIG = 0xADB00000;		// ������ʶ����Ҫ���� WM_ADBLOCK_NOTIFY ��Ϣ�ǹ��˹�����Ϣ

		// ����Ϊ�˼����, ֱ�Ӱѱ�ʶ���Ƿ���˺� Content Type ƴװ������
		// aContentType ������ PRUint32 ���͵�, ���� nsIContentPolicy �� Content Type ���ֻ�� 13, ������һ�� WORD �͹���
		WPARAM dwData = ADBLOCK_NOTIFY_SIG | ( block ? 1 : 0 ) << 16 | ( request.contentType & 0xFFFF );
		size_t len = strlen(request.url)+1;
		LPARAM lpData = (LPARAM)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, len + 1);
		if ( lpData ) strcpy_s((char*)lpData, len, request.url);

		// ����֪ͨ����ֻ�������߳��н���, ������������Ϣ���ư�֪ͨת�����߳���
		::PostMessage( pIEHostWindow->m_hWnd, WM_ADBLOCK_NOTIFY, dwData, lpData);
	}

	return ! block;
}

//////////////////////////////////////////////////////////////////////////

CHttpFilterAPP::~CHttpFilterAPP()
{
	g_active_handlers.Remove(this);   // okay to Remove() multiple times from set
}

HRESULT CHttpFilterAPP::FinalConstruct() {
	
	m_Sink = HttpFilterStartPolicy::GetSink(this);
	
	m_nDataWritten = 0;

	return S_OK;
}

STDMETHODIMP CHttpFilterAPP::Start(
		/* [in] */ LPCWSTR url,
		/* [in] */ IInternetProtocolSink *protocol_sink,
		/* [in] */ IInternetBindInfo *bind_info,
		/* [in] */ DWORD flags,
		/* [in] */ HANDLE_PTR reserved)
{
	g_active_handlers.Add(this);

	return BaseClass::Start(url, protocol_sink, bind_info, flags, reserved);
}

STDMETHODIMP CHttpFilterAPP::Read(
							   /* [in, out] */ void *pv,
							   /* [in] */ ULONG cb,
							   /* [out] */ ULONG *pcbRead)
{
	const BYTE * pBuffer = m_Sink->pTargetBuffer;
	DWORD dwBufLen = m_Sink->dwTargetBufSize;
	if ( pBuffer && ( dwBufLen > 0 ) )
	{
		DWORD dwSize = 0;
		if ( dwBufLen > m_nDataWritten )
		{
			dwSize = min(dwBufLen - m_nDataWritten, cb);
		}

		* pcbRead = dwSize;

		if ( dwSize > 0 )
		{
			memcpy( pv, pBuffer + m_nDataWritten, dwSize );

			m_nDataWritten += dwSize;

			if ( m_nDataWritten == dwBufLen ) return S_FALSE;
		}
		else
		{
			return S_FALSE;
		}

		return S_OK;
	}
	else
	{
		return BaseClass::Read(pv, cb, pcbRead);
	}
}

STDMETHODIMP CHttpFilterAPP::Terminate(/* [in] */ DWORD options)
{
	g_active_handlers.Remove(this);

	return BaseClass::Terminate(options);
}

//////////////////////////////////////////////////////////////////////////

// ���� \0 �ָ��� Raw HTTP Header ����ת������ \r\n �ָ��� Header
VOID _HttpRawHeader2CrLfHeader(LPCSTR szRawHeader, CString & strCrLfHeader)
{
	strCrLfHeader.Empty();

	LPCSTR p = szRawHeader;
	while ( p[0] )
	{
		CString strHeaderLine(p);

		p += strHeaderLine.GetLength() + 1;

		strCrLfHeader += strHeaderLine + _T("\r\n");
	}
}

LPWSTR _ExtractFieldValue( LPCWSTR szHeader, LPCWSTR szFieldName, LPWSTR * pFieldValue, size_t * pSize )
{
	LPWSTR r = NULL;

	do 
	{
		// ���� RFC2616 �涨, HTTP field name �����ִ�Сд
		LPWSTR pStart = StrStrIW( szHeader, szFieldName );
		if ( ! pStart ) break;
		pStart += wcslen(szFieldName);
		while ( L' ' == pStart[0] ) pStart++;		// ������ͷ�Ŀո�
		LPWSTR pEnd = StrStrW( pStart, L"\r\n" );
		if ( ( ! pEnd ) || ( pEnd <= pStart ) ) break;

		size_t nSize = pEnd - pStart;
		size_t nBufLen = nSize + 2;		// �����ַ����� 0 ������
		LPWSTR lpBuffer = (LPWSTR)VirtualAlloc( NULL, nBufLen * sizeof(WCHAR), MEM_COMMIT, PAGE_READWRITE );
		if ( !lpBuffer ) break;

		if ( wcsncpy_s( lpBuffer, nBufLen, pStart, nSize) )
		{
			VirtualFree( lpBuffer, 0, MEM_RELEASE);
			break;
		}

		* pSize = nBufLen;
		* pFieldValue = lpBuffer;
		r = pEnd;

	} while(false);

	return r;
}

PRUint32 CHttpFilterSink::_ScanContentType(LPCWSTR szContentType)
{
	static const struct	{ const wchar_t * name;	const int value; } MAP [] = {
		{L"image/", nsIContentPolicy::TYPE_IMAGE},
		{L"text/css", nsIContentPolicy::TYPE_STYLESHEET},
		{L"text/javascript", nsIContentPolicy::TYPE_SCRIPT},
		{L"text/", nsIContentPolicy::TYPE_DOCUMENT},
		{L"application/x-javascript", nsIContentPolicy::TYPE_SCRIPT},
		{L"application/javascript", nsIContentPolicy::TYPE_SCRIPT},
		{L"application/", nsIContentPolicy::TYPE_OBJECT},
	};

	for ( int i = 0; i < ARRAYSIZE(MAP); i++ )
	{
		if ( _wcsnicmp(MAP[i].name, szContentType, wcslen(MAP[i].name)) == 0 )
		{
			return MAP[i].value;
		}
	}

	return nsIContentPolicy::TYPE_OTHER;
}