#ifndef EXTERN_DISPLAY_TIME_H
#define EXTERN_DISPLAY_TIME_H

/* used by twinkles */
extern char *disp_time_ampm_string(time_t) __attribute__ ((const));
extern char *disp_time_second_number(time_t) __attribute__ ((const));
extern char *disp_time_minute_number(time_t) __attribute__ ((const));
extern char *disp_time_hour_number_24(time_t) __attribute__ ((const));
extern char *disp_time_hour_number_12(time_t) __attribute__ ((const));
extern char *disp_time_day_name(time_t) __attribute__ ((const));
extern char *disp_time_day_name_short(time_t) __attribute__ ((const));
extern char *disp_time_week_day_number(time_t) __attribute__ ((const));
extern char *disp_time_month_name(time_t) __attribute__ ((const));
extern char *disp_time_month_name_short(time_t) __attribute__ ((const));
extern char *disp_time_month_number(time_t) __attribute__ ((const));
extern char *disp_time_month_day_number(time_t) __attribute__ ((const));
extern char *disp_time_week_number(time_t) __attribute__ ((const));
extern char *disp_time_year_number(time_t) __attribute__ ((const));
extern char *disp_time_year_day_number(time_t) __attribute__ ((const));
      
/* functions used in the code */
extern char *disp_time_cmp_string(time_t) __attribute__ ((const));
extern time_t disp_time_string_cmp(const char *);
extern char *disp_time_file_name(time_t) __attribute__ ((const));

extern time_t disp_time_add(time_t, int) __attribute__ ((const));
extern char *disp_time_hour_min(time_t, int, int) __attribute__ ((const));
extern char *disp_time_std(time_t, int, int, int) __attribute__ ((const));
extern char *disp_date_std(time_t, int, int) __attribute__ ((const));

extern time_t disp_time_create(int, int, int, int, int, int)
    __attribute__ ((const));

#define DISP_TIME_P_STD(now, p) \
 disp_time_std(now, (p)->gmt_offset, \
               (p)->flag_use_24_clock, (p)->flag_use_long_clock)
#define DISP_DATE_P_STD(now, p) \
 disp_date_std(now, (p)->gmt_offset, (p)->flag_use_long_clock)

extern char *disp_date_birthday_string(time_t, int)
    __attribute__ ((const));

extern const char *disp_time_filename(time_t, const char *, const char *);



#endif
