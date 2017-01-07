#define DISK_C
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

/* NOTE: you'll have to convert if your OS isn't using the same char rep.
 * usually ASCII */

#include "main.h"

/* forward reference */
static int check_entry_tag(const char *, file_io *);


static void setup_for_entry(file_io *fs)
{ 
 fs->disk_tags.used = 0;
}

static void file_setup_file_io(file_io *fs)
{
 fs->io.stdio = NULL;
 fs->read_only = TRUE;
 fs->used_popen = FALSE;
 fs->used_mmap = FALSE;
 fs->prefix_tags.used = 0;
 FILE_PREV_NULL(fs);
   
 setup_for_entry(fs);
}

void file_section_beg(const char *tag, file_io *fs)
{
 FILE_IO_ASSERT(fs);
 assert(fs->prefix_tags.used < FILE_IO_MAX_PREFIX_TAGS);
 assert(!C_strchr(tag, '.') && !C_strchr(tag, ':'));
 assert(strlen(tag) < FILE_TAG_MAX_SIZE);
 FILE_PREV_ASSERT_GR(fs);
 
 FILE_PREV_COPY(fs, tag);

 if (fs->prefix_tags.used < FILE_IO_MAX_PREFIX_TAGS)
 {
  fs->prefix_tags.tag[fs->prefix_tags.used++] = tag;
  FILE_PREV_NULL(fs);
 }
 else
 {
  assert(FALSE);
 }
}

void file_section_end(const char *tag, file_io *fs)
{
 FILE_IO_ASSERT(fs);
 assert(fs->prefix_tags.used > 0);
 assert(!C_strchr(tag, '.') && !C_strchr(tag, ':'));
 assert(strlen(tag) < FILE_TAG_MAX_SIZE);
 
 if (fs->prefix_tags.used > 0)
 {
  --fs->prefix_tags.used;
  
  FILE_PREV_ASSERT_EQ(fs);
 }
}



/* input stuff .... */






static void finnish_get_entry(file_io *fs)
{
 int tmp = FILE_IO_GETC(fs);
 
 FILE_IO_ASSERT(fs);
 
 assert(tmp == '\n');
 
 setup_for_entry(fs);
}

static size_t get_entry_length(file_io *fs)
{
 int tmp = FILE_IO_GETC(fs);
 size_t length = 0;
 
 FILE_IO_ASSERT(fs);
 
 while (!FILE_IO_QFEOF(tmp, fs) && isdigit(tmp))
 {
  length = (length * 10) + TONUMB(tmp);
  tmp = FILE_IO_GETC(fs);
 }
 assert(tmp == ':');
 
 return (length);
}

static void skip_file_length(size_t length, file_io *fs)
{
#if 0 /* ? use fseek, will popen work with it ? */
  char buffer[512];

  FILE_IO_ASSERT(fs);
  
  if (fs->used_mmap)
  {
   fs->io.mmap.ptr += length;
   FILE_IO_ASSERT(fs);
  }
  else
  {
   while ((length > 512) && !FILE_IO_FEOF(fs))
     length -= fread(buffer, sizeof(char), 512, fs->io.stdio);
   
   fread(buffer, sizeof(char), length, fs->io.stdio);
  }
#else
  FILE_IO_ASSERT(fs);
  
#ifdef USE_MMAP
  /* massively quicker, funnily enough */
  if (fs->used_mmap)
  {
   fs->io.mmap.ptr += length;
   FILE_IO_ASSERT(fs);
  }
  else
#endif
    fseek(fs->io.stdio, length, SEEK_CUR);
#endif
}

static void skip_entry(file_io *fs)
{
 int length = get_entry_length(fs);
 
 skip_file_length(length, fs);
 finnish_get_entry(fs);
}

static void skip_rest_tag(file_io *fs)
{ /* use memchr for mmap */
 int tmp = FILE_IO_GETC(fs);
 
 while (!FILE_IO_QFEOF(tmp, fs) && (tmp != ':'))
   tmp = FILE_IO_GETC(fs);
 assert(tmp == ':');
}

