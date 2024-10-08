#include "../include/client.h"
#include "../include/form.h"
#include <stdio.h>
#include <libpq-fe.h>

int main() {
    PGconn *db_conn = PQconnectdb("your_connection_string");
    if (PQstatus(db_conn) != CONNECTION_OK) {
        fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(db_conn));
        PQfinish(db_conn);
        return 1;
    }

    // Client example
    ClientDataPtr client = client_create();
    client_set_code(client, "ABC123");
    client_set_active(client, true);
    client_set_address_id(client, 1);
    client_set_contact_id(client, 2);
    client_set_territory_id(client, 3);
    client_set_rep_id(client, 4);
    client_set_account_code(client, "ACCT001");
    client_set_status(client, "Active");
    client_set_contact_name_id(client, 5);
    client_set_contact_title_id(client, 6);
    client_set_name_id(client, 7);

    if (client_insert(db_conn, client)) {
        printf("Client inserted successfully with ID: %d\n", client_get_id(client));
    } else {
        printf("Failed to insert client\n");
    }

    client_free(client);

    // Form example
    FormDataPtr form = form_create();
    form_set_name(form, "Sample Form");
    form_set_visit_id(form, 1);  // Assuming visit_id 1 exists
    form_set_time_id(form, 1);   // Assuming time_id 1 exists
    form_set_signature_url(form, "http://example.com/signature.jpg");
    
    form_add_item(form, "Question 1", "Answer 1");
    form_add_item(form, "Question 2", "Answer 2");

    if (form_insert(db_conn, form)) {
        printf("Form inserted successfully with ID: %d\n", form_get_id(form));
    } else {
        printf("Failed to insert form\n");
    }

    form_free(form);

    PQfinish(db_conn);
    return 0;
}