#define TEST_DISK_C
#define DISK_C
#define DISPLAY_TIME_C

#include "main.h"

#include "../disk.c"
#include "../display_time.c"

time_t now;

int skip_atoi(const char **input)
{
 int tmp = 0;
 
 while (isdigit((unsigned char) **input))
   tmp = (tmp * 10) + TONUMB(*((*input)++));
 
 return (tmp);
}

long int skip_atol(const char **input)
{
 long int tmp = 0;
 
 while (isdigit((unsigned char) **input))
   tmp = (tmp * 10) + TONUMB(*((*input)++));
 
 return (tmp);
}

static void do_write(file_io *fs, int changer)
{
 char buffer[1024];
 sprintf(buffer, "this is my string.\n Isn't it: %d.\n", changer);
 file_put_string("001", buffer, fs);
 file_put_unsigned_int("002", 4000000000U + changer, fs);
 file_put_int("004", 2000000000 + changer, fs);
 file_put_unsigned_long("008", 4000000000UL + changer, fs);
 file_put_long("009", 2000000000L + changer, fs);
 file_put_bitflag("020", FALSE, fs);
 file_put_bitflag("022", TRUE, fs);
 file_put_double("023", 3.14569 + changer, fs);
 file_put_time_t("024", now + changer, fs);
}

static void do_read(file_io *fs, int skip_beg)
{
 if (!skip_beg)
 {
  char buffer[1024];
  file_get_string("001", buffer, 1023, fs);
  printf(" str     = %s\n", buffer);
  assert(!FILE_IO_CREATED(fs));
  printf(" uint    = %u\n", file_get_unsigned_int("002", fs));
  assert(!FILE_IO_CREATED(fs));
  printf(" int     = %d\n", file_get_int("004", fs));
  assert(!FILE_IO_CREATED(fs));
 }
 
 printf(" ulong   = %lu\n", file_get_unsigned_long("008", fs));
 assert(!FILE_IO_CREATED(fs));
 printf(" long    = %ld\n", file_get_long("009", fs));
 assert(!FILE_IO_CREATED(fs));
 printf(" bitflag = %d\n", file_get_bitflag("020", fs));
 assert(!FILE_IO_CREATED(fs));
 printf(" bitflag = %d\n", file_get_bitflag("022", fs));
 assert(!FILE_IO_CREATED(fs));
 printf(" double  = %f\n", file_get_double("023", fs));
 assert(!FILE_IO_CREATED(fs));
 printf(" time    = %d (%d)\n", file_get_time_t("024", fs), now);
 assert(!FILE_IO_CREATED(fs));
}

static void simple_test(void)
{
 file_io fs;

 file_write_open("abcd", 1, &fs);
 do_write(&fs, 2020);
 file_write_close(&fs);

 file_read_open("abcd", &fs);
 assert(FILE_IO_VERSION(&fs) == 1);
 do_read(&fs, FALSE);
 file_read_close(&fs);
}

static void more_complx_test(void)
{
 file_io fs;
 
 file_write_open("abcdefg", 1, &fs);

 file_section_beg("abcd", &fs);
 do_write(&fs, 4040);
 file_section_end(&fs);
 
 file_write_close(&fs);


 
 file_read_open("abcdefg", &fs);
 assert(FILE_IO_VERSION(&fs) == 1);

 file_section_beg("abcd", &fs);
 do_read(&fs, FALSE);
 file_section_end(&fs);

 file_read_close(&fs); 
}

static void even_more_complx_test(void)
{
 file_io fs;
 char buffer[1024];
 int tmp = 0;

 file_write_open("abcd_loop", 4379, &fs);

 file_section_beg("abcd", &fs);
 while (tmp < 20)
 {
  sprintf(buffer, "%02d", tmp++);

  file_section_beg(buffer, &fs);
  do_write(&fs, tmp);
  file_section_end(&fs);
 }
 file_section_end(&fs);
 file_write_close(&fs);
 
 tmp = 0;
 
 file_read_open("abcd_loop", &fs);
 assert(FILE_IO_VERSION(&fs) == 4379);

 file_section_beg("abcd", &fs);
 while (tmp < 20)
 {
  sprintf(buffer, "%02d", tmp++);

  file_section_beg(buffer, &fs);
  do_read(&fs, FALSE);
  file_section_end(&fs); 
 }
 file_section_end(&fs); 
 file_read_close(&fs);
}

static void after_even_more_complx_test_setup(void)
{
 file_io fs;

 file_read_open("abcd_loop", &fs);
 assert(FILE_IO_VERSION(&fs) == 4379);

 file_section_beg("abcd", &fs);
 
 file_section_beg("04", &fs);
 file_get_int("004", &fs);
 assert(!FILE_IO_CREATED(&fs));
 do_read(&fs, TRUE);
 assert(!file_get_int("zzz", &fs));
 assert(FILE_IO_CREATED(&fs));
 file_section_end(&fs);

 assert(!file_get_int("zzz", &fs));
 assert(FILE_IO_CREATED(&fs));
 file_section_end(&fs); 
 assert(!file_get_int("zzz", &fs));
 assert(FILE_IO_CREATED(&fs));
 file_read_close(&fs);
}

int main(void)
{
 now = time(0);

 while (difftime(time(0), now) < MK_MINUTES(4))
 {
  simple_test();
  more_complx_test();
  even_more_complx_test();
  after_even_more_complx_test_setup();
 }
 
 exit (EXIT_SUCCESS);
}

void vwlog(const char *first, const char *fmt, ...)
{
 va_list va;

 va_start(va, fmt);
 
 fprintf(stderr, "%s: ", first);
 vfprintf(stderr, fmt, va);

 va_end(va);
}
