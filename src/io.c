/*
 * This file handles input/output to files (including log)
 */
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>  
#include <sys/stat.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>

/* include main header file */
#include "mud.h"

time_t current_time;

/*
 * Nifty little extendable logfunction,
 * if it wasn't for Erwins social editor,
 * I would never have known about the
 * va_ functions.
 */
void log_string(const char *txt, ...)
{
  FILE *fp;
  char logfile[MAX_BUFFER];
  char buf[MAX_BUFFER];
  char *strtime = get_time();
  va_list args;

  va_start(args, txt);
  vsnprintf(buf, MAX_BUFFER, txt, args);
  va_end(args);

  /* point to the correct logfile */
  snprintf(logfile, MAX_BUFFER, "../log/%6.6s.log", strtime);

  /* try to open logfile */
  if ((fp = fopen(logfile, "a")) == NULL)
  {
    communicate(NULL, "log: cannot open logfile", COMM_LOG);
    return;
  }

  fprintf(fp, "%s: %s\n", strtime, buf);
  fclose(fp);

  communicate(NULL, buf, COMM_LOG);
}

/*
 * Nifty little extendable bugfunction,
 * if it wasn't for Erwins social editor,
 * I would never have known about the
 * va_ functions.
 */
void bug(const char *txt, ...)
{
  FILE *fp;
  char buf[MAX_BUFFER];
  va_list args;
  char *strtime = get_time();

  va_start(args, txt);
  vsnprintf(buf, MAX_BUFFER, txt, args);
  va_end(args);

  /* try to open logfile */
  if ((fp = fopen("../log/bugs.txt", "a")) == NULL)
  {
    communicate(NULL, "bug: cannot open bugfile", COMM_LOG);
    return;
  }

  fprintf(fp, "%s: %s\n", strtime, buf);
  fprintf(stderr, "%s: %s\n", strtime, buf);
  fclose(fp);

  communicate(NULL, buf, COMM_LOG);
}

/*
 * This function will return the time of
 * the last modification made to helpfile.
 */
time_t last_modified(char *helpfile)
{
  char fHelp[MAX_BUFFER];
  struct stat sBuf;
  time_t mTime = 0;

  snprintf(fHelp, MAX_BUFFER, "../help/%s", helpfile);
  if (stat(fHelp, &sBuf) >= 0)
    mTime = sBuf.st_mtime;

  return mTime;
}

char *read_help_entry(const char *helpfile)
{
  FILE *fp;
  static char entry[MAX_HELP_ENTRY];
  char fHelp[MAX_BUFFER];
  int c, ptr = 0;

  /* location of the help file */
  snprintf(fHelp, MAX_BUFFER, "../help/%s", helpfile);

  /* if there is no help file, return NULL */
  if ((fp = fopen(fHelp, "r")) == NULL)
    return NULL;

  /* just to have something to work with */
  c = getc(fp);

  /* read the file in the buffer */
  while (c != EOF)
  {
    if (c == '\n')
      entry[ptr++] = '\r';
    entry[ptr] = c;
    if (++ptr > MAX_HELP_ENTRY - 2)
    {
      bug("Read_help_entry: String to long.");
      abort();
    }
    c = getc(fp);
  }
  entry[ptr] = '\0';

  fclose(fp);

  /* return a pointer to the static buffer */
  return entry;
}
