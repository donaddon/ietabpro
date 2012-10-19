#pragma once

#include "../gecko-sdk/include/npapi.h"
#include "../gecko-sdk/include/prtypes.h"

#include "IEHostWindow.h"

/**
 * ������ nsPluginInstance �Ĺ��캯�����ݲ��������ݽṹ
 * �� struct �������������ÿһ�������ĺô��ǿ�����߹��캯���ӿڵĿ���չ��
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

/** �����ʵ�ʾ���һ�����ݽṹ, ���ڹ��� NP Plugin �� IE �ؼ��Ľ��� */
class nsPluginInstance
{
public:
	nsPluginInstance(nsNPPNewParams * params);
	~nsPluginInstance();

public:

	NPBool isInitialized();

	NPBool init(NPWindow* aWindow);
	void shutdown();

	/** �� NP ���ڴ�С����λ�øı�ʱ, ͨ�� NPP_SetWindow ֪ͨ update */
	void update(NPWindow* aWindow);

	/** �� NPP_GetValue() ����, �� Firefox ��ѯ JavaScript �ӿ�ʱ, �ѽӿڴ��ݸ� Firefox */
	NPError GetValue(NPPVariable aVariable, void *aValue);
	
	/** ���ش����� NPObject ��װ���� */
	CoralIETabNPObject * getScriptObject();

	/** ���ش����� prefs ��װ���� */
	nsConfigManager * getConfigManager();

	/** ���ش����� CIEHostWindow ���� */
	CIEHostWindow * getIEHostWindow();

	/** ���� Plugin NPP ָ�� */
	NPP instance();

public:

	/** ������ݽ����� flags */
	PRInt32 flags;

private:

	bool initLoadingUrl();

	bool getHostUrl(LPWSTR pszUrl, DWORD dwSize);

private:

	/** �Ƿ��Ѿ���ʼ�����ˣ� */
	NPBool m_bInitialized;

	/** Plugin ��ʵ�� */
	NPP m_nppInstance;

	/** ������ص� URL */
	CString m_strLoadingUrl;

	/** IE ���ڿؼ� */
	CIEHostWindow * m_pIEHostWindow;

	/** ����� JavaScript �໥�ӿڵ� NPObject ���� */
	CoralIETabNPObject * m_pScriptableNPObject;

	/** ��ȡ Firefox prefs �ķ�װ���� */
	nsConfigManager * m_configManager;
};
