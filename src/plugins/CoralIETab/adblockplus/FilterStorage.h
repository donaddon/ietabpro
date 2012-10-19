#pragma once

#include <string>

namespace adblockplus
{
	class FilterStorage
	{
	public:

		/** �Ӵ��̼��� adblockplus �Ĺ��˹���*/
		static bool loadFromDisk(const std::string & patternsFile);

	private:

		/** ���� adblockplus �� patterns.ini �ļ���ȫ·�� */
		static const char * getPatternFilename();

		/** �����ζ�, ��һ��, �����еĹ���ȫ����һ�� */
		static void readFile(FILE * fp);

	private:

		/** adblockplus �� patterns.ini �ļ� */
		static std::string m_patternFilename;
	};
}
