/*
 * This file contains all sorts of utility functions used
 * all sorts of places in the code.
 */
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>

/* include main header file */
#include "mud.h"

/*
 * Check to see if a given name is
 * legal, returning FALSE if it
 * fails our high standards...
 */
bool check_name(const char *name)
{
  int size, i;
  char filename[MAX_STRING_LENGTH];
  snprintf( filename, MAX_STRING_LENGTH,  "../players/%c/%c%s.json",
        toupper(name[0]), toupper(name[0]), name + 1 );//Make sure the name is capitalized for the filename

  if ((size = strlen(name)) < 3 || size > 24)
    return FALSE;

  for (i = 0 ;i < size; i++)
    if (!isalpha(name[i])) return FALSE;

  FILE *fp;
  if( ( fp = fopen( filename, "r" ) ) != NULL ) //Character already exists...
  {
     fclose( fp );
     return FALSE;
  }

  return TRUE;
}

D_AREA *new_area()
{
   D_AREA *area = calloc( 1, sizeof( D_AREA ) );
   area->name = NULL;
   area->author = NULL;
   area->filename = NULL;

   area->objects = AllocList();
   area->mobiles = AllocList();
   area->rooms   = AllocList();
   area->resets  = AllocList();

   area->r_low = 0;
   area->r_hi  = 0;
   area->m_low = 0;
   area->m_hi  = 0;
   area->o_low = 0;
   area->o_hi  = 0;

   area->last_reset = time(NULL);
   area->reset_interval = 15;

   return area;
}

void free_area( D_AREA *area )
{
   ITERATOR Iter;

   free( area->name );
   free( area->author );
   free( area->filename );
   free( area->reset_script );

   room_cleanup( area->rooms );
   FreeList( area->rooms );

   D_MOBILE *pMob;
   AttachIterator( &Iter, area->mobiles );
   while( ( pMob = (D_MOBILE *)NextInList( &Iter ) ) != NULL )
   {
      DetachFromList( pMob, area->mobiles );
      DetachFromList( pMob, mobile_protos );
      free_mobile( pMob );
   }
   DetachIterator( &Iter );
   FreeList( area->mobiles );

   D_OBJECT *pObj;
   AttachIterator( &Iter, area->objects );
   while( ( pObj = (D_OBJECT *)NextInList( &Iter ) ) != NULL )
   {
      DetachFromList( pObj, area->objects );
      DetachFromList( pObj, object_protos );
      free_object( pObj );
   }
   DetachIterator( &Iter );
   FreeList( area->objects );

   D_RESET *pReset;
   AttachIterator( &Iter, area->resets );
   while( ( pReset = (D_RESET *)NextInList( &Iter ) ) != NULL )
   {
      free_reset( pReset );
   }
   DetachIterator( &Iter );
   FreeList( area->resets );

   free( area );

   return;
}
D_EXIT *new_exit()
{
   D_EXIT *exit = calloc( 1, sizeof( D_EXIT ) );

   exit->name = NULL;
   exit->farside_name = NULL;
   exit->to_room = NULL;
   exit->to_vnum = 0;
   exit->exit = EXIT_FREE;
   exit->lock = LOCK_FREE;
   exit->lock_level = 0;
   exit->to_vnum = 0;

   return exit;
}

void free_exit( D_EXIT *exit )
{
   free( exit->name );
   free( exit->farside_name );
   free( exit );

   return;
}

D_ROOM *new_room()
{
   D_ROOM *room = calloc( 1, sizeof( D_ROOM ) );
   room->name = NULL;
   room->description = NULL;
   room->mobiles = AllocList();
   room->objects = AllocList();
   room->exits   = AllocList();
   room->scripts = AllocList();

   AttachToList( room, droom_list );

   return room;
}

D_RESET *new_reset()
{
   D_RESET *reset = calloc( 1, sizeof( D_RESET ) );
   reset->location = 1;
   reset->what     = NULL;
   reset->data     = json_object();
   reset->type     = MAX_RESET;

   return reset;
}

void free_reset( D_RESET *reset )
{
   json_object_clear( reset->data );
   json_decref( reset->data );
   free( reset );
}

void rand_str(char *dest, size_t length) 
{
   char charset[] = "0123456789"
                    "abcdefghijklmnopqrstuvwxyz"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

   while (length-- > 0) 
   {
      size_t index = (double) rand() / RAND_MAX * (sizeof charset - 1);
      *dest++ = charset[index];
   }
   *dest = '\0';
}

