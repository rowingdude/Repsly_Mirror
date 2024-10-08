#include "../include/api.h"
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

static CURL *curl;

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr) {
        printf("Not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

static char* base64_encode(const char *input) {
    BIO *bmem, *b64;
    BUF_MEM *bptr;

    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_write(b64, input, strlen(input));
    BIO_flush(b64);
    BIO_get_mem_ptr(b64, &bptr);

    char *buff = (char *)malloc(bptr->length + 1);
    memcpy(buff, bptr->data, bptr->length);
    buff[bptr->length] = 0;

    BIO_free_all(b64);

    return buff;
}

void api_init(void) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
}

void api_cleanup(void) {
    curl_easy_cleanup(curl);
    curl_global_cleanup();
}

json_t* api_fetch_data(const char* endpoint, long last_id) {
    if (!curl) {
        fprintf(stderr, "CURL not initialized\n");
        return NULL;
    }

    char url[256];
    snprintf(url, sizeof(url), "%s%s/%ld", API_BASE_URL, endpoint, last_id);

    struct MemoryStruct chunk;
    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    // Set up Basic Auth
    const char *username = getenv("REPSLY_USERNAME");
    const char *password = getenv("REPSLY_PASSWORD");
    if (!username || !password) {
        fprintf(stderr, "REPSLY_USERNAME or REPSLY_PASSWORD not set\n");
        free(chunk.memory);
        return NULL;
    }

    char auth_string[256];
    snprintf(auth_string, sizeof(auth_string), "%s:%s", username, password);
    char *base64_auth = base64_encode(auth_string);

    char auth_header[300];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Basic %s", base64_auth);
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, auth_header);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);

    free(base64_auth);
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        free(chunk.memory);
        return NULL;
    }

    json_t *root;
    json_error_t error;
    root = json_loads(chunk.memory, 0, &error);

    free(chunk.memory);

    if (!root) {
        fprintf(stderr, "JSON parsing error: %s\n", error.text);
        return NULL;
    }

    return root;
}