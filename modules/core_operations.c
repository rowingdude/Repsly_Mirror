#include "core_operations.h"
#include <libpq-fe.h>
#include <string.h>
#include <stdio.h>


// Unified cursor handling to modularize the helper functions a bit further...

static int execute_int_query(PGconn *conn, const char *query, int n_params, const char **param_values) {
    int param_lengths[n_params];
    int param_formats[n_params];
    for (int i = 0; i < n_params; i++) {
        param_lengths[i] = strlen(param_values[i]);
        param_formats[i] = 0;  // text format
    }

    PGresult *res = PQexecParams(conn, query, n_params, NULL, param_values, param_lengths, param_formats, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Query failed: %s", PQerrorMessage(conn));
        PQclear(res);
        return -1;
    }

    int result = -1;
    if (PQntuples(res) > 0) {
        result = atoi(PQgetvalue(res, 0, 0));
    }

    PQclear(res);
    return result;
}


// These are to help us with referential integrity. The basic idea of an UPSERT is to:

//   1. Check to see if there is a record that matches our insertion value
//   2. If the record exists, we return the ID value immediately
//   3. If it does not exist, we insert the new record and then return the value

// The premise here is to cut down on as much redundant data as possible!


int get_or_create_address(PGconn *conn, const char *street, const char *zip, const char *city, const char *state, const char *country) {
    const char *query =
        "WITH new_address AS ("
        "    INSERT INTO core.addresses (street_address, zip_code, city, state, country) "
        "    VALUES ($1, $2, $3, $4, $5) "
        "    ON CONFLICT (street_address, zip_code, city, state, country) DO NOTHING "
        "    RETURNING address_id"
        ")"
        "SELECT address_id FROM new_address "
        "UNION ALL "
        "SELECT address_id FROM core.addresses "
        "WHERE street_address = $1 AND zip_code = $2 AND city = $3 AND state = $4 AND country = $5 "
        "LIMIT 1";

    const char *param_values[] = {street, zip, city, state, country};
    return execute_int_query(conn, query, 5, param_values);
}

int get_or_create_contact_info(PGconn *conn, const char *phone, const char *mobile, const char *website) {
    const char *query =
        "WITH new_contact AS ("
        "    INSERT INTO core.contact_info (phone, mobile, website) "
        "    VALUES ($1, $2, $3) "
        "    ON CONFLICT (phone, mobile, website) DO NOTHING "
        "    RETURNING contact_id"
        ")"
        "SELECT contact_id FROM new_contact "
        "UNION ALL "
        "SELECT contact_id FROM core.contact_info "
        "WHERE phone = $1 AND mobile = $2 AND website = $3 "
        "LIMIT 1";

    const char *param_values[] = {phone, mobile, website};
    return execute_int_query(conn, query, 3, param_values);
}

int get_or_create_territory(PGconn *conn, const char *territory_name) {
    const char *query =
        "WITH new_territory AS ("
        "    INSERT INTO core.territories (name) "
        "    VALUES ($1) "
        "    ON CONFLICT (name) DO NOTHING "
        "    RETURNING territory_id"
        ")"
        "SELECT territory_id FROM new_territory "
        "UNION ALL "
        "SELECT territory_id FROM core.territories "
        "WHERE name = $1 "
        "LIMIT 1";

    const char *param_values[] = {territory_name};
    return execute_int_query(conn, query, 1, param_values);
}

int get_or_create_representative(PGconn *conn, const char *rep_code, const char *rep_name) {
    const char *query =
        "WITH new_rep AS ("
        "    INSERT INTO field_ops.representatives (rep_code, name) "
        "    VALUES ($1, $2) "
        "    ON CONFLICT (rep_code) DO NOTHING "
        "    RETURNING rep_id"
        ")"
        "SELECT rep_id FROM new_rep "
        "UNION ALL "
        "SELECT rep_id FROM field_ops.representatives "
        "WHERE rep_code = $1 "
        "LIMIT 1";

    const char *param_values[] = {rep_code, rep_name};
    return execute_int_query(conn, query, 2, param_values);
}

int get_or_create_name(PGconn *conn, const char *name) {
    const char *query =
        "WITH new_name AS ("
        "    INSERT INTO core.names (full_name) "
        "    VALUES ($1) "
        "    ON CONFLICT (full_name) DO NOTHING "
        "    RETURNING name_id"
        ")"
        "SELECT name_id FROM new_name "
        "UNION ALL "
        "SELECT name_id FROM core.names "
        "WHERE full_name = $1 "
        "LIMIT 1";

    const char *param_values[] = {name};
    return execute_int_query(conn, query, 1, param_values);
}