char *gen_guid()
{
   char r[40];
   rand_str( r, sizeof(r)-1 );
   return strdup( r );
}

D_EXIT *get_exit_by_name( D_ROOM *room, char *name )
{
   ITERATOR Iter;
   D_EXIT *exit;
   if( !strcasecmp( name, "n" ) )
      return get_exit_by_name( room, "north" );
   if( !strcasecmp( name, "s" ) )
      return get_exit_by_name( room, "south" );
   if( !strcasecmp( name, "e" ) )
      return get_exit_by_name( room, "east" );
   if( !strcasecmp( name, "w" ) )
      return get_exit_by_name( room, "west" );
   if( !strcasecmp( name, "ne" ) )
      return get_exit_by_name( room, "northeast" );
   if( !strcasecmp( name, "se" ) )
      return get_exit_by_name( room, "southeast" );
   if( !strcasecmp( name, "sw" ) )
      return get_exit_by_name( room, "southwest" );
   if( !strcasecmp( name, "nw" ) )
      return get_exit_by_name( room, "northwest" );
   if( !strcasecmp( name, "u" ) )
      return get_exit_by_name( room, "up" );
   if( !strcasecmp( name, "d" ) )
      return get_exit_by_name( room, "down" );

   AttachIterator( &Iter, room->exits );
   while( ( exit = NextInList( &Iter ) ) != NULL )
   {
      if( !strcasecmp( name, exit->name ) )
         break;
   }
   DetachIterator( &Iter );

   return exit;
}

D_MOBILE *get_mobile_by_vnum( unsigned int vnum )
{
   ITERATOR Iter;
   D_MOBILE *proto;

   AttachIterator( &Iter, mobile_protos );
   while( ( proto = (D_MOBILE *)NextInList( &Iter ) ) != NULL )
      if( proto->vnum == vnum )
         break;
   DetachIterator( &Iter );
   return proto;
}

D_MOBILE *spawn_mobile( unsigned int vnum )
{
   D_MOBILE *proto = get_mobile_by_vnum( vnum );

   if( proto == NULL )
      return NULL;

   D_MOBILE *mob = new_mobile();
   mob->vnum = proto->vnum;
   if( mob->name ) free( mob->name );
   mob->name = strdup( proto->name ? proto->name : "new mobile" );
   if( mob->sdesc ) free( mob->sdesc );
   mob->sdesc = strdup( proto->sdesc ? proto->sdesc : "new mobile" );
   if( mob->ldesc ) free( mob->ldesc );
   mob->ldesc = strdup( proto->ldesc ? proto->ldesc : "a newly created mobile" );
   if( mob->race ) free( mob->race );
   mob->race = strdup( proto->race ? proto->race : "Human" );
   if( mob->citizenship ) free( mob->citizenship );
   mob->citizenship = strdup( proto->citizenship ? proto->citizenship : "None" );
   if( mob->association ) free( mob->association );
   mob->association = strdup( proto->association ? proto->association : "None" );
   if( mob->eyecolor ) free( mob->eyecolor );
   mob->eyecolor = strdup( proto->eyecolor ? proto->eyecolor : "brown" );
   if( mob->skincolor ) free( mob->skincolor );
   mob->skincolor = strdup( proto->skincolor ? proto->skincolor : "white" );
   if( mob->eyeshape ) free( mob->eyeshape );
   mob->eyeshape = strdup( proto->eyeshape ? proto->eyeshape : "round" );
   if( mob->hairstyle ) free( mob->hairstyle );
   mob->hairstyle = strdup( proto->hairstyle ? proto->hairstyle : "short" );
   if( mob->haircolor ) free( mob->haircolor );
   mob->haircolor = strdup( proto->haircolor ? proto->haircolor : "brown" );
   if( mob->build ) free( mob->build );
   mob->build = strdup( proto->build ? proto->build : "thin" );
   if( mob->height ) free( mob->height );
   mob->height = strdup( proto->height ? proto->height : "average" );
   mob->btc = proto->btc;
   mob->brains = proto->brains;
   mob->brawn = proto->brawn;
   mob->coordination = proto->coordination;
   mob->stamina = proto->stamina;
   mob->senses = proto->senses;
   mob->luck = proto->luck;
   mob->cool = proto->cool;
   mob->cur_hp = proto->max_hp;
   mob->max_hp = proto->max_hp;

   AttachToList( mob, dmobile_list );
   return mob;
}

