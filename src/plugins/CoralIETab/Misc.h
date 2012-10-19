#pragma once

#include <string>
#include <vector>

/**
* 模糊匹配两个 URL.
* http://my.com/path/file.html#123 和 http://my.com/path/file.html 会认为是同一个 URL
* http://my.com/path/query?p=xyz 和 http://my.com/path/query 不认为是同一个 URL
*/
BOOL FuzzyUrlCompare( LPCTSTR lpszUrl1, LPCTSTR lpszUrl2 );

/** 正则表达式匹配 */
bool regexpr_match(const char * pattern, const char * sourceString, bool ignoreCase, std::string * results = NULL, size_t resultCount = 0);

/** 正则表达式替换 */
void regexpr_replace(const char * pattern, const char * sourceString, const char * replaceString, std::string & resultString, bool bGlobal = false);

/** 拆分字符串 */
void tokenize_string(const std::string & str, std::vector<std::string> & tokens, const std::string & delimiters = " ");

/** 字符串替换 */
void string_replace( std::string & target, const std::string & oldstr, const std::string & newstr );
