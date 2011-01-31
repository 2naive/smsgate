#include "PersonalPage.h"
#include "PartnerManager.h"

using namespace Wt;

PGSql& PersonalPage::db( PGSqlConnPoolStats::get_mutable_instance().getdb() );

WStatPageHeader::WStatPageHeader( PersonalPage* ppage ) {
    this->ppage = ppage;

}

int WStatPageHeader::getTotalLines() {
    return 2;
}

void WStatPageHeader::execute( int lnl, int lnr, RowList &data ) {
    data.clear();
    WLabel* report_status = new WLabel();
    report_status->clicked().connect( boost::bind( &PersonalPage::onSummaryShow, ppage ) );

    for ( int line = lnl; line <= lnr; line++ ) {
        if ( line == 0 ) {
            Row r;
            r.reserve( 7 );
            WLineEdit* pid;
            WLineEdit* phone;
            WDatePicker* date_from, *date_to;
            WLineEdit* text;
            WComboBox* status;
            WTable* date;
            WLineEdit* country;
            WPushButton* reportbtn;
            WTable* controlBlock;
            WLabel* next;
            WLabel* prev;
            WSpinBox* page;
            WLayout* lnext;
            WLayout* lprev;
            WLayout* lpage;

            if ( ppage->isAdmin ) {
                //Pid input field
                pid = new WLineEdit(); pid->setMaximumSize(  WLength( 1, WLength::Centimeter ), WLength::Auto  );
            } else {
                pid = NULL;
            }
            //Phone input field
            phone = new WLineEdit();
            //Date input field
            date_from = new WDatePicker(); date_from->setDate( WDate::currentDate().addDays( -7 ) );
            date_from->calendar()->setSingleClickSelect( true );
            date_to = new WDatePicker(); date_to->setDate( WDate::currentDate() );
            date_to->calendar()->setSingleClickSelect( true );
            date = new WTable();
            date->elementAt(0, 0)->addWidget( new WLabel( WString::fromUTF8("С") ) );
            date->elementAt(0, 1)->addWidget( date_from );
            date->elementAt(1, 0)->addWidget( new WLabel( WString::fromUTF8("По") ) );
            date->elementAt(1, 1)->addWidget( date_to );
            date->setStyleClass("datetable");
            //Message text field
            text = new WLineEdit();
            text->setMinimumSize( WLength( 50, WLength::Centimeter ), WLength::Auto );
            //Delivery status field
            status = new WComboBox();
            status->addItem(WString::fromUTF8("Любой"));
            status->addItem(WString::fromUTF8("Доставлено"));
            status->addItem(WString::fromUTF8("Не доставлено"));
            status->addItem(WString::fromUTF8("Неверный номер"));
            //Country input field
            country = new WLineEdit(); country->setMaximumSize(  WLength( 1, WLength::Centimeter ), WLength::Auto  );
            //Report button
            reportbtn = new WPushButton( WString::fromUTF8("Сгенерировать отчет") );

            page = new WSpinBox(); page->setRange( 1, 1 );
            page->setValue(1);
            page->setMaximumSize(  WLength( 1, WLength::Centimeter ), WLength::Auto  );
            page->valueChanged().connect( boost::bind(
                                             &PersonalPage::widthCorrect,
                                             ppage
                                                     ) );
            page->valueChanged().connect( boost::bind(
                                             &WScrollTable::exactPage,
                                             ppage->statistics,
                                             _1
                                                     ) );

            next = new WLabel( WString::fromUTF8(">>") );
            next->addStyleClass("link");
            next->clicked().connect( boost::bind(
                                        &PersonalPage::onPageInc,
                                        ppage,
                                        page
                                                ) );

            prev = new WLabel( WString::fromUTF8("<<") );
            prev->addStyleClass("link");
            prev->clicked().connect( boost::bind(
                                        &PersonalPage::onPageDec,
                                        ppage,
                                        page
                                                ) );
            lnext = new WVBoxLayout();
            lnext->addWidget( next );
            lprev = new WVBoxLayout();
            lprev->addWidget( prev );
            lpage = new WVBoxLayout();
            lpage->addWidget( page );

            controlBlock = new WTable();
            controlBlock->elementAt( 0, 0 )->addWidget( reportbtn );
            controlBlock->elementAt( 0, 0 )->setColumnSpan( 3 );
            controlBlock->elementAt( 1, 0 )->setLayout( lprev, AlignLeft );
            controlBlock->elementAt( 1, 1 )->setLayout( lpage, AlignCenter );
            controlBlock->elementAt( 1, 2 )->setLayout( lnext, AlignRight );
            controlBlock->setStyleClass("datetable");

            reportbtn->clicked().connect(boost::bind(
                                             &PersonalPage::onReportBtnClicked,
                                             ppage,
                                             pid,
                                             phone,
                                             date_from,
                                             date_to,
                                             std::make_pair(text, country),
                                             std::make_pair(status, page),
                                             reportbtn,
                                             report_status
                                             )
                                         );
            if ( ppage->isAdmin )
                r.push_back( pid );
            r.push_back( phone );
            r.push_back( date );
            r.push_back( text );
            r.push_back( status );
            r.push_back( country );
            r.push_back( NULL );
            r.push_back( NULL );
            r.push_back( controlBlock );

            data.push_back( r );
        }

        if ( line == 1 ) {
            Row r;
            r.reserve( 7 );
            if ( ppage->isAdmin )
                r.push_back( new WLabel(WString::fromUTF8("IDP")) );
            r.push_back( new WLabel(WString::fromUTF8("Телефон")) );
            r.push_back( new WLabel(WString::fromUTF8("Дата")) );
            r.push_back( new WLabel(WString::fromUTF8("Текст")) );
            r.push_back( new WLabel(WString::fromUTF8("Статус")) );
            r.push_back( new WLabel(WString::fromUTF8("Страна")) );
            r.push_back( new WLabel(WString::fromUTF8("Регион")) );
            r.push_back( new WLabel(WString::fromUTF8("Цена")) );
            r.push_back( report_status );

            data.push_back( r );
        }
    }
}

