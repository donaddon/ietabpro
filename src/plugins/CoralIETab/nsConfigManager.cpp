#include "StdAfx.h"

#include <algorithm>

#include "../gecko-sdk/include/nsIPrefBranch.h"

#include "nsPluginInstance.h"
#include "CoralIETabNPObject.h"

#include "nsConfigManager.h"

bool nsConfigManager::isClassicMode = true;
bool nsConfigManager::isCookieSyncEnabled = false;
bool nsConfigManager::isAdblockEnabled = false;
nsConfigManager::OpenNewWindowMode nsConfigManager::open_newwindow = nsConfigManager::OpenInNewFirefox;
bool nsConfigManager::isInIsolatedProcess = true;

nsConfigManager::nsConfigManager( nsPluginInstance * parent )
{
	m_ScriptableNPObject = parent->getScriptObject();
}

void nsConfigManager::Init()
{
	// 这里初始化的都是一些全局静态的参数，所以只需要读取一次就够了
	static bool bOnce = true;

	if ( bOnce && m_ScriptableNPObject )
	{
		std::string mode;
		if ( this->getStrPref("coral.ietab.mode", mode) )
		{
			std::transform(mode.begin(), mode.end(), mode.begin(), ::tolower);
			nsConfigManager::isClassicMode = mode.compare("advanced") != 0;
		}

		nsConfigManager::isCookieSyncEnabled = (!isClassicMode) && this->getBoolPref("coral.ietab.cookieSync");
		nsConfigManager::isAdblockEnabled = (!isClassicMode) && this->getBoolPref("coral.ietab.adblock");
		nsConfigManager::open_newwindow = nsConfigManager::isClassicMode ? nsConfigManager::OpenInFirefoxTab : (nsConfigManager::OpenNewWindowMode)this->getIntPref("coral.ietab.open_newwindow", 1);

		// 检测 plugin 是不是被放到 plugin-container.exe 进程里面去了
		TCHAR szFilename[MAX_PATH];
		if ( GetModuleFileName( NULL, szFilename, ARRAYSIZE(szFilename)) > 0 )
		{
			PathStripPath(szFilename);
			nsConfigManager::isInIsolatedProcess = ( _tcsicmp( szFilename, _T("plugin-container.exe") ) == 0 );
		}

		bOnce = false;
	}
}

int nsConfigManager::getIntPref(const char * prefName, int defVal /*= 0*/)
{
	if ( m_ScriptableNPObject )
	{
		NPVariant value;
		NULL_TO_NPVARIANT(value);

		if ( m_ScriptableNPObject->getPref(prefName, nsIPrefBranch::PREF_INT, value) && NPVARIANT_IS_INT32(value) )
		{
			defVal = NPVARIANT_TO_INT32(value);
		}

		if ( ! NPVARIANT_IS_NULL(value) ) NPN_ReleaseVariantValue(&value);
	}

	return defVal;
}

PRBool nsConfigManager::getBoolPref(const char * prefName, PRBool defVal /*= false*/)
{
	if ( m_ScriptableNPObject )
	{
		NPVariant value;
		NULL_TO_NPVARIANT(value);

		if ( m_ScriptableNPObject->getPref(prefName, nsIPrefBranch::PREF_BOOL, value) && NPVARIANT_IS_BOOLEAN(value) )
		{
			defVal = NPVARIANT_TO_BOOLEAN(value);
		}

		if ( ! NPVARIANT_IS_NULL(value) ) NPN_ReleaseVariantValue(&value);
	}

	return defVal;
}

bool nsConfigManager::getStrPref(const char * prefName, std::string & strVal)
{
	bool b = false;

	if ( m_ScriptableNPObject )
	{
		NPVariant value;
		NULL_TO_NPVARIANT(value);
		
		if ( m_ScriptableNPObject->getPref(prefName, nsIPrefBranch::PREF_STRING, value) && NPVARIANT_IS_STRING(value) )
		{
			NPString s = NPVARIANT_TO_STRING(value);
			strVal = s.UTF8Characters;

			b = true;
		}

		if ( ! NPVARIANT_IS_NULL(value) ) NPN_ReleaseVariantValue(&value);
	}

	return b;
}

std::string nsConfigManager::getAdblockPlusPatternsFile()
{
	static std::string result;		// 用 static 使得只需要初始化一次就可以了

	do 
	{
		if ( ! result.empty() ) break;

		if ( ! m_ScriptableNPObject ) break;

		// 查询 Firefox 的 profile 目录
		std::string profileDir = m_ScriptableNPObject->queryDirectoryService("ProfD");
		if ( profileDir.empty() ) break;

		// 查询 Adblock Plus 设置的 patterns.ini 文件的目录
		std::string patternsfile;
		if ( ! nsConfigManager::getStrPref("extensions.adblockplus.patternsfile", patternsfile) ) break;

		char fullPath[MAX_PATH];
		PathCombineA(fullPath, profileDir.c_str(), patternsfile.c_str());

		result = fullPath;

	} while(false);

	return result;
}