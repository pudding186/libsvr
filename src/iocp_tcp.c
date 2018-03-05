#ifdef WIN32

#include <stdio.h>
#include "./data_def.h"
#include "../include/loop_cache.h"
#include "../include/timer.h"
#include "../include/memory_pool.h"
#include "../include/rb_tree.h"

#include "../include/iocp_tcp.h"

#define CRUSH_CODE char* p = 0;*p = 'a';

#define MAX_BACKLOG     256
#define MAX_ADDR_SIZE   64

//socket state
#define SOCKET_STATE_NONE       0
#define SOCKET_STATE_CONNECT    1
#define SOCKET_STATE_LISTEN     2
#define SOCKET_STATE_ESTABLISH  3
#define SOCKET_STATE_ERROR      4
#define SOCKET_STATE_TERMINATE  5
#define SOCKET_STATE_DELETE     6
#define SOCKET_STATE_ACCEPT     7

#define IOCP_OPT_RECV           0
#define IOCP_OPT_SEND           1
#define IOCP_OPT_ACCEPT         2
#define IOCP_OPT_CONNECT        3
#define IOCP_OPT_CONNECT_REQ    4
#define IOCP_OPT_ACCEPT_REQ     5

#define NET_EVENT_ESTABLISH     1
#define NET_EVENT_TERMINATE     2
#define NET_EVENT_DATA          3
#define NET_EVENT_SYSTEM_ERROR  4
#define NET_EVENT_MODULE_ERROR  5
#define NET_EVENT_RECV_ACTIVE   6
#define NET_EVENT_CONNECT_FAIL  7

#define DELAY_CLOSE_SOCKET      15
#define DELAY_SEND_CHECK        5

typedef struct st_event_establish 
{
    struct st_iocp_tcp_listener* listener;
}event_establish;

typedef struct st_evt_data
{
    int data_len;
}evt_data;

typedef struct st_event_system_error
{
    unsigned int err_code;
}event_system_error;

typedef struct st_event_module_error
{
    unsigned int err_code;
}event_module_error;

typedef struct st_event_connect_fail
{
    unsigned int err_code;
}event_connect_fail;

typedef struct st_event_terminate
{
    struct iocp_tcp_socket* socket;
}event_terminate;

#pragma warning( push )
#pragma warning( disable : 4201 )
typedef struct st_net_event
{
    struct st_iocp_tcp_socket*      socket;
    int                             type;
    union
    {
        struct st_event_establish       evt_establish;
        struct st_evt_data              evt_data;
        struct st_event_system_error    evt_system_error;
        struct st_event_module_error    evt_module_error;
        struct st_event_terminate       evt_terminate;
        struct st_event_connect_fail    evt_connect_fail;
    };
}net_event;

typedef struct st_iocp_data
{
    OVERLAPPED  over_lapped;
    WSABUF      wsa_buf;
    int         operation;
    union
    {
        struct st_iocp_tcp_listener*    listener;
        struct st_iocp_tcp_socket*      socket;
    };
}iocp_data;

#pragma warning( pop )

typedef struct st_iocp_listen_data
{
    struct st_iocp_data data;
    SOCKET              accept_socket;
}iocp_listen_data;

typedef struct st_iocp_tcp_listener
{
    SOCKET                      socket;
    int                         send_buf_size;
    int                         recv_buf_size;
    int                         max_accept_ex_num;
    pfn_parse_packet            pkg_parser;
    char*                       arry_addr_cache;
    struct st_iocp_listen_data* arry_iocp_data;
    LONG                        state;
    void*                       user_data;
    struct st_iocp_tcp_manager* mgr;
}iocp_tcp_listener;

typedef struct st_iocp_tcp_socket
{
    SOCKET                      socket;
    void*                       user_data;

    struct st_iocp_data            iocp_recv_data;
    struct st_iocp_data            iocp_send_data;

    HLOOPCACHE                  recv_loop_cache;
    HLOOPCACHE                  send_loop_cache;

    unsigned char               recv_req;
    unsigned char               recv_ack;
    LONG                        send_req;
    LONG                        send_ack;

    LONG                        state;

    unsigned int                data_need_send;
    unsigned int                data_has_send;
    //LONG                        data_to_send;
    LONG                        data_has_recv;

    LONG                        data_delay_send;
    LONG                        data_delay_send_size;

    unsigned int                local_ip;
    unsigned short              local_port;

    unsigned int                peer_ip;
    unsigned short              peer_port;

    HTIMERINFO                  timer_send;
    HTIMERINFO                  timer_close;

    pfn_parse_packet            pkg_parser;
    struct st_iocp_tcp_manager*    mgr;
}iocp_tcp_socket;

typedef struct st_iocp_tcp_manager
{
    pfn_on_establish            func_on_establish;
    pfn_on_terminate            func_on_terminate;
    pfn_on_error                func_on_error;
    pfn_on_recv                 func_on_recv;

    HANDLE                      iocp_port;

    int                         work_thread_num;
    HANDLE*                     work_threads;

    LPFN_CONNECTEX              func_connectex;
    LPFN_ACCEPTEX               func_acceptex;
    LPFN_GETACCEPTEXSOCKADDRS   func_getacceptexsockaddrs;

    CRITICAL_SECTION            evt_lock;
    CRITICAL_SECTION            socket_lock;

    HLOOPCACHE                  evt_queue;
    HTIMERMANAGER               timer_mgr;

    HMEMORYUNIT                 socket_pool;
    HRBTREE                     memory_mgr;

    int                         max_socket_num;
    int                         max_accept_ex_num;

    char*                       max_pkg_buf;
    int                         max_pkg_buf_size;
}iocp_tcp_manager;

/////////////////////////////////////////////////////////////////////////

void _iocp_tcp_socket_reset(struct st_iocp_tcp_socket* socket)
{
    socket->user_data = 0;
    socket->state = SOCKET_STATE_NONE;

    socket->recv_req = 0;
    socket->recv_ack = 0;

    socket->send_req = 0;
    socket->send_ack = 0;

    //socket->data_to_send = 0;
    socket->data_need_send = 0;
    socket->data_has_send = 0;

    socket->data_has_recv = 0;

    socket->data_delay_send = 0;
    socket->data_delay_send_size = 0;

    socket->local_ip = 0;
    socket->local_port = 0;

    socket->peer_ip = 0;
    socket->peer_port = 0;

    socket->timer_send = 0;
    socket->timer_close = 0;
}

#define SOCKET_LOCK EnterCriticalSection(&mgr->socket_lock)
#define SOCKET_UNLOCK LeaveCriticalSection(&mgr->socket_lock)

bool _iocp_tcp_socket_bind_iocp_port(HSESSION socket)
{
    if (CreateIoCompletionPort((HANDLE)socket->socket, socket->mgr->iocp_port, (ULONG_PTR)socket, 0))
    {
        return true;
    }

    return false;
}

void* _iocp_tcp_manager_alloc_memory(HNETMANAGER mgr, int buffer_size)
{
    HMEMORYUNIT unit;
    HRBNODE memory_node = rb_tree_find_int(mgr->memory_mgr, buffer_size);

    if (!memory_node)
    {
        unit = create_memory_unit(buffer_size);

        if (!unit)
        {
            return 0;
        }

        rb_tree_insert_int(mgr->memory_mgr, buffer_size, unit);
    }
    else
    {
        unit = (HMEMORYUNIT)rb_node_value(memory_node);
    }

    return memory_unit_alloc(unit, 4*1024);
}

