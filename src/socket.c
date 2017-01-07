#define SOCKET_C
/*
 *  Copyright (C) 1999 James Antill
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * email: james@twocan.org
 */
#include "main.h"

/* internal variables... */

#if 0
static struct pollfd io_indicators[256];
static unsigned long io_indicator_num = 0;
static unsigned long io_indicator_max = 0;
#endif

static socket_interface_node *socket_bind_io_start = NULL;


static ssize_t read_player(player *p, void *where, size_t length)
{
 int ammount_read = 0;
 int fd = -1;
 
 if (INVALID_PLAYER_SOCKET(p))
   return (0);

 fd = SOCKET_POLL_INDICATOR(p->io_indicator)->fd;
 if ((ammount_read = read(fd, where, length)) == -1)
   switch (errno)
   {
    case EINTR: /* interupt occured */
    case EAGAIN: /* too little data */
      return (-1);
      
    default:
    {
     socket_poll_del(p->io_indicator);
     close(fd);
     p->io_indicator = 0;
     user_logoff(p, NULL);
     return (0);
    }
   }
 else
   return (ammount_read);

 assert(FALSE);
 return (0);
}

ssize_t socket_writev(int *fd, const struct iovec *iovs, size_t num)
{
 ssize_t output_length = 0;
 
 assert(num);

 if (!num)
 {
  assert(FALSE);
  /* not sure if it's a brilliant idea but... least ppl won't be quit */
  return (0);
 }
 
 if (*fd == -1)
   return (0);
 
 if ((output_length = writev(*fd, iovs, num)) == -1)
   switch (errno)
   {
    case EINTR: /* interupt occured */
    case EAGAIN: /* too much data */
    case ENOSPC: /* no mem */
      return (-1);
      
    case EPIPE: /* sigpipe */
    default:
      shutdown(*fd, SHUT_RDWR);
      close(*fd);
      *fd = -1; /* for above check */
      
      return (0);
   }

 return (output_length);
}

