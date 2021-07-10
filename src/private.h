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

#ifndef CONFIG_MANAGER_PRIVATE_H
#define CONFIG_MANAGER_PRIVATE_H
//------------------------------------------------------------------------------
#include "config_manager.h"
//------------------------------------------------------------------------------
typedef enum {
    CM_REQ_NONE = 0,
    CM_REQ_GET,
    CM_REQ_SET,
    CM_REQ_GET_INT,
    CM_REQ_SET_INT,
    CM_REQ_GET_DOUBLE,
    CM_REQ_SET_DOUBLE,
    CM_REQ_PING,
    CM_REQ_RELOAD,
    CM_REQ_CLOSE,
    CM_REQ_SAVE,

    CM_REQ_LAST
} CM_REQ_CMD;
//------------------------------------------------------------------------------
typedef struct {
    int cmd;
    char filename[128];
    char xpath[128];
    union {
        int i;
        double d;
        char s[CM_VAL_BUF_LEN];
    } value;
} cm_req_t;
//------------------------------------------------------------------------------
typedef struct {
    int err;
    union {
        int i;
        double d;
        char s[CM_VAL_BUF_LEN];
    } value;
} cm_res_t;
//------------------------------------------------------------------------------
#define UDS_SOCKET_PATH "/tmp/config_manager.sock"
//------------------------------------------------------------------------------
int uds_open_tcp(int nonblock);
int uds_listen(int fd, int max_conn);
int uds_connect(int fd, const char *uds_path);
int uds_send(int fd, const void *msg, int len);
int uds_recv(int fd, void *buf, int len);
//------------------------------------------------------------------------------
#define LOG(fmt, ...)      fprintf(stdout, fmt, ##__VA_ARGS__)

#if (DEBUG_LEVEL >= 4)
    #define LOGE(fmt, ...) LOG(DEBUG_MODULE " | " fmt,  ##__VA_ARGS__)
#else
    #define LOGE(...)
#endif

#if (DEBUG_LEVEL >= 5)
    #define LOGI(fmt, ...) LOG(DEBUG_MODULE " | " fmt,  ##__VA_ARGS__)
#else
    #define LOGI(...)
#endif

#if (DEBUG_LEVEL >= 6)
    #define LOGD(fmt, ...) LOG(DEBUG_MODULE " | " fmt,  ##__VA_ARGS__)
#else
    #define LOGD(...)
#endif

#if (DEBUG_LEVEL >= 7)
    #define LOGT(fmt, ...) LOG(DEBUG_MODULE " | " fmt,  ##__VA_ARGS__)
#else
    #define LOGT(...)
#endif
//------------------------------------------------------------------------------
#endif
