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

#include <WinInet.h>
#include <ShObjIdl.h>

#include "nsPluginInstance.h"
#include "CoralIETabNPObject.h"
#include "CriticalSection.h"

#include "../3rdParty/passthru_app/ProtocolCF.h"
#include "HttpFilterSink.h"
#include "nsConfigManager.h"
#include "Misc.h"

#include "adblockplus/adblockplus.h"

#include "IEHostWindow.h"

#pragma comment(lib, "Oleacc.lib")

typedef PassthroughAPP::CMetaFactory<PassthroughAPP::CComClassFactoryProtocol, CHttpFilterAPP> MetaFactory;

/** ATL Host 窗口到 CIEWindow 对象的映射, 便于通过 HWND 快速找到对应的 CIEWindow 对象 */
CSimpleMap< HWND, CIEHostWindow * > IEWindowMap;
/** 这个 CriticalSection 对象是与 IEWindowMap 配对使用的, 保证线程安全 */
CriticalSection cs;

/** WH_GETMESSAGE, 用于拦截键盘消息 */
static HHOOK hhookGetMsg = NULL;
/** WH_CALLWNDPROCRET, 用于拦截 WM_KILLFOCUS 消息 */
static HHOOK hhookCallWndProcRet = NULL;

CString CIEHostWindow::m_nsUserAgent;

CIEHostWindow::CIEHostWindow()
{
	CanGoBack = false;
	CanGoForward = false;
	AutoSwitchBack = false;
	SyncCookies = false;
	SyncUserAgent = false;
	m_bSwitchBack = false;

	progress = -1;
	closing = false;

	m_pUIController = NULL;

	m_pPluginInstance = NULL;
}

CIEHostWindow::~CIEHostWindow()
{
	m_hWnd = NULL;
}

CComPtr<IClassFactory> m_spCFHTTP;
CComPtr<IClassFactory> m_spCFHTTPS;

inline VOID ProcessInit()
{
	static bool b = true;

	if ( b )
	{
		if ( ! nsConfigManager::isClassicMode )
		{
			CComPtr<IInternetSession> spSession;
			if ( SUCCEEDED(CoInternetGetSession(0, &spSession, 0)) && spSession )
			{
				MetaFactory::CreateInstance(CLSID_HttpProtocol, &m_spCFHTTP);
				spSession->RegisterNameSpace(m_spCFHTTP, CLSID_NULL, L"http", 0, 0, 0);

				MetaFactory::CreateInstance(CLSID_HttpSProtocol, &m_spCFHTTPS);
				spSession->RegisterNameSpace(m_spCFHTTPS, CLSID_NULL, L"https", 0, 0, 0);
			}
		}

		// 设置启用 IE 的一些特性
		HMODULE hUrlMonDll = LoadLibrary( _T("urlmon.dll") );
		if( hUrlMonDll )
		{
			typedef HRESULT (WINAPI *PCoInternetSetFeatureEnabled)(INTERNETFEATURELIST, DWORD, BOOL);

			PCoInternetSetFeatureEnabled CoInternetSetFeatureEnabled = (PCoInternetSetFeatureEnabled)GetProcAddress( hUrlMonDll, "CoInternetSetFeatureEnabled" );
			if( CoInternetSetFeatureEnabled )
			{
				CoInternetSetFeatureEnabled( FEATURE_WEBOC_POPUPMANAGEMENT, SET_FEATURE_ON_PROCESS, TRUE );
				CoInternetSetFeatureEnabled( FEATURE_SECURITYBAND, SET_FEATURE_ON_PROCESS, TRUE );
				CoInternetSetFeatureEnabled( FEATURE_LOCALMACHINE_LOCKDOWN, SET_FEATURE_ON_PROCESS, TRUE );
				CoInternetSetFeatureEnabled( FEATURE_SAFE_BINDTOOBJECT, SET_FEATURE_ON_PROCESS, TRUE );
				#define FEATURE_TABBED_BROWSING 19
				CoInternetSetFeatureEnabled( (INTERNETFEATURELIST)FEATURE_TABBED_BROWSING, SET_FEATURE_ON_PROCESS, TRUE );		// IE7+
			}

			FreeLibrary( hUrlMonDll );
		}

		b = false;
	}
}

VOID UnloadAPP()
{
	if ( ! nsConfigManager::isClassicMode )
	{
		CComPtr<IInternetSession> spSession;
		CoInternetGetSession(0, &spSession, 0);
		spSession->UnregisterNameSpace(m_spCFHTTP, L"http");
		m_spCFHTTP.Release();
		spSession->UnregisterNameSpace(m_spCFHTTPS, L"https");
		m_spCFHTTPS.Release();
	}

	if ( hhookGetMsg )
	{
		UnhookWindowsHookEx(hhookGetMsg);
		hhookGetMsg = NULL;
	}
	
	if ( hhookCallWndProcRet )
	{
		UnhookWindowsHookEx(hhookCallWndProcRet);
		hhookCallWndProcRet = NULL;
	}
}

CIEHostWindow * CIEHostWindow::Create( HWND hParent, _U_RECT rc )
{
	CIEHostWindow * pIEHostWindow = new CIEHostWindow();
	if ( pIEHostWindow )
	{
		if (FAILED(pIEHostWindow->CreateWebBrowser(hParent, rc)))
		{
			pIEHostWindow = NULL;
		}
	}

	return pIEHostWindow;
}

LRESULT CIEHostWindow::OnDestroy(UINT uMsg, WPARAM wParam , LPARAM lParam, BOOL& bHandled)
{
	Disconnect();

	return 0;
}

void CIEHostWindow::OnFinalMessage(HWND /*hWnd*/)
{
	if ( m_hWnd ) delete this;
}

