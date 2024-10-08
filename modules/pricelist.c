#include "../include/pricelist.h"
#include "../include/api.h"
#include "../include/core_operations.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <jansson.h>

#define MAX_PRICELIST_ITEMS 1000

struct PricelistItem {
    int product_id;
    double price;
    bool active;
    char client_code[51];
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

void pricelist_add_item(PricelistDataPtr pricelist, int product_id, double price, bool active, 
                        const char* client_code, const char* manufacture_id, 
                        const char* date_available_from, const char* date_available_to, 
                        int min_quantity, int max_quantity) {
    if (pricelist->item_count < MAX_PRICELIST_ITEMS) {
        struct PricelistItem *item = &pricelist->items[pricelist->item_count];
        item->product_id = product_id;
        item->price = price;
        item->active = active;
        strncpy(item->client_code, client_code, sizeof(item->client_code) - 1);
        item->client_code[sizeof(item->client_code) - 1] = '\0';
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

bool pricelist_insert(PGconn *db_conn, PricelistDataPtr pricelist) {
    const char *insert_pricelist_query = 
        "INSERT INTO inventory.pricelists "
        "(name, is_default, active, use_prices) "
        "VALUES ($1, $2, $3, $4) "
        "RETURNING pricelist_id";

    const char *param_values[4];
    int param_lengths[4];
    int param_formats[4] = {0, 0, 0, 0};  // All text format
    char is_default_str[6], active_str[6], use_prices_str[6];

    param_values[0] = pricelist->name;
    snprintf(is_default_str, sizeof(is_default_str), "%s", pricelist->is_default ? "true" : "false");
    param_values[1] = is_default_str;
    snprintf(active_str, sizeof(active_str), "%s", pricelist->active ? "true" : "false");
    param_values[2] = active_str;
    snprintf(use_prices_str, sizeof(use_prices_str), "%s", pricelist->use_prices ? "true" : "false");
    param_values[3] = use_prices_str;

    for (int i = 0; i < 4; i++) {
        param_lengths[i] = strlen(param_values[i]);
    }

    PGresult *result = PQexecParams(db_conn, insert_pricelist_query, 4, NULL, param_values, param_lengths, param_formats, 0);

    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        fprintf(stderr, "INSERT INTO inventory.pricelists failed: %s", PQerrorMessage(db_conn));
        PQclear(result);
        return false;
    }

    pricelist->pricelist_id = atoi(PQgetvalue(result, 0, 0));
    PQclear(result);

    // Insert pricelist items
    const char *insert_item_query = 
        "INSERT INTO inventory.pricelist_items "
        "(pricelist_id, product_id, price, active, client_id, manufacture_id, date_available_from_id, date_available_to_id, min_quantity, max_quantity) "
        "VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10)";

    for (int i = 0; i < pricelist->item_count; i++) {
        const char *item_param_values[10];
        int item_param_lengths[10];
        int item_param_formats[10] = {0};  // All text format
        char pricelist_id_str[20], product_id_str[20], price_str[20], active_str[6], 
             client_id_str[20], date_from_id_str[20], date_to_id_str[20],
             min_quantity_str[20], max_quantity_str[20];

        // Get or create product
        int product_id = get_or_create_product(db_conn, pricelist->items[i].product_code, pricelist->items[i].product_name);
        if (product_id < 0) {
            fprintf(stderr, "Failed to get or create product\n");
            return false;
        }

        // Get or create client
        int client_id = get_or_create_client(db_conn, pricelist->items[i].client_code, pricelist->items[i].client_name);
        if (client_id < 0) {
            fprintf(stderr, "Failed to get or create client\n");
            return false;
        }

        // Get or create dates
        int date_from_id = get_or_create_date(db_conn, pricelist->items[i].date_available_from);
        int date_to_id = get_or_create_date(db_conn, pricelist->items[i].date_available_to);
        if (date_from_id < 0 || date_to_id < 0) {
            fprintf(stderr, "Failed to get or create dates\n");
            return false;
        }

        snprintf(pricelist_id_str, sizeof(pricelist_id_str), "%d", pricelist->pricelist_id);
        item_param_values[0] = pricelist_id_str;
        snprintf(product_id_str, sizeof(product_id_str), "%d", product_id);
        item_param_values[1] = product_id_str;
        snprintf(price_str, sizeof(price_str), "%f", pricelist->items[i].price);
        item_param_values[2] = price_str;
        snprintf(active_str, sizeof(active_str), "%s", pricelist->items[i].active ? "true" : "false");
        item_param_values[3] = active_str;
        snprintf(client_id_str, sizeof(client_id_str), "%d", client_id);
        item_param_values[4] = client_id_str;
        item_param_values[5] = pricelist->items[i].manufacture_id;
        snprintf(date_from_id_str, sizeof(date_from_id_str), "%d", date_from_id);
        item_param_values[6] = date_from_id_str;
        snprintf(date_to_id_str, sizeof(date_to_id_str), "%d", date_to_id);
        item_param_values[7] = date_to_id_str;
        snprintf(min_quantity_str, sizeof(min_quantity_str), "%d", pricelist->items[i].min_quantity);
        item_param_values[8] = min_quantity_str;
        snprintf(max_quantity_str, sizeof(max_quantity_str), "%d", pricelist->items[i].max_quantity);
        item_param_values[9] = max_quantity_str;

        for (int j = 0; j < 10; j++) {
            item_param_lengths[j] = strlen(item_param_values[j]);
        }

        PGresult *item_result = PQexecParams(db_conn, insert_item_query, 10, NULL, item_param_values, item_param_lengths, item_param_formats, 0);

        if (PQresultStatus(item_result) != PGRES_COMMAND_OK) {
            fprintf(stderr, "INSERT INTO inventory.pricelist_items failed: %s", PQerrorMessage(db_conn));
            PQclear(item_result);
            return false;
        }

        PQclear(item_result);
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
    PricelistDataPtr pricelist = pricelist_create();
    
    pricelist_set_name(pricelist, json_string_value(json_object_get(pricelist_json, "Name")));
    pricelist_set_is_default(pricelist, json_is_true(json_object_get(pricelist_json, "IsDefault")));
    pricelist_set_active(pricelist, json_is_true(json_object_get(pricelist_json, "Active")));
    pricelist_set_use_prices(pricelist, json_is_true(json_object_get(pricelist_json, "UsePrices")));

    json_t *items = json_object_get(pricelist_json, "Items");
    if (json_is_array(items)) {
        size_t index;
        json_t *item;
        json_array_foreach(items, index, item) {
            int product_id = json_integer_value(json_object_get(item, "ProductID"));
            double price = json_real_value(json_object_get(item, "Price"));
            bool active = json_is_true(json_object_get(item, "Active"));
            const char *client_code = json_string_value(json_object_get(item, "ClientID"));
            const char *manufacture_id = json_string_value(json_object_get(item, "ManufactureID"));
            const char *date_available_from = json_string_value(json_object_get(item, "DateAvailableFrom"));
            const char *date_available_to = json_string_value(json_object_get(item, "DateAvailableTo"));
            int min_quantity = json_integer_value(json_object_get(item, "MinQuantity"));
            int max_quantity = json_integer_value(json_object_get(item, "MaxQuantity"));

            pricelist_add_item(pricelist, product_id, price, active, client_code, manufacture_id,
                               date_available_from, date_available_to, min_quantity, max_quantity);
        }
    }
    
    return pricelist;
}

bool pricelist_fetch_and_insert(PGconn *db_conn) {
    json_t *root = api_fetch_data("pricelists", 0);  // Assuming we always fetch all pricelists
    if (!root) {
        return false;
    }

    size_t index;
    json_t *pricelist_json;
    json_array_foreach(root, index, pricelist_json) {
        PricelistDataPtr pricelist = pricelist_from_json(pricelist_json);
        
        if (!pricelist_insert(db_conn, pricelist)) {
            fprintf(stderr, "Failed to insert pricelist\n");
            pricelist_free(pricelist);
            continue;
        }

        pricelist_free(pricelist);
    }

    json_decref(root);
    return true;
}