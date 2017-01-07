#ifndef SOCKET_H
#define SOCKET_H

#ifndef SOCKET_DEBUG
# ifndef NDEBUG
#  define SOCKET_DEBUG 0
# else
#  define SOCKET_DEBUG 0
# endif
#endif

#ifdef SOCKET_C
typedef struct socket_interface_node
{
 struct socket_interface_node *next;
 Socket_poll_typedef_offset io;
} socket_interface_node;
#endif

#define SOCKET_DNS_ADDRESS_SZ 80 /* FIXME: guessing */

#endif
