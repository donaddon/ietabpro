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

/** 用于 WM_COMMAND 消息 */
static const WORD CMD_SIG_EXECWB = 0xE0EC;
static const WORD CMD_SIG_CMDTARGET = 0xC3D7;

/** 我们通过这个类对 IE 的界面进行控制, 例如屏蔽脚本错误提示等 */
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
	 * 通常情况下, 我们都是用 IE 默认的弹出窗口过滤程序, 然后在 NewWindow3 事件中处理
	 * 然而, NewWindow3 无法得知 window.open() 调用时的参数, 例如: window.open("", "childWin", "width=320,height=240");
	 * 这个时候我们就要靠 pszFeatures 得到 "width=320,height=240"
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

	// 调用 Firefox 的 window.moveTo(), window.resizeBy() 等方法的通用方法
	STDMETHOD(window_call)(const char * methodName, LONG x, LONG y);

public:

	VOID Assign(CIEHostWindow * pParent) { m_pParent = pParent; }

private:

	friend CIEHostWindow;

	CIEHostWindow * m_pParent;

	/** 根据 EvaluateNewWindow 的参数判断是否让 IE 自己弹出窗口而不是我们用 Tab 接管 */
	BOOL m_bShouldOpenNewWindow;

	/** window.open(url, name, features) 的 name 参数 */
	CString m_strOpenNewWindowName;

	/** window.open(url, name, features) 的 features 参数 */
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

	/** IE 控件关闭时会收到 WM_PARENTNOTIFY 消息 */
	LRESULT OnParentNotify(UINT uMsg, WPARAM wParam , LPARAM lParam, BOOL& bHandled);

	/** 有广告被过滤时 HttpFilterSink 通过 WM_ADBLOCK_NOTIFY 消息通知主窗口 */
	LRESULT OnAdblockNotify(UINT uMsg, WPARAM wParam , LPARAM lParam, BOOL& bHandled);

	/** CoralNPObject 会有一些命令通过 WM_COMMAND 通知窗口执行 */
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

	/** 返回 Firefox 的 User-Agent */
	LPCTSTR nsUserAgent();

	/** 根据 CIEHostWindow 的 HWND 寻找对应的 CIEHostWindow 对象 */
	static CIEHostWindow * FromHwnd( HWND hwnd );

	/** 根据 URL 寻找对应的 CIEHostWindow 对象，如果 URL 为空，则返回所有 CIEHostWindow 对象中的第一个 */
	static CIEHostWindow * FromUrl( LPCTSTR lpszUrl );

	VOID Assign( nsPluginInstance * pPluginInstance ) { m_pPluginInstance = pPluginInstance; }
	
	/** 浏览指定页面 */
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

	/** 正在加载的 URL. */
	CString m_strLoadingUrl;

	/** DIRTY FIX: NewWindow3 里面创建的 IE 窗口不能设置 Referer */
	CString m_strUrlContext;

private:

	HRESULT CreateWebBrowser(HWND hParent, _U_RECT rc);

	/** 将浏览器与我们的对象关联 */
	HRESULT Connect(IWebBrowser2 * ptrWB);

	/** 解除我们的对象与浏览器的关联 */
	HRESULT Disconnect();

	/** 为了能够处理 IE 窗口的键盘消息, 我们用了 WH_GETMESSAGE */
	static LRESULT CALLBACK GetMsgProc( int code, WPARAM wParam, LPARAM lParam );
	/**
	 * 为了处理窗口焦点的问题，我们用了 WH_CALLWNDPROCRET. 为什么不和 WH_GETMESSAGE 放在一起呢？因为
	 * WH_GETMESSAGE 只能处理 Message Queue 里面的消息，而 WM_SETFOCUS 和 WM_KILLFOCUS 是直接被操作系统
	 * 用 CallWndProc 发过来的.
	 */
	static LRESULT CALLBACK CallWndRetProc( int code, WPARAM wParam, LPARAM lParam );

	/** 安慰性的功能 */
	VOID MinimizeMemory();

	/** 将焦点设置到 Firefox 主窗口 */
	VOID HandOverFocus();

	/** "Internet Explorer_Server" 窗口的 HWND */
	HWND m_hwndInternetExplorerServer;

	bool m_bSwitchBack;

	/** 缓存的 Firefox 的 User-Agent */
	static CString m_nsUserAgent;

	CComObject<IAxHostUIHandlerImpl> * m_pUIController;

	CAxWindow m_webBrowser;
};

