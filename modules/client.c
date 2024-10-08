#include "../include/client.h"
#include "../include/api.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <jansson.h>

struct ClientData {
    int client_id;
    char code[51];
    bool active;
    int address_id;
    int contact_id;
    int territory_id;
    int rep_id;
    char account_code[256];
    char status[256];
    int contact_name_id;
    int contact_title_id;
    int name_id;
};

ClientDataPtr client_create(void) {
    return (ClientDataPtr)calloc(1, sizeof(struct ClientData));
}

void client_set_code(ClientDataPtr client, const char* code) {
    strncpy(client->code, code, sizeof(client->code) - 1);
    client->code[sizeof(client->code) - 1] = '\0';
}
void client_set_active(ClientDataPtr client, bool active) {
    client->active = active;
}

void client_set_address_id(ClientDataPtr client, int address_id) {
    client->address_id = address_id;
}

void client_set_contact_id(ClientDataPtr client, int contact_id) {
    client->contact_id = contact_id;
}

void client_set_territory_id(ClientDataPtr client, int territory_id) {
    client->territory_id = territory_id;
}

void client_set_rep_id(ClientDataPtr client, int rep_id) {
    client->rep_id = rep_id;
}

void client_set_account_code(ClientDataPtr client, const char* account_code) {
    strncpy(client->account_code, account_code, sizeof(client->account_code) - 1);
    client->account_code[sizeof(client->account_code) - 1] = '\0';
}

void client_set_status(ClientDataPtr client, const char* status) {
    strncpy(client->status, status, sizeof(client->status) - 1);
    client->status[sizeof(client->status) - 1] = '\0';
}

void client_set_contact_name_id(ClientDataPtr client, int contact_name_id) {
    client->contact_name_id = contact_name_id;
}

void client_set_contact_title_id(ClientDataPtr client, int contact_title_id) {
    client->contact_title_id = contact_title_id;
}

void client_set_name_id(ClientDataPtr client, int name_id) {
    client->name_id = name_id;
}

bool client_insert(PGconn *db_conn, ClientDataPtr client) {
    int address_id = get_or_create_address(db_conn, client->street_address, client->zip, client->city, client->state, client->country);
    int contact_id = get_or_create_contact_info(db_conn, client->phone, client->mobile, client->website);
    int territory_id = get_or_create_territory(db_conn, client->territory);
    int rep_id = get_or_create_representative(db_conn, client->rep_code, client->rep_name);
    int contact_name_id = get_or_create_name(db_conn, client->contact_name);
    int contact_title_id = get_or_create_name(db_conn, client->contact_title);
    int name_id = get_or_create_name(db_conn, client->name);

    if (address_id < 0 || contact_id < 0 || territory_id < 0 || rep_id < 0 || 
        contact_name_id < 0 || contact_title_id < 0 || name_id < 0) {
        fprintf(stderr, "Failed to get or create referenced data for client\n");
        return false;
    }

    const char *insert_client_query = 
        "INSERT INTO sales.clients "
        "(code, active, address_id, contact_id, territory_id, rep_id, account_code, status, contact_name_id, contact_title_id, name_id) "
        "VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11) "
        "RETURNING client_id";

    const char *param_values[11];
    int param_lengths[11];
    int param_formats[11] = {0};  
    char active_str[6];
    char id_str[9][20]; 

    param_values[0] = client->code;
    snprintf(active_str, sizeof(active_str), "%s", client->active ? "true" : "false");
    param_values[1] = active_str;
    snprintf(id_str[0], sizeof(id_str[0]), "%d", client->address_id);
    param_values[2] = id_str[0];
    snprintf(id_str[1], sizeof(id_str[1]), "%d", client->contact_id);
    param_values[3] = id_str[1];
    snprintf(id_str[2], sizeof(id_str[2]), "%d", client->territory_id);
    param_values[4] = id_str[2];
    snprintf(id_str[3], sizeof(id_str[3]), "%d", client->rep_id);
    param_values[5] = id_str[3];
    param_values[6] = client->account_code;
    param_values[7] = client->status;
    snprintf(id_str[4], sizeof(id_str[4]), "%d", client->contact_name_id);
    param_values[8] = id_str[4];
    snprintf(id_str[5], sizeof(id_str[5]), "%d", client->contact_title_id);
    param_values[9] = id_str[5];
    snprintf(id_str[6], sizeof(id_str[6]), "%d", client->name_id);
    param_values[10] = id_str[6];

    for (int i = 0; i < 11; i++) {
        param_lengths[i] = strlen(param_values[i]);
    }

    PGresult *result = PQexecParams(db_conn, insert_client_query, 11, NULL, param_values, param_lengths, param_formats, 0);

    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        fprintf(stderr, "INSERT INTO sales.clients failed: %s", PQerrorMessage(db_conn));
        PQclear(result);
        return false;
    }

    char *client_id_str = PQgetvalue(result, 0, 0);
    client->client_id = atoi(client_id_str);

    PQclear(result);
    return true;
}