#if FILE_IO_DEBUG
static void seperate_disk_tag_type(file_io *fs)
{
 int disk_tag = fs->disk_tags.used;
 int length = fs->disk_tags.len[disk_tag];
 char *tmp = NULL;
 
 assert(length < FILE_TAG_MAX_SIZE);

 if (length > 1)
 {
  tmp = fs->disk_tags.tag[disk_tag];
  tmp[length + 1] = 0;
  tmp[length] = tmp[length - 1];
  tmp[length - 1] = tmp[length - 2];
  tmp[length - 2] = 0;
  
  fs->disk_tags.type[disk_tag] = (tmp + length - 1);
 }
 else
 {
  assert(FILE_IO_FEOF(fs));
 }
}
#else
/* remove the type from the disk tag */
# define seperate_disk_tag_type(fs) do { \
 if ((fs)->disk_tags.len[(fs)->disk_tags.used] > 1) \
  (fs)->disk_tags.tag[(fs)->disk_tags.used] \
                     [(fs)->disk_tags.len[(fs)->disk_tags.used] - 2] = 0; \
 } while (FALSE)
#endif

static int get_entry_next_tag(char *disk_tags, int *disk_tags_len,
                              int *current_char, file_io *fs)
{
 char *tmp = disk_tags;
 
 FILE_IO_ASSERT(fs);
 
 if (FILE_IO_QFEOF(*current_char, fs))
   return (FALSE);

 while ((*current_char != '.') && (*current_char != ':'))
 {
  assert(!FILE_IO_QFEOF(*current_char, fs) &&
         ((tmp - disk_tags) < (FILE_TAG_MAX_SIZE + 2)));

  if (FILE_IO_DEBUG && FILE_IO_QFEOF(*current_char, fs))
    return (FALSE);

  *tmp++ = *current_char;
  *current_char = FILE_IO_GETC(fs);
 }
 
 *tmp = 0;
 *disk_tags_len = (tmp - disk_tags);

 return (TRUE);
}

static int get_entry_tag(const char *tag, const char *tag_type, file_io *fs)
{
 int saved_cmp = 0;
 int load_tag_return_false = FALSE;
 
 FILE_IO_ASSERT(fs);
 assert(fs->read_only);
 assert(strlen(tag) < FILE_TAG_MAX_SIZE);
 FILE_PREV_ASSERT_GR(fs);

 FILE_PREV_COPY(fs, tag);

 if (fs->disk_tags.used)
 {
  if (!(saved_cmp = check_entry_tag(tag, fs)))
  {
   FILE_TYPE_ASSERT_EQ(tag_type, fs, fs->disk_tags.used - 1);
   return (fs->found_on_disk = TRUE);
  }
  else if (saved_cmp < 0)
    return (fs->found_on_disk = FALSE);
  else
    skip_entry(fs);
 }
 
 restart_get_entry_tag:

#ifndef USE_MMAP
 ungetc(FILE_IO_NOMMAP_GETC(fs), fs->io.stdio);
#endif

 if (FILE_IO_FEOF(fs))
   return (fs->found_on_disk = FALSE);
 
 while (TRUE)
 {
  char *disk_tags = fs->disk_tags.tag[fs->disk_tags.used];
  int *disk_tags_len = (fs->disk_tags.len + fs->disk_tags.used);
  int tmp = FILE_IO_GETC(fs);
  
  *disk_tags_len = 0;
  
  if (!get_entry_next_tag(disk_tags, disk_tags_len, &tmp, fs))
  {
   if (fs->disk_tags.used)
   {
    log_assert(FALSE); /* File corruption, hit EOF in the middle of a tag! */
    fs->disk_tags.used = 0;
   }
   return (fs->found_on_disk = FALSE);
  }
  
  seperate_disk_tag_type(fs);
  
  if (!load_tag_return_false)
  {
   const char *want_tag = NULL;
   
   if (fs->disk_tags.used < fs->prefix_tags.used)
     want_tag = fs->prefix_tags.tag[fs->disk_tags.used];
   else if (fs->disk_tags.used == fs->prefix_tags.used)
     want_tag = tag;
   
   if ((saved_cmp = strcmp(want_tag, disk_tags)) > 0)
   {
    if (tmp == '.')
      skip_rest_tag(fs);
    else
    {
     assert(tmp == ':');
    }
    
    skip_entry(fs);
    goto restart_get_entry_tag;
   }
   else if (saved_cmp < 0)
     load_tag_return_false = TRUE;
  }
  
  if (tmp == '.')
  {
   FILE_TYPE_ASSERT_EQ(FILE_TAG_TYPE_OBJECT, fs, fs->disk_tags.used);
   
   ++fs->disk_tags.used;
  }
  else
  {
   assert(tmp == ':');
   break;
  }
 }

 assert((fs->disk_tags.used == fs->prefix_tags.used) ||
        load_tag_return_false);
 
 if (load_tag_return_false)
 {
  ++fs->disk_tags.used;
  return (fs->found_on_disk = FALSE);
 }
 else
   if (saved_cmp > 0)
   {
    skip_entry(fs);
    goto restart_get_entry_tag;
   }

 FILE_TYPE_ASSERT_EQ(tag_type, fs, fs->disk_tags.used);
 ++fs->disk_tags.used;
 return (fs->found_on_disk = TRUE);
}

