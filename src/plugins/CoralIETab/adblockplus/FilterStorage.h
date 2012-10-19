#pragma once

#include <string>

namespace adblockplus
{
	class FilterStorage
	{
	public:

		/** 从磁盘加载 adblockplus 的过滤规则*/
		static bool loadFromDisk(const std::string & patternsFile);

	private:

		/** 返回 adblockplus 的 patterns.ini 文件的全路径 */
		static const char * getPatternFilename();

		/** 分两次读, 第一次, 把所有的规则全部读一遍 */
		static void readFile(FILE * fp);

	private:

		/** adblockplus 的 patterns.ini 文件 */
		static std::string m_patternFilename;
	};
}