bool is_name( char *str, char *namelist )
{
   char one[MAX_STRING_LENGTH];

   while( namelist[0] != '\0' )
   {
      if( is_prefix( str, namelist ) )
         return TRUE;
      namelist = one_arg( namelist, one );
   }
   return FALSE;
}
   
D_MOBILE *get_mobile_list( const char *name, LIST *list )
{
   ITERATOR Iter;
   D_MOBILE *mob;

   AttachIterator( &Iter, list );
   while( ( mob = NextInList( &Iter ) ) != NULL )
      if( is_name( (char*)name, mob->name ) || !strcmp( mob->guid, name ) )
         break;
   DetachIterator( &Iter );
   return mob;
}

/* Completely destroys a room and everything in it.
 * Arguments: A pointer to a room.
 * Returns: None.
 */
void free_room( D_ROOM *room )
{
   if( !room )
      return;

   ITERATOR Iter;
   D_MOBILE *mob;
   D_OBJECT *obj;
   D_EXIT *exit;
   char *script;

   free( room->name );
   free( room->description );

   AttachIterator( &Iter, room->mobiles );
   while( ( mob = (D_MOBILE *) NextInList( &Iter ) ) != NULL )
   {
      DetachFromList( mob, dmobile_list );
      DetachFromList( mob, room->mobiles );
      free_mobile( mob );
   }
   DetachIterator( &Iter );
   AttachIterator( &Iter, room->exits );
   while( ( exit = (D_EXIT *) NextInList( &Iter ) ) != NULL )
   {
      DetachFromList( exit, room->exits );
      free_exit( exit );
   }
   DetachIterator( &Iter );
   AttachIterator( &Iter, room->objects );
   while( ( obj = (D_OBJECT *) NextInList( &Iter ) ) != NULL )
   {
      DetachFromList( obj, dobject_list );
      free_object( obj );
   }
   DetachIterator( &Iter );
   AttachIterator( &Iter, room->scripts );
   while( ( script = (char *)NextInList( &Iter ) ) != NULL )
   {
      DetachFromList( script, room->scripts );
      free( script );
   }
   DetachIterator( &Iter );

   FreeList( room->mobiles );
   FreeList( room->exits );
   FreeList( room->objects );
   FreeList( room->scripts );

   DetachFromList( room, droom_list );

   free( room );
   return;
}

D_MOBILE *new_mobile()
{
   D_MOBILE *mob = NULL;
   if( ( mob = calloc( 1, sizeof( D_MOBILE ) ) ) == NULL )
   {
      bug( "Error: (System) Unable to allocate memory for new mobile." );
      abort();
      return NULL;
   }

   clear_mobile( mob );
   return mob;
}

void clear_mobile(D_MOBILE *dMob)
{
   memset(dMob, 0, sizeof(*dMob));
   dMob->quit         =  FALSE;

   dMob->race         =  NULL;
   dMob->eyecolor     =  NULL;
   dMob->haircolor    =  NULL;
   dMob->eyeshape     =  NULL;
   dMob->hairstyle    =  NULL;
   dMob->skincolor    =  NULL;
   dMob->build        =  NULL;
   dMob->height       =  NULL;
   dMob->age          =  18;

   dMob->level        =  LEVEL_PLAYER;
   dMob->events       =  AllocList();
   dMob->com_cmd_list   =  AllocList();
   dMob->act_cmd_list   =  AllocList();
   dMob->ooc_cmd_list   =  AllocList();
   dMob->wiz_cmd_list   =  AllocList();
   dMob->scripts        =  AllocList();
   dMob->offer_right  =  calloc( 1, sizeof( D_OFFER ) );
   dMob->offer_left   =  calloc( 1, sizeof( D_OFFER ) );
   dMob->guid         =  gen_guid();

   for( size_t pos = WEAR_HEAD; pos < MAX_WEAR; pos++ )
   {
      dMob->equipment[pos] = calloc( 1, sizeof( D_EQUIPMENT ) );
      dMob->equipment[pos]->worn[0] = NULL;
      dMob->equipment[pos]->worn[1] = NULL;
   }

   for( size_t pos = BODY_HEAD; pos < MAX_BODY; pos++ )
   {
      dMob->body[pos] = calloc( 1, sizeof( D_BODYPART ) );
      dMob->body[pos]->cur_hp = 100;
      dMob->body[pos]->max_hp = 100;
      dMob->body[pos]->wound_trauma = TRAUMA_NONE;
      dMob->body[pos]->blunt_trauma = TRAUMA_NONE;
      dMob->body[pos]->burn_trauma  = TRAUMA_NONE;
   }
  
   dMob->btc          =  0;
   dMob->brains       =  6;
   dMob->brawn        =  6;
   dMob->coordination =  6;
   dMob->stamina      =  6;
   dMob->senses       =  6;
   dMob->luck         =  6;
   dMob->cool         =  6;

   dMob->skills       =  calloc( 1, sizeof( SKILLS ) );

   dMob->position     = POS_STANDING;
}

