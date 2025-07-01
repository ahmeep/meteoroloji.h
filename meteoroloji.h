/* Copyright (c) 2025 Ahmet Aygör.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
   For using meteoroloji.h add these two lines in ONLY ONE file of your project.
   ```
   #define METEOROLOJI_IMPL
   #include "meteoroloji.h"
   ```

   For using in other files just use `#include "meteoroloji.h"`.

   meteoroloji.h uses curl and cJSON. Linking with curl is trivial, use
   `-lcurl`. For cJSON, either use it from your distribution (debian/rpm/arch
   has it.) or add cJSON.h and cJSON.c files to your project and compile them
   too. You can look at demo/ directory for a minimal setup.
*/

#ifndef METEOROLOJI_H_
#define METEOROLOJI_H_

#include <stddef.h>

typedef enum {
    MTRLJ_OK = 0,
    MTRLJ_REQUEST_FAILED,
    MTRLJ_JSON_PARSING_FAILED
} MTRLJ_CODE;

struct mtrlj_district {
    int id;
    int height;
    double longitude, latitude;
    char *name;

    char *city_name;
    int city_plate_code; /* between 1 and 81 */
};

/* Functions for getting information about city and districts, also you need
   these for getting the actual weather information. */
MTRLJ_CODE mtrlj_get_cities(struct mtrlj_district **cities, size_t *size);
MTRLJ_CODE mtrlj_get_district(struct mtrlj_district *district,
                              const char *city_name, const char *district_name);
MTRLJ_CODE mtrlj_get_districts_in_city(struct mtrlj_district **districts,
                                       size_t *size, const char *city_name);

/* Functions for getting information about weather in a specific district. */
/* TODO: MTRLJ_CODE mtrlj_latest_situation(struct mtrlj_district district, ); */

/* Be responsible and free your memory! */
void mtrlj_free_district(struct mtrlj_district district);
void mtrlj_free_ndistrict(struct mtrlj_district *pdistrict, size_t size);

#endif

#ifdef METEOROLOJI_IMPL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
#include <cJSON.h>

/* CURL HELPERS */

struct mtrlj_curl_response {
    char *response;
    size_t size;
};

static size_t mtrlj_writer_callback(char *ptr, size_t size, size_t nmemb,
                                    void *cptr)
{
    size_t total;
    struct mtrlj_curl_response *mcp;
    char *response;

    total = size * nmemb;
    mcp = (struct mtrlj_curl_response *)cptr;
    response = realloc(mcp->response, mcp->size + total + 1);

    if (!response)
        return 0;

    mcp->response = response;
    memcpy(mcp->response + mcp->size, ptr, total);
    mcp->size += total;
    mcp->response[mcp->size] = 0;

    return total;
}

int mtrlj_curl_get_params(const char *url, const char **params,
                          size_t param_count, struct mtrlj_curl_response *mcp)
{
    CURL *curl;
    CURLcode res;
    CURLU *urlp;
    CURLUcode uc;
    long response_code = 0;
    size_t i;

    urlp = curl_url();
    uc = curl_url_set(urlp, CURLUPART_URL, url, 0);
    for (i = 0; i < param_count; i++) {
        uc = curl_url_set(urlp, CURLUPART_QUERY, params[i],
                          CURLU_APPENDQUERY | CURLU_URLENCODE);
    }

    if (uc) {
        curl_url_cleanup(urlp);
        return 0;
    }

    curl = curl_easy_init();
    if (curl) {
        struct curl_slist *hchunk = NULL;

        hchunk = curl_slist_append(hchunk, "Origin: https://www.mgm.gov.tr");
        curl_easy_setopt(curl, CURLOPT_CURLU, urlp);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hchunk);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, mtrlj_writer_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)mcp);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        } else {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(hchunk);
    }
    curl_url_cleanup(urlp);

    return res == CURLE_OK && response_code == 200;
}

int mtrlj_curl_get(const char *url, struct mtrlj_curl_response *mcp)
{
    return mtrlj_curl_get_params(url, NULL, 0, mcp);
}

/* District parsing helper */

MTRLJ_CODE mtrlj_json_parse_district(const cJSON *district_json,
                                     struct mtrlj_district *district)
{
    size_t name_size;
    size_t city_name_size;

    cJSON *id = cJSON_GetObjectItem(district_json, "merkezId");
    cJSON *height = cJSON_GetObjectItem(district_json, "yukseklik");
    cJSON *longitude = cJSON_GetObjectItem(district_json, "boylam");
    cJSON *latitude = cJSON_GetObjectItem(district_json, "enlem");
    cJSON *name = cJSON_GetObjectItem(district_json, "ilce");
    cJSON *city_name = cJSON_GetObjectItem(district_json, "il");
    cJSON *city_plate_code = cJSON_GetObjectItem(district_json, "ilPlaka");

    if (!cJSON_IsNumber(id) || !cJSON_IsNumber(height)
        || !cJSON_IsNumber(longitude) || !cJSON_IsNumber(latitude)
        || !cJSON_IsNumber(city_plate_code) || !cJSON_IsString(name)
        || !cJSON_IsString(city_name) || (name->valuestring == NULL)
        || (city_name->valuestring == NULL)) {
        return MTRLJ_JSON_PARSING_FAILED;
    }

