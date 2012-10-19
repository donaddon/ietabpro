#pragma once

#include <string>
#include <vector>

/**
* ģ��ƥ������ URL.
* http://my.com/path/file.html#123 �� http://my.com/path/file.html ����Ϊ��ͬһ�� URL
* http://my.com/path/query?p=xyz �� http://my.com/path/query ����Ϊ��ͬһ�� URL
*/
BOOL FuzzyUrlCompare( LPCTSTR lpszUrl1, LPCTSTR lpszUrl2 );

/** ������ʽƥ�� */
bool regexpr_match(const char * pattern, const char * sourceString, bool ignoreCase, std::string * results = NULL, size_t resultCount = 0);

/** ������ʽ�滻 */
void regexpr_replace(const char * pattern, const char * sourceString, const char * replaceString, std::string & resultString, bool bGlobal = false);

/** ����ַ��� */
void tokenize_string(const std::string & str, std::vector<std::string> & tokens, const std::string & delimiters = " ");

/** �ַ����滻 */
void string_replace( std::string & target, const std::string & oldstr, const std::string & newstr );
