CREATE TABLE message_status (
    "REQUESTID" BIGINT NOT NULL,
    "MESSAGEID" INTEGER NOT NULL,
    "STATUS" INTEGER NOT NULL,
    "TO" TEXT NOT NULL,
    "PARTS" INTEGER NOT NULL,
    "PARTNERPRICE" REAL DEFAULT 0,
    "OURPRICE" REAL DEFAULT 0,
    "GATEWAY" TEXT NOT NULL,
    "COUNTRY" TEXT,
    "COUNTRYCODE" TEXT,
    "OPERATOR" TEXT,
    "OPERATORCODE" TEXT,
    "REGION" TEXT,
    "WHEN" INTEGER
);
CREATE INDEX message_status_REQUESTID on message_status( "REQUESTID" );
CREATE INDEX message_status_WHEN on message_status( "WHEN" );
CREATE INDEX message_status_TO on message_status( "TO" );
CREATE INDEX message_status_COUNTRY on message_status( "COUNTRY" );
CREATE INDEX message_status_COUNTRYCODE on message_status( "COUNTRYCODE" );
CREATE INDEX message_status_OPERATOR on message_status( "OPERATOR" );
CREATE INDEX message_status_OPERATORCODE on message_status( "OPERATORCODE" );
CREATE INDEX message_status_REGION on message_status( "REGION" );

CREATE TABLE message_history (
    "REQUESTID" BIGINT NOT NULL,
    "MESSAGEID" INTEGER NOT NULL,
    "OP_CODE" INTEGER NOT NULL,
    "OP_DIRECTION" INTEGER NOT NULL,
    "OP_RESULT" INTEGER NOT NULL,
    "GATEWAY" TEXT NOT NULL,
    "WHEN" INTEGER
);
CREATE INDEX message_history_REQUESTID on message_history( "REQUESTID" );
CREATE INDEX message_history_WHEN on message_history( "WHEN" );
CREATE INDEX message_history_GATEWAY on message_history( "GATEWAY" );
CREATE INDEX message_history_WHEN_GATEWAY on message_history( "WHEN", "GATEWAY" );

CREATE TABLE smsrequest (
    "REQUESTID" BIGINT PRIMARY KEY NOT NULL,
    "USER" TEXT NOT NULL,
    "PASS" TEXT NOT NULL,
    "TO" TEXT NOT NULL,
    "TXT" TEXT NOT NULL,
    "TID" TEXT,
    "FROM" TEXT,
    "UTF" INTEGER,
    "SUBPREF" TEXT,
    "HEX" INTEGER,
    "UDH" TEXT,
    "DELAY" INTEGER,
    "DLR" INTEGER,
    "PID" TEXT,
    "PRIORITY" INTEGER,
    "GARANT" INTEGER,
    "WHEN" INTEGER
);
CREATE INDEX smsrequest_REQUESTID on smsrequest( "REQUESTID" );
CREATE INDEX smsrequest_WHEN on smsrequest( "WHEN" );
CREATE INDEX smsrequest_TXT on smsrequest( "TXT" );
CREATE INDEX smsrequest_FROM on smsrequest( "FROM" );
CREATE INDEX smsrequest_TO on smsrequest( "TO" );
CREATE INDEX smsrequest_PID on smsrequest( "PID" );

CREATE TABLE gateways (
    "gName" TEXT PRIMARY KEY NOT NULL,
    "uName" TEXT NOT NULL,
    "uPass" TEXT NOT NULL,
    "gPort" INTEGER NOT NULL,
    "gPriority" INTEGER NOT NULL,
    "gEnabled" INTEGER NOT NULL,
    "gRule" TEXT,
    "gOptions" TEXT
);
CREATE INDEX gateways_gName on gateways( "gName" );

