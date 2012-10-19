#pragma once

#include <string>

#include "Filter.h"

namespace adblockplus
{
	/** 启用/禁用 adblockplus 支持 */
	void enable(bool b);

	/** 初始化 adblockplus 支持 */
	void init(const std::string & patternsFile);

	/** 测试一个 HTTP 请求是否要被过滤 */
	bool test(const HttpRequest * request);
}