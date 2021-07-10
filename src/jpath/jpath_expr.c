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


#define DEBUG_MODULE "jpath_expr"
#define DEBUG_LEVEL 5

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "jpath_expr.h"
//------------------------------------------------------------------------------
jpath_expr_p jpath_expr_parse(const char *str) {
    jpath_expr_p expr = NULL;
    jpath_expr_p curr = NULL;
    jpath_expr_p prev = NULL;
    int str_len = 0;
    int buf_len = 0;
    int ret = 0;
    int i = 0;
    char buf[128] = {0};

    if (!str || !(*str))
        return NULL;

    str_len = strlen(str);

    for (i = 0; i < str_len; ++i) {
        if ('/' == str[i]) {
            if (1 == sscanf(str + i, "/%[^/\\[]", buf)) {
                buf_len = strlen(buf);
                curr = malloc(sizeof(jpath_expr_t));
                memset(curr, 0x0, sizeof(jpath_expr_t));
                curr->name = malloc(buf_len + 1);
                memcpy(curr->name, buf, buf_len + 1);

                if (!expr)
                    expr = curr;
                if (prev)
                    prev->next = curr;
                prev = curr;

                i += buf_len;
                ++ret;
                continue;
            }
            ret = 0;
            break;
        }

        if ('[' == str[i]) {
            if (1 == sscanf(str + i, "[%[\\-0-9]]", buf)) {
                buf_len = strlen(buf);
                curr = malloc(sizeof(jpath_expr_t));
                memset(curr, 0x0, sizeof(jpath_expr_t));
                curr->index = (int)strtol(buf, NULL, 0);

                if (!expr)
                    expr = curr;
                if (prev)
                    prev->next = curr;
                prev = curr;

                i += (buf_len + 1);
                ++ret;
                continue;
            }
            ret = 0;
            break;
        }
    }

    return expr;
}
//------------------------------------------------------------------------------
void jpath_expr_free(jpath_expr_p expr) {
    jpath_expr_p curr = expr;
    jpath_expr_p next = NULL;

    while (curr) {
        next = curr->next;
        if (curr->name)
            free(curr->name);
        free(curr);
        curr = next;
    }
}
//------------------------------------------------------------------------------
