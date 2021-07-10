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

#define DEBUG_MODULE "cms::config_file"
#define DEBUG_LEVEL 5

//#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "private.h"
#include "config_file.h"
//------------------------------------------------------------------------------
config_file_t *
config_file_create(const char * filename) {
    config_file_t* cf = NULL;
    const char *ext = config_file_ext(filename);

    LOGT("%s:%d | filename=%s, ext=%s\n", __func__, __LINE__, filename, ext);

    if (ext && (0 == strncmp(ext, ".json", 5))) {
        cf = config_json_create(filename);
    } else {
        //cf = config_xml_create(filename);
    }

    if (cf) {
        if (0 != cf->ctor(cf)) {
            config_file_destroy(cf);
            cf = NULL;
        }
    }
    return cf;
}
//------------------------------------------------------------------------------
void
config_file_destroy(config_file_t *cf) {
    if (cf && cf->dtor) {
        if (cf->dtor)
            cf->dtor(cf);
        free(cf);
    }
}
//------------------------------------------------------------------------------
const char *
config_file_ext(const char * filename) {
    int i, len;

    if (!filename)
        return NULL;

    len = strlen(filename);

    for (i = len - 1; i >= 0; --i) {
        if ('.' == filename[i])
            return (filename + i);
    }

    return NULL;
}
//------------------------------------------------------------------------------
int
config_file_save(config_file_t *cf) {
    if (!cf || !cf->save)
        return -1;

    return cf->save(cf);
}
//------------------------------------------------------------------------------
int
config_file_get_s(config_file_t *cf, const char *xpath, char *val, int len) {
    if (!cf || !cf->get_s)
        return -1;

    LOGT("%s | xpath=%s\n", __func__, xpath);

    return cf->get_s(cf, xpath, val, len);
}
//------------------------------------------------------------------------------
int
config_file_set_s(config_file_t *cf, const char *xpath, const char *val, int len) {
    if (!cf || !cf->set_s)
        return -1;

    return cf->set_s(cf, xpath, val, len);
}
//------------------------------------------------------------------------------
int
config_file_get_i(config_file_t *cf, const char *xpath, int *val) {
    if (!cf || !cf->get_i)
        return -1;

    return cf->get_i(cf, xpath, val);
}
//------------------------------------------------------------------------------
int
config_file_set_i(config_file_t *cf, const char *xpath, int val) {
    if (!cf || !cf->set_i)
        return -1;

    return cf->set_i(cf, xpath, val);
}
//------------------------------------------------------------------------------
int
config_file_get_f(config_file_t *cf, const char *xpath, double *val) {
    if (!cf || !cf->get_f)
        return -1;

    return cf->get_f(cf, xpath, val);
}
//------------------------------------------------------------------------------
int
config_file_set_f(config_file_t *cf, const char *xpath, double val) {
    if (!cf || !cf->set_f)
        return -1;

    return cf->set_f(cf, xpath, val);
}
//------------------------------------------------------------------------------
