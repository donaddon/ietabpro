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

#include <ExDispid.h>
#include <shlobj.h>

class nsPluginInstance;

class CIEHostWindow;

#define WM_ADBLOCK_NOTIFY		WM_USER + 1234

/** ���� WM_COMMAND ��Ϣ */
static const WORD CMD_SIG_EXECWB = 0xE0EC;
static const WORD CMD_SIG_CMDTARGET = 0xC3D7;

/** ����ͨ�������� IE �Ľ�����п���, �������νű�������ʾ�� */
class IAxHostUIHandlerImpl :
	public CComObjectRootEx<CComSingleThreadModel>,
	public IOleCommandTarget,
	public IOleClientSite,
	public IDocHostUIHandler,
	public IHTMLOMWindowServices,
	public INewWindowManager,
	public IServiceProviderImpl<IAxHostUIHandlerImpl>
{
public:

	IAxHostUIHandlerImpl() : m_pParent( NULL ), m_bShouldOpenNewWindow(FALSE) {}

	BEGIN_COM_MAP(IAxHostUIHandlerImpl)
		COM_INTERFACE_ENTRY(IOleCommandTarget)
		COM_INTERFACE_ENTRY(IOleClientSite)
		COM_INTERFACE_ENTRY(IDocHostUIHandler)
		COM_INTERFACE_ENTRY(IHTMLOMWindowServices)
		COM_INTERFACE_ENTRY(INewWindowManager)
		COM_INTERFACE_ENTRY(IServiceProvider)
	END_COM_MAP()

	BEGIN_SERVICE_MAP(IAxHostUIHandlerImpl)
		SERVICE_ENTRY(SID_SHTMLOMWindowServices)
		SERVICE_ENTRY(SID_SNewWindowManager)
	END_SERVICE_MAP()

	// IOleClientSite
	STDMETHOD(SaveObject)(void) { return S_OK; }
	STDMETHOD(GetMoniker)(DWORD nAssign, DWORD nWhichMoniker, IMoniker **ppMoniker) { return E_NOTIMPL; }
	STDMETHOD(GetContainer)(IOleContainer **ppContainer) { *ppContainer = NULL; return E_NOINTERFACE; }
	STDMETHOD(ShowObject)(void) { return S_OK; }
	STDMETHOD(OnShowWindow)(BOOL fShow) { return S_OK; }
	STDMETHOD(RequestNewObjectLayout)(void) { return E_NOTIMPL; }

	// IDocHostUIHandler
	STDMETHOD(GetHostInfo)(DOCHOSTUIINFO FAR* pInfo);
	STDMETHOD(TranslateAccelerator)(LPMSG lpMsg, const GUID FAR* pguidCmdGroup, DWORD nCmdID);
	STDMETHOD(GetExternal)(IDispatch** ppDispatch);
	STDMETHOD(ShowContextMenu)(DWORD dwID, POINT FAR* ppt, IUnknown FAR* pcmdtReserved,
		IDispatch FAR* pdispReserved) { return S_FALSE; }
	STDMETHOD(ShowUI)(DWORD dwID, IOleInPlaceActiveObject FAR* pActiveObject,
		IOleCommandTarget FAR* pCommandTarget,
		IOleInPlaceFrame  FAR* pFrame,
		IOleInPlaceUIWindow FAR* pDoc) { return S_FALSE; }
	STDMETHOD(HideUI)(void) { return S_OK; }
	STDMETHOD(UpdateUI)(void) { return S_OK; }
	STDMETHOD(EnableModeless)(BOOL fEnable) { return E_NOTIMPL; }
	STDMETHOD(OnDocWindowActivate)(BOOL fActivate) { return E_NOTIMPL; }
	STDMETHOD(OnFrameWindowActivate)(BOOL fActivate) { return E_NOTIMPL; }
	STDMETHOD(ResizeBorder)(LPCRECT prcBorder, IOleInPlaceUIWindow FAR* pUIWindow, BOOL fRameWindow) { return E_NOTIMPL; }
	STDMETHOD(GetOptionKeyPath)(LPOLESTR FAR* pchKey, DWORD dw) { return S_FALSE; }
	STDMETHOD(GetDropTarget)(IDropTarget* pDropTarget, IDropTarget** ppDropTarget) { return E_NOTIMPL; }
	STDMETHOD(TranslateUrl)(DWORD dwTranslate, OLECHAR* pchURLIn, OLECHAR** ppchURLOut) { * ppchURLOut  = NULL; return S_FALSE; }
	STDMETHOD(FilterDataObject)(IDataObject* pDO, IDataObject** ppDORet) { * ppDORet = NULL; return S_FALSE; }

	// IOleCommandTarget
	STDMETHOD(QueryStatus)( 
		/* [unique][in] */ const GUID *pguidCmdGroup,
		/* [in] */ ULONG cCmds,
		/* [out][in][size_is] */ OLECMD prgCmds[  ],
		/* [unique][out][in] */ OLECMDTEXT *pCmdText) { return pguidCmdGroup ? OLECMDERR_E_UNKNOWNGROUP : OLECMDERR_E_NOTSUPPORTED; }
	STDMETHOD(Exec)( 
		/* [unique][in] */ const GUID *pguidCmdGroup,
		/* [in] */ DWORD nCmdID,
		/* [in] */ DWORD nCmdexecopt,
		/* [unique][in] */ VARIANT *pvaIn,
		/* [unique][out][in] */ VARIANT *pvaOut);

	// IHTMLOMWindowServices
	STDMETHOD(moveTo)(LONG x, LONG y);
	STDMETHOD(moveBy)(LONG x, LONG y);
	STDMETHOD(resizeTo)(LONG x, LONG y);
	STDMETHOD(resizeBy)(LONG x, LONG y);

	// INewWindowManager
	/*
	 * ͨ�������, ���Ƕ����� IE Ĭ�ϵĵ������ڹ��˳���, Ȼ���� NewWindow3 �¼��д���
	 * Ȼ��, NewWindow3 �޷���֪ window.open() ����ʱ�Ĳ���, ����: window.open("", "childWin", "width=320,height=240");
	 * ���ʱ�����Ǿ�Ҫ�� pszFeatures �õ� "width=320,height=240"
	 */
	STDMETHOD(EvaluateNewWindow)(
		/* [string][in] */ LPCWSTR pszUrl,
		/* [string][in] */ LPCWSTR pszName,
		/* [string][in] */ LPCWSTR pszUrlContext,
		/* [string][in] */ LPCWSTR pszFeatures,
		/* [in] */ BOOL fReplace,
		/* [in] */ DWORD dwFlags,
		/* [in] */ DWORD dwUserActionTime);

private:

	// ���� Firefox �� window.moveTo(), window.resizeBy() �ȷ�����ͨ�÷���
	STDMETHOD(window_call)(const char * methodName, LONG x, LONG y);

public:

	VOID Assign(CIEHostWindow * pParent) { m_pParent = pParent; }

private:

	friend CIEHostWindow;

	CIEHostWindow * m_pParent;

	/** ���� EvaluateNewWindow �Ĳ����ж��Ƿ��� IE �Լ��������ڶ����������� Tab �ӹ� */
	BOOL m_bShouldOpenNewWindow;

	/** window.open(url, name, features) �� name ���� */
	CString m_strOpenNewWindowName;

	/** window.open(url, name, features) �� features ���� */
	CString m_strOpenNewWindowFeatures;

private:

	CComPtr<IShellUIHelper> m_spShellUIHelper;
};

