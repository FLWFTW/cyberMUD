/**
 * @file io.c
 * @author Brian Graverson with mods by Will Sayin
 * @version 1.0
 *
 * @section DESCRIPTION
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
time_t boot_time;

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
  char strtime[32];
  va_list args;

  va_start(args, txt);
  vsnprintf(buf, MAX_BUFFER, txt, args);
  va_end(args);

  /* We want log files to be of format: YYYY-MM-DD.log */
  time_t rawtime;
  struct tm *timeinfo;
  time( &rawtime );
  timeinfo = localtime( &rawtime );

  /* point to the correct logfile */
  strftime( logfile, MAX_BUFFER, "../log/%F.log", timeinfo );
  strftime( strtime, 32, "%F @ %T", timeinfo );

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

