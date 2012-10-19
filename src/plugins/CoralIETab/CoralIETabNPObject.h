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

#pragma once

#include <string>

#include "../gecko-sdk/include/npapi.h"
#include "../gecko-sdk/include/npruntime.h"
#include "../gecko-sdk/include/nsStringAPI.h"

class ScriptablePluginObjectBase : public NPObject
{
public:
	
	ScriptablePluginObjectBase(NPP npp)	: m_npp(npp) {}
	virtual ~ScriptablePluginObjectBase() {}

	virtual void Invalidate();
	virtual bool HasMethod(NPIdentifier name);
	virtual bool Invoke(NPIdentifier name, const NPVariant *args,
		uint32_t argCount, NPVariant *result);
	virtual bool InvokeDefault(const NPVariant *args, uint32_t argCount,
		NPVariant *result);
	virtual bool HasProperty(NPIdentifier name);
	virtual bool GetProperty(NPIdentifier name, NPVariant *result);
	virtual bool SetProperty(NPIdentifier name, const NPVariant *value);
	virtual bool RemoveProperty(NPIdentifier name);
	virtual bool Enumerate(NPIdentifier **identifier, uint32_t *count);
	virtual bool Construct(const NPVariant *args, uint32_t argCount,
		NPVariant *result);

public:
	static void _Deallocate(NPObject *npobj);
	static void _Invalidate(NPObject *npobj);
	static bool _HasMethod(NPObject *npobj, NPIdentifier name);
	static bool _Invoke(NPObject *npobj, NPIdentifier name,
		const NPVariant *args, uint32_t argCount,
		NPVariant *result);
	static bool _InvokeDefault(NPObject *npobj, const NPVariant *args,
		uint32_t argCount, NPVariant *result);
	static bool _HasProperty(NPObject * npobj, NPIdentifier name);
	static bool _GetProperty(NPObject *npobj, NPIdentifier name,
		NPVariant *result);
	static bool _SetProperty(NPObject *npobj, NPIdentifier name,
		const NPVariant *value);
	static bool _RemoveProperty(NPObject *npobj, NPIdentifier name);
	static bool _Enumerate(NPObject *npobj, NPIdentifier **identifier,
		uint32_t *count);
	static bool _Construct(NPObject *npobj, const NPVariant *args,
		uint32_t argCount, NPVariant *result);

protected:
	NPP m_npp;
};

//////////////////////////////////////////////////////////////////////////

class nsPluginInstance;

class CoralIETabNPObject : public ScriptablePluginObjectBase
{
public:

	CoralIETabNPObject(NPP npp);
	virtual ~CoralIETabNPObject();

protected:

	static const int propertyCount;
	static const NPUTF8 * const propertyNames[];

	static const int methodCount;
	static const NPUTF8 * const methodNames[];

	int indexOfProperty(NPIdentifier name) const;
	int indexOfMethod(NPIdentifier name) const;

public:

	virtual bool HasMethod(NPIdentifier name);
	virtual bool HasProperty(NPIdentifier name);
	virtual bool GetProperty(NPIdentifier name, NPVariant *result);
	virtual bool Invoke(NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result);

public:

	void NewTab( const PRUnichar *url, PRInt32 flags );
	void NewWindow( const PRUnichar *url, const PRUnichar * name, const PRUnichar *features);
	void OnProgressChange(long progress);
	void OnSecurityChange( long security );
	void CloseTab();
	void notifyAdblock( PRInt32 aContentType, const char * url, PRInt32 aBlock);

	/** 调用 Firefox 脚本来读取 prefs 的封装函数 */
	bool getPref(const char * prefName, PRInt32 prefType, NPVariant & prefValue);

	/** 查询 Firefox 的 Directory Service 获取一些系统目录的信息 */
	std::string queryDirectoryService(const char * aPropName);

private:
	
	NPIdentifier *propertyIdentifiers;
	NPIdentifier *methodIdentifiers;

private:

	/**
	 * IE 的 Find 和 View Source 功能是通过 IHTMLDocument( IOleCommandTarget ) 的 Exec 方法
	 * 来实现的, 这里写一个通用的函数, 便于其它方法调用
	 *
	 * @param dwCmdID	IOleCommandTarget::Exec() 的 nCmdID 参数
	 */
	BOOL OleCmdTargetExec( DWORD dwCmdID );

	/**
	 * IE 的 Save As, Print, Copy, Paste 等功能是通过 IWebBrowser2 的 ExecWB 方法来实现的
	 * 这里也写一个通用函数, 便于其它方法调用
	 */
	BOOL ExecCmd(OLECMDID id);

	/** 测试 IE 的某个功能是否可用, 如 Refresh, Cut, Copy 等 */
	BOOL OleCmdEnabled( OLECMDID dwCmdID );

	/** 向 Firefox 发送消息事件 */
	void dispatchMessageEvent(const char * message);

	/** 内部使用的函数 */
	bool Navigate2(const NPVariant & url, PRInt32 flags);

private:

	friend class nsPluginInstance;

	/** CoralIETabNPObject 是从属于 nsPluginInstance 的，这里指定其从属的对象 */
	VOID SetParent(nsPluginInstance * parent);

	nsPluginInstance * m_parent;
};