static int check_entry_tag(const char *tag, file_io *fs)
{
 int tmp = 0;
 int saved_cmp = 0;
 
 while (tmp < fs->prefix_tags.used)
 {
  if ((saved_cmp = strcmp(fs->prefix_tags.tag[tmp], fs->disk_tags.tag[tmp])))
    return (saved_cmp);
  
  ++tmp;
 }

 return (strcmp(tag, fs->disk_tags.tag[tmp]));
}

int file_read_open(const char *file_name, file_io *fs)
{
 struct stat the_file;
 
 if (!stat(file_name, &the_file) && S_ISREG(the_file.st_mode))
 {
  int fd = 0;
  
  file_setup_file_io(fs);
#ifdef USE_MMAP
  fs->used_mmap = TRUE;
  fs->io.mmap.len = the_file.st_size;
  if ((fd = open(file_name, O_RDONLY)) != -1)
  {
   if ((fs->io.mmap.ptr = mmap(NULL, the_file.st_size,
                               PROT_READ, MAP_SHARED, fd, 0)) != MAP_FAILED)
   {
    fs->io.mmap.start = fs->io.mmap.ptr;
    close(fd);
    /* ascii 001 is SOH (start of header) which seems apropriate :) */
    fs->version = file_get_int("\001version", fs);
    return (TRUE);
   }
   else
   {
    close(fd);
   }
  }
#else
  if ((fs->io.stdio = fopen(file_name, "rb")))
  {
   IGNORE_PARAMETER(fd);
   
   /* ascii 001 is SOH (start of header) which seems apropriate :) */
   fs->version = file_get_int("\001version", fs);
   return (TRUE);
  }
#endif
 }
 else
 {
  int skip_prog = 0;
  int prefix_count = 0;

  while (file_io_compress_types[prefix_count].prog_path)
  {
   char prog_name[1024 + PATH_MAX];
   int suffix_count = 0;
   size_t len = 0;

   if (!file_io_compress_types[prefix_count].working)
   {
    ++prefix_count;
    continue;
   }
   
   len = sprintf(prog_name, "%s%s%n%.*s",
                 file_io_compress_types[prefix_count].prog_path,
                 file_io_compress_types[prefix_count].prog_flags,
                 &skip_prog,
                 PATH_MAX - 1, file_name);
   
   while (file_io_compress_types[prefix_count].suffix[suffix_count])
   {
    strcpy(prog_name + len,
           file_io_compress_types[prefix_count].suffix[suffix_count]);

    if (!stat(prog_name + skip_prog, &the_file) && S_ISREG(the_file.st_mode))
    {
     FILE *fp = popen(prog_name, "r");

     if (!fp)
     {
      file_io_compress_types[prefix_count].working = FALSE;
      break; /* out of while loop,
                Ie. assume file_io_compress_types[prefix_count]
                is borken */
     }
     
     file_setup_file_io(fs);
     fs->io.stdio = fp;
     fs->used_popen = TRUE;
     fs->version = file_get_int("\001version", fs);
     return (TRUE);
    }

    ++suffix_count;
   }

   ++prefix_count;
  }
 }
 
 return (FALSE);
}

