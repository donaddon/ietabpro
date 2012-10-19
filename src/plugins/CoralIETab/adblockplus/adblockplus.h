#pragma once

#include <string>

#include "Filter.h"

namespace adblockplus
{
	/** ����/���� adblockplus ֧�� */
	void enable(bool b);

	/** ��ʼ�� adblockplus ֧�� */
	void init(const std::string & patternsFile);

	/** ����һ�� HTTP �����Ƿ�Ҫ������ */
	bool test(const HttpRequest * request);
}