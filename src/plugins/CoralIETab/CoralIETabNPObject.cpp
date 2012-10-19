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
* The Original Code is Coral IE Tab.
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

/*
#include "../gecko-sdk/include/nsStringAPI.h"
#include "../gecko-sdk/include/nsCOMPtr.h"
#include "../gecko-sdk/include/nsServiceManagerUtils.h"
#include "../gecko-sdk/include/nsIDOMDocument.h"
#include "../gecko-sdk/include/nsIDOMDocumentEvent.h"
#include "../gecko-sdk/include/nsIDOMEvent.h"
#include "../gecko-sdk/include/nsIDOMEventTarget.h"
#include "../gecko-sdk/include/nsIDOMMessageEvent.h"
#include "../gecko-sdk/include/nsIWindowWatcher.h"
#include "../gecko-sdk/include/nsIDOMWindow.h"
*/

#include "../gecko-sdk/include/nsIPrefBranch.h"

#include <WinInet.h>

#include "nsConfigManager.h"
#include "IEHostWindow.h"
#include "nsPluginInstance.h"

#include "CoralIETabNPObject.h"

#pragma warning(disable:4267)

static const char * EXCEPTION_NO_SUCH_PROPERTY = "No such property";
static const char * EXCEPTION_NO_SUCH_METHOD = "No such method";
static const char * EXCEPTION_OUT_OF_MEMORY = "Out of memory";
static const char * EXCEPTION_INVALID_ARGS = "Invalid arguments";
static const char * EXCEPTION_CONVERT_FAILED = "String conversion failed";

//////////////////////////////////////////////////////////////////////////

#define DECLARE_NPOBJECT_CLASS_WITH_BASE(_class, ctor)                        \
	static NPClass s##_class##_NPClass = {                                        \
	NP_CLASS_STRUCT_VERSION_CTOR,                                               \
	ctor,                                                                       \
	ScriptablePluginObjectBase::_Deallocate,                                    \
	ScriptablePluginObjectBase::_Invalidate,                                    \
	ScriptablePluginObjectBase::_HasMethod,                                     \
	ScriptablePluginObjectBase::_Invoke,                                        \
	ScriptablePluginObjectBase::_InvokeDefault,                                 \
	ScriptablePluginObjectBase::_HasProperty,                                   \
	ScriptablePluginObjectBase::_GetProperty,                                   \
	ScriptablePluginObjectBase::_SetProperty,                                   \
	ScriptablePluginObjectBase::_RemoveProperty,                                \
	ScriptablePluginObjectBase::_Enumerate,                                     \
	ScriptablePluginObjectBase::_Construct                                      \
}

#define GET_NPOBJECT_CLASS(_class) &s##_class##_NPClass

void ScriptablePluginObjectBase::Invalidate()
{
}

bool ScriptablePluginObjectBase::HasMethod(NPIdentifier name)
{
	return false;
}

bool ScriptablePluginObjectBase::Invoke(NPIdentifier name, const NPVariant *args,
								   uint32_t argCount, NPVariant *result)
{
	return false;
}

bool ScriptablePluginObjectBase::InvokeDefault(const NPVariant *args,
										  uint32_t argCount, NPVariant *result)
{
	return false;
}

bool ScriptablePluginObjectBase::HasProperty(NPIdentifier name)
{
	return false;
}

bool ScriptablePluginObjectBase::GetProperty(NPIdentifier name, NPVariant *result)
{
	return false;
}

bool ScriptablePluginObjectBase::SetProperty(NPIdentifier name,
										const NPVariant *value)
{
	return false;
}

bool ScriptablePluginObjectBase::RemoveProperty(NPIdentifier name)
{
	return false;
}

bool ScriptablePluginObjectBase::Enumerate(NPIdentifier **identifier,
									  uint32_t *count)
{
	return false;
}

bool ScriptablePluginObjectBase::Construct(const NPVariant *args, uint32_t argCount,
									  NPVariant *result)
{
	return false;
}

// static
void ScriptablePluginObjectBase::_Deallocate(NPObject *npobj)
{
	// Call the virtual destructor.
	delete (ScriptablePluginObjectBase *)npobj;
}

// static
void ScriptablePluginObjectBase::_Invalidate(NPObject *npobj)
{
	((ScriptablePluginObjectBase *)npobj)->Invalidate();
}

// static
bool ScriptablePluginObjectBase::_HasMethod(NPObject *npobj, NPIdentifier name)
{
	return ((ScriptablePluginObjectBase *)npobj)->HasMethod(name);
}

