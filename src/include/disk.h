#ifndef DISK_H
#define DISK_H

#ifndef FILE_IO_DEBUG
# ifndef NDEBUG
#  define FILE_IO_DEBUG 1
# else
#  define FILE_IO_DEBUG 0
# endif
#endif

# ifdef DISK_C

static const char *dummy_suffix_gzip[] = {".gz", ".Z", ".z", NULL};
static const char *dummy_suffix_bzip2[] = {".bz2", NULL};

struct
{
 int working;
 const char *prog_path;
 const char *prog_flags;
 const char **suffix; /* first one in the list is the one to write out in */
} file_io_compress_types[] =
{
 {TRUE, BZIP2_PATH, "-dc", dummy_suffix_gzip},
 {TRUE, GZIP_PATH, "-dc", dummy_suffix_bzip2},
 {FALSE, NULL, NULL, NULL}
};

# endif

#define FILE_IO_MAX_PREFIX_TAGS 8
#define FILE_TAG_MAX_SIZE 32 /* 3 gets added to this for the type info */

typedef struct file_io
{
 union
 {
  FILE *stdio;
#ifdef USE_MMAP
  struct mmap
  {
   char *start;
   char *ptr;
   size_t len;
  } mmap;
#endif
 } io;
 
#if FILE_IO_DEBUG
 char prev_tags[FILE_IO_MAX_PREFIX_TAGS + 1][FILE_TAG_MAX_SIZE + 3];
#endif

 struct
 {
  char tag[FILE_IO_MAX_PREFIX_TAGS + 1][FILE_TAG_MAX_SIZE + 3];
  int len[FILE_IO_MAX_PREFIX_TAGS + 1];
#if FILE_IO_DEBUG
  const char *type[FILE_IO_MAX_PREFIX_TAGS + 1];
#endif
  int used;
 } disk_tags;

 struct
 {
  const char *tag[FILE_IO_MAX_PREFIX_TAGS];
  /* type will always be FILE_TAG_TYPE_OBJECT */
  int used;
 } prefix_tags;

 int version;

 /* 1 struct for read and write... but we only open read only or write only */
 bitflag found_on_disk : 1;
 bitflag read_only : 1;
 bitflag used_popen : 1;
 bitflag used_mmap : 1; /* can't have this and popen at the same time */
} file_io;

#endif
