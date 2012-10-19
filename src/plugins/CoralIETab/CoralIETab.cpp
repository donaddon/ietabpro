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

#include "stdafx.h"

#include "../gecko-sdk/include/nscore.h"
#include "../gecko-sdk/include/npapi.h"
#include "../gecko-sdk/include/npfunctions.h"

#include "nsPluginInstance.h"

CComModule _Module;

extern VOID UnloadAPP();

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch ( ul_reason_for_call )
	{
	case DLL_PROCESS_ATTACH:
		{
			_Module.Init( NULL, hModule );
			DisableThreadLibraryCalls(hModule);

			/*
			::MessageBox( NULL, _T("INIT"), _T("CoralIETab"), MB_OK );
			*/

			break;
		}
	case DLL_PROCESS_DETACH:
		{
			// 注销 Asynchronous Pluggable Protocols, 否则 Firefox 可能会崩溃
			UnloadAPP();

			_Module.Term();

			break;
		}
	}
    return TRUE;
}

// 下面是 Firefox 的 NPAPI 要求的 3 个导出函数

NPError WINAPI NP_GetEntryPoints(NPPluginFuncs* pFuncs)
{
	if(pFuncs == NULL) return NPERR_INVALID_FUNCTABLE_ERROR;

	if(pFuncs->size < sizeof(NPPluginFuncs)) return NPERR_INVALID_FUNCTABLE_ERROR;

	// 这是告诉浏览器可以调用的插件的函数
	pFuncs->version			= (NP_VERSION_MAJOR << 8) | NP_VERSION_MINOR;
	pFuncs->newp			= NPP_New;
	pFuncs->destroy			= NPP_Destroy;
	pFuncs->setwindow		= NPP_SetWindow;
	pFuncs->newstream		= NPP_NewStream;
	pFuncs->destroystream	= NPP_DestroyStream;
	pFuncs->asfile			= /*NPP_StreamAsFile*/nsnull;
	pFuncs->writeready		= /*NPP_WriteReady*/nsnull;
	pFuncs->write			= /*NPP_Write*/nsnull;
	pFuncs->print			= /*NPP_Print*/nsnull;
	pFuncs->event			= /*NPP_HandleEvent*/nsnull;
	pFuncs->urlnotify		= /*NPP_URLNotify*/nsnull;
	pFuncs->getvalue		= NPP_GetValue;
	pFuncs->setvalue		= /*NPP_SetValue*/nsnull;
	pFuncs->javaClass		= nsnull;

	return NPERR_NO_ERROR;
}

NPNetscapeFuncs * gNPNFuncs = NULL;

NPError WINAPI NP_Initialize(NPNetscapeFuncs* pFuncs)
{
	if(pFuncs == NULL) return NPERR_INVALID_FUNCTABLE_ERROR;

	if(HIBYTE(pFuncs->version) > NP_VERSION_MAJOR) return NPERR_INCOMPATIBLE_VERSION_ERROR;

	if(pFuncs->size < sizeof(NPNetscapeFuncs))	return NPERR_INVALID_FUNCTABLE_ERROR;

	// 这是获得插件可以调用的浏览器的函数的函数指针
	gNPNFuncs = pFuncs;

	return NPP_Initialize();
}

NPError WINAPI NP_Shutdown()
{
	NPP_Shutdown();

	return NPERR_NO_ERROR;
}

//////////////////////////////////////////////////////////////////////////

NPError NPP_New(NPMIMEType pluginType, NPP instance, uint16 mode, int16 argc, char* argn[], char* argv[], NPSavedData* saved)
{
	if ( instance == NULL ) return NPERR_INVALID_INSTANCE_ERROR;

	// <embed id="IETab" type="application/coralietab" style="width:100%;height:100%"></embed>
	nsNPPNewParams params;
	params.instance = instance;
	params.type     = pluginType;
	params.mode     = mode; 
	params.argc     = argc;		// = 3
	params.argn     = argn;		// argn[0] = "id", argn[1] = "type", argn[2] = "style"
	params.argv     = argv;		// argv[0] = "IETab", argv[1] = "application/coralietab", argv[2] = "width:100%; height:100%;"
	params.saved    = saved;

	nsPluginInstance * plugin = new nsPluginInstance( & params );
	if ( plugin == NULL ) return NPERR_OUT_OF_MEMORY_ERROR;

	instance->pdata = (void *)plugin;

	NPN_SetValue(instance, NPPVpluginWindowBool, (void*)TRUE);

	return NPERR_NO_ERROR;
}

NPError NPP_Destroy (NPP instance, NPSavedData** save)
{
	if ( instance == NULL ) return NPERR_INVALID_INSTANCE_ERROR;

	nsPluginInstance * plugin = (nsPluginInstance *)instance->pdata;
	if ( plugin != NULL )
	{
		plugin->shutdown();
		
		delete plugin;
	}
	
	return NPERR_NO_ERROR;
}

