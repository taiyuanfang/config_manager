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

#define DEBUG_MODULE "cms::config_json"
#define DEBUG_LEVEL 5

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cJSON.h>
#include <jpath.h>
#include "config_file.h"
//------------------------------------------------------------------------------
typedef struct {
    config_file_t base;

    cJSON* json;

} config_json_t;
//------------------------------------------------------------------------------
static void conf_json_dtor(void *arg) {
    config_json_t *cf = arg;

    if (cf->json) {
        cJSON_Delete(cf->json);
        cf->json = NULL;
    }
}
//------------------------------------------------------------------------------
static int conf_json_ctor(void *self) {
    config_json_t *cf = self;

    cf->json = json_loadf(cf->base.filename);

    return (cf->json ? 0 : -1);
}
//------------------------------------------------------------------------------
static int conf_json_save(void *self) {
    config_json_t *cf = self;
    int ret = -1;
    char *doc = NULL;
    FILE *fp = NULL;

    if (cf->json)
        doc = cJSON_Print(cf->json);

    if (doc) {
        fp = fopen(cf->base.filename, "w");
        if (fp) {
            fwrite(doc, 1, strlen(doc), fp);
            fclose(fp);
            ret = 0;
        }
        free(doc);
    }

    return ret;
}
//------------------------------------------------------------------------------
static int conf_json_get_s(void *self, const char *xpath, char *val, int len) {
    config_json_t *cf = self;

    val[0] = 0;

    if (!cf->json)
        return -1;

    if (json_get_str(cf->json, xpath, val, len) > 0)
        val[len - 1] = 0;

    return 0;
}
//------------------------------------------------------------------------------
static int conf_json_set_s(void *self, const char *xpath, const char *val, int len) {
    config_json_t *cf = self;

    if (!cf->json)
        return -1;

    if (0 == json_set_str(cf->json, xpath, val))
        return 0;

    return -1;
}
//------------------------------------------------------------------------------
static int conf_json_get_i(void *self, const char *xpath, int *val) {
    config_json_t *cf = self;

    if (!cf->json)
        return -1;

    *val = json_get_int(cf->json, xpath, 0);

    return 0;
}
//------------------------------------------------------------------------------
static int conf_json_set_i(void *self, const char *xpath, int val) {
    config_json_t *cf = self;

    if (!cf->json)
        return -1;

    if (0 == json_set_int(cf->json, xpath, val))
        return 0;

    return -1;
}
//------------------------------------------------------------------------------
static int conf_json_get_f(void *self, const char *xpath, double *val) {
    config_json_t *cf = self;

    if (!cf->json)
        return -1;

    *val = json_get_double(cf->json, xpath, 0);

    return 0;
}
//------------------------------------------------------------------------------
static int conf_json_set_f(void *self, const char *xpath, double val) {
    config_json_t *cf = self;

    if (!cf->json)
        return -1;

    if (0 == json_set_double(cf->json, xpath, val))
        return 0;

    return -1;
}
//------------------------------------------------------------------------------
config_file_t *
config_json_create(const char * filename) {
    config_file_t* cf = NULL;

    cf = malloc(sizeof(config_json_t));
    if (!cf)
        return NULL;

    memset(cf, 0x0, sizeof(config_json_t));
    strcpy(cf->filename, filename);
    cf->ctor = conf_json_ctor;
    cf->dtor = conf_json_dtor;
    cf->save = conf_json_save;
    cf->get_s = conf_json_get_s;
    cf->set_s = conf_json_set_s;
    cf->get_i = conf_json_get_i;
    cf->set_i = conf_json_set_i;
    cf->get_f = conf_json_get_f;
    cf->set_f = conf_json_set_f;
    return cf;
}
//------------------------------------------------------------------------------