void init_socket(void)
{
 struct protoent *the_protocol = getprotobyname("ip");
 configure_interface_node *inter = configure.socket_interfaces_start;
 int count = 0;

 if (!the_protocol)
 {
  vwlog("error", "socket protocol error: %d %s\n", errno, strerror(errno));
  exit (EXIT_FAILURE);
 }

#if 0
 while (count < 256)
   io_indicators[count++].fd = -1;
 count = 0;
#endif
 
 log_assert(inter);
 while (inter)
 {
  struct sockaddr_in socket_bind_in;
  socket_interface_node *socket_bind = NULL;
  int fd = -1;
  int dummy = 1;
  char ip_buf[sizeof("xxx.xxx.xxx.xxx")];

  socket_bind_in.sin_family = AF_INET;
  
  switch (inter->type)
  {
   case CONFIGURE_INTERFACE_TYPE_ANY:
     socket_bind_in.sin_addr.s_addr = htonl(INADDR_ANY);
     break;

   case CONFIGURE_INTERFACE_TYPE_IPV4:
   {
    struct hostent *tmp = NULL;

    /* ip_buf needed for later ... when printing errors */
    sprintf(ip_buf, "%d.%d.%d.%d", inter->u.ipv4->ip_address[0],
            inter->u.ipv4->ip_address[1],
            inter->u.ipv4->ip_address[2],
            inter->u.ipv4->ip_address[3]);
    tmp = gethostbyname(ip_buf);
    
    if (!tmp)
    {
     vwlog("error", "gethostbyname ipv4 error: %d\n", h_errno);
     inter = inter->next;
     continue;
    }
    socket_bind_in.sin_addr = *(struct in_addr *) tmp->h_addr_list[0];
   }
   break;

   case CONFIGURE_INTERFACE_TYPE_NAME:
   {
    struct hostent *tmp = gethostbyname(inter->u.name->dns_address);
    if (!tmp)
    {
     vwlog("error", "gethostbyname name error: %d\n", h_errno);
     inter = inter->next;
     continue;
    }
    socket_bind_in.sin_addr = *(struct in_addr *) tmp->h_addr_list[0];
   }
   break;
     
   default:
     vwlog("error", "Interface type %d\n", inter->type);
     inter = inter->next;
     continue;
  }

  socket_bind_in.sin_port = htons(inter->port);
  
  if ((fd = socket(PF_INET, SOCK_STREAM, the_protocol->p_proto)) == -1)
    shutdown_error("socket main error: %d %s\n", errno, strerror(errno));
  
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &dummy, sizeof(dummy)) == -1)
    shutdown_error("setsockopt: %d %s.\n", errno, strerror(errno));
  
  if (fcntl(fd, F_SETFD, 1) == -1)
    shutdown_error("fcntl COE: %d %s.\n", errno, strerror(errno));
  
  if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
    shutdown_error("fcntl no-blocking: %d %s.\n", errno, strerror(errno));
  
  if (bind(fd, (struct sockaddr *) &socket_bind_in, sizeof(struct sockaddr_in)) == -1)
    vwlog("error", "bind(%s:%d): %d %s.\n",
          (inter->type == CONFIGURE_INTERFACE_TYPE_ANY) ? "any" :
          (inter->type == CONFIGURE_INTERFACE_TYPE_IPV4) ? ip_buf :
          (inter->type == CONFIGURE_INTERFACE_TYPE_NAME) ? inter->u.name->dns_address :
          "** ERROR **", inter->port,
          errno, strerror(errno));
  else
  {
   if (!count++)
     fprintf(stderr, "\n"); /* only wants doing once */
   
   if (inter->port)
     fprintf(stderr, " Talker bound on: %s%s%d\n", 
             (inter->type == CONFIGURE_INTERFACE_TYPE_ANY) ? "" :
             (inter->type == CONFIGURE_INTERFACE_TYPE_IPV4) ? ip_buf :
             (inter->type == CONFIGURE_INTERFACE_TYPE_NAME) ? inter->u.name->dns_address :
             "**ERROR**",
             (inter->type == CONFIGURE_INTERFACE_TYPE_ANY) ? "" : ":",
             inter->port);
   else
   {
    socklen_t sz = sizeof(struct sockaddr_in);
    
    if (getsockname(fd, (struct sockaddr *) &socket_bind_in, &sz) == -1)
      shutdown_error("getsockname: %d %s.\n", errno, strerror(errno));
    
    fprintf(stderr, " Talker dynamically bound on: %s%s%d\n",
            (inter->type == CONFIGURE_INTERFACE_TYPE_ANY) ? "" :
            (inter->type == CONFIGURE_INTERFACE_TYPE_IPV4) ? ip_buf :
            (inter->type == CONFIGURE_INTERFACE_TYPE_NAME) ? inter->u.name->dns_address :
            "**ERROR**",
            (inter->type == CONFIGURE_INTERFACE_TYPE_ANY) ? "" : ":",
            ntohs(socket_bind_in.sin_port));
   }
  }
  inter = inter->next;
  
  if (listen(fd, configure.socket_listen_len) == -1)
    vwlog("error", "listen(%d): %d %s.\n", configure.socket_listen_len,
          errno, strerror(errno));
  
  if (!(socket_bind = XMALLOC(sizeof(socket_interface_node),
                              SOCKET_INTERFACE_NODE)))
    SHUTDOWN_MEM_ERR();
  
  socket_bind->next = socket_bind_io_start;
  socket_bind_io_start = socket_bind;
  if (!(socket_bind->io = socket_poll_add(fd)))
    shutdown_error(" Ran out of sockets on bind()");
  SOCKET_POLL_INDICATOR(socket_bind->io)->events |= POLLIN;
 }

 endprotoent(); /* cleanup */
}

