#pragma once

#include "../gecko-sdk/include/prtypes.h"

#include <string>

class nsPluginInstance;
class CoralIETabNPObject;

/**
 * �� Firefox ��ȡ���õķ�װ�ࡣ
 */
class nsConfigManager
{
private:
	
	friend class nsPluginInstance;

	/**
	 * ���캯�������� nsConfigManager �Ǵ����� nsPluginInstance ��ģ�����ֻ��
	 * nsPluginInstance �ܹ�����˶���
	 */
	nsConfigManager( nsPluginInstance * parent );

public:

	/** ��ʼ��һЩȫ�־�̬�� prefs */
	void Init();

public:

	typedef enum { OpenInNewIE = 0, OpenInNewFirefox = 1, OpenInFirefoxTab = 2 } OpenNewWindowMode;

	/** ���¶���һЩȫ��ͳһ�� prefs���������ɾ�̬������ֻ��Ҫ��ʼ��һ���Ժ󵽴������Զ�ȡ�ˣ���ʵ���൱����һ�� cache */
	static bool isClassicMode;

	static bool isCookieSyncEnabled;

	static bool isAdblockEnabled;

	static OpenNewWindowMode open_newwindow;

	/** �� Firefox 3.6.4 ��ʼ������ OOPP����������ڶ������������� */
	static bool isInIsolatedProcess;

public:

	/** ע�⣺���������߳��е������к��������� Firefox ����� */
	int getIntPref(const char * prefName, int defVal = 0);

	PRBool getBoolPref(const char * prefName, PRBool defVal = false);

	bool getStrPref(const char * prefName, std::string & strVal);

public:

	/** ���� adblockplus �� patterns.ini �ļ���ȫ·�� */
	std::string getAdblockPlusPatternsFile();

private:

	/**
	 * �� Firefox �ж�ȡ prefs �����ַ�����һ������ nsIPrefService ���������� Firefox 3.6.4 �� OOPP �ж�������
	 * ��һ����������ͨ�� NPObject ���ýű��������������˺��ߡ�
	 */
	CoralIETabNPObject * m_ScriptableNPObject;
};