// static
bool ScriptablePluginObjectBase::_Invoke(NPObject *npobj, NPIdentifier name,
									const NPVariant *args, uint32_t argCount,
									NPVariant *result)
{
	return ((ScriptablePluginObjectBase *)npobj)->Invoke(name, args, argCount, result);
}

// static
bool ScriptablePluginObjectBase::_InvokeDefault(NPObject *npobj,
										   const NPVariant *args,
										   uint32_t argCount,
										   NPVariant *result)
{
	return ((ScriptablePluginObjectBase *)npobj)->InvokeDefault(args, argCount, result);
}

// static
bool ScriptablePluginObjectBase::_HasProperty(NPObject * npobj, NPIdentifier name)
{
	return ((ScriptablePluginObjectBase *)npobj)->HasProperty(name);
}

// static
bool ScriptablePluginObjectBase::_GetProperty(NPObject *npobj, NPIdentifier name, NPVariant *result)
{
	return ((ScriptablePluginObjectBase *)npobj)->GetProperty(name, result);
}

// static
bool ScriptablePluginObjectBase::_SetProperty(NPObject *npobj, NPIdentifier name, const NPVariant *value)
{
	return ((ScriptablePluginObjectBase *)npobj)->SetProperty(name, value);
}

// static
bool ScriptablePluginObjectBase::_RemoveProperty(NPObject *npobj, NPIdentifier name)
{
	return ((ScriptablePluginObjectBase *)npobj)->RemoveProperty(name);
}

// static
bool ScriptablePluginObjectBase::_Enumerate(NPObject *npobj,
									   NPIdentifier **identifier,
									   uint32_t *count)
{
	return ((ScriptablePluginObjectBase *)npobj)->Enumerate(identifier, count);
}

// static
bool ScriptablePluginObjectBase::_Construct(NPObject *npobj, const NPVariant *args,
									   uint32_t argCount, NPVariant *result)
{
	return ((ScriptablePluginObjectBase *)npobj)->Construct(args, argCount, result);
}

//////////////////////////////////////////////////////////////////////////

#define COUNTNAMES(a,b,c) const int a::b = sizeof(a::c)/sizeof(NPUTF8 *)

static NPObject * AllocateCoralIETabNPObject(NPP npp, NPClass *aClass)
{
	return new CoralIETabNPObject(npp);
}

DECLARE_NPOBJECT_CLASS_WITH_BASE(CoralIETabNPObject, AllocateCoralIETabNPObject);

const NPUTF8 * const CoralIETabNPObject::propertyNames[] =
{
	"canClose",
	"canBack",
	"canForward",
	"canRefresh",
	"canStop",
	"progress",
	"url",
	"flags",
	"title",
	"canCut",
	"canCopy",
	"canPaste",
};
COUNTNAMES(CoralIETabNPObject,propertyCount,propertyNames);

enum CoralIETabNPObjectPropertyIds
{
	ID_PROPERTY_canClose = 0,
	ID_PROPERTY_canBack,
	ID_PROPERTY_canForward,
	ID_PROPERTY_canRefresh,
	ID_PROPERTY_canStop,
	ID_PROPERTY_progress,
	ID_PROPERTY_url,
	ID_PROPERTY_flags,
	ID_PROPERTY_title,
	ID_PROPERTY_canCut,
	ID_PROPERTY_canCopy,
	ID_PROPERTY_canPaste,
};

const NPUTF8 * const CoralIETabNPObject::methodNames[] =
{
	"goBack",
	"goForward",
	"navigate",
	"navigate2",
	"refresh",
	"stop",
	"saveAs",
	"print",
	"printPreview",
	"printSetup",
	"cut",
	"copy",
	"paste",
	"selectAll",
	"find",
	"viewSource",
	"focus",
	"displaySecurityInfo",
	"zoom",
	"handOverFocus"
};
COUNTNAMES(CoralIETabNPObject,methodCount,methodNames);

enum CoralIETabNPObjectMethodIds
{
	ID_METHOD_goBack,
	ID_METHOD_goForward,
	ID_METHOD_navigate,
	ID_METHOD_navigate2,
	ID_METHOD_refresh,
	ID_METHOD_stop,
	ID_METHOD_saveAs,
	ID_METHOD_print,
	ID_METHOD_printPreview,
	ID_METHOD_printSetup,
	ID_METHOD_cut,
	ID_METHOD_copy,
	ID_METHOD_paste,
	ID_METHOD_selectAll,
	ID_METHOD_find,
	ID_METHOD_viewSource,
	ID_METHOD_focus,
	ID_METHOD_displaySecurityInfo,
	ID_METHOD_zoom,
	ID_METHOD_handOverFocus
};

