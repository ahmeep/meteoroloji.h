#define METEOROLOJI_IMPL
#include "../meteoroloji.h"

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>

int value_available(double val)
{
    return val != -9999 && val != -99;
}

const char *direction_text(double degrees)
{
    if ((degrees >= 0 && degrees < 22.5) || (degrees > 337.5 && degrees <= 360))
        return "North";

    if (degrees >= 22.5 && degrees < 67.5)
        return "Northeast";

    if (degrees >= 67.5 && degrees < 112.5)
        return "East";

    if (degrees >= 112.5 && degrees < 157.5)
        return "Southeast";

    if (degrees >= 157.5 && degrees < 202.5)
        return "South";

    if (degrees >= 202.5 && degrees < 247.5)
        return "Southwest";

    if (degrees >= 247.5 && degrees < 292.5)
        return "West";

    if (degrees >= 292.5 && degrees <= 337.5)
        return "Northwest";

    return "";
}

const char *condition_text(MTRLJ_WEATHER_CONDITION condition)
{
    switch (condition) {
    case MTRLJ_WEATHER_CLEAR:
        return "Clear";
    case MTRLJ_WEATHER_SOME_CLOUDS:
        return "Some Clouds";
    case MTRLJ_WEATHER_PARTLY_CLOUDY:
        return "Partly Cloudy";
    case MTRLJ_WEATHER_MOSTLY_CLOUDY:
        return "Mostly Cloudy";
    case MTRLJ_WEATHER_LIGHT_RAINY:
        return "Light Rainy";
    case MTRLJ_WEATHER_RAINY:
        return "Rainy";
    case MTRLJ_WEATHER_HEAVY_RAINY:
        return "Heavy Rainy";
    case MTRLJ_WEATHER_SLEETY:
        return "Sleety";
    case MTRLJ_WEATHER_LIGHT_SNOWY:
        return "Light Snowy";
    case MTRLJ_WEATHER_SNOWY:
        return "Snowy";
    case MTRLJ_WEATHER_HEAVY_SNOWY:
        return "Heavy Snowy";
    case MTRLJ_WEATHER_LIGHT_DOWNPOURS:
        return "Light Downpours";
    case MTRLJ_WEATHER_DOWNPOURS:
        return "Downpours";
    case MTRLJ_WEATHER_HEAVY_DOWNPOURS:
        return "Heavy Downpours";
    case MTRLJ_WEATHER_ISOLATED_DOWNPOURS:
        return "Isolated Downpours";
    case MTRLJ_WEATHER_HAIL:
        return "Hail";
    case MTRLJ_WEATHER_THUNDERY_SHOWERS:
        return "Thundery Showers";
    case MTRLJ_WEATHER_HEAVY_THUNDERY_SHOWERS:
        return "Heavy Thundery Showers";
    case MTRLJ_WEATHER_FOGGY:
        return "Foggy";
    case MTRLJ_WEATHER_HAZY:
        return "Hazy";
    case MTRLJ_WEATHER_SMOGGY:
        return "Smoggy";
    case MTRLJ_WEATHER_DUST_STORM:
        return "Dust Storm";
    case MTRLJ_WEATHER_WINDY:
        return "Windy";
    case MTRLJ_WEATHER_STRONG_SOUTHERLY_WIND:
        return "Strong Southerly Wind";
    case MTRLJ_WEATHER_STRONG_NORTHERLY_WIND:
        return "Strong Northerly Wind";
    case MTRLJ_WEATHER_WARM:
        return "Warm";
    case MTRLJ_WEATHER_COLD:
        return "Cold";
    default:
        return "";
    }
}

struct table_element {
    wchar_t *field;
    size_t field_len;
    wchar_t *value;
    size_t value_len;
    size_t len; /* max(strlen(field), strlen(value)) */
};