LRESULT CIEHostWindow::OnParentNotify(UINT uMsg, WPARAM wParam , LPARAM lParam, BOOL& bHandled)
{
	if ( WM_DESTROY == LOWORD(wParam) )
	{
		HWND hwnd = (HWND)lParam;
		if ( ::IsWindow(hwnd) )		// Shell Embedding
		{
			hwnd = ::GetParent(hwnd);
			if ( hwnd == m_hWnd )
			{
				if ( m_pPluginInstance )
				{
					CoralIETabNPObject * npobj = m_pPluginInstance->getScriptObject();
					if ( npobj )
					{
						closing = true;

						npobj->CloseTab();
					}
				}
			}
		}
		
	}

	return 0;
}

LRESULT CIEHostWindow::OnAdblockNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if ( m_pPluginInstance )
	{
		static const DWORD ADBLOCK_NOTIFY_SIG = 0xADB00000;

		if ( ADBLOCK_NOTIFY_SIG == (wParam & ADBLOCK_NOTIFY_SIG) )
		{
			PRInt32 aContentType = LOWORD(wParam);
			PRInt32 aBlock = HIWORD(wParam) & 0x0F;
			const char * url = (const char *)lParam;
			if ( url )
			{
				CoralIETabNPObject * npobj = m_pPluginInstance->getScriptObject();
				if ( npobj )
				{
					npobj->notifyAdblock(aContentType, url, aBlock);
				}

				HeapFree(GetProcessHeap(), 0, (LPVOID)url);
			}
		}
	}

	return 0;
}

LRESULT CIEHostWindow::OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	WORD wFrom = HIWORD(wParam);
	WORD wCmdId = LOWORD(wParam);
	switch ( wFrom )
	{
	case CMD_SIG_EXECWB:
		{
			(*this)->ExecWB( (OLECMDID)wCmdId, OLECMDEXECOPT_DODEFAULT, NULL, NULL );
			break;
		}
	case CMD_SIG_CMDTARGET:
		{
			CComPtr< IDispatch > spDoc;
			if ( SUCCEEDED( (*this)->get_Document( & spDoc ) ) && spDoc )
			{
				CComQIPtr< IOleCommandTarget, & IID_IOleCommandTarget > spCmd(spDoc);
				if ( spCmd )
				{
					static const IID CGID_IWebBrowser = { 0xed016940, 0xbd5b, 0x11cf, 0xba, 0x4e, 0x0, 0xc0, 0x4f, 0xd7, 0x08, 0x16 };

					spCmd->Exec( &CGID_IWebBrowser, wCmdId, 0, NULL, NULL );
				}
			}

			break;
		}
	default:
		break;
	}

	return 0;
}

STDMETHODIMP_(void) CIEHostWindow::OnProgressChange(long nProgress,long nProgressMax)
{
	if ( m_pPluginInstance )
	{
		CoralIETabNPObject * npobj = m_pPluginInstance->getScriptObject();
		if ( npobj )
		{
			if (nProgress == -1) nProgress = nProgressMax;
			if (nProgressMax) nProgress = (100*nProgress)/nProgressMax; else nProgress = -1;

			progress = nProgress;
			npobj->OnProgressChange(nProgress);
		}
	}
}

inline BOOL UrlCanHandle(LPCTSTR szUrl)
{
	// 可能性1: 类似 C:\Documents and Settings\<username>\My Documents\Filename.mht 这样的文件名
	TCHAR c = _totupper(szUrl[0]);
	if ( (c >= _T('A')) && (c <= _T('Z')) && (szUrl[1]==_T(':')) && (szUrl[2]==_T('\\')) )
	{
		return TRUE;
	}

	// 可能性2: \\fileserver\folder 这样的 UNC 路径
	if ( PathIsUNC(szUrl) )
	{
		return TRUE;
	}

	// 可能性3: http://... https://... file://...
	URL_COMPONENTS uc;
	ZeroMemory( &uc, sizeof(uc) );
	uc.dwStructSize = sizeof(uc);
	if ( ! InternetCrackUrl( szUrl, 0, 0, &uc ) ) return FALSE;

	switch ( uc.nScheme )
	{
	case INTERNET_SCHEME_HTTP:
	case INTERNET_SCHEME_HTTPS:
	case INTERNET_SCHEME_FILE:
		return TRUE;
	default:
		{
			// 可能性4: about:blank
			return ( _tcsncmp(szUrl, _T("about:"), 6) == 0 );
		}
	}
}

STDMETHODIMP_(void) CIEHostWindow::OnBeforeNavigate2(IDispatch *pDisp, VARIANT *url, VARIANT *Flags, VARIANT *TargetFrameName, VARIANT *PostData, VARIANT *Headers, VARIANT_BOOL *Cancel )
{
	if ( ! url ) return;

	COLE2T szUrl(url->bstrVal);

	// 滤掉非 HTTP 协议
	if ( ! UrlCanHandle(szUrl) ) return;

	// 如果是在一个 frame 里面, 忽略
	CComQIPtr<IWebBrowser2> spBrowser(pDisp);
	if ( spBrowser )
	{
		VARIANT_BOOL vbIsTopLevelContainer;
		if ( SUCCEEDED( spBrowser->get_RegisterAsBrowser( & vbIsTopLevelContainer ) ) )
		{
			if ( VARIANT_FALSE == vbIsTopLevelContainer ) return;
		}
	}

	if ( m_bSwitchBack )
	{
		if ( ! m_pPluginInstance ) return;

		static const char * TARGET_SELF = "_self";

		CW2A loadUrl(url->bstrVal);

		// 可能有两种情况, 一种是 POST, 一种是 GET
		if ( (PostData != NULL) && ( PostData->vt & (VT_VARIANT|VT_BYREF) ) && ( PostData->pvarVal->vt != VT_EMPTY ) )
		{
			// 如果 PostData 不为空, 我们就按照 POST 来处理

			do 
			{
				nsresult rv = NS_OK;

				
				SAFEARRAY * p = PostData->pvarVal->parray;

				long plLbound, plUbound;
				HRESULT hr = SafeArrayGetLBound(p, 1, & plLbound);
				if ( FAILED(hr) ) break;
				hr = SafeArrayGetUBound(p, 1, & plUbound);
				if ( FAILED(hr) ) break;

				long nPostLen = plUbound - plLbound + 1;

				char * szPostData = NULL;
				hr = SafeArrayAccessData(p, (LPVOID HUGEP *) &szPostData);
				if ( FAILED(hr) || ! szPostData ) break;

				if ( szPostData[nPostLen-1] == '\0' ) nPostLen--;			// 传递过来的 POST 数据会以 0 结尾, 但实际我们 POST 的时候不能带上这个 0

				PRInt32 nHeaderLen = Headers ? SysStringLen(Headers->bstrVal) : 0;
				CW2A szHeader(Headers ? Headers->bstrVal : NULL);
				
				NPN_PostURL(m_pPluginInstance->instance(), loadUrl, TARGET_SELF, nPostLen, szPostData, PR_FALSE);
				
				SafeArrayUnaccessData(p);

			} while ( false );
		}
		else
		{
			// 否则按照 GET 来处理
			NPN_GetURL( m_pPluginInstance->instance(), loadUrl, TARGET_SELF );
		}

		* Cancel = VARIANT_TRUE;
	}
	else
	{
		m_strLoadingUrl = szUrl;

		// adblockplus 支持
		if ( ( ! nsConfigManager::isClassicMode ) && nsConfigManager::isAdblockEnabled && m_pPluginInstance )
		{
			nsConfigManager * configManager = m_pPluginInstance->getConfigManager();
			if ( configManager )
			{
				if ( configManager->getBoolPref("extensions.adblockplus.enabled") == PR_TRUE )
				{
					adblockplus::enable(true);

					adblockplus::init(configManager->getAdblockPlusPatternsFile());
				}
				else
				{
					adblockplus::enable(false);
				}
			}
		}

		progress = -1;
	}
}

