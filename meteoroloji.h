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

/* I am not so sure about translations but should be OK. */
typedef enum {
    MTRLJ_WEATHER_INVALID = 0,
    MTRLJ_WEATHER_CLEAR = 0x00410000,
    MTRLJ_WEATHER_SOME_CLOUDS = 0x00414200,
    MTRLJ_WEATHER_PARTLY_CLOUDY = 0x00504200,
    MTRLJ_WEATHER_MOSTLY_CLOUDY = 0x00434200,
    MTRLJ_WEATHER_LIGHT_RAINY = 0x00485900,
    MTRLJ_WEATHER_RAINY = 0x00590000,
    MTRLJ_WEATHER_HEAVY_RAINY = 0x004b5900,
    MTRLJ_WEATHER_SLEETY = 0x004b4b59,
    MTRLJ_WEATHER_LIGHT_SNOWY = 0x00484b59,
    MTRLJ_WEATHER_SNOWY = 0x004b0000,
    MTRLJ_WEATHER_HEAVY_SNOWY = 0x004b594b, /* also 0x00594b59 */
    MTRLJ_WEATHER_LIGHT_DOWNPOURS = 0x00485359,
    MTRLJ_WEATHER_DOWNPOURS = 0x00535900,
    MTRLJ_WEATHER_HEAVY_DOWNPOURS = 0x004b5359,
    MTRLJ_WEATHER_ISOLATED_DOWNPOURS = 0x004d5359,
    MTRLJ_WEATHER_HAIL = 0x00445900,
    MTRLJ_WEATHER_THUNDERY_SHOWERS = 0x00475359,
    MTRLJ_WEATHER_HEAVY_THUNDERY_SHOWERS = 0x004b4759,
    MTRLJ_WEATHER_FOGGY = 0x00534943,
    MTRLJ_WEATHER_HAZY = 0x00505553,
    MTRLJ_WEATHER_SMOGGY = 0x00444e4d,
    MTRLJ_WEATHER_DUST_STORM = 0x004b4600,
    MTRLJ_WEATHER_WINDY = 0x00520000,
    MTRLJ_WEATHER_STRONG_SOUTHERLY_WIND = 0x00474b52,
    MTRLJ_WEATHER_STRONG_NORTHERLY_WIND = 0x004b4b52,
    MTRLJ_WEATHER_WARM = 0x0053434b,
    MTRLJ_WEATHER_COLD = 0x0053474b
} MTRLJ_WEATHER_CONDITION;

struct mtrlj_district {
    int id;
    int height;
    int daily_forecast_station;
    int hourly_forecast_station;
    double longitude, latitude;
    char *name;

    char *city_name;
    int city_plate_code; /* between 1 and 81 */
};

/* If value of a variable is -99 or -9999 it means it is not available. */
struct mtrlj_situation {
    /* Condition of weather, probably you are looking for this. */
    MTRLJ_WEATHER_CONDITION condition;

    /* Pressure information, unit is hectopascal. */
    double actual_pressure, reduced_pressure_at_sea;

    /* Temperature of sea, unavailable on shoreless cities. unit is celcius. */
    double sea_temperature;

    /* Height of snow, unit is meters. */
    double snow_height;

    /* Relative humidity, percentage. */
    double humidity_percent;

    /* Speed of wind, unit is kmh. */
    double wind_speed;

    /* Direction of wind, unit is degrees (0-360).*/
    double wind_direction;

    /* How foul the sky is, percentage. */
    double cloudiness_percent;

    /* Temperature, unit is celcius. */
    double temperature;

    /* Rainfall, unit is millimeters. */
    double rainfall, rainfall_10_mins, rainfall_1_hour, rainfall_6_hours,
        rainfall_12_hours, rainfall_24_hours;

    /* Time of this data in ISO 8601 format. Parsing this is up to user. If you
       want an example look at `demo/demo.c`. */
    char *time;
};

/* Daily forecast information */
struct mtrlj_daily_forecast {
    MTRLJ_WEATHER_CONDITION condition;
    double temperature_min;
    double temperature_max;
    double humidity_min;
    double humidity_max;
    double wind_speed;
    double wind_direction;
    double past_peak_temperature_min;
    double past_peak_temperature_max;
    double past_average_temperature_min;
    double past_average_temperature_max;
    char *time;
};

/* Functions for getting information about city and districts, also you need
   these for getting the actual weather information. */
