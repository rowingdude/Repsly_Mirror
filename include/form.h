#ifndef FORM_H
#define FORM_H

#include <stdbool.h>
#include <libpq-fe.h>

typedef struct FormData* FormDataPtr;
FormDataPtr form_create(void);

void form_set_name(FormDataPtr form, const char* name);
void form_set_visit_id(FormDataPtr form, int visit_id);
void form_set_time_id(FormDataPtr form, int time_id);
void form_set_signature_url(FormDataPtr form, const char* signature_url);

void form_add_item(FormDataPtr form, const char* field, const char* value);

bool form_insert(PGconn *db_conn, FormDataPtr form);

int form_get_id(FormDataPtr form);
void form_free(FormDataPtr form);

#endif // FORM_H