STDMETHODIMP_(void) CIEHostWindow::OnCommandStateChange(long lCommand, VARIANT_BOOL vbEnable)
{
	switch( lCommand )
	{
	case CSC_NAVIGATEBACK:
		CanGoBack = vbEnable ? true:false;
		break;
	case CSC_NAVIGATEFORWARD:
		CanGoForward = vbEnable ? true:false;
		break;
	}
}

STDMETHODIMP_(void) CIEHostWindow::OnNewWindow3(IDispatch **ppDisp, VARIANT_BOOL *Cancel, DWORD dwFlags, BSTR bstrUrlContext, BSTR bstrUrl)
{
	// 我们依靠 IE 自身的弹出窗口过滤程序, 调用 OnNewWindow3 时的窗口都是可以允许的
	* Cancel = FALSE;

	if ( nsConfigManager::OpenInNewIE == nsConfigManager::open_newwindow )
	{
		// 让 IE 自己弹出窗口
	}
	else
	{
		if ( m_pUIController )
		{
			// 用新 Tab 打开弹出窗口
			if ( m_pPluginInstance )
			{
				CoralIETabNPObject * npobj = m_pPluginInstance->getScriptObject();
				if ( npobj )
				{
					RECT rc;
					GetClientRect(&rc);

					// 提前把子窗口创建出来
					CIEHostWindow * pIEHostWindow = CIEHostWindow::Create(GetParent() , rc);
					if ( pIEHostWindow )
					{
						// 告诉 IE 打开的子窗口是什么
						HRESULT hr = (*pIEHostWindow)->get_Application(ppDisp);
						ATLASSERT(SUCCEEDED(hr));

						// 在指定了 ppDisp 之后, IE 会自己控制我们创建的 IEHostWindow 里的 IWebBrowser2, 所以这里不
						// 用再 navigate 了（就算调用了下面的代码也没用）
						// (*pIEHostWindow)->Navigate( bstrUrl, NULL, NULL, NULL, NULL );

						// FIX for IE: 上面的办法打开的子窗口IE不会去设置 Referer, 会导致盗链识别错误等问题
						pIEHostWindow->m_strUrlContext = bstrUrlContext;

						// 传递创建的子窗口的ID
						CStringW strTabUrl;
						strTabUrl.Format(L"ietab:%d", pIEHostWindow->m_hWnd);

						if ( m_pUIController->m_bShouldOpenNewWindow )
						{
							npobj->NewWindow(strTabUrl, m_pUIController->m_strOpenNewWindowName, m_pUIController->m_strOpenNewWindowFeatures);
						}
						else
						{
							npobj->NewTab(strTabUrl, m_pPluginInstance->flags);
						}
					}
				}
			}
		}
	}
}

VOID CIEHostWindow::MinimizeMemory()
{
	static const DWORD INTERVAL_MIN = 30*1000;
	static const DWORD INTERVAL_MAX = 24*60*60*1000;

	static DWORD INTERVAL = m_pPluginInstance->getConfigManager()->getIntPref("coral.ietab.minimize_memory")*1000;
	if ( ( INTERVAL > INTERVAL_MIN ) && ( INTERVAL < INTERVAL_MAX ) )
	{
		static DWORD nTickCount = 0;

		DWORD nCurrentTickCount = GetTickCount();
		if ( ( ( nTickCount > nCurrentTickCount ) && ( nCurrentTickCount > INTERVAL ) ) ||
			( nCurrentTickCount - nTickCount > INTERVAL ) )
		{
			nTickCount = nCurrentTickCount;

			SetProcessWorkingSetSize( GetCurrentProcess(), -1, -1 );
		}
	}
}

