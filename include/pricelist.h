#ifndef PRICELIST_H
#define PRICELIST_H

#include <stdbool.h>
#include <libpq-fe.h>

typedef struct PricelistData* PricelistDataPtr;

PricelistDataPtr pricelist_create(void);

void pricelist_set_name(PricelistDataPtr pricelist, const char* name);
void pricelist_set_is_default(PricelistDataPtr pricelist, bool is_default);
void pricelist_set_active(PricelistDataPtr pricelist, bool active);
void pricelist_set_use_prices(PricelistDataPtr pricelist, bool use_prices);

void pricelist_add_item(PricelistDataPtr pricelist, const char* product_code, const char* product_name,
                        double price, bool active, const char* client_code, const char* client_name,
                        const char* manufacture_id, const char* date_available_from, const char* date_available_to, 
                        int min_quantity, int max_quantity);

bool pricelist_fetch_and_insert(PGconn *db_conn, long last_processed_id);
bool pricelist_insert(PGconn *db_conn, PricelistDataPtr pricelist);
bool pricelist_update(PGconn *db_conn, PricelistDataPtr pricelist);


int pricelist_get_id(PricelistDataPtr pricelist);

void pricelist_free(PricelistDataPtr pricelist);

#endif // PRICELIST_H