#ifndef _WEATHER_H_
#define _WEATHER_H_

// Libraries
#include "../json/json.h"
#include <curl/curl.h>

// Weather API source: https://open-meteo.com/

// Macros & equivalences
#define WEATHER_URL         "https://api.open-meteo.com/v1/forecast?latitude=%s&longitude=%s" \
                            "&hourly=temperature_2m,relativehumidity_2m,precipitation_probability,precipitation," \
                            "weathercode,windspeed_10m,winddirection_10m&daily=weathercode,temperature_2m_max," \
                            "temperature_2m_min,sunrise,sunset,precipitation_sum,windspeed_10m_max," \
                            "winddirection_10m_dominant&timeformat=unixtime&timezone=auto&forecast_days=%d"
#define WEATHER_URLL        (1024)

#define WEATHER_NDAYS       (7)

#define WEATHER_P_TEMP_C    "&temperature_unit=celsius"
#define WEATHER_F_TEMP_C    (0 << 0)
#define WEATHER_P_TEMP_F    "&temperature_unit=fahrenheit"
#define WEATHER_F_TEMP_F    (1 << 0)

#define WEATHER_M_WS        (0x3 << 4)
#define WEATHER_P_WS_KMH    "&windspeed_unit=kmh"
#define WEATHER_F_WS_KMH    (0 << 4)
#define WEATHER_P_WS_MS     "&windspeed_unit=ms"
#define WEATHER_F_WS_MS     (1 << 4)
#define WEATHER_P_WS_MPH    "&windspeed_unit=mph"
#define WEATHER_F_WS_MPH    (2 << 4)
#define WEATHER_P_WS_KN     "&windspeed_unit=kn"
#define WEATHER_F_WS_KN     (3 << 4)

#define WEATHER_M_PREC      (0x1 << 8)
#define WEATHER_P_PREC_MM   "&precipitation_unit=mm"
#define WEATHER_F_PREC_MM   (0 << 8)
#define WEATHER_P_PREC_IN   "&precipitation_unit=inch"
#define WEATHER_F_PREC_IN   (1 << 8)

#define WEATHER_F_NONE      (0)

#define W_PRMS(x,y,f)       ((weather_params_t){ .lat=x, .lon=y, .flags=f })
#define W_PPRMS(x,y,f)      (&(W_PRMS(x,y,f)))

// #define WEATHER_DEBUG

// Stuctures
typedef struct {
    char lat[16];
    char lon[16];
    uint32_t flags;
} weather_params_t;

typedef struct {
    time_t time;
    float temp;
    uint8_t hum;
    uint8_t prob;
    float prec;
    uint8_t wco;
    float ws;
    uint16_t wd;
} weather_hour_t;

typedef struct {
    time_t time;
    uint8_t wco;
    float tmax;
    float tmin;
    time_t sunrise;
    time_t sunset;
    float prec;
    float ws;
    uint16_t wd;
} weather_day_t;

typedef struct {
    char city[64];
    int32_t utc_offset;
    float altitude;
    char unit_temp[4];
    char unit_prec[6];
    char unit_speed[6];
    weather_hour_t hours[24];
    weather_day_t days[WEATHER_NDAYS];
} weather_t;

// Functions
void        weather_url(char *url, const weather_params_t *wp);
uint8_t     weather_get(weather_t *w, const char *url);
void        weather_print(weather_t *wt);

#endif
