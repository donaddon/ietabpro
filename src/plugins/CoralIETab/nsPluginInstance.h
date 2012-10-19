#pragma once

#include "../gecko-sdk/include/npapi.h"
#include "../gecko-sdk/include/prtypes.h"

#include "IEHostWindow.h"

/**
 * 用于向 nsPluginInstance 的构造函数传递参数的数据结构
 * 用 struct 而不是逐个传递每一个参数的好处是可以提高构造函数接口的可扩展性
 */
struct nsNPPNewParams
{
	NPP instance;
	NPMIMEType type; 
	PRUint16 mode; 
	int16 argc; 
	char** argn; 
	char** argv; 
	NPSavedData* saved;
};

class CoralIETabNPObject;
class nsConfigManager;

/** 这个类实际就是一个数据结构, 用于管理 NP Plugin 到 IE 控件的交互 */
class nsPluginInstance
{
public:
	nsPluginInstance(nsNPPNewParams * params);
	~nsPluginInstance();

public:

	NPBool isInitialized();

	NPBool init(NPWindow* aWindow);
	void shutdown();

	/** 当 NP 窗口大小或者位置改变时, 通过 NPP_SetWindow 通知 update */
	void update(NPWindow* aWindow);

	/** 供 NPP_GetValue() 调用, 在 Firefox 查询 JavaScript 接口时, 把接口传递给 Firefox */
	NPError GetValue(NPPVariable aVariable, void *aValue);
	
	/** 返回从属的 NPObject 封装对象 */
	CoralIETabNPObject * getScriptObject();

	/** 返回从属的 prefs 封装对象 */
	nsConfigManager * getConfigManager();

	/** 返回从属的 CIEHostWindow 对象 */
	CIEHostWindow * getIEHostWindow();

	/** 返回 Plugin NPP 指针 */
	NPP instance();

public:

	/** 插件传递进来的 flags */
	PRInt32 flags;

private:

	bool initLoadingUrl();

	bool getHostUrl(LPWSTR pszUrl, DWORD dwSize);

private:

	/** 是否已经初始化过了？ */
	NPBool m_bInitialized;

	/** Plugin 的实例 */
	NPP m_nppInstance;

	/** 插件加载的 URL */
	CString m_strLoadingUrl;

	/** IE 窗口控件 */
	CIEHostWindow * m_pIEHostWindow;

	/** 插件与 JavaScript 相互接口的 NPObject 对象 */
	CoralIETabNPObject * m_pScriptableNPObject;

	/** 读取 Firefox prefs 的封装对象 */
	nsConfigManager * m_configManager;
};