void _iocp_tcp_manager_free_memory(HNETMANAGER mgr, void* mem, int buffer_size)
{
    HRBNODE memory_node = rb_tree_find_int(mgr->memory_mgr, buffer_size);

    HMEMORYUNIT check_unit = *(mem_unit**)((unsigned char*)mem - sizeof(void*));

    if (rb_node_value(memory_node) != check_unit)
    {
        CRUSH_CODE;
        return;
    }

    memory_unit_free((HMEMORYUNIT)rb_node_value(memory_node), mem);
}

extern HLOOPCACHE create_loop_cache_ex(size_t size, char* data);

HSESSION _iocp_tcp_manager_alloc_socket(HNETMANAGER mgr, int recv_buf_size, int send_buf_size)
{
    HSESSION socket = 0;

    if (recv_buf_size < 1024)
    {
        recv_buf_size = 1024;
    }

    if (send_buf_size < 1024)
    {
        send_buf_size = 1024;
    }

    SOCKET_LOCK;
    socket = memory_unit_alloc_ex(mgr->socket_pool, 0);
    SOCKET_UNLOCK;

    if (socket)
    {
        if (socket->mgr != mgr)
        {
            CRUSH_CODE;
        }

        _iocp_tcp_socket_reset(socket);

        if (!socket->recv_loop_cache)
        {
            if (socket->send_loop_cache)
            {
                CRUSH_CODE;
            }

            SOCKET_LOCK;
            socket->recv_loop_cache = create_loop_cache_ex(recv_buf_size, (char*)_iocp_tcp_manager_alloc_memory(mgr, recv_buf_size));
            socket->send_loop_cache = create_loop_cache_ex(send_buf_size, (char*)_iocp_tcp_manager_alloc_memory(mgr, send_buf_size));
            SOCKET_UNLOCK;
        }
        else
        {
            if ((int)loop_cache_size(socket->recv_loop_cache) != recv_buf_size)
            {
                SOCKET_LOCK;
                _iocp_tcp_manager_free_memory(mgr, loop_cache_get_cache(socket->recv_loop_cache), (int)loop_cache_size(socket->recv_loop_cache));
                loop_cache_reset(socket->recv_loop_cache, recv_buf_size, (char*)_iocp_tcp_manager_alloc_memory(mgr, recv_buf_size));
                SOCKET_UNLOCK;
            }

            if ((int)loop_cache_size(socket->send_loop_cache) != send_buf_size)
            {
                SOCKET_LOCK;
                _iocp_tcp_manager_free_memory(mgr, loop_cache_get_cache(socket->send_loop_cache), (int)loop_cache_size(socket->send_loop_cache));
                loop_cache_reset(socket->send_loop_cache, send_buf_size, (char*)_iocp_tcp_manager_alloc_memory(mgr, send_buf_size));
                SOCKET_UNLOCK;
            }
        }

        loop_cache_reinit(socket->recv_loop_cache);
        loop_cache_reinit(socket->send_loop_cache);

        return socket;
    }
    else
    {
        return 0;
    }
}

void _iocp_tcp_manager_free_socket(HNETMANAGER mgr, HSESSION socket)
{
    SOCKET_LOCK;
    memory_unit_free(mgr->socket_pool, socket);
    SOCKET_UNLOCK;
}

#define EVENT_LOCK EnterCriticalSection(&socket->mgr->evt_lock)
#define EVENT_UNLOCK LeaveCriticalSection(&socket->mgr->evt_lock)

void _push_data_event(HSESSION socket, int data_len)
{
    if (socket->state == SOCKET_STATE_ESTABLISH)
    {
        struct st_net_event* evt;
        size_t evt_len = sizeof(struct st_net_event);

        EVENT_LOCK;
        loop_cache_get_free(socket->mgr->evt_queue, (char**)&evt, &evt_len);

        if (evt_len != sizeof(struct st_net_event))
        {
#ifdef _DEBUG
            char sz_len[32];
            sprintf(sz_len, "len=%zu\n", evt_len);
            OutputDebugString(sz_len);
#endif
            CRUSH_CODE;
        }

        evt->socket = socket;
        evt->type = NET_EVENT_DATA;
        evt->evt_data.data_len = data_len;

        loop_cache_push(socket->mgr->evt_queue, evt_len);
        EVENT_UNLOCK;
    }
}

void _push_establish_event(HLISTENER listener, HSESSION socket)
{
    struct st_net_event* evt;
    size_t evt_len = sizeof(struct st_net_event);
    EVENT_LOCK;
    loop_cache_get_free(socket->mgr->evt_queue, (char**)&evt, &evt_len);

    if (evt_len != sizeof(struct st_net_event))
    {
#ifdef _DEBUG
        char sz_len[32];
        sprintf(sz_len, "len=%zu\n", evt_len);
        OutputDebugString(sz_len);
#endif
        CRUSH_CODE;
    }

    evt->socket = socket;
    evt->type = NET_EVENT_ESTABLISH;
    evt->evt_establish.listener = listener;

    loop_cache_push(socket->mgr->evt_queue, evt_len);
    EVENT_UNLOCK;
}

void _push_system_error_event(HSESSION socket, int err_code)
{
    struct st_net_event* evt;
    size_t evt_len = sizeof(struct st_net_event);
    EVENT_LOCK;
    loop_cache_get_free(socket->mgr->evt_queue, (char**)&evt, &evt_len);

    if (evt_len != sizeof(struct st_net_event))
    {
#ifdef _DEBUG
        char sz_len[32];
        sprintf(sz_len, "len=%zu\n", evt_len);
        OutputDebugString(sz_len);
#endif
        CRUSH_CODE;
    }

    evt->socket = socket;
    evt->type = NET_EVENT_SYSTEM_ERROR;
    evt->evt_system_error.err_code = err_code;

    loop_cache_push(socket->mgr->evt_queue, evt_len);
    EVENT_UNLOCK;
}

void _push_module_error_event(HSESSION socket, int err_code)
{
    struct st_net_event* evt;
    size_t evt_len = sizeof(struct st_net_event);
    EVENT_LOCK;
    loop_cache_get_free(socket->mgr->evt_queue, (char**)&evt, &evt_len);

    if (evt_len != sizeof(struct st_net_event))
    {
#ifdef _DEBUG
        char sz_len[32];
        sprintf(sz_len, "len=%zu\n", evt_len);
        OutputDebugString(sz_len);
#endif
        CRUSH_CODE;
    }

    evt->socket = socket;
    evt->type = NET_EVENT_MODULE_ERROR;
    evt->evt_module_error.err_code = err_code;

    loop_cache_push(socket->mgr->evt_queue, evt_len);
    EVENT_UNLOCK;
}

void _push_terminate_event(HSESSION socket)
{
    struct st_net_event* evt;
    size_t evt_len = sizeof(struct st_net_event);
    EVENT_LOCK;
    loop_cache_get_free(socket->mgr->evt_queue, (char**)&evt, &evt_len);

    if (evt_len != sizeof(struct st_net_event))
    {
#ifdef _DEBUG
        char sz_len[32];
        sprintf(sz_len, "len=%zu\n", evt_len);
        OutputDebugString(sz_len);
#endif
        CRUSH_CODE;
    }

    evt->socket = socket;
    evt->type = NET_EVENT_TERMINATE;

    loop_cache_push(socket->mgr->evt_queue, evt_len);
    EVENT_UNLOCK;
}

