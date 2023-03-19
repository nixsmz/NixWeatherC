#include "weather.h"

// Local structure
typedef struct {
	uint32_t length;
	char *ptr;
} weather_string_t;

// Function generating an url, depending on parameters
void weather_url(char *url, const weather_params_t *wp) {
    snprintf(url, WEATHER_URLL, WEATHER_URL, wp->lat, wp->lon, WEATHER_NDAYS);
    if(wp->flags & WEATHER_F_TEMP_F) strcat(url, WEATHER_P_TEMP_F);
    switch(wp->flags & WEATHER_M_WS) {
        case WEATHER_F_WS_MS: strcat(url, WEATHER_P_WS_MS); break;
        case WEATHER_F_WS_MPH: strcat(url, WEATHER_P_WS_MPH); break;
        case WEATHER_F_WS_KN: strcat(url, WEATHER_P_WS_KN); break;
    }
    if(wp->flags & WEATHER_M_PREC) strcat(url, WEATHER_P_PREC_IN);
}

// Write function for curl
static size_t weather_write(void *ptr, size_t size, size_t nmemb, weather_string_t *string) {
	size_t block = size * nmemb;
	if(!block) return 0;
	if(!string->ptr) string->ptr = calloc(block+1, sizeof(char));
	else string->ptr = realloc(string->ptr, string->length+block+1);
	memcpy(&(string->ptr[string->length]), ptr, block);
	string->length += block;
	string->ptr[string->length] = 0;
	return block;
}

static void weather_print_hour(weather_hour_t* wh) {
    struct tm *info = localtime(&wh->time);
    char buf[64];
    strftime(buf, 64, "%d.%m.%Y %H:%M", info);
    printf("\tDatetime: %s\n", buf);
    printf("\tTemperature: %.2f\n", wh->temp);
    printf("\tHumidity: %d\n", wh->hum);
    printf("\tPrecipitation chance: %d\n", wh->prob);
    printf("\tPrecipitations: %.2f\n", wh->prec);
    printf("\tWeathercode: %d\n", wh->wco);
    printf("\tWind speed: %.2f\n", wh->ws);
    printf("\tWind direction: %d\n\n", wh->wd);
}

static void weather_print_day(weather_day_t *wd) {
    char buf[64]; struct tm *info = localtime(&wd->time);
    strftime(buf, 64, "%d.%m.%Y %H:%M", info);
    printf("\tDatetime: %s\n", buf);
    printf("\tTemperature: %.2f / %.2f\n", wd->tmin, wd->tmax);
    info = localtime(&wd->sunrise);
    strftime(buf, 64, "%d.%m.%Y %H:%M", info);
    printf("\tSunrise: %s\n", buf);
    info = localtime(&wd->sunset);
    strftime(buf, 64, "%d.%m.%Y %H:%M", info);
    printf("\tSunset: %s\n", buf);
    printf("\tWeathercode: %d\n", wd->wco);
    printf("\tPrecipitations: %.2f\n", wd->prec);
    printf("\tWind speed: %.2f\n", wd->ws);
    printf("\tWind direction: %d\n\n", wd->wd);
}

void weather_print(weather_t *wt) {
    printf("Weather for %s:\n", wt->city);
    printf("UTC: %+d\n", wt->utc_offset / 3600);
    printf("Altitude: %.2fm\n\n", wt->altitude);
    printf("Units: %s, %s, %s\n", wt->unit_temp, wt->unit_speed, wt->unit_prec);
    printf("\nHourly forecast:\n");
    for(uint32_t i = 0; i < 24; i++) weather_print_hour(&(wt->hours[i]));
    printf("\nDaily forecast:\n");
    for(uint32_t i = 0; i < WEATHER_NDAYS; i++) weather_print_day(&(wt->days[i]));
}

