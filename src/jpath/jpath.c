/*
 * Copyright (c) 2021 Tai-Yuan Fang <taiyuan.fang@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#define DEBUG_MODULE "jpath"
#define DEBUG_LEVEL 5

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "cJSON.h"
#include "jpath_expr.h"
#include "jpath.h"
//------------------------------------------------------------------------------
cJSON * json_loads(const char *str) {
    if (str && *str)
        return cJSON_Parse(str);

    return NULL;
}
//------------------------------------------------------------------------------
cJSON * json_loadf(const char *filename) {
    int fd = -1;
    void *ptr = MAP_FAILED;
    cJSON *json = NULL;
    struct stat fst;

    memset(&fst, 0x0, sizeof(struct stat));

    if (stat(filename, &fst) < 0 || fst.st_size < 1)
        return NULL;

    fd = open(filename, O_RDONLY);
    if (fd < 0)
        return NULL;

    ptr = mmap(NULL, fst.st_size, PROT_READ, MAP_SHARED, fd, 0);
    close(fd);

    if (MAP_FAILED == ptr)
        return NULL;

    json = json_loads((const char *)ptr);
    munmap(ptr, fst.st_size);
    return json;
}
//------------------------------------------------------------------------------
char * json_tostring(cJSON *json) {
    if (!json)
        return NULL;

    return cJSON_Print(json);
}
//------------------------------------------------------------------------------
static cJSON * create_object_child(cJSON *parent, const char *name) {
    cJSON *new_item = NULL;

    if (cJSON_IsNull(parent))
        parent->type = cJSON_Object;

    if (cJSON_IsObject(parent)) {
        new_item = cJSON_CreateNull();
        cJSON_AddItemToObject(parent, name, new_item);
    }

    return new_item;
}
//------------------------------------------------------------------------------
static cJSON * create_array_child(cJSON *parent) {
    cJSON *new_item = NULL;

    if (cJSON_IsNull(parent))
        parent->type = cJSON_Array;

    if (cJSON_IsArray(parent)) {
        new_item = cJSON_CreateNull();
        cJSON_AddItemToArray(parent, new_item);
    }

    return new_item;
}
//------------------------------------------------------------------------------
cJSON * json_get(cJSON *json, const char *path, int auto_create) {
    jpath_expr_p expr = NULL;
    jpath_expr_p curr = NULL;
    jpath_expr_p next = NULL;
    cJSON *parent = json;
    cJSON *child = NULL;
    int n = 0;

    if (!json)
        return NULL;

    expr = jpath_expr_parse(path);
    if (!expr)
        return NULL;

    curr = expr;

    while (curr) {
        next = curr->next;

        if (child) {
            parent = child;
            child = NULL;
        }

        if (curr->name) {
            if (cJSON_IsObject(parent))
                child = cJSON_GetObjectItemCaseSensitive(parent, curr->name);

            if (!child && auto_create)
                child = create_object_child(parent, curr->name);
        } else {
            if (cJSON_IsArray(parent)) {
                n = cJSON_GetArraySize(parent);

                if (curr->index < 0) {
                    if (n > 0)
                        curr->index += n;

                    if (curr->index < 0)
                        curr->index = 0;
                }

                if (curr->index < n)
                    child = cJSON_GetArrayItem(parent, curr->index);
            } else
                n = 0;

            if (!child && auto_create) {
                while (n <= curr->index) {
                    create_array_child(parent);
                    ++n;
                }
                child = cJSON_GetArrayItem(parent, curr->index);
            }
        }

        if (!child)
            break;

        curr = next;
    }

    jpath_expr_free(expr);
    return child;
}
//------------------------------------------------------------------------------
int json_set_str(cJSON *json, const char *path, const char *val) {
    cJSON *obj = json_get(json, path, 1);

    if (obj) {
        if (cJSON_Number == obj->type) {
            // try to convert
            if (sscanf(val, "%lf", &(obj->valuedouble)) == 1) {
                obj->valueint = (int)(obj->valuedouble);
                return 0;
            }
        }

        if (obj->valuestring) {
            free(obj->valuestring);
            obj->valuestring = NULL;
        }

        obj->type = cJSON_String;
        obj->valuestring = malloc(strlen(val) + 1);
        strcpy(obj->valuestring, val);
        return 0;
    }

    return -1;
}
//------------------------------------------------------------------------------
int json_set_int(cJSON *json, const char *path, const int val) {
    cJSON *obj = json_get(json, path, 1);
    if (obj) {
        if (obj->valuestring) {
            free(obj->valuestring);
            obj->valuestring = NULL;
        }

        obj->type = cJSON_Number;
        cJSON_SetIntValue(obj, val);
        return 0;
    }

    return -1;
}
//------------------------------------------------------------------------------
int json_set_double(cJSON *json, const char *path, const double val) {
    cJSON *obj = json_get(json, path, 1);
    if (obj) {
        if (obj->valuestring) {
            free(obj->valuestring);
            obj->valuestring = NULL;
        }

        obj->type = cJSON_Number;
        cJSON_SetNumberValue(obj, val);
        return 0;
    }

    return -1;
}
//------------------------------------------------------------------------------
int json_get_str(cJSON *json, const char *path, char *buff, int size) {
    cJSON *obj = json_get(json, path, 0);
    int len = -1;

    if (!obj)
        return -1;

    switch (obj->type) {
        case cJSON_Number:
            if ((int64_t)(obj->valueint) != (int64_t)(obj->valuedouble))
                snprintf(buff, size - 1, "%lf", obj->valuedouble);
            else
                snprintf(buff, size - 1, "%d", obj->valueint);
            len = strlen(buff);
            break;

        case cJSON_String:
            len = strlen(obj->valuestring);
            if (len >= size)
                len = size - 1;
            if (len > 0)
                strncpy(buff, obj->valuestring, len);
            break;

        case cJSON_NULL:
            buff[0] = 0;
            len = 0;
            break;

        default:
            buff[0] = 0;
            len = -1;
            break;
    }

    return len;
}
//------------------------------------------------------------------------------
int json_get_int(cJSON *json, const char *path, const int default_val) {
    cJSON *obj = json_get(json, path, 0);

    if (!obj)
        return default_val;

    if (cJSON_IsNumber(obj))
        return obj->valueint;

    if (cJSON_IsString(obj)) {
        int result = 0;
        if (1 == sscanf(obj->valuestring, "%d", &result))
            return result;
    }

    return default_val;
}
//------------------------------------------------------------------------------
double json_get_double(cJSON *json, const char *path, const double default_val) {
    cJSON *obj = json_get(json, path, 0);

    if (!obj)
        return default_val;

    if (cJSON_IsNumber(obj))
        return obj->valuedouble;

    if (cJSON_IsString(obj)) {
        double result = 0;
        if (1 == sscanf(obj->valuestring, "%lf", &result))
            return result;
    }

    return default_val;
}
//------------------------------------------------------------------------------
int jsonf_get_str(const char *filename, const char *path, char *buff, int size) {
    cJSON *json = NULL;
    int res = -1;

    json = json_loadf(filename);
    if (!json)
        return -1;

    res = json_get_str(json, path, buff, size);
    cJSON_Delete(json);
    return res;
}
//------------------------------------------------------------------------------
int jsonf_get_int(const char *filename, const char *path, const int default_val) {
    cJSON *json = NULL;
    int res = default_val;

    json = json_loadf(filename);
    if (!json)
        return default_val;

    res = json_get_int(json, path, default_val);
    cJSON_Delete(json);
    return res;
}
//------------------------------------------------------------------------------
double jsonf_get_double(const char *filename, const char *path, const double default_val) {
    cJSON *json = NULL;
    double res = default_val;

    json = json_loadf(filename);
    if (!json)
        return default_val;

    res = json_get_double(json, path, default_val);
    cJSON_Delete(json);
    return res;
}
//------------------------------------------------------------------------------
int jsonf_set_str(const char *filename, const char *path, const char *val) {
    cJSON *json = NULL;
    int res = -1;

    json = json_loadf(filename);
    if (!json)
        return -1;

    if (0 == json_set_str(json, path, val)) {
        FILE *fp = fopen(filename, "w");
        if (fp) {
            char *doc = json_tostring(json);
            fwrite(doc, 1, strlen(doc), fp);
            fclose(fp);
            free(doc);
            res = 0;
        }
    }

    cJSON_Delete(json);
    return res;
}
//------------------------------------------------------------------------------
int jsonf_set_int(const char *filename, const char *path, const int val) {
    cJSON *json = NULL;
    int res = -1;

    json = json_loadf(filename);
    if (!json)
        return -1;

    if (0 == json_set_int(json, path, val)) {
        FILE *fp = fopen(filename, "w");
        if (fp) {
            char *doc = json_tostring(json);
            fwrite(doc, 1, strlen(doc), fp);
            fclose(fp);
            free(doc);
            res = 0;
        }
    }

    cJSON_Delete(json);
    return res;
}
//------------------------------------------------------------------------------
int jsonf_set_double(const char *filename, const char *path, const double val) {
    cJSON *json = NULL;
    int res = -1;

    json = json_loadf(filename);
    if (!json)
        return -1;

    if (0 == json_set_double(json, path, val)) {
        FILE *fp = fopen(filename, "w");
        if (fp) {
            char *doc = json_tostring(json);
            fwrite(doc, 1, strlen(doc), fp);
            fclose(fp);
            free(doc);
            res = 0;
        }
    }

    cJSON_Delete(json);
    return res;
}
//------------------------------------------------------------------------------
int jsons_get_str(const char *json_str, const char *path, char *buff, int size) {
    cJSON *json = NULL;
    int res = -1;

    json = json_loads(json_str);
    if (!json)
        return -1;

    res = json_get_str(json, path, buff, size);
    cJSON_Delete(json);
    return res;
}
//------------------------------------------------------------------------------
int jsons_get_int(const char *json_str, const char *path, const int default_val) {
    cJSON *json = NULL;
    int res = default_val;

    json = json_loads(json_str);
    if (!json)
        return default_val;

    res = json_get_int(json, path, default_val);
    cJSON_Delete(json);
    return res;
}
//------------------------------------------------------------------------------
double jsons_get_double(const char *json_str, const char *path, const double default_val) {
    cJSON *json = NULL;
    double res = default_val;

    json = json_loads(json_str);
    if (!json)
        return default_val;

    res = json_get_double(json, path, default_val);
    cJSON_Delete(json);
    return res;
}
//------------------------------------------------------------------------------
