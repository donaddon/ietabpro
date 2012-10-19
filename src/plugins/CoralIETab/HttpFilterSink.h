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

#pragma once

#include <urlmon.h>

#include "../3rdParty/passthru_app/ProtocolImpl.h"

class CIEHostWindow;

class CHttpFilterSink :
	public PassthroughAPP::CInternetProtocolSinkWithSP<CHttpFilterSink>,
	public IHttpNegotiate,
	public IAuthenticate
{
	typedef PassthroughAPP::CInternetProtocolSinkWithSP<CHttpFilterSink> BaseClass;

public:

	CHttpFilterSink();
	
	BEGIN_COM_MAP(CHttpFilterSink)
		COM_INTERFACE_ENTRY(IHttpNegotiate)
		COM_INTERFACE_ENTRY_FUNC(IID_IAuthenticate, 0, QueryIAuthenticate)
		COM_INTERFACE_ENTRY_CHAIN(BaseClass)
	END_COM_MAP()

	BEGIN_SERVICE_MAP(CHttpFilterSink)
		SERVICE_ENTRY(IID_IHttpNegotiate)
		SERVICE_ENTRY(IID_IAuthenticate)
	END_SERVICE_MAP()

	/**
	 * 为了保证兼容性，在不确定能否正确登录的时候，屏蔽 IAuthenticate 接口，这样，IE 会弹出自己的 login 对话框。
	 */
	static HRESULT WINAPI QueryIAuthenticate(void* pv, REFIID riid, LPVOID* ppv, DWORD dw);

	// IHttpNegotiate
	STDMETHODIMP BeginningTransaction(
	/* [in] */ LPCWSTR szURL,
	/* [in] */ LPCWSTR szHeaders,
	/* [in] */ DWORD dwReserved,
	/* [out] */ LPWSTR *pszAdditionalHeaders);

	STDMETHODIMP OnResponse(
		/* [in] */ DWORD dwResponseCode,
		/* [in] */ LPCWSTR szResponseHeaders,
		/* [in] */ LPCWSTR szRequestHeaders,
		/* [out] */ LPWSTR *pszAdditionalRequestHeaders);

	// IInternetProtocolSink
	STDMETHODIMP ReportProgress(
		/* [in] */ ULONG ulStatusCode,
		/* [in] */ LPCWSTR szStatusText);

	// IAuthenticate
	STDMETHODIMP Authenticate( 
		/* [out] */ HWND *phwnd,
		/* [out] */ LPWSTR *pszUsername,
		/* [out] */ LPWSTR *pszPassword);

private:

	/** 查询本请求所对应的 CIEHostWindow 对象 */
	VOID _QueryIEHostWindow();

	// 把我们要定制的 Headers 加上去
	VOID _SetCustomHeaders(LPWSTR *pszAdditionalHeaders);

	/** 从 Firefox 中导入 Cookie */
	VOID _ImportCookies();

	/** 从 HTTP Response Headers 中扫描出 Cookies 并设置到 Firefox 中 */
	VOID _ExportCookies(LPCWSTR szResponseHeaders);

	// （ReportProgress() 方法内部专用）检查是否要过滤
	BOOL _CanLoadContent( PRUint32 aContentType );

	/** 解析 Content-Type 成 nsIContentPolicy 的定义 */
	PRUint32 _ScanContentType(LPCWSTR szContentType);

private:

	/** 本次请求的 URL */
	CString m_strURL;

	/** 本次请求的 Referer */
	CString m_strReferer;

	/** Login 用户名*/
	CString m_strUsername;
	/** Login 密码 */
	CString m_strPassword;

	/** 发起本次请求的 CIEHostWindow 对象 */
	CIEHostWindow * m_pIEHostWindow;

	/** 是否是页面的子请求？例如, 对HTML页面里面包含的图片、脚本等的请求就是子请求 */
	bool m_bIsSubRequest;

private:

	friend class CHttpFilterAPP;

	/**
	 * 这是用来过滤广告的, 其基本原理是, 对于 HTML 或者图片, 我们给 IE 传一个空
	 * 文件过去, 这样就过滤了. 这个空文件的内容, 我们用一个缓冲区来保存它, 然后
	 * 在 IE 读取的时候把缓冲区的内容返回给 IE.
	 */
	const BYTE * pTargetBuffer;
	DWORD dwTargetBufSize;
};

class CHttpFilterAPP;
typedef PassthroughAPP::CustomSinkStartPolicy<CHttpFilterAPP, CHttpFilterSink> HttpFilterStartPolicy;

class CHttpFilterAPP : public PassthroughAPP::CInternetProtocol<HttpFilterStartPolicy>
{
	typedef PassthroughAPP::CInternetProtocol<HttpFilterStartPolicy> BaseClass;

public:

	CHttpFilterAPP() {}

	~CHttpFilterAPP();

	// IInternetProtocolRoot
	STDMETHODIMP Start(
		/* [in] */ LPCWSTR szUrl,
		/* [in] */ IInternetProtocolSink *pOIProtSink,
		/* [in] */ IInternetBindInfo *pOIBindInfo,
		/* [in] */ DWORD grfPI,
		/* [in] */ HANDLE_PTR dwReserved);

	STDMETHODIMP Read(
		/* [in, out] */ void *pv,
		/* [in] */ ULONG cb,
		/* [out] */ ULONG *pcbRead);

	STDMETHODIMP Terminate(
		/* [in] */ DWORD dwOptions);

public:

	HRESULT FinalConstruct();

private:

	/** 对应的 CHttpFilterSink 对象 */
	CHttpFilterSink * m_Sink;

	/** 计数器, 因为 Read() 方法会被反复调用, 我们需要一个变量来记录已经 Read 了多少数据了 */
	DWORD m_nDataWritten;
};
