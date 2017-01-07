#ifndef EXTERN_DISK_H
#define EXTERN_DISK_H

#ifdef DISK_C

# define FILE_IO_NOMMAP_GETC(x) getc((x)->io.stdio)
# define FILE_IO_NOMMAP_READ(x, y, z) do { FILE_IO_NOMMAP_ASSERT(z); \
 (y) = fread(x, sizeof(char), y, (z)->io.stdio); } while (FALSE)
# define FILE_IO_NOMMAP_FEOF(x) feof((x)->io.stdio)
# define FILE_IO_NOMMAP_QFEOF(tmp, x) (tmp == EOF)
# define FILE_IO_NOMMAP_ASSERT(x) do { \
 assert((x) && !(x)->used_mmap); assert((x)->io.stdio); } while (FALSE)
# define FILE_IO_NOMMAP_CLOSE(x) do { int err = 0; FILE_IO_NOMMAP_ASSERT(x); \
 if (ferror((x)->io.stdio)) err = 1; \
 if ((x)->used_popen) return ((pclose((x)->io.stdio) != -1) && !err); \
 else return (!fclose((x)->io.stdio) && !err); } while (FALSE)

#ifdef USE_MMAP
# define FILE_IO_GETC(x) ((int)((x)->used_mmap ? \
 *(x)->io.mmap.ptr++ : FILE_IO_NOMMAP_GETC(x)))
# define FILE_IO_READ(x, y, z) do { FILE_IO_ASSERT(z); \
 if ((z)->used_mmap) { \
  if ((size_t)(((z)->io.mmap.ptr + (y)) - (z)->io.mmap.start) >= (z)->io.mmap.len) \
    (y) = ((z)->io.mmap.len - ((z)->io.mmap.ptr - (z)->io.mmap.start)); \
  memcpy(x, (z)->io.mmap.ptr, y); \
  (z)->io.mmap.ptr += y; \
 } else FILE_IO_NOMMAP_READ(x, y, z); } while (FALSE)
# define FILE_IO_FEOF(x) ((x)->used_mmap ? \
 ((size_t)((x)->io.mmap.ptr - (x)->io.mmap.start) >= (x)->io.mmap.len) : \
  FILE_IO_NOMMAP_FEOF(x))
# define FILE_IO_QFEOF(tmp, x) ((x)->used_mmap ? \
 ((size_t)((x)->io.mmap.ptr - (x)->io.mmap.start) >= (x)->io.mmap.len) : \
  FILE_IO_NOMMAP_QFEOF(tmp, x))
# define FILE_IO_ASSERT(x) do { assert(x); \
 if ((x)->used_mmap) \
  assert((x)->io.mmap.start && (x)->io.mmap.ptr && (x)->io.mmap.len && \
         ((x)->io.mmap.ptr <= ((x)->io.mmap.start + (x)->io.mmap.len))); \
 else FILE_IO_NOMMAP_ASSERT(x); } while (FALSE)
# define FILE_IO_CLOSE(x) do { \
 if ((x)->used_mmap) \
      return (munmap((x)->io.mmap.start, (x)->io.mmap.len) != -1); \
 else FILE_IO_NOMMAP_CLOSE(x); } while (FALSE)
#else
# define FILE_IO_GETC(x) FILE_IO_NOMMAP_GETC(x)
# define FILE_IO_READ(x, y, z) FILE_IO_NOMMAP_READ(x, y, z)
# define FILE_IO_FEOF(x) FILE_IO_NOMMAP_FEOF(x)
# define FILE_IO_QFEOF(tmp, x) FILE_IO_NOMMAP_QFEOF(tmp, x)
# define FILE_IO_ASSERT(x) FILE_IO_NOMMAP_ASSERT(x)
# define FILE_IO_CLOSE(x) FILE_IO_NOMMAP_CLOSE(x)
#endif

# if FILE_IO_DEBUG
#  define FILE_PREV_ASSERT_GR(fs) \
 assert(strcmp(tag, (fs)->prev_tags[(fs)->prefix_tags.used]) > 0)
#  define FILE_PREV_ASSERT_EQ(fs) \
 assert(!strcmp(tag, (fs)->prev_tags[(fs)->prefix_tags.used]))
#  define FILE_TYPE_ASSERT_EQ(tag_type, fs, offset) \
 assert(!strcmp(tag_type, (fs)->disk_tags.type[offset]))
#  define FILE_PREV_NULL(fs) *(fs)->prev_tags[(fs)->prefix_tags.used] = 0
#  define FILE_PREV_COPY(fs, tag) \
    memcpy((fs)->prev_tags[(fs)->prefix_tags.used], tag, strlen(tag) + 1)
# else
#  define FILE_PREV_ASSERT_GR(fs) /* nothing */
#  define FILE_PREV_ASSERT_EQ(fs) /* nothing */
#  define FILE_TYPE_ASSERT_EQ(type, fs, offset) /* nothing */
#  define FILE_PREV_NULL(fs) /* nothing */
#  define FILE_PREV_COPY(fs, tag) /* nothing */
# endif

