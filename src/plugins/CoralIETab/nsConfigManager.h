#pragma once

#include "../gecko-sdk/include/prtypes.h"

#include <string>

class nsPluginInstance;
class CoralIETabNPObject;

/**
 * 从 Firefox 读取设置的封装类。
 */
class nsConfigManager
{
private:
	
	friend class nsPluginInstance;

	/**
	 * 构造函数。由于 nsConfigManager 是从属于 nsPluginInstance 类的，所以只有
	 * nsPluginInstance 能够构造此对象。
	 */
	nsConfigManager( nsPluginInstance * parent );

public:

	/** 初始化一些全局静态的 prefs */
	void Init();

public:

	typedef enum { OpenInNewIE = 0, OpenInNewFirefox = 1, OpenInFirefoxTab = 2 } OpenNewWindowMode;

	/** 以下都是一些全局统一的 prefs，所以做成静态变量，只需要初始化一次以后到处都可以读取了，其实就相当于是一个 cache */
	static bool isClassicMode;

	static bool isCookieSyncEnabled;

	static bool isAdblockEnabled;

	static OpenNewWindowMode open_newwindow;

	/** 从 Firefox 3.6.4 开始引入了 OOPP，插件可能在独立进程中运行 */
	static bool isInIsolatedProcess;

public:

	/** 注意：必须在主线程中调用下列函数，否则 Firefox 会崩溃 */
	int getIntPref(const char * prefName, int defVal = 0);

	PRBool getBoolPref(const char * prefName, PRBool defVal = false);

	bool getStrPref(const char * prefName, std::string & strVal);

public:

	/** 返回 adblockplus 的 patterns.ini 文件的全路径 */
	std::string getAdblockPlusPatternsFile();

private:

	/**
	 * 从 Firefox 中读取 prefs 有两种方法，一种是用 nsIPrefService 读，但是在 Firefox 3.6.4 的 OOPP 中读不到；
	 * 另一种做法就是通过 NPObject 调用脚本来读。这里用了后者。
	 */
	CoralIETabNPObject * m_ScriptableNPObject;
};