int file_read_close(file_io *fs)
{
 FILE_IO_ASSERT(fs);
 assert(!fs->prefix_tags.used);

 FILE_IO_CLOSE(fs);

 return (TRUE);
}

size_t file_get_string(const char *tag, char *buffer, size_t size, file_io *fs)
{
 assert(size > 0);
 
 if (get_entry_tag(tag, FILE_TAG_TYPE_STRING, fs))
 {
  size_t length;
  size_t entry_length = length = get_entry_length(fs);

  if (length >= size)
    length = size - 1;

  FILE_IO_READ(buffer, length, fs);
  buffer[length] = 0;

  if (!FILE_IO_FEOF(fs))
  {
   if (entry_length > length)
     skip_file_length(entry_length - length, fs);
   finnish_get_entry(fs);
  }
  
  return (length);
 }

 buffer[0] = 0;
 return (0);
}

void *file_get_malloc(const char *tag, size_t *return_length, file_io *fs)
{
 if (get_entry_tag(tag, FILE_TAG_TYPE_STRING, fs))
 {
  size_t length = get_entry_length(fs);
  char *new_str = MALLOC(length + 1);

  if (new_str)
  {
   if (return_length)
     *return_length = length;

   FILE_IO_READ(new_str, length, fs);
   new_str[length] = 0;
   
   if (!FILE_IO_FEOF(fs))
     finnish_get_entry(fs);
  }
  else
  {
   if (return_length)
     *return_length = 0;

   if (!FILE_IO_FEOF(fs))
   {
    skip_file_length(length, fs);
    finnish_get_entry(fs);
   }
  }
    
  return (new_str);
 }

 return (calloc(1, 1));
}

unsigned short file_get_unsigned_short(const char *tag, file_io *fs)
{
 if (get_entry_tag(tag, FILE_TAG_TYPE_USHORT, fs))
 {
  int length = get_entry_length(fs);
  unsigned long number = 0;

  FILE_GET_UNUMBER(length, fs);

  finnish_get_entry(fs);

  assert(number <= USHRT_MAX);
  return (number);
 }

 return (0);
}

short file_get_short(const char *tag, file_io *fs)
{
 if (get_entry_tag(tag, FILE_TAG_TYPE_SHORT, fs))
 {
  int length = get_entry_length(fs);
  long number = 0;
  
  FILE_GET_NUMBER(length, fs);

  finnish_get_entry(fs);

  assert((number <= SHRT_MAX) && (number >= SHRT_MIN));
  return (number);
 }

 return (0);
}

unsigned int file_get_unsigned_int(const char *tag, file_io *fs)
{
 if (get_entry_tag(tag, FILE_TAG_TYPE_UINT, fs))
 {
  int length = get_entry_length(fs);
  unsigned long number = 0;

  FILE_GET_UNUMBER(length, fs);

  finnish_get_entry(fs);

  assert(number <= UINT_MAX);
  return (number);
 }

 return (0);
}

int file_get_int(const char *tag, file_io *fs)
{
 if (get_entry_tag(tag, FILE_TAG_TYPE_INT, fs))
 {
  int length = get_entry_length(fs);
  long number = 0;
  
  FILE_GET_NUMBER(length, fs);

  finnish_get_entry(fs);

  assert((number <= INT_MAX) && (number >= INT_MIN));
  return (number);
 }

 return (0);
}

unsigned long file_get_unsigned_long(const char *tag, file_io *fs)
{
 if (get_entry_tag(tag, FILE_TAG_TYPE_ULONG, fs))
 {
  int length = get_entry_length(fs);
  unsigned long number = 0;
  
  FILE_GET_UNUMBER(length, fs);
  
  finnish_get_entry(fs);

  return (number);
 }

 return (0);
}