void _push_connect_fail_event(HSESSION socket, int err_code)
{
    struct st_net_event* evt;
    size_t evt_len = sizeof(struct st_net_event);
    EVENT_LOCK;
    loop_cache_get_free(socket->mgr->evt_queue, (char**)&evt, &evt_len);

    if (evt_len != sizeof(struct st_net_event))
    {
#ifdef _DEBUG
        char sz_len[32];
        sprintf(sz_len, "len=%zu\n", evt_len);
        OutputDebugString(sz_len);
#endif
        CRUSH_CODE;
    }

    evt->socket = socket;
    evt->type = NET_EVENT_CONNECT_FAIL;
    evt->evt_connect_fail.err_code = err_code;

    loop_cache_push(socket->mgr->evt_queue, evt_len);
    EVENT_UNLOCK;
}

void _push_recv_active_event(HSESSION socket)
{
    if (socket->state == SOCKET_STATE_ESTABLISH)
    {
        struct st_net_event* evt;
        size_t evt_len = sizeof(struct st_net_event);
        EVENT_LOCK;
        loop_cache_get_free(socket->mgr->evt_queue, (char**)&evt, &evt_len);

        if (evt_len != sizeof(struct st_net_event))
        {
#ifdef _DEBUG
            char sz_len[32];
            sprintf(sz_len, "len=%zu\n", evt_len);
            OutputDebugString(sz_len);
#endif
            CRUSH_CODE;
        }

        evt->socket = socket;
        evt->type = NET_EVENT_RECV_ACTIVE;

        loop_cache_push(socket->mgr->evt_queue, evt_len);
        EVENT_UNLOCK;
    }
}

void _iocp_tcp_socket_close(HSESSION socket, int error)
{
    if (SOCKET_STATE_ESTABLISH == InterlockedCompareExchange(&socket->state, SOCKET_STATE_TERMINATE, SOCKET_STATE_ESTABLISH))
    {
        switch (error)
        {
        case ERROR_SYSTEM:
        {
            socket->data_has_send = socket->data_need_send;
            _push_system_error_event(socket, WSAGetLastError());
        }
        break;
        case ERROR_SEND_OVERFLOW:
        case ERROR_RECV_OVERFLOW:
        case ERROR_PACKET:
        {
            socket->data_has_send = socket->data_need_send;
            _push_module_error_event(socket, error);
        }
        break;
        default:
        {
            _push_terminate_event(socket);
        }
        break;
        }
    }
}

bool _iocp_tcp_socket_post_recv_req(HSESSION socket)
{
    ZeroMemory(&socket->iocp_recv_data.over_lapped, sizeof(socket->iocp_recv_data.over_lapped));

    ++socket->recv_req;

    if (PostQueuedCompletionStatus(socket->mgr->iocp_port, 0xffffffff, (ULONG_PTR)socket, &socket->iocp_recv_data.over_lapped))
    {
        return true;
    }

    --socket->recv_req;
    return false;
}

bool _iocp_tcp_socket_post_recv(HSESSION socket)
{
    DWORD byte_received;
    DWORD flags = 0;

    char* recv_ptr = 0;
    size_t recv_len = 0;

    ZeroMemory(&socket->iocp_recv_data.over_lapped, sizeof(socket->iocp_recv_data.over_lapped));

    loop_cache_get_free(socket->recv_loop_cache, &recv_ptr, &recv_len);

    socket->iocp_recv_data.wsa_buf.buf = recv_ptr;
    socket->iocp_recv_data.wsa_buf.len = (ULONG)recv_len;

    ++socket->recv_req;

    if (WSARecv(socket->socket, &socket->iocp_recv_data.wsa_buf, 1, &byte_received, &flags, &socket->iocp_recv_data.over_lapped, NULL))
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            --socket->recv_req;
            return false;
        }
    }

    return true;
}

void _iocp_tcp_socket_on_recv(HSESSION socket, BOOL ret, DWORD trans_byte)
{
    ++socket->recv_ack;
    if (!ret)
    {
        _iocp_tcp_socket_close(socket, ERROR_SYSTEM);
        return;
    }

    if (socket->state != SOCKET_STATE_ESTABLISH)
    {
        return;
    }

    switch (trans_byte)
    {
    case 0:
        {
            _iocp_tcp_socket_close(socket, ERROR_NONE);
            return;
        }
        break;
    case 0xffffffff:
        {
            trans_byte = 0;
        }
        break;
    }

    if (trans_byte)
    {
        if (!loop_cache_push(socket->recv_loop_cache, trans_byte))
        {
            CRUSH_CODE;
        }

        _push_data_event(socket, trans_byte);
    }

    if (loop_cache_free_size(socket->recv_loop_cache))
    {
        if (!_iocp_tcp_socket_post_recv(socket))
        {
            _iocp_tcp_socket_close(socket, ERROR_SYSTEM);
        }
    }
    else
    {
        _push_recv_active_event(socket);
    }
}

bool _iocp_tcp_socket_post_send_req(HSESSION socket)
{
    ZeroMemory(&socket->iocp_send_data.over_lapped, sizeof(socket->iocp_send_data.over_lapped));

    ++socket->send_req;

    if (PostQueuedCompletionStatus(socket->mgr->iocp_port, 0xffffffff, (ULONG_PTR)socket, &socket->iocp_send_data.over_lapped))
    {
        return true;
    }

    --socket->send_req;

    return false;
}

bool _iocp_tcp_socket_post_send(HSESSION socket)
{
    DWORD number_of_byte_sent = 0;

    ZeroMemory(&socket->iocp_send_data.over_lapped, sizeof(socket->iocp_send_data.over_lapped));

    ++socket->send_req;

    if (WSASend(socket->socket, &socket->iocp_send_data.wsa_buf, 1, &number_of_byte_sent, 0, &socket->iocp_send_data.over_lapped, NULL))
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            --socket->send_req;
            return false;
        }
    }

    return true;
}

void _iocp_tcp_socket_on_send(HSESSION socket, BOOL ret, DWORD trans_byte)
{
    if (!ret)
    {
        _iocp_tcp_socket_close(socket, ERROR_SYSTEM);

        InterlockedIncrement(&socket->send_ack);
        return;
    }

    if ((socket->state != SOCKET_STATE_ESTABLISH) &&
        (socket->state != SOCKET_STATE_TERMINATE))
    {
        InterlockedIncrement(&socket->send_ack);
        return;
    }

    if (0 == trans_byte)
    {
        _iocp_tcp_socket_close(socket, ERROR_SYSTEM);

        InterlockedIncrement(&socket->send_ack);
        return;
    }

    if (0xffffffff == trans_byte)
    {
        trans_byte = 0;
        socket->iocp_send_data.wsa_buf.len = 0;
    }
    else
    {
        socket->data_has_send += trans_byte;

        if (!loop_cache_pop(socket->send_loop_cache, trans_byte))
        {
            CRUSH_CODE;
        }
    }

    char* send_ptr = 0;
    size_t send_len = 0;

    loop_cache_get_data(socket->send_loop_cache, &send_ptr, &send_len);

    if (send_len)
    {
        socket->iocp_send_data.wsa_buf.buf = send_ptr;
        socket->iocp_send_data.wsa_buf.len = (ULONG)send_len;

        if (!_iocp_tcp_socket_post_send(socket))
        {
            _iocp_tcp_socket_close(socket, ERROR_SYSTEM);
        }
    }

    InterlockedIncrement(&socket->send_ack);
}