D_ACCOUNT *new_account()
{
   D_ACCOUNT *account = NULL;
   if( ( account = calloc( 1, sizeof( D_ACCOUNT ) ) ) == NULL )
   {
      bug( "ERROR: (System) Unable to allocate memory for new account." );
      abort();
      return NULL;
   }

   account->characters = AllocList();
   account->name = NULL;
   account->password = NULL;
   account->email = NULL;
   account->acceptANSI = TRUE;
   return account;
}

void free_account( D_ACCOUNT *account )
{
   if( !account )
      return;

   ITERATOR Iter;
   char *name;

   free( account->name );
   free( account->password );
   free( account->email );

   AttachIterator( &Iter, account->characters );
   while( ( name = (char *) NextInList( &Iter ) ) != NULL )
      free( name );
   DetachIterator( &Iter );
   FreeList( account->characters );
   return;
}

void free_mobile_proto( D_MOBILE *dMob )
{
  EVENT_DATA *pEvent;
  ITERATOR Iter;

  free_mobile( dMob );
  return;

  AttachIterator(&Iter, dMob->events);
  while ((pEvent = (EVENT_DATA *) NextInList(&Iter)) != NULL)
    dequeue_event(pEvent);
  DetachIterator(&Iter);
  FreeList(dMob->events);

  /* free allocated memory */
  free( dMob->name );
  free( dMob->password );
  free( dMob->race );
  free( dMob->citizenship );
  free( dMob->association );
  free( dMob->eyecolor );
  free( dMob->prompt );
  free( dMob->offer_right );
  free( dMob->offer_left );
  free( dMob->sdesc );
  free( dMob->ldesc );
  free( dMob->guid );
  free( dMob->skills );

  for( size_t i = WEAR_HEAD; i < WEAR_NONE; i++ )
  {
     if( dMob->equipment[i]->worn[0] )
        free_object( dMob->equipment[i]->worn[0] );
     if( dMob->equipment[i]->worn[1] )
        free_object( dMob->equipment[i]->worn[1] );
     free( dMob->equipment[i] );
  }

   if( SizeOfList( dMob->scripts ) > 0 )
   {
      char *script;
      AttachIterator( &Iter, dMob->scripts );
      while( ( script = (char *)NextInList( &Iter ) ) != NULL )
      {
         free( script );
      }
      DetachIterator( &Iter );
   }
   FreeList( dMob->scripts );

  DetachFromList( dMob, mobile_protos );
  PushStack( dMob, dmobile_free );
  free( dMob );
}
void free_mobile(D_MOBILE *dMob)
{
  EVENT_DATA *pEvent;
  ITERATOR Iter;

  if( dMob->room )
     DetachFromList( dMob, dMob->room->mobiles );
  DetachFromList( dMob, dmobile_list);

  if (dMob->socket) dMob->socket->player = NULL;

  AttachIterator(&Iter, dMob->events);
  while ((pEvent = (EVENT_DATA *) NextInList(&Iter)) != NULL)
    dequeue_event(pEvent);
  DetachIterator(&Iter);
  FreeList(dMob->events);

  if( dMob->reset )
  {
     dMob->reset->what = NULL;
  }

  /* free allocated memory */
  free( dMob->name );
  free( dMob->password );
  free( dMob->race );
  free( dMob->citizenship );
  free( dMob->association );
  free( dMob->eyecolor );
  free( dMob->eyeshape );
  free( dMob->haircolor );
  free( dMob->hairstyle );
  free( dMob->skincolor );
  free( dMob->build );
  free( dMob->height );
  free( dMob->prompt );
  free( dMob->offer_right );
  free( dMob->offer_left );
  free( dMob->sdesc );
  free( dMob->ldesc );
  free( dMob->guid );
  free( dMob->skills );

  if( dMob->hold_left )  free_object( dMob->hold_left );
  if( dMob->hold_right ) free_object( dMob->hold_right );


   for( size_t i = WEAR_HEAD; i < WEAR_NONE; i++ )
   {
      if( dMob->equipment[i]->worn[0] )
        free_object( dMob->equipment[i]->worn[0] );
      if( dMob->equipment[i]->worn[1] )
        free_object( dMob->equipment[i]->worn[1] );
      free( dMob->equipment[i] );
   }

   for( size_t i = BODY_HEAD; i < MAX_BODY; i++ )
   {
      free( dMob->body[i] );
   }

   if( SizeOfList( dMob->com_cmd_list ) > 0 )
   {
      D_COMMAND *pCmd;
      AttachIterator( &Iter, dMob->com_cmd_list );
      while( ( pCmd = (D_COMMAND *)NextInList( &Iter ) ) != NULL )
      {
        free( pCmd->arg );
        free( pCmd );
      }
      DetachIterator( &Iter );

   }
   FreeList( dMob->com_cmd_list );
   if( SizeOfList( dMob->act_cmd_list ) > 0 )
   {
      D_COMMAND *pCmd;
      AttachIterator( &Iter, dMob->act_cmd_list );
      while( ( pCmd = (D_COMMAND *)NextInList( &Iter ) ) != NULL )
      {
        free( pCmd->arg );
        free( pCmd );
      }
      DetachIterator( &Iter );
   }
   FreeList( dMob->act_cmd_list );
   if( SizeOfList( dMob->ooc_cmd_list ) > 0 )
   {
      D_COMMAND *pCmd;
      AttachIterator( &Iter, dMob->ooc_cmd_list );
      while( ( pCmd = (D_COMMAND *)NextInList( &Iter ) ) != NULL )
      {
        free( pCmd->arg );
        free( pCmd );
      }
      DetachIterator( &Iter );
   }
   FreeList( dMob->ooc_cmd_list );
   if( SizeOfList( dMob->wiz_cmd_list ) > 0 )
   {
     D_COMMAND *pCmd;
     AttachIterator( &Iter, dMob->wiz_cmd_list );
     while( ( pCmd = (D_COMMAND *)NextInList( &Iter ) ) != NULL )
     {
        free( pCmd->arg );
        free( pCmd );
     }
     DetachIterator( &Iter );
   } 
   FreeList( dMob->wiz_cmd_list );

   if( SizeOfList( dMob->scripts ) > 0 )
   {
      char *script;
      AttachIterator( &Iter, dMob->scripts );
      while( ( script = (char *)NextInList( &Iter ) ) != NULL )
      {
         free( script );
      }
      DetachIterator( &Iter );
   }
   FreeList( dMob->scripts );
   PushStack( dMob, dmobile_free );
   free( dMob );
}


