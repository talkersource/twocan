#ifndef EXTERN_DNS_H
#define EXTERN_DNS_H

#ifdef USE_DNS_SERVER
extern void dns_add(const unsigned char *);
extern void dns_resolve(void);
extern void dns_delete(void);
extern void init_dns(void);

# define DNS_ADD(x) dns_add(x)
# define DNS_RESOLVE() dns_resolve()
# define DNS_DELETE() dns_delete()
# define INIT_DNS() init_dns()
#else
# define DNS_ADD(x) IGNORE_PARAMETER(x)
# define DNS_RESOLVE() do { } while (FALSE)
# define DNS_DELETE() do { } while (FALSE)
# define INIT_DNS() do { } while (FALSE)
#endif

#endif