CoralIETabNPObject::CoralIETabNPObject(NPP npp) : ScriptablePluginObjectBase(npp), m_parent(NULL)
{
	propertyIdentifiers = new NPIdentifier[propertyCount];
	if( propertyIdentifiers )
	{
		for ( int i = 0; i < propertyCount; i++ )
		{
			propertyIdentifiers[i] = NPN_GetStringIdentifier(propertyNames[i]);
		}
	}

	methodIdentifiers = new NPIdentifier[methodCount];
	if( methodIdentifiers )
	{
		for ( int i = 0; i < methodCount; i++ )
		{
			methodIdentifiers[i] = NPN_GetStringIdentifier(methodNames[i]);
		}
	}
}

CoralIETabNPObject::~CoralIETabNPObject()
{
	if ( propertyIdentifiers ) delete[] propertyIdentifiers;
	if ( methodIdentifiers ) delete[] methodIdentifiers;
}

int CoralIETabNPObject::indexOfMethod(NPIdentifier name) const
{
	if( methodIdentifiers )
	{
		for(int i=0; i < methodCount; ++i )
		{
			if( name == methodIdentifiers[i] )
				return i;
		}
	}
	return -1;
}

int CoralIETabNPObject::indexOfProperty(NPIdentifier name) const
{
	if( propertyIdentifiers )
	{
		for(int i=0; i < propertyCount; ++i )
		{
			if( name == propertyIdentifiers[i] )
				return i;
		}
	}
	return -1;
}

bool CoralIETabNPObject::HasMethod(NPIdentifier name)
{
	return indexOfMethod(name) != -1;
}

bool CoralIETabNPObject::HasProperty(NPIdentifier name)
{
	return indexOfProperty(name) != -1;
}

bool CoralIETabNPObject::GetProperty(NPIdentifier name, NPVariant *result)
{
	NULL_TO_NPVARIANT(*result);

	CIEHostWindow * pIEHostWindow = m_parent ? m_parent->getIEHostWindow() : NULL;

	if ( pIEHostWindow && pIEHostWindow->IsWindow() )
	{
		switch (indexOfProperty(name))
		{
		case ID_PROPERTY_canClose:
			{
				BOOLEAN_TO_NPVARIANT(pIEHostWindow->closing, *result);
				break;
			}
		case ID_PROPERTY_canBack:
			{
				BOOLEAN_TO_NPVARIANT(pIEHostWindow->CanGoBack, *result);
				break;
			}
		case ID_PROPERTY_canForward:
			{
				BOOLEAN_TO_NPVARIANT(pIEHostWindow->CanGoForward, *result);
				break;
			}
		case ID_PROPERTY_canRefresh:
			{
				BOOLEAN_TO_NPVARIANT(OleCmdEnabled(OLECMDID_REFRESH), *result);
				break;
			}
		case ID_PROPERTY_canStop:
			{
				BOOLEAN_TO_NPVARIANT(pIEHostWindow->progress != -1, *result);
				break;
			}
		case ID_PROPERTY_progress:
			{
				INT32_TO_NPVARIANT(pIEHostWindow->progress, *result);
				break;
			}
		case ID_PROPERTY_url:
			{
				nsCString url_utf8;
				if ( NS_UTF16ToCString(nsDependentString((LPCWSTR)pIEHostWindow->m_strLoadingUrl), NS_CSTRING_ENCODING_UTF8, url_utf8) == NS_OK )
				{
					PRInt32 nLen = url_utf8.Length();
					if ( char * p = (char *)NPN_MemAlloc( nLen + 1) )
					{
						strncpy_s(p, nLen + 1, url_utf8.get(), nLen );
						// p[nLen] = 0;
						STRINGZ_TO_NPVARIANT(p, *result);
					}
					else
					{
						NPN_SetException(this, EXCEPTION_OUT_OF_MEMORY);
					}
				}
				else
				{
					NPN_SetException(this, EXCEPTION_CONVERT_FAILED);
				}

				break;
			}
		case ID_PROPERTY_flags:
			{
				INT32_TO_NPVARIANT(m_parent->flags, *result);
				break;
			}
		case ID_PROPERTY_title:
			{
				// 本来正统的方法是用 IWebBrowser2::get_LocationName() 的, 但是它有时候返回的 title 不对
				// 所以这里就用这个办法绕过去
				CComPtr<IDispatch> spDisp;
				if ( SUCCEEDED((*pIEHostWindow)->get_Document(&spDisp)) && spDisp )
				{
					CComQIPtr<IHTMLDocument2> spDoc(spDisp);
					if ( spDoc )
					{
						CComBSTR bstrTitle;
						if ( SUCCEEDED(spDoc->get_title( &bstrTitle )) )
						{
							nsCString title_utf8;
							if ( NS_UTF16ToCString(nsDependentString(bstrTitle), NS_CSTRING_ENCODING_UTF8, title_utf8) == NS_OK )
							{
								PRInt32 nLen = title_utf8.Length();
								if ( char * p = (char *)NPN_MemAlloc( nLen + 1) )
								{
									strncpy_s(p, nLen + 1, title_utf8.get(), nLen );
									// p[nLen] = 0;
									STRINGZ_TO_NPVARIANT(p, *result);
								}
								else
								{
									NPN_SetException(this, EXCEPTION_OUT_OF_MEMORY);
								}
							}
							else
							{
								NPN_SetException(this, EXCEPTION_CONVERT_FAILED);
							}
						}
					}
				}

				break;
			}
		case ID_PROPERTY_canCut:
			{
				BOOLEAN_TO_NPVARIANT(OleCmdEnabled(OLECMDID_CUT), *result);
				break;
			}
		case ID_PROPERTY_canCopy:
			{
				BOOLEAN_TO_NPVARIANT(OleCmdEnabled(OLECMDID_COPY), *result);
				break;
			}
		case ID_PROPERTY_canPaste:
			{
				BOOLEAN_TO_NPVARIANT(OleCmdEnabled(OLECMDID_PASTE), *result);
				break;
			}
		default:
			{
				NPN_SetException(this, EXCEPTION_NO_SUCH_PROPERTY);
				return false;
			}
		}
	}
	
	return true;
}

