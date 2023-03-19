#include "weather/weather.h"

// Example of city
#define CITY        "Geneva, CH"
#define CITY_LAT    "46.2"
#define CITY_LON    "6.16"

int main() {
    printf("Weather program.\n\n");

    char url[WEATHER_URLL];
    weather_url(url, W_PPRMS(CITY_LAT,CITY_LON,WEATHER_F_NONE));
    weather_t wt = { .city = CITY };
    if(weather_get(&wt, url)) {
        printf("Error in weather_get.\n");
        return 1;
    }
    weather_print(&wt);
    
    char condition[WEATHER_CONDL];
    if(!weather_conditions(condition, wt.days[0].wco))
        printf("Today's conditions: %s\n", condition);

    return 0;
}
