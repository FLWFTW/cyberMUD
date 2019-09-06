/******************************************************************************
 *  communication functions                                                   *
 *****************************************************************************/
#include "mud.h"

/*
 * Text_to_socket()
 *
 * Sends text directly to the socket,
 * will compress the data if needed.
 */
 
static char *parse_prompt( D_MOBILE *dMob );

bool text_to_socket( D_SOCKET *dsock, const char *txt2, ... )
{
   int iBlck, iPtr, iWrt = 0, length, icontrol = dsock->control;
   va_list args;
   char buf[MAX_BUFFER], *txt = buf;

   va_start( args, txt2 );
   vsnprintf( buf, MAX_BUFFER, txt2, args );
   va_end( args );

   length = strlen( txt );

   /* write compressed */
   if( dsock && dsock->out_compress )
   {
      dsock->out_compress->next_in = (unsigned char *)txt;
      dsock->out_compress->avail_in = length;

      while( dsock->out_compress->avail_in )
      {
         dsock->out_compress->avail_out = COMPRESS_BUF_SIZE - ( dsock->out_compress->next_out - dsock->out_compress_buf );

         if( dsock->out_compress->avail_out )
         {
            int status = deflate( dsock->out_compress, Z_SYNC_FLUSH );

            if( status != Z_OK )
               return FALSE;
         }

         length = dsock->out_compress->next_out - dsock->out_compress_buf;
         if( length > 0 )
         {
            for( iPtr = 0; iPtr < length; iPtr += iWrt )
            {
               iBlck = UMIN( length - iPtr, 4096 );
               if( ( iWrt = write( icontrol, dsock->out_compress_buf + iPtr, iBlck ) ) < 0 )
               {
                  perror( "Text_to_socket (compressed):" );
                  return FALSE;
               }
            }
            if( iWrt <= 0 )
               break;
            if( iPtr > 0 )
            {
               if( iPtr < length )
                  memmove( dsock->out_compress_buf, dsock->out_compress_buf + iPtr, length - iPtr );

               dsock->out_compress->next_out = dsock->out_compress_buf + length - iPtr;
            }
         }
      }
      return TRUE;
   }

   /* write uncompressed */
   for( iPtr = 0; iPtr < length; iPtr += iWrt )
   {
      iBlck = UMIN( length - iPtr, 4096 );
      if( ( iWrt = write( icontrol, txt + iPtr, iBlck ) ) < 0 )
      {
         perror( "Text_to_socket:" );
         return FALSE;
      }
   }

   return TRUE;
}

/*
 * Text_to_buffer()
 *
 * Stores outbound text in a buffer, where it will
 * stay untill it is flushed in the gameloop.
 *
 * Will also parse ANSI colors and other tags.
 */
void text_to_buffer( D_SOCKET *dsock, const char *txt2, ... )
{
   static char output[8 * MAX_BUFFER];
   char buf[MAX_BUFFER];
   va_list args;
   if( !dsock ) //In case we end up pointing at a mobile or something
      return;

   va_start( args, txt2 );
   vsnprintf( buf, MAX_BUFFER, txt2, args );
   va_end( args );

   /* always start with a leading space 
   if( dsock->top_output == 0 )
   {
      dsock->outbuf[0] = '\n';
      dsock->outbuf[1] = '\r';
      dsock->top_output = 2;
   }*/
   snprintf( output, MAX_BUFFER * 8, "%s", buf );

   /* check to see if the socket can accept that much data */
   if( dsock->top_output + strlen(output) >= MAX_OUTPUT )
   {
      bug( "Text_to_buffer: ouput overflow on %s.", dsock->hostname );
      return;
   }

   /* add data to buffer */
   strncat( dsock->outbuf, output, MAX_OUTPUT - strlen( dsock->outbuf ) -1 );
   dsock->top_output = strlen( dsock->outbuf );
}

void send_json_m( D_MOBILE *dMob, const char *txt2, ... )
{
   va_list args;
   char buf[MAX_BUFFER], *txt = buf;

   va_start( args, txt2 );
   vsnprintf( buf, MAX_BUFFER, txt2, args );
   va_end( args );

   if( dMob->socket )
   {
      text_to_socket( dMob->socket, "%c%s%c", (char)2, txt, (char)3 );
   }
}