#define HTMLID_FIND			1
#define HTMLID_VIEWSOURCE	2
#define SHDVID_SSLSTATUS	33

bool CoralIETabNPObject::Invoke(NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
	NULL_TO_NPVARIANT(*result);

	CIEHostWindow * pIEHostWindow = m_parent ? m_parent->getIEHostWindow() : NULL;

	if ( pIEHostWindow && pIEHostWindow->IsWindow() )
	{
		switch (indexOfMethod(name))
		{
		case ID_METHOD_goBack:
			{
				(*pIEHostWindow)->GoBack();

				break;
			}
		case ID_METHOD_goForward:
			{
				(*pIEHostWindow)->GoForward();

				break;
			}
		case ID_METHOD_navigate:
			{
				if ( ( 1 == argCount ) && NPVARIANT_IS_STRING(args[0]) )
				{
					Navigate2(args[0], 0);
				}
				else
				{
					NPN_SetException(this, EXCEPTION_INVALID_ARGS);
					return false;
				}

				break;
			}
		case ID_METHOD_navigate2:
			{
				if ( ( 2 == argCount ) && NPVARIANT_IS_INT32(args[1]) )
				{
					Navigate2(args[0], NPVARIANT_TO_INT32(args[1]));
				}
				else
				{
					NPN_SetException(this, EXCEPTION_INVALID_ARGS);
					return false;
				}

				break;
			}
		case ID_METHOD_refresh:
			{
				(*pIEHostWindow)->Refresh();
				break;
			}
		case ID_METHOD_stop:
			{
				(*pIEHostWindow)->Stop();
				break;
			}
		case ID_METHOD_saveAs:
			{
				if ( nsConfigManager::isInIsolatedProcess )
				{
					// 从 Firefox 3.6.4 开始引入了 OOPP，我们不能阻塞脚本操作，否则 plugin 会被杀掉
					// 所以用 PostMessage() 先让本方法返回，后面再处理
					pIEHostWindow->PostMessage(WM_COMMAND, MAKEWPARAM(OLECMDID_SAVEAS, CMD_SIG_EXECWB), 0);
				}
				else
				{
					ExecCmd(OLECMDID_SAVEAS);
				}
				
				break;
			}
		case ID_METHOD_print:
			{
				// 同上
				if ( nsConfigManager::isInIsolatedProcess )
				{
					pIEHostWindow->PostMessage(WM_COMMAND, MAKEWPARAM(OLECMDID_PRINT, CMD_SIG_EXECWB), 0);
				}
				else
				{
					ExecCmd(OLECMDID_PRINT);
				}

				break;
			}
		case ID_METHOD_printPreview:
			{
				// 同上
				if ( nsConfigManager::isInIsolatedProcess )
				{
					pIEHostWindow->PostMessage(WM_COMMAND, MAKEWPARAM(OLECMDID_PRINTPREVIEW, CMD_SIG_EXECWB), 0);
				}
				else
				{
					ExecCmd(OLECMDID_PRINTPREVIEW);
				}
				break;
			}
		case ID_METHOD_printSetup:
			{
				// 同上
				if ( nsConfigManager::isInIsolatedProcess )
				{
					pIEHostWindow->PostMessage(WM_COMMAND, MAKEWPARAM(OLECMDID_PRINT, CMD_SIG_EXECWB), 0);
				}
				else
				{
					ExecCmd(OLECMDID_PRINT);
				}
				break;
			}
		case ID_METHOD_cut:
			{
				ExecCmd( OLECMDID_CUT );
				break;
			}
		case ID_METHOD_copy:
			{
				ExecCmd( OLECMDID_COPY );
				break;
			}
		case ID_METHOD_paste:
			{
				ExecCmd( OLECMDID_PASTE );
				break;
			}
		case ID_METHOD_selectAll:
			{
				ExecCmd( OLECMDID_SELECTALL );
				break;
			}
		case ID_METHOD_find:
			{
				// 同上
				if ( nsConfigManager::isInIsolatedProcess )
				{
					pIEHostWindow->PostMessage(WM_COMMAND, MAKEWPARAM(HTMLID_FIND, CMD_SIG_CMDTARGET), 0);
				}
				else
				{
					OleCmdTargetExec(HTMLID_FIND);
				}
				break;
			}
		case ID_METHOD_viewSource:
			{
				OleCmdTargetExec(HTMLID_VIEWSOURCE);
				break;
			}
		case ID_METHOD_focus:
			{
				CComPtr< IDispatch > spDisp;
				if ( SUCCEEDED( (*pIEHostWindow)->get_Document( & spDisp )) && spDisp )
				{
					CComQIPtr< IHTMLDocument4 > spDoc(spDisp);
					if ( spDoc )
					{
						spDoc->focus();
					}
				}

				break;
			}
		case ID_METHOD_displaySecurityInfo:
			{
				IOleCommandTarget *pct;
				if (SUCCEEDED((*pIEHostWindow)->QueryInterface(IID_IOleCommandTarget, (void **)&pct)))
				{
					pct->Exec(&CGID_ShellDocView, SHDVID_SSLSTATUS, 0, NULL, NULL);
					pct->Release();
				}

				break;
			}
		case ID_METHOD_zoom:
			{
				if ( argCount >= 1 )
				{
					double zoomLevel = 1;
					if ( NPVARIANT_IS_DOUBLE(args[0]) ) zoomLevel = NPVARIANT_TO_DOUBLE(args[0]);
					else if ( NPVARIANT_IS_INT32(args[0]) ) zoomLevel = NPVARIANT_TO_INT32(args[0]);
					if ( zoomLevel > 0 )
					{
						int nZoomLevel = (int)(zoomLevel * 100 + 0.5);

						CComVariant vZoomLevel(nZoomLevel);

						static const int OLECMDID_OPTICAL_ZOOM = 63;		// IE7+
						if ( FAILED( (*pIEHostWindow)->ExecWB( (OLECMDID)OLECMDID_OPTICAL_ZOOM, OLECMDEXECOPT_DONTPROMPTUSER, &vZoomLevel, NULL ) ) )
						{
							// IE6 只支持文字缩放, 最小为0, 最大为4, 默认为2

							int nLegecyZoomLevel = (int)((zoomLevel - 0.8) * 10 + 0.5);
							nLegecyZoomLevel = nLegecyZoomLevel < 0 ? 0 : nLegecyZoomLevel;
							nLegecyZoomLevel = nLegecyZoomLevel > 4 ? 4 : nLegecyZoomLevel;

							vZoomLevel.intVal = nLegecyZoomLevel;
							(*pIEHostWindow)->ExecWB( OLECMDID_ZOOM, OLECMDEXECOPT_DONTPROMPTUSER, &vZoomLevel, NULL );
						}
					}
				}

				break;
			}
		case ID_METHOD_handOverFocus:
			{
				// Bug #23763: IE 会抢 FF 的焦点，导致地址栏无法输入
				// 这是因为 FF 对 plugin 焦点的管理还有问题，暂时的解决办法就是让 plugin 自己把焦点交出来吧
				pIEHostWindow->HandOverFocus();
				break;
			}
		default:
			{
				NPN_SetException(this, EXCEPTION_NO_SUCH_METHOD);
				return false;
			}
		}
	}
	
	return true;
}

