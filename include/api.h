#ifndef API_H
#define API_H

#include <jansson.h>

#define API_BASE_URL "https://api.repsly.com/v3/export/"

void api_init(void);
void api_cleanup(void);

json_t* api_fetch_data(const char* endpoint, long last_id);

#endif 