CREATE TABLE dlrs (
    oid INTEGER PRIMARY KEY,
    smsc TEXT,
    ts TEXT,
    source TEXT,
    destination TEXT,
    service TEXT,
    url TEXT,
    mask TEXT,
    status TEXT,
    boxc TEXT
);
CREATE INDEX dlrs_smsc on dlrs( smsc );
CREATE INDEX dlrs_ts on dlrs( ts );
CREATE INDEX dlrs_source on dlrs( source );
CREATE INDEX dlrs_destination on dlrs( destination );
CREATE INDEX dlrs_service on dlrs( service );
CREATE INDEX dlrs_url on dlrs( url );
CREATE INDEX dlrs_mask on dlrs( mask );
CREATE INDEX dlrs_status on dlrs( status );
CREATE INDEX dlrs_boxc on dlrs( boxc );

CREATE LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION insert_dlr() RETURNS trigger AS $$
BEGIN
  new.added = NOW();
  RETURN new;
END
$$ LANGUAGE plpgsql;

CREATE TRIGGER insert_price_change BEFORE INSERT ON dlrs FOR EACH ROW EXECUTE PROCEDURE insert_dlr();

CREATE TABLE partners (
    pid         INTEGER PRIMARY KEY,
    uname       TEXT NOT NULL,
    pass        TEXT NOT NULL,
    cname       TEXT NOT NULL,
    manager     TEXT NOT NULL,
    phone       TEXT NOT NULL DEFAULT '',
    contact     TEXT NOT NULL DEFAULT '',
    tariff	TEXT NOT NULL DEFAULT '',
    balance     REAL DEFAULT 20,
    credit      REAL DEFAULT 0,
    plimit      INTEGER DEFAULT 60,
    postplay    BOOL DEFAULT FALSE,
    trial       BOOL DEFAULT FALSE,
    priority    INTEGER DEFAULT 0
);

CREATE TABLE matviews (
  mv_name NAME NOT NULL PRIMARY KEY
  , v_name NAME NOT NULL
  , last_refresh TIMESTAMP WITH TIME ZONE
);

CREATE OR REPLACE FUNCTION create_matview(NAME, NAME)
RETURNS VOID
SECURITY DEFINER
LANGUAGE plpgsql AS '
DECLARE
    matview ALIAS FOR $1;
    view_name ALIAS FOR $2;
    entry matviews%ROWTYPE;
BEGIN
    SELECT * INTO entry FROM matviews WHERE mv_name = matview;

    IF FOUND THEN
        RAISE EXCEPTION ''Materialized view ''''%'''' already exists.'',
          matview;
    END IF;

    EXECUTE ''REVOKE ALL ON '' || view_name || '' FROM PUBLIC''; 

    EXECUTE ''GRANT SELECT ON '' || view_name || '' TO PUBLIC'';

    EXECUTE ''CREATE TABLE '' || matview || '' AS SELECT * FROM '' || view_name;

    EXECUTE ''REVOKE ALL ON '' || matview || '' FROM PUBLIC'';

    EXECUTE ''GRANT SELECT ON '' || matview || '' TO PUBLIC'';

    INSERT INTO matviews (mv_name, v_name, last_refresh)
      VALUES (matview, view_name, CURRENT_TIMESTAMP); 
    
    RETURN;
END
';

CREATE OR REPLACE FUNCTION drop_matview(NAME) RETURNS VOID
SECURITY DEFINER
LANGUAGE plpgsql AS '
DECLARE
    matview ALIAS FOR $1;
    entry matviews%ROWTYPE;
BEGIN

    SELECT * INTO entry FROM matviews WHERE mv_name = matview;

    IF NOT FOUND THEN
        RAISE EXCEPTION ''Materialized view % does not exist.'', matview;
    END IF;

    EXECUTE ''DROP TABLE '' || matview;
    DELETE FROM matviews WHERE mv_name=matview;

    RETURN;
END
';

CREATE OR REPLACE FUNCTION ts2int(timestamp without time zone) RETURNS int AS
$$
select extract('epoch' from $1)::integer;
$$ LANGUAGE SQL STRICT STABLE;

CREATE OR REPLACE FUNCTION int2ts(integer) RETURNS timestamp AS
$$
SELECT ( TIMESTAMP WITH TIME ZONE 'epoch' + $1 * INTERVAL '1second')::timestamp without time zone;
$$ LANGUAGE SQL STRICT STABLE;


