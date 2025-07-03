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

struct tm mtrlj_to_tm(struct mtrlj_time time)
{
    struct tm tm = {0};
    tm.tm_year = time.year - 1900;
    tm.tm_mon = time.month - 1;
    tm.tm_mday = time.day;
    tm.tm_hour = time.hour;
    tm.tm_min = time.minute;
    tm.tm_sec = time.second;
    return tm;
}

void print_time(struct mtrlj_time time)
{
    static char time_str[128];
    struct tm tm;

    tm = mtrlj_to_tm(time);
    strftime(time_str, 128, "%X %x", &tm);
    printf("Time: %s\n", time_str);
}

int main(int argc, char **argv)
{
    char *city_name = "Ankara";
    char *district_name = "";
    struct mtrlj_district district;
    struct mtrlj_situation situation;
    struct mtrlj_daily_forecast *forecasts;
    size_t i;

    setlocale(LC_TIME, ""); /* for strftime */

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
    print_time(situation.time);

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

    if (mtrlj_five_days_forecast(district, &forecasts) != MTRLJ_OK) {
        fprintf(stderr, "Could not get 5 days forecast for %s-%s\n",
                district.city_name, district.name);
        return 1;
    }

    for (i = 0; i < 5; i++) {
        print_time(forecasts[i].time);
    }

    mtrlj_free_district(district);
    mtrlj_free_forecasts(forecasts);
    return 0;
}