int client_get_id(ClientDataPtr client) {
    return client->client_id;
}

void client_free(ClientDataPtr client) {
    free(client);
}


static ClientDataPtr client_from_json(json_t *client_json) {
    ClientDataPtr client = client_create();
    
    client_set_code(client, json_string_value(json_object_get(client_json, "Code")));
    client_set_active(client, json_is_true(json_object_get(client_json, "Active")));
    client_set_name_id(client, json_integer_value(json_object_get(client_json, "Name")));
    client_set_tag(client, json_string_value(json_object_get(client_json, "Tag")));
    client_set_territory(client, json_string_value(json_object_get(client_json, "Territory")));
    client_set_rep_code(client, json_string_value(json_object_get(client_json, "RepresentativeCode")));
    client_set_rep_name(client, json_string_value(json_object_get(client_json, "RepresentativeName")));
    client_set_street_address(client, json_string_value(json_object_get(client_json, "StreetAddress")));
    client_set_zip(client, json_string_value(json_object_get(client_json, "ZIP")));
    client_set_zip_ext(client, json_string_value(json_object_get(client_json, "ZIPExt")));
    client_set_city(client, json_string_value(json_object_get(client_json, "City")));
    client_set_state(client, json_string_value(json_object_get(client_json, "State")));
    client_set_country(client, json_string_value(json_object_get(client_json, "Country")));
    client_set_email(client, json_string_value(json_object_get(client_json, "Email")));
    client_set_phone(client, json_string_value(json_object_get(client_json, "Phone")));
    client_set_mobile(client, json_string_value(json_object_get(client_json, "Mobile")));
    client_set_website(client, json_string_value(json_object_get(client_json, "Website")));
    client_set_contact_name(client, json_string_value(json_object_get(client_json, "ContactName")));
    client_set_contact_title(client, json_string_value(json_object_get(client_json, "ContactTitle")));
    client_set_note(client, json_string_value(json_object_get(client_json, "Note")));
    client_set_status(client, json_string_value(json_object_get(client_json, "Status")));
    client_set_account_code(client, json_string_value(json_object_get(client_json, "AccountCode")));

    json_t *custom_fields = json_object_get(client_json, "CustomFields");
    if (json_is_array(custom_fields)) {
        size_t index;
        json_t *field;
        json_array_foreach(custom_fields, index, field) {
            const char *field_name = json_string_value(json_object_get(field, "Field"));
            const char *field_value = json_string_value(json_object_get(field, "Value"));
            client_add_custom_field(client, field_name, field_value);
        }
    }

    // ******* NEED TO ADD PRICE LISTS BEFORE RUNNING THIS!!!! ************

    json_t *price_lists = json_object_get(client_json, "PriceLists");
    if (json_is_array(price_lists)) {
        size_t index;
        json_t *price_list;
        json_array_foreach(price_lists, index, price_list) {
            const char *price_list_name = json_string_value(json_object_get(price_list, "Name"));
            client_add_price_list(client, price_list_name);
        }
    }

    return client;
}

bool client_fetch_and_insert(PGconn *db_conn, long last_timestamp) {
    json_t *root = api_fetch_data("clients", last_timestamp);
    if (!root) {
        return false;
    }

    json_t *clients = json_object_get(root, "Clients");
    if (!json_is_array(clients)) {
        fprintf(stderr, "JSON root is not an array\n");
        json_decref(root);
        return false;
    }

    size_t index;
    json_t *client_json;
    json_array_foreach(clients, index, client_json) {
        ClientDataPtr client = client_from_json(client_json);
        
        if (!client_insert(db_conn, client)) {
            fprintf(stderr, "Failed to insert client\n");
            client_free(client);
            continue;
        }

        client_free(client);
    }

    json_decref(root);
    return true;
}