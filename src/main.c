#include "client.h"
#include "form.h"
#include "pricelists.h"
#include "api.h"
#include "core_operations.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>
#include <stdbool.h>

#define MAX_ENTITIES 10

typedef struct {
    const char *name;
    bool (*fetch_and_insert)(PGconn *db_conn, long last_id_or_timestamp);
} EntityInfo;

int main() {
    PGconn *db_conn = db_connect();
    if (!db_conn) {
        fprintf(stderr, "Failed to connect to the database\n");
        return 1;
    }

    api_init();

    EntityInfo entities[] = {
        {"clients", client_fetch_and_insert},
        {"forms", form_fetch_and_insert},
        {"pricelist", pricelist_fetch_and_insert},
        //{"clientnotes", clientnotes_fetch_and_insert},
        //{"visits", visits_fetch_and_insert},
        //{"purchaseorders", purchaseorders_fetch_and_insert},
        //{"retailaudits", retailaudits_fetch_and_insert},
        //{"products", products_fetch_and_insert},
        //{"pricelistitems", pricelistitems_fetch_and_insert},
        //{"forms", forms_fetch_and_insert},
        //{"photos", photos_fetch_and_insert},
        //{"dailyworkingtime", dailyworkingtime_fetch_and_insert},
        //{"visitschedules", visitschedules_fetch_and_insert},
        //{"visitrealizations", visitrealizations_fetch_and_insert},
        //{"users", users_fetch_and_insert},
        //{"reps", reps_fetch_and_insert},
        //{"documenttypes", documenttypes_fetch_and_insert},
        
    };

    int num_entities = sizeof(entities) / sizeof(EntityInfo);

    bool all_done = false;
    while (!all_done) {
        all_done = true;
        for (int i = 0; i < num_entities; i++) {
            long last_processed = get_last_processed(db_conn, entities[i].name);
            bool success = entities[i].fetch_and_insert(db_conn, last_processed);
            
            if (success) {
                long new_last_processed = get_last_processed(db_conn, entities[i].name);
                
                if (new_last_processed > last_processed) {
                    all_done = false; 
                }
            } else {
                fprintf(stderr, "Failed to fetch and insert %s\n", entities[i].name);
            }
        }
    }

    api_cleanup();
    db_disconnect(db_conn);
    return 0;
}