//////////////////////////////////////////////////////////////////////////

void CoralIETabNPObject::NewTab( const PRUnichar *url, PRInt32 flags )
{
	CStringW strMessage;
	strMessage.Format(L"{\"message\":\"newTab\", \"url\":\"%s\", \"flags\":%d }", url, flags);

	CW2A szMsg(strMessage);
	dispatchMessageEvent(szMsg);
}

void CoralIETabNPObject::NewWindow( const PRUnichar *url, const PRUnichar * name, const PRUnichar *features)
{
	CStringW strMessage;
	strMessage.Format(L"{\"message\":\"openNewWindow\", \"url\":\"%s\", \"name\":\"%s\", \"features\":\"%s\" }", url, name, features);

	CW2A szMsg(strMessage);
	dispatchMessageEvent(szMsg);
}

void CoralIETabNPObject::OnProgressChange(long progress)
{
	CStringA strMessage;
	strMessage.Format("{ \"message\":\"onProgressChange\", \"progress\":%d }", progress);

	dispatchMessageEvent(strMessage);
}

void CoralIETabNPObject::OnSecurityChange( long security )
{
	CStringA strMessage;
	strMessage.Format("{ \"message\":\"onSecurityChange\", \"security\":%d	}", security);

	dispatchMessageEvent(strMessage);
}