# define FILE_GET_UNUMBER(x, fs) do { \
 int count = (x); \
 int tmp_num = 0; \
 while (count > 0) { \
 switch (count) { \
 default: \
 case 8: number = (10 * number) + (tmp_num = TONUMB(FILE_IO_GETC(fs))); \
         assert((tmp_num >= 0) && (tmp_num < 10)); --count; \
 case 7: number = (10 * number) + (tmp_num = TONUMB(FILE_IO_GETC(fs))); \
         assert((tmp_num >= 0) && (tmp_num < 10)); --count; \
 case 6: number = (10 * number) + (tmp_num = TONUMB(FILE_IO_GETC(fs))); \
         assert((tmp_num >= 0) && (tmp_num < 10)); --count; \
 case 5: number = (10 * number) + (tmp_num = TONUMB(FILE_IO_GETC(fs))); \
         assert((tmp_num >= 0) && (tmp_num < 10)); --count; \
 case 4: number = (10 * number) + (tmp_num = TONUMB(FILE_IO_GETC(fs))); \
         assert((tmp_num >= 0) && (tmp_num < 10)); --count; \
 case 3: number = (10 * number) + (tmp_num = TONUMB(FILE_IO_GETC(fs))); \
         assert((tmp_num >= 0) && (tmp_num < 10)); --count; \
 case 2: number = (10 * number) + (tmp_num = TONUMB(FILE_IO_GETC(fs))); \
         assert((tmp_num >= 0) && (tmp_num < 10)); --count; \
 case 1: number = (10 * number) + (tmp_num = TONUMB(FILE_IO_GETC(fs))); \
         assert((tmp_num >= 0) && (tmp_num < 10)); --count; \
 case 0: break; } } \
 } while (FALSE)

# define FILE_GET_NUMBER(x, fs) do { \
 int sign_char = FILE_IO_GETC(fs); \
 if (sign_char != '-') { assert(isdigit(sign_char)); \
  number = (10 * number) + TONUMB(sign_char); \
  FILE_GET_UNUMBER((x) - 1, fs); } \
 else { \
 int count = ((x) - 1); \
 while (count > 0) { \
 switch (count) { \
 default: \
 case 8: number = (10 * number) - TONUMB(FILE_IO_GETC(fs)); --count; \
 case 7: number = (10 * number) - TONUMB(FILE_IO_GETC(fs)); --count; \
 case 6: number = (10 * number) - TONUMB(FILE_IO_GETC(fs)); --count; \
 case 5: number = (10 * number) - TONUMB(FILE_IO_GETC(fs)); --count; \
 case 4: number = (10 * number) - TONUMB(FILE_IO_GETC(fs)); --count; \
 case 3: number = (10 * number) - TONUMB(FILE_IO_GETC(fs)); --count; \
 case 2: number = (10 * number) - TONUMB(FILE_IO_GETC(fs)); --count; \
 case 1: number = (10 * number) - TONUMB(FILE_IO_GETC(fs)); --count; \
 case 0: break; } } } \
 } while (FALSE)


#endif

#define FILE_TAG_TYPE_USTRING "us"
#define FILE_TAG_TYPE_SSTRING "ss"
#define FILE_TAG_TYPE_STRING FILE_TAG_TYPE_SSTRING

#define FILE_TAG_TYPE_SHORT "sh"
#define FILE_TAG_TYPE_USHORT "uh"

#define FILE_TAG_TYPE_INT "si"
#define FILE_TAG_TYPE_UINT "ui"

#define FILE_TAG_TYPE_LONG "sl"
#define FILE_TAG_TYPE_ULONG "ul"

#define FILE_TAG_TYPE_BITFLAG "bf"

#define FILE_TAG_TYPE_DOUBLE "dl"

#define FILE_TAG_TYPE_TIMESTAMP "ti"

#define FILE_TAG_TYPE_OBJECT "ob" /* for start_section etc... */


#define FILE_IO_VERSION(x) ((x)->version)
#define FILE_IO_CREATED(x) (!(x)->found_on_disk)


extern void file_ops_setup(file_io *);
extern void file_section_beg(const char *, file_io *);
extern void file_section_end(const char *, file_io *);

/* input */

extern int file_read_open(const char *, file_io *);
extern int file_read_close(file_io *);

extern size_t file_get_string(const char *, char *, size_t, file_io *);
extern void *file_get_malloc(const char *, size_t *, file_io *);
extern unsigned short file_get_unsigned_short(const char *, file_io *);
extern short file_get_short(const char *, file_io *);
extern unsigned int file_get_unsigned_int(const char *, file_io *);
extern int file_get_int(const char *, file_io *);
extern unsigned long file_get_unsigned_long(const char *, file_io *);
extern long file_get_long(const char *, file_io *);
extern int file_get_bitflag(const char *, file_io *);
extern double file_get_double(const char *, file_io *);
extern time_t file_get_time_t(const char *, file_io *);

/* output */

extern int file_write_open(const char *, int, file_io *);
extern int file_write_close(file_io *);

extern void file_put_string(const char *, const char *, size_t, file_io *);
extern void file_put_unsigned_short(const char *, unsigned int, file_io *);
extern void file_put_short(const char *, int, file_io *);
extern void file_put_unsigned_int(const char *, unsigned int, file_io *);
extern void file_put_int(const char *, int, file_io *);
extern void file_put_unsigned_long(const char *, unsigned long, file_io *);
extern void file_put_long(const char *, long, file_io *);
extern void file_put_bitflag(const char *, int, file_io *);
extern void file_put_double(const char *, double, file_io *);
extern void file_put_time_t(const char *, time_t, file_io *);

/* extra */
extern void file_copy_io(file_io *, file_io *, unsigned int);

#endif
