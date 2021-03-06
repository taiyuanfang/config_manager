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

#include <signal.h>
#include "socket_handler.h"
//------------------------------------------------------------------------------
static int quit = 0;
//------------------------------------------------------------------------------
static void sigpipe_handler(int sig) {
}
//------------------------------------------------------------------------------
static void signal_handler(int sig) {
    quit = 1;
}
//------------------------------------------------------------------------------
int main(int argc, char *argv[]) {
    signal(SIGINT, signal_handler);
    signal(SIGALRM, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGPIPE, sigpipe_handler);

    server_start();

    while (!quit) {
        server_poll();
    }

    server_stop();

    return 0;
}
//------------------------------------------------------------------------------