bool _iocp_tcp_socket_post_connect_req(HSESSION socket, BOOL reuse_addr)
{
    ZeroMemory(&socket->iocp_recv_data.over_lapped, sizeof(socket->iocp_recv_data.over_lapped));

    ++socket->recv_req;

    socket->iocp_recv_data.operation= IOCP_OPT_CONNECT_REQ;

    if (PostQueuedCompletionStatus(socket->mgr->iocp_port, reuse_addr, (ULONG_PTR)socket, &socket->iocp_recv_data.over_lapped))
    {
        return true;
    }

    --socket->recv_req;
    return false;
}

void _iocp_tcp_socket_connect_ex(HSESSION socket_ptr, BOOL reuse_addr)
{
    struct sockaddr_in addr={0};

    ++socket_ptr->recv_ack;

    socket_ptr->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (INVALID_SOCKET == socket_ptr->socket)
    {
        goto ERROR_DEAL;
    }

    if (reuse_addr)
    {
        INT32 nReuse = 1;
        if (setsockopt(socket_ptr->socket, SOL_SOCKET, SO_REUSEADDR, (char*)&nReuse, sizeof(nReuse)) == SOCKET_ERROR)
        {
            goto ERROR_DEAL;
        }
    }

    addr.sin_family = AF_INET;

    if (socket_ptr->local_ip)
    {
        addr.sin_addr.S_un.S_addr = socket_ptr->local_ip;
    }
    else
    {
        addr.sin_addr.S_un.S_addr = htonl(ADDR_ANY);
    }

    if (socket_ptr->local_port)
    {
        addr.sin_port = socket_ptr->local_port;
    }
    else
    {
        addr.sin_port = htons(0);
    }

    if (SOCKET_ERROR == bind(socket_ptr->socket, (struct sockaddr*)&addr, sizeof(addr)))
    {
        goto ERROR_DEAL;
    }

    if (!_iocp_tcp_socket_bind_iocp_port(socket_ptr))
    {
        goto ERROR_DEAL;
    }

    addr.sin_port = socket_ptr->peer_port;
    addr.sin_addr.s_addr = socket_ptr->peer_ip;

    socket_ptr->iocp_recv_data.operation = IOCP_OPT_CONNECT;

    ++socket_ptr->recv_req;

    if (!socket_ptr->mgr->func_connectex(socket_ptr->socket, (struct sockaddr*)&addr, sizeof(addr), 0, 0, 0, &socket_ptr->iocp_recv_data.over_lapped))
    {
        if (WSAGetLastError() != ERROR_IO_PENDING)
        {
            --socket_ptr->recv_req;
            goto ERROR_DEAL;
        }
    }

    return;

ERROR_DEAL:

    _push_connect_fail_event(socket_ptr, WSAGetLastError());

    if (socket_ptr->socket != INVALID_SOCKET)
    {
        closesocket(socket_ptr->socket);
        socket_ptr->socket = INVALID_SOCKET;
    }
}

void _iocp_tcp_socket_on_connect(HSESSION socket, BOOL ret)
{
    struct sockaddr_in addr = {0};
    int addr_len = sizeof(addr);

    ++socket->recv_ack;

    if (!ret)
    {
        goto ERROR_DEAL;
    }

    if (SOCKET_ERROR == setsockopt(socket->socket, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, 0, 0))
    {
        goto ERROR_DEAL;
    }

    if (SOCKET_ERROR == getsockname(socket->socket, (struct sockaddr*)&addr, &addr_len))
    {
        goto ERROR_DEAL;
    }

    socket->local_ip = addr.sin_addr.s_addr;
    socket->local_port = addr.sin_port;

    socket->state = SOCKET_STATE_ESTABLISH;

    socket->iocp_recv_data.operation = IOCP_OPT_RECV;
    socket->iocp_send_data.operation = IOCP_OPT_SEND;

    int no_delay = 1;
    setsockopt(socket->socket, IPPROTO_TCP,
        TCP_NODELAY, (char*)&no_delay, sizeof(no_delay));

    _push_establish_event(0, socket);

    if (!_iocp_tcp_socket_post_recv(socket))
    {
        _iocp_tcp_socket_close(socket, ERROR_SYSTEM);
    }

    return;

ERROR_DEAL:

    _push_connect_fail_event(socket, WSAGetLastError());

    closesocket(socket->socket);
    socket->socket = INVALID_SOCKET;
}

void _mod_timer_close(HSESSION socket, int elapse)
{
    if (socket->timer_send)
    {
        timer_del(socket->timer_send);
        socket->timer_send = 0;
    }

    if (socket->timer_close)
    {
        timer_mod(socket->timer_close, elapse, -1, socket);
        return;
    }

    socket->timer_close = timer_add(socket->mgr->timer_mgr, elapse, -1, socket);

    if (!socket->timer_close)
    {
        CRUSH_CODE;
    }
}

void _mod_timer_send(HSESSION socket, int elapse)
{
    if (socket->timer_send)
    {
        timer_mod(socket->timer_send, elapse, -1, socket);
        return;
    }

    socket->timer_send = timer_add(socket->mgr->timer_mgr, elapse, -1, socket);

    if (!socket->timer_send)
    {
        CRUSH_CODE;
    }
}

//////////////////////////////////////////////////////////////////////////
bool _iocp_tcp_listener_bind_iocp_port(HLISTENER listener)
{
    if (CreateIoCompletionPort((HANDLE)listener->socket, listener->mgr->iocp_port, (ULONG_PTR)listener, 0))
    {
        return true;
    }

    return false;
}

bool _iocp_tcp_listener_post_accept_ex(HLISTENER listener, struct st_iocp_listen_data* iocp_listen_data_ptr)
{
    DWORD bytes = 0;

    BOOL ret;

    SOCKET accept_socket = WSASocket(
        AF_INET,
        SOCK_STREAM,
        0,
        NULL,
        0,
        WSA_FLAG_OVERLAPPED);

    if (INVALID_SOCKET == accept_socket)
    {
        goto ERROR_DEAL;
    }

    memset(&(iocp_listen_data_ptr->data.over_lapped), 0, sizeof(iocp_listen_data_ptr->data.over_lapped));

    iocp_listen_data_ptr->accept_socket = accept_socket;
    iocp_listen_data_ptr->data.listener = listener;

    ret = listener->mgr->func_acceptex(
        listener->socket,
        accept_socket,
        iocp_listen_data_ptr->data.wsa_buf.buf,
        0,
        sizeof(SOCKADDR_IN)+16,
        sizeof(SOCKADDR_IN)+16,
        &bytes,
        &(iocp_listen_data_ptr->data.over_lapped));

    if (!ret)
    {
        if (WSA_IO_PENDING != WSAGetLastError())
        {
            goto ERROR_DEAL;
        }
    }

    //::InterlockedIncrement(&m_post_acceptex);

    return true;

ERROR_DEAL:

    iocp_listen_data_ptr->data.listener = 0;
    if (accept_socket != INVALID_SOCKET)
    {
        closesocket(accept_socket);
    }

    return false;
}

