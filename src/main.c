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
    PGconn *db_conn = PQconnectdb("your_connection_string");
    if (PQstatus(db_conn) != CONNECTION_OK) {
        fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(db_conn));
        PQfinish(db_conn);
        return 1;
    }

    api_init();

    EntityInfo entities[] = {
        {"clients", client_fetch_and_insert},
        {"forms", form_fetch_and_insert},
        {"pricelists", pricelist_fetch_and_insert},
        // Add more entities here as needed
    };

    int num_entities = sizeof(entities) / sizeof(EntityInfo);

    // Main processing loop
    bool all_done = false;
    while (!all_done) {
        all_done = true;
        for (int i = 0; i < num_entities; i++) {
            long last_processed = get_last_processed(db_conn, entities[i].name);
            bool success = entities[i].fetch_and_insert(db_conn, last_processed);
            
            if (success) {
                long new_last_processed = get_last_processed(db_conn, entities[i].name);
                
                if (new_last_processed > last_processed) {
                    all_done = false;  // We processed some data, so we're not done yet
                }
            } else {
                fprintf(stderr, "Failed to fetch and insert %s\n", entities[i].name);
            }
        }
    }

    api_cleanup();
    PQfinish(db_conn);
    return 0;
}