// Function getting weather data
uint8_t weather_get(weather_t *w, const char *url) {
    weather_string_t string = { .length = 0, .ptr = NULL };
    CURL *curl = curl_easy_init();
    if(!curl) return -1;
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, weather_write);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &string);
    CURLcode res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
        printf("\33[1;31mE\33[0m %s\n", curl_easy_strerror(res));
        return -1;
    }
    #ifdef WEATHER_DEBUG
    printf("%s (%d)\n\n", string.ptr, string.length);
    #endif
    char buf[JSON_BUFSIZE] = "", path[128] = "";
    // Recuperation des infos
    w->utc_offset = atoi(json_search(buf, "utc_offset_seconds", string.ptr));
    w->altitude = atof(json_search(buf, "elevation", string.ptr));
    strcpy(w->unit_temp, json_search(buf, "hourly_units/temperature_2m", string.ptr));
    strcpy(w->unit_speed, json_search(buf, "hourly_units/windspeed_10m", string.ptr));
    strcpy(w->unit_prec, json_search(buf, "hourly_units/precipitation", string.ptr));
    // Recuperation heure par heure
    uint8_t ho = 0;
    {
        time_t tmt = time(NULL);
        struct tm *inf = localtime(&tmt);
        ho = inf->tm_hour;
    }
    for(uint32_t i = 0; i < 24; i++) {
        // Time
        snprintf(path, 128, "hourly/time/%%%d", i+ho);
        w->hours[i].time = atoi(json_search(buf, path, string.ptr));
        // Temperature
        snprintf(path, 128, "hourly/temperature_2m/%%%d", i+ho);
        w->hours[i].temp = atof(json_search(buf, path, string.ptr));
        // Humidity
        snprintf(path, 128, "hourly/relativehumidity_2m/%%%d", i+ho);
        w->hours[i].hum = atoi(json_search(buf, path, string.ptr));
        // Precipitation chance
        snprintf(path, 128, "hourly/precipitation_probability/%%%d", i+ho);
        w->hours[i].prob = atoi(json_search(buf, path, string.ptr));
        // Precipitations
        snprintf(path, 128, "hourly/precipitation/%%%d", i+ho);
        w->hours[i].prec = atof(json_search(buf, path, string.ptr));
        // Weather code
        snprintf(path, 128, "hourly/weathercode/%%%d", i+ho);
        w->hours[i].wco = atoi(json_search(buf, path, string.ptr));
        // Wind speed
        snprintf(path, 128, "hourly/windspeed_10m/%%%d", i+ho);
        w->hours[i].ws = atof(json_search(buf, path, string.ptr));
        // Wind direction
        snprintf(path, 128, "hourly/winddirection_10m/%%%d", i+ho);
        w->hours[i].wd = atoi(json_search(buf, path, string.ptr));
    }
    // Recuperationdes 7 prochains jours
    for(uint32_t i = 0; i < WEATHER_NDAYS; i++) {
        // Time
        snprintf(path, 128, "daily/time/%%%d", i);
        w->days[i].time = atoi(json_search(buf, path, string.ptr));
        // Minimal temperature
        snprintf(path, 128, "daily/temperature_2m_min/%%%d", i);
        w->days[i].tmin = atof(json_search(buf, path, string.ptr));
        // Maximal temperature
        snprintf(path, 128, "daily/temperature_2m_max/%%%d", i);
        w->days[i].tmax = atof(json_search(buf, path, string.ptr));
        // Surise
        snprintf(path, 128, "daily/sunrise/%%%d", i);
        w->days[i].sunrise = atoi(json_search(buf, path, string.ptr));
        // Sunset
        snprintf(path, 128, "daily/sunset/%%%d", i);
        w->days[i].sunset = atoi(json_search(buf, path, string.ptr));
        // Precipitations
        snprintf(path, 128, "daily/precipitation_sum/%%%d", i);
        w->days[i].prec = atof(json_search(buf, path, string.ptr));
        // Weather code
        snprintf(path, 128, "daily/weathercode/%%%d", i);
        w->days[i].wco = atoi(json_search(buf, path, string.ptr));
        // Wind speed
        snprintf(path, 128, "daily/windspeed_10m_max/%%%d", i);
        w->days[i].ws = atof(json_search(buf, path, string.ptr));
        // Wind direction
        snprintf(path, 128, "daily/winddirection_10m_dominant/%%%d", i);
        w->days[i].wd = atoi(json_search(buf, path, string.ptr));
    }
    if(string.ptr) free(string.ptr);
    curl_easy_cleanup(curl);
    return 0;
}

// Function getting weather conditions
uint8_t weather_conditions(char *buffer, weathercode_t code) {
    FILE *f = fopen(WEATHER_CONDFILE, "r");
    if(!f) return -1;
    char *token, temp[WEATHER_CONDL];
    while(fgets(temp, WEATHER_CONDL, f)) {
        token = strtok(temp, WEATHER_CONDSEP);
        if(atoi(token) == code) {
            strncpy(buffer, strtok(NULL, WEATHER_CONDSEP), WEATHER_CONDL);
            return 0;
        }
    }
    fclose(f);
    return -1;
}