WStatPageData::WStatPageData( PersonalPage* _ppage ) {
    initialized = false;
    __total_lines = 0;
    ppage = _ppage;

    view_name = string("v") + ppage->sessionId();
    res_name =string("p") + ppage->sessionId();
}

WStatPageData::~WStatPageData( ) {
    PGSql& db = ppage->db;

    // Drop temp table if exists
    if ( initialized ) {
        try {
            PGSql::ConnectionHolder cHold( db );
            ConnectionPTR conn = cHold.get();
            TransactionPTR tr = db.openTransaction( conn, "WStatPageData::~WStatPageData ( drop result table ) " );

            std::ostringstream req;
            req     <<  "SELECT drop_matview( '" << res_name << "' );";

            tr->exec( req.str() );
            tr->commit();

        } catch ( ... ) {
            initialized = false;
        }
    }

}

void WStatPageData::prepareRequest( ) {
    PGSql& db = ppage->db;

    // Drop temp table if exists
    if ( initialized ) {
        try {
            PGSql::ConnectionHolder cHold( db );
            ConnectionPTR conn = cHold.get();
            TransactionPTR tr = db.openTransaction( conn, "WStatPageData::prepareRequest ( drop result table ) " );

            std::ostringstream req;
            req     <<  "SELECT drop_matview( '" << res_name << "' );";

            tr->exec( req.str() );
            tr->commit();

        } catch ( ... ) {
            initialized = false;
        }

        try {
            PGSql::ConnectionHolder cHold( db );
            ConnectionPTR conn = cHold.get();
            TransactionPTR tr = db.openTransaction( conn, "WStatPageData::prepareRequest ( drop result view ) " );

            std::ostringstream req;
            req     <<  "DROP VIEW " << view_name << ";";

            tr->exec( req.str() );
            tr->commit();

        } catch ( ... ) {
            initialized = false;
        }
    }

    initialized = true;

    // create temp view according setup filters
    try {
        PGSql::ConnectionHolder cHold( db );
        ConnectionPTR conn = cHold.get();
        TransactionPTR tr = db.openTransaction( conn, "WStatPageData::prepareRequest ( create temp view ) " );

        std::ostringstream req;
        req     <<  "CREATE OR REPLACE TEMP VIEW " << view_name << " AS ";
        req     <<  "SELECT message_status.\"REQUESTID\", message_status.\"MESSAGEID\", \"TXT\", \"FROM\", \"PID\", smsrequest.\"WHEN\" AS REQUESTDATE, "
                <<  "\"STATUS\", message_status.\"TO\", \"PARTS\", \"COUNTRY\", \"COUNTRYCODE\", \"OPERATOR\", \"OPERATORCODE\", \"REGION\", message_status.\"WHEN\" AS DELIVERYDATE, \"PARTNERPRICE\" "
                <<  "FROM smsrequest, message_status WHERE smsrequest.\"REQUESTID\"=message_status.\"REQUESTID\"  ";
        if ( pid_filter )
            req <<  "AND \"PID\"='" << tr->esc( pid_value ) << "' ";
        if ( phone_filter )
            req <<  "AND message_status.\"TO\"='" << tr->esc( phone_value ) << "' ";
        if ( date_from_filter )
            req <<  "AND smsrequest.\"WHEN\">'" << date_from_value << "' ";
        if ( date_to_filter )
            req <<  "AND smsrequest.\"WHEN\"<'" << date_to_value << "' ";
        if ( text_filter )
            req << "AND \"TXT\" LIKE '%" << utils::escapeString( tr->esc( text_value ), "%_", "\\" ) << "%' ESCAPE E'\\\\' ";
        if ( country_filter )
            req <<  "AND \"COUNTRY\"='" << tr->esc(country_value) << "' ";
        if ( status_filter )
            req <<  "AND \"STATUS\"='" << status_value() << "' ";
        req << "ORDER BY smsrequest.\"WHEN\" DESC;";


        {
            boost::xtime from, to;
            boost::xtime_get( &from, boost::TIME_UTC );
            tr->exec( req.str() );
            boost::xtime_get( &to, boost::TIME_UTC );
            req << " for " << to.sec - from.sec << " seconds";
            Logger::get_mutable_instance().dbloginfo( req.str() );
        }

        std::ostringstream req2;
        req2    <<  "SELECT create_matview( '" << res_name << "', '" << view_name << "' );";

        {
            boost::xtime from, to;
            boost::xtime_get( &from, boost::TIME_UTC );
            tr->exec( req2.str() );
            tr->commit();
            boost::xtime_get( &to, boost::TIME_UTC );
            req2 << " for " << to.sec - from.sec << " seconds";
            Logger::get_mutable_instance().dbloginfo( req2.str() );
        }


    } catch ( ... ) {
        initialized = false;
        throw;
    }

    // calculate total_lines_value
    try {
        PGSql::ConnectionHolder cHold( db );
        ConnectionPTR conn = cHold.get();
        TransactionPTR tr = db.openTransaction( conn, "WStatPageData::prepareRequest ( fill result table ) " );

        std::ostringstream req;
        req     <<  "SELECT count(*) from " << res_name << ";";


        {
            boost::xtime from, to;
            boost::xtime_get( &from, boost::TIME_UTC );
            Result res = tr->exec( req.str() );
            tr->commit();

            for ( Result::const_iterator it = res.begin(); it != res.end(); it++ ) {
                __total_lines = (*it)[0].as<int>();
            }
            boost::xtime_get( &to, boost::TIME_UTC );
            req << " for " << to.sec - from.sec << " seconds";
            Logger::get_mutable_instance().dbloginfo( req.str() );
        }

    } catch ( ... ) {
        initialized = false;
        throw;
    }

}