/*
 * Loading of help files, areas, etc, at boot time.
 */
void load_muddata(bool fCopyOver)
{  
  load_helps();

  /* copyover */
  if (fCopyOver)
    copyover_recover();
}

char *get_time()
{
  static char buf[16];
  char *strtime;
  int i;

  strtime = ctime(&current_time);
  for (i = 0; i < 15; i++)   
    buf[i] = strtime[i + 4];
  buf[15] = '\0';

  return buf;
}

/* Recover from a copyover - load players */
void copyover_recover()
{     
  D_MOBILE *dMob;
  D_SOCKET *dsock;
  FILE *fp;
  char name [100];
  char host[MAX_BUFFER];
  int desc;
      
  log_string("Copyover recovery initiated");
   
  if ((fp = fopen(COPYOVER_FILE, "r")) == NULL)
  {  
    log_string("Copyover file not found. Exitting.");
    exit (1);
  }
      
  /* In case something crashes - doesn't prevent reading */
  unlink(COPYOVER_FILE);
    
  for (;;)
  {  
    fscanf(fp, "%d %s %s\n", &desc, name, host);
    if (desc == -1)
      break;

    dsock = malloc(sizeof(*dsock));
    clear_socket(dsock, desc);
  
    dsock->hostname     =  strdup(host);
    AttachToList(dsock, dsock_list);
 
    /* load player data */
    if ((dMob = load_player(name)) != NULL)
    {
      /* attach to socket */
      dMob->socket     =  dsock;
      dsock->player    =  dMob;
  
      /* attach to mobile list */
      AttachToList(dMob, dmobile_list);

      /* initialize events on the player */
      //init_events_player(dMob);
    }
    else /* ah bugger */
    {
      close_socket(dsock, FALSE);
      continue;
    }
   
    /* Write something, and check if it goes error-free */
    if (!text_to_socket(dsock, "\n\r <*>  And before you know it, everything has changed  <*>\n\r"))
    { 
      close_socket(dsock, FALSE);
      continue;
    }
  
    /* make sure the socket can be used */
    dsock->bust_prompt    =  TRUE;
    dsock->lookup_status  =  TSTATE_DONE;
    dsock->state          =  STATE_PLAYING;

    /* negotiate compression */
    text_to_buffer(dsock, (char *) compress_will2);
    text_to_buffer(dsock, (char *) compress_will);
  }
  fclose(fp);
}     