void print_table(char **fields, char **values, size_t count)
{
    wchar_t *table_string;
    wchar_t *cursor;
    struct table_element *elements;
    size_t i;

#define MAX_CHARACTERS 16000 /* just a reasonable amount will be alright. */
    table_string = calloc(MAX_CHARACTERS, sizeof(wchar_t));
    cursor = table_string;

    elements = calloc(count, sizeof(struct table_element));
    for (i = 0; i < count; i++) {
        size_t field_len = mbstowcs(NULL, fields[i], 0);
        size_t value_len = mbstowcs(NULL, values[i], 0);
        elements[i].field_len = field_len;
        elements[i].value_len = value_len;
        elements[i].field = calloc(field_len + 1, sizeof(wchar_t));
        elements[i].value = calloc(value_len + 1, sizeof(wchar_t));
        mbstowcs(elements[i].field, fields[i], field_len + 1);
        mbstowcs(elements[i].value, values[i], value_len + 1);
        elements[i].len = field_len < value_len ? value_len : field_len;
    }

    /* First row */
    {
        *(cursor++) = L'┌';
        for (i = 0; i < count; i++) {
            wmemset(cursor, L'─', elements[i].len);
            cursor += elements[i].len;
            if (i == count - 1)
                *(cursor++) = L'┐';
            else
                *(cursor++) = L'┬';
        }
        *(cursor++) = L'\n';
    }

    /* Second row (fields) */
    {
        *(cursor++) = L'│';
        for (i = 0; i < count; i++) {
            size_t spaces = elements[i].len - elements[i].field_len;
            size_t front_spaces = spaces / 2;
            size_t rear_spaces = front_spaces + spaces % 2;

            wmemset(cursor, L' ', front_spaces);
            cursor += front_spaces;
            wmemcpy(cursor, elements[i].field, elements[i].field_len);
            cursor += elements[i].field_len;
            wmemset(cursor, L' ', rear_spaces);
            cursor += rear_spaces;
            *(cursor++) = L'│';
        }
        *(cursor++) = L'\n';
    }

    /* Third row (seperator) */
    {
        *(cursor++) = L'├';
        for (i = 0; i < count; i++) {
            wmemset(cursor, L'─', elements[i].len);
            cursor += elements[i].len;
            if (i == count - 1)
                *(cursor++) = L'┤';
            else
                *(cursor++) = L'┼';
        }
        *(cursor++) = L'\n';
    }

    /* Fourth row (values) */
    {
        *(cursor++) = L'│';
        for (i = 0; i < count; i++) {
            size_t spaces = elements[i].len - elements[i].value_len;
            size_t front_spaces = spaces / 2;
            size_t rear_spaces = front_spaces + spaces % 2;

            wmemset(cursor, L' ', front_spaces);
            cursor += front_spaces;
            wmemcpy(cursor, elements[i].value, elements[i].value_len);
            cursor += elements[i].value_len;
            wmemset(cursor, L' ', rear_spaces);
            cursor += rear_spaces;
            *(cursor++) = L'│';
        }
        *(cursor++) = L'\n';
    }

    /* Fifth row */
    {
        *(cursor++) = L'└';
        for (i = 0; i < count; i++) {
            wmemset(cursor, L'─', elements[i].len);
            cursor += elements[i].len;
            if (i == count - 1)
                *(cursor++) = L'┘';
            else
                *(cursor++) = L'┴';
        }
        *(cursor++) = L'\n';
    }

    {
        char *mbstring;
        size_t size = wcstombs(NULL, table_string, 0);
        mbstring = calloc(size + 1, sizeof(char));
        wcstombs(mbstring, table_string, size + 1);
        printf("%s", mbstring);
        free(mbstring);
    }

    for (i = 0; i < count; i++) {
        free(elements[i].field);
        free(elements[i].value);
    }
    free(elements);
    free(table_string);
}

struct tm mtrlj_to_tm(struct mtrlj_time time)
{
    struct tm tm = {0};
    tm.tm_year = time.year - 1900;
    tm.tm_mon = time.month - 1;
    tm.tm_mday = time.day;
    tm.tm_hour = time.hour;
    tm.tm_min = time.minute;
    tm.tm_sec = time.second;
    mktime(&tm);
    return tm;
}

char *tmtostr(struct mtrlj_time time, char *fmt)
{
    char *str;
    struct tm tm;

    str = calloc(129, sizeof(char));
    tm = mtrlj_to_tm(time);
    strftime(str, 128, fmt, &tm);
    return str;
}