void WStatPageData::resetFilter( ) {
    initialized = true;
    pid_filter = false;
    phone_filter = false;
    date_from_filter = false;
    date_to_filter = false;
    text_filter = false;
    country_filter = false;
    status_filter = false;
}

void WStatPageData::setPidFilter( string pid ) {
    pid_filter = true;
    pid_value = pid;
}

void WStatPageData::setPhoneFilter( string phone ) {
    phone_filter = true;
    phone_value = phone;
}

void WStatPageData::setDateFromFilter( long date_from ) {
    date_from_filter = true;
    date_from_value = date_from;
}

void WStatPageData::setDateToFilter( long date_to ) {
    date_to_filter = true;
    date_to_value = date_to;
}

void WStatPageData::setTextFilter( string text ) {
    text_filter = true;
    text_value = text;
}

void WStatPageData::setCountryFilter( string country ) {
    country_filter = true;
    country_value = country;
}

void WStatPageData::setStatusFilter( SMSMessage::Status status ) {
    status_filter = true;
    status_value = status;
}

int WStatPageData::getTotalLines() {
    if ( !initialized ) {
        return 0;
    }
    return __total_lines;
}

void WStatPageData::evaluateSummary( double &price, int &total, int &delivered, int &rejected, int &undelivered ) {
    PGSql& db = ppage->db;

    price = 0;
    total = 0;
    delivered = 0;
    rejected = 0;
    undelivered = 0;
    try {
        PGSql::ConnectionHolder cHold( db );
        ConnectionPTR conn = cHold.get();
        TransactionPTR tr = db.openTransaction( conn, "WStatPageData::evaluateSummary ( summary results ) " );

        std::ostringstream req;
            req     <<  "select \"STATUS\", sum(\"PARTS\"), sum(\"PARTNERPRICE\") from " << res_name << " GROUP BY \"STATUS\";";

        Result res = tr->exec( req.str() );
        tr->commit();

        for ( Result::const_iterator it = res.begin(); it != res.end(); it++ ) {
            SMSMessage::Status status = SMSMessage::Status( (*it)[0].as<int>() );
            if ( status != SMSMessage::Status::ST_REJECTED ) {
                price += (*it)[2].as<double>();
            }

            if ( status == SMSMessage::Status::ST_REJECTED ) {
                rejected += (*it)[1].as<int>();
            }

            if ( status == SMSMessage::Status::ST_DELIVERED ) {
                delivered += (*it)[1].as<int>();
            }

            if ( status == SMSMessage::Status::ST_NOT_DELIVERED ) {
                undelivered += (*it)[1].as<int>();
            }

            total += (*it)[1].as<int>();
        }

    } catch ( ... ) {
    }
}