MTRLJ_CODE mtrlj_get_cities(struct mtrlj_district **cities, size_t *size);
MTRLJ_CODE mtrlj_get_district(struct mtrlj_district *district,
                              const char *city_name, const char *district_name);
MTRLJ_CODE mtrlj_get_districts_in_city(struct mtrlj_district **districts,
                                       size_t *size, const char *city_name);

/* Functions for getting information about weather in a specific district. */
MTRLJ_CODE mtrlj_latest_situation(struct mtrlj_district district,
                                  struct mtrlj_situation *situation);
MTRLJ_CODE mtrlj_five_days_forecast(struct mtrlj_district district,
                                    struct mtrlj_daily_forecast **forecasts);

/* Be responsible and free your memory! */
void mtrlj_free_district(struct mtrlj_district district);
void mtrlj_free_ndistrict(struct mtrlj_district *pdistrict, size_t size);
void mtrlj_free_situation(struct mtrlj_situation situation);
void mtrlj_free_forecasts(struct mtrlj_daily_forecast *pforecast);

#endif

#ifdef METEOROLOJI_IMPL

#include <stdint.h>
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
    cJSON *daily_forecast_station =
        cJSON_GetObjectItem(district_json, "gunlukTahminIstNo");
    cJSON *hourly_forecast_station =
        cJSON_GetObjectItem(district_json, "saatlikTahminIstNo");
    cJSON *longitude = cJSON_GetObjectItem(district_json, "boylam");
    cJSON *latitude = cJSON_GetObjectItem(district_json, "enlem");
    cJSON *name = cJSON_GetObjectItem(district_json, "ilce");
    cJSON *city_name = cJSON_GetObjectItem(district_json, "il");
    cJSON *city_plate_code = cJSON_GetObjectItem(district_json, "ilPlaka");

    if (!cJSON_IsNumber(id) || !cJSON_IsNumber(height)
        || !cJSON_IsNumber(daily_forecast_station)
        || !cJSON_IsNumber(hourly_forecast_station)
        || !cJSON_IsNumber(longitude) || !cJSON_IsNumber(latitude)
        || !cJSON_IsNumber(city_plate_code) || !cJSON_IsString(name)
        || !cJSON_IsString(city_name) || (name->valuestring == NULL)
        || (city_name->valuestring == NULL)) {
        return MTRLJ_JSON_PARSING_FAILED;
    }

    district->id = id->valueint;
    district->height = height->valueint;
    district->daily_forecast_station = daily_forecast_station->valueint;
    district->hourly_forecast_station = hourly_forecast_station->valueint;
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

