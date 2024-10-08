#include "../include/pricelist.h"
#include "../include/api.h"
#include "../include/core_operations.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <jansson.h>

#define MAX_PRICELIST_ITEMS 1000

struct PricelistItem {
    char product_code[51];
    char product_name[256];
    double price;
    bool active;
    char client_code[51];
    char client_name[256];
    char manufacture_id[51];
    char date_available_from[20];
    char date_available_to[20];
    int min_quantity;
    int max_quantity;
};

struct PricelistData {
    int pricelist_id;
    char name[256];
    bool is_default;
    bool active;
    bool use_prices;
    struct PricelistItem items[MAX_PRICELIST_ITEMS];
    int item_count;
};

PricelistDataPtr pricelist_create(void) {
    return (PricelistDataPtr)calloc(1, sizeof(struct PricelistData));
}

void pricelist_set_name(PricelistDataPtr pricelist, const char* name) {
    strncpy(pricelist->name, name, sizeof(pricelist->name) - 1);
    pricelist->name[sizeof(pricelist->name) - 1] = '\0';
}

void pricelist_set_is_default(PricelistDataPtr pricelist, bool is_default) {
    pricelist->is_default = is_default;
}

void pricelist_set_active(PricelistDataPtr pricelist, bool active) {
    pricelist->active = active;
}

void pricelist_set_use_prices(PricelistDataPtr pricelist, bool use_prices) {
    pricelist->use_prices = use_prices;
}

void pricelist_add_item(PricelistDataPtr pricelist, const char* product_code, const char* product_name,
                        double price, bool active, const char* client_code, const char* client_name,
                        const char* manufacture_id, const char* date_available_from, const char* date_available_to, 
                        int min_quantity, int max_quantity) {
    if (pricelist->item_count < MAX_PRICELIST_ITEMS) {
        struct PricelistItem *item = &pricelist->items[pricelist->item_count];
        strncpy(item->product_code, product_code, sizeof(item->product_code) - 1);
        item->product_code[sizeof(item->product_code) - 1] = '\0';
        strncpy(item->product_name, product_name, sizeof(item->product_name) - 1);
        item->product_name[sizeof(item->product_name) - 1] = '\0';
        item->price = price;
        item->active = active;
        strncpy(item->client_code, client_code, sizeof(item->client_code) - 1);
        item->client_code[sizeof(item->client_code) - 1] = '\0';
        strncpy(item->client_name, client_name, sizeof(item->client_name) - 1);
        item->client_name[sizeof(item->client_name) - 1] = '\0';
        strncpy(item->manufacture_id, manufacture_id, sizeof(item->manufacture_id) - 1);
        item->manufacture_id[sizeof(item->manufacture_id) - 1] = '\0';
        strncpy(item->date_available_from, date_available_from, sizeof(item->date_available_from) - 1);
        item->date_available_from[sizeof(item->date_available_from) - 1] = '\0';
        strncpy(item->date_available_to, date_available_to, sizeof(item->date_available_to) - 1);
        item->date_available_to[sizeof(item->date_available_to) - 1] = '\0';
        item->min_quantity = min_quantity;
        item->max_quantity = max_quantity;
        pricelist->item_count++;
    }
}
static bool execute_pricelist_query(PGconn *db_conn, const char *query, const char **param_values, int param_count) {
    int param_lengths[param_count];
    int param_formats[param_count];
    for (int i = 0; i < param_count; i++) {
        param_lengths[i] = strlen(param_values[i]);
        param_formats[i] = 0;  // All text format
    }

    PGresult *result = PQexecParams(db_conn, query, param_count, NULL, param_values, param_lengths, param_formats, 0);

    bool success = (PQresultStatus(result) == PGRES_COMMAND_OK);
    if (!success) {
        fprintf(stderr, "Query execution failed: %s", PQerrorMessage(db_conn));
    }

    PQclear(result);
    return success;
}