void WStatPageData::execute( int lnl, int lnr, RowList &data ) {
    if ( !initialized ) {
        data.clear();
        return;
    }
    PGSql& db = ppage->db;



    // build required widgets
    try {
        PGSql::ConnectionHolder cHold( db );
        ConnectionPTR conn = cHold.get();
        TransactionPTR tr = db.openTransaction( conn, "WStatPageData::execute ( extract results ) " );

        std::ostringstream req;
        req     <<  "SELECT * from " << res_name << " ORDER BY REQUESTDATE DESC limit " << lnr - lnl + 1 << " offset " << lnl << ";";

        Result res = tr->exec( req.str() );
        tr->commit();

        data.clear();
        for ( Result::const_iterator it = res.begin(); it != res.end(); it++ ) {
            SMSMessage::ID msgid = SMSMessage::ID( (*it)[0].as<long long>(), (*it)[1].as<int>() );
            string __pid = (*it)[4].as<string>();
            string __phone = (*it)[7].as<string>();
            long __date_num = (*it)[5].as<long>();
            boost::gregorian::date orig( 1970, boost::gregorian::Jan, 1 );
            boost::posix_time::ptime dt( orig, boost::posix_time::seconds( __date_num + 3*60*60 ) );
            string __date = boost::posix_time::to_simple_string(dt);
            string __txt = (*it)[2].as<string>();
            string __status = SMSMessage::Status::russianDescr( SMSMessage::Status( (*it)[6].as<int>() ) );
            float price = (*it)[15].as<double>();
            if ( SMSMessage::Status( (*it)[6].as<int>() ) == SMSMessage::Status::ST_REJECTED ) {
                price = 0;
            }
            string __price = boost::lexical_cast< string >( int(floor( price * 100 )) / 100) + string(".") +  boost::lexical_cast< string >( int(floor( price * 100 )) % 100 );

            string __country = (*it)[9].as<string>();
            string __region = (*it)[13].as<string>();

            Row row;
            if ( ppage->isAdmin )
                row.push_back( new WLabel( WString::fromUTF8( __pid ) ) );

            WLabel* txt_label = new WLabel( WString::fromUTF8( __txt ) );
            txt_label->setWordWrap( true );

            WLabel* status_label = new WLabel( WString::fromUTF8( __status ) );
            if ( SMSMessage::Status( (*it)[6].as<int>() ) == SMSMessage::Status::ST_DELIVERED ) {
                status_label->setStyleClass("rowOk");
            } else
            if ( SMSMessage::Status( (*it)[6].as<int>() )() < 0 ) {
                status_label->setStyleClass("rowErr");
            } else {
                status_label->setStyleClass("rowWarn");
            }

            row.push_back( new WLabel( WString::fromUTF8( __phone ) ) );
            row.push_back( new WLabel( WString::fromUTF8( __date ) ) );
            row.push_back( txt_label );
            row.push_back( status_label );
            row.push_back( new WLabel( WString::fromUTF8( __country ) ) );
            row.push_back( new WLabel( WString::fromUTF8( __region ) ) );
            row.push_back( new WLabel( WString::fromUTF8( __price ) ) );

            data.push_back( row );
        }

    } catch ( ... ) {
        initialized = false;
    }

}

