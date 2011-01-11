#include "Tariff.h"
#include "utils.h"
#include <iostream>
#include <cstring>

using namespace std;

void print_usage( ) {
    cerr << "Invalid usage" << endl;
}

int main( int argc, char** argv ) {
    if ( argc <= 2 ) {
        print_usage();
        return 0;
    }

    string fname = argv[1];
    if ( fname == "tariff_create" ) {
        if ( argc < 4 ) {
            print_usage();
            return 0;
        }

        string tariff_name = argv[2];
        string tariff_file = argv[3];

        Tariff base = Tariff::buildEmpty( tariff_name );

        while ( !cin.eof() ) {
            char line[1024];
            vector< string > params;

            cin.getline( line, 1024 );
            sms::utils::Tokenize( line, params, " \n" );

            if ( params.size() == 2 ) {
                string cname = params[0];
                float costs = boost::lexical_cast< float >( params[ 1 ] );

                base.addFilterCountry( cname, costs );
            }

            if ( params.size() == 3 ) {
                string cname = params[0];
                string opcode = params[1];
                float costs = boost::lexical_cast< float >( params[ 2 ] );

                base.addFilterCountryOperator( cname, opcode, costs );
            }
        }

        base.saveToFile( tariff_file );
    }

    if ( fname == "tariff_inherit" ) {
        if ( argc < 5 ) {
            print_usage();
            return 0;
        }

        string tariff_name = argv[2];
        string tariff_file = argv[3];
        string base_file = argv[4];

        Tariff base = Tariff::buildInherit( tariff_name, base_file );

        while ( !cin.eof() ) {
            char line[1024];
            vector< string > params;

            cin.getline( line, 1024 );
            sms::utils::Tokenize( line, params, " \n" );

            if ( params.size() == 2 ) {
                string cname = params[0];
                float costs = boost::lexical_cast< float >( params[ 1 ] );

                base.addFilterCountry( cname, costs );
            }

            if ( params.size() == 3 ) {
                string cname = params[0];
                string opcode = params[1];
                float costs = boost::lexical_cast< float >( params[ 2 ] );

                base.addFilterCountryOperator( cname, opcode, costs );
            }
        }

        base.saveToFile( tariff_file );
    }


    if ( fname == "tariff_check" ) {
        if ( argc < 5 ) {
            print_usage();
            return 0;
        }

        string tariff_file = argv[2];
        string cname = argv[3];
        string opcode = argv[4];
        float costs;

        Tariff base = Tariff::buildFromFile( tariff_file );
        try {
            costs = base.costs( cname, opcode );
        } catch ( ... ) {
            costs = 1.0;
        }

        cout << costs << endl;
    }

}