static void socket_accept_connection(socket_interface_node *socket_bind)
{
 struct sockaddr_in incoming;
 socklen_t length = sizeof(incoming);
 player *p = NULL;
 int new_socket = accept(SOCKET_POLL_INDICATOR(socket_bind->io)->fd,
                         (struct sockaddr *) &incoming, &length);
 char buf[SOCKET_DNS_ADDRESS_SZ];
 
 if (new_socket == -1)
 {
  vwlog("error", "accept: %d %s\n", errno, strerror(errno));
  return;
 }

#if 0
 /* Sometimes this catches things that the accept() doesn't, however I'm
  * reluctant to leave it in because it will be dealt with via. the normal
  * means anyway */
 if (getpeername(new_socket, (struct sockaddr *) &incoming, &length) == -1)
 {
  close(new_socket);
  return;
 }
#endif
 
 fcntl(new_socket, F_SETFD, TRUE); /* set close on exec */
 fcntl(new_socket, F_SETFL, O_NONBLOCK);
 
 if (!(p = player_create()))
 { /* FIXME: add message */
  close(new_socket);
  return;
 }
 current_player = p; /* Can use in twinkles... */

 assert(logging_onto_count >= 0);
 ++logging_onto_count;

 player_list_cron_add(p);
 
 if (gettimeofday(&last_entered_left, NULL))
 {
  assert(FALSE);
 }
 
 p->io_indicator = socket_poll_add(new_socket);
 assert(p->io_indicator);
 if (!p->io_indicator)
   SHUTDOWN_MEM_ERR();
 SOCKET_POLL_INDICATOR(p->io_indicator)->events |= POLLIN;
 
 COPY_STR(p->dns_address,
          inet_ntop(incoming.sin_family, &incoming.sin_addr,
                    buf, SOCKET_DNS_ADDRESS_SZ),
          SOCKET_DNS_ADDRESS_SZ);
 if (!auth_parse_ip_addr(p->dns_address, p->ip_address, FALSE))
 {
  vwlog("error", "inet_ntop: %d %s\n", __LINE__, __FILE__);
  user_logoff(p, NULL);
  return;
 }

 DNS_ADD(p->ip_address);
  
 if (!auth_check_logon(p))
 {
  user_logoff(p, NULL);
  return;
 }
 
 logon_start(p);
 
 current_player = NULL;
}

void socket_copy_fd(player *to, player *from)
{
 char buffer[PROMPT_OUTPUT_SZ];
 
 SWAP_TYPE(to->io_indicator, from->io_indicator, Socket_poll_typedef_offset);

 assert(from->input_start &&
        (from->flag_raw_passwd ?
         !strcmp(from->passwd, from->input_start->input) : TRUE));
 assert((to->saved == from->saved) && (to == from->saved->player_ptr));
 assert(!to->flag_tmp_dont_save_after_this);
 
 /* the from pointer has just entered a password */
 if (from->input_start->comp_generated)
   ++from->input_comp_count;
 else
   from->input_comp_count = 0;
 input_del(from, from->input_start);

 SWAP_TYPE(to->input_start, from->input_start, input_node *);
 SWAP_TYPE(to->input_node_count, from->input_node_count, unsigned int);

 SWAP_TYPE(to->output_start, from->output_start, output_node *);
 assert(!to->output_buffer_tmp);
 assert(!from->output_buffer_tmp);
 SWAP_TYPE(to->output_buffer_tmp, from->output_buffer_tmp, output_node *);
 SWAP_TYPE(to->output_has_priority, from->output_has_priority, int);

 SWAP_TYPE(to->flag_tmp_prompt_do_output,
           from->flag_tmp_prompt_do_output, int);
 SWAP_TYPE(to->prompt_print_length, from->prompt_print_length, int);
 SWAP_TYPE(to->prompt_out_length, from->prompt_out_length, int);
 SWAP_TYPE(to->prompt_length, from->prompt_length, int);
 qstrcpy(buffer, from->prompt_output);
 qstrcpy(from->prompt_output, to->prompt_output);
 qstrcpy(to->prompt_output, buffer);

 qstrcpy(to->dns_address, from->dns_address);
 memcpy(to->ip_address, from->ip_address, 4);
 
 if ((to->automatic_term_size_got = from->automatic_term_size_got))
 {
  to->term_height = from->term_height;
  to->term_width = from->term_width;
 }

 to->last_command_timestamp = from->last_command_timestamp;
 to->logon_timestamp = from->logon_timestamp;
 
 SWAP_TYPE(to->input_last_eol, from->input_last_eol, int);
 SWAP_TYPE(to->extra_screen_routines, from->extra_screen_routines, int);
 SWAP_TYPE(to->repeat_prompt, from->repeat_prompt, int);
 SWAP_TYPE(to->telnet_option_eor_on, from->telnet_option_eor_on, int);
 SWAP_TYPE(to->telnet_option_bounce_echo_off,
           from->telnet_option_bounce_echo_off, int);
 SWAP_TYPE(to->telnet_option_do_echo, from->telnet_option_do_echo, int);
 
 SWAP_TYPE(to->telopt_do_echo, from->telopt_do_echo, int);
 SWAP_TYPE(to->telopt_do_eor, from->telopt_do_eor, int);
 SWAP_TYPE(to->telopt_do_sga, from->telopt_do_sga, int);
 SWAP_TYPE(to->telopt_do_naws, from->telopt_do_naws, int);
 SWAP_TYPE(to->telopt_do_compress, from->telopt_do_compress, int);
 
 SWAP_TYPE(to->output_compress_do, from->output_compress_do, int);

#ifdef HAVE_ZLIB_H
 SWAP_TYPE(to->output_compress_buf,
           from->output_compress_buf, unsigned char *);
 SWAP_TYPE(to->output_compress_ptr,
           from->output_compress_ptr, unsigned char *);
 SWAP_TYPE(to->output_compress_lib, from->output_compress_lib, z_stream *);
#endif
 
 if ((to->automatic_term_name_got = from->automatic_term_name_got) &&
     (!to->termcap || (from->termcap != to->termcap)))
 {
  terminal_setup(to, from->termcap->name);
 }

 to->allow_run_commands = from->allow_run_commands;
 to->event = from->event;
}

