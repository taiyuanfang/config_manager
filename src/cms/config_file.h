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

#ifndef CMS_CONFIG_FILE_H
#define CMS_CONFIG_FILE_H
//------------------------------------------------------------------------------
typedef int  (conf_ctor_h)(void *self);
typedef void (conf_dtor_h)(void *self);

typedef int (conf_open_h)(void *self);
typedef int (conf_save_h)(void *self);

typedef int (conf_get_s_h)(void *self, const char *xpath, char *val, int len);
typedef int (conf_set_s_h)(void *self, const char *xpath, const char *val, int len);

typedef int (conf_get_i_h)(void *self, const char *xpath, int *val);
typedef int (conf_set_i_h)(void *self, const char *xpath, int val);

typedef int (conf_get_f_h)(void *self, const char *xpath, double *val);
typedef int (conf_set_f_h)(void *self, const char *xpath, double val);
//------------------------------------------------------------------------------
typedef struct {
    conf_ctor_h *ctor;
    conf_dtor_h *dtor;

    conf_save_h *save;

    conf_get_s_h *get_s;
    conf_set_s_h *set_s;

    conf_get_i_h *get_i;
    conf_set_i_h *set_i;

    conf_get_f_h *get_f;
    conf_set_f_h *set_f;

    char filename[128];
}  config_file_t;
//------------------------------------------------------------------------------
config_file_t *
config_file_create(const char * filename);

void
config_file_destroy(config_file_t *cf);

const char *
config_file_ext(const char * filename);

int
config_file_save(config_file_t *cf);

int
config_file_get_s(config_file_t *cf, const char *xpath, char *val, int len);

int
config_file_set_s(config_file_t *cf, const char *xpath, const char *val, int len);

int
config_file_get_i(config_file_t *cf, const char *xpath, int *val);

int
config_file_set_i(config_file_t *cf, const char *xpath, int val);

int
config_file_get_f(config_file_t *cf, const char *xpath, double *val);

int
config_file_set_f(config_file_t *cf, const char *xpath, double val);
//------------------------------------------------------------------------------
config_file_t *
config_xml_create(const char * filename);

config_file_t *
config_json_create(const char * filename);
//------------------------------------------------------------------------------
#endif
