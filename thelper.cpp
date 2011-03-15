#include "Tariff.h"
#include "SMSMessage.h"
#include "utils.h"
#include <iostream>
#include <cstring>

using namespace std;

void print_usage( ) {
    cerr << "Invalid usage:" << endl;
    cerr << "\tthelper check tariff_name " << endl;

}

int main( int argc, char** argv ) {
    if ( argc <= 2 ) {
        print_usage();
        return 0;
    }

    if ( std::string(argv[1]) == "check" ) {
        if ( argc != 3 ) {
            print_usage();
            return 0;
        }

        Tariff tariff = TariffManager::get_mutable_instance().loadTariff( argv[ 2 ] );

        while ( !cin.eof() ) {
            std::string phone;
            int status;
            cin >> phone >> status;

            sms::MessageClassifier::CountryInfo ci = sms::MessageClassifier::get_mutable_instance().getMsgClass( phone );
            if ( ci.operators.empty() ) {
                cout << tariff.costs( ci.mcc, SMSMessage::Status( status ) ) << endl;
            } else {
                cout << tariff.costs( ci.mcc, ci.operators.begin()->second.mnc, SMSMessage::Status( status ) ) << endl;
            }
        }

    }

}