void _iocp_tcp_listener_on_accept(HLISTENER listener, BOOL ret, struct st_iocp_listen_data* iocp_listen_data_ptr)
{
    HSESSION socket;

    SOCKET accept_socket;

    char addr_cache[MAX_ADDR_SIZE];

    struct sockaddr_in* remote_addr = NULL;
    struct sockaddr_in* local_addr = NULL;

    INT32 remote_addr_len = sizeof(struct sockaddr_in);
    INT32 local_addr_len = sizeof(struct sockaddr_in);
    INT32 addr_len = sizeof(struct sockaddr_in)+16;

    iocp_listen_data_ptr->data.listener = 0;

    if (listener->state != SOCKET_STATE_LISTEN)
    {
        closesocket(iocp_listen_data_ptr->accept_socket);
        return;
    }

    memcpy(addr_cache, iocp_listen_data_ptr->data.wsa_buf.buf, MAX_ADDR_SIZE);

    accept_socket = iocp_listen_data_ptr->accept_socket;

    if (!_iocp_tcp_listener_post_accept_ex(listener, iocp_listen_data_ptr))
    {
    }

    if (!ret)
    {
        closesocket(accept_socket);
        return;
    }

    socket = _iocp_tcp_manager_alloc_socket(listener->mgr, listener->recv_buf_size, listener->send_buf_size);

    if (!socket)
    {
        closesocket(accept_socket);
        return;
    }

    listener->mgr->func_getacceptexsockaddrs(
        addr_cache,
        0,
        addr_len,
        addr_len,
        (SOCKADDR**)&local_addr,
        &local_addr_len,
        (SOCKADDR**)&remote_addr,
        &remote_addr_len);

    socket->socket = accept_socket;
    socket->pkg_parser = listener->pkg_parser;

    if (!_iocp_tcp_socket_bind_iocp_port(socket))
    {
        closesocket(accept_socket);
        _iocp_tcp_manager_free_socket(listener->mgr, socket);
        return;
    }

    socket->peer_ip = remote_addr->sin_addr.s_addr;
    socket->peer_port = remote_addr->sin_port;

    socket->local_ip = local_addr->sin_addr.s_addr;
    socket->local_port = local_addr->sin_port;

    socket->iocp_recv_data.operation = IOCP_OPT_RECV;
    socket->iocp_send_data.operation = IOCP_OPT_SEND;

    socket->state = SOCKET_STATE_ESTABLISH;

    int no_delay = 1;
    setsockopt(socket->socket, IPPROTO_TCP,
        TCP_NODELAY, (char*)&no_delay, sizeof(no_delay));

    _push_establish_event(listener, socket);

    if (!_iocp_tcp_socket_post_recv(socket))
    {
        _iocp_tcp_socket_close(socket, ERROR_SYSTEM);
    }
}

void iocp_tcp_close_listener(HLISTENER listener)
{
    int i;
    listener->state = SOCKET_STATE_NONE;

    if (listener->socket != INVALID_SOCKET)
    {
        closesocket(listener->socket);
        listener->socket = INVALID_SOCKET;
    }

CHECK_POST_ACCEPT_BEGIN:

    for (i = 0; i < listener->max_accept_ex_num; i++)
    {
        if (listener->arry_iocp_data[i].data.listener)
        {
            Sleep(50);
            goto CHECK_POST_ACCEPT_BEGIN;
        }
    }

    if (listener->arry_addr_cache)
    {
        free(listener->arry_addr_cache);
        listener->arry_addr_cache = 0;
    }

    if (listener->arry_iocp_data)
    {
        free(listener->arry_iocp_data);
        listener->arry_iocp_data = 0;
    }
}

bool _iocp_tcp_listener_listen(HLISTENER listener, int max_accept_ex_num, const char* ip, UINT16 port, bool bReUseAddr)
{
    int i;
    struct sockaddr_in addr={0};

    listener->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    listener->arry_addr_cache = NULL;
    listener->arry_iocp_data = NULL;

    if (INVALID_SOCKET == listener->socket)
    {
        goto ERROR_DEAL;
    }

    if (bReUseAddr)
    {
        INT32 nReuse = 1;
        if (setsockopt(listener->socket, SOL_SOCKET, SO_REUSEADDR, (char*)&nReuse, sizeof(nReuse)) == SOCKET_ERROR)
        {
            goto ERROR_DEAL;
        }
    }

    addr.sin_family = AF_INET;

    if((!strcmp(ip, "0.0.0.0")) || (!strcmp(ip, "0")))
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    else
        addr.sin_addr.s_addr = inet_addr(ip);

    addr.sin_port = htons(port);

    if (SOCKET_ERROR == bind(listener->socket, (struct sockaddr*)&addr, sizeof(addr)))
    {
        goto ERROR_DEAL;
    }

    if (SOCKET_ERROR == listen(listener->socket, MAX_BACKLOG))
    {
        goto ERROR_DEAL;
    }

    if (!_iocp_tcp_listener_bind_iocp_port(listener))
    {
        goto ERROR_DEAL;
    }

    listener->max_accept_ex_num = max(max_accept_ex_num, 256);

    listener->arry_addr_cache = (char*)malloc(listener->max_accept_ex_num*MAX_ADDR_SIZE);
    if (!listener->arry_addr_cache)
    {
        goto ERROR_DEAL;
    }

    listener->arry_iocp_data = (struct st_iocp_listen_data*)malloc(sizeof(struct st_iocp_listen_data)*listener->max_accept_ex_num);
    if (!listener->arry_iocp_data)
    {
        goto ERROR_DEAL;
    }

    //m_post_acceptex = 0;

    listener->state = SOCKET_STATE_LISTEN;
    //SetState(SOCKET_STATE_LISTEN);

    for (i = 0; i < listener->max_accept_ex_num; i++)
    {
        listener->arry_iocp_data[i].data.wsa_buf.buf = listener->arry_addr_cache+i*MAX_ADDR_SIZE;
        listener->arry_iocp_data[i].data.wsa_buf.len = MAX_ADDR_SIZE;
        listener->arry_iocp_data[i].data.operation = IOCP_OPT_ACCEPT;
        listener->arry_iocp_data[i].data.listener = 0;


        if (!_iocp_tcp_listener_post_accept_ex(listener, listener->arry_iocp_data+i))
        {
            return false;
        }
    }

    return true;

ERROR_DEAL:

    iocp_tcp_close_listener(listener);

    return false;
}

//////////////////////////////////////////////////////////////////////////

