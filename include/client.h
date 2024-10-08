#ifndef CLIENT_H
#define CLIENT_H

#include <stdbool.h>
#include <libpq-fe.h>


typedef struct ClientData* ClientDataPtr;
ClientDataPtr client_create(void);

void client_set_code(ClientDataPtr client, const char* code);
void client_set_active(ClientDataPtr client, bool active);
void client_set_address_id(ClientDataPtr client, int address_id);
void client_set_contact_id(ClientDataPtr client, int contact_id);
void client_set_territory_id(ClientDataPtr client, int territory_id);
void client_set_rep_id(ClientDataPtr client, int rep_id);
void client_set_account_code(ClientDataPtr client, const char* account_code);
void client_set_status(ClientDataPtr client, const char* status);
void client_set_contact_name_id(ClientDataPtr client, int contact_name_id);
void client_set_contact_title_id(ClientDataPtr client, int contact_title_id);
void client_set_name_id(ClientDataPtr client, int name_id);

bool client_insert(PGconn *db_conn, ClientDataPtr client);

int client_get_id(ClientDataPtr client);
void client_free(ClientDataPtr client);

bool client_fetch_and_insert(PGconn *db_conn, long last_timestamp);

#endif 