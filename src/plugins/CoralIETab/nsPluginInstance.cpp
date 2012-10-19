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

#include <cstdlib>

#include <WinInet.h>
#include <comutil.h>

#include "../gecko-sdk/include/npapi.h"
#include "../gecko-sdk/include/npruntime.h"
#include "../gecko-sdk/include/nsStringAPI.h"
#include "../gecko-sdk/include/nsEmbedString.h"

#include "CoralIETabNPObject.h"
#include "nsConfigManager.h"
#include "Misc.h"
#include "nsPluginInstance.h"

nsPluginInstance::nsPluginInstance(nsNPPNewParams * params) : m_nppInstance( params->instance ),
m_bInitialized( false ),
flags( 0 ),
m_pScriptableNPObject( NULL ),
m_configManager( NULL ),
m_pIEHostWindow( NULL )
{
	// ͨ�������� TAG ���ݹ����� URL: <object id="IETab" type="application/coralietab" url="http://xxx" style="width:100%;height:100%">
	if( params->mode == NP_EMBED )
	{
		for ( int i = 0; i < params->argc; ++i )
		{
			if( 0 == _stricmp( "url", params->argn[i] ) )
				m_strLoadingUrl = params->argv[i];
		}
	}
}

nsPluginInstance::~nsPluginInstance()
{
}

#ifndef NS_STATIC_CAST
#define NS_STATIC_CAST(__type, __ptr)      static_cast< __type >(__ptr)
#endif

NPBool nsPluginInstance::isInitialized()
{
	return m_bInitialized;
}

NPBool nsPluginInstance::init(NPWindow* aWindow)
{
	if(aWindow == NULL)	return false;

	HWND npWnd = (HWND)aWindow->window;
	if ( ! ::IsWindow(npWnd) ) return false;

	::OleInitialize(NULL);

	// ͨ�� GWL_USERDATA �� nsPluginInstance ��ʵ��ָ�봫�� Browser ����ȥ
	// SetWindowLong( npWnd, GWL_USERDATA, (LONG)this );

	m_bInitialized = true;

	NPBool r = false;

	do
	{
		if ( m_strLoadingUrl.IsEmpty() ) if ( ! initLoadingUrl() ) break;

		if ( ! this->getScriptObject() ) break;

		nsConfigManager * configManager = this->getConfigManager();
		if ( ! configManager ) break;
		configManager->Init();

		RECT rc;
		GetWindowRect( npWnd, & rc );
		OffsetRect( & rc, -rc.left, -rc.top );

		// �μ� CIEHostWindow::OnNewWindow3()
		std::string result;
		CW2A szUrl(m_strLoadingUrl);
		if ( regexpr_match("^ietab:(\\d+)$", szUrl, true, &result, 1) )
		{
			HWND hwnd = (HWND)_atoi64(result.c_str());
			if ( ! IsWindow(hwnd) ) break;

			m_pIEHostWindow = CIEHostWindow::FromHwnd(hwnd);
			if ( ! m_pIEHostWindow ) break;

			m_pIEHostWindow->Assign(this);
			m_pIEHostWindow->MoveWindow(&rc);
			m_pIEHostWindow->SetParent(npWnd);

			r = true;
			break;
		}

		m_pIEHostWindow = CIEHostWindow::Create(npWnd, rc);
		if ( ! m_pIEHostWindow ) break;

		m_pIEHostWindow->Assign(this);

		m_pIEHostWindow->Go(szUrl, flags);

		// ����������, Firefox ���ڱ仯��ʱ��Ż�֪ͨ IE ����ˢ����ʾ
		SetWindowLong(npWnd, GWL_STYLE, GetWindowLong(npWnd, GWL_STYLE)|WS_CLIPCHILDREN);
		SetWindowLong(npWnd, GWL_EXSTYLE, GetWindowLong(npWnd, GWL_EXSTYLE)|WS_EX_CONTROLPARENT);

		r = true;

	} while(false);

	return r;
}

void nsPluginInstance::shutdown()
{
	if ( m_pIEHostWindow && m_pIEHostWindow->IsWindow() )
	{
		m_pIEHostWindow->DestroyWindow();
		// delete m_pIEHostWindow;			// CIEHostWindow ���Լ� delete
		m_pIEHostWindow = NULL;
	}

	if ( m_pScriptableNPObject )
	{
		NPN_ReleaseObject(m_pScriptableNPObject);
		m_pScriptableNPObject = NULL;
	}

	if ( m_configManager )
	{
		delete m_configManager;
		m_configManager = NULL;
	}

	CoFreeUnusedLibraries();

	Sleep(0);

	::OleUninitialize();
}

void nsPluginInstance::update(NPWindow* aWindow)
{
	if ( m_pIEHostWindow )
	{
		HWND npWnd = (HWND)aWindow->window;
		if ( IsWindow( npWnd ) && m_pIEHostWindow->IsWindow() )
		{
			RECT rc = { 0, 0, aWindow->width, aWindow->height };

			m_pIEHostWindow->MoveWindow( & rc );
		}
	}
}

NPError nsPluginInstance::GetValue(NPPVariable aVariable, void *aValue)
{
	switch ( aVariable )
	{
	case NPPVpluginScriptableNPObject:
		{
			if ( NPObject * pObject = getScriptObject() )
			{
				NPN_RetainObject(pObject);

				*(NPObject**)aValue = pObject;
			}

			break;
		}
	}
	
	return NPERR_NO_ERROR;
}