STDMETHODIMP_(void) CIEHostWindow::OnDocumentComplete( IDispatch * pDisp, VARIANT* )
{
	m_bSwitchBack = AutoSwitchBack;

	if ( m_pPluginInstance )
	{
		CoralIETabNPObject * npobj = m_pPluginInstance->getScriptObject();
		if ( npobj )
		{
			progress = -1;
			npobj->OnProgressChange(progress);
		}
	}

	// 去掉 IE 默认的 Scrollbar
	CComQIPtr<IWebBrowser2> spBrowser(pDisp);
	if ( spBrowser )
	{
		VARIANT_BOOL vbIsTopLevelContainer;
		if ( SUCCEEDED( spBrowser->get_RegisterAsBrowser( & vbIsTopLevelContainer ) ) )
		{
			if ( VARIANT_TRUE == vbIsTopLevelContainer )		// 判断是否是 frame, 不要对 frame 进行处理
			{
				CComPtr<IDispatch> spDisp;
				if ( SUCCEEDED(spBrowser->get_Document(&spDisp)) && spDisp )
				{
					CComQIPtr<IHTMLDocument2> spDoc (spDisp);
					if ( spDoc )
					{
						CComPtr<IHTMLElement> spElem;
						if ( SUCCEEDED(spDoc->get_body( & spElem )) && spElem )
						{
							CComQIPtr<IHTMLBodyElement> spBody(spElem);
							if ( spBody )
							{
								CComBSTR bstrScroll;
								if ( SUCCEEDED(spBody->get_scroll(&bstrScroll)) && !bstrScroll )
								{
									CComBSTR bstr(L"auto");
									spBody->put_scroll(bstr);
								}

								// 定时清理内存
								MinimizeMemory();
							}
						}
					}
				}
			}
		}
	}
}

STDMETHODIMP_(void) CIEHostWindow::OnSetSecureLockIcon( long SecureLockIcon )
{
	if ( m_pPluginInstance )
	{
		CoralIETabNPObject * npobj = m_pPluginInstance->getScriptObject();
		if ( npobj )
		{
			npobj->OnSecurityChange(SecureLockIcon);
		}
	}
}

STDMETHODIMP_(void) CIEHostWindow::OnTitleChange(BSTR Text)
{
	if ( m_pPluginInstance )
	{
		NPObject * window_object_ = NULL;
		if ( NPN_GetValue(m_pPluginInstance->instance(), NPNVWindowNPObject, &window_object_) == NPERR_NO_ERROR )
		{
			NPVariant document;
			NULL_TO_NPVARIANT(document);
			if ( NPN_GetProperty(m_pPluginInstance->instance(), window_object_, NPN_GetStringIdentifier("document"), &document) && NPVARIANT_IS_OBJECT(document) )
			{
				nsCString title_utf8;
				nsresult nr = NS_UTF16ToCString(nsDependentString(Text), NS_CSTRING_ENCODING_UTF8, title_utf8);

				NPVariant title;
				STRINGZ_TO_NPVARIANT(title_utf8.get(), title);
				NPN_SetProperty(m_pPluginInstance->instance(), NPVARIANT_TO_OBJECT(document), NPN_GetStringIdentifier("title"), &title);
				// NPN_ReleaseVariantValue(&title);

				NPN_ReleaseVariantValue(&document);
			}

			NPN_ReleaseObject(window_object_);
		}
	}
}

STDMETHODIMP_(void) CIEHostWindow::OnStatusTextChange(BSTR bsStatusText)
{
	if ( m_pPluginInstance )
	{
		nsCString url_utf8;
		nsresult nr = NS_UTF16ToCString(nsDependentString(bsStatusText), NS_CSTRING_ENCODING_UTF8, url_utf8);

		NPN_Status( m_pPluginInstance->instance(), url_utf8.get() );
	}
}

LPCTSTR CIEHostWindow::nsUserAgent()
{
	if ( m_nsUserAgent.IsEmpty() )
	{
		CA2T szUserAgent(NPN_UserAgent(m_pPluginInstance->instance()));

		m_nsUserAgent = szUserAgent;
	}

	return (LPCTSTR)m_nsUserAgent;
}

CIEHostWindow * CIEHostWindow::FromHwnd( HWND hwnd )
{
	cs.Lock();
	CIEHostWindow * pRet = IEWindowMap.Lookup(hwnd);
	cs.Unlock();

	return pRet;
}

CIEHostWindow * CIEHostWindow::FromUrl( LPCTSTR lpszUrl )
{
	CIEHostWindow * pRet = NULL;

	if ( lpszUrl && (lpszUrl[0] != _T('\0')) )
	{
		cs.Lock();
		for ( int i = 0; i < IEWindowMap.GetSize(); i++ )
		{
			if ( CIEHostWindow * p = IEWindowMap.GetValueAt(i) )
			{
				if ( FuzzyUrlCompare( p->m_strLoadingUrl, lpszUrl ) )
				{
					pRet = p;
					break;
				}
			}
		}
		cs.Unlock();
	}
	else
	{
		cs.Lock();
		if ( IEWindowMap.GetSize() > 0 )
		{
			pRet = IEWindowMap.GetValueAt(0);
		}
		cs.Unlock();
	}

	return pRet;
}

bool CIEHostWindow::Go( char * pszUrl, long flags )
{
	static const PRInt32 IETAB_SYNC_COOKIES = 1;
	static const PRInt32 IETAB_SYNC_USER_AGENT = 2;
	static const PRInt32 IETAB_SWITCH_BACK = 4;
	static const PRInt32 IETAB_MANUAL_SWITCH_FLAG = 0x4000;

	bool b = false;

	if ( IsWindow() )
	{
		m_pPluginInstance->flags = flags;
		progress = -1;

		AutoSwitchBack = ( flags & IETAB_SWITCH_BACK  ) == IETAB_SWITCH_BACK;
		SyncUserAgent = ( flags & IETAB_SYNC_USER_AGENT ) == IETAB_SYNC_USER_AGENT;
		SyncCookies =( flags & IETAB_SYNC_COOKIES ) == IETAB_SYNC_COOKIES;

		if ( pszUrl && pszUrl[0] )	// 如果 url == NULL, 表明调用时并不想真正访问一个页面, 只是想更新一下 flag
		{
			CComVariant vUrl(pszUrl);
			try
			{
				CComBSTR bstrCurrentURL;
				(*this)->get_LocationURL( & bstrCurrentURL );

				if ( bstrCurrentURL != pszUrl )
				{
					(*this)->Navigate2( &vUrl, NULL, NULL, NULL, NULL );
				}

				b = true;

			} catch(...) {}
		}
	}

	return b;
}

