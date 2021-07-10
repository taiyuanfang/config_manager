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

#ifndef JPATH_H
#define JPATH_H
//------------------------------------------------------------------------------
struct cJSON;
//------------------------------------------------------------------------------
struct cJSON * json_loads(const char *str);
struct cJSON * json_loadf(const char *filename);
struct cJSON * json_get(struct cJSON *json, const char *path, int auto_create);

int json_get_str(struct cJSON *json, const char *path, char *buff, int size);
int json_get_int(struct cJSON *json, const char *path, const int default_val);
double json_get_double(struct cJSON *json, const char *path, const double default_val);

int json_set_str(struct cJSON *json, const char *path, const char *val);
int json_set_int(struct cJSON *json, const char *path, const int val);
int json_set_double(struct cJSON *json, const char *path, const double val);
//------------------------------------------------------------------------------
int jsonf_get_str(const char *filename, const char *path, char *result, int size);
int jsonf_get_int(const char *filename, const char *path, const int default_val);
double jsonf_get_double(const char *filename, const char *path, const double default_val);

int jsonf_set_str(const char *filename, const char *path, const char *val);
int jsonf_set_int(const char *filename, const char *path, const int val);
int jsonf_set_double(const char *filename, const char *path, const double val);

int jsons_get_str(const char *json_str, const char *path, char *result, int size);
int jsons_get_int(const char *json_str, const char *path, const int default_val);
double jsons_get_double(const char *json_str, const char *path, const double default_val);
//------------------------------------------------------------------------------
#endif
