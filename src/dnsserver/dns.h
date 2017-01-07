#ifndef DNS_H
#define DNS_H

extern time_t now;

#define MAX_PARALLELISM 16
#define MAX_DNS_TIMEOUT MK_MINUTES(4)
#define MAX_SERVER_TIMEOUT MK_MINUTES(8)

#define MAX_INET_ADDRESS ((3 * 4) + 4)

typedef struct dns_request
{
 char inet_address[MAX_INET_ADDRESS];
 child_com *child;
} dns_request;


#endif