HRESULT CIEHostWindow::CreateWebBrowser(HWND hParent, _U_RECT rc)
{
	// 进程级别的初始化, 都是和 WebBrowser 相关的, 所以放在这里
	ProcessInit();

	HRESULT hr = E_FAIL;

	CComPtr<IWebBrowser2> ptrWB;

	// do-while 里面的步骤, 只要有一步不成功, 就不能往下走了
	do 
	{
		HWND hwnd = CIEHostWindowWinImpl::Create(hParent, rc, _T("about:blank"));
		if ( ! ::IsWindow(hwnd) )
		{
			break;
		}

		m_webBrowser.Attach(hwnd);
		if (FAILED(m_webBrowser.CreateControl(OLESTR("Shell.Explorer"))))
		{
			break;
		}

		if (FAILED(m_webBrowser.QueryControl(&ptrWB)) || !ptrWB)
		{
			break;
		}

		cs.Lock();
		IEWindowMap.Add(hwnd, this);
		cs.Unlock();

		CComPtr<IWebBrowser2>::Attach(ptrWB);	// 注意我们的 CIEHostWindow 对象是继承了 CComPtr<IWebBrowser2> 的, 所以这里用 Attach() 使得
		// 我们可以用 pIEHostWindow->Navigate2() 这样的方式来访问 IWebBrowser2 的接口

		ptrWB->put_RegisterAsBrowser(VARIANT_TRUE);
		ptrWB->put_RegisterAsDropTarget(VARIANT_TRUE);
		// ptrWB->put_Silent(VARIANT_TRUE);		// 虽然能阻止弹出脚本错误提示, 但是 ActiveX 安装提示也一起禁止了, 导致网银不能用
		// 所以用 IAxHostUIHandlerImpl 来

		AtlGetObjectSourceInterface(ptrWB,&m_libid, &m_iid, &m_wMajorVerNum, &m_wMinorVerNum);
		DispEventAdvise(ptrWB, &m_iid);

		hr = S_OK;

	} while(false);

	if ( SUCCEEDED(hr) )
	{
		hr = Connect(ptrWB);
	}
	else
	{
		if ( IsWindow() )
		{
			DestroyWindow();
		}
		else
		{
			// 如果 IsWindow() 为 FALSE 说明前面 do-while 的步骤中连 CIEHostWindowWinImpl::Create() 都没有成功, 因此可以直接 delete 了
			delete this;
		}
	}

	return hr;
}

HRESULT CIEHostWindow::Connect(IWebBrowser2 * ptrWB)
{
	HRESULT hr = E_FAIL;

	ATLASSERT(!m_pUIController);
	m_pUIController = new CComObject<IAxHostUIHandlerImpl>();
	if ( m_pUIController )
	{
		m_pUIController->Assign(this);

		if ( NULL == hhookGetMsg )
		{
			hhookGetMsg = SetWindowsHookEx( WH_GETMESSAGE, GetMsgProc, NULL, GetCurrentThreadId() );
		}
		
		if ( NULL == hhookCallWndProcRet )
		{
			hhookCallWndProcRet = SetWindowsHookEx( WH_CALLWNDPROCRET, CallWndRetProc, NULL, GetCurrentThreadId() );
		}

		CComQIPtr<IObjectWithSite> ptrObjWithSite;
		hr = m_webBrowser.QueryHost(&ptrObjWithSite);
		ATLASSERT(SUCCEEDED(hr));
		if(SUCCEEDED(hr) && ptrObjWithSite)
		{
			hr = ptrObjWithSite->SetSite(m_pUIController->GetUnknown());
			ATLASSERT(SUCCEEDED(hr));
		}

		CComQIPtr<IOleObject> ptrOleObj(ptrWB);
		if ( ptrOleObj )
		{
			CComQIPtr<IOleClientSite> spSite(m_pUIController->GetUnknown());
			hr = ptrOleObj->SetClientSite(spSite);
			ATLASSERT(SUCCEEDED(hr));
		}
	}

	return hr;
}

HRESULT CIEHostWindow::Disconnect()
{
	HRESULT hr = E_FAIL;

	cs.Lock();
	IEWindowMap.Remove( m_hWnd );
	int n = IEWindowMap.GetSize();
	cs.Unlock();

	if ( n == 0 )
	{
		if ( hhookGetMsg != NULL )
		{
			if ( UnhookWindowsHookEx(hhookGetMsg) )
			{
				hhookGetMsg = NULL;
			}
		}
		
		if ( hhookCallWndProcRet != NULL )
		{
			if ( UnhookWindowsHookEx(hhookCallWndProcRet) )
			{
				hhookCallWndProcRet = NULL;
			}
		}
	}

	CComPtr<IWebBrowser2> ptrWB;
	hr = m_webBrowser.QueryControl(&ptrWB);
	if ( SUCCEEDED(hr) && ptrWB )
	{
		AtlGetObjectSourceInterface(ptrWB,&m_libid, &m_iid, &m_wMajorVerNum, &m_wMinorVerNum);
		hr = DispEventUnadvise(ptrWB, &m_iid);
		ATLASSERT(SUCCEEDED(hr));
	}

	CComQIPtr<IObjectWithSite> ptrObjWithSite;
	hr = m_webBrowser.QueryHost(&ptrObjWithSite);
	if(SUCCEEDED(hr) && ptrObjWithSite)
	{
		hr = ptrObjWithSite->SetSite(NULL);
		ATLASSERT(SUCCEEDED(hr));

		CComQIPtr<IOleObject> ptrOleObj(ptrWB);
		if ( ptrOleObj )
		{
			hr = ptrOleObj->SetClientSite(NULL);
			ATLASSERT(SUCCEEDED(hr));
		}
	}

	return hr;
}

// 返回 ATL:xxxxx 窗口的句柄
inline HWND GetWebBrowserControlWindow(HWND hwnd)
{
	// Internet Explorer_Server 往上三级是 ATL:xxxxx 窗口
	HWND hwndAtl = ::GetParent(::GetParent(::GetParent(hwnd)));
	TCHAR szClassName[MAX_PATH];
	if ( GetClassName(hwndAtl, szClassName, ARRAYSIZE(szClassName)) > 0 )
	{
		if ( _tcsncmp(szClassName, _T("ATL:"), 4) == 0 )
		{
			return hwndAtl;
		}
	}
	
	return NULL;
}