static bool insert_pricelist_item(PGconn *db_conn, int pricelist_id, struct PricelistItem *item) {
    const char *insert_item_query = 
        "INSERT INTO inventory.pricelist_items "
        "(pricelist_id, product_id, price, active, client_id, manufacture_id, date_available_from_id, date_available_to_id, min_quantity, max_quantity) "
        "VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10)";

    int product_id = get_or_create_product(db_conn, item->product_code, item->product_name);
    int client_id = get_or_create_client(db_conn, item->client_code, item->client_name);
    int date_from_id = get_or_create_date(db_conn, item->date_available_from);
    int date_to_id = get_or_create_date(db_conn, item->date_available_to);

    if (product_id < 0 || client_id < 0 || date_from_id < 0 || date_to_id < 0) {
        return false;
    }

    char pricelist_id_str[20], product_id_str[20], price_str[20], active_str[6], 
         client_id_str[20], date_from_id_str[20], date_to_id_str[20],
         min_quantity_str[20], max_quantity_str[20];

    snprintf(pricelist_id_str, sizeof(pricelist_id_str), "%d", pricelist_id);
    snprintf(product_id_str, sizeof(product_id_str), "%d", product_id);
    snprintf(price_str, sizeof(price_str), "%f", item->price);
    snprintf(active_str, sizeof(active_str), "%s", item->active ? "true" : "false");
    snprintf(client_id_str, sizeof(client_id_str), "%d", client_id);
    snprintf(date_from_id_str, sizeof(date_from_id_str), "%d", date_from_id);
    snprintf(date_to_id_str, sizeof(date_to_id_str), "%d", date_to_id);
    snprintf(min_quantity_str, sizeof(min_quantity_str), "%d", item->min_quantity);
    snprintf(max_quantity_str, sizeof(max_quantity_str), "%d", item->max_quantity);

    const char *param_values[] = {
        pricelist_id_str, product_id_str, price_str, active_str, client_id_str,
        item->manufacture_id, date_from_id_str, date_to_id_str,
        min_quantity_str, max_quantity_str
    };

    return execute_pricelist_query(db_conn, insert_item_query, param_values, 10);
}

static bool update_pricelist_item(PGconn *db_conn, int pricelist_id, struct PricelistItem *item) {
    const char *update_item_query = 
        "UPDATE inventory.pricelist_items SET "
        "price = $3, active = $4, manufacture_id = $6, "
        "date_available_from_id = $7, date_available_to_id = $8, "
        "min_quantity = $9, max_quantity = $10 "
        "WHERE pricelist_id = $1 AND product_id = $2 AND client_id = $5";

    int product_id = get_or_create_product(db_conn, item->product_code, item->product_name);
    int client_id = get_or_create_client(db_conn, item->client_code, item->client_name);
    int date_from_id = get_or_create_date(db_conn, item->date_available_from);
    int date_to_id = get_or_create_date(db_conn, item->date_available_to);

    if (product_id < 0 || client_id < 0 || date_from_id < 0 || date_to_id < 0) {
        return false;
    }

    char pricelist_id_str[20], product_id_str[20], price_str[20], active_str[6], 
         client_id_str[20], date_from_id_str[20], date_to_id_str[20],
         min_quantity_str[20], max_quantity_str[20];

    snprintf(pricelist_id_str, sizeof(pricelist_id_str), "%d", pricelist_id);
    snprintf(product_id_str, sizeof(product_id_str), "%d", product_id);
    snprintf(price_str, sizeof(price_str), "%f", item->price);
    snprintf(active_str, sizeof(active_str), "%s", item->active ? "true" : "false");
    snprintf(client_id_str, sizeof(client_id_str), "%d", client_id);
    snprintf(date_from_id_str, sizeof(date_from_id_str), "%d", date_from_id);
    snprintf(date_to_id_str, sizeof(date_to_id_str), "%d", date_to_id);
    snprintf(min_quantity_str, sizeof(min_quantity_str), "%d", item->min_quantity);
    snprintf(max_quantity_str, sizeof(max_quantity_str), "%d", item->max_quantity);

    const char *param_values[] = {
        pricelist_id_str, product_id_str, price_str, active_str, client_id_str,
        item->manufacture_id, date_from_id_str, date_to_id_str,
        min_quantity_str, max_quantity_str
    };

    return execute_pricelist_query(db_conn, update_item_query, param_values, 10);
}

