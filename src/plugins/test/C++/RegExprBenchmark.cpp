// RegExprBenchmark.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <list>

#define USE_PCRE

#ifdef USE_PCRE

#include <string>

#include <pcreposix.h>
#include <pcrecpp.h>

#pragma comment(lib, "pcreposixd.lib")
#pragma comment(lib, "pcrecppd.lib")

BOOL RegExprMatch(LPCTSTR pPattern, LPCTSTR sourceString, BOOL bIgnoreCase, CString * pResult /*= NULL*/, long nResultCount /*= 0*/)
{
	BOOL bMatched = FALSE;

	regex_t reobj;
	if ( regcomp(&reobj, pPattern, bIgnoreCase ? REG_ICASE|REG_EXTENDED:REG_EXTENDED) == 0 )
	{
		long N = nResultCount + 1;
		regmatch_t * match = new regmatch_t[N];
		if ( regexec(&reobj, sourceString, N, match, 0) == 0 )
		{
			for ( long i = 1; i < N; i++ )
			{
				pResult[i-1].Format(_T("%.*s"), match[i].rm_eo - match[i].rm_so, sourceString + match[i].rm_so);
			}

			bMatched = TRUE;
		}
		delete [] match;

		regfree(&reobj);
	}

	return bMatched;
}

VOID RegExprReplace(LPCTSTR pPattern, LPCTSTR sourceString, LPCTSTR replaceString, CString & resultString, BOOL bGlobal = FALSE)
{
	std::string s(sourceString);
	if ( bGlobal )
		pcrecpp::RE(pPattern).GlobalReplace(replaceString, &s);
	else
		pcrecpp::RE(pPattern).Replace(replaceString, &s);

	resultString = s.c_str();
}

#endif

#ifdef USE_TRE

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <regex.h>

#pragma comment(lib, "tre.lib")

BOOL RegExprMatch(LPCTSTR pPattern, LPCTSTR sourceString, BOOL bIgnoreCase, CString * pResult /*= NULL*/, long nResultCount /*= 0*/)
{
	BOOL bMatched = FALSE;

	regex_t reobj;
	if ( regwcomp(&reobj, pPattern, bIgnoreCase ? REG_ICASE|REG_EXTENDED:REG_EXTENDED) == 0 )
	{
		long N = nResultCount + 1;
		regmatch_t * match = new regmatch_t[N];
		if ( regwexec(&reobj, sourceString, N, match, 0) == 0 )
		{
			for ( long i = 1; i < N; i++ )
			{
				pResult[i-1].Format(_T("%.*s"), match[i].rm_eo - match[i].rm_so, sourceString + match[i].rm_so);
			}

			bMatched = TRUE;
		}
		delete [] match;

		regfree(&reobj);
	}

	return bMatched;
}

#endif

#ifdef USE_GRETA

#include <iostream>
#include <string>

#include "greta/regexpr2.h"

BOOL RegExprMatch(LPCWSTR pPattern, LPCWSTR sourceString, BOOL bIgnoreCase, CString * pResult /*= NULL*/, long nResultCount /*= 0*/)
{
	regex::match_results results;

	regex::rpattern pat(pPattern);
	std::wstring str(sourceString);

	regex::match_results::backref_type br = pat.match( str, results );
	if ( pResult && ( nResultCount > 0 ) )
	{
		regex::match_results::backref_vector vec = results.all_backrefs();
		regex::match_results::backref_vector::iterator iter = vec.begin();

		int i = 0;
		while ( ( ++iter != vec.end() ) && ( i < nResultCount ) )
		{
			pResult[i++] = (*iter).str().c_str();
		}
	}

	return br.matched;
}

VOID RegExprReplace(LPCWSTR pPattern, LPCWSTR sourceString, LPCWSTR replaceString, CString & resultString, BOOL bGlobal = FALSE)
{
	try
	{
		regex::rpattern pat(pPattern, replaceString, bGlobal ? regex::GLOBAL : regex::NOFLAGS);
		std::wstring str(sourceString);

		regex::subst_results results;
		pat.substitute(str, results);

		resultString = str.c_str();
	}
	catch (...)	{}
}

#endif

#ifdef USE_VBSCRIPT_REGEXP

#import "progid:VBScript.RegExp" no_namespace