int get_or_create_visit(PGconn *conn, const char *visit_start, const char *visit_end, const char *rep_code, const char *client_code) {
    const char *query =
        "WITH new_visit AS ("
        "    INSERT INTO field_ops.visits (time_start_id, time_end_id, rep_id, client_id) "
        "    VALUES ("
        "        (SELECT time_id FROM meta.time WHERE timestamp = $1::timestamp), "
        "        (SELECT time_id FROM meta.time WHERE timestamp = $2::timestamp), "
        "        (SELECT rep_id FROM field_ops.representatives WHERE rep_code = $3), "
        "        (SELECT client_id FROM sales.clients WHERE code = $4)"
        "    ) "
        "    ON CONFLICT (time_start_id, rep_id, client_id) DO NOTHING "
        "    RETURNING visit_id"
        ")"
        "SELECT visit_id FROM new_visit "
        "UNION ALL "
        "SELECT v.visit_id FROM field_ops.visits v "
        "JOIN meta.time ts ON v.time_start_id = ts.time_id "
        "JOIN field_ops.representatives r ON v.rep_id = r.rep_id "
        "JOIN sales.clients c ON v.client_id = c.client_id "
        "WHERE ts.timestamp = $1::timestamp AND r.rep_code = $3 AND c.code = $4 "
        "LIMIT 1";

    const char *param_values[] = {visit_start, visit_end, rep_code, client_code};
    return execute_int_query(conn, query, 4, param_values);
}

int get_or_create_time(PGconn *conn, const char *timestamp) {
    const char *query =
        "WITH new_time AS ("
        "    INSERT INTO meta.time (timestamp) "
        "    VALUES ($1::timestamp) "
        "    ON CONFLICT (timestamp) DO NOTHING "
        "    RETURNING time_id"
        ")"
        "SELECT time_id FROM new_time "
        "UNION ALL "
        "SELECT time_id FROM meta.time "
        "WHERE timestamp = $1::timestamp "
        "LIMIT 1";

    const char *param_values[] = {timestamp};
    return execute_int_query(conn, query, 1, param_values);
}

int get_or_create_date(PGconn *conn, const char *date) {
    const char *query =
        "WITH new_date AS ("
        "    INSERT INTO meta.date (date) "
        "    VALUES ($1::date) "
        "    ON CONFLICT (date) DO NOTHING "
        "    RETURNING date_id"
        ")"
        "SELECT date_id FROM new_date "
        "UNION ALL "
        "SELECT date_id FROM meta.date "
        "WHERE date = $1::date "
        "LIMIT 1";

    const char *param_values[] = {date};
    return execute_int_query(conn, query, 1, param_values);
}

int get_or_create_note(PGconn *conn, const char *note_text) {
    const char *query =
        "WITH new_note AS ("
        "    INSERT INTO meta.notes (note_text) "
        "    VALUES ($1) "
        "    ON CONFLICT (note_text) DO NOTHING "
        "    RETURNING note_id"
        ")"
        "SELECT note_id FROM new_note "
        "UNION ALL "
        "SELECT note_id FROM meta.notes "
        "WHERE note_text = $1 "
        "LIMIT 1";

    const char *param_values[] = {note_text};
    return execute_int_query(conn, query, 1, param_values);
}

int get_or_create_lat(PGconn *conn, double latitude) {
    char lat_str[20];
    snprintf(lat_str, sizeof(lat_str), "%f", latitude);

    const char *query =
        "WITH new_lat AS ("
        "    INSERT INTO geo.lat (latitude) "
        "    VALUES ($1::decimal) "
        "    ON CONFLICT (latitude) DO NOTHING "
        "    RETURNING lat_id"
        ")"
        "SELECT lat_id FROM new_lat "
        "UNION ALL "
        "SELECT lat_id FROM geo.lat "
        "WHERE latitude = $1::decimal "
        "LIMIT 1";

    const char *param_values[] = {lat_str};
    return execute_int_query(conn, query, 1, param_values);
}

int get_or_create_long(PGconn *conn, double longitude) {
    char long_str[20];
    snprintf(long_str, sizeof(long_str), "%f", longitude);

    const char *query =
        "WITH new_long AS ("
        "    INSERT INTO geo.long (longitude) "
        "    VALUES ($1::decimal) "
        "    ON CONFLICT (longitude) DO NOTHING "
        "    RETURNING long_id"
        ")"
        "SELECT long_id FROM new_long "
        "UNION ALL "
        "SELECT long_id FROM geo.long "
        "WHERE longitude = $1::decimal "
        "LIMIT 1";

    const char *param_values[] = {long_str};
    return execute_int_query(conn, query, 1, param_values);
}