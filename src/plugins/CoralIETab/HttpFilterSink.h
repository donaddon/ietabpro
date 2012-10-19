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
	 * Ϊ�˱�֤�����ԣ��ڲ�ȷ���ܷ���ȷ��¼��ʱ������ IAuthenticate �ӿڣ�������IE �ᵯ���Լ��� login �Ի���
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

	/** ��ѯ����������Ӧ�� CIEHostWindow ���� */
	VOID _QueryIEHostWindow();

	// ������Ҫ���Ƶ� Headers ����ȥ
	VOID _SetCustomHeaders(LPWSTR *pszAdditionalHeaders);

	/** �� Firefox �е��� Cookie */
	VOID _ImportCookies();

	/** �� HTTP Response Headers ��ɨ��� Cookies �����õ� Firefox �� */
	VOID _ExportCookies(LPCWSTR szResponseHeaders);

	// ��ReportProgress() �����ڲ�ר�ã�����Ƿ�Ҫ����
	BOOL _CanLoadContent( PRUint32 aContentType );

	/** ���� Content-Type �� nsIContentPolicy �Ķ��� */
	PRUint32 _ScanContentType(LPCWSTR szContentType);

private:

	/** ��������� URL */
	CString m_strURL;

	/** ��������� Referer */
	CString m_strReferer;

	/** Login �û���*/
	CString m_strUsername;
	/** Login ���� */
	CString m_strPassword;

	/** ���𱾴������ CIEHostWindow ���� */
	CIEHostWindow * m_pIEHostWindow;

	/** �Ƿ���ҳ�������������, ��HTMLҳ�����������ͼƬ���ű��ȵ�������������� */
	bool m_bIsSubRequest;

private:

	friend class CHttpFilterAPP;

	/**
	 * �����������˹���, �����ԭ����, ���� HTML ����ͼƬ, ���Ǹ� IE ��һ����
	 * �ļ���ȥ, �����͹�����. ������ļ�������, ������һ����������������, Ȼ��
	 * �� IE ��ȡ��ʱ��ѻ����������ݷ��ظ� IE.
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

	/** ��Ӧ�� CHttpFilterSink ���� */
	CHttpFilterSink * m_Sink;

	/** ������, ��Ϊ Read() �����ᱻ��������, ������Ҫһ����������¼�Ѿ� Read �˶��������� */
	DWORD m_nDataWritten;
};
