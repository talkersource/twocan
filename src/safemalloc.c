#define SAFEMALLOC_C
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

static unsigned int malloc_count = 0;
#ifdef EXTRA_MALLOC_WRAPPER
static unsigned int malloc_size = 0;
static malloc_lookup *malref_start[MALLOC_TYPES_SIZE] = {NULL};

static void addto_malloc_ref(malloc_lookup *tmp, unsigned int type)
{
 assert(tmp);
 
 if ((tmp->next = malref_start[tmp->type]))
   tmp->next->prev = tmp;

 tmp->prev = NULL;
 malref_start[type] = tmp;

 malloc_size += (tmp->size + sizeof(malloc_lookup));
}

static malloc_lookup *internal_find_malloc_ref(void *ptr, malloc_lookup *tmp)
{
 while (tmp)
 {
  if (tmp->mem_ref == ptr)
    return (tmp);

  tmp = tmp->next;
 }
 
 return (NULL);
}

static malloc_lookup *find_malloc_ref(void *ptr, unsigned int type,
                                      const char *file_ref,
                                      unsigned int line_ref,
                                      const char *calling_func_name)
{
 malloc_lookup *tmp = NULL;

 assert(type < MALLOC_TYPES_SIZE);
 
 if ((tmp = internal_find_malloc_ref(ptr, malref_start[type])))
   return (tmp);
 else
 {
  unsigned int count = 0;

  while (count < MALLOC_TYPES_SIZE)
  {
   if ((tmp = internal_find_malloc_ref(ptr, malref_start[count])))
   {
    log_assert(FALSE);
    vwlog("malloc", " (%s) wrong section: \n"
          " orig = file %s, line %u, type %u\n"
          " found = file %s, line %u, type %u\n",
          calling_func_name,
          tmp->file_ref, tmp->line_ref, tmp->type, file_ref, line_ref, type);
    return (tmp);
   }

   ++count;
  } 
 }
 
 log_assert(FALSE);
 vwlog("malloc", "(%s) block not found = file %s, line %u, type %u\n",
       calling_func_name, file_ref, line_ref, type);
 return (NULL);
}

#ifdef USE_POINTER_COMPARES
static malloc_lookup *internal_find_malloc_block(const void *ptr,
                                                 malloc_lookup *tmp)
{
 while (tmp)
 {
  if (PTR_EQUAL(ptr, tmp->mem_ref) ||
      (!PTR_LESS(ptr, tmp->mem_ref) &&
       (PTR_DIFF(ptr, tmp->mem_ref) < tmp->size)))
    return (tmp);

  tmp = tmp->next;
 }
 
 return (NULL);
}

static malloc_lookup *find_malloc_block(const void *ptr, unsigned int type,
                                        const char *file_ref,
                                        unsigned int line_ref,
                                        const char *calling_func_name)
{
 malloc_lookup *tmp = NULL;

 assert(type < MALLOC_TYPES_SIZE);
 
 if ((tmp = internal_find_malloc_block(ptr, malref_start[type])))
   return (tmp);
 else
 {
  unsigned int count = 0;

  while (count < MALLOC_TYPES_SIZE)
  {
   if ((tmp = internal_find_malloc_block(ptr, malref_start[count])))
   {
    log_assert(FALSE);
    vwlog("malloc", " (%s) wrong section: \n"
          " orig  = file %s, line %u, type %u\n"
          " found = file %s, line %u, type %u\n",
          calling_func_name,
          tmp->file_ref, tmp->line_ref, tmp->type, file_ref, line_ref, type);
    return (tmp);
   }
   ++count;
  }
 }

 log_assert(FALSE);
 vwlog("malloc", "(%s) block not found = file %s, line %u, type %u\n",
       calling_func_name, file_ref, line_ref, type);
 return (NULL);
}
#endif

static int remove_malloc_ref(void *ptr, unsigned int type,
                             const char *file_ref, unsigned int line_ref)
{
 malloc_lookup *tmp = NULL;
 
 assert(ptr && (type < MALLOC_TYPES_SIZE) && file_ref && line_ref);
 
 if ((tmp = find_malloc_ref(ptr, type, file_ref, line_ref,
                            "remove_malloc_ref")))
 {
  if (tmp->prev)
    tmp->prev->next = tmp->next;
  else
    malref_start[type] = tmp->next;
  
  if (tmp->next)
    tmp->next->prev = tmp->prev;
  
  malloc_size -= (tmp->size + sizeof(malloc_lookup));
  memset(ptr, MALLOC_FREE_GARBAGE, tmp->size);
  memset(tmp, MALLOC_FREE_GARBAGE, sizeof(malloc_lookup));
  free (tmp);
  
  return (TRUE);
 }

 return (FALSE);
}
#endif

