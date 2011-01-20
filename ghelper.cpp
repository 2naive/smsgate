#include "SMPPGateFilterParser.h"

using namespace sms;
using namespace std;

int main() {
    std::map< std::string, boost::any > args;

    args[ "TO" ] = string("79852970920");
    args[ "COUNTRY" ] = string("ru");
    args[ "COUNTRYCODE" ] = string("7");
    args[ "OPERATOR" ] = string("mts");
    args[ "OPERATORCODE" ] = string("mts");
    args[ "FROM" ] = string("1312");

    SMPPGateFilterParser parser;
    SMPPGateFilterParser::ResT res = parser.parseStr( "COUNTRY IN [\"af\";\"us\";\"au\";\"ru\";]" );

    bool match = res.filter.check( args );

    return match;
}