/*
 * Text_to_mobile()
 *
 * If the mobile has a socket, then the data will
 * be send to text_to_buffer().
 */
void text_to_mobile( D_MOBILE *dMob, const char *txt2, ... )
{
   va_list args;
   char buf[MAX_BUFFER], *txt = buf;

   va_start( args, txt2 );
   vsnprintf( buf, MAX_BUFFER, txt2, args );
   va_end( args );
   //buf[0] = toupper( buf[0] ); //sentences always begin with a capital letter...
   //except when they dont.... --Will

   if( dMob->socket )
   {
      text_to_buffer( dMob->socket, txt );
   }
}

void text_to_mobile_j( D_MOBILE *dMob, const char *type, const char *txt2, ... )
{
   va_list args;
   //If we're ever having issues with item lists getting truncated in the game
   //it's because there's too much JSON for this buffer to handle and we'll need to go bigger :-D
   char buf[MAX_BUFFER*10];

   va_start( args, txt2 );
   vsnprintf( buf, MAX_BUFFER, txt2, args );
   va_end( args );
   buf[0] = toupper( buf[0] ); //sentences always begin with a capital letter...

   json_t *json = json_object();
   json_t *data = json_object();
   json_object_set_new( json, "type", json_string( type ) );
   json_object_set_new( data, "text", json_string( buf ) );
   json_object_set_new( json, "data", data );

   if( dMob->socket )
   {
      char *dump = json_dumps( json, 0 );
      text_to_socket( dMob->socket, "%c%s%c", (char)2, dump, (char)3 );
      free( dump );
   }
   json_decref( json );
   return;
}


//Assumes one and two are in the same room.
void echo_around_two( D_MOBILE *one, D_MOBILE *two, char *type, char *txt, ... )
{
   D_MOBILE *around;
   ITERATOR Iter;
   va_list args;
   char buf[MAX_BUFFER];

   va_start( args, txt );
   vsnprintf( buf, MAX_BUFFER, txt, args );
   va_end( args );


   AttachIterator( &Iter, one->room->mobiles );
   while( ( around = (D_MOBILE *)NextInList( &Iter ) ) != NULL )
   {
      if( around == one || around == two )
         continue;
      else
         text_to_mobile_j( around, type, buf );
   }
   DetachIterator( &Iter );
}

void echo_around( D_MOBILE *dMob, char *type, char *txt, ... )
{
   D_MOBILE *around;
   ITERATOR Iter;
   va_list args;
   char buf[MAX_BUFFER];

   va_start( args, txt );
   vsnprintf( buf, MAX_BUFFER, txt, args );
   va_end( args );

   AttachIterator( &Iter, dMob->room->mobiles );
   while( ( around = (D_MOBILE *)NextInList( &Iter ) ) != NULL )
   {
      if( around == dMob || around->position < POS_RESTRAINED )
         continue;
      else
         text_to_mobile_j( around, type, "%s", buf );
   }
   DetachIterator( &Iter );
}

void echo_room( D_ROOM *room, char *type, char *txt, ... )
{
   D_MOBILE *mobs;
   ITERATOR Iter;
   va_list args;
   char buf[MAX_BUFFER];

   va_start( args, txt );
   vsnprintf( buf, MAX_BUFFER, txt, args );
   va_end( args );

   AttachIterator( &Iter, room->mobiles );
   while( ( mobs = NextInList( &Iter ) ) != NULL )
      text_to_mobile_j( mobs, type, "%s", buf );
   DetachIterator( &Iter );
}