void *safecalloc(size_t nmemb, size_t size, unsigned int type,
                 const char *file_ref, unsigned int line_ref)
{
 void *ptr = NULL;

 BTRACE("safecalloc");
 
 ptr = calloc (nmemb, size);
  
 if (!ptr)
   goto fail;
 
#ifdef EXTRA_MALLOC_WRAPPER
 {
  malloc_lookup *tmp = malloc (sizeof(malloc_lookup));
  
  if (!tmp)
    goto fail_free;
  
  tmp->size = (nmemb * size);
  tmp->type = type;
  tmp->file_ref = file_ref;
  tmp->line_ref = line_ref;
  tmp->mem_ref = ptr;
  addto_malloc_ref(tmp, type);
 }
#else
 IGNORE_PARAMETER(type && file_ref && line_ref); 
#endif

 malloc_count++;
 /* *******************************
  * no memset garbage needed, as calloc garantee's to
  * memset everything to 0's */
 BTRACE("safecalloc");
 
 return (ptr);
#ifdef EXTRA_MALLOC_WRAPPER

 fail_free:
 free (ptr);
#endif
 
 fail:
 BTRACE("safecalloc (end)");
 return (NULL);
} 

void *safemalloc(size_t size, unsigned int type,
                 const char *file_ref, unsigned int line_ref)
{
 void *ptr = NULL;

 BTRACE("safemalloc");

#ifdef NDEBUG
 ptr = calloc (1, size); /* get 0'd out structs etc. */
#else
 ptr = malloc (size);
#endif
 
 if (!ptr)
   goto fail;

#ifdef EXTRA_MALLOC_WRAPPER
 {
  malloc_lookup *tmp = malloc (sizeof(malloc_lookup));

  if (!tmp)
    goto fail_free;
  
  tmp->size = size;
  tmp->type = type;
  tmp->file_ref = file_ref;
  tmp->line_ref = line_ref;
  tmp->mem_ref = ptr;
  addto_malloc_ref(tmp, type);
 }
#else
 IGNORE_PARAMETER(type && file_ref && line_ref);
#endif

 if (ptr)
 {
#ifndef NDEBUG
  memset(ptr, MALLOC_MALLOC_GARBAGE, size);
#endif
  malloc_count++;
  BTRACE("safemalloc");
 }

 return (ptr);

#ifdef EXTRA_MALLOC_WRAPPER
 fail_free:
 free (ptr);
#endif
 fail:
 BTRACE("safemalloc");
 return (NULL);
}

void *saferealloc(void *ptr, size_t length, unsigned int type,
                  const char *file_ref, unsigned int line_ref)
{
 BTRACE("saferealloc");

#ifndef EXTRA_MALLOC_WRAPPER
 IGNORE_PARAMETER(type && file_ref && line_ref);
 ptr = realloc(ptr, length);
 BTRACE("saferealloc");
 return (ptr);
#else
 if (!ptr)
   return (safemalloc(length, type, file_ref, line_ref));
 else
 {
  malloc_lookup *tmp = NULL;
  void *new_ptr = NULL;
  
  assert(ptr != (void *)0xfbfbfbfb);
  assert(ptr != (void *)0xfafafafa);

  if (!(tmp = find_malloc_block(ptr, type, file_ref, line_ref,
                                "saferealloc")))
  {
   assert(FALSE);
   return (NULL);
  }
  assert(PTR_EQUAL(ptr, tmp->mem_ref));
  new_ptr = realloc(tmp->mem_ref, length);
  
  if (!new_ptr)
  {
   BTRACE("saferealloc");
   return (NULL);
  }
  
  tmp->mem_ref = new_ptr;
  tmp->size = length;
  assert(tmp->type == type);
  tmp->type = type;
  tmp->file_ref = file_ref;
  tmp->line_ref = line_ref;
  
  BTRACE("saferealloc");
  return (tmp->mem_ref);
 }
#endif
}

void safefree(void *ptr, unsigned int type,
              const char *file_ref, unsigned int line_ref)
{
 BTRACE("safefree");

 log_assert(ptr);
 
 assert(malloc_count > 0);
 assert(ptr != (void *)0xfbfbfbfb);
 assert(ptr != (void *)0xfafafafa);
 malloc_count--;
#ifndef EXTRA_MALLOC_WRAPPER
 IGNORE_PARAMETER(type && file_ref && line_ref);
#else
 if (remove_malloc_ref(ptr, type, file_ref, line_ref))
#endif
   free (ptr);
 BTRACE("safefree");
}