WApplication *createPersonalPage(const WEnvironment& env) {
  /*
   * You could read information from the environment to decide whether
   * the user has permission to start a new application
   */
  return new PersonalPage(env);
}

PersonalPage::PersonalPage( const WEnvironment& env ):
    WApplication( env ),
    header( this ),
    data( this ),
    footer( this )
{

    this->useStyleSheet( "/resources/css/PersonalPage.css" );
    authorized = false;

    WContainerWidget* wLoginBox = buildLoginPage( env );
    root()->addWidget( wLoginBox );
}

WContainerWidget* PersonalPage::buildLoginPage( const WEnvironment &env ) {
    setTitle( WString::fromUTF8("GreenSMS: Страница авторизации") );
    setCssTheme("polished");

    WString failedMsg = WString::fromUTF8( "Логин или пароль неверны" );
    WString greetMsg = WString::fromUTF8( "Представьтесь, пожалуйста" );
    WString unameMsg = WString::fromUTF8( "Ваш логин" );
    WString passMsg = WString::fromUTF8( "Ваш пароль" );
    WString loginMsg = WString::fromUTF8( "Вход" );

    WLabel* failedMsgLabel = new WLabel( failedMsg );
    WLabel* greetMsgLabel = new WLabel( greetMsg );
    WHintLineEdit* loginBox = new WHintLineEdit( unameMsg );
    WHintLinePassEdit* passBox = new WHintLinePassEdit( passMsg );
    WPushButton* loginBtn = new WPushButton( loginMsg );

    failedMsgLabel->setObjectName( "wLoginBlockFailedMsgLabel" );
    failedMsgLabel->setStyleClass( "wLoginBlockFailedMsgLabel" );
    failedMsgLabel->setHidden( true );

    loginBox->setObjectName( "wLoginBlockLoginBox" );
    loginBox->enterPressed().connect(SLOT(passBox, WHintLinePassEdit::setFocus));

    passBox->setObjectName( "wLoginBlockPassBox" );
    passBox->enterPressed().connect(SLOT(this, PersonalPage::onLogin));

    loginBtn->setMaximumSize( WLength( 50, WLength::Percentage ), WLength::Auto );
    loginBtn->clicked().connect(SLOT(this, PersonalPage::onLogin));

    WVBoxLayout *wLoginGrp = new WVBoxLayout();
    wLoginGrp->addWidget( greetMsgLabel );
    wLoginGrp->addWidget( loginBox );
    wLoginGrp->addWidget( passBox );
    wLoginGrp->addWidget( failedMsgLabel );
    wLoginGrp->addWidget( loginBtn );
    WContainerWidget* wLoginBlock = new WContainerWidget();
    wLoginBlock->setObjectName( "wLoginBlock" );
    wLoginBlock->setLayout( wLoginGrp, AlignMiddle | AlignCenter );

    return wLoginBlock;
}

