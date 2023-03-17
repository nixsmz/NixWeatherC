/*
    File        : json.c
    Author      : nix.dev
    Date        : 17.03.23
    Version     : 2.0.0
    Description : JSON file reader API
*/
#include "json.h"

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