void communicate(D_MOBILE *dMob, char *txt, int range)
{
   D_MOBILE *xMob;
   ITERATOR Iter;
   json_t *json = json_object();
   json_t *data = json_object();

   switch(range)
   {
      default:
         bug("Communicate: Bad Range %d.", range);
         return;
      case COMM_LOCAL:  /* everyone is in the same room for now... */
         json_object_set_new( json, "type", json_string( "say" ) );
         json_object_set_new( data, "from", json_string( MOBNAME( dMob ) ) );
         json_object_set_new( data, "message", json_string( txt ) );
         json_object_set_new( json, "data", data );

         AttachIterator(&Iter, dMob->room->mobiles);
         while ((xMob = (D_MOBILE *) NextInList(&Iter)) != NULL)
         {
            char *dump = json_dumps( json, 0 );
            send_json_m( xMob, "%s", dump );
            free( dump );
         }
         DetachIterator(&Iter);
         break;
      case COMM_CHAT:
         json_object_set_new( json, "type", json_string( "chat" ) );
         json_object_set_new( data, "from", json_string( MOBNAME( dMob ) ) );
         json_object_set_new( data, "message", json_string( txt ) );
         json_object_set_new( json, "data", data );

         AttachIterator(&Iter, dmobile_list);
         while ((xMob = (D_MOBILE *) NextInList(&Iter)) != NULL)
         {
            char *dump = json_dumps( json, 0 );
            send_json_m( xMob, "%s", dump );
            free( dump );
         }
         DetachIterator(&Iter);
         break;

      case COMM_LOG:
         json_object_set_new( json, "type", json_string( "log" ) );
         json_object_set_new( data, "message", json_string( txt ) );
         json_object_set_new( json, "data", data );
         AttachIterator(&Iter, dmobile_list);
         while ((xMob = (D_MOBILE *) NextInList(&Iter)) != NULL)
         {
           if (!IS_ADMIN(xMob))
              continue;
            char *dump = json_dumps( json, 0 );
            send_json_m( xMob, "%s", dump );
            free( dump );
         }
         DetachIterator(&Iter);
         break;
   }
   json_decref( json );
}

bool flush_output( D_SOCKET *dsock )
{
   /* nothing to send */
   if( dsock->outbuf[0] == '\0' && !( dsock->bust_prompt && dsock->state == STATE_PLAYING ) )
      return TRUE;

   /* bust a prompt */
   /* if( dsock->state == STATE_PLAYING && dsock->bust_prompt ) */
   {
      json_t *json = json_object();

      parse_prompt( dsock->player ); //Does nothing, except get rid of the 'unused function' error
      /* text_to_buffer( dsock, "\n%s", parse_prompt( dsock->player ) ); */ /* Get rid of the traditional prompt */
      dsock->bust_prompt = FALSE;
      json_object_set_new( json, "type", json_string( "prompt" ) );
      json_object_set_new( json, "data", player_to_json( dsock->player, FALSE ) );
      char *dump = json_dumps( json, 0 );
      text_to_buffer( dsock, "%c%s%c", (char)2, dump, (char)3 );
      free( dump );
      json_decref( json );
   }

   /*
    * Send the buffer, and return FALSE
    * if the write fails.
    */
   if( dsock->account && dsock->account->acceptANSI == TRUE )
      text_to_socket( dsock, "%s", parse_color_codes( dsock->outbuf ) );
   else if( dsock->account && dsock->account->acceptANSI == FALSE )
      text_to_socket( dsock, "%s",  strip_color_codes( dsock->outbuf ) );
   else
      text_to_socket( dsock, "%s",  dsock->outbuf );

   /* reset the top pointer */
   dsock->outbuf[0] = '\0';
   /* Success */
   return TRUE;
}

static char *parse_prompt( D_MOBILE *dMob )
{
   static char result[MAX_BUFFER * 2 ];
   char *orig, *start;
   
   if( !dMob->prompt )
      dMob->prompt = strdup( "<&Y%h&W/&Y%Hhp&W>" );
      
   start = result;
   orig  = dMob->prompt;
   
   for( ; *orig; orig++ )
   {
      if( *orig == '%' ) //background color
      {
         orig++;
         switch( *orig )
         {
            case 'h':
               start += sprintf( start, "%i", dMob->cur_hp );
               break;
            case 'H':
               start += sprintf( start, "%i", dMob->max_hp );
               break;
            default:
               start += sprintf( start, "$%c", *orig );
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
   
   return result;
}