/* Condition from 1-3 letter codes */
MTRLJ_WEATHER_CONDITION mtrlj_condition_from_code(const char *code)
{
    /* i am just using bitwise operations to encode characters to shorten this
       function */
    size_t len;
    uint32_t condition = 0;

    len = strlen(code);
    if (len < 1 || len > 3)
        return MTRLJ_WEATHER_INVALID;

    condition |= code[0] << 16; /* 0x00XX0000 */

    if (len > 1)
        condition |= code[1] << 8; /* 0x00xxXX00 */

    if (len > 2)
        condition |= code[2]; /* 0x00xxxxXX */

    /* This condition has 2 codes(`KYK` and `YKY`) */
    if (condition == 0x00594b59)
        return MTRLJ_WEATHER_HEAVY_SNOWY;

    return (MTRLJ_WEATHER_CONDITION)condition;
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

MTRLJ_CODE mtrlj_latest_situation(struct mtrlj_district district,
                                  struct mtrlj_situation *situation)
{
    const char *LATEST_SITUATION_ENDPOINT =
        "https://servis.mgm.gov.tr/web/sondurumlar";
    char *url_parameter;
    MTRLJ_CODE return_code = MTRLJ_OK;
    struct mtrlj_curl_response mcp = {0};
    cJSON *situation_json = NULL;

    url_parameter = calloc(128, sizeof(char));
    sprintf(url_parameter, "merkezid=%d", district.id);

    if (!mtrlj_curl_get_params(LATEST_SITUATION_ENDPOINT,
                               (const char **)&url_parameter, 1, &mcp)) {
        return_code = MTRLJ_REQUEST_FAILED;
        goto end;
    }

    situation_json = cJSON_Parse(mcp.response);
    if (situation_json == NULL || !cJSON_IsArray(situation_json)
        || cJSON_GetArraySize(situation_json) != 1) {
        return_code = MTRLJ_JSON_PARSING_FAILED;
        goto end;
    }

    {
        size_t time_size;
        cJSON *json = cJSON_GetArrayItem(situation_json, 0);
        cJSON *actual_pressure = cJSON_GetObjectItem(json, "aktuelBasinc");
        cJSON *reduced_pressure_at_sea =
            cJSON_GetObjectItem(json, "denizeIndirgenmisBasinc");
        cJSON *sea_temperature = cJSON_GetObjectItem(json, "denizSicaklik");
        cJSON *condition_code = cJSON_GetObjectItem(json, "hadiseKodu");
        cJSON *cloudiness_percent = cJSON_GetObjectItem(json, "kapalilik");
        cJSON *snow_height = cJSON_GetObjectItem(json, "karYukseklik");
        cJSON *humidity_percent = cJSON_GetObjectItem(json, "nem");
        cJSON *wind_speed = cJSON_GetObjectItem(json, "ruzgarHiz");
        cJSON *wind_direction = cJSON_GetObjectItem(json, "ruzgarYon");
        cJSON *temperature = cJSON_GetObjectItem(json, "sicaklik");
        cJSON *rainfall = cJSON_GetObjectItem(json, "yagis00Now");
        cJSON *rainfall_10_mins = cJSON_GetObjectItem(json, "yagis10Dk");
        cJSON *rainfall_1_hour = cJSON_GetObjectItem(json, "yagis1Saat");
        cJSON *rainfall_6_hours = cJSON_GetObjectItem(json, "yagis6Saat");
        cJSON *rainfall_12_hours = cJSON_GetObjectItem(json, "yagis12Saat");
        cJSON *rainfall_24_hours = cJSON_GetObjectItem(json, "yagis24Saat");
        cJSON *time = cJSON_GetObjectItem(json, "veriZamani");

        /* this is a huge if statement lol */
        if (!cJSON_IsNumber(actual_pressure)
            || !cJSON_IsNumber(reduced_pressure_at_sea)
            || !cJSON_IsNumber(sea_temperature)
            || !cJSON_IsNumber(cloudiness_percent)
            || !cJSON_IsNumber(snow_height) || !cJSON_IsNumber(humidity_percent)
            || !cJSON_IsNumber(wind_speed) || !cJSON_IsNumber(wind_direction)
            || !cJSON_IsNumber(temperature) || !cJSON_IsNumber(rainfall)
            || !cJSON_IsNumber(rainfall_10_mins)
            || !cJSON_IsNumber(rainfall_1_hour)
            || !cJSON_IsNumber(rainfall_6_hours)
            || !cJSON_IsNumber(rainfall_12_hours)
            || !cJSON_IsNumber(rainfall_24_hours) || !cJSON_IsString(time)
            || (time->valuestring == NULL) || !cJSON_IsString(condition_code)
            || (condition_code->valuestring == NULL)) {
            return_code = MTRLJ_JSON_PARSING_FAILED;
            goto end;
        }

        situation->actual_pressure = actual_pressure->valuedouble;
        situation->reduced_pressure_at_sea =
            reduced_pressure_at_sea->valuedouble;
        situation->sea_temperature = sea_temperature->valuedouble;
        situation->condition =
            mtrlj_condition_from_code(condition_code->valuestring);
        situation->cloudiness_percent = cloudiness_percent->valuedouble;
        situation->snow_height = snow_height->valuedouble;
        situation->humidity_percent = humidity_percent->valuedouble;
        situation->wind_speed = wind_speed->valuedouble;
        situation->wind_direction = wind_direction->valuedouble;
        situation->temperature = temperature->valuedouble;
        situation->rainfall = rainfall->valuedouble;
        situation->rainfall_10_mins = rainfall_10_mins->valuedouble;
        situation->rainfall_1_hour = rainfall_1_hour->valuedouble;
        situation->rainfall_6_hours = rainfall_6_hours->valuedouble;
        situation->rainfall_12_hours = rainfall_12_hours->valuedouble;
        situation->rainfall_24_hours = rainfall_24_hours->valuedouble;

        time_size = (strlen(time->valuestring) + 1) * sizeof(char);
        situation->time = malloc(time_size);
        memcpy(situation->time, time->valuestring, time_size);
    }

end:
    free(url_parameter);
    cJSON_Delete(situation_json);
    free(mcp.response);
    return return_code;
}

MTRLJ_CODE mtrlj_five_days_forecast(struct mtrlj_district district,
                                    struct mtrlj_daily_forecast **forecasts)
{
    const char *DAILY_FORECAST_ENDPOINT =
        "https://servis.mgm.gov.tr/web/tahminler/gunluk";
    char *url_parameter;
    MTRLJ_CODE return_code = MTRLJ_OK;
    struct mtrlj_curl_response mcp = {0};
    cJSON *daily_json = NULL;

    url_parameter = calloc(128, sizeof(char));
    sprintf(url_parameter, "istno=%d", district.daily_forecast_station);

    if (!mtrlj_curl_get_params(DAILY_FORECAST_ENDPOINT,
                               (const char **)&url_parameter, 1, &mcp)) {
        return_code = MTRLJ_REQUEST_FAILED;
        goto end;
    }

    daily_json = cJSON_Parse(mcp.response);
    if (daily_json == NULL || !cJSON_IsArray(daily_json)
        || cJSON_GetArraySize(daily_json) != 1) {
        return_code = MTRLJ_JSON_PARSING_FAILED;
        goto end;
    }

    *forecasts = calloc(5, sizeof(struct mtrlj_daily_forecast));

    {
        char condition_key[] = "hadiseGun ";
        char min_temp_key[] = "enDusukGun ";
        char max_temp_key[] = "enYuksekGun ";
        char min_humidity_key[] = "enDusukNemGun ";
        char max_humidity_key[] = "enYuksekNemGun ";
        char wind_speed_key[] = "ruzgarHizGun ";
        char wind_direction_key[] = "ruzgarYonGun ";
        char time_key[] = "tarihGun ";
        size_t i;
        cJSON *condition_code;
        cJSON *temperature_min;
        cJSON *temperature_max;
        cJSON *humidity_min;
        cJSON *humidity_max;
        cJSON *wind_speed;
        cJSON *wind_direction;
        cJSON *time;
        size_t time_size;
        cJSON *json = cJSON_GetArrayItem(daily_json, 0);

        for (i = 0; i < 5; i++) {
            char num = '1' + i;
            condition_key[9] = num;
            min_temp_key[10] = num;
            max_temp_key[11] = num;
            min_humidity_key[13] = num;
            max_humidity_key[14] = num;
            wind_speed_key[12] = num;
            wind_direction_key[12] = num;
            time_key[8] = num;

            condition_code = cJSON_GetObjectItem(json, condition_key);
            temperature_min = cJSON_GetObjectItem(json, min_temp_key);
            temperature_max = cJSON_GetObjectItem(json, max_temp_key);
            humidity_min = cJSON_GetObjectItem(json, min_humidity_key);
            humidity_max = cJSON_GetObjectItem(json, max_humidity_key);
            wind_speed = cJSON_GetObjectItem(json, wind_speed_key);
            wind_direction = cJSON_GetObjectItem(json, wind_direction_key);
            time = cJSON_GetObjectItem(json, time_key);

            if (!cJSON_IsNumber(temperature_min)
                || !cJSON_IsNumber(temperature_max)
                || !cJSON_IsNumber(humidity_min)
                || !cJSON_IsNumber(humidity_max) || !cJSON_IsNumber(wind_speed)
                || !cJSON_IsNumber(wind_direction)
                || !cJSON_IsString(condition_code)
                || (condition_code->valuestring == NULL)
                || !cJSON_IsString(time) || (time->valuestring == NULL)) {
                return_code = MTRLJ_JSON_PARSING_FAILED;
                goto end;
            }

            (*forecasts)[i].condition =
                mtrlj_condition_from_code(condition_code->valuestring);
            (*forecasts)[i].temperature_min = temperature_min->valuedouble;
            (*forecasts)[i].temperature_max = temperature_max->valuedouble;
            (*forecasts)[i].humidity_min = humidity_min->valuedouble;
            (*forecasts)[i].humidity_max = humidity_max->valuedouble;
            (*forecasts)[i].wind_speed = wind_speed->valuedouble;
            (*forecasts)[i].wind_direction = wind_direction->valuedouble;

            time_size = (strlen(time->valuestring) + 1) * sizeof(char);
            (*forecasts)[i].time = malloc(time_size);
            memcpy((*forecasts)[i].time, time->valuestring, time_size);
        }
    }

    /* TODO: Add past min maxs */

end:
    free(url_parameter);
    cJSON_Delete(daily_json);
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

void mtrlj_free_situation(struct mtrlj_situation situation)
{
    free(situation.time);
}

void mtrlj_free_forecasts(struct mtrlj_daily_forecast *pforecast)
{
    size_t i;
    /* This is always 5 */
    for (i = 0; i < 5; i++) {
        free(pforecast[i].time);
    }
}
#endif
