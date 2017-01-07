#define TRACE_C
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

#if USING_TRACE
static unsigned int number = 0;

static void do_trace(unsigned int max_slots, unsigned int *current_slot,
                     trace_slots *slots, const char *info,
                     const char *filename, int line_number)
{
 ++*current_slot;
 *current_slot %= max_slots;
 
 slots[*current_slot].info = info;
 slots[*current_slot].filename = filename;
 slots[*current_slot].line_number = line_number;
 slots[*current_slot].timestamp = now;
 slots[*current_slot].number = ++number;
}

static void dump_trace(unsigned int max_slots, unsigned int current_slot,
                       trace_slots *slots, FILE *log_out)
{
 unsigned int current = 0;
 char time_str[32] = "";
 
 while (current < max_slots)
 {
  const char *end_name = NULL;
  
  ++current_slot;
  ++current;
  current_slot %= max_slots;

  if (!slots[current_slot].filename)
    continue;
  
  strftime(time_str, 30, "%H:%M:%S", gmtime(&slots[current_slot].timestamp));
  time_str[31] = 0;

  if ((end_name = C_strrchr(slots[current_slot].filename, '/')))
    slots[current_slot].filename = end_name + 1;
  
  fprintf(log_out, "(%u) %s %-40s@%s:%d\n",
          slots[current_slot].number, time_str,
          slots[current_slot].info,
          slots[current_slot].filename, slots[current_slot].line_number);
 }
}


#ifdef TRACE_INPUT_TO

static trace_slots ic_slots[MAX_TRACE_INPUT_TO];
static unsigned int ic_count = 0;

/* input to commands... ones that miss the parser etc... */
void do_ictrace(const char *str, const char *filename, int line_number)
{
 do_trace(MAX_TRACE_INPUT_TO, &ic_count, ic_slots,
          str, filename, line_number);
}

void dump_ictrace(void)
{
 FILE *log_out = open_wlog("trace");
 if (log_out)
 {
  fprintf(log_out, "%s", "\n(trace_input_to function)\n");
  dump_trace(MAX_TRACE_INPUT_TO, ic_count, ic_slots, log_out);
  close_wlog(log_out);
 }
}

#endif




#ifdef TRACE_TIMED
/* timed commands.. */

static trace_slots tc_slots[MAX_TRACE_TIMED];
static unsigned int tc_count = 0;

void do_tctrace(const char *str, const char *filename, int line_number)
{
 do_trace(MAX_TRACE_TIMED, &tc_count, tc_slots,
          str, filename, line_number);
}

void dump_tctrace(void)
{
 FILE *log_out = open_wlog("trace");
 if (log_out)
 {
  fprintf(log_out, "%s", "\n(trace_timed functions)\n");
  dump_trace(MAX_TRACE_TIMED, tc_count, tc_slots, log_out);
  close_wlog(log_out);
 }
}

#endif




#ifdef TRACE_BASE
/* base functions... */

static trace_slots base_slots[MAX_TRACE_BASE];
static unsigned int base_count = 0;

void do_btrace(const char *str, const char *filename, int line_number)
{
 do_trace(MAX_TRACE_BASE, &base_count, base_slots,
          str, filename, line_number);
}

void dump_btrace(void)
{
 FILE *log_out = open_wlog("trace");
 if (log_out)
 {
  fprintf(log_out, "%s", "\n(trace_base function)\n");
  dump_trace(MAX_TRACE_BASE, base_count, base_slots, log_out);
  close_wlog(log_out);
 }
}

#endif




#ifdef TRACE_SECTION
/* sections of the code... */

static trace_slots section_slots[MAX_TRACE_SECTION];
static unsigned int section_count = 0;

void do_strace(const char *str, const char *filename, int line_number)
{
 do_trace(MAX_TRACE_SECTION, &section_count, section_slots,
          str, filename, line_number);
}

void dump_strace(void)
{
 FILE *log_out = open_wlog("trace");
 if (log_out)
 {
  fprintf(log_out, "%s", "\n(trace_section function)\n");
  dump_trace(MAX_TRACE_SECTION, section_count, section_slots, log_out);
  close_wlog(log_out);
 }
}

#endif


#ifdef TRACE_COMMANDS
/* list of last comnmands run... */

static trace_slots commands_slots[MAX_TRACE_COMMANDS];
static unsigned int commands_count = 0;

void do_ctrace(const char *str, const char *filename, int line_number)
{
 do_trace(MAX_TRACE_SECTION, &section_count, section_slots,
          str, filename, line_number);
}

void dump_ctrace(void)
{
 FILE *log_out = open_wlog("trace");
 if (log_out)
 {
  fprintf(log_out, "%s", "\n(trace_commands function)\n");
  dump_trace(MAX_TRACE_COMMANDS, commands_count, commands_slots, log_out);
  close_wlog(log_out);
 }
}

#endif


#endif
/* end of if any of trace funcs */

