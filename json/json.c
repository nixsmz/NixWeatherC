/*
    File        : json.c
    Author      : nix.dev
    Date        : 17.03.23
    Version     : 2.0.0
    Description : JSON file reader API
*/
#include "json.h"

#define TEXT    "{\"latitude\":46.2,\"longitude\":6.1599994,\"generationtime_ms\":1.5770196914672852," \
                "\"utc_offset_seconds\":3600,\"timezone\":\"Europe/Zurich\"," \
                "\"timezone_abbreviation\":\"CET\",\"elevation\":392.0,\"daily\":{\"time\":[1679007600," \
                "1679094000,1679180400,1679266800,1679353200,1679439600,1679526000],\"weathercode\":[45," \
                "80,80,2,3,61,80],\"temperature_2m_max\":[17.0,14.8,13.1,15.0,17.5,16.3,11.2]," \
                "\"temperature_2m_min\":[1.3,2.1,7.2,3.4,5.2,8.3,8.9],\"sunrise\":[1679031781,1679118064," \
                "1679204348,1679290631,1679376914,1679463198,1679549481],\"sunset\":[1679075073," \
                "1679161555,1679248037,1679334518,1679421000,1679507481,1679593961]," \
                "\"precipitation_sum\":[0.00,0.30,4.30,0.00,0.00,0.90,15.30],\"windspeed_10m_max\":[18.8," \
                "12.3,14.9,8.1,20.4,25.2,10.9],\"winddirection_10m_dominant\":[216,219,205,89,216,232,254]}}"

static char* json_next_char(char *json) {
    char *ptr = json;
    for(; strchr("\t\n ", ptr[0]); ptr++);
    return ptr;
}

static char* json_next_field(char *json) {
    uint8_t level = 1;
    for(uint32_t i = 0; json[i] && level; i++) {
        switch(json[i]) {
            case '[': level++; break;
            case ']': level--; break;
            case '{': level++; break;
            case '}': level--; break;
            case ',': if(level == 1) return json_next_char(&(json[i+1]));
        }
    }
    return NULL;
}

static char* json_enter_level(char *json) {
    char *ptr = json;
    for(; !strchr("{[", ptr[0]); ptr++)
        if(ptr[0] == ',') return NULL;
    return json_next_char(++ptr);
}

static char* json_get_value(char* json) {
    char *ptr = json;
    for(; !strchr(":", ptr[0]); ptr++)
        if(ptr[0] == ',') return NULL;
    return json_next_char(++ptr);
}

static char* json_return(char *buffer, char *json) {
    uint32_t i;
    for(i = 0; json[i] && !strchr(",[{}]", json[i]); i++) buffer[i] = json[i];
    buffer[i] = 0;
    return buffer;
}

static char* json_search_r(char *buf, const char *path, char *json) {
    char p[JSON_BUFSIZE+2] = "\"";
    {
        uint32_t i;
        for(i = 0; path[i] && (path[i] != '/'); i++) p[i+1] = path[i];
        strcat(p, "\"");
    }
    char *v = strchr(path, '/'), *temp = json_enter_level(json);
    if(p[1] == '%') {
        for(int32_t j = 0; j < atoi(&p[2]); j++) temp = json_next_field(temp);
        if(v) return json_search_r(buf, (++v), temp);
        return json_return(buf, temp);
    }
    for(; temp; temp = json_next_field(temp)) {
        if(!strncmp(temp, p, strlen(p))) {
            temp = json_get_value(temp);
            if(v) return json_search_r(buf, (++v), temp);
            return json_return(buf, temp);
        }
    }
    return NULL;
}

char* json_search(char *buf, char *path, char *json) {
    char *p = path, *j = json;
    json_search_r(buf, p, j);
    if(buf[0] == '\"') {
        uint32_t i; for(i = 0; i < (strlen(buf)-2); i++) buf[i] = buf[i+1];
        buf[i] = 0;
    }
    return buf;
}
