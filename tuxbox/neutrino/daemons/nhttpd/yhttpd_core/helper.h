//=============================================================================
// YHTTPD
// Helper
//=============================================================================

#ifndef __yhttpd_helper_h__
#define __yhttpd_helper_h__
// c
#include <ctime>
// c++
#include <string>
#include <vector>
#include "ytypes_globals.h"

//-----------------------------------------------------------------------------
int minmax(int value,int min, int max);
void correctTime(struct tm *zt);

//-----------------------------------------------------------------------------
// String Converter
//-----------------------------------------------------------------------------
std::string itoa(unsigned int conv);
std::string itoh(unsigned int conv);
std::string decodeString(const std::string &encodedString);
std::string encodeString(const std::string &decodedString);
std::string string_tolower(std::string str);

//-----------------------------------------------------------------------------
// String Helpers
//-----------------------------------------------------------------------------
std::string trim(std::string const& source, char const* delims = " \t\r\n");
void replace(std::string &str, const std::string &find_what, const std::string &replace_with);
std::string string_printf(const char *fmt, ...);
bool ySplitString(const std::string &str, const std::string &delimiter, std::string &left, std::string &right);
bool ySplitStringExact(const std::string &str, const std::string &delimiter, std::string &left, std::string &right);
bool ySplitStringLast(const std::string &str, const std::string &delimiter, std::string &left, std::string &right);
CStringArray ySplitStringVector(const std::string &str, const std::string &delimiter);
bool nocase_compare (char c1, char c2);
std::string timeString(time_t time);
bool write_to_file(const std::string &filename, const std::string &content);

#endif /* __yhttpd_helper_h__ */