NPError NPP_SetWindow (NPP instance, NPWindow* pNPWindow)
{
	if ( instance == NULL ) return NPERR_INVALID_INSTANCE_ERROR;

	if ( pNPWindow == NULL ) return NPERR_GENERIC_ERROR;

	nsPluginInstance * plugin = (nsPluginInstance *)instance->pdata;
	if(plugin == NULL) return NPERR_GENERIC_ERROR;

	if ( pNPWindow->window != NULL )
	{
		if ( ! plugin->isInitialized() )
		{
			// 初始化
			if ( ! plugin->init(pNPWindow) )
			{
				delete plugin;
				instance->pdata = NULL;

				return NPERR_MODULE_LOAD_FAILED_ERROR;
			}
		}
		else
		{
			// 窗口大小或位置变化了
			plugin->update(pNPWindow);
		}
	}

	return NPERR_NO_ERROR;
}

// Firefox 会通过此接口查询插件是否支持 JavaScript 接口
NPError	NPP_GetValue(NPP instance, NPPVariable variable, void *value)
{
	if ( instance == NULL ) return NPERR_INVALID_INSTANCE_ERROR;

	nsPluginInstance * plugin = (nsPluginInstance *)instance->pdata;
	if(plugin == NULL) return NPERR_GENERIC_ERROR;

	NPError rv = plugin->GetValue(variable, value);
	return rv;
}

// 这是 NPAPI Plugin 里面的, 但是目前 Firefox 还不支持, 留给 Chrome 和 Safari
/*
NPObject* NPP_GetScriptableInstance(NPP instance)
{
	NPObject * npobj = NULL;

	if ( instance )
	{
		nsPluginInstance * plugin = (nsPluginInstance *)instance->pdata;
		if ( plugin )
		{
			npobj = (NPObject *)plugin->getScriptObject();
		}
	}

	return npobj;
}

char * NP_GetMIMEDescription(void)
{
	return "application/coralietab";
}

NPError NP_GetValue(void* future, NPPVariable variable, void *value)
{
	return NPP_GetValue((NPP_t *)future, variable, value);
}
*/

// 下面两个函数虽然什么都不干, 但是还是需要, 因为 Firefox 有的时候会调用它（至少目前发现在 NPN_GetURL 调用后会）
// 如果没有实现这两个函数就会崩溃

NPError NPP_NewStream(NPP instance, NPMIMEType type, NPStream* stream, NPBool seekable, uint16* stype)
{
	return NPERR_NO_ERROR;
}

NPError NPP_DestroyStream (NPP instance, NPStream *stream, NPError reason)
{
	return NPERR_NO_ERROR;
}

/*
int32 NPP_WriteReady (NPP instance, NPStream *stream)
{
	return 0x0fffffff;
}

int32 NPP_Write (NPP instance, NPStream *stream, int32 offset, int32 len, void *buffer)
{
	return len;
}

void NPP_StreamAsFile (NPP instance, NPStream* stream, const char* fname)
{

}

void NPP_URLNotify(NPP instance, const char* url, NPReason reason, void* notifyData)
{

}

void NPP_Print (NPP instance, NPPrint* printInfo)
{

}

NPError NPP_SetValue(NPP instance, NPNVariable variable, void *value)
{
	return NPERR_NO_ERROR;
}

int16	NPP_HandleEvent(NPP instance, void* event)
{
	return NPERR_NO_ERROR;
}
*/

//////////////////////////////////////////////////////////////////////////

NPError NPP_Initialize( void )
{
	return NPERR_NO_ERROR;
}

void NPP_Shutdown( void )
{
}

//////////////////////////////////////////////////////////////////////////

NPError NPN_SetValue(NPP instance, NPPVariable variable, void *value)
{
	if ( gNPNFuncs )
	{
		NPError rv = gNPNFuncs->setvalue(instance, variable, value);
		return rv;
	}
	else
	{
		return NPERR_INVALID_FUNCTABLE_ERROR;
	}
}

NPError NPN_GetValue(NPP instance, NPNVariable variable, void *value)
{
	if ( gNPNFuncs )
	{
		NPError rv = gNPNFuncs->getvalue(instance, variable, value);
		return rv;
	}
	else
	{
		return NPERR_INVALID_FUNCTABLE_ERROR;
	}
}

void* NPN_MemAlloc(uint32_t size)
{
	void * rv = NULL;
	if ( gNPNFuncs )
	{
		rv = gNPNFuncs->memalloc(size);
	}
	return rv;
}

void NPN_MemFree(void* ptr)
{
	if ( gNPNFuncs )
	{
		gNPNFuncs->memfree(ptr);
	}
}

NPError NPN_GetURL(NPP instance, const char *url, const char *target)
{
	if ( gNPNFuncs )
	{
		NPError rv = gNPNFuncs->geturl(instance, url, target);
		return rv;
	}
	else
	{
		return NPERR_INVALID_FUNCTABLE_ERROR;
	}
}

void NPN_Status(NPP instance, const char *message)
{
	if ( gNPNFuncs )
	{
		gNPNFuncs->status(instance, message);
	}
}

