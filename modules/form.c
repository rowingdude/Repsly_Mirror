#include "../include/form.h"
#include "../include/api.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <jansson.h>

#define MAX_FORM_ITEMS 50

struct FormItem {
    char field[256];
    char value[1024];
};

struct FormData {
    int form_id;
    char name[256];
    int visit_id;
    int time_id;
    char signature_url[512];
    struct FormItem items[MAX_FORM_ITEMS];
    int item_count;
};

FormDataPtr form_create(void) {
    return (FormDataPtr)calloc(1, sizeof(struct FormData));
}

void form_set_name(FormDataPtr form, const char* name) {
    strncpy(form->name, name, sizeof(form->name) - 1);
    form->name[sizeof(form->name) - 1] = '\0';
}

void form_set_visit_id(FormDataPtr form, int visit_id) {
    form->visit_id = visit_id;
}

void form_set_time_id(FormDataPtr form, int time_id) {
    form->time_id = time_id;
}

void form_set_signature_url(FormDataPtr form, const char* signature_url) {
    strncpy(form->signature_url, signature_url, sizeof(form->signature_url) - 1);
    form->signature_url[sizeof(form->signature_url) - 1] = '\0';
}

void form_add_item(FormDataPtr form, const char* field, const char* value) {
    if (form->item_count < MAX_FORM_ITEMS) {
        strncpy(form->items[form->item_count].field, field, sizeof(form->items[form->item_count].field) - 1);
        form->items[form->item_count].field[sizeof(form->items[form->item_count].field) - 1] = '\0';
        strncpy(form->items[form->item_count].value, value, sizeof(form->items[form->item_count].value) - 1);
        form->items[form->item_count].value[sizeof(form->items[form->item_count].value) - 1] = '\0';
        form->item_count++;
    }
}

bool form_insert(PGconn *db_conn, FormDataPtr form) {
    const char *insert_form_query = 
        "INSERT INTO field_ops.forms "
        "(name, visit_id, time_id, signature_url) "
        "VALUES ($1, $2, $3, $4) "
        "RETURNING form_id";

    const char *param_values[4];
    int param_lengths[4];
    int param_formats[4] = {0}; 
    char id_str[2][20];  

    param_values[0] = form->name;
    snprintf(id_str[0], sizeof(id_str[0]), "%d", form->visit_id);
    param_values[1] = id_str[0];
    snprintf(id_str[1], sizeof(id_str[1]), "%d", form->time_id);
    param_values[2] = id_str[1];
    param_values[3] = form->signature_url;

    for (int i = 0; i < 4; i++) {
        param_lengths[i] = strlen(param_values[i]);
    }

    PGresult *result = PQexecParams(db_conn, insert_form_query, 4, NULL, param_values, param_lengths, param_formats, 0);

    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        fprintf(stderr, "INSERT INTO field_ops.forms failed: %s", PQerrorMessage(db_conn));
        PQclear(result);
        return false;
    }

    char *form_id_str = PQgetvalue(result, 0, 0);
    form->form_id = atoi(form_id_str);

    PQclear(result);

    const char *insert_item_query = 
        "INSERT INTO field_ops.form_items "
        "(form_id, field, value) "
        "VALUES ($1, $2, $3)";

    for (int i = 0; i < form->item_count; i++) {
        const char *item_param_values[3];
        int item_param_lengths[3];
        int item_param_formats[3] = {0}; 
        
        snprintf(id_str[0], sizeof(id_str[0]), "%d", form->form_id);
        item_param_values[0] = id_str[0];
        item_param_values[1] = form->items[i].field;
        item_param_values[2] = form->items[i].value;

        for (int j = 0; j < 3; j++) {
            item_param_lengths[j] = strlen(item_param_values[j]);
        }

        PGresult *item_result = PQexecParams(db_conn, insert_item_query, 3, NULL, item_param_values, item_param_lengths, item_param_formats, 0);

        if (PQresultStatus(item_result) != PGRES_COMMAND_OK) {
            fprintf(stderr, "INSERT INTO field_ops.form_items failed: %s", PQerrorMessage(db_conn));
            PQclear(item_result);
            return false;
        }

        PQclear(item_result);
    }

    return true;
}

int form_get_id(FormDataPtr form) {
    return form->form_id;
}

void form_free(FormDataPtr form) {
    free(form);
}

static FormDataPtr form_from_json(json_t *form_json) {
    FormDataPtr form = form_create();
    
    form_set_name(form, json_string_value(json_object_get(form_json, "FormName")));
    form_set_form_id(form, json_integer_value(json_object_get(form_json, "FormID")));
    form_set_client_code(form, json_string_value(json_object_get(form_json, "ClientCode")));
    form_set_client_name(form, json_string_value(json_object_get(form_json, "ClientName")));
    form_set_date_and_time(form, json_integer_value(json_object_get(form_json, "DateAndTime")));
    form_set_rep_code(form, json_string_value(json_object_get(form_json, "RepresentativeCode")));
    form_set_rep_name(form, json_string_value(json_object_get(form_json, "RepresentativeName")));
    form_set_street_address(form, json_string_value(json_object_get(form_json, "StreetAddress")));
    form_set_zip(form, json_string_value(json_object_get(form_json, "ZIP")));
    form_set_zip_ext(form, json_string_value(json_object_get(form_json, "ZIPExt")));
    form_set_city(form, json_string_value(json_object_get(form_json, "City")));
    form_set_state(form, json_string_value(json_object_get(form_json, "State")));
    form_set_country(form, json_string_value(json_object_get(form_json, "Country")));
    form_set_email(form, json_string_value(json_object_get(form_json, "Email")));
    form_set_phone(form, json_string_value(json_object_get(form_json, "Phone")));
    form_set_mobile(form, json_string_value(json_object_get(form_json, "Mobile")));
    form_set_territory(form, json_string_value(json_object_get(form_json, "Territory")));
    form_set_longitude(form, json_real_value(json_object_get(form_json, "Longitude")));
    form_set_latitude(form, json_real_value(json_object_get(form_json, "Latitude")));
    form_set_signature_url(form, json_string_value(json_object_get(form_json, "SignatureURL")));
    form_set_visit_start(form, json_integer_value(json_object_get(form_json, "VisitStart")));
    form_set_visit_end(form, json_integer_value(json_object_get(form_json, "VisitEnd")));
    form_set_visit_id(form, json_integer_value(json_object_get(form_json, "VisitID")));

    // Parse and add form items
    json_t *items = json_object_get(form_json, "Item");
    if (json_is_array(items)) {
        size_t item_index;
        json_t *item;
        json_array_foreach(items, item_index, item) {
            const char *field = json_string_value(json_object_get(item, "Field"));
            const char *value = json_string_value(json_object_get(item, "Value"));
            form_add_item(form, field, value);
        }
    }
    
    return form;
}

bool form_fetch_and_insert(PGconn *db_conn, long last_form_id) {
    json_t *root = api_fetch_data("forms", last_form_id);
    if (!root) {
        return false;
    }

    json_t *forms = json_object_get(root, "Forms");
    if (!json_is_array(forms)) {
        fprintf(stderr, "JSON root is not an array\n");
        json_decref(root);
        return false;
    }

    size_t index;
    json_t *form_json;
    json_array_foreach(forms, index, form_json) {
        FormDataPtr form = form_from_json(form_json);
        
        if (!form_insert(db_conn, form)) {
            fprintf(stderr, "Failed to insert form\n");
            form_free(form);
            continue;
        }

        form_free(form);
    }

    json_decref(root);
    return true;
}