long file_get_long(const char *tag, file_io *fs)
{
 if (get_entry_tag(tag, FILE_TAG_TYPE_LONG, fs))
 {
  int length = get_entry_length(fs);
  long number = 0;
  
  FILE_GET_NUMBER(length, fs);
  
  finnish_get_entry(fs);
  
  return (number);
 }

 return (0);
}

int file_get_bitflag(const char *tag, file_io *fs)
{ 
 if (get_entry_tag(tag,  FILE_TAG_TYPE_BITFLAG, fs))
 {
  char bit;
#if FILE_IO_DEBUG
  int length = get_entry_length(fs);
  
  assert(length == 1);
#else
  FILE_IO_GETC(fs); /* == 1 */
  FILE_IO_GETC(fs); /* == : */
#endif
  bit = FILE_IO_GETC(fs);
  
  finnish_get_entry(fs);

  assert((bit == '1') || (bit == '0'));
  return (bit == '1');
 }
 
 return (0);
}

double file_get_double(const char *tag, file_io *fs)
{
 if (get_entry_tag(tag, FILE_TAG_TYPE_DOUBLE, fs))
 {
  int length = get_entry_length(fs);
#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
  char buffer[length + 1];
#else
  char buffer[1024]; /* should be long enough for any _NUMBER_ */

  assert(length < 1024);

#endif

  FILE_IO_READ(buffer, length, fs);
  buffer[length] = 0;

  finnish_get_entry(fs);
  
  return (strtod(buffer, NULL));
 }

 return (0);
}

time_t file_get_time_t(const char *tag, file_io *fs)
{
 if (get_entry_tag(tag,  FILE_TAG_TYPE_TIMESTAMP, fs))
 {
  int length = get_entry_length(fs);
#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
  char buffer[length + 1];
#else
  char buffer[1024]; /* should be long enough for any _NUMBER_ */
  
  assert(length < 1024);
  
#endif

  FILE_IO_READ(buffer, length, fs);
  buffer[length] = 0;
  
  finnish_get_entry(fs);

  return (disp_time_string_cmp(buffer));
 }

 return (now);
}





/* output stuff .... */



/* NOTE: even though we don't store the tag type for reading, we still
 * _NEED_ to have the tag type on disk */
static void put_entry_tag(const char *tag, const char *tag_type, file_io *fs)
{
 int tmp = 0;

 FILE_IO_NOMMAP_ASSERT(fs);
 assert(!fs->read_only);
 assert(strlen(tag) < FILE_TAG_MAX_SIZE);
 FILE_PREV_ASSERT_GR(fs);

 FILE_PREV_COPY(fs, tag);
 
 while (tmp < fs->prefix_tags.used)
   fprintf(fs->io.stdio, "%s%s.",
           fs->prefix_tags.tag[tmp++], FILE_TAG_TYPE_OBJECT);

 fprintf(fs->io.stdio, "%s%s:", tag, tag_type);
}

static void finnish_put_entry(const char *buffer, size_t length, file_io *fs)
{
 FILE_IO_NOMMAP_ASSERT(fs);
 log_assert(buffer);
 assert(!length || (length == strlen(buffer)));
 
 if (!length)
   length = strlen(buffer);
 fprintf(fs->io.stdio, "%lu:%s\n", (unsigned long)length, buffer);
}

int file_write_open(const char *file_name, int file_version, file_io *fs)
{
 file_setup_file_io(fs);

 if (!(fs->io.stdio = fopen(file_name, "wb")))
   return (FALSE);
 
 fs->read_only = FALSE;
 file_put_int("\001version", file_version, fs);
 return (TRUE);
}

int file_write_close(file_io *fs)
{
 FILE_IO_NOMMAP_ASSERT(fs);
 assert(!fs->prefix_tags.used);

 FILE_IO_NOMMAP_CLOSE(fs);
}

void file_put_string(const char *tag, const char *str, size_t length,
                     file_io *fs)
{
 put_entry_tag(tag, FILE_TAG_TYPE_STRING, fs);
 finnish_put_entry(str, length, fs);
}