bool pricelist_insert(PGconn *db_conn, PricelistDataPtr pricelist) {
    if (!db_conn || !pricelist) {
        fprintf(stderr, "Error: Invalid database connection or pricelist data\n");
        return false;
    }

    const char *insert_pricelist_query = 
        "INSERT INTO inventory.pricelists "
        "(name, is_default, active, use_prices) "
        "VALUES ($1, $2, $3, $4) "
        "RETURNING pricelist_id";

    char is_default_str[6], active_str[6], use_prices_str[6];
    snprintf(is_default_str, sizeof(is_default_str), "%s", pricelist->is_default ? "true" : "false");
    snprintf(active_str, sizeof(active_str), "%s", pricelist->active ? "true" : "false");
    snprintf(use_prices_str, sizeof(use_prices_str), "%s", pricelist->use_prices ? "true" : "false");

    const char *param_values[] = {pricelist->name, is_default_str, active_str, use_prices_str};

    PGresult *result = PQexecParams(db_conn, insert_pricelist_query, 4, NULL, param_values, NULL, NULL, 0);

    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        fprintf(stderr, "INSERT INTO inventory.pricelists failed: %s", PQerrorMessage(db_conn));
        PQclear(result);
        return false;
    }

    pricelist->pricelist_id = atoi(PQgetvalue(result, 0, 0));
    PQclear(result);

    for (int i = 0; i < pricelist->item_count; i++) {
        if (!insert_pricelist_item(db_conn, pricelist->pricelist_id, &pricelist->items[i])) {
            return false;
        }
    }

    return true;
}

bool pricelist_update(PGconn *db_conn, PricelistDataPtr pricelist) {
    if (!db_conn || !pricelist) {
        fprintf(stderr, "Error: Invalid database connection or pricelist data\n");
        return false;
    }

    const char *update_pricelist_query = 
        "UPDATE inventory.pricelists SET "
        "is_default = $2, active = $3, use_prices = $4 "
        "WHERE name = $1 "
        "RETURNING pricelist_id";

    char is_default_str[6], active_str[6], use_prices_str[6];
    snprintf(is_default_str, sizeof(is_default_str), "%s", pricelist->is_default ? "true" : "false");
    snprintf(active_str, sizeof(active_str), "%s", pricelist->active ? "true" : "false");
    snprintf(use_prices_str, sizeof(use_prices_str), "%s", pricelist->use_prices ? "true" : "false");

    const char *param_values[] = {pricelist->name, is_default_str, active_str, use_prices_str};

    PGresult *result = PQexecParams(db_conn, update_pricelist_query, 4, NULL, param_values, NULL, NULL, 0);

    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        fprintf(stderr, "UPDATE inventory.pricelists failed: %s", PQerrorMessage(db_conn));
        PQclear(result);
        return false;
    }

    pricelist->pricelist_id = atoi(PQgetvalue(result, 0, 0));
    PQclear(result);

    for (int i = 0; i < pricelist->item_count; i++) {
        if (!update_pricelist_item(db_conn, pricelist->pricelist_id, &pricelist->items[i])) {
            return false;
        }
    }

    return true;
}


int pricelist_get_id(PricelistDataPtr pricelist) {
    return pricelist->pricelist_id;
}

void pricelist_free(PricelistDataPtr pricelist) {
    free(pricelist);
}

