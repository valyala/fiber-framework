#ifndef FF_WIN_NET_H
#define FF_WIN_NET_H

#include "ff_win_stdafx.h"
#include "private/arch/ff_arch_completion_port.h"

#ifdef __cplusplus
extern "C" {
#endif

void ff_win_net_initialize(struct ff_arch_completion_port *completion_port);

void ff_win_net_shutdown();

void ff_win_net_register_socket(SOCKET socket);

int ff_win_net_complete_overlapped_io(SOCKET socket, WSAOVERLAPPED *overlapped);

enum ff_result ff_win_net_connect(SOCKET socket, const struct sockaddr_in *addr);

enum ff_result ff_win_net_accept(SOCKET listenSocket, SOCKET acceptSocket, struct sockaddr_in *acceptAddr);

#ifdef __cplusplus
}
#endif

#endif