#ifdef EXTRA_MALLOC_WRAPPER
void malloc_no_members(unsigned int type,
                       const char *file_ref, unsigned int line_ref)
{
 if (malref_start[type])
 {
  assert(FALSE);
  vwlog("error", " ** malloc error: %d %s\n", line_ref, file_ref);
 }
}

# ifdef USE_POINTER_COMPARES
int malloc_valid(const void *ptr, size_t size, unsigned int type,
                 const char *file_ref, unsigned int line_ref)             
{
 malloc_lookup *tmp = NULL;

 assert(ptr != (void *)0xfbfbfbfb);
 assert(ptr != (void *)0xfafafafa);

 assert(ptr && (type < MALLOC_TYPES_SIZE) && file_ref && line_ref);
 
 if ((tmp = find_malloc_block(ptr, type, file_ref, line_ref,
                              "malloc_valid")))
 {
  if ((tmp->size - PTR_DIFF(ptr, tmp->mem_ref)) >= size)
    return (TRUE);

  log_assert(FALSE);
  vwlog("malloc", " (malloc_valid) wrong size: \n"
       " malloc = file %s, line %u, type %u, bytes left = %u\n"
       " valid  = file %s, line %u, type %u, bytes asked = %u\n",
       tmp->file_ref, tmp->line_ref, tmp->type,
       (unsigned int) (tmp->size - PTR_DIFF(ptr, tmp->mem_ref)),
       file_ref, line_ref, type, (unsigned int) size);
 }
  
 return (FALSE);
}
# endif             
#endif

#ifdef TALKER_MAIN_H
static void malloc_count_show(player *p, parameter_holder *params)
{
#ifdef EXTRA_MALLOC_WRAPPER
 malloc_lookup *tmp = NULL;
 unsigned int count = 0;
 unsigned int type = 0;
 unsigned int size = 0;

 if (params->last_param == 2)
 {
  type = atol(GET_PARAMETER_STR(params, 1));
  size = atol(GET_PARAMETER_STR(params, 2));
 }
 else
 {
  fvtell_player(SYSTEM_T(p), " Types go from 0 to %u.\n",
                MALLOC_TYPES_SIZE - 1);
  TELL_FORMAT(p, "<malloc_type> <minimum size>");
 }
 
 if (type < MALLOC_TYPES_SIZE)
 {
  tmp = malref_start[type];
  fvtell_player(SYSTEM_T(p), " Starting type (%u).\n", type);
 }
 else
 {
  fvtell_player(SYSTEM_T(p), " Types go from 0 to %u.\n",
                MALLOC_TYPES_SIZE - 1);
  return;
 }
 
 while (tmp)
 {
  ++count;
  if (tmp->size > size)
    fvtell_player(SYSTEM_T(p), " (%05u) is size %06u from %s, at line %u.\n",
                  count, (unsigned int) tmp->size, tmp->file_ref,
                  tmp->line_ref);
  tmp = tmp->next;
 }
 
 fvtell_player(SYSTEM_T(p), " Total malloc size is now at: ^B%d^N\n",
               malloc_size);
#else
 IGNORE_PARAMETER(params); 
#endif
 fvtell_player(SYSTEM_T(p), " Total malloc count is now at: ^B%d^N\n",
               malloc_count);
 pager(p, PAGER_USE_FORCE);
}

static void user_sys_mallinfo(player *p)
{
#ifdef HAVE_MALLINFO
 struct mallinfo mem_info = mallinfo(); /* warning */

 fvtell_player(NORMAL_T(p),
               "$Line_fill(-=)$Line_fill(-)\n"
               " arena    = %08d\n"
               " ordblks  = %08d\n"
               " smblks   = %08d\n"
               " hblks    = %08d\n"
               " hblkhd   = %08d\n"
               " usmblks  = %08d\n"
               " fsmblks  = %08d\n"
               " uordblks = %08d\n"
               " fordblks = %08d\n"
               " keepcost = %08d\n"
               "$Line_fill(-)\n",
               mem_info.arena,
               mem_info.ordblks,
               mem_info.smblks,
               mem_info.hblks,
               mem_info.hblkhd,
               mem_info.usmblks,
               mem_info.fsmblks,
               mem_info.uordblks,
               mem_info.fordblks,
               mem_info.keepcost);
#else
 fvtell_player(SYSTEM_T(p), "%s", " This command isn't available.\n");
#endif
}

void cmds_init_safemalloc(void)
{
 CMDS_BEGIN_DECLS();
 
 CMDS_ADD("malloc", malloc_count_show, PARSE_PARAMS, ADMIN);
 CMDS_PRIV(coder_admin);
 CMDS_ADD("mallinfo", user_sys_mallinfo, NO_CHARS, SPOD);
 CMDS_PRIV(spod);
}

#endif