bool _proc_net_event(HNETMANAGER mgr)
{
    struct st_net_event* evt;
    size_t evt_len = sizeof(struct st_net_event);

    loop_cache_get_data(mgr->evt_queue, (char**)&evt, &evt_len);

    if (evt_len < sizeof(struct st_net_event))
    {
        return false;
    }

    if (evt->socket->mgr != mgr)
    {
        CRUSH_CODE;
    }

    switch (evt->type)
    {
    case NET_EVENT_DATA:
    {
        char* data_ptr = 0;
        size_t data_len;
        int parser_len = 0;

        HSESSION socket = evt->socket;

        socket->data_has_recv += evt->evt_data.data_len;

        data_len = socket->data_has_recv;

        loop_cache_get_data(socket->recv_loop_cache, &data_ptr, &data_len);

        if ((int)data_len < socket->data_has_recv)
        {
            if (socket->data_has_recv > mgr->max_pkg_buf_size)
            {
                for (;;)
                {
                    mgr->max_pkg_buf_size += 1024;

                    if (mgr->max_pkg_buf_size > socket->data_has_recv)
                    {
                        free(mgr->max_pkg_buf);
                        mgr->max_pkg_buf = (char*)malloc(mgr->max_pkg_buf_size);
                        break;
                    }
                }
            }

            if (!loop_cache_copy_data(socket->recv_loop_cache, mgr->max_pkg_buf, socket->data_has_recv))
            {
                CRUSH_CODE;
            }

            data_ptr = mgr->max_pkg_buf;
        }

        while (socket->data_has_recv)
        {
            int pkg_len = 0;
            if (socket->pkg_parser)
            {
                pkg_len = socket->pkg_parser(socket, data_ptr, socket->data_has_recv);
            }
            else
            {
                pkg_len = socket->data_has_recv;
            }

            if (pkg_len > 0)
            {
                if (pkg_len > socket->data_has_recv)
                {
                    _iocp_tcp_socket_close(socket, ERROR_PACKET);
                    break;
                }

                mgr->func_on_recv(socket, data_ptr, pkg_len);

                data_ptr += pkg_len;
                socket->data_has_recv -= pkg_len;
                parser_len += pkg_len;
            }
            else if (pkg_len == 0)
            {
                break;
            }
            else
            {
                _iocp_tcp_socket_close(socket, ERROR_PACKET);
                break;
            }
        }

        if (parser_len)
        {
            if (!loop_cache_pop(socket->recv_loop_cache, parser_len))
            {
                CRUSH_CODE;
            }
        }
    }
    break;
    case NET_EVENT_ESTABLISH:
    {
        _mod_timer_send(evt->socket, DELAY_SEND_CHECK);
        mgr->func_on_establish(evt->evt_establish.listener, evt->socket);
    }
    break;
    case NET_EVENT_MODULE_ERROR:
    {
        mgr->func_on_error(evt->socket, evt->evt_module_error.err_code, 0);
        mgr->func_on_terminate(evt->socket);

        _mod_timer_close(evt->socket, DELAY_CLOSE_SOCKET);
    }
    break;
    case NET_EVENT_SYSTEM_ERROR:
    {
        mgr->func_on_error(evt->socket, ERROR_SYSTEM, evt->evt_system_error.err_code);
        mgr->func_on_terminate(evt->socket);

        _mod_timer_close(evt->socket, DELAY_CLOSE_SOCKET);
    }
    break;
    case NET_EVENT_TERMINATE:
    {
        mgr->func_on_terminate(evt->socket);
        _mod_timer_close(evt->socket, DELAY_CLOSE_SOCKET);
    }
    break;
    case NET_EVENT_CONNECT_FAIL:
    {
        mgr->func_on_error(evt->socket, ERROR_CONNECT_FAIL, evt->evt_connect_fail.err_code);
        evt->socket->state = SOCKET_STATE_TERMINATE;

        _mod_timer_close(evt->socket, DELAY_CLOSE_SOCKET);
    }
    break;
    //case NET_EVENT_SEND_CONTROL:
    //    {
    //        CIocpTcpSocket* socket = (CIocpTcpSocket*)evt->socket;
    //        ModTimer(socket, evt->evt_send_control.send_delay_time, -1);
    //    }
    //    break;
    case NET_EVENT_RECV_ACTIVE:
    {
        if (loop_cache_free_size(evt->socket->recv_loop_cache))
        {
            _iocp_tcp_socket_post_recv_req(evt->socket);
        }
        else
        {
            _iocp_tcp_socket_close(evt->socket, ERROR_RECV_OVERFLOW);
        }
    }
    break;
    }

    if (!loop_cache_pop(mgr->evt_queue, sizeof(struct st_net_event)))
    {
        CRUSH_CODE;
    }

    return true;
}

unsigned WINAPI _iocp_thread_func(LPVOID param)
{
    HNETMANAGER mgr = (HNETMANAGER)param;

    struct st_iocp_data* iocp_data_ptr;
    HSESSION iocp_tcp_socket_ptr;
    BOOL ret;
    DWORD byte_transferred;

    for (;;)
    {
        iocp_data_ptr = 0;
        iocp_tcp_socket_ptr = 0;
        byte_transferred = 0;

        ret = GetQueuedCompletionStatus(
            mgr->iocp_port,
            &byte_transferred,
            (PULONG_PTR)&iocp_tcp_socket_ptr,
            (LPOVERLAPPED*)&iocp_data_ptr,
            INFINITE);

        if (!iocp_data_ptr)
        {
            return 0;
        }

        switch (iocp_data_ptr->operation)
        {
        case IOCP_OPT_RECV:
            _iocp_tcp_socket_on_recv(iocp_data_ptr->socket, ret, byte_transferred);
            break;
        case IOCP_OPT_SEND:
            _iocp_tcp_socket_on_send(iocp_data_ptr->socket, ret, byte_transferred);
            break;
        case IOCP_OPT_ACCEPT:
        {
            struct st_iocp_listen_data* iocp_listen_data_ptr = (struct st_iocp_listen_data*)iocp_data_ptr;
            _iocp_tcp_listener_on_accept(iocp_listen_data_ptr->data.listener, ret, iocp_listen_data_ptr);
        }
        break;
        case IOCP_OPT_CONNECT_REQ:
            _iocp_tcp_socket_connect_ex(iocp_data_ptr->socket, (BOOL)byte_transferred);
            break;
        case IOCP_OPT_CONNECT:
            _iocp_tcp_socket_on_connect(iocp_data_ptr->socket, ret);
            break;
        case IOCP_OPT_ACCEPT_REQ:
            break;
        }
    }
}

bool _start_iocp_thread(HNETMANAGER mgr)
{
    int i;

    if (mgr->work_thread_num <= 0)
    {
        SYSTEM_INFO sys_info;
        GetSystemInfo(&sys_info);

        mgr->work_thread_num = sys_info.dwNumberOfProcessors*2+2;
    }

    mgr->work_threads = (HANDLE*)malloc(sizeof(HANDLE)*mgr->work_thread_num);

    for (i = 0; i < mgr->work_thread_num; i++)
    {
        mgr->work_threads[i] = NULL;
    }

    for (i = 0; i < mgr->work_thread_num; i++)
    {
        unsigned thread_id = 0;

        mgr->work_threads[i] = (HANDLE)_beginthreadex(NULL,
            0,
            _iocp_thread_func,
            mgr,
            0,
            &thread_id);

        if (!mgr->work_threads[i])
        {
            return false;
        }
    }

    return true;
}

void _stop_iocp_thread(HNETMANAGER mgr)
{
    int i;
    ULONG_PTR ptr = 0;
    for (i = 0; i < mgr->work_thread_num; i++)
    {
        if (mgr->work_threads[i])
        {

            PostQueuedCompletionStatus(mgr->iocp_port, 0, ptr, NULL);
        }
    }

    for (i = 0; i < mgr->work_thread_num; i++)
    {
        if (mgr->work_threads[i])
        {
            WaitForSingleObject(mgr->work_threads[i], INFINITE);
            CloseHandle(mgr->work_threads[i]);
            mgr->work_threads[i] = NULL;
        }
    }

    free(mgr->work_threads);
}

void _iocp_tcp_socket_on_timer_send(HSESSION socket)
{
    if (socket->state == SOCKET_STATE_ESTABLISH)
    {
        if (socket->data_need_send != socket->data_has_send)
        {
            if (socket->send_req == socket->send_ack)
            {
                if (!_iocp_tcp_socket_post_send_req(socket))
                {
                    _iocp_tcp_socket_close(socket, ERROR_SYSTEM);
                }
            }
        }
    }
}

