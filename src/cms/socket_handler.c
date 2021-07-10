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

#define DEBUG_MODULE "cms::socket_handler"
#define DEBUG_LEVEL 5

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <cJSON.h>
#include <jpath.h>
#include "private.h"
#include "config_file.h"
#include "socket_handler.h"
//------------------------------------------------------------------------------
#define MAX_EVENTS   64
#define MAX_CONFIG   64
#define TIMEOUT_MSEC 100

#define EVENT_ERR    (EPOLLRDHUP|EPOLLERR|EPOLLHUP)
//------------------------------------------------------------------------------
typedef struct {
    config_file_t *cf;
} config_t;

typedef struct {
    config_t config[MAX_CONFIG];

    int fd;
    int epfd;
    struct epoll_event events[MAX_EVENTS];

} server_t;
//------------------------------------------------------------------------------
static server_t *server = NULL;
//------------------------------------------------------------------------------
static int ep_init(void) {
    if (!server)
        return -1;

    server->epfd = epoll_create(MAX_EVENTS);

    return (server->epfd < 0 ? -1 : 0);
}
//------------------------------------------------------------------------------
static void ep_destroy(void) {
    if (!server || (server->epfd < 0))
        return;

    close(server->epfd);
    server->epfd = -1;
}
//------------------------------------------------------------------------------
static int ep_add(int fd) {
    if (!server || (server->epfd < 0) || (fd < 0))
        return -1;

    struct epoll_event event;
    memset(&event, 0x0, sizeof(struct epoll_event));
    event.data.fd = fd;
    event.events = (EPOLLIN | EVENT_ERR);

    return (epoll_ctl(server->epfd, EPOLL_CTL_ADD, fd, &event) < 0 ? -1 : 0);
}
//------------------------------------------------------------------------------
static int ep_remove(int fd) {
    if (!server || (server->epfd < 0) || (fd < 0))
        return -1;

    epoll_ctl(server->epfd, EPOLL_CTL_DEL, fd, NULL);

    return 0;
}
//------------------------------------------------------------------------------
static int get_realpath(const char *filename, char *rpath) {
    LOGT("filename=%s\n", filename);
    if (0 == access(filename, F_OK)) {
        realpath(filename, rpath);
        LOGT("rpath=%s\n", rpath);
        return 0;
    }
    LOGT("access(%s, F_OK) != 0\n", filename);
    return -1;
}
//------------------------------------------------------------------------------
config_file_t * find_config_file(const char *filename, int auto_create) {
    config_file_t *cf = NULL;
    int i = 0;
    char rpath[128] = {0};

    if (get_realpath(filename, rpath) < 0)
        return NULL;

    for (i = 0; i < MAX_CONFIG; ++i) {
        cf = server->config[i].cf;
        if (cf && !strcmp(cf->filename, rpath))
            return cf;
    }

    if (auto_create) {
        for (i = 0; i < MAX_CONFIG; ++i) {
            if (server->config[i].cf)
                continue;

            return (server->config[i].cf = config_file_create(rpath));
        }
    }

    return NULL;
}
//------------------------------------------------------------------------------
static void req_close(cm_req_t *req, cm_res_t *res) {
    config_file_t *cf = NULL;
    int i = 0;

    res->err = 0;

    cf = find_config_file(req->filename, 0);
    if (cf) {
        for (i = 0; i < MAX_CONFIG; ++i) {
            if (cf == server->config[i].cf) {
                server->config[i].cf = NULL;
                config_file_destroy(cf);
                return;
            }
        }
    }
}
//------------------------------------------------------------------------------
static void req_save(cm_req_t *req, cm_res_t *res) {
    config_file_t *cf = NULL;
    int i = 0;

    res->err = 0;

    cf = find_config_file(req->filename, 0);
    if (cf) {
        for (i = 0; i < MAX_CONFIG; ++i) {
            if (cf == server->config[i].cf) {
                config_file_save(cf);
                return;
            }
        }
    }
}
//------------------------------------------------------------------------------
static void req_get_s(cm_req_t *req, cm_res_t *res) {
    LOGT("%s:%d | xpath=%s\n", __func__, __LINE__, req->xpath);

    res->err = config_file_get_s(find_config_file(req->filename, 1),
                                 req->xpath,
                                 res->value.s,
                                 CM_VAL_BUF_LEN);
}
//------------------------------------------------------------------------------
static void req_set_s(cm_req_t *req, cm_res_t *res) {
    LOGT("%s:%d | xpath=%s\n", __func__, __LINE__, req->xpath);

    res->err = config_file_set_s(find_config_file(req->filename, 1),
                                 req->xpath,
                                 req->value.s,
                                 CM_VAL_BUF_LEN);
}
//------------------------------------------------------------------------------
static void req_get_i(cm_req_t *req, cm_res_t *res) {
    res->err = config_file_get_i(find_config_file(req->filename, 1),
                                 req->xpath,
                                 &res->value.i);
}
//------------------------------------------------------------------------------
static void req_set_i(cm_req_t *req, cm_res_t *res) {
    res->err = config_file_set_i(find_config_file(req->filename, 1),
                                 req->xpath,
                                 req->value.i);
}
//------------------------------------------------------------------------------
static void req_get_f(cm_req_t *req, cm_res_t *res) {
    res->err = config_file_get_f(find_config_file(req->filename, 1),
                                 req->xpath,
                                 &res->value.d);
}
//------------------------------------------------------------------------------
static void req_set_f(cm_req_t *req, cm_res_t *res) {
    res->err = config_file_set_f(find_config_file(req->filename, 1),
                                 req->xpath,
                                 req->value.d);
}
//------------------------------------------------------------------------------
static int read_req(int fd) {
    int ret = 0;
    cm_req_t req;
    cm_res_t res;

    memset(&req, 0x0, sizeof(req));
    memset(&res, 0x0, sizeof(res));

    ret = uds_recv(fd, &req, sizeof(req));
    if (ret < 0)
        return -1;

    switch (req.cmd) {
        case CM_REQ_GET:
            LOGT("CM_REQ_GET\n");
            req_get_s(&req, &res);
            break;
        case CM_REQ_SET:
            LOGT("CM_REQ_SET\n");
            req_set_s(&req, &res);
            break;
        case CM_REQ_GET_INT:
            LOGT("CM_REQ_GET_INT\n");
            req_get_i(&req, &res);
            break;
        case CM_REQ_SET_INT:
            LOGT("CM_REQ_SET_INT\n");
            req_set_i(&req, &res);
            break;
        case CM_REQ_GET_DOUBLE:
            LOGT("CM_REQ_GET_DOUBLE\n");
            req_get_f(&req, &res);
            break;
        case CM_REQ_SET_DOUBLE:
            LOGT("CM_REQ_SET_DOUBLE\n");
            req_set_f(&req, &res);
            break;
        case CM_REQ_PING:
            LOGT("CM_REQ_PING\n");
            strcpy(res.value.s, "PONG");
            res.err = 0;
            break;
        case CM_REQ_RELOAD:
            LOGT("CM_REQ_RELOAD\n");
            req_close(&req, &res);
            break;
        case CM_REQ_CLOSE:
            LOGT("CM_REQ_CLOSE\n");
            req_close(&req, &res);
            break;
        case CM_REQ_SAVE:
            LOGT("CM_REQ_SAVE\n");
            req_save(&req, &res);
            break;
        default:
            res.err = 1;
            break;
    }

    LOGT("send resp\n");
	ret = uds_send(fd, &res, sizeof(res));
    return (ret < 0 ? -1 : 0);
}
//------------------------------------------------------------------------------
static int on_data(int fd, int err, void *arg) {
    if (err) {
        close(fd);
        return -1;
    }

    if (read_req(fd) < 0) {
        close(fd);
        return -1;
    }

    return 0;
}
//------------------------------------------------------------------------------
static int on_listen(int fd, int err, void *arg) {
    int client_fd = accept(fd, NULL, NULL);
    if (client_fd < 0)
        return 0;

    ep_add(client_fd);
    return 0;
}
//------------------------------------------------------------------------------
static int ep_poll(void) {
    int i, n, fd, err;

    if (!server || (server->epfd < 0))
        return -1;

    memset(server->events, 0x0, MAX_EVENTS * sizeof(struct epoll_event));
    n = epoll_wait(server->epfd, server->events, MAX_EVENTS, TIMEOUT_MSEC);

    for (i = n - 1; i >= 0; --i) {
        fd = server->events[i].data.fd;
        err = (EVENT_ERR & server->events[i].events);

        if (fd == server->fd)
            err = on_listen(fd, err, server);
        else
            err = on_data(fd, err, NULL);

        if (err)
            ep_remove(fd);
    }

    return 0;
}
//------------------------------------------------------------------------------
static void server_dtor(void *arg) {
    if (server->fd >= 0) {
        ep_remove(server->fd);
        close(server->fd);
    }

    int i;
    for (i = 0; i < MAX_CONFIG; ++i) {
        if (server->config[i].cf) {
            config_file_destroy(server->config[i].cf);
            server->config[i].cf = NULL;
        }
    }

    ep_destroy();
    unlink(UDS_SOCKET_PATH);
}
//------------------------------------------------------------------------------
static int server_ctor(void) {
    if (ep_init() < 0)
        return -1;

    server->fd = uds_open_tcp(1);
    if (server->fd < 0)
        return -1;

    if (uds_bind(server->fd, UDS_SOCKET_PATH) < 0)
        return -1;

    if (uds_listen(server->fd, MAX_EVENTS) < 0)
        return -1;

    if (ep_add(server->fd) < 0)
        return -1;

    return 0;
}
//------------------------------------------------------------------------------
void server_start(void) {
    if (server)
        return;

    unlink(UDS_SOCKET_PATH);

    server = malloc(sizeof(server_t));
    if (!server)
        return;

    memset(server, 0x0, sizeof(server_t));
    server->fd = -1;

    if (server_ctor() < 0)
        server_stop();
}
//------------------------------------------------------------------------------
void server_stop(void) {
    if (!server)
        return;

    server_dtor(server);
    free(server);
    server = NULL;
}
//------------------------------------------------------------------------------
void server_poll(void) {
    if (!server || (server->fd < 0))
        return;

    ep_poll();
}
//------------------------------------------------------------------------------
