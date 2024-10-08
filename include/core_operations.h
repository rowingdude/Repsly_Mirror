#ifndef CORE_OPERATIONS_H
#define CORE_OPERATIONS_H

long get_last_processed(PGconn *conn, const char *entity_name);
bool update_last_processed(PGconn *conn, const char *entity_name, long last_value);

PGconn* db_connect(void);
void db_disconnect(PGconn *conn);

int get_or_create_address(PGconn *conn, const char *street, const char *zip, const char *city, const char *state, const char *country);
int get_or_create_contact_info(PGconn *conn, const char *phone, const char *mobile, const char *website);
int get_or_create_territory(PGconn *conn, const char *territory_name);
int get_or_create_representative(PGconn *conn, const char *rep_code, const char *rep_name);
int get_or_create_name(PGconn *conn, const char *name);
int get_or_create_long(PGconn *conn, double longitude);
int get_or_create_lat(PGconn *conn, double latitude);
int get_or_create_note(PGconn *conn, const char *note_text);
int get_or_create_date(PGconn *conn, const char *date);
int get_or_create_visit(PGconn *conn, const char *visit_start, const char *visit_end, const char *rep_code, const char *client_code);
int get_or_create_time(PGconn *conn, const char *timestamp);

int get_or_create_product(PGconn *conn, const char *product_code, const char *product_name);
int get_or_create_client(PGconn *conn, const char *client_code, const char *client_name);



#endif // CORE_OPERATIONS_H
