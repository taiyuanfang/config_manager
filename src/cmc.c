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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <config_manager.h>
//------------------------------------------------------------------------------
static void usage(void) {
    fprintf(stdout, "\n");
    fprintf(stdout, "cmc set [config file] [xpath] [value]\n");
    fprintf(stdout, "ex:   #./cmc set /mnt/mtd/config.xml /root/General/DefaultIP 192.168.1.26\n\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "cmc get [config file] [xpath]\n");
    fprintf(stdout, "ex:   #./cmc get /mnt/mtd/config.xml /root/General/DefaultIP\n");
    fprintf(stdout, "       192.168.126\n\n");
}
//------------------------------------------------------------------------------
int main(int argc, char*argv[]) {
    char *op = NULL;

    if (argc < 2) {
        usage();
        return 0;
    }

    op = argv[1];

    if (!strcmp(op, "ping")) {
        if(0 == cmc_ping()) {
            fprintf(stdout, "PONG\n");
        } else {
            fprintf(stdout, "ERROR\n");
        }
        return 0;
    }

    if (!strcmp(op, "reload")) {
        if (argc >= 2) {
            if (0 != cmc_reload(argc > 2 ? argv[2] : NULL)) {
                fprintf(stdout, "ERROR\n");
            }
            return 0;
        }
    }

    if (!strcmp(op, "close")) {
        if (argc >= 2) {
            cmc_close(argc > 2 ? argv[2] : NULL);
            return 0;
        }
    }

    if (!strcmp(op, "save")) {
        if (argc >= 2) {
            cmc_save(argc > 2 ? argv[2] : NULL);
            return 0;
        }
    }

    if (!strcmp(argv[1], "get")) {
        if (argc >= 4) {
            char res[512] = {0};
            if (0 == cmc_get(argv[2], argv[3], res, argc > 4 ? argv[4] : "")) {
                fprintf(stdout, "%s\n", res);
            }
            return 0;
        }
    }

    if (!strcmp(argv[1], "set")) {
        if (5 == argc) {
            if (0 != cmc_set(argv[2], argv[3], argv[4])) {
                fprintf(stdout, "ERROR\n");
            }
            return 0;
        }
    }

    usage();
    return 0;
}
//------------------------------------------------------------------------------
