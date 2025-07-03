#define METEOROLOJI_IMPL
#include "../meteoroloji.h"

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

/* this will modify the str because of strtok!!! */
struct tm parse_time(char *str)
{
    struct tm time = {0};
    char *after_t = strchr(str, 'T') + 1;
    char *part = strtok(str, "-");
    time.tm_year = atoi(part) - 1900;
    part = strtok(NULL, "-");
    time.tm_mon = atoi(part) - 1;
    part = strtok(NULL, "-");
    time.tm_mday = atoi(part);
    part = strtok(after_t, ":");
    time.tm_hour = atoi(part) + 3; /* convert to Türkiye time */
    part = strtok(NULL, ":");
    time.tm_min = atoi(part);
    part = strtok(NULL, ":");
    part = strtok(part, ".");
    time.tm_sec = atoi(part);
    return time;
}

int main(int argc, char **argv)
{
    char *city_name = "Ankara";
    char *district_name = "";
    struct mtrlj_district district;
    struct mtrlj_situation situation;

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

    printf("City: %s\n", district.city_name);
    printf("District: %s\n", district.name);
    {
        char time_str[128];
        struct tm time;

        setlocale(LC_TIME, "");
        time = parse_time(situation.time);
        strftime(time_str, 128, "%X %x", &time);
        printf("Time: %s\n", time_str);
    }

    if (value_available(situation.temperature))
        printf("Temperature: %.2f°C\n", situation.temperature);

    if (value_available(situation.humidity_percent))
        printf("Humidity: %%%d\n", (int)situation.humidity_percent);

    if (value_available(situation.cloudiness_percent))
        printf("Cloudiness: %%%d\n", (int)situation.cloudiness_percent);

    if (value_available(situation.wind_speed)
        && value_available(situation.wind_direction))
        printf("Wind: %.2f kmh from %s\n", situation.wind_speed,
               direction_text(situation.wind_direction));

    if (value_available(situation.rainfall))
        printf("Rainfall: %d mm\n", (int)situation.rainfall);

    if (value_available(situation.actual_pressure))
        printf("Pressure: %.2f hPa\n", situation.actual_pressure);

    if (value_available(situation.reduced_pressure_at_sea))
        printf("Pressure (sea): %.2f hPa\n", situation.reduced_pressure_at_sea);

    if (value_available(situation.sea_temperature))
        printf("Temperature (sea): %.2f°C\n", situation.sea_temperature);

    if (value_available(situation.snow_height))
        printf("Snow Height: %d m\n", (int)situation.snow_height);

    mtrlj_free_district(district);
    mtrlj_free_situation(situation);
    return 0;
}
