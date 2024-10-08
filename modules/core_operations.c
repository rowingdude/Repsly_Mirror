#include "core_operations.h"
#include <libpq-fe.h>
#include <string.h>
#include <stdio.h>

int get_or_create_address(PGconn *conn, const char *street, const char *zip, const char *city, const char *state, const char *country) {
    char query[1024];
    snprintf(query, sizeof(query),
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
        "LIMIT 1");

    const char *param_values[] = {street, zip, city, state, country};
    int param_lengths[] = {strlen(street), strlen(zip), strlen(city), strlen(state), strlen(country)};
    int param_formats[] = {0, 0, 0, 0, 0};  // all text

    PGresult *res = PQexecParams(conn, query, 5, NULL, param_values, param_lengths, param_formats, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "get_or_create_address failed: %s", PQerrorMessage(conn));
        PQclear(res);
        return -1;
    }

    int address_id = atoi(PQgetvalue(res, 0, 0));
    PQclear(res);
    return address_id;
}

int get_or_create_contact_info(PGconn *conn, const char *phone, const char *mobile, const char *website) {
    // Similar implementation to get_or_create_address
}

int get_or_create_territory(PGconn *conn, const char *territory_name) {
    // Similar implementation
}

int get_or_create_representative(PGconn *conn, const char *rep_code, const char *rep_name) {
    // Similar implementation
}

int get_or_create_name(PGconn *conn, const char *name) {
    // Similar implementation
}