void file_put_unsigned_short(const char *tag, unsigned int to_put, file_io *fs)
{
 char buffer[BUF_NUM_TYPE_SZ(unsigned short)];
 size_t length = 0;
 
 put_entry_tag(tag, FILE_TAG_TYPE_USHORT, fs);

 length = sprintf(buffer, "%hu", to_put);
 
 finnish_put_entry(buffer, length, fs);
}

void file_put_short(const char *tag, int to_put, file_io *fs)
{
 char buffer[BUF_NUM_TYPE_SZ(short)];
 size_t length = 0;
 
 put_entry_tag(tag, FILE_TAG_TYPE_SHORT, fs);

 length = sprintf(buffer, "%hd", to_put);
 
 finnish_put_entry(buffer, length, fs);
}

void file_put_unsigned_int(const char *tag, unsigned int to_put, file_io *fs)
{
 char buffer[BUF_NUM_TYPE_SZ(unsigned int)];
 size_t length = 0;
 
 put_entry_tag(tag, FILE_TAG_TYPE_UINT, fs);

 length = sprintf(buffer, "%u", to_put);
 
 finnish_put_entry(buffer, length, fs);
}

void file_put_int(const char *tag, int to_put, file_io *fs)
{
 char buffer[BUF_NUM_TYPE_SZ(int)];
 size_t length = 0;
 
 put_entry_tag(tag, FILE_TAG_TYPE_INT, fs);

 length = sprintf(buffer, "%d", to_put);
 
 finnish_put_entry(buffer, length, fs);
}

void file_put_unsigned_long(const char *tag, unsigned long to_put, file_io *fs)
{
 char buffer[BUF_NUM_TYPE_SZ(unsigned long)];
 size_t length = 0;
 
 put_entry_tag(tag, FILE_TAG_TYPE_ULONG, fs);

 length = sprintf(buffer, "%lu", to_put);
 
 finnish_put_entry(buffer, length, fs);
}

void file_put_long(const char *tag, long to_put, file_io *fs)
{
 char buffer[BUF_NUM_TYPE_SZ(long)];
 size_t length = 0;

 put_entry_tag(tag, FILE_TAG_TYPE_LONG, fs);

 length = sprintf(buffer, "%ld", to_put);
 
 finnish_put_entry(buffer, length, fs);
}

void file_put_bitflag(const char *tag, int to_put, file_io *fs)
{
 char buffer[2] = "1";
 
 assert((to_put == TRUE) || (to_put == FALSE));

 put_entry_tag(tag, FILE_TAG_TYPE_BITFLAG, fs);

 if (!to_put)
   buffer[0] = '0';
 
 finnish_put_entry(buffer, 1, fs);
}

void file_put_double(const char *tag, double to_put, file_io *fs)
{
 char buffer[1024];
 size_t length = 0;
 
 put_entry_tag(tag, FILE_TAG_TYPE_DOUBLE, fs);

 /* not sure how good we want this, but 32 should be ok */
 length = sprintf(buffer, "%.32f", to_put);
 
 finnish_put_entry(buffer, length, fs);
}

void file_put_time_t(const char *tag, time_t to_put, file_io *fs)
{
 put_entry_tag(tag, FILE_TAG_TYPE_TIMESTAMP, fs); 
 finnish_put_entry(disp_time_cmp_string(to_put), 0, fs);
}

/* extra stuff... */
/* not logical ordering, apart from to a C programer
 * think strcpy and not cp :*/
void file_copy_io(file_io *to, file_io *from, unsigned int objects)
{
 FILE_IO_ASSERT(from);
 FILE_IO_NOMMAP_ASSERT(to);
 assert(from->read_only);
 assert(!to->read_only);
 assert(objects == UINT_MAX); /* I haven't written it for by object copies */

 while (!FILE_IO_FEOF(from))
 {
  char buffer[1024];
  size_t read_data = 1024;

  FILE_IO_READ(buffer, read_data, from);

  if (read_data)
    fwrite(buffer, sizeof(char), read_data, to->io.stdio);
 }
}