void socket_player_input(player *p)
{
 input_node *in_node = input_find_current(p);
 size_t read_result = 0;
 char scan[INPUT_BUFFER_SZ];
 
 BTRACE("socket_player_input");
 
 assert(SOCKET_POLL_INDICATOR(p->io_indicator)->revents & POLLIN);

 if (!in_node)
 {
  log_assert(FALSE);
  user_logoff(p, NULL);
  return;
 }

 if (p->input_doing_iac)
 {
  assert(p->input_doing_iac < 4);
  read_result = read_player(p, scan + p->input_doing_iac,
                            INPUT_BUFFER_SZ - p->input_doing_iac);
  if (read_result && ((ssize_t)read_result != -1))
  {
   memcpy(scan, in_node->input + in_node->length, p->input_doing_iac);
   read_result += p->input_doing_iac;
  }
 }
 else
   read_result = read_player(p, scan, INPUT_BUFFER_SZ);

 if (!read_result)
 {
  user_logoff(p, NULL);
  PLAYER_EVENT_UPGRADE(p, BAD_FD);
  return;
 }
 
 if ((ssize_t)read_result == -1)
 {
  int saved_errno = errno;
   /* Note: EINTR shouldn't be possible */
  switch (saved_errno)
  {
   default:
     assert(FALSE);
     user_logoff(p, NULL);
     vwlog("error", "read: %d %s\n", saved_errno, strerror(saved_errno));
     return;
     
     /* FALLTHROUGH */
   case EINTR: /* interupt occured */
   case EAGAIN: /* too little data */
     return;
  }
 }
 
 input_process(p, in_node, scan, read_result);
}

#if 0
struct pollfd *socket_add(int fd)
{
 unsigned long tmp = 0;

 assert(io_indicator_num < (sizeof(io_indicators) / sizeof(io_indicators[0])));

 while (tmp < 256)
 {
  if (io_indicators[tmp].fd == -1)
  {
   ++io_indicator_num;
   
   ++tmp;

   io_indicators[tmp - 1].fd = fd;
   io_indicators[tmp - 1].events = POLLIN;
   io_indicators[tmp - 1].revents = 0;

   assert(tmp != io_indicator_max);
   if (tmp > io_indicator_max)
     io_indicator_max = tmp;
   
   return (&io_indicators[tmp - 1]);
  }
  
  ++tmp;
 }

 assert(FALSE);
 
 return (NULL); 
}

void socket_del(struct pollfd *to_del)
{
 unsigned long offset = (to_del - io_indicators);

 assert(io_indicator_num && io_indicator_num && io_indicator_max);
 
 --io_indicator_num;
 if (to_del->fd != -1)
 {
  close(to_del->fd);
  to_del->fd = -1;
 }

 assert(offset < io_indicator_max);
 if ((offset + 1) == io_indicator_max)
 {
  while (to_del > io_indicators)
  {
   if (to_del->fd != -1)
   {
    io_indicator_max = (to_del - io_indicators) + 1;
    return;
   }
   
   --to_del;
  }

  if (io_indicators[0].fd == -1)
  {
   io_indicator_max = 0;
   assert(!io_indicator_num);
  }
  else
  {
   io_indicator_max = 1;
   assert(io_indicator_num == 1);
  }
 }
}
#endif