D_MOBILE *check_reconnect(char *player)
{
  D_MOBILE *dMob;
  ITERATOR Iter;

  AttachIterator(&Iter, dmobile_list);
  while ((dMob = (D_MOBILE *) NextInList(&Iter)) != NULL)
  {
    if (!strcasecmp(dMob->name, player))
    {
      if (dMob->socket)
        close_socket(dMob->socket, TRUE);

      break;
    }
  }
  DetachIterator(&Iter);

  return dMob;
}

void show_mob_obj_list( D_MOBILE *dMob, LIST *list, size_t indent )
{
   ITERATOR Iter;
   D_OBJECT *obj;

   if( SizeOfList( list ) < 1 )
   {
      text_to_mobile_j( dMob, "text", "%*s", indent, "Nothing." );
      return;
   }
   AttachIterator( &Iter, list );
   while( ( obj = (D_OBJECT *)NextInList( &Iter ) ) != NULL )
      text_to_mobile_j( dMob, "object", "%*s", indent, obj->sdesc );
   DetachIterator( &Iter );

   return;
}

char *proper( const char *word )
{
   static char buf[MAX_BUFFER];
   strncpy( buf, word, MAX_BUFFER );
   buf[0] = toupper( buf[0] );
   return buf;
}

void sentence_case( char *sentence )
{
   if( !sentence )
      return;

   int len = 0;
   sentence[0] = toupper( sentence[0] );
   len = strlen( sentence );
   for( int i = 0; i < len; i++ )
   {
      if( sentence[i] == '.' )
      {
         i++;
         while( isspace( sentence[i] ) )
            i++;
         sentence[i] = toupper( sentence[i] );
      }
   }
   return;
}
   

D_ROOM *get_room_by_vnum( unsigned int vnum )
{
   ITERATOR Iter;
   D_ROOM *room = NULL;

   AttachIterator( &Iter, droom_list );
   while( ( room = (D_ROOM *)NextInList( &Iter ) ) != NULL )
   {
      if( room->vnum == vnum )
         break;
   }
   DetachIterator( &Iter );
   
   return room;
}

D_ROOM *mob_to_room( D_MOBILE *dMob, D_ROOM *to )
{
   D_ROOM *from = dMob->room;
   if( to == NULL )
      return NULL;
   
   if( from ) DetachFromList( dMob, from->mobiles );
   dMob->room = to;
   AppendToList( dMob, to->mobiles );

   return to;
}

