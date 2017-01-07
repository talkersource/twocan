#ifndef EXTERN_OUTPUT_COMPRESS_H
#define EXTERN_OUTPUT_COMPRESS_H

#ifdef HAVE_ZLIB_H
# define OUTPUT_COMPRESS_MEM(x, y) ((x) -> y)
#else
# define OUTPUT_COMPRESS_MEM(x, y) 0
#endif

extern int output_compress_start_1(player *);
extern void output_compress_start_2(player *);
extern int output_compress_stop_1(player *);
extern void output_compress_stop_2(player *);

extern int output_compress_writev_1(player *, struct iovec *, int, int);
extern int output_compress_writev_2(player *);

#endif
    