static void socket_poll_retval(int retval)
{
 if (retval == -1)
 {
  switch (errno)
  {
   case EINVAL:
     shutdown_error("%s",
                    " Poll got an io_indicator_num that was out of bounds.");
     break;
     
   case EAGAIN:
     SHUTDOWN_MEM_ERR();
     break;
     
   case EFAULT: /* some of the fds were illegal */
     log_assert(FALSE);
   case EINTR: /* interupt */
     break;
     
   default: /* shouldn't happen ? */
     shutdown_error("poll error: %d %s\n", errno, strerror(errno));
  }
  
  return;
 }
}

void socket_update_indicators(int for_time)
{
 socket_poll_retval(socket_poll_update_all(for_time));
}

void socket_update_player(player *p)
{
 if (!p->io_indicator)
   return;

 socket_poll_retval(socket_poll_update_one(p->io_indicator, 0));
}

void socket_main_socket(void)
{
 socket_interface_node *scan = socket_bind_io_start;

 while (scan)
 {
  if (scan->io)
  {
   if (SOCKET_POLL_INDICATOR(scan->io)->revents &
       (POLLERR | POLLHUP | POLLNVAL))
   {
    assert(!(SOCKET_POLL_INDICATOR(scan->io)->revents & POLLNVAL));
    /* errrr... do something better */
    shutdown_error("%s\n", "Problem on main fd.");
   }
   else if (SOCKET_POLL_INDICATOR(scan->io)->revents & POLLIN)
     socket_accept_connection(scan);
  }
  
  scan = scan->next;
 }

 DNS_RESOLVE();
 
 angel_socket_do();

 INTERCOM_POLL();
}

void socket_player_output(player *p)
{ /* This doesn't work "properly" but it works well enough to be used if
   * it's "needed". In short, just don't use it */
 int more_left = 0;

 if (!p->io_indicator)
   return;
 
 socket_update_player(p);
 
 if ((SOCKET_POLL_INDICATOR(p->io_indicator)->revents & POLLOUT) &&
     !(SOCKET_POLL_INDICATOR(p->io_indicator)->revents & (POLLERR | POLLHUP | POLLNVAL)))  
 {
  more_left = output_for_player(p);
  
  output_for_player_cleanup(p);
  
  if (!more_left)
    player_list_io_del(p);
 }
}

void socket_all_players_output(void)
{ /* This doesn't work "properly" but it works well enough to be used if
   * it's "needed". In short, just don't use it */
 player_linked_list *tmp = NULL;
 int more_left = 0;

 socket_update_indicators(1);

 tmp = player_list_io_start();
 while (tmp)
 {
  player_linked_list *tmp_next = PLAYER_LINK_NEXT(tmp);

  if (PLAYER_LINK_GET(tmp)->io_indicator &&
      (SOCKET_POLL_INDICATOR(PLAYER_LINK_GET(tmp)->io_indicator)->revents &
       POLLOUT) &&
      !(SOCKET_POLL_INDICATOR(PLAYER_LINK_GET(tmp)->io_indicator)->revents &
        (POLLERR | POLLHUP | POLLNVAL)))  
  {
   more_left = output_for_player(PLAYER_LINK_GET(tmp));

   output_for_player_cleanup(PLAYER_LINK_GET(tmp));
  
  if (!more_left)
    player_list_io_del(PLAYER_LINK_GET(tmp));
  }
  
  tmp = tmp_next;
 }
}

static int close_down_player_sockets(player_linked_list *passed_scan,
                                     va_list ap __attribute__ ((unused)))
{
 player *scan = PLAYER_LINK_GET(passed_scan);
 
 if (scan->io_indicator)
 {
  int fd = 0;
  
  socket_player_output(scan);
  
  output_list_cleanup(&scan->output_start);
  
  if (player_list_io_find(scan))
    player_list_io_del(scan);

  fd = SOCKET_POLL_INDICATOR(scan->io_indicator)->fd;
  socket_poll_del(scan->io_indicator);
  close(fd);
  scan->io_indicator = 0;
 }
 
 return (TRUE);
}

