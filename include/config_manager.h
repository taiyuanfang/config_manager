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

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H
//------------------------------------------------------------------------------
#define CM_VAL_BUF_LEN 256
//------------------------------------------------------------------------------
int uds_open_tcp(int nonblock);
int uds_listen(int fd, int max_conn);
int uds_connect(int fd, const char *uds_path);
int uds_send(int fd, const void *msg, int len);
int uds_recv(int fd, void *buf, int len);
//------------------------------------------------------------------------------
int cmc_set(const char *filename, const char *xpath, const char *str);
int cmc_set_int(const char *filename, const char *xpath, const int val);
int cmc_get(const char *filename, const char *xpath, char *str, const char *default_value);
int cmc_get_int(const char *filename, const char *xpath, int default_value);
int cmc_get_hex(const char *filename, const char *xpath, int default_value);
int cmc_ping(void);
int cmc_reload(const char *filename);
int cmc_close(const char *filename);
int cmc_save(const char *filename);
//------------------------------------------------------------------------------
#endif