    district->id = id->valueint;
    district->height = height->valueint;
    district->longitude = longitude->valuedouble;
    district->latitude = latitude->valuedouble;

    name_size = (strlen(name->valuestring) + 1) * sizeof(char);
    district->name = malloc(name_size);
    memcpy(district->name, name->valuestring, name_size);

    city_name_size = (strlen(city_name->valuestring) + 1) * sizeof(char);
    district->city_name = malloc(city_name_size);
    memcpy(district->city_name, city_name->valuestring, city_name_size);

    district->city_plate_code = city_plate_code->valueint;

    return MTRLJ_OK;
}

/* Exposed functions */

MTRLJ_CODE mtrlj_get_cities(struct mtrlj_district **cities, size_t *size)
{
    const char *CITIES_ENDPOINT =
        "https://servis.mgm.gov.tr/web/merkezler/iller";
    MTRLJ_CODE return_code = MTRLJ_OK;
    struct mtrlj_curl_response mcp = {0};
    cJSON *cities_json = NULL;
    const cJSON *city_json = NULL;
    size_t i;

    if (!mtrlj_curl_get(CITIES_ENDPOINT, &mcp)) {
        return_code = MTRLJ_REQUEST_FAILED;
        goto end;
    }

    cities_json = cJSON_Parse(mcp.response);
    if (cities_json == NULL || !cJSON_IsArray(cities_json)) {
        return_code = MTRLJ_JSON_PARSING_FAILED;
        goto end;
    }

    *size = cJSON_GetArraySize(cities_json);
    *cities = calloc(*size, sizeof(struct mtrlj_district));

    i = 0;
    cJSON_ArrayForEach(city_json, cities_json)
    {
        MTRLJ_CODE res = mtrlj_json_parse_district(city_json, *cities + i);

        if (res != MTRLJ_OK) {
            return_code = res;
            goto end;
        }

        i++;
    }

end:
    cJSON_Delete(cities_json);
    free(mcp.response);
    return return_code;
}

MTRLJ_CODE mtrlj_get_district(struct mtrlj_district *district,
                              const char *city_name, const char *district_name)
{
    const char *DISTRICT_ENDPOINT = "https://servis.mgm.gov.tr/web/merkezler";
    char *url_parameters[2];
    MTRLJ_CODE return_code = MTRLJ_OK;
    struct mtrlj_curl_response mcp = {0};
    cJSON *district_json = NULL;

    url_parameters[0] = calloc(128, sizeof(char));
    url_parameters[1] = calloc(128, sizeof(char));
    sprintf(url_parameters[0], "il=%s", city_name);
    sprintf(url_parameters[1], "ilce=%s", district_name);

    if (!mtrlj_curl_get_params(DISTRICT_ENDPOINT, (const char **)url_parameters,
                               2, &mcp)) {
        return_code = MTRLJ_REQUEST_FAILED;
        goto end;
    }

    district_json = cJSON_Parse(mcp.response);
    if (district_json == NULL || !cJSON_IsArray(district_json)
        || cJSON_GetArraySize(district_json) != 1) {
        return_code = MTRLJ_JSON_PARSING_FAILED;
        goto end;
    }

    return_code = mtrlj_json_parse_district(
        cJSON_GetArrayItem(district_json, 0), district);

end:
    free(url_parameters[0]);
    free(url_parameters[1]);
    cJSON_Delete(district_json);
    free(mcp.response);
    return return_code;
}

MTRLJ_CODE mtrlj_get_districts_in_city(struct mtrlj_district **districts,
                                       size_t *size, const char *city_name)
{
    const char *DISTRICTS_ENDPOINT =
        "https://servis.mgm.gov.tr/web/merkezler/ililcesi";
    char *url_parameter;
    MTRLJ_CODE return_code = MTRLJ_OK;
    struct mtrlj_curl_response mcp = {0};
    cJSON *districts_json = NULL;
    const cJSON *district_json = NULL;
    size_t i;

    url_parameter = calloc(128, sizeof(char));
    sprintf(url_parameter, "il=%s", city_name);

    if (!mtrlj_curl_get_params(DISTRICTS_ENDPOINT,
                               (const char **)&url_parameter, 1, &mcp)) {
        return_code = MTRLJ_REQUEST_FAILED;
        goto end;
    }

    districts_json = cJSON_Parse(mcp.response);
    if (districts_json == NULL || !cJSON_IsArray(districts_json)) {
        return_code = MTRLJ_JSON_PARSING_FAILED;
        goto end;
    }

    *size = cJSON_GetArraySize(districts_json);
    *districts = calloc(*size, sizeof(struct mtrlj_district));

    i = 0;
    cJSON_ArrayForEach(district_json, districts_json)
    {
        MTRLJ_CODE res =
            mtrlj_json_parse_district(district_json, *districts + i);

        if (res != MTRLJ_OK) {
            return_code = res;
            goto end;
        }

        i++;
    }

end:
    free(url_parameter);
    cJSON_Delete(districts_json);
    free(mcp.response);
    return return_code;
}

void mtrlj_free_district(struct mtrlj_district district)
{
    free(district.name);
    free(district.city_name);
}

void mtrlj_free_ndistrict(struct mtrlj_district *pdistrict, size_t size)
{
    size_t i;
    for (i = 0; i < size; i++) {
        mtrlj_free_district(pdistrict[i]);
    }
    free(pdistrict);
}
#endif
