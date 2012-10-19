#include "stdafx.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "../3rdParty/pcre/pcre-8.02/pcreposix.h"
#include "../3rdParty/pcre/pcre-8.02/pcrecpp.h"

#include "Misc.h"

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

bool regexpr_match(const char * pattern, const char * sourceString, bool ignoreCase, std::string * results /*= NULL*/, size_t resultCount /*= 0*/)
{
	bool r = false;

	regex_t reobj;
	if ( regcomp(&reobj, pattern, ignoreCase ? REG_ICASE|REG_EXTENDED:REG_EXTENDED) == 0 )
	{
		long N = resultCount + 1;
		regmatch_t * match = new regmatch_t[N];
		if ( regexec(&reobj, sourceString, N, match, 0) == 0 )
		{
			for ( long i = 1; i < N; i++ )
			{
				results[i-1].assign(sourceString + match[i].rm_so, match[i].rm_eo - match[i].rm_so);
			}

			r = true;
		}
		delete [] match;

		regfree(&reobj);
	}

	return r;
}

void regexpr_replace( const char * pattern, const char * sourceString, const char * replaceString, std::string & resultString, bool bGlobal /*= false*/ )
{
	std::string s(sourceString);
	if ( bGlobal )
		pcrecpp::RE(pattern).GlobalReplace(replaceString, &s);
	else
		pcrecpp::RE(pattern).Replace(replaceString, &s);

	resultString = s.c_str();
}

void tokenize_string( const std::string & str, std::vector<std::string> & tokens, const std::string & delimiters /*= " "*/ )
{
	// Skip delimiters at beginning.
	std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	// Find first "non-delimiter".
	std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

	while (std::string::npos != pos || std::string::npos != lastPos)
	{
		// Found a token, add it to the vector.
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		// Skip delimiters.  Note the "not_of"
		lastPos = str.find_first_not_of(delimiters, pos);
		// Find next "non-delimiter"
		pos = str.find_first_of(delimiters, lastPos);
	}
}

void string_replace( std::string & target, const std::string & oldstr, const std::string & newstr )
{
	std::string::size_type pos = 0;
	std::string::size_type srclen=oldstr.size();
	std::string::size_type dstlen=newstr.size();
	while( ( pos = target.find(oldstr, pos)) != string::npos )
	{
		target.replace(pos, srclen, newstr);
		pos += dstlen;
	}
}