BOOL RegExprMatch(LPCWSTR pPattern, LPCWSTR sourceString, BOOL bIgnoreCase, CString * pResult /*= NULL*/, long nResultCount /*= 0*/)
{
	BOOL bMatched = FALSE;

	try
	{
		do 
		{
			IRegExp2Ptr pRegExp;
			if ( FAILED(pRegExp.CreateInstance(__uuidof(RegExp))) ) break;

			CComBSTR bstrPattern(pPattern);
			if ( FAILED(pRegExp->put_Pattern(bstrPattern)) ) break;

			if ( FAILED(pRegExp->put_IgnoreCase( bIgnoreCase ? VARIANT_TRUE : VARIANT_FALSE )) ) break;

			IMatchCollection2Ptr matches = pRegExp->Execute(sourceString);
			if ( ! matches ) break;

			bMatched = matches->Count > 0;

			if ( !bMatched ) break;
			if ( ( NULL == pResult ) || ( nResultCount <= 0 ) ) break;

			IMatch2Ptr match = matches->Item[0];
			if ( ! match ) break;

			ISubMatchesPtr submatches = match->GetSubMatches();
			if ( ! submatches ) break;

			int n = submatches->Count;
			bMatched = n == nResultCount;

			if ( !bMatched ) break;

			for ( long i = 0; i < n; i++ )
			{
				if ( V_VT(&submatches->Item[i]) == VT_BSTR )
				{
					pResult[i] = submatches->Item[i].bstrVal;
				}
			}

		} while(false);
	}
	catch (_com_error& e)
	{
		_tprintf_s(_T("ERROR: %s\r\n"), e.ErrorMessage());
	}
	catch (...) {}

	return bMatched;
}

VOID RegExprReplace(LPCWSTR pPattern, LPCWSTR sourceString, LPCWSTR replaceString, CString & resultString, BOOL bGlobal = FALSE)
{
	_bstr_t bstrResult;

	try
	{
		IRegExp2Ptr pRegExp;
		if ( SUCCEEDED(pRegExp.CreateInstance(__uuidof(RegExp))) )
		{
			CComBSTR bstrPattern(pPattern);
			if ( SUCCEEDED(pRegExp->put_Pattern(bstrPattern)) )
			{
				pRegExp->put_Global(bGlobal ? VARIANT_TRUE : VARIANT_FALSE);

				_bstr_t bstrSourceString(sourceString);
				_bstr_t replaceVar(replaceString);

				resultString = (wchar_t *)pRegExp->Replace(bstrSourceString, replaceString);
			}
		}
	} catch (_com_error& e)
	{
		_tprintf_s(_T("ERROR: %s\r\n"), e.ErrorMessage());
	}
	catch (...) {}
}

#endif

#ifdef USE_ATL_REGEXP

#include <atlrx.h>

BOOL RegExprMatch(LPCTSTR pPattern, LPCTSTR sourceString, BOOL bIgnoreCase, CString * pResults /*= NULL*/, long nResultCount /*= 0*/)
{
	BOOL bMatched = FALSE;

	CAtlRegExp<> re;
	if ( re.Parse(pPattern, ! bIgnoreCase) == REPARSE_ERROR_OK )
	{
		CAtlREMatchContext<> mc;
		if ( re.Match(sourceString, &mc) )
		{
			for (UINT nGroupIndex = 0; nGroupIndex < mc.m_uNumGroups; ++nGroupIndex)
			{
				const CAtlREMatchContext<>::RECHAR * szStart = 0;
				const CAtlREMatchContext<>::RECHAR * szEnd = 0;
				mc.GetMatch(nGroupIndex, &szStart, &szEnd);

				ptrdiff_t nLength = szEnd - szStart;
				pResults[nGroupIndex-1].Format(_T("%.*s"), nLength, szStart);
			}

			bMatched = TRUE;
		}
	}

	return bMatched;
}

#endif