void _iocp_tcp_socket_on_timer_close(HSESSION socket)
{
    switch (socket->state)
    {
    case SOCKET_STATE_TERMINATE:
    {
        if (socket->data_need_send != socket->data_has_send)
        {
            if (socket->send_req == socket->send_ack)
            {
                if (!_iocp_tcp_socket_post_send_req(socket))
                {
                    _iocp_tcp_socket_close(socket, ERROR_SYSTEM);
                }
            }

            return;
        }

        shutdown(socket->socket, SD_RECEIVE);

        socket->state = SOCKET_STATE_DELETE;
    }
    break;
    case SOCKET_STATE_DELETE:
    {
        if (socket->socket != INVALID_SOCKET)
        {
            closesocket(socket->socket);
            socket->socket = INVALID_SOCKET;
        }

        if ((socket->recv_req == socket->recv_ack) && (socket->send_req == socket->send_ack))
        {
            timer_del(socket->timer_close);
            socket->timer_close = 0;
            _iocp_tcp_manager_free_socket(socket->mgr, socket);
        }
    }
    break;
    }
}

void _iocp_tcp_on_timer(HTIMERINFO timer)
{
    HSESSION socket = (HSESSION)timer_get_data(timer);

    if (!socket)
    {
        CRUSH_CODE;
    }

    if (socket->timer_send == timer)
    {
        _iocp_tcp_socket_on_timer_send(socket);
    }
    else if (socket->timer_close == timer)
    {
        _iocp_tcp_socket_on_timer_close(socket);
    }
    else
    {
        CRUSH_CODE;
    }
}

//void _iocp_tcp_on_timer(HTIMERINFO timer)
//{
//    HSESSION socket = (HSESSION)timer_get_data(timer);
//
//    if (!socket)
//    {
//        CRUSH_CODE;
//    }
//
//    if (socket->timer_check != timer)
//    {
//        CRUSH_CODE;
//    }
//
//
//    switch (socket->state)
//    {
//    case SOCKET_STATE_ESTABLISH:
//    {
//        if (socket->data_need_send != socket->data_has_send)
//        {
//            if (socket->send_req == socket->send_ack)
//            {
//                if (!_iocp_tcp_socket_post_send_req(socket))
//                {
//                    _iocp_tcp_socket_close(socket, ERROR_SYSTEM);
//                }
//            }
//        }
//    }
//    break;
//    case SOCKET_STATE_TERMINATE:
//    {
//        if (socket->data_need_send != socket->data_has_send)
//        {
//            if (socket->send_req == socket->send_ack)
//            {
//                if (!_iocp_tcp_socket_post_send_req(socket))
//                {
//                    _iocp_tcp_socket_close(socket, ERROR_SYSTEM);
//                }
//            }
//
//            return;
//        }
//
//        shutdown(socket->socket, SD_RECEIVE);
//
//        socket->state = SOCKET_STATE_DELETE;
//
//        _mod_timer_check(socket, DELAY_CLOSE_SOCKET);
//    }
//    break;
//    case SOCKET_STATE_CONNECT_FAIL:
//    {
//        socket->state = SOCKET_STATE_DELETE;
//    }
//    break;
//    case SOCKET_STATE_DELETE:
//    {
//        if (socket->socket != INVALID_SOCKET)
//        {
//            closesocket(socket->socket);
//            socket->socket = INVALID_SOCKET;
//        }
//
//        if ((socket->recv_req == socket->recv_ack) && (socket->send_req == socket->send_ack))
//        {
//            timer_del(socket->timer_check);
//            socket->timer_check = 0;
//            _iocp_tcp_manager_free_socket(socket->mgr, socket);
//        }
//    }
//    break;
//    default:
//    {
//        CRUSH_CODE;
//    }
//    }
//}

bool _set_wsa_function(HNETMANAGER mgr)
{
    DWORD bytes;

    GUID func_guid = WSAID_CONNECTEX;

    GUID func_guid1 = WSAID_ACCEPTEX;

    GUID func_guid2 = WSAID_GETACCEPTEXSOCKADDRS;

    SOCKET tmp_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == tmp_sock)
    {
        return false;
    }

    if (WSAIoctl(tmp_sock, 
        SIO_GET_EXTENSION_FUNCTION_POINTER,
        &func_guid,
        sizeof(func_guid),
        &mgr->func_connectex,
        sizeof(mgr->func_connectex),
        &bytes, 0, 0))
    {
        closesocket(tmp_sock);
        return false;
    }

    if (WSAIoctl(tmp_sock,
        SIO_GET_EXTENSION_FUNCTION_POINTER,
        &func_guid1,
        sizeof(func_guid1),
        &mgr->func_acceptex,
        sizeof(mgr->func_acceptex),
        &bytes, 0, 0))
    {
        closesocket(tmp_sock);
        return false;
    }

    if (WSAIoctl(tmp_sock,
        SIO_GET_EXTENSION_FUNCTION_POINTER,
        &func_guid2,
        sizeof(func_guid2),
        &mgr->func_getacceptexsockaddrs,
        sizeof(mgr->func_getacceptexsockaddrs),
        &bytes, 0, 0))
    {
        closesocket(tmp_sock);
        return false;
    }

    closesocket(tmp_sock);

    return true;
}

extern void destroy_rb_tree_ex(HRBTREE tree);
void destroy_iocp_tcp(HNETMANAGER mgr)
{
    _stop_iocp_thread(mgr);

    if (mgr->max_pkg_buf)
    {
        free(mgr->max_pkg_buf);
        mgr->max_pkg_buf_size = 0;
    }

    if (mgr->memory_mgr)
    {
        HRBNODE memory_node = rb_first(mgr->memory_mgr);
        while (memory_node)
        {
            destroy_memory_unit((HMEMORYUNIT)rb_node_value(memory_node));
            memory_node = rb_next(memory_node);
        }
        destroy_rb_tree_ex(mgr->memory_mgr);
        mgr->memory_mgr = 0;
    }

    if (mgr->timer_mgr)
    {
        destroy_timer_manager(mgr->timer_mgr);
        mgr->timer_mgr = 0;
    }

    if (mgr->socket_pool)
    {
        destroy_memory_unit(mgr->socket_pool);
        mgr->socket_pool = 0;
    }

    if (mgr->iocp_port)
    {
        CloseHandle(mgr->iocp_port);
        mgr->iocp_port = 0;
    }

    WSACleanup();

    DeleteCriticalSection(&mgr->evt_lock);
    DeleteCriticalSection(&mgr->socket_lock);

    free(mgr);
}

extern HRBTREE create_rb_tree_ex(key_cmp cmp_func);