void check_mobiles()
{
   ITERATOR Iter;
   D_MOBILE *pMob;

   AttachIterator( &Iter, dmobile_list );
   while( ( pMob = NextInList( &Iter ) ) != NULL )
   {
      //Check for any outstanding gives
      if( pMob->offer_right->what != NULL && difftime( time(NULL), pMob->offer_right->when ) > 20 )
      {
         text_to_mobile_j( pMob->offer_right->to, "text", "You ignore %s\'s offer of  %s.", 
               pMob->name, pMob->offer_right->what->sdesc );
         text_to_mobile_j( pMob, "text", "%s ignores your offer of %s.", 
               pMob->offer_right->to->name, 
               pMob->offer_right->what->sdesc );
         pMob->offer_right->what = NULL;
         pMob->offer_right->to = NULL;
         pMob->offer_right->when = 0;
      }
      if( pMob->offer_left->what != NULL && difftime( time(NULL), pMob->offer_left->when ) > 20 )
      {
         text_to_mobile_j( pMob->offer_left->to, "text", "You ignore %s\'s offer of  %s.", 
               pMob->name, pMob->offer_left->what->sdesc );
         text_to_mobile_j( pMob, "text", "%s ignores your offer of %s.", 
               pMob->offer_left->to->name, 
               pMob->offer_left->what->sdesc );
         pMob->offer_left->what = NULL;
         pMob->offer_left->to = NULL;
         pMob->offer_left->when = 0;
      }
      if( SizeOfList( pMob->act_cmd_list ) > 0 )
      {
         ITERATOR cmdIter;
         struct command_data *cmd = NULL;

         AttachIterator( &cmdIter, pMob->act_cmd_list );
         cmd = NextInList( &cmdIter );
         cmd->func( cmd->dMob, cmd->arg );
         //free( cmd->arg );
         DetachFromList( cmd, pMob->act_cmd_list );
         free( cmd );
         DetachIterator( &cmdIter );
      }
      if( SizeOfList( pMob->com_cmd_list ) > 0 )
      {
         ITERATOR cmdIter;
         struct command_data *cmd = NULL;

         AttachIterator( &cmdIter, pMob->com_cmd_list );
         cmd = NextInList( &cmdIter );
         cmd->func( cmd->dMob, cmd->arg );
         //free( cmd->arg );
         DetachFromList( cmd, pMob->com_cmd_list );
         free( cmd );
         DetachIterator( &cmdIter );
      }
      if( SizeOfList( pMob->ooc_cmd_list ) > 0 )
      {
         ITERATOR cmdIter;
         struct command_data *cmd = NULL;

         AttachIterator( &cmdIter, pMob->ooc_cmd_list );
         cmd = NextInList( &cmdIter );
         cmd->func( cmd->dMob, cmd->arg );
         //free( cmd->arg );
         DetachFromList( cmd, pMob->ooc_cmd_list );
         free( cmd );
         DetachIterator( &cmdIter );
      }
      if( SizeOfList( pMob->wiz_cmd_list ) > 0 )
      {
         ITERATOR cmdIter;
         struct command_data *cmd = NULL;

         AttachIterator( &cmdIter, pMob->wiz_cmd_list );
         cmd = NextInList( &cmdIter );
         cmd->func( cmd->dMob, cmd->arg );
         //free( cmd->arg );
         DetachFromList( cmd, pMob->wiz_cmd_list );
         free( cmd );
         DetachIterator( &cmdIter );
      }
      if( pMob->quit == TRUE )
      {

         pMob->socket->player = NULL;
         pMob->socket->state = STATE_MAIN_MENU;
         extern const char *mainMenu;
         text_to_buffer( pMob->socket, mainMenu );
         text_to_mobile_j( pMob, "ui_command", "hide_all_windows" );
         free_mobile(pMob);
         return;
      }
   }
   DetachIterator( &Iter );

   return;
}

void check_objects()
{
}

void check_rooms()
{
}

void check_areas( bool force_reset )
{
   ITERATOR Iter;
   D_AREA *pArea;

   AttachIterator( &Iter, darea_list );
   while( (pArea = (D_AREA *)NextInList( &Iter ) ) != NULL )
   {
      if( ( difftime( current_time, pArea->last_reset ) > pArea->reset_interval*60  ) || force_reset == TRUE )
      {
         reset_area( pArea );
      }
   }
   DetachIterator( &Iter );
}

size_t roll( size_t min, size_t max )
{
   return rand() % max + min;
}