int _tmain(int argc, _TCHAR* argv[])
{
	static const LPCTSTR ELEMHIDE_PATTERN = _T("^([^\\/\\*\\|\\@\"]*?)#(?:([\\w\\-]+|\\*)((?:\\([\\w\\-]+(?:[$^*]?=[^\\(\\)\"]*)?\\))*)|#([^{}]+))$");
	static const LPCTSTR REGEXPR_PATTERN = _T("^\\/(.*)\\/");
	static const LPCTSTR OPTIONS_PATTERN = _T("\\$(~?[\\w\\-]+(?:=[^,\\s]+)?(?:,~?[\\w\\-]+(?:=[^,\\s]+)?)*)$");

#ifdef USE_VBSCRIPT_REGEXP
	::CoInitialize(NULL);
#endif

	int i = 0, j = 0, k = 0, l = 0;

	std::list<CString> patternFileLines;

	FILE * fp = NULL;
	if ( fopen_s(&fp, "patterns.ini", "r") == 0 )
	{
		char line[2048];
		while ( fgets(line, ARRAYSIZE(line), fp) != NULL )
		{
			// 去掉尾部的换行符
			size_t tail_pos = strlen(line);
			if ( '\n' == line[tail_pos-1] )
			{
				line[tail_pos-1] = 0;
			}

			CString str(line);
			patternFileLines.push_back(str);

			i++;
		}

		fclose(fp);
	}

	std::list<CString> output;

	std::list<CString> replace_task;

	DWORD dwTick = GetTickCount();

	for( std::list<CString>::iterator iter = patternFileLines.begin(); iter != patternFileLines.end(); iter++ )
	{
		CString & str = * iter;

		CString results[4];
		if ( RegExprMatch(ELEMHIDE_PATTERN, str, FALSE, results, 4) )
		{
			/*
			CString str;
			str.Format(_T("1 = %s, 2 = %s, 3 = %s, 4 = %s\r\n"), (LPCTSTR)results[0], (LPCTSTR)results[1], (LPCTSTR)results[2], (LPCTSTR)results[3]);
			output.push_back(str);
			*/

			j++;
		}
		else
		{
			if ( RegExprMatch(OPTIONS_PATTERN, str, FALSE, results, 1) )
			{
				RegExprReplace(OPTIONS_PATTERN, str, _T(""), str);
				
				k++;
			}

			if ( RegExprMatch(REGEXPR_PATTERN, str, FALSE, results, 1) )
			{
				l++;
			}
			else
			{
				if ( str[0] != _T('!') )
				{
					replace_task.push_back(str);
				}
			}
		}
	}

	DWORD dwElapsed = GetTickCount() - dwTick;

	_tprintf_s(_T("%d lines:\r\n%d lines of elem hide, %d lines of options, %d lines of regexpr in %d ticks.\r\n"), i, j, k, l, dwElapsed);

	dwTick = GetTickCount();

	for( std::list<CString>::iterator iter = replace_task.begin(); iter != replace_task.end(); iter++ )
	{
		CString & str = * iter;

		RegExprReplace(_T("\\*+"), str, _T("*"), str, TRUE);
		RegExprReplace(_T("\\^\\|$"), str, _T("^"), str, FALSE);
#ifdef USE_VBSCRIPT_REGEXP
		RegExprReplace(_T("(\\W)"), str, _T("\\$1"), str, TRUE);
#endif
#ifdef USE_PCRE
		RegExprReplace(_T("(\\W)"), str, _T("\r\\1"), str, TRUE);
		str.Replace(_T('\r'), _T('\\'));
#endif
#ifdef USE_GRETA
		RegExprReplace(_T("(\\W)"), str, _T("\r$1"), str, TRUE);
		str.Replace(_T('\r'), _T('\\'));
#endif
		RegExprReplace(_T("\\\\\\*"), str, _T(".*"), str, TRUE);
#ifdef USE_PCRE
		RegExprReplace(_T("\\\\\\^"), str, _T("(?:[^\rw\r-.%\ru0080-\ruFFFF]|$)"), str, TRUE);
		RegExprReplace(_T("^\\\\\\|\\\\\\|"), str, _T("^[\rw\r-]+:\r/+(?!\r/)(?:[^\r/]+\r.)?"), str, FALSE);
		str.Replace(_T('\r'), _T('\\'));
#endif
#ifdef USE_VBSCRIPT_REGEXP
		RegExprReplace(_T("\\\\\\^"), str, _T("(?:[^\\w\\-.%\\u0080-\\uFFFF]|$)"), str, TRUE);
		RegExprReplace(_T("^\\\\\\|\\\\\\|"), str, _T("^[\\w\\-]+:\\/+(?!\\/)(?:[^\\/]+\\.)?"), str, FALSE);
#endif
		RegExprReplace(_T("^\\\\\\|"), str, _T("^"), str, FALSE);
		RegExprReplace(_T("\\\\\\|$"), str, _T("$"), str, FALSE);
		RegExprReplace(_T("^(\\.\\*)"), str, _T(""), str, FALSE);
		RegExprReplace(_T("(\\.\\*)$"), str, _T(""), str, FALSE);

		output.push_back(str);
	}

	dwElapsed = GetTickCount() - dwTick;

	_tprintf_s(_T("Replaced %d lines in %d ticks.\r\n"), replace_task.size(), dwElapsed);

	if ( _tfopen_s(&fp, _T("replaced.txt"), _T("wb")) == 0 )
	{
		for (std::list<CString>::iterator iter = output.begin(); iter != output.end(); iter++ )
		{
			CString str = * iter;
			str.Append(_T("\r\n"));
			_fputts(str, fp);
		}

		fclose(fp);
	}

	return 0;
}