NPP nsPluginInstance::instance()
{
	return m_nppInstance;
}

bool nsPluginInstance::initLoadingUrl()
{
	// �ȵõ����� Firefox TAB �� URL, һ���� chrome://ietab/content/container.html?url=16874,about:blank ����
	WCHAR swWholeUrl[INTERNET_MAX_URL_LENGTH] = L"\0";
	getHostUrl(swWholeUrl, ARRAYSIZE(swWholeUrl));

	// ��������������� URL
	std::string results[2];
	CW2A szWholeUrl(swWholeUrl);
	if ( !regexpr_match("^chrome:.*\\?url=(\\d+),(\\S+)$", szWholeUrl, true, results, 2) )
	{
		return false;
	}

	// ��1��ƥ���Ǽ��ص� flags
	flags = atoi(results[0].c_str());

	if ( results[1].empty() )
	{
		return false;
	}

	// ��2��ƥ�����IEҪ���ʵ�URL
	CA2W swIEUrl(results[1].c_str());

	m_strLoadingUrl.Empty();

	URL_COMPONENTS uc;
	ZeroMemory( &uc, sizeof(uc) );
	uc.dwStructSize = sizeof(uc);
	if ( InternetCrackUrlW(swIEUrl, 0, 0, & uc ) )
	{
		if ( uc.nScheme == INTERNET_SCHEME_FILE )
		{
			// file:// Э���е�����, ���������, InternetCrackUrlW ������ lpszPath �Ǵ��, UrlUnescapeW �Ľ��Ҳ������, ���Ի���Ҫ�� ASCII
			CHAR szUnescapedUrl[INTERNET_MAX_URL_LENGTH] = "\0";
			DWORD dwSize = ARRAYSIZE(szUnescapedUrl);
			if (SUCCEEDED(UrlUnescapeA((LPSTR)results[1].c_str(), szUnescapedUrl, &dwSize, 0)))
			{
				// ��� UrlUnescapeA �����ľ�Ȼ�� UTF-8 �����, ֻ����תһ��
				WCHAR swUrl[INTERNET_MAX_URL_LENGTH] = L"\0";
				if ( MultiByteToWideChar( CP_UTF8, 0, szUnescapedUrl, -1, swUrl, ARRAYSIZE(swUrl) ) > 0 )
				{
					m_strLoadingUrl = swUrl;
				}
			}
		}
		else
		{
			if ( LPWSTR pUrl = (LPWSTR)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, INTERNET_MAX_URL_LENGTH * sizeof(WCHAR) ) )
			{
				DWORD dwLen = INTERNET_MAX_URL_LENGTH;
				if ( SUCCEEDED(UrlUnescapeW( swIEUrl, pUrl, & dwLen, 0)) )
				{
					m_strLoadingUrl = pUrl;
				}

				HeapFree(GetProcessHeap(), 0, pUrl);
			}
		}
	}
	else
	{
		if (  GetLastError() == ERROR_INTERNET_UNRECOGNIZED_SCHEME )
		{
			// ͨ����ԭ������Ϊ�������� url ����������ʽ: g.cn û�� http:// ��ǰ׺
			// ���ֻ�ǲ���ʶ���Э��, ���� afp://test/ ������ʽ InternetCrackUrlW �ǲ���ʧ�ܵ�
			// �������������һ�� http ��ǰ׺
			m_strLoadingUrl.Format(L"http://%s", swIEUrl);
		}
	}

	if ( m_strLoadingUrl.IsEmpty() )
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool nsPluginInstance::getHostUrl(LPWSTR pszUrl, DWORD dwSize)
{
	bool b = false;

	char * p = NULL;
	NPObject *object = NULL;
	NPVariant _location;
	NPVariant _href;

	do 
	{
		
		if (( NPN_GetValue( m_nppInstance, NPNVWindowNPObject, &object) != NPERR_NO_ERROR ) || !object ) break;

		if ((!NPN_GetProperty( m_nppInstance, object, NPN_GetStringIdentifier ("location"), &_location)) || !NPVARIANT_IS_OBJECT (_location)) break;

		if ((!NPN_GetProperty( m_nppInstance, NPVARIANT_TO_OBJECT(_location), NPN_GetStringIdentifier ("href"), &_href)) || !NPVARIANT_IS_STRING(_href)) break;

		const NPString url = NPVARIANT_TO_STRING(_href);

		p = (char *)NPN_MemAlloc(url.UTF8Length + 1);
		strncpy_s(p, url.UTF8Length+1, url.UTF8Characters, url.UTF8Length);
		// p[url.UTF8Length] = 0;

		if ( MultiByteToWideChar( CP_UTF8, 0, p, -1, pszUrl, dwSize ) == 0 ) break;

		b = true;

	} while(false);

	NPN_ReleaseVariantValue (&_location);
	NPN_ReleaseVariantValue (&_href);
	if (object) NPN_ReleaseObject(object);
	if (p) NPN_MemFree(p);

	return b;
}

nsConfigManager * nsPluginInstance::getConfigManager()
{
	if ( ! m_configManager )
	{
		m_configManager = new nsConfigManager(this);
	}

	return m_configManager;
}

CIEHostWindow * nsPluginInstance::getIEHostWindow()
{
	return m_pIEHostWindow;
}