void CoralIETabNPObject::CloseTab()
{
	dispatchMessageEvent("{ \"message\":\"closeIeTab\" }");
}

void CoralIETabNPObject::notifyAdblock( PRInt32 aContentType, const char * url, PRInt32 aBlock)
{
	
	CStringA strMessage;
	strMessage.Format("{ \"message\":\"notifyAdBlock\", \"contentType\":%d, \"url\":\"%s\", \"block\":%d }", aContentType, url, aBlock);

	dispatchMessageEvent(strMessage);
}

//////////////////////////////////////////////////////////////////////////

BOOL CoralIETabNPObject::OleCmdTargetExec( DWORD dwCmdID )
{
	BOOL b = FALSE;

	CIEHostWindow * pIEHostWindow = m_parent ? m_parent->getIEHostWindow() : NULL;

	if ( pIEHostWindow && pIEHostWindow->IsWindow() )
	{
		CComPtr< IDispatch > spDoc;
		if ( SUCCEEDED( (*pIEHostWindow)->get_Document( & spDoc ) ) && spDoc )
		{
			CComQIPtr< IOleCommandTarget, & IID_IOleCommandTarget > spCmd(spDoc);
			if ( spCmd )
			{
				static const IID CGID_IWebBrowser = { 0xed016940, 0xbd5b, 0x11cf, 0xba, 0x4e, 0x0, 0xc0, 0x4f, 0xd7, 0x08, 0x16 };

				spCmd->Exec( &CGID_IWebBrowser, dwCmdID, 0, NULL, NULL );

				b = TRUE;
			}
		}
	}

	return b;
}

BOOL CoralIETabNPObject::ExecCmd(OLECMDID dwCmdID)
{
	CIEHostWindow * pIEHostWindow = m_parent ? m_parent->getIEHostWindow() : NULL;

	if( pIEHostWindow && pIEHostWindow->IsWindow() )
	{
		return SUCCEEDED( (*pIEHostWindow)->ExecWB( dwCmdID, OLECMDEXECOPT_DODEFAULT, NULL, NULL ) );
	}

	return FALSE;
}

BOOL CoralIETabNPObject::OleCmdEnabled( OLECMDID dwCmdID )
{
	BOOL b = FALSE;

	CIEHostWindow * pIEHostWindow = m_parent ? m_parent->getIEHostWindow() : NULL;

	if ( pIEHostWindow && pIEHostWindow->IsWindow() )
	{
		OLECMDF ocf;
		if (SUCCEEDED((*pIEHostWindow)->QueryStatusWB( dwCmdID, & ocf )))
		{
			b = (ocf & (OLECMDF_ENABLED|OLECMDF_SUPPORTED)) != 0;
		}
	}

	return b;
}

bool CoralIETabNPObject::getPref(const char * prefName, PRInt32 prefType, NPVariant & prefValue)
{
	bool r = false;

	NPObject * window = NULL;
	NPVariant v_gIeTab;

	NULL_TO_NPVARIANT(v_gIeTab);

	do 
	{

		if ( NPN_GetValue( m_npp, NPNVWindowNPObject, &window ) != NPERR_NO_ERROR ) break;
		if ( ! window ) break;
		
		if ( ! NPN_GetProperty( m_npp, window, NPN_GetStringIdentifier("gIeTab"), &v_gIeTab ) ) break;
		if ( ! NPVARIANT_IS_OBJECT(v_gIeTab) ) break;

		NPObject * np_gIeTab = NPVARIANT_TO_OBJECT(v_gIeTab);
		if ( ! np_gIeTab ) break;

		NPIdentifier methodName;
		switch ( prefType )
		{
		case nsIPrefBranch::PREF_INT:
			methodName = NPN_GetStringIdentifier("getIntPref");
			break;
		case nsIPrefBranch::PREF_BOOL:
			methodName = NPN_GetStringIdentifier("getBoolPref");
			break;
		case nsIPrefBranch::PREF_STRING:
			methodName = NPN_GetStringIdentifier("getStrPref");
			break;
		}

		NPVariant args[2];
		STRINGZ_TO_NPVARIANT(prefName, args[0]);
		args[1] = prefValue;

		r = NPN_Invoke( m_npp, np_gIeTab, methodName, args, 2, &prefValue);
		NPN_ReleaseObject(np_gIeTab);

	} while (false);

	if ( window ) NPN_ReleaseObject(window);
	if ( ! NPVARIANT_IS_NULL(v_gIeTab) ) NPN_ReleaseVariantValue(&v_gIeTab);

	return r;
}

