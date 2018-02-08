#pragma once
#ifdef WIN32
#include <WinSock2.h>
#include <MSWSock.h>
#include <process.h>
#include <WinDef.h>
#include <stddef.h>

#ifdef  __cplusplus
extern "C" {
#endif

#define ERROR_CONNECT_FAIL  -5
#define ERROR_SYSTEM        -4
#define ERROR_SEND_OVERFLOW -3
#define ERROR_RECV_OVERFLOW -2
#define ERROR_PACKET        -1
#define ERROR_NONE          0

typedef struct iocp_tcp_socket* HSESSION;
typedef struct iocp_tcp_listener* HLISTENER;
typedef struct iocp_tcp_manager* HNETMANAGER;

typedef int (*pfn_parse_packet)(HSESSION session, const char* data, const int len);

typedef void (*pfn_on_establish)(HLISTENER net_handle, HSESSION session);
typedef void (*pfn_on_terminate)(HSESSION session);
typedef void (*pfn_on_error)(HSESSION session, int module_error, int system_error);
typedef void (*pfn_on_recv)(HSESSION session, const char* data, const int len);

extern HNETMANAGER (create_iocp_tcp)(pfn_on_establish func_on_establish, 
                                    pfn_on_terminate func_on_terminate, 
                                    pfn_on_error func_on_error, 
                                    pfn_on_recv func_onrecv,
                                    int max_socket_num, 
                                    int max_io_thread_num, 
                                    int max_accept_ex_num);

extern void (destroy_iocp_tcp)(HNETMANAGER mgr);

extern HSESSION (iocp_tcp_connect)(HNETMANAGER mgr,
                                 const char* ip, unsigned short port, 
                                 int recv_buf_size, int send_buf_size, 
                                 pfn_parse_packet func, bool reuse_addr, 
                                 const char* bind_ip, unsigned short bind_port);

extern HLISTENER (iocp_tcp_listen)(HNETMANAGER mgr,
                                 const char* ip, unsigned short port, 
                                 int recv_buf_size, int send_buf_size, 
                                 pfn_parse_packet func, bool reuse_addr);

extern bool (iocp_tcp_send)(HSESSION socket, const char* data, size_t len);

extern void (iocp_tcp_close_session)(HSESSION socket);

extern void (iocp_tcp_close_listener)(HLISTENER listener);

extern bool (iocp_tcp_run)(HNETMANAGER mgr, unsigned int run_time);

extern SOCKET (iocp_tcp_session_socket)(HSESSION socket);

extern SOCKET (iocp_tcp_listener_socket)(HLISTENER listener);

extern void (iocp_tcp_set_listener_data)(HLISTENER listener, void* user_data);

extern void (iocp_tcp_set_session_data)(HSESSION socket, void* user_data);

extern void* (iocp_tcp_get_listener_data)(HLISTENER listener);

extern void* (iocp_tcp_get_session_data)(HSESSION socket);

extern unsigned int (iocp_tcp_get_peer_ip)(HSESSION socket);

extern unsigned short (iocp_tcp_get_peer_port)(HSESSION socket);

extern unsigned int (iocp_tcp_get_local_ip)(HSESSION socket);

extern unsigned short (iocp_tcp_get_local_port)(HSESSION socket);

extern size_t (iocp_tcp_get_send_free_size)(HSESSION socket);

extern bool (iocp_tcp_set_send_control)(HSESSION socket, int pkg_size, int delay_time);

#ifdef  __cplusplus
}
#endif

#endif