size_t dice( char *str2 )
{
   size_t result = 0, num = 0, size = 0, mod = 0, i = 0, len = 0;
   char operator = '\0', buf[MAX_STRING_LENGTH], *str;
   strncpy( buf, str2, MAX_STRING_LENGTH );
   len = strlen( buf );
   str = buf;

   for( i = 0; i < len; i++ )
   {
      if( toupper( str[i] ) == 'D' )
      {
         str[i] = '\0';
         break;
      }
   }

   num = strtoul( str, NULL, 10 );
   i++;
   str += i;
   len = strlen( str );

   for( i = 0; i < len; i++ )
   {
      if( str[i] == '+' || str[i] == '-' || str[i] == '*' || str[i] == '/' || str[i] == '^' )
      {
         operator = str[i];
         str[i] = '\0';
         break;
      }
   }

   size = strtoul( str, NULL, 10 );
   i++;
   str += i;
   mod = strtoul( str, NULL, 10 );

   if( num == 0 || size == 0 )
   {
      return 0;
   }

   for( i = 0; i < num; i++ )
   {
      result += roll( 1, size );
   }

   switch( operator )
   {
      case '+':
         result += mod;
         break;
      case '-':
         result -= mod;
         break;
      case '/':
         if( mod == 0 ) mod = 1;
         result /= mod;
         break;
      case '*':
         result *= mod;
         break;
      case '^':
         result = pow( result, mod );
         break;
      default:
         break;
   }

   return result;
}

D_OBJECT *get_armor_pos( D_MOBILE *dMob, enum wear_pos_tb pos )
{
   if( dMob->equipment[pos]->worn[0] != NULL && dMob->equipment[pos]->worn[0]->type == ITEM_ARMOR )
      return dMob->equipment[pos]->worn[0];
   else if( dMob->equipment[pos]->worn[1] != NULL && dMob->equipment[pos]->worn[1]->type == ITEM_ARMOR )
      return dMob->equipment[pos]->worn[1];
   else
      return NULL;
}

unsigned int calc_brawn( D_MOBILE *dMob )
{
   return dMob->brawn;
}

unsigned int calc_brains( D_MOBILE *dMob )
{
   return dMob->brains;
}

unsigned int calc_senses( D_MOBILE *dMob )
{
   return dMob->senses;
}

unsigned int calc_stamina( D_MOBILE *dMob )
{
   return dMob->stamina;
}

unsigned int calc_cool( D_MOBILE *dMob )
{
   return dMob->cool;
}

unsigned int calc_coordination( D_MOBILE *dMob )
{
   return dMob->coordination;
}

unsigned int calc_luck( D_MOBILE *dMob )
{
   return dMob->luck;
}

void mobile_cleanup( LIST *list )
{
   ITERATOR Iter;

   AttachIterator( &Iter, list );
   D_MOBILE *pMob;
   while( ( pMob = (D_MOBILE *)NextInList( &Iter ) ) != NULL )
   {
      DetachFromList( pMob, list );
      free_mobile( pMob );
   }
   DetachIterator( &Iter );
}

void object_cleanup( LIST *list )
{

   ITERATOR Iter;

   AttachIterator( &Iter, list );
   D_OBJECT *pObj;
   while( ( pObj = (D_OBJECT *)NextInList( &Iter ) ) != NULL )
   {
      DetachFromList( pObj, dobject_list );
      free_object( pObj );
   }
   DetachIterator( &Iter );

   return;
}

void exit_cleanup( LIST *list )
{
   ITERATOR Iter;
   D_EXIT *pExit;

   AttachIterator( &Iter, list );
   while( ( pExit = (D_EXIT*)NextInList( &Iter ) ) != NULL )
   {
      DetachFromList( pExit, list );
      free_exit( pExit );
   }
   DetachIterator( &Iter );

   return;
}

void room_cleanup( LIST *list )
{

   ITERATOR Iter;
   D_ROOM *pRoom;

   AttachIterator( &Iter, list );
   while( ( pRoom = (D_ROOM *)NextInList( &Iter ) ) != NULL )
   {
      DetachFromList( pRoom, list );
      DetachFromList( pRoom, droom_list );
      free_room( pRoom );
   }
   DetachIterator( &Iter );

   return;
}

void area_cleanup( LIST *list )
{
   ITERATOR Iter;
   D_AREA *pArea;

   AttachIterator( &Iter, list );
   while( ( pArea = (D_AREA *)NextInList( &Iter ) ) != NULL )
   {
      DetachFromList( pArea, list );
      free_area( pArea );
   }
   DetachIterator( &Iter );

   return;
}

void socket_cleanup( LIST *list )
{
   ITERATOR Iter;
   D_SOCKET *dsock;

   AttachIterator( &Iter, dsock_list );
   while( ( dsock = (D_SOCKET *)NextInList( &Iter ) ) != NULL )
   {
      if( dsock->account )
         free_account( dsock->account );
      if( dsock->player )
         free_mobile( dsock->player );
   }
   DetachIterator( &Iter );

   return;
}