HNETMANAGER create_iocp_tcp(pfn_on_establish func_on_establish, pfn_on_terminate func_on_terminate, 
    pfn_on_error func_on_error, pfn_on_recv func_on_recv,
    int max_socket_num, int max_io_thread_num, int max_accept_ex_num)
{
    WORD version_requested;
    WSADATA wsa_data;
    int i;

    HSESSION* arry_socket_ptr = 0;

    HNETMANAGER mgr = (HNETMANAGER)malloc(sizeof(struct st_iocp_tcp_manager));

    mgr->max_socket_num = max_socket_num;
    mgr->work_thread_num = max_io_thread_num;
    mgr->max_accept_ex_num = max_accept_ex_num;

    mgr->func_on_establish = func_on_establish;
    mgr->func_on_terminate = func_on_terminate;
    mgr->func_on_error = func_on_error;
    mgr->func_on_recv = func_on_recv;

    InitializeCriticalSection(&mgr->evt_lock);
    InitializeCriticalSection(&mgr->socket_lock);

    version_requested = MAKEWORD(2, 2);

    if (WSAStartup(version_requested, &wsa_data))
    {
        goto ERROR_DEAL;
    }

    if (!_set_wsa_function(mgr))
    {
        goto ERROR_DEAL;
    }

    mgr->iocp_port = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

    if (!mgr->iocp_port)
    {
        goto ERROR_DEAL;
    }

    mgr->socket_pool = create_memory_unit(sizeof(struct st_iocp_tcp_socket));
    if (!mgr->socket_pool)
    {
        goto ERROR_DEAL;
    }

    arry_socket_ptr = (HSESSION*)malloc(sizeof(HSESSION)*max_socket_num);

    for (i = 0; i < max_socket_num; i++)
    {
        arry_socket_ptr[i] = memory_unit_alloc_ex(mgr->socket_pool, mgr->max_socket_num);

        arry_socket_ptr[i]->mgr = mgr;
        arry_socket_ptr[i]->recv_loop_cache = 0;
        arry_socket_ptr[i]->send_loop_cache = 0;
        arry_socket_ptr[i]->iocp_recv_data.socket = arry_socket_ptr[i];
        arry_socket_ptr[i]->iocp_send_data.socket = arry_socket_ptr[i];
    }

    for (i = 0; i < max_socket_num; i++)
    {
        memory_unit_free(mgr->socket_pool, arry_socket_ptr[i]);
    }

    free(arry_socket_ptr);
    arry_socket_ptr = 0;

    mgr->evt_queue = create_loop_cache(mgr->max_socket_num*5*sizeof(struct st_net_event), 0);
    if (!mgr->evt_queue)
    {
        goto ERROR_DEAL;
    }

    mgr->timer_mgr = create_timer_manager(_iocp_tcp_on_timer);
    if (!mgr->timer_mgr)
    {
        goto ERROR_DEAL;
    }

    mgr->memory_mgr = create_rb_tree_ex(0);
    if (!mgr->memory_mgr)
    {
        goto ERROR_DEAL;
    }

    mgr->max_pkg_buf_size = 1024;
    mgr->max_pkg_buf = (char*)malloc(mgr->max_pkg_buf_size);

    if (!_start_iocp_thread(mgr))
    {
        goto ERROR_DEAL;
    }

    return mgr;

ERROR_DEAL:
    destroy_iocp_tcp(mgr);
    return 0;
}

HSESSION iocp_tcp_connect(HNETMANAGER mgr,
    const char* ip, unsigned short port, int recv_buf_size, 
    int send_buf_size, pfn_parse_packet func, bool reuse_addr, 
    const char* bind_ip, unsigned short bind_port )
{
    HSESSION socket = _iocp_tcp_manager_alloc_socket(mgr, recv_buf_size, send_buf_size);

    if (!socket)
    {
        return 0;
    }

    if (mgr != socket->mgr)
    {
        CRUSH_CODE;
    }

    if (bind_ip)
    {
        socket->local_ip = inet_addr(bind_ip);
    }
    else
    {
        socket->local_ip = 0;
    }

    if (bind_port)
    {
        socket->local_port = htons(bind_port);
    }
    else
    {
        socket->local_port = 0;
    }

    socket->peer_ip = inet_addr(ip);
    socket->peer_port = htons(port);

    socket->pkg_parser = func;

    socket->state = SOCKET_STATE_CONNECT;

    if (!_iocp_tcp_socket_post_connect_req(socket, reuse_addr))
    {
        _iocp_tcp_manager_free_socket(mgr, socket);

        return 0;
    }

    return socket;
}

HLISTENER iocp_tcp_listen(HNETMANAGER mgr,
    const char* ip, unsigned short port, int recv_buf_size, int send_buf_size, 
    pfn_parse_packet func, bool reuse_addr)
{
    HLISTENER listener = (HLISTENER)malloc(sizeof(struct st_iocp_tcp_listener));

    listener->recv_buf_size = recv_buf_size;
    listener->send_buf_size = send_buf_size;

    listener->pkg_parser = func;
    listener->mgr = mgr;

    if (!_iocp_tcp_listener_listen(listener, mgr->max_accept_ex_num, ip, port, reuse_addr))
    {
        iocp_tcp_close_listener(listener);

        free(listener);

        return 0;
    }

    return listener;
}

bool iocp_tcp_send(HSESSION socket, const char* data, int len)
{
    if (len <= 0)
    {
        return true;
    }

    if (socket->state != SOCKET_STATE_ESTABLISH)
    {
        return false;
    }

    if (!loop_cache_push_data(socket->send_loop_cache, data, len))
    {
        _iocp_tcp_socket_close(socket, ERROR_SEND_OVERFLOW);
        return false;
    }

    socket->data_need_send += (unsigned int)len;

    if ((socket->data_need_send - socket->data_has_send) < (unsigned int)socket->data_delay_send_size)
    {
        return true;
    }

    if (socket->send_req == socket->send_ack)
    {
        if (!_iocp_tcp_socket_post_send_req(socket))
        {
            _iocp_tcp_socket_close(socket, ERROR_SYSTEM);
            return false;
        }
    }

    return true;
}

void iocp_tcp_close_session(HSESSION socket)
{
    _iocp_tcp_socket_close(socket, ERROR_NONE);
}

bool iocp_tcp_run(HNETMANAGER mgr, unsigned int run_time)
{
    unsigned int tick = GetTickCount();

    for (;;)
    {
        timer_update(mgr->timer_mgr, 0);

        if (!_proc_net_event(mgr))
        {
            return false;
        }

        if (run_time)
        {
            if (GetTickCount() - tick >= run_time)
            {
                break;
            }
        }
    }

    return true;
}

SOCKET iocp_tcp_session_socket(HSESSION socket)
{
    return socket->socket;
}

SOCKET iocp_tcp_listener_socket(HLISTENER listener)
{
    return listener->socket;
}

void iocp_tcp_set_listener_data(HLISTENER listener, void* user_data)
{
    listener->user_data = user_data;
}

void iocp_tcp_set_session_data(HSESSION socket, void* user_data)
{
    socket->user_data = user_data;
}

void* iocp_tcp_get_listener_data(HLISTENER listener)
{
    return listener->user_data;
}

void* iocp_tcp_get_session_data(HSESSION socket)
{
    return socket->user_data;
}

unsigned int iocp_tcp_get_peer_ip(HSESSION socket)
{
    return socket->peer_ip;
}

unsigned short iocp_tcp_get_peer_port(HSESSION socket)
{
    return socket->peer_port;
}

unsigned int iocp_tcp_get_local_ip(HSESSION socket)
{
    return socket->local_ip;
}

unsigned short iocp_tcp_get_local_port(HSESSION socket)
{
    return socket->local_port;
}

int iocp_tcp_get_send_free_size(HSESSION socket)
{
    return (int)loop_cache_free_size(socket->send_loop_cache);
}

void iocp_tcp_set_send_control(HSESSION socket, int pkg_size, int delay_time)
{
    int no_delay = 1;
    setsockopt(socket->socket, IPPROTO_TCP,
        TCP_NODELAY, (char*)&no_delay, sizeof(no_delay));

    socket->data_delay_send_size = pkg_size;
    _mod_timer_send(socket, delay_time);
}
#endif
