#ifndef __UTILS_H
#define __UTILS_H

#include <string>
#include <map>
#include <vector>
#include <cstdlib>

#include "Error.h"

using std::vector;
using std::map;
using std::string;
using std::wstring;

namespace sms {
    
    namespace utils {


        void Tokenize(const string& str, vector<string>& tokens, const string& delimiters = " ");
        void splitArgs(const string& s, map<string, string>& m);

        int UrlDecode(const char *source, char *dest);
        int UrlEncode(const char *source, char *dest, unsigned max);
        string UrlEncodeString(const std::string & decoded);
        string UrlDecodeString(const std::string& st);

        string StringUtf8ToCp1251(std::string src) throw ( ParamError );
        string StringCp1251ToUtf8(std::string src) throw ( ParamError );

        string StringUtf8ToUcs2be(std::string src) throw ( ParamError );
        string StringCp1251ToUcs2be(std::string src) throw ( ParamError );

        string Hex2String(std::string src);
        string str_join(const std::vector<std::string> & vec,const std::string & sep);
        string joinArgs(const std::map<string, string>& m);
        string escapeString( std::string where, std::string what, std::string by );
        std::wstring widen( const std::string& str );
        string narrow( const std::wstring& str );

        int getGsmParts( const string& orig );

    }
}
#endif