typedef CWinTraits<WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN> CIEHostWinTraits;
typedef CWindowImpl<CIEHostWindow, CWindow, CIEHostWinTraits> CIEHostWindowWinImpl;

class CIEHostWindow :
	public CComPtr<IWebBrowser2>,
	public CIEHostWindowWinImpl,
	public IDispEventImpl< /*nID =*/ 1, CIEHostWindow>
{
public:

	CIEHostWindow();

	~CIEHostWindow();

	static CIEHostWindow * Create(HWND hParent, _U_RECT rc);

public:

	BEGIN_MSG_MAP( CIEHostWindow )
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_PARENTNOTIFY, OnParentNotify)
		MESSAGE_HANDLER(WM_ADBLOCK_NOTIFY, OnAdblockNotify)
		MESSAGE_HANDLER(WM_COMMAND, OnCommand)
	END_MSG_MAP()

	LRESULT OnDestroy(UINT uMsg, WPARAM wParam , LPARAM lParam, BOOL& bHandled);

	/** IE �ؼ��ر�ʱ���յ� WM_PARENTNOTIFY ��Ϣ */
	LRESULT OnParentNotify(UINT uMsg, WPARAM wParam , LPARAM lParam, BOOL& bHandled);

	/** �й�汻����ʱ HttpFilterSink ͨ�� WM_ADBLOCK_NOTIFY ��Ϣ֪ͨ������ */
	LRESULT OnAdblockNotify(UINT uMsg, WPARAM wParam , LPARAM lParam, BOOL& bHandled);

	/** CoralNPObject ����һЩ����ͨ�� WM_COMMAND ֪ͨ����ִ�� */
	LRESULT OnCommand(UINT uMsg, WPARAM wParam , LPARAM lParam, BOOL& bHandled);

	virtual void OnFinalMessage(HWND /*hWnd*/);

	BEGIN_SINK_MAP( CIEHostWindow )
		SINK_ENTRY(/*nID =*/ 1, DISPID_NEWWINDOW3, OnNewWindow3)
		SINK_ENTRY(/*nID =*/ 1, DISPID_DOCUMENTCOMPLETE, OnDocumentComplete )
		SINK_ENTRY(/*nID =*/ 1, DISPID_SETSECURELOCKICON, OnSetSecureLockIcon )
		SINK_ENTRY(/*nID =*/ 1, DISPID_PROGRESSCHANGE, OnProgressChange )
		SINK_ENTRY(/*nID =*/ 1, DISPID_TITLECHANGE, OnTitleChange)
		SINK_ENTRY(/*nID =*/ 1, DISPID_STATUSTEXTCHANGE, OnStatusTextChange)
		SINK_ENTRY(/*nID =*/ 1, DISPID_BEFORENAVIGATE2, OnBeforeNavigate2)
		SINK_ENTRY(/*nID =*/ 1, DISPID_COMMANDSTATECHANGE, OnCommandStateChange)
	END_SINK_MAP()

	STDMETHOD_(void, OnBeforeNavigate2)(IDispatch *pDisp, VARIANT *url, VARIANT *Flags, VARIANT *TargetFrameName, VARIANT *PostData, VARIANT *Headers, VARIANT_BOOL *Cancel );
	STDMETHOD_(void, OnNewWindow3)(IDispatch **ppDisp, VARIANT_BOOL *Cancel, DWORD dwFlags, BSTR bstrUrlContext, BSTR bstrUrl);
	STDMETHOD_(void, OnDocumentComplete)( IDispatch *, VARIANT* );
	STDMETHOD_(void, OnSetSecureLockIcon)( long SecureLockIcon );
	STDMETHOD_(void, OnProgressChange)(long nProgress,long nProgressMax);
	STDMETHOD_(void, OnTitleChange)(BSTR Text);
	STDMETHOD_(void, OnStatusTextChange)(BSTR Text);
	STDMETHOD_(void, OnCommandStateChange)(long lCommand, VARIANT_BOOL vbEnable);

