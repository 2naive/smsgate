#include "MTHandler.h"

#include <iostream>
#include <Wt/WApplication>
#include <Wt/Http/Request>

#include <boost/thread/mutex.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/thread/xtime.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "MTHandler.h"
#include "RequestTracker.h"
#include "utils.h"
#include "PartnerManager.h"
#include "HttpClient.h"

using namespace std;
using namespace sms;
using namespace sms::utils;

MTHandler::MTHandler(Wt::WObject *parent) : Wt::WResource(parent) {
}

MTHandler::~MTHandler() {
        beingDeleted();
}

void MTHandler::handleRequest(const Wt::Http::Request& request, Wt::Http::Response& response) {

        Logger::get_mutable_instance().smslogwarn( "MTHandler::handleRequest started" );

        response.setMimeType("text/html");

        SMSRequest* req = new SMSRequest();
        const string e = "";

        RequestTracker *trck = RequestTracker::Instance();
        ostringstream o, out;

        const string pref 		= request.getParameter("pref")		? utils::StringCp1251ToUtf8(*request.getParameter("pref"))	: e;
        const string txt 		= request.getParameter("txt")		? utils::StringCp1251ToUtf8(*request.getParameter("txt"))	: e;
        const string tid		= request.getParameter("tid")		? *request.getParameter("tid")          : e;
        const string cn 		= request.getParameter("cn")  		? *request.getParameter("cn")		: e;
        const string op 		= request.getParameter("op")  		? *request.getParameter("op")		: e;
        const string phone 		= request.getParameter("phone") 	? *request.getParameter("phone")	: e;
        const string sn 		= request.getParameter("sn")  		? *request.getParameter("sn")		: e;

        const string test 		= request.getParameter("test")          ? *request.getParameter("test")		: e;
        const string repeat 		= request.getParameter("repeat")        ? *request.getParameter("repeat")	: e;
        const string rtime 		= request.getParameter("rtime") 	? *request.getParameter("rtime")	: e;
        const string opn		= request.getParameter("opn")  		? *request.getParameter("opn")		: e;
        const string mpref 		= request.getParameter("mpref")  	? *request.getParameter("mpref")	: e;
        const string tg 		= request.getParameter("tg")  		? *request.getParameter("tg")		: e;
        const string cost 		= request.getParameter("cost") 		? *request.getParameter("cost")		: e;
        const string md5 		= request.getParameter("md5")  		? *request.getParameter("md5")		: e;

        response.out() << utils::StringUtf8ToCp1251("sms=Тест принят");

        Logger::get_mutable_instance().smslogwarn( "MTHandler::handleRequest finished" );
}