void socket_close_all(void)
{
 socket_interface_node *scan = socket_bind_io_start;

 INTERCOM_SHUTDOWN();

 DNS_DELETE();

 do_order_misc_on_all(close_down_player_sockets, player_list_cron_start());

 while (scan)
 {
  int fd = 0;
  
  fd = SOCKET_POLL_INDICATOR(scan->io)->fd;
  socket_poll_del(scan->io);
  close(fd);
  scan->io = 0;
  
  scan = scan->next;
 }
}

void user_configure_socket_listen_len(player *p, const char *str)
{
 USER_CONFIGURE_INT_FUNC(socket_listen_len,
                         "Socket", "listen queue size", 1, INT_MAX);

 fvtell_player(NORMAL_T(p), "%s", 
               " This change won't take effect untill you reboot.\n");
}

void socket_interfaces_show(player *p, const char *msg)
{
 configure_interface_node *inter = configure.socket_interfaces_start;
 int count = 0;

 ptell_mid(NORMAL_T(p), msg, FALSE);
 
 while (inter)
 {
  fvtell_player(NORMAL_T(p), " % 2d Port: % 6d Type: ",
                ++count, inter->port);
  
  switch (inter->type)
  {
   case CONFIGURE_INTERFACE_TYPE_ANY:
     fvtell_player(NORMAL_T(p), " Any.\n");
     break;
     
   case CONFIGURE_INTERFACE_TYPE_IPV4:
     fvtell_player(NORMAL_T(p), " Ipv4 address = %d.%d.%d.%d\n",
                   inter->u.ipv4->ip_address[0],
                   inter->u.ipv4->ip_address[1],
                   inter->u.ipv4->ip_address[2],
                   inter->u.ipv4->ip_address[3]);
     break;
     
   case CONFIGURE_INTERFACE_TYPE_IPV6:
     fvtell_player(NORMAL_T(p), " Ipv6 address = \n");
     break;
     
   case CONFIGURE_INTERFACE_TYPE_NAME:
     fvtell_player(NORMAL_T(p), " Dns address = %s\n",
                   inter->u.name->dns_address);
     break;
     
   default:
     assert(FALSE);
  }
  
  inter = inter->next;
 }
 
 fvtell_player(NORMAL_T(p), "%s", DASH_LEN);
}