public:

	/** ���� Firefox �� User-Agent */
	LPCTSTR nsUserAgent();

	/** ���� CIEHostWindow �� HWND Ѱ�Ҷ�Ӧ�� CIEHostWindow ���� */
	static CIEHostWindow * FromHwnd( HWND hwnd );

	/** ���� URL Ѱ�Ҷ�Ӧ�� CIEHostWindow ������� URL Ϊ�գ��򷵻����� CIEHostWindow �����еĵ�һ�� */
	static CIEHostWindow * FromUrl( LPCTSTR lpszUrl );

	VOID Assign( nsPluginInstance * pPluginInstance ) { m_pPluginInstance = pPluginInstance; }
	
	/** ���ָ��ҳ�� */
	bool Go( char * pszUrl, long flags );

private:

	friend class CHttpFilterSink;
	friend class IAxHostUIHandlerImpl;
	friend class CoralIETabNPObject;
	friend class nsPluginInstance;

	nsPluginInstance * m_pPluginInstance;

	bool CanGoBack;
	bool CanGoForward;

	long progress;
	bool closing;

	bool AutoSwitchBack;
	bool SyncUserAgent;
	bool SyncCookies;

	/** ���ڼ��ص� URL. */
	CString m_strLoadingUrl;

	/** DIRTY FIX: NewWindow3 ���洴���� IE ���ڲ������� Referer */
	CString m_strUrlContext;

private:

	HRESULT CreateWebBrowser(HWND hParent, _U_RECT rc);

	/** ������������ǵĶ������ */
	HRESULT Connect(IWebBrowser2 * ptrWB);

	/** ������ǵĶ�����������Ĺ��� */
	HRESULT Disconnect();

	/** Ϊ���ܹ����� IE ���ڵļ�����Ϣ, �������� WH_GETMESSAGE */
	static LRESULT CALLBACK GetMsgProc( int code, WPARAM wParam, LPARAM lParam );
	/**
	 * Ϊ�˴����ڽ�������⣬�������� WH_CALLWNDPROCRET. Ϊʲô���� WH_GETMESSAGE ����һ���أ���Ϊ
	 * WH_GETMESSAGE ֻ�ܴ��� Message Queue �������Ϣ���� WM_SETFOCUS �� WM_KILLFOCUS ��ֱ�ӱ�����ϵͳ
	 * �� CallWndProc ��������.
	 */
	static LRESULT CALLBACK CallWndRetProc( int code, WPARAM wParam, LPARAM lParam );

	/** ��ο�ԵĹ��� */
	VOID MinimizeMemory();

	/** ���������õ� Firefox ������ */
	VOID HandOverFocus();

	/** "Internet Explorer_Server" ���ڵ� HWND */
	HWND m_hwndInternetExplorerServer;

	bool m_bSwitchBack;

	/** ����� Firefox �� User-Agent */
	static CString m_nsUserAgent;

	CComObject<IAxHostUIHandlerImpl> * m_pUIController;

	CAxWindow m_webBrowser;
};