int main(int argc, char **argv)
{
    char *city_name = "Ankara";
    char *district_name = "";
    struct mtrlj_district district;
    struct mtrlj_situation situation;
    struct mtrlj_hourly_forecast *hourly_forecasts;
    size_t hourly_forecast_count;
    struct mtrlj_daily_forecast *daily_forecasts;
    size_t i;

    setlocale(LC_ALL, "en_US.utf8");
    setlocale(LC_TIME, "");

    if (argc > 1) {
        city_name = argv[1];
    }

    if (argc > 2) {
        district_name = argv[2];
    }

    if (mtrlj_get_district(&district, city_name, district_name) != MTRLJ_OK) {
        fprintf(stderr, "Could not find district %s-%s\n", city_name,
                district_name);
        return 1;
    }

    if (mtrlj_latest_situation(district, &situation) != MTRLJ_OK) {
        fprintf(stderr, "Could not get latest situation for %s-%s\n",
                district.city_name, district.name);
        return 1;
    }

    {
        char *fields[4] = {"City", "District", "Time", "Condition"};
        char *values[4];

        values[0] = district.city_name;
        values[1] = district.name;
        values[2] = tmtostr(situation.time, "%X %x");
        values[3] = calloc(32, sizeof(char));

        strcpy(values[3], condition_text(situation.condition));

        print_table(fields, values, 4);

        free(values[2]);
        free(values[3]);
    }

    {
        char *fields[9];
        char **values = calloc(9, sizeof(char *));
        size_t count = 0;

        for (i = 0; i < 9; i++) {
            values[i] = calloc(65, sizeof(char));
        }

        if (value_available(situation.temperature)) {
            fields[count] = "Temperature";
            sprintf(values[count], "%.2f°C", situation.temperature);
            count++;
        }

        if (value_available(situation.humidity_percent)) {
            fields[count] = "Humidity";
            sprintf(values[count], "%%%d", (int)situation.humidity_percent);
            count++;
        }

        if (value_available(situation.cloudiness_percent)) {
            fields[count] = "Cloudiness";
            sprintf(values[count], "%%%d", (int)situation.cloudiness_percent);
            count++;
        }

        if (value_available(situation.wind_speed)
            && value_available(situation.wind_direction)) {
            fields[count] = "Wind";
            sprintf(values[count], "%.2f kmh from %s", situation.wind_speed,
                    direction_text(situation.wind_direction));
            count++;
        }

        if (value_available(situation.rainfall)) {
            fields[count] = "Rainfall";
            sprintf(values[count], "%d mm", (int)situation.rainfall);
            count++;
        }

        if (value_available(situation.actual_pressure)) {
            fields[count] = "Pressure";
            sprintf(values[count], "%.2f hPa", situation.actual_pressure);
            count++;
        }

        if (value_available(situation.reduced_pressure_at_sea)) {
            fields[count] = "Pressure (sea)";
            sprintf(values[count], "%.2f hPa",
                    situation.reduced_pressure_at_sea);
            count++;
        }

        if (value_available(situation.sea_temperature)) {
            fields[count] = "Temperature (sea)";
            sprintf(values[count], "%.2f°C", situation.sea_temperature);
            count++;
        }

        if (value_available(situation.snow_height)) {
            fields[count] = "Snow Height";
            sprintf(values[count], "%d m", (int)situation.snow_height);
            count++;
        }

        print_table(fields, values, count);

        for (i = 0; i < 9; i++) {
            free(values[i]);
        }
        free(values);
    }

    if (mtrlj_hourly_forecasts(district, &hourly_forecasts,
                               &hourly_forecast_count)
        != MTRLJ_OK) {
        fprintf(stderr, "Could not get hourly forecasts for %s-%s\n",
                district.city_name, district.name);
        return 1;
    }

    for (i = 0; i < hourly_forecast_count; i++) {
        struct mtrlj_hourly_forecast forecast = hourly_forecasts[i];
        struct mtrlj_time time = forecast.time;
        char *day = tmtostr(time, "%A");
        char *starthour = tmtostr(time, "%R");
        char *endhour;

        time.hour += 3;
        endhour = tmtostr(time, "%R");

        printf("Forecast for %s %s-%s\n", day, starthour, endhour);

        free(day);
        free(starthour);
        free(endhour);
        {
            size_t j;
            char *fields[7];
            char **values = calloc(7, sizeof(char *));
            size_t count = 0;

            for (j = 0; j < 7; j++) {
                values[j] = calloc(65, sizeof(char));
            }

            fields[count] = "Condition";
            sprintf(values[count], "%s", condition_text(forecast.condition));
            count++;

            if (value_available(forecast.temperature)) {
                fields[count] = "Temperature";
                sprintf(values[count], "%.2f°C", forecast.temperature);
                count++;
            }

            if (value_available(forecast.felt_temperature)) {
                fields[count] = "Felt Temperature";
                sprintf(values[count], "%.2f°C", forecast.felt_temperature);
                count++;
            }

            if (value_available(forecast.humidity_percent)) {
                fields[count] = "Humidity";
                sprintf(values[count], "%%%d", (int)forecast.humidity_percent);
                count++;
            }

            if (value_available(forecast.wind_direction)) {
                fields[count] = "Wind Direction";
                sprintf(values[count], "%s",
                        direction_text(forecast.wind_direction));
                count++;
            }

            if (value_available(forecast.wind_speed_avg)) {
                fields[count] = "Avg Wind Speed";
                sprintf(values[count], "%.2f kmh", forecast.wind_speed_avg);
                count++;
            }

            if (value_available(forecast.wind_speed_max)) {
                fields[count] = "Max Wind Speed";
                sprintf(values[count], "%.2f kmh", forecast.wind_speed_max);
                count++;
            }

            print_table(fields, values, count);

            for (j = 0; j < 7; j++) {
                free(values[j]);
            }
            free(values);
        }
    }

    if (mtrlj_five_days_forecast(district, &daily_forecasts) != MTRLJ_OK) {
        fprintf(stderr, "Could not get 5 days forecast for %s-%s\n",
                district.city_name, district.name);
        return 1;
    }

    for (i = 0; i < 5; i++) {
        struct mtrlj_daily_forecast forecast = daily_forecasts[i];
        struct mtrlj_time time = forecast.time;
        char *timestr = tmtostr(time, "%x");

        printf("Forecast for %s\n", timestr);
        free(timestr);

        {
            size_t j;
            char *fields[6];
            char **values = calloc(6, sizeof(char *));
            size_t count = 0;

            for (j = 0; j < 6; j++) {
                values[j] = calloc(65, sizeof(char));
            }

            fields[count] = "Condition";
            sprintf(values[count], "%s", condition_text(forecast.condition));
            count++;

            if (value_available(forecast.temperature_min)
                && value_available(forecast.temperature_max)) {
                fields[count] = "Temperature (min/max)";
                sprintf(values[count], "%.2f°C/%.2f°C",
                        forecast.temperature_min, forecast.temperature_max);
                count++;
            }

            if (value_available(forecast.humidity_min)
                && value_available(forecast.humidity_max)) {
                fields[count] = "Humidity (min/max)";
                sprintf(values[count], "%%%d/%%%d", (int)forecast.humidity_min,
                        (int)forecast.humidity_max);
                count++;
            }

            if (value_available(forecast.wind_speed)
                && value_available(forecast.wind_direction)) {
                fields[count] = "Wind";
                sprintf(values[count], "%.2f kmh from %s", forecast.wind_speed,
                        direction_text(forecast.wind_direction));
                count++;
            }

            if (value_available(forecast.past_peak_temperature_min)
                && value_available(forecast.past_peak_temperature_max)) {
                fields[count] = "Peak Temp In Past (min/max)";
                sprintf(values[count], "%.2f°C/%.2f°C",
                        forecast.past_peak_temperature_min,
                        forecast.past_peak_temperature_max);
                count++;
            }

            if (value_available(forecast.past_average_temperature_min)
                && value_available(forecast.past_average_temperature_max)) {
                fields[count] = "Avg Temp In Past (min/max)";
                sprintf(values[count], "%.2f°C/%.2f°C",
                        forecast.past_average_temperature_min,
                        forecast.past_average_temperature_max);
                count++;
            }

            print_table(fields, values, count);

            for (j = 0; j < 6; j++) {
                free(values[j]);
            }
            free(values);
        }
    }

    mtrlj_free_district(district);
    mtrlj_free_daily_forecasts(daily_forecasts);
    mtrlj_free_hourly_forecasts(hourly_forecasts);
    return 0;
}
