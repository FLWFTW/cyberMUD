/**
 * @file colors.c
 * @author Brian Graverson
 * @version 1.0
 *
 * @section DESCRIPTION
 * File original to SocketMUD. Strips color codes from text.
 */
#include "mud.h"

char *strip_color_codes( char *msg )
{
   static char result[MAX_BUFFER*2];
   char *start;
   char *orig;
   
   start = result;
   orig  = msg;

   for( ; *orig; orig++ )
   {
      if( *orig == '&' ) //background color
      {
         orig++;
         switch( *orig )
         {
            case 'x': //xrgoybpcw
            case 'r':
            case 'g':
            case 'o':
            case 'y':
            case 'b':
            case 'p':
            case 'c':
            case 'w':
            case 'X': //xrgoybpcw
            case 'R':
            case 'G':
            case 'O':
            case 'Y':
            case 'B':
            case 'P':
            case 'C':
            case 'W':
               orig++;
               break;
            default:
               start += sprintf( start, "&%c", *orig );
               orig++;
               break;
         }
      }
      if( *orig == '^' ) //background color
      {
         orig++;
         switch( *orig )
         {
            case 'x': //xrgoybpcw
            case 'r':
            case 'g':
            case 'o':
            case 'y':
            case 'b':
            case 'p':
            case 'c':
            case 'w':
            case 'X':
            case 'R':
            case 'G':
            case 'O':
            case 'Y':
            case 'B':
            case 'P':
            case 'C':
            case 'W':
               orig++;
               break;
            default:
               start += sprintf( start, "^%c", *orig );
               orig++;
               break;
         }
      }
      else
      {
         *start = *orig;
         start++;
      }
   }
   *start = '\0';
   strcat( start, "\033[0m" );
   return result;
}
char *parse_color_codes( char *msg )
{
   static char result[MAX_BUFFER*2];
   char *start;
   char *orig;
   
   start = result;
   orig  = msg;

   for( ; *orig; orig++ )
   {
      if( *orig == '&' ) //foreground color
      {
         orig++;
         switch( *orig )
         {
            case 'x': //normal/dull colors
               start += sprintf( start, "\033[0m\033[30m" );
               break;
            case 'r':
               start += sprintf( start, "\033[0m\033[31m" );
               break;
            case 'g':
               start += sprintf( start, "\033[0m\033[32m" );
               break;
            case 'o':
            case 'y':
               start += sprintf( start, "\033[0m\033[33m" );
               break;
            case 'b':
               start += sprintf( start, "\033[0m\033[34m" );
               break;
            case 'p':
               start += sprintf( start, "\033[0m\033[35m" );
               break;
            case 'c':
               start += sprintf( start, "\033[0m\033[36m" );
               break;
            case 'w':
               start += sprintf( start, "\033[0m\033[37m" );
               break;
            case 'X': //bright/light colors
               start += sprintf( start, "\033[0m\033[1;30m" );
               break;
            case 'R':
               start += sprintf( start, "\033[0m\033[1;31m" );
               break;
            case 'G':
               start += sprintf( start, "\033[0m\033[1;32m" );
               break;
            case 'O':
            case 'Y':
               start += sprintf( start, "\033[0m\033[1;33m" );
               break;
            case 'B':
               start += sprintf( start, "\033[0m\033[1;34m" );
               break;
            case 'P':
               start += sprintf( start, "\033[0m\033[1;35m" );
               break;
            case 'C':
               start += sprintf( start, "\033[0m\033[1;36m" );
               break;
            case 'W':
               start += sprintf( start, "\033[0m\033[1;37m" );
               break;
            case '0':
               start += sprintf( start, "\033[0m");
               break;
            default:
               start += sprintf( start, "&%c", *orig );
               break;
         }
      }
      else if( *orig == '^' ) //background color
      {
         orig++;
         switch( *orig )
         {
            case 'x': //normal/dull colors
               start += sprintf( start, "\033[40m" );
               break;
            case 'r':
               start += sprintf( start, "\033[41m" );
               break;
            case 'g':
               start += sprintf( start, "\033[42m" );
               break;
            case 'o':
            case 'y':
               start += sprintf( start, "\033[43m" );
               break;
            case 'b':
               start += sprintf( start, "\033[44m" );
               break;
            case 'p':
               start += sprintf( start, "\033[45m" );
               break;
            case 'c':
               start += sprintf( start, "\033[46m" );
               break;
            case 'w':
               start += sprintf( start, "\033[47m" );
               break;
            case 'X': //bright/light colors
               start += sprintf( start, "\033[100m" );
               break;
            case 'R':
               start += sprintf( start, "\033[101m" );
               break;
            case 'G':
               start += sprintf( start, "\033[102m" );
               break;
            case 'O':
            case 'Y':
               start += sprintf( start, "\033[103m" );
               break;
            case 'B':
               start += sprintf( start, "\033[104m" );
               break;
            case 'P':
               start += sprintf( start, "\033[105m" );
               break;
            case 'C':
               start += sprintf( start, "\033[106m" );
               break;
            case 'W':
               start += sprintf( start, "\033[107m" );
               break;
            default:
               start += sprintf( start, "&%c", *orig );
               break;
         }
      }
      else
      {
         *start = *orig;
         start++;
      }
   }
   *start = '\0';
   strcat( start, "\033[0m" );
   return result;
}
