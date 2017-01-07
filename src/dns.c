#define DNS_C
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

#ifdef USE_DNS_SERVER

static char *dns_argv[2] = {NULL, NULL};
static char dns_ps_name[sizeof("-=> %s <=- Dns Server") + CONFIGURE_NAME_SZ];

static child_com *dns_server = NULL;

static void internal_dns_add(const unsigned char *ip_address, int reboot)
{
 if (reboot && !dns_server)
 {
  sprintf(dns_ps_name, "-=> %s <=- Dns Server", configure.name_long);
  if ((dns_server = child_com_create("bin/dns_server", dns_argv)))
  {
   dns_server->io_indicator = socket_poll_add(dns_server->input);
   SOCKET_POLL_INDICATOR(dns_server->io_indicator)->events |= POLLIN;
  }
 }

 if (dns_server)
 {  
  child_com_send(dns_server, "%d.%d.%d.%d\n",
                 ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
  if (!child_com_flush(dns_server))
  {
   socket_poll_del(dns_server->io_indicator);
   child_com_delete(dns_server);
   dns_server = NULL;
   
   internal_dns_add(ip_address, FALSE);
  }
 }
}


void dns_add(const unsigned char *ip_address)
{
 BTRACE("dns_add");
 internal_dns_add(ip_address, TRUE);
}

static int internal_dns_resolve(player_linked_list *passed_scan, va_list ap)
{
 const unsigned char *ips = va_arg(ap, const unsigned char *);
 const char *name = va_arg(ap, const char *);
 player *scan = PLAYER_LINK_GET(passed_scan);

 
 if (!memcmp(scan->ip_address, ips, 4) && strcmp(scan->dns_address, name))
 {
  COPY_STR(scan->dns_address, name, SOCKET_DNS_ADDRESS_SZ);
  
  if (scan->is_fully_on) /* TODO: have another channel ? -- for inform too */
    channels_wall("staff", 3, NULL, " -=> %-*s dns has been resolved: %s",
                  PLAYER_S_NAME_SZ, scan->saved->name, scan->dns_address);
  else
    logon_shortcut_logon_start(scan);
 }
 
 return (TRUE);
}

void dns_resolve(void)
{
 size_t chars_toget = 0;
 
 if (!dns_server)
   return;
 
 chars_toget = child_com_gather(dns_server, '\n');
  
 if (dns_server->bad ||
     child_com_waiting_input(dns_server, DNS_SERVER_TIMEOUT))
 {
  socket_poll_del(dns_server->io_indicator);
  child_com_delete(dns_server);
  dns_server = NULL;
  return;
 } 

 if (chars_toget)
 {
  char buffer[1024];
  char *name = NULL;
  unsigned char ips[4];
  
  if (child_com_recv(dns_server, buffer, chars_toget) == chars_toget)
    buffer[chars_toget - 1] = 0;
  else
  {
   assert(FALSE);
  }
  
  if ((name = N_strchr(buffer, ':')))
  {
   *name++ = 0;
   
   /* TODO: change to a seperate list for speed */
   if (strcmp(buffer, name) && auth_parse_ip_addr(buffer, ips, FALSE))
     do_order_misc_on_all(internal_dns_resolve, player_list_cron_start(),
                          ips, name);
  }
  else
  {
   socket_poll_del(dns_server->io_indicator);
   child_com_delete(dns_server);
   dns_server = NULL;
  }
 }
 else if (dns_server->bad)
 {
  socket_poll_del(dns_server->io_indicator);
  child_com_delete(dns_server);
  dns_server = NULL;
 }
}

void dns_delete(void)
{
 if (dns_server)
 {
  child_com_delete(dns_server);
  dns_server = NULL;
 }
}

void init_dns(void)
{
 const unsigned char ip_address[4] = {127, 0, 0, 1};

 dns_argv[0] = dns_ps_name;
 
 dns_add(ip_address);
}

#endif