void PersonalPage::onLogin() {
    authorized = false;
    isAdmin = false;

    WContainerWidget* wLoginBlock = static_cast<WContainerWidget*>( root()->find( "wLoginBlock" ) );
    if ( !wLoginBlock )
        return;

    WHintLineEdit* loginBox = static_cast<WHintLineEdit*>( root()->find( "wLoginBlockLoginBox" ) );
    if ( !loginBox )
        return;

    WHintLinePassEdit* passBox = static_cast<WHintLinePassEdit*>( root()->find( "wLoginBlockPassBox" ) );
    if ( !passBox )
        return;

    WLabel* failedMsgLabel = static_cast<WLabel*>( root()->find( "wLoginBlockFailedMsgLabel" ) );
    if ( !failedMsgLabel )
        return;

    try {
        PartnerInfo p = PartnerManager::get_mutable_instance().findByName( loginBox->text().toUTF8() );
        authorized = ( p.pPass == passBox->text().toUTF8() );
        pId = p.pId;
        isAdmin = p.isAdmin();
    } catch ( ... ) {
        return;
    }

    if ( !authorized ) {
        failedMsgLabel->setHidden( false );
        passBox->setText( "" );
        passBox->setFocus( );
    } else {
        root()->removeWidget( wLoginBlock );
        buildPersonalPage();
    }
}

void PersonalPage::onReportBtnClicked(
        WLineEdit* pid,
        WLineEdit* phone,
        WDatePicker* date_from,
        WDatePicker* date_to,
        std::pair<WLineEdit*, WLineEdit*>  text_country,
        std::pair<WComboBox*, WSpinBox*>  status_page,
        WPushButton* reportbtn,
        WLabel* reportstatus ) {

    data.resetFilter();

    if ( pid && isAdmin && !pid->text().empty() )
        data.setPidFilter( pid->text().toUTF8() );

    if ( !isAdmin )
        data.setPidFilter( pId );

    if ( !phone->text().empty() )
        data.setPhoneFilter( phone->text().toUTF8() );

    if ( !text_country.first->text().empty() )
        data.setTextFilter( text_country.first->text().toUTF8() );

    if ( !text_country.second->text().empty() )
        data.setCountryFilter( text_country.second->text().toUTF8() );

    std::string _ldate = date_from->date().toString("yyyy/MM/dd").toUTF8();
    if ( !_ldate.empty() ) {
        boost::gregorian::date date = boost::gregorian::from_string( _ldate );
        boost::gregorian::date orig( 1970, boost::gregorian::Jan, 1 );
        boost::posix_time::ptime from( date, boost::posix_time::hours(0) );
        boost::posix_time::ptime begin( orig, boost::posix_time::hours(0) );
        boost::posix_time::time_period lv( begin, from );

        data.setDateFromFilter( lv.length().total_seconds()-3*60*60 );
    }

    std::string _rdate = date_to->date().toString("yyyy/MM/dd").toUTF8();
    if ( !_rdate.empty() ) {
        boost::gregorian::date date = boost::gregorian::from_string( _rdate );
        boost::gregorian::date orig( 1970, boost::gregorian::Jan, 1 );
        boost::posix_time::ptime from( date, boost::posix_time::hours(0) );
        boost::posix_time::ptime to = from + boost::posix_time::hours(24);
        boost::posix_time::ptime begin( orig, boost::posix_time::hours(0) );
        boost::posix_time::time_period rv( begin, to );

        data.setDateToFilter( rv.length().total_seconds()-3*60*60 );
    }

    if ( status_page.first->currentIndex() == 0 ) {
        // Ничего не делаем
    }

    if ( status_page.first->currentIndex() == 1 ) {
        data.setStatusFilter( SMSMessage::Status( SMSMessage::Status::ST_DELIVERED ) );
    }

    if ( status_page.first->currentIndex() == 2 ) {
        data.setStatusFilter( SMSMessage::Status( SMSMessage::Status::ST_NOT_DELIVERED ) );
    }

    if ( status_page.first->currentIndex() == 3 ) {
        data.setStatusFilter( SMSMessage::Status( SMSMessage::Status::ST_REJECTED ) );
    }


    reportbtn->disable();
    reportstatus->setText(WString::fromUTF8("Обработка"));
    this->processEvents();

    try {
        data.prepareRequest();
    } catch ( PGBrokenConnection& err ) {
        reportbtn->enable();
        reportstatus->setText(WString::fromUTF8(string("Ошибка соеднинения с базой") + err.what()));
        return;
    } catch ( PGSqlError& err ) {
        reportbtn->enable();
        reportstatus->setText(WString::fromUTF8(string("Ошибка SQL запроса") + err.what()));
        return;
    }

    statistics->rebuildData();

    reportbtn->enable();
    reportstatus->setText(WString::fromUTF8(string("Готово: ") + boost::lexical_cast<string>( statistics->getLastPage() + 1 ) + string(" страниц")));
    if ( data.getTotalLines() ) {
        reportstatus->setStyleClass("link");
    } else {
        reportstatus->setStyleClass("");
    }

    lastColumnLength = reportstatus->width();
    statistics->columnAt( statistics->columnCount() - 1 )->setWidth( lastColumnLength );

    onPageUpdate( status_page.second );
}

