// Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <Windows.h>
#include <WinInet.h>
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

/**
* 模糊匹配两个 URL.
* http://my.com/path/file.html#123 和 http://my.com/path/file.html 会认为是同一个 URL
* http://my.com/path/query?p=xyz 和 http://my.com/path/query 不认为是同一个 URL
*/
BOOL FuzzyUrlCompare( LPCTSTR lpszUrl1, LPCTSTR lpszUrl2 )
{
	static const TCHAR ANCHOR = _T('#');
	static const TCHAR FILE_PROTOCOL [] = _T("file://");
	static const size_t FILE_PROTOCOL_LENGTH = _tcslen(FILE_PROTOCOL);

	BOOL bMatch = TRUE;

	if ( lpszUrl1 && lpszUrl2 )
	{
		TCHAR szDummy1[MAX_PATH];
		TCHAR szDummy2[MAX_PATH];

		if ( _tcsncmp( lpszUrl1, FILE_PROTOCOL, FILE_PROTOCOL_LENGTH ) == 0 )
		{
			DWORD dwLen = MAX_PATH;
			if ( PathCreateFromUrl( lpszUrl1, szDummy1, & dwLen, 0 ) == S_OK )
			{
				lpszUrl1 = szDummy1;
			}
		}

		if ( _tcsncmp( lpszUrl2, FILE_PROTOCOL, FILE_PROTOCOL_LENGTH ) == 0 )
		{
			DWORD dwLen = MAX_PATH;
			if ( PathCreateFromUrl( lpszUrl2, szDummy2, & dwLen, 0 ) == S_OK )
			{
				lpszUrl2 = szDummy2;
			}
		}

		do
		{
			if ( *lpszUrl1 != *lpszUrl2 )
			{
				if ( ( ( ANCHOR == *lpszUrl1 ) && ( 0 == *lpszUrl2 ) ) ||
					( ( ANCHOR == *lpszUrl2 ) && ( 0 == *lpszUrl1 ) ) )
				{
					bMatch = TRUE;
				}
				else
				{
					bMatch = FALSE;
				}

				break;
			}

			lpszUrl1++;
			lpszUrl2++;

		} while ( *lpszUrl1 || *lpszUrl2 );
	}

	return bMatch;
}

int main(void)
{
	static const TCHAR URL01[] = _T("http://my.com/path/file.html#123");
	static const TCHAR URL02[] = _T("http://my.com/path/file.html");

	static const TCHAR URL11[] = _T("http://my.com/path/query?p=xyz");
	static const TCHAR URL12[] = _T("http://my.com/path/query");

	static const TCHAR URL21[] = _T("http://www.19lou.com/viewthread.php?tid=23130085&fid=1415&page=1&extra=page%3D1");
	static const TCHAR URL22[] = _T("http://www.19lou.com/viewthread.php?tid=23130085&fid=1415&page=1&extra=page%3D1#pid297542731");

	static const TCHAR URL31[] = _T("http://my.com/path/#123");
	static const TCHAR URL32[] = _T("http://my.com/path/file.html");

	static const TCHAR URL41[] = _T("http://hello.com/path/");
	static const TCHAR URL42[] = _T("http://abc.com/newpath/");

	static const TCHAR URL51[] = _T("http://www.yahoo.com.cn");
	static const TCHAR URL52[] = _T("http://www.yahoo.com.cn");

	static const TCHAR URL61[] = _T("file:///F:/Project/CoralIETab/src/plugins/test/window_close_test.html");
	static const TCHAR URL62[] = _T("F:\\Project\\CoralIETab\\src\\plugins\\test\\window_close_test.html");

	BOOL b = FuzzyUrlCompare(URL01, URL02);

	b = FuzzyUrlCompare(URL11, URL12);

	b = FuzzyUrlCompare(URL21, URL22);
	
	b = FuzzyUrlCompare(URL31, URL32);

	b = FuzzyUrlCompare(URL41, URL42);

	b = FuzzyUrlCompare(URL51, URL52);

	b = FuzzyUrlCompare(URL61, URL62);


	return 0;
}
