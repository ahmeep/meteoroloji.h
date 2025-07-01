#define METEOROLOJI_IMPL
#include "../meteoroloji.h"

#include <stdio.h>
#include <stdlib.h>

int main()
{
    struct mtrlj_district *districts;
    size_t district_count;
    size_t i;

    if (mtrlj_get_districts_in_city(&districts, &district_count, "Ankara")
        != MTRLJ_OK) {
        printf("dümdüz bok\n");
        return 1;
    }

    for (i = 0; i < district_count; i++) {
        printf("%s\n", districts[i].name);
    }
    return 0;
}
