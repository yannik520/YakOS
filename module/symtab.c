/*
 * Copyright (c) 2013 Yannik Li(Yanqing Li)
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <module/symtab.h>
#include <module/symbols.h>
#include <string.h>

/* Binary search is twice as large but still small. */
#ifndef SYMTAB_CONF_BINARY_SEARCH
#define SYMTAB_CONF_BINARY_SEARCH 1
#endif

#if SYMTAB_CONF_BINARY_SEARCH
void *
symtab_lookup(const char *name)
{
  int start, middle, end;
  int r;
  
  start = 0;
  end = symbols_nelts - 1;	/* The last entry is { 0, 0 }. */

  while(start <= end) {
    /* Check middle, divide */
    middle = (start + end) / 2;
    r = strcmp(name, symbols[middle].name);
    if(r < 0) {
      end = middle - 1;
    } else if(r > 0) {
      start = middle + 1;
    } else {
      return symbols[middle].value;   
    }
  }
  return NULL;
}
#else /* SYMTAB_CONF_BINARY_SEARCH */
void *
symtab_lookup(const char *name)
{
  const struct symbols *s;
  for(s = symbols; s->name != NULL; ++s) {
    if(strcmp(name, s->name) == 0) {
      return s->value;
    }
  }
  return 0;
}
#endif /* SYMTAB_CONF_BINARY_SEARCH */