std::string CoralIETabNPObject::queryDirectoryService(const char * aPropName)
{
	std::string result;

	NPObject * window = NULL;
	NPVariant v_gIeTab;

	NULL_TO_NPVARIANT(v_gIeTab);

	do 
	{

		if ( NPN_GetValue( m_npp, NPNVWindowNPObject, &window ) != NPERR_NO_ERROR ) break;
		if ( ! window ) break;

		if ( ! NPN_GetProperty( m_npp, window, NPN_GetStringIdentifier("gIeTab"), &v_gIeTab ) ) break;
		if ( ! NPVARIANT_IS_OBJECT(v_gIeTab) ) break;

		NPObject * np_gIeTab = NPVARIANT_TO_OBJECT(v_gIeTab);
		if ( ! np_gIeTab ) break;

		NPIdentifier methodName = NPN_GetStringIdentifier("queryDirectoryService");

		NPVariant args[1];
		STRINGZ_TO_NPVARIANT(aPropName, args[0]);

		NPVariant r;

		bool b = NPN_Invoke( m_npp, np_gIeTab, methodName, args, 1, &r);
		NPN_ReleaseObject(np_gIeTab);

		if (!b) break;
		if ( ! NPVARIANT_IS_STRING(r) ) break;

		NPString s = NPVARIANT_TO_STRING(r);

		result = s.UTF8Characters;

	} while (false);

	if ( window ) NPN_ReleaseObject(window);
	if ( ! NPVARIANT_IS_NULL(v_gIeTab) ) NPN_ReleaseVariantValue(&v_gIeTab);

	return result;
}

