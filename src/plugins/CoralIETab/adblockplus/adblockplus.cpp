#include "stdafx.h"

#include "../nsConfigManager.h"

#include "FilterStorage.h"
#include "HttpRequestFilters.h"
#include "adblockplus.h"

namespace adblockplus
{
	bool isEnabled = true;

	void enable(bool b)
	{
		isEnabled = b;
	}

	void init(const std::string & patternsFile)
	{
		if ( isEnabled )
		{
			HttpRequestFilters::init();

			FilterStorage::loadFromDisk(patternsFile);
		}
	}

	bool test(const HttpRequest * request)
	{
		if ( isEnabled )
		{
			return HttpRequestFilters::test(request);
		}
		else
		{
			return false;
		}
	}
}