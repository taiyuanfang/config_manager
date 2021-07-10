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

#define DEBUG_MODULE "config_manager"
#define DEBUG_LEVEL 5

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <fcntl.h>
#include "private.h"
//------------------------------------------------------------------------------
int uds_open_tcp(int nonblock) {
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0)
        return -1;

    if (nonblock)
        fcntl(fd, F_SETFL, O_NONBLOCK);

    return fd;
}
//------------------------------------------------------------------------------
int uds_bind(int fd, const char *uds_path) {
    struct sockaddr_un un;
    const socklen_t slen = sizeof(struct sockaddr_un);

    memset(&un, 0, slen);
    un.sun_family = AF_UNIX;
    strncpy(un.sun_path, uds_path, sizeof(un.sun_path) - 1);

    if (bind(fd, (struct sockaddr *) &un, slen) < 0) {
        LOGE("bind err: %d\n", errno);
        return -1;
    }

    return 0;
}
//------------------------------------------------------------------------------
int uds_listen(int fd, int max_conn) {
    if (listen(fd, max_conn) < 0)
        return -1;

    return 0;
}
//------------------------------------------------------------------------------
int uds_connect(int fd, const char *uds_path) {
    struct sockaddr_un un;
    const socklen_t slen = sizeof(struct sockaddr_un);

    memset(&un, 0, slen);
    un.sun_family = AF_UNIX;
    strncpy(un.sun_path, uds_path, sizeof(un.sun_path) - 1);

    if (connect(fd, (struct sockaddr *) &un, slen) < 0)
        return -1;

    return 0;
}
//------------------------------------------------------------------------------
int uds_send(int fd, const void *msg, int len) {
    return send(fd, msg, len, MSG_NOSIGNAL);
}
//------------------------------------------------------------------------------
int uds_recv(int fd, void *buf, int len) {
    return recv(fd, buf, len, 0 /* MSG_WAITALL */);
}
//------------------------------------------------------------------------------
static int create_connection(void) {
    int fd = uds_open_tcp(0);
    if (fd < 0) {
        LOGE("uds_open_tcp err\n");
        return -1;
    }

    if (uds_connect(fd, UDS_SOCKET_PATH) < 0) {
        LOGE("uds_connect err\n");
        close(fd);
        return -1;
    }

    return fd;
}
//------------------------------------------------------------------------------
static int send_req(int fd, cm_req_t *req, cm_res_t *res) {
    int ret = -1;
    struct timeval timeout;
    fd_set fds;

    ret = uds_send(fd, req, sizeof(cm_req_t));
    if (sizeof(cm_req_t) != ret)
        return -1;

    res->err = -1;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    while (timeout.tv_sec > 0 || timeout.tv_usec > 0) {
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        ret = select(fd + 1, &fds, NULL, NULL, &timeout);

        if (0 == ret) // timeout
            return -1;

        if (ret < 0) // error
            return -1;

        if (FD_ISSET(fd, &fds))
            break;
    }

    ret = uds_recv(fd, res, sizeof(cm_res_t));
    if (sizeof(cm_res_t) != ret) // error
        return -1;

    return res->err;
}
//------------------------------------------------------------------------------
int cmc_get(const char *filename, const char *xpath, char *str, const char *default_value) {
    int fd = -1;
    int ret = 0;
    cm_req_t req;
    cm_res_t res;

    if (!filename || !xpath || !str)
        return -1;

    fd = create_connection();
    if (fd < 0)
        return -1;

    memset(&req, 0x0, sizeof(req));
    req.cmd = CM_REQ_GET;
    realpath(filename, req.filename);
    strcpy(req.xpath, xpath);

    if (default_value && default_value[0])
        strcpy(req.value.s, default_value);

    if (0 == send_req(fd, &req, &res)) {
        strcpy(str, res.value.s);
    } else {
        if (default_value && default_value[0])
            strcpy(str, default_value);
    }

    close(fd);
    return ret;
}
//------------------------------------------------------------------------------
int cmc_get_int(const char *filename, const char *xpath, int default_value) {
    int ret = -1;
    char tmp[16] = {0};
    char default_value_str[16] = {0};

    sprintf(default_value_str, "%d", default_value);
    ret = cmc_get(filename, xpath, tmp, default_value_str);
    if ((ret != 0) || (0 == tmp[0])) {
        sprintf(tmp, "%d", default_value);
    }

    return (int)strtol(tmp, NULL, 0);
}
//------------------------------------------------------------------------------
int cmc_get_hex(const char *filename, const char *xpath, int default_value) {
    int ret = -1;
    char tmp[16] = {0};
    char default_value_str[16] = {0};

    sprintf(default_value_str, "0x%08X", default_value);
    ret = cmc_get(filename, xpath, tmp, default_value_str);
    if ((ret != 0) || (0 == tmp[0])) {
        sprintf(tmp, "0x%08X", default_value);
    }

    return (int)strtol(tmp, NULL, 16);
}
//------------------------------------------------------------------------------
int cmc_set(const char *filename, const char *xpath, const char *str) {
    int fd = -1;
    int ret = 0;
    cm_req_t req;
    cm_res_t res;

    if (!filename || !xpath)
        return -1;

    fd = create_connection();
    if (fd < 0)
        return -1;

    memset(&req, 0x0, sizeof(req));
    req.cmd = CM_REQ_SET;
    realpath(filename, req.filename);
    strcpy(req.xpath, xpath);
    if (str && str[0])
        strcpy(req.value.s, str);

    ret = send_req(fd, &req, &res);
    close(fd);
    return ret;
}
//------------------------------------------------------------------------------
int cmc_set_int(const char *filename, const char *xpath, const int val) {
    int fd = -1;
    int ret = 0;
    cm_req_t req;
    cm_res_t res;

    if (!filename || !xpath)
        return -1;

    fd = create_connection();
    if (fd < 0)
        return -1;

    memset(&req, 0x0, sizeof(req));
    req.cmd = CM_REQ_SET_INT;
    //strcpy(req.filename, filename);
    realpath(filename, req.filename);
    strcpy(req.xpath, xpath);
    req.value.i = val;
    ret = send_req(fd, &req, &res);
    close(fd);
    return ret;
}
//------------------------------------------------------------------------------
int cmc_ping(void) {
    int fd = -1;
    int ret = 0;
    cm_req_t req;
    cm_res_t res;

    fd = create_connection();
    if (fd < 0)
        return -1;

    req.cmd = CM_REQ_PING;

    ret = send_req(fd, &req, &res);
    if (0 == ret) {
        if (0 != strcmp(res.value.s, "PONG"))
            ret = -1;
    }

    close(fd);
    return ret;
}
//------------------------------------------------------------------------------
int cmc_reload(const char *filename) {
    int fd = -1;
    int ret = 0;
    cm_req_t req;
    cm_res_t res;

    fd = create_connection();
    if (fd < 0)
        return -1;

    memset(&req, 0x0, sizeof(req));
    req.cmd = CM_REQ_RELOAD;
    if (filename)
        strcpy(req.filename, filename);

    ret = send_req(fd, &req, &res);
    close(fd);
    return ret;
}
//------------------------------------------------------------------------------
int cmc_close(const char *filename) {
    int fd = -1;
    int ret = 0;
    cm_req_t req;
    cm_res_t res;

    fd = create_connection();
    if (fd < 0)
        return -1;

    memset(&req, 0x0, sizeof(req));
    req.cmd = CM_REQ_CLOSE;
    if (filename)
        strcpy(req.filename, filename);

    ret = send_req(fd, &req, &res);
    close(fd);
    return ret;
}
//------------------------------------------------------------------------------
int cmc_save(const char *filename) {
    int fd = -1;
    int ret = 0;
    cm_req_t req;
    cm_res_t res;

    fd = create_connection();
    if (fd < 0)
        return -1;

    memset(&req, 0x0, sizeof(req));
    req.cmd = CM_REQ_SAVE;
    if (filename)
        strcpy(req.filename, filename);

    ret = send_req(fd, &req, &res);
    close(fd);
    return ret;
}
//------------------------------------------------------------------------------
