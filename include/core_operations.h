#ifndef CORE_OPERATIONS_H
#define CORE_OPERATIONS_H

int get_or_create_address(PGconn *conn, const char *street, const char *zip, const char *city, const char *state, const char *country);

// Placeholders

int get_or_create_contact_info(PGconn *conn, const char *phone, const char *mobile, const char *website);
int get_or_create_territory(PGconn *conn, const char *territory_name);
int get_or_create_representative(PGconn *conn, const char *rep_code, const char *rep_name);
int get_or_create_name(PGconn *conn, const char *name);


#endif // CORE_OPERATIONS_H