// 我们要把消息发送到 MozillaContentWindow 的子窗口，但是这个窗口结构比较复杂，Firefox/SeaMonkey各不相同，
// Firefox 如果开启了 OOPP 也会增加一级，所以这里专门写一个查找的函数
inline HWND GetMozillaContentWindow(HWND hwndAtl)
{
	//这里来个土办法，用一个循环往上找，直到找到 MozillaContentWindow 为止
	HWND hwnd = ::GetParent(hwndAtl);
	for ( int i = 0; i < 5; i++ )
	{
		hwnd = ::GetParent( hwnd );
		TCHAR szClassName[MAX_PATH];
		if ( GetClassName(::GetParent(hwnd), szClassName, ARRAYSIZE(szClassName)) > 0 )
		{
			if ( _tcscmp(szClassName, _T("MozillaContentWindowClass")) == 0 )
			{
				return hwnd;
			}
		}
	}
	
	return NULL;
}

// Firefox 4.0 开始采用了新的窗口结构
// 对于插件，是放在 GeckoPluginWindow 窗口里，往上有一个 MozillaWindowClass，再往上是顶层的
// MozillaWindowClass，我们的消息要发到顶层，所以再写一个查找的函数
inline HWND GetTopMozillaWindowClassWindow(HWND hwndAtl)
{
	HWND hwnd = ::GetParent(hwndAtl);
	for ( int i = 0; i < 5; i++ )
	{
		HWND hwndParent = ::GetParent( hwnd );
		if ( NULL == hwndParent ) break;
		hwnd = hwndParent;
	}
	
	TCHAR szClassName[MAX_PATH];
	if ( GetClassName(hwnd, szClassName, ARRAYSIZE(szClassName)) > 0 )
	{
		if ( _tcscmp(szClassName, _T("MozillaWindowClass")) == 0 )
		{
			return hwnd;
		}
	}

	return NULL;
}

inline VOID PreTranslateAccelerator(HWND hwnd, MSG * pMsg)
{
	static const UINT  WM_HTML_GETOBJECT = ::RegisterWindowMessage(_T( "WM_HTML_GETOBJECT" ));

	LRESULT lRes;
	if ( ::SendMessageTimeout( hwnd, WM_HTML_GETOBJECT, 0, 0, SMTO_ABORTIFHUNG, 1000, (DWORD*)&lRes ) && lRes )
	{
		CComPtr<IHTMLDocument2> spDoc;
		if (SUCCEEDED(::ObjectFromLresult(lRes, IID_IHTMLDocument2, 0, (void**)&spDoc)))
		{
			CComQIPtr<IServiceProvider> spSP(spDoc);
			if (spSP)
			{
				CComPtr<IWebBrowser2> spWB2;
				if ( SUCCEEDED(spSP->QueryService(IID_IWebBrowserApp, IID_IWebBrowser2, (void**)&spWB2)) && spWB2 )
				{
					CComPtr< IDispatch > pDisp;
					if ( SUCCEEDED(spWB2->get_Application(&pDisp)) && pDisp )
					{
						CComQIPtr< IOleInPlaceActiveObject > spObj( pDisp );
						if ( spObj )
						{
							if (spObj->TranslateAccelerator(pMsg) == S_OK)
							{
								pMsg->message = /*uMsg = */WM_NULL;
							}
						}
					}
				}
			}
		}
	}
}

VOID CIEHostWindow::HandOverFocus()
{
	HWND hwndMessageTarget = GetMozillaContentWindow(m_hWnd);
	if ( ! hwndMessageTarget )
	{
		hwndMessageTarget = GetTopMozillaWindowClassWindow(m_hWnd);
	}

	if ( hwndMessageTarget != NULL )
	{
		::SetFocus(hwndMessageTarget);
	}
}