void user_configure_socket_interfaces(player *p, parameter_holder *params)
{
 configure_interface_node *inter = configure.socket_interfaces_start;
 int count = 0;

 switch (params->last_param)
 {
  case 1:
  case 2:
  case 3:
  case 4:
    break;
    
  default:
    TELL_FORMAT(p, "<cmd> [value(s)]");
 }

 lower_case(GET_PARAMETER_STR(params, 1));
 if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "show"))
 {
  if (params->last_param != 1)
    TELL_FORMAT(p, "<\"show\">");

  socket_interfaces_show(p, "interfaces");
 }
 else if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "add"))
 {
  switch (params->last_param)
  {
   case 3:
   case 4:
     break;
     
   default:
     TELL_FORMAT(p, "<\"add\"> <type> <value(s)>");
  }

  lower_case(GET_PARAMETER_STR(params, 2));
  if (!beg_strcmp(GET_PARAMETER_STR(params, 2), "any"))
  {
   if (params->last_param != 3)
     TELL_FORMAT(p, "<\"add\"> <\"any\"> <port>");

   if ((inter = configure_add_interface(atoi(GET_PARAMETER_STR(params, 3)),
                                        CONFIGURE_INTERFACE_TYPE_ANY)))
   {
    if (inter->port)
      fvtell_player(NORMAL_T(p), " Added an interface on port '^S^B%d^s', "
                    "although you'll need to reboot for this to take effect.\n",
                    inter->port);
    else
      fvtell_player(NORMAL_T(p), "%s", " Added an interface on a dynamic port, "
                    "although you'll need to reboot for this to take effect.\n");
   }
   else
     P_MEM_ERR(p);
 
   configure_save(FALSE);
  }
  else if (!beg_strcmp(GET_PARAMETER_STR(params, 2), "ipv4 address") ||
           !beg_strcmp(GET_PARAMETER_STR(params, 2), "ipv4_address") ||
           !beg_strcmp(GET_PARAMETER_STR(params, 2), "ip address") ||
           !beg_strcmp(GET_PARAMETER_STR(params, 2), "ip_address"))
  {
   unsigned char ips[4];
   
   if (params->last_param != 4)
     TELL_FORMAT(p, "<\"add\"> <\"ip address\"> <address> <port>");

   if (!auth_parse_ip_addr(GET_PARAMETER_STR(params, 3), ips, FALSE))
     TELL_FORMAT(p, "<\"add\"> <\"ip address\"> <address> <port>");
   
   if ((inter = configure_add_interface(atoi(GET_PARAMETER_STR(params, 4)),
                                        CONFIGURE_INTERFACE_TYPE_IPV4)))
   {
    inter->u.ipv4->ip_address[0] = ips[0];
    inter->u.ipv4->ip_address[1] = ips[1];
    inter->u.ipv4->ip_address[2] = ips[2];
    inter->u.ipv4->ip_address[3] = ips[3];
    
    if (inter->port)
      fvtell_player(NORMAL_T(p), " Added an interface on port '^S^B%d^s' with the "
                    "ip address of '^S^B%d.%d.%d.%d^s', "
                    "although you'll need to reboot for this to take effect.\n",
                    inter->port, ips[0], ips[1], ips[2], ips[3]);
    else
      fvtell_player(NORMAL_T(p), " Added an interface on a dynamic port with the "
                    "ip address of '^S^B%d.%d.%d.%d^s', "
                    "although you'll need to reboot for this to take effect.\n",
                    ips[0], ips[1], ips[2], ips[3]);
   }
   else
     P_MEM_ERR(p);
   
   configure_save(FALSE);
  }
  else if (!beg_strcmp(GET_PARAMETER_STR(params, 2), "ipv6 address") ||
           !beg_strcmp(GET_PARAMETER_STR(params, 2), "ipv6_address"))
  {
   fvtell_player(NORMAL_T(p),
                 " This option -- ^S^B%s^s -- is not implemented yet.\n",
                 "ipv6");
   return;
  }
  else if (!beg_strcmp(GET_PARAMETER_STR(params, 2), "name") ||
           !beg_strcmp(GET_PARAMETER_STR(params, 2), "dns address") ||
           !beg_strcmp(GET_PARAMETER_STR(params, 2), "dns_address"))
  {
   if (params->last_param != 4)
     TELL_FORMAT(p, "<\"add\"> <\"dns address\"> <address> <port>");
   
   if ((inter = configure_add_interface(atoi(GET_PARAMETER_STR(params, 4)),
                                        CONFIGURE_INTERFACE_TYPE_NAME)))
   {
    COPY_STR(inter->u.name->dns_address, GET_PARAMETER_STR(params, 3),
             CONFIGURE_DNS_ADDRESS_SZ);
    
    if (inter->port)
      fvtell_player(NORMAL_T(p), " Added an interface on port '^S^B%d^s' with the "
                    "dns address of '^S^B%s^s', "
                    "although you'll need to reboot for this to take effect.\n",
                    inter->port, inter->u.name->dns_address);
    else
      fvtell_player(NORMAL_T(p), " Added an interface on a dynamic port with the "
                    "dns address of '^S^B%s^s', "
                    "although you'll need to reboot for this to take effect.\n",
                    inter->u.name->dns_address);
   }
   else
     P_MEM_ERR(p);
   
   configure_save(FALSE);
  }
  else
    TELL_FORMAT(p, "<\"add\"> <any|ipv4|ipv6|dns address> <value(s)>");
  
 }
 else if (!beg_strcmp(GET_PARAMETER_STR(params, 1), "delete"))
 {
  int sve_count = 0;
  
  if (params->last_param != 2)
    TELL_FORMAT(p, "<\"delete\"> <number>");

  if ((count = atoi(GET_PARAMETER_STR(params, 2))) < 0)
    TELL_FORMAT(p, "<\"delete\"> <number>");
  sve_count = count;
  
  while ((--count > 0) && inter)
    inter = inter->next;

  if (!inter)
  {
   fvtell_player(NORMAL_T(p), " Interface -- ^S^B%d^s -- not found.\n",
                 sve_count);
   return;
  }

  configure_del_interface(inter);

  fvtell_player(NORMAL_T(p), " Delted interfaces number '^S^B%d^s'.\n",
                sve_count);
  
  configure_save(FALSE);
 }
 else
   TELL_FORMAT(p, "<\"show|add|delete\"> [value]");
}
