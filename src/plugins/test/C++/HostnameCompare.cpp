// Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <windows.h>

#include <assert.h>

BOOL HostnameCompare( LPCTSTR lpszHost1, LPCTSTR lpszHost2 )
{
	TCHAR * p1 = (TCHAR *)lpszHost1 + _tcslen(lpszHost1);
	TCHAR * p2 = (TCHAR *)lpszHost2 + _tcslen(lpszHost2);

	while ( ( p1 >= lpszHost1 ) && ( p2 >= lpszHost2 ) )
	{
		if ( (*p1) != (*p2) ) return FALSE;

		p1--;
		p2--;
	}

	return ( ( p1 >= lpszHost1 ) && ( _T('.') == *p1 ) ) || ( ( p2 >= lpszHost2 ) && ( _T('.') == *p2 ) ) || ( ( p1 < lpszHost1 ) && ( p2 < lpszHost2 ) );
}


int _tmain(int argc, _TCHAR* argv[])
{
	assert(HostnameCompare(_T("www.google.com"), _T("google.com")) == TRUE);
	assert(HostnameCompare(_T("www.google.com.hacker.com"), _T("google.com")) == FALSE);
	assert(HostnameCompare(_T("www.google.com"), _T("hackgoogle.com")) == FALSE);
	assert(HostnameCompare(_T("www.google.com"), _T("images.google.com")) == FALSE);
	assert(HostnameCompare(_T("google.com"), _T("images.google.com")) == TRUE);
	assert(HostnameCompare(_T("google.com"), _T("hackgoogle.com")) == FALSE);
	assert(HostnameCompare(_T("google.com"), _T("google.com")) == TRUE);
	assert(HostnameCompare(_T("google.com"), _T(".google.com")) == TRUE);

	return 0;
}