LRESULT CALLBACK CIEHostWindow::GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam) 
{ 
	if ( (nCode >= 0) && (wParam == PM_REMOVE) && lParam)
	{
		MSG * pMsg = (MSG *)lParam;

		UINT uMsg = pMsg->message;
		if ( (uMsg >= WM_KEYFIRST) && (uMsg <= WM_KEYLAST) )
		{
			HWND hwnd = pMsg->hwnd;
			TCHAR szClassName[MAX_PATH];
			if ( GetClassName( hwnd, szClassName, ARRAYSIZE(szClassName) ) > 0 )
			{
				/*
				CString str;
				str.Format(_T("%s: Msg = %.4X, wParam = %.8X, lParam = %.8X\r\n"), szClassName, uMsg, pMsg->wParam, pMsg->lParam );
				OutputDebugString(str);
				*/
				if ( ( WM_KEYDOWN == uMsg ) && ( VK_TAB == pMsg->wParam ) && (_tcscmp(szClassName, _T("Internet Explorer_TridentCmboBx")) == 0) )
				{
					hwnd = ::GetParent(pMsg->hwnd);
				}
				else
				{
					if ( _tcscmp(szClassName, _T("Internet Explorer_Server")) == 0 )
					{
						// hwnd = pMsg->hwnd;
					}
					else
					{
						hwnd = NULL;
					}
				}

				if (hwnd)
				{
					PreTranslateAccelerator(hwnd, pMsg);

					if ( (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN /*|| uMsg == WM_CHAR*/) )
					{
						// BUG FIX: Characters like @, #, � (and others that require AltGr on European keyboard layouts) cannot be entered in CoralIETab
						// Suggested by Meyer Kuno (Helbling Technik): AltGr is represented in Windows massages as the combination of Alt+Ctrl, and that is used for text input, not for menu naviagation.
						// 
						bool bCtrlPressed = HIBYTE(GetKeyState(VK_CONTROL))!=0;
						bool bAltPressed = HIBYTE(GetKeyState(VK_MENU))!=0;

						if ( ( bCtrlPressed ^ bAltPressed ) || ((pMsg->wParam >= VK_F1) && (pMsg->wParam <= VK_F24)) )
						{
							/* Test Cases by Meyer Kuno (Helbling Technik):
							Ctrl-L (change keyboard focus to navigation bar)
							Ctrl-O (open a file)
							Ctrl-P (print)
							Alt-d (sometimes Alt-s): "Address": the IE-way of Ctrl-L
							Alt-F (open File menu) (NOTE: BUG: keyboard focus is not moved)
							*/
							HWND hwndAtl = GetWebBrowserControlWindow(hwnd);
							if (hwndAtl)
							{
								HWND hwndMessageTarget = GetMozillaContentWindow(hwndAtl);
								if ( ! hwndMessageTarget )
								{
									hwndMessageTarget = GetTopMozillaWindowClassWindow(hwndAtl);
								}

								if ( hwndMessageTarget )
								{
									if ( bAltPressed )
									{
										// BUG FIX: Alt-F (open File menu): keyboard focus is not moved
										switch ( pMsg->wParam )
										{
										case 'F':
										case 'E':
										case 'V':
										case 'S':
										case 'B':
										case 'T':
										case 'H':
											::SetFocus( hwndMessageTarget );
											break;
											// 以下快捷键由 IE 内部处理, 如果传给 Firefox 的话会导致重复
										case VK_LEFT:
										case VK_RIGHT:
											uMsg = WM_NULL;
											break;
										default:
											break;
										}
									}
									else
									{
										if ( bCtrlPressed )
										{
											switch ( pMsg->wParam )
											{
												// 以下快捷键由 IE 内部处理, 如果传给 Firefox 的话会导致重复
											case 'P':
											case 'F':
												uMsg = WM_NULL;
												break;
											default:
												break;
											}
										}
									}

									if ( uMsg != WM_NULL )
									{
										if ( ! nsConfigManager::isInIsolatedProcess )
										{
											::SendMessage( hwndMessageTarget, uMsg, pMsg->wParam, pMsg->lParam );
										}
										else
										{
											// 不是同一个进程的话调用 SendMessage() 会崩溃
											::PostMessage( hwndMessageTarget, uMsg, pMsg->wParam, pMsg->lParam );
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return CallNextHookEx(hhookGetMsg, nCode, wParam, lParam); 
} 

LRESULT CALLBACK CIEHostWindow::CallWndRetProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION)
	{
		CWPRETSTRUCT * info = (CWPRETSTRUCT*) lParam;
		UINT uMsg = info->message;
		// info->wParam == NULL 表示焦点移到其它进程去了，我们只有在这个时候才要保护IE的焦点
		if ( ( WM_KILLFOCUS == uMsg ) && ( NULL == info->wParam ) )
		{
			HWND hwnd = info->hwnd;
			TCHAR szClassName[MAX_PATH];
			if ( GetClassName( hwnd, szClassName, ARRAYSIZE(szClassName) ) > 0 )
			{
				if ( _tcscmp(szClassName, _T("Internet Explorer_Server")) == 0 )
				{
					// 重新把焦点移到 ATL:xxxx 窗口上，这样从别的进程窗口切换回来的时候IE才能有焦点
					HWND hwndAtl = GetWebBrowserControlWindow(hwnd);
					if (hwndAtl) ::SetFocus(hwndAtl);
				}
			}
		}
	}

	return CallNextHookEx(hhookCallWndProcRet, nCode, wParam, lParam);
}

//////////////////////////////////////////////////////////////////////////

STDMETHODIMP IAxHostUIHandlerImpl::GetHostInfo(DOCHOSTUIINFO FAR* pInfo)
{
	if(pInfo != NULL)
	{
		pInfo->dwFlags = DOCHOSTUIFLAG_NO3DBORDER
			|DOCHOSTUIFLAG_THEME
			|DOCHOSTUIFLAG_ENABLE_FORMS_AUTOCOMPLETE
			|DOCHOSTUIFLAG_LOCAL_MACHINE_ACCESS_CHECK
			|DOCHOSTUIFLAG_ENABLE_INPLACE_NAVIGATION
			/*|DOCHOSTUIFLAG_DISABLE_OFFSCREEN*/;		// 启用此标志会造成改变窗口大小时屏幕闪烁
		pInfo->dwDoubleClick = DOCHOSTUIDBLCLK_DEFAULT;
	}

	return S_OK;
}

STDMETHODIMP IAxHostUIHandlerImpl::TranslateAccelerator(LPMSG lpMsg, const GUID FAR* pguidCmdGroup, DWORD nCmdID)
{
	HRESULT hr = S_FALSE;

	if ( lpMsg )
	{
		switch ( lpMsg->message )
		{
		case WM_KEYDOWN:
			{
				bool bCtrlPressed = HIBYTE(GetKeyState(VK_CONTROL))!=0;
				bool bAltPressed = HIBYTE(GetKeyState(VK_MENU))!=0;
				bool bShiftPressed = HIBYTE(GetKeyState(VK_SHIFT))!=0;

				// Ctrl-N 会让 IE 自己打开一个 IE 窗口，该窗口无法被我们的处理函数拦截
				// BUG #22839: AltGr + N (也就是 Ctrl-Alt-N）也被拦截了，在这里只拦截 Ctrl-N
				if ( bCtrlPressed && (!bAltPressed) && (!bShiftPressed) && ('N' == lpMsg->wParam) )
				{
					hr = S_OK;
				}

				break;
			}
		}
	}

	return hr;
}

STDMETHODIMP IAxHostUIHandlerImpl::GetExternal(IDispatch** ppDispatch)
{
	if ( ! m_spShellUIHelper )
	{
		if ( FAILED(m_spShellUIHelper.CoCreateInstance(CLSID_ShellUIHelper)) ) return E_FAIL;
	}
	
	if ( m_spShellUIHelper )
	{
		CComQIPtr<IDispatch> spDisp(m_spShellUIHelper);
		* ppDispatch = spDisp.Detach();

		return S_OK;
	}
	else
	{
		* ppDispatch = NULL;

		return S_FALSE;
	}
}

STDMETHODIMP IAxHostUIHandlerImpl::Exec( 
	/* [unique][in] */ const GUID *pguidCmdGroup,
	/* [in] */ DWORD nCmdID,
	/* [in] */ DWORD nCmdexecopt,
	/* [unique][in] */ VARIANT *pvaIn,
	/* [unique][out][in] */ VARIANT *pvaOut)
{
	HRESULT hr = pguidCmdGroup ? OLECMDERR_E_UNKNOWNGROUP : OLECMDERR_E_NOTSUPPORTED;

	if ( pguidCmdGroup && IsEqualGUID(*pguidCmdGroup, CGID_DocHostCommandHandler))
	{
		// 屏蔽脚本错误提示
		if ( nCmdID == OLECMDID_SHOWSCRIPTERROR )
		{
			hr = S_OK;

			// 这里只是简单屏蔽掉
			// 如果要进一步处理, 参考:
			// 《How to handle script errors as a WebBrowser control host》
			// http://support.microsoft.com/default.aspx?scid=kb;en-us;261003

			(*pvaOut).vt = VT_BOOL;
			// Continue running scripts on the page.
			(*pvaOut).boolVal = VARIANT_TRUE;
		}
	}

	return hr;
}

STDMETHODIMP IAxHostUIHandlerImpl::moveTo(LONG x, LONG y)
{
	return window_call("moveTo", x, y);
}

STDMETHODIMP IAxHostUIHandlerImpl::moveBy(LONG x, LONG y)
{
	return window_call("moveBy", x, y);
}

STDMETHODIMP IAxHostUIHandlerImpl::resizeTo(LONG x, LONG y)
{
	return window_call("resizeTo", x, y);
}

STDMETHODIMP IAxHostUIHandlerImpl::resizeBy(LONG x, LONG y)
{
	return window_call("resizeBy", x, y);
}

STDMETHODIMP IAxHostUIHandlerImpl::EvaluateNewWindow(
	/* [string][in] */ LPCWSTR pszUrl,
	/* [string][in] */ LPCWSTR pszName,
	/* [string][in] */ LPCWSTR pszUrlContext,
	/* [string][in] */ LPCWSTR pszFeatures,
	/* [in] */ BOOL fReplace,
	/* [in] */ DWORD dwFlags,
	/* [in] */ DWORD /*dwUserActionTime*/)
{
	m_bShouldOpenNewWindow = FALSE;

	m_strOpenNewWindowName = pszName;
	m_strOpenNewWindowFeatures = pszFeatures;

	do
	{
		if ( dwFlags & NWMF_FORCEWINDOW )
		{
			m_bShouldOpenNewWindow = TRUE;
			break;
		}

		if (( dwFlags & NWMF_FORCETAB ) || ( nsConfigManager::OpenInFirefoxTab == nsConfigManager::open_newwindow ))
		{
			m_bShouldOpenNewWindow = FALSE;
			break;
		}

		// 按下 SHIFT 键打开新窗口
		if ( HIBYTE(GetKeyState(VK_SHIFT)) )
		{
			m_bShouldOpenNewWindow = TRUE;
			break;
		}

		// 以上都不是, 按照 Firefox 的规则来
		// 这里是实时读取当前的设置, 因为 Firefox 对于这两个设置是立即生效的
		if ( ! m_pParent ) break;
		if ( ! m_pParent->m_pPluginInstance ) break;
		int open_newwindow = m_pParent->m_pPluginInstance->getConfigManager()->getIntPref("browser.link.open_newwindow", 3);
		int open_newwindow_restriction = m_pParent->m_pPluginInstance->getConfigManager()->getIntPref("browser.link.open_newwindow.restriction", 2);
		switch (open_newwindow_restriction)
		{
		case 0:
			{
				// 总是以 browser.link.open_newwindow 为准
				m_bShouldOpenNewWindow = (open_newwindow == 2);
				break;
			}
		case 1:
			{
				// 忽略 browser.link.open_newwindow 设置, 总是打开新窗口
				m_bShouldOpenNewWindow = TRUE;
				break;
			}
		case 2:
			{
				// pszFeatures 非空时就会弹出新窗口, 否则以 browser.link.open_newwindow 为准
				m_bShouldOpenNewWindow = ( pszFeatures && pszFeatures[0] ) || (open_newwindow == 2);
				break;
			}
		}

	} while(false);

	return E_FAIL;		// 用 IE 默认的弹出窗口过滤程序
}

STDMETHODIMP IAxHostUIHandlerImpl::window_call(const char * methodName, LONG x, LONG y)
{
	// Suggested by Sonja's boodschappenlijst (jeepeenl@gmail.com):
	// Tools -> Options -> Content -> Enable Javascript [Advanced] -> Allow scripts to “Move or resize existing windows”.
	if ( ! m_pParent ) return S_OK;
	if ( ! m_pParent->m_pPluginInstance ) return S_OK;
	if (nsConfigManager::isClassicMode || m_pParent->m_pPluginInstance->getConfigManager()->getBoolPref("dom.disable_window_move_resize")) return S_OK;

	HRESULT hr = E_FAIL;

	do 
	{
		if ( ! m_pParent ) break;
		if ( ! m_pParent->m_pPluginInstance ) break;

		NPP npp = m_pParent->m_pPluginInstance->instance();

		NPObject * window_object_ = NULL;
		if ( NPN_GetValue(npp, NPNVWindowNPObject, &window_object_) != NPERR_NO_ERROR ) break;

		NPIdentifier id_method = NPN_GetStringIdentifier(methodName);
		if ( !id_method ) break;

		NPVariant pt[2];
		INT32_TO_NPVARIANT(x, pt[0]);
		INT32_TO_NPVARIANT(y, pt[1]);
		NPVariant result;
		NULL_TO_NPVARIANT(result);
		if ( NPN_Invoke(npp, window_object_, id_method, pt, 2, &result) )
		{
			NPN_ReleaseVariantValue(&result);
		}

		hr = S_OK;

	} while(false);

	return hr;
}