const char* NPN_UserAgent(NPP instance)
{
	if ( gNPNFuncs )
	{
		return gNPNFuncs->uagent(instance);
	}
	else
	{
		return NULL;
	}
}

NPIdentifier NPN_GetStringIdentifier(const NPUTF8 *name)
{
	if ( gNPNFuncs )
	{
		return gNPNFuncs->getstringidentifier(name);
	}
	else
	{
		return nsnull;
	}
}

bool NPN_GetProperty(NPP npp, NPObject *npobj, NPIdentifier propertyName,
					 NPVariant *result)
{
	if ( gNPNFuncs )
	{
		return gNPNFuncs->getproperty(npp, npobj, propertyName, result);
	}
	else
	{
		return false;
	}
}
bool NPN_SetProperty(NPP npp, NPObject *npobj, NPIdentifier propertyName,
					 const NPVariant *value)
{
	if ( gNPNFuncs )
	{
		return gNPNFuncs->setproperty(npp, npobj, propertyName, value);
	}
	else
	{
		return false;
	}
}

bool NPN_Invoke(NPP npp, NPObject *npobj, NPIdentifier methodName,
				const NPVariant *args, uint32_t argCount, NPVariant *result)
{
	if ( gNPNFuncs )
	{
		return gNPNFuncs->invoke(npp, npobj, methodName, args, argCount, result);
	}
	else
	{
		return false;
	}
}

void NPN_ReleaseVariantValue(NPVariant *variant)
{
	if ( gNPNFuncs )
	{
		return gNPNFuncs->releasevariantvalue(variant);
	}
}

void NPN_ReleaseObject(NPObject *npobj)
{
	if ( gNPNFuncs )
	{
		return gNPNFuncs->releaseobject(npobj);
	}
}

void NPN_PluginThreadAsyncCall(NPP instance, void (*func) (void *), void *userData)
{
	if ( gNPNFuncs )
	{
		return gNPNFuncs->pluginthreadasynccall(instance, func, userData);
	}
}

NPObject *NPN_CreateObject(NPP npp, NPClass *aClass)
{
	if ( gNPNFuncs )
	{
		return gNPNFuncs->createobject(npp, aClass);
	}
	else
	{
		return nsnull;
	}
}

NPObject *NPN_RetainObject(NPObject *npobj)
{
	if ( gNPNFuncs )
	{
		return gNPNFuncs->retainobject(npobj);
	}
	else
	{
		return nsnull;
	}
}

void NPN_SetException(NPObject *npobj, const NPUTF8 *message)
{
	if ( gNPNFuncs )
	{
		return gNPNFuncs->setexception(npobj, message);
	}
}

/*
void NPN_GetStringIdentifiers(const NPUTF8 **names, int32_t nameCount, NPIdentifier *identifiers)
{
	if ( gNPNFuncs )
	{
		return gNPNFuncs->getstringidentifiers(names, nameCount, identifiers);
	}
}
*/

bool NPN_Evaluate(NPP npp, NPObject *npobj, NPString *script,
				  NPVariant *result)
{
	if ( gNPNFuncs )
	{
		return gNPNFuncs->evaluate(npp, npobj, script, result);
	}
	else
	{
		return false;
	}
}

NPError NPN_PostURL(NPP instance, const char* url,
	const char* target, uint32_t len,
	const char* buf, NPBool file)
{
	if ( gNPNFuncs )
	{
		return gNPNFuncs->posturl(instance, url, target, len, buf, file);
	}
	else
	{
		return NPERR_INVALID_FUNCTABLE_ERROR;
	}
}

/*
NPUTF8 *NPN_UTF8FromIdentifier(NPIdentifier identifier)
{
	if ( gNPNFuncs )
	{
		return gNPNFuncs->utf8fromidentifier(identifier);
	}
	else
	{
		return NULL;
	}
}
*/

NPError NPN_GetAuthenticationInfo(NPP instance,
	const char *protocol,
	const char *host, int32_t port,
	const char *scheme,
	const char *realm,
	char **username, uint32_t *ulen,
	char **password,
	uint32_t *plen)
{
	if ( gNPNFuncs )
	{
		return gNPNFuncs->getauthenticationinfo(instance, protocol, host, port, scheme, realm, username, ulen, password, plen);
	}
	else
	{
		return NPERR_INVALID_FUNCTABLE_ERROR;
	}
}

NPError NPN_GetValueForURL(NPP instance, NPNURLVariable variable,
	const char *url, char **value,
	uint32_t *len)
{
	if ( gNPNFuncs )
	{
		return gNPNFuncs->getvalueforurl(instance, variable, url, value, len);
	}
	else
	{
		return NPERR_INVALID_FUNCTABLE_ERROR;
	}
}

NPError NPN_SetValueForURL(NPP instance, NPNURLVariable variable,
	const char *url, const char *value,
	uint32_t len)
{
	if ( gNPNFuncs )
	{
		return gNPNFuncs->setvalueforurl(instance, variable, url, value, len);
	}
	else
	{
		return NPERR_INVALID_FUNCTABLE_ERROR;
	}
}