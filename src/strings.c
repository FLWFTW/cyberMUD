/**
 * @file strings.c
 * @author Brian Graverson
 * @version 1.0
 *
 * @section DESCRIPTION
 * This file handles string copy/search/comparison/etc.
 */
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>

/* include main header file */
#include "mud.h"

int isascii( int c )
{
   if( c >= 0 && c < 128 )
   {
      return TRUE;
   }
   return FALSE;
}

bool is_number( char *str )
{
   for( int i = 0; str[i] != '\0'; i++ )
   {
      if( isalpha(str[i] ) )
         return FALSE;
   }

   return TRUE;
}

/*
 * Checks if aStr is a prefix of bStr.
 */
bool is_prefix(const char *aStr, const char *bStr)
{
  /* NULL strings never compares */
  if (aStr == NULL || bStr == NULL) return FALSE;

  /* empty strings never compares */
  if (aStr[0] == '\0' || bStr[0] == '\0') return FALSE;

  /* check if aStr is a prefix of bStr */
  while (*aStr)
  {
    if (tolower(*aStr++) != tolower(*bStr++))
      return FALSE;
  }

  /* success */
  return TRUE;
}

char *one_arg(char *fStr, char *bStr)
{
  /* skip leading spaces */
  while (isspace(*fStr))
    fStr++; 

  /* copy the beginning of the string */
  while (*fStr != '\0')
  {
    /* have we reached the end of the first word ? */
    if (*fStr == ' ')
    {
      fStr++;
      break;
    }

    /* copy one char */
    *bStr++ = *fStr++;
  }

  /* terminate string */
  *bStr = '\0';

  /* skip past any leftover spaces */
  while (isspace(*fStr))
    fStr++;

  /* return the leftovers */
  return fStr;
}

char *capitalize(char *txt)
{
  static char buf[MAX_BUFFER];
  int size, i;

  buf[0] = '\0';

  if (txt == NULL || txt[0] == '\0')
    return buf;

  size = strlen(txt);

  for (i = 0; i < size; i++)
    buf[i] = toupper(txt[i]);
  buf[size] = '\0';

  return buf;
}

/*  
 * Create a new buffer.
 */
BUFFER *__buffer_new(int size)
{
  BUFFER *buffer;
    
  buffer = malloc(sizeof(BUFFER));
  buffer->size = size;
  buffer->data = malloc(size);
  buffer->len = 0;
  return buffer;
}

/*
 * Add a string to a buffer. Expand if necessary
 */
void __buffer_strcat(BUFFER *buffer, const char *text)  
{
  int new_size;
  int text_len;
  char *new_data;
 
  /* Adding NULL string ? */
  if (!text)
    return;

  text_len = strlen(text);
    
  /* Adding empty string ? */ 
  if (text_len == 0)
    return;

  /* Will the combined len of the added text and the current text exceed our buffer? */
  if ((text_len + buffer->len + 1) > buffer->size)
  { 
    new_size = buffer->size + text_len + 1;
   
    /* Allocate the new buffer */
    new_data = malloc(new_size);
  
    /* Copy the current buffer to the new buffer */
    memcpy(new_data, buffer->data, buffer->len);
    free(buffer->data);
    buffer->data = new_data;  
    buffer->size = new_size;
  }
  memcpy(buffer->data + buffer->len, text, text_len);
  buffer->len += text_len;
  buffer->data[buffer->len] = '\0';
}

/* free a buffer */
void buffer_free(BUFFER *buffer)
{
  /* Free data */
  free(buffer->data);
 
  /* Free buffer */
  free(buffer);
}

/* Clear a buffer's contents, but do not deallocate anything */
void buffer_clear(BUFFER *buffer)
{
  buffer->len = 0;
  buffer->data[0] = '\0';
}

/* print stuff, append to buffer. safe. */
int bprintf(BUFFER *buffer, char *fmt, ...)
{  
  char buf[MAX_BUFFER];
  va_list va;
  int res;
    
  va_start(va, fmt);
  res = vsnprintf(buf, MAX_BUFFER, fmt, va);
  va_end(va);
    
  if (res >= MAX_BUFFER - 1)  
  {
    buf[0] = '\0';
    bug("Overflow when printing string %s", fmt);
  }
  else
    buffer_strcat(buffer, buf);
   
  return res;
}

/*
char *strdup(const char *s)
{
  char *pstr;
  int len;

  len = strlen(s) + 1;
  pstr = (char *) calloc(1, len);
  strcpy(pstr, s);

  return pstr;
}

char *strcasestr( const char *haystack, const char *needle )
{
   int c = tolower((unsigned char)*needle);
   if (c == '\0')
      return (char *)haystack;
   for (; *haystack; haystack++)
   {
      if (tolower((unsigned char)*haystack) == c)
      {
         for (size_t i = 0;;)
         {
            if (needle[++i] == '\0')
               return (char *)haystack;
            if (tolower((unsigned char)haystack[i]) != tolower((unsigned char)needle[i]))
               break;
         }
      }
   }
   return NULL;
}
*/

int strcasecmp(const char *s1, const char *s2)
{
  int i = 0;

  while (s1[i] != '\0' && s2[i] != '\0' && toupper(s1[i]) == toupper(s2[i]))
    i++;

  /* if they matched, return 0 */
  if (s1[i] == '\0' && s2[i] == '\0')
    return 0;

  /* is s1 a prefix of s2? */
  if (s1[i] == '\0')
    return -110;

  /* is s2 a prefix of s1? */
  if (s2[i] == '\0')
    return 110;

  /* is s1 less than s2? */
  if (toupper(s1[i]) < toupper(s2[i]))
    return -1;

  /* s2 is less than s1 */
  return 1;
}

int strncasecmp(const char *s1, const char *s2, size_t n)
{
   if (n != 0) 
   {
      const unsigned char *us1 = (const unsigned char *)s1;
      const unsigned char *us2 = (const unsigned char *)s2;

      do
      {
         if (tolower(*us1) != tolower(*us2))
            return (tolower(*us1) - tolower(*us2));
         if (*us1++ == '\0')
            break;
         us2++;
      } while (--n != 0);
   }
   return (0);
}



char *strcasestr(const char *s, const char *find)
{
      char c, sc;
      size_t len;
      if ((c = *find++) != 0) 
      {
         c = (char)tolower((unsigned char)c);
         len = strlen(find);
         do
         {
            do
            {
               if ((sc = *s++) == 0)
                  return (NULL);
            } while ((char)tolower((unsigned char)sc) != c);
         } while (strncasecmp(s, find, len) != 0);
         s--;
      }
   return ((char *)s);
}

char *strdupf( const char *s, ... )
{
   char buf[MAX_BUFFER];
   va_list args;

   va_start( args, s );
   vsnprintf( buf, MAX_BUFFER, s, args );
   va_end( args );

   return strdup( buf );
}
   