static PricelistDataPtr pricelist_from_json(json_t *pricelist_json) {
    if (!json_is_object(pricelist_json)) {
        fprintf(stderr, "Error: pricelist_json is not a JSON object\n");
        return NULL;
    }

    PricelistDataPtr pricelist = pricelist_create();
    if (!pricelist) {
        fprintf(stderr, "Error: Failed to create pricelist\n");
        return NULL;
    }    PricelistDataPtr pricelist = pricelist_create();
    
    pricelist_set_name(pricelist, json_string_value(json_object_get(pricelist_json, "Name")));
    pricelist_set_is_default(pricelist, json_is_true(json_object_get(pricelist_json, "IsDefault")));
    pricelist_set_active(pricelist, json_is_true(json_object_get(pricelist_json, "Active")));
    pricelist_set_use_prices(pricelist, json_is_true(json_object_get(pricelist_json, "UsePrices")));

    json_t *items = json_object_get(pricelist_json, "Items");
    if (json_is_array(items)) {
        size_t index;
        json_t *item;
        json_array_foreach(items, index, item) {
            const char *product_code = json_string_value(json_object_get(item, "ProductCode"));
            const char *product_name = json_string_value(json_object_get(item, "ProductName"));
            double price = json_real_value(json_object_get(item, "Price"));
            bool active = json_is_true(json_object_get(item, "Active"));
            const char *client_code = json_string_value(json_object_get(item, "ClientCode"));
            const char *client_name = json_string_value(json_object_get(item, "ClientName"));
            const char *manufacture_id = json_string_value(json_object_get(item, "ManufactureID"));
            const char *date_available_from = json_string_value(json_object_get(item, "DateAvailableFrom"));
            const char *date_available_to = json_string_value(json_object_get(item, "DateAvailableTo"));
            int min_quantity = json_integer_value(json_object_get(item, "MinQuantity"));
            int max_quantity = json_integer_value(json_object_get(item, "MaxQuantity"));

            pricelist_add_item(pricelist, product_code, product_name, price, active, 
                               client_code, client_name, manufacture_id,
                               date_available_from, date_available_to, min_quantity, max_quantity);
        }
    }
    
    return pricelist;
}

static bool pricelist_exists(PGconn *db_conn, const char *name) {
    const char *query = "SELECT 1 FROM inventory.pricelists WHERE name = $1";
    const char *param_values[] = { name };
    int param_lengths[] = { strlen(name) };
    int param_formats[] = { 0 };  

    PGresult *result = PQexecParams(db_conn, query, 1, NULL, param_values, param_lengths, param_formats, 0);

    bool exists = (PQresultStatus(result) == PGRES_TUPLES_OK && PQntuples(result) > 0);
    PQclear(result);

    return exists;
}

bool pricelist_fetch_and_insert(PGconn *db_conn, long last_processed_id) {
    json_t *root = api_fetch_data("pricelists", last_processed_id);
    if (!root) {
        return false;
    }

    size_t index;
    json_t *pricelist_json;
    long max_processed_id = last_processed_id;

    json_array_foreach(root, index, pricelist_json) {
        PricelistDataPtr pricelist = pricelist_from_json(pricelist_json);
        
        if (pricelist_exists(db_conn, pricelist->name)) {
            if (!pricelist_update(db_conn, pricelist)) {
                fprintf(stderr, "Failed to update pricelist\n");
                pricelist_free(pricelist);
                continue;
            }
        } else {
            if (!pricelist_insert(db_conn, pricelist)) {
                fprintf(stderr, "Failed to insert pricelist\n");
                pricelist_free(pricelist);
                continue;
            }
        }

        long current_id = pricelist_get_id(pricelist);
        if (current_id > max_processed_id) {
            max_processed_id = current_id;
        }

        pricelist_free(pricelist);
    }

    json_decref(root);

    // Update the last processed ID
    if (!update_last_processed(db_conn, "pricelists", max_processed_id)) {
        fprintf(stderr, "Failed to update last processed ID for pricelists\n");
    }

    return true;
}