void PersonalPage::onSummaryShow() {
    if ( !data.getTotalLines() ) {
        return;
    }

    WDialog summary;
    summary.setWindowTitle( WString::fromUTF8("Итоги") );
    summary.setTitleBarEnabled( true );

    double price;
    int total, delivered, rejected, undelivered;

    WTable report( summary.contents() );
    report.setStyleClass("restable");
    report.elementAt(0, 0)->addWidget( new WLabel( WString::fromUTF8("Всего сообщений") ) );
    report.elementAt(1, 0)->addWidget( new WLabel( WString::fromUTF8("Доставлено") ) );
    report.elementAt(2, 0)->addWidget( new WLabel( WString::fromUTF8("Отказ в передаче") ) );
    report.elementAt(3, 0)->addWidget( new WLabel( WString::fromUTF8("Не доставлено") ) );
    report.elementAt(4, 0)->addWidget( new WLabel( WString::fromUTF8("Общей стоимостью") ) );

    data.evaluateSummary(price, total, delivered, rejected, undelivered);
    char price_str[100];
    sprintf( price_str, "%0.2f", price );
    report.elementAt(0, 1)->addWidget( new WLabel( WString::fromUTF8( boost::lexical_cast<std::string>( total ) ) ) );
    report.elementAt(1, 1)->addWidget( new WLabel( WString::fromUTF8( boost::lexical_cast<std::string>( delivered ) ) ) );
    report.elementAt(2, 1)->addWidget( new WLabel( WString::fromUTF8( boost::lexical_cast<std::string>( rejected ) ) ) );
    report.elementAt(3, 1)->addWidget( new WLabel( WString::fromUTF8( boost::lexical_cast<std::string>( undelivered ) ) ) );
    report.elementAt(4, 1)->addWidget( new WLabel( WString::fromUTF8( std::string( price_str ) + " руб" ) ) );

    WPushButton closeBtn( WString::fromUTF8("ОК"), summary.contents() );
    closeBtn.clicked().connect(&summary, &WDialog::accept);

    summary.exec();


}

void PersonalPage::onPageUpdate( WSpinBox* page ) {
    page->setRange( 1, statistics->getLastPage() + 1 );
    page->setValue( statistics->getPage() + 1 );
}

void PersonalPage::onPageInc( WSpinBox* page ) {
    page->setValue( page->value() + 1 );
    onPageUpdate( page );
}

void PersonalPage::onPageDec( WSpinBox* page ) {
    page->setValue( page->value() - 1 );
    onPageUpdate( page );
}

void PersonalPage::widthCorrect( ) {
    statistics->columnAt( statistics->columnCount() - 1 )->setWidth( lastColumnLength );
}

void PersonalPage::buildPersonalPage( ) {
    setTitle( WString::fromUTF8("GreenSMS: Личный кабинет ") );
    setCssTheme("polished");

    this->useStyleSheet( "/resources/css/PersonalPage.css" );
    isAdmin = false;
    authorized = false;

    statistics = new WScrollTable( header, data, footer );
    statistics->setMinimumSize( WLength( 100, WLength::Percentage), WLength::Auto );
    statistics->setMaximumSize( WLength( 100, WLength::Percentage), WLength::Auto );
    statistics->addStyleClass("restable");
    statistics->buildHeader();
    statistics->buildData();
    statistics->buildFooter();
    root()->addWidget( statistics );
}