void CoralIETabNPObject::dispatchMessageEvent(const char * message)
{
	NPObject * window = NULL;
	NPObject * npDoc = NULL;
	NPObject * npEvt = NULL;
	NPVariant vDoc;
	NPVariant evt;

	NULL_TO_NPVARIANT(vDoc);
	NULL_TO_NPVARIANT(evt);

	do 
	{
		
		if ( NPN_GetValue( m_npp, NPNVWindowNPObject, &window ) != NPERR_NO_ERROR ) break;
		if ( ! window ) break;
		
		if ( ! NPN_GetProperty( m_npp, window, NPN_GetStringIdentifier("document"), &vDoc ) ) break;
		if ( ! NPVARIANT_IS_OBJECT(vDoc) ) break;

		npDoc = NPVARIANT_TO_OBJECT(vDoc);
		if ( ! npDoc ) break;

		// evt = document.createEvent("MessageEvent");
		NPVariant eventArgs[1];
		STRINGZ_TO_NPVARIANT("MessageEvent", eventArgs[0]);
		bool b = NPN_Invoke( m_npp, npDoc, NPN_GetStringIdentifier("createEvent"), eventArgs, 1, &evt);
		if ( ! b ) break;
		if ( ! NPVARIANT_IS_OBJECT(evt) ) break;

		// evt.initMessageEvent("IETabNotify", true, true, message, "chrome://coralietab/", "0", window)
		NPVariant result;
		npEvt = NPVARIANT_TO_OBJECT(evt);
		NPVariant initArgs[7];
		STRINGZ_TO_NPVARIANT("IETabNotify", initArgs[0]);
		BOOLEAN_TO_NPVARIANT(true, initArgs[1]);
		BOOLEAN_TO_NPVARIANT(true, initArgs[2]);
		STRINGZ_TO_NPVARIANT(message, initArgs[3]);
		STRINGZ_TO_NPVARIANT("chrome://coralietab/", initArgs[4]);
		STRINGZ_TO_NPVARIANT("0", initArgs[5]);
		OBJECT_TO_NPVARIANT(window, initArgs[6]);
		b = NPN_Invoke( m_npp, npEvt, NPN_GetStringIdentifier("initMessageEvent"), initArgs, 7, &result );
		NPN_ReleaseVariantValue(&result);
		if ( ! b ) break;

		// document.dispatchEvent(evt);
		NPVariant dispatchArgs[1];
		dispatchArgs[0] = evt;
		NPN_Invoke( m_npp, npDoc, NPN_GetStringIdentifier("dispatchEvent"), dispatchArgs, 1, &result );
		NPN_ReleaseVariantValue(&result);


		/* 下面这种方法的问题是，在 Firefox 3.6.4 以后的版本中，如果开启了 dom.ipc.plugin.enabled（即 OOPP ），
		   因为 plugin 运行在独立的进程中，window watcher 或者 window mediator 所管理的窗口都为空, 因此无法
		   取到可以发送的 DOM window，只能调用 NPAPI
		 */
		/*
		nsCOMPtr<nsIWindowWatcher> wwatcher(do_GetService("@mozilla.org/embedcomp/window-watcher;1"));
		if ( ! wwatcher ) break;

		nsCOMPtr<nsIDOMWindow> aWindow;
		if (NS_FAILED(wwatcher->GetActiveWindow(getter_AddRefs(aWindow)))) break;
		if ( ! aWindow ) break;

		nsCOMPtr<nsIDOMDocument> aDoc;
		if ( NS_FAILED(aWindow->GetDocument(getter_AddRefs(aDoc))) ) break;
		if ( ! aDoc ) break;

		nsCOMPtr<nsIDOMDocumentEvent> docEvent(do_QueryInterface(aDoc));
		if ( ! docEvent ) break;

		nsCOMPtr<nsIDOMEvent> event;
		if ( NS_FAILED(docEvent->CreateEvent(NS_LITERAL_STRING("MessageEvent"), getter_AddRefs(event))) ) break;
		if ( ! event ) break;

		nsCOMPtr<nsIDOMMessageEvent> messageEvent(do_QueryInterface(event));
		if ( ! messageEvent ) break;

		if ( NS_FAILED(messageEvent->InitMessageEvent(NS_LITERAL_STRING("IETabNotify"), PR_TRUE, PR_TRUE, message, NS_LITERAL_STRING("chrome://coralietab/"), NS_LITERAL_STRING("0"), aWindow)) ) break;

		nsCOMPtr<nsIDOMEventTarget> eventTarget(do_QueryInterface(aDoc));
		if ( ! eventTarget ) break;

		PRBool defaultActionEnabledWin = PR_FALSE;
		eventTarget->DispatchEvent(event, &defaultActionEnabledWin);
		*/
		
	} while (false);

	if ( window ) NPN_ReleaseObject(window);
	if ( npDoc ) NPN_ReleaseObject(npDoc);
	if ( npEvt ) NPN_ReleaseObject(npEvt);
	if ( ! NPVARIANT_IS_NULL(vDoc)) NPN_ReleaseVariantValue(&vDoc);
	if ( ! NPVARIANT_IS_NULL(evt)) NPN_ReleaseVariantValue(&evt);
}

bool CoralIETabNPObject::Navigate2(const NPVariant & url, PRInt32 flags)
{
	CIEHostWindow * pIEHostWindow = m_parent ? m_parent->getIEHostWindow() : NULL;
	if ( pIEHostWindow && pIEHostWindow->IsWindow() )
	{
		char * p = NULL;
		if ( NPVARIANT_IS_STRING(url) )
		{
			NPString s = NPVARIANT_TO_STRING(url);
			if ( s.UTF8Length < INTERNET_MAX_URL_LENGTH )
			{
				if ( p = (char *)NPN_MemAlloc(s.UTF8Length + 1) )
				{
					strncpy_s(p, s.UTF8Length+1, s.UTF8Characters, s.UTF8Length);
					// p[s.UTF8Length] = 0;
				}
			}
		}

		bool b = pIEHostWindow->Go(p, flags);

		if (p) NPN_MemFree(p);

		return b;
	}
	else
	{
		return false;
	}
}

VOID CoralIETabNPObject::SetParent(nsPluginInstance * parent)
{
	m_parent = parent;
}

//////////////////////////////////////////////////////////////////////////

CoralIETabNPObject * nsPluginInstance::getScriptObject()
{
	if ( ! m_pScriptableNPObject )
	{
		m_pScriptableNPObject = (CoralIETabNPObject *)NPN_CreateObject(m_nppInstance, GET_NPOBJECT_CLASS(CoralIETabNPObject));

		if ( m_pScriptableNPObject ) m_pScriptableNPObject->SetParent(this);
	}

	return m_pScriptableNPObject;
}