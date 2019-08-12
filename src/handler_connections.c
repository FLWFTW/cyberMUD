#include "mud.h"

const char *mainMenu = "\r\nMain Menu\r\n"
                        "---------\r\n"
                        "   1) Change your account password.\r\n" 
                        "   2) Read the MOTD.\r\n"
                        "   3) Play an existing character.\r\n"
                        "   4) Create a new character.\r\n"
                        "   5) Delete an existing character.\r\n"
                        "   0) Log out of the system.\r\n"
                        "Which sounds good to you? (1,2,3,4,5,0)\r\n";

static void state_get_account( D_SOCKET *dsock, char *arg )
{
   if( !strcasecmp( arg, "new" ) )
   {
      text_to_buffer( dsock, "Please enter an account name to create an account: " );
      dsock->state = STATE_NEW_ACCOUNT;
      return;
   }
   snprintf( dsock->loginName, MAX_BUFFER, "%s", arg );
   text_to_buffer( dsock, "Password: " );
   text_to_buffer(dsock, (char *) dont_echo); 
   dsock->state = STATE_ASK_PASSWORD;
   return;
}

static void state_ask_password( D_SOCKET *dsock, char *arg )
{
   text_to_buffer(dsock, (char *) do_echo); 
   if( ( dsock->account = load_account( dsock->loginName,arg ) ) == NULL )
   {
      text_to_buffer( dsock, 
         "That username/password combination does not exist.\r\n"
         "Type 'new' to create a new account\r\nLogin: " );
      dsock->state = STATE_GET_ACCOUNT;
      return;
   }
   text_to_buffer( dsock, "Welcome!\r\n" );
   dsock->state = STATE_MAIN_MENU;
   text_to_buffer( dsock, mainMenu );
   return;
}

static void state_press_enter (D_SOCKET *dsock, char *arg )
{
   dsock->state = STATE_PLAYING;
   cmd_look( dsock->player, "" );
   return;
}

static void state_new_account( D_SOCKET *dsock, char *arg )
{
   FILE *fp;
   char filename[MAX_STRING_LENGTH];

   for( size_t i = 0; i < strlen( arg ); i++ )
      arg[i] = tolower( arg[i] );

   snprintf( filename, MAX_STRING_LENGTH, "../accounts/%c/%s.json", toupper(arg[0]), arg );

   if( ( fp = fopen( filename, "r" ) ) != NULL )
   {
      text_to_buffer( dsock, "\r\nThat account name is unavailable.\r\n" );
      fclose( fp );
      return;
   }

   dsock->account = new_account();
   dsock->account->name = strdup( arg );
   text_to_buffer( dsock, "Creating account \"%s\"...\r\nCreate a secure password: ", arg );
   text_to_buffer(dsock, (char *) dont_echo); 
   dsock->state = STATE_NEW_PASSWORD;

}

static void state_main_menu( D_SOCKET *dsock, char *arg )
{
   dsock->player = NULL;
   save_account( dsock->account );
   switch( atoi(arg) )
   {
      default:
      {
         text_to_buffer( dsock, "Please make a valid selection (1-5, 0).\r\n" );
         text_to_buffer( dsock, mainMenu );
         break;
      }
      case 1:
      {
         text_to_buffer( dsock, "\r\nEnter new password: " );
         dsock->state = STATE_NEW_PASSWORD;
         break;
      }
      case 2:
      {
         text_to_buffer(dsock, motd);
         break;
      }
      case 3:
      {
         char *name;
         if( dsock->account->characters == NULL || SizeOfList( dsock->account->characters ) == 0 )
         {
            text_to_buffer( dsock, "\r\nNo personnel assigned to this account. Register personnel (Option 4) prior to loading.\r\n" );
            break;
         }
         else
         {
            text_to_buffer( dsock, "Select one of the following personnel:\r\n" );
            ITERATOR Iter;
            AttachIterator(&Iter, dsock->account->characters );
            int i = 1;
            while ((name = (char *) NextInList(&Iter)) != NULL)
            {
               text_to_buffer( dsock, "%i. %s\r\n", i, name );
               i++;
            }
            DetachIterator( &Iter );
         }
         dsock->state = STATE_CHOOSE_CHAR;

         break;
      }
      case 4:
      {
         /*
         text_to_buffer( dsock, "----[ PERSONNEL REGISTRATION ]----\r\n" );
         text_to_buffer( dsock, " An unshaven immigrations control officer slowly reads to you from a paper,\r\n\r\n"
                                "     \"The Ministry of Personnel Affairs requires biographical information\r\n"
                                "      on all individuals applying for residency in The City. Providing false\r\n"
                                "      or fraudulent information is a crime and will result in your arrest,\r\n"
                                "      imprisonment, and expulsion from the city.\"\r\n\r\n"
                                " The officer glances over you and gruffly states, \"Look, I don't care what\r\n"
                                " the fuck you put in there  as long as it doesn't look fake. So just put in\r\n"
                                " a normal name--no crazy wastoid bullshit--put in physical features that are\r\n"
                                " at least close  to what you look like, and we'll call it a day. You're not\r\n"
                                " gonna live long enough for anyone to know the difference, anyways, so get\r\n"
                                " typing. I've got better shit to do.\"\r\n\r\n"
                                " Enter your name, or 'cancel' to end registration.\r\n\r\nName: ");
                                */
         text_to_socket( dsock, "%c{\"type\":\"ui_command\", \"data\":{\"text\":\"chargen_menu\"}}%c", (char)2, (char)3 );
         dsock->state = STATE_CHARGEN;
         break;
      }
      case 5:
      {
         text_to_buffer( dsock, "You selected delete an existing character.\r\n" );
         break;
      }
      case 0:
      {
         text_to_buffer( dsock, "\r\nCome back real soon!\r\n" );
         free_account( dsock->account );
         dsock->account = NULL;
         close_socket(dsock, FALSE);
         return;
      }
   }
   return;
}

static void handle_reconnect( D_MOBILE *dMob )
{
   ITERATOR Iter;
   D_MOBILE *old;
   D_SOCKET *dsock;

   AttachIterator( &Iter, dmobile_list );
   while( ( old = NextInList( &Iter ) ) != NULL )
   {
      if( IS_PC( old ) && !strcasecmp( old->name, dMob->name ) )
      {
         text_to_socket( old->socket, "\r\nConnection booted and/or hijacked. Goodbye!\r\n" );
         save_player(old);
         dsock = old->socket;
         dsock->player = NULL;
         free_mobile(old);
         free_account( dsock->account );
         dsock->account = NULL;
         close_socket(dsock, FALSE);
      }
   }
   DetachIterator( &Iter );
   return;
}

static void state_choose_char( D_SOCKET *dsock, char *arg )
{
   char *name;
   if( isdigit( arg[0] ) )
   {
      int cnum;
      if( ( cnum = atoi( arg ) ) == 0 )
      {
         text_to_buffer( dsock, mainMenu );
         dsock->state = STATE_MAIN_MENU;
         return;
      }
      else
      {
         if( cnum > SizeOfList( dsock->account->characters ) || cnum < 0 )
         {
            text_to_buffer( dsock, "Selection out of range.\r\n" );
            return;
         }
         ITERATOR Iter;
         AttachIterator(&Iter, dsock->account->characters );
         for( int i = 0; i < cnum; i++ )
         {
            name = (char *) NextInList( &Iter );
         }
         DetachIterator( &Iter );

         if( ( dsock->player = load_player( name ) ) == NULL )
         {
            text_to_buffer( dsock, "Your playerfile is either corrupt or missing.\r\nPlease contact an immortal immediately or email the Admin Team.\r\n" );
            return;
         }
         handle_reconnect( dsock->player );
         log_string("Player: %s has entered the game.", dsock->player->name);
         dsock->player->socket = dsock;
         AttachToList( dsock->player, dmobile_list );
         AppendToList( dsock->player, dsock->player->room->mobiles );
         echo_around( dsock->player, "text", "The world vibrates slightly and %s flickers into existance.", dsock->player->name );
         //init_events_player(dsock->player);
         //strip_event_socket(dsock, EVENT_SOCKET_IDLE);
         text_to_mobile_j( dsock->player, "text", "%s\n\n", motd);
         cmd_look( dsock->player, "" );
         text_to_mobile_j( dsock->player, "ui_command", "show_all_windows" );
         dsock->state = STATE_PLAYING;
      }
   }
   return;
}

static void state_chargen( D_SOCKET *dsock, char *arg )
{
   log_string( "state_chargen" );

   char cmd[MAX_STRING_LENGTH];

   arg = one_arg( arg, cmd );

   if( !strcmp( cmd, "checkname" ) )
   {
      for( size_t i = 0; i < strlen( arg ); i++ )
      {
         arg[i] = tolower( arg[i] );
      }
      arg[0] = toupper( arg[0] ); //Properly format their name
      if( check_name( arg ) )
         text_to_socket( dsock, "%c{\"type\":\"ui_command\", \"data\":{\"text\":\"valid_name\"}}%c", (char)2, (char)3 );
      else
         text_to_socket( dsock, "%c{\"type\":\"ui_command\", \"data\":{\"text\":\"invalid_name\"}}%c", (char)2, (char)3 );
   }
   else if( !strcmp( cmd, "cancel" ) )
   {
      dsock->state = STATE_MAIN_MENU;
      text_to_socket( dsock, mainMenu );
      text_to_socket( dsock, "%c{\"type\":\"ui_command\", \"data\":{\"text\":\"Hide_all_windows\"}}%c", (char)2, (char)3 );
      return;
   }
   //submit followed by a json string with the character's data. 
   //We check to see if everything is kosher on the server side too.
   else if( !strcmp( cmd, "submit" ) )
   {
      json_t *json = json_loads( arg, 0, NULL );
      if( json == NULL )
      {
         text_to_socket( dsock, "%c{\"type\":\"ui_command\", \"data\":{\"text\":\"invalid_chargen\"}}%c", (char)2, (char)3 );
         log_string( "Chargen error: Invalid JSON submitted (%s)", arg );
         return;
      }
      D_MOBILE *dMob = json_to_mobile( json );
      if( dMob == NULL )
      {
         text_to_socket( dsock, "%c{\"type\":\"ui_command\", \"data\":{\"text\":\"invalid_chargen\"}}%c", (char)2, (char)3 );
         log_string( "Chargen error: Unable to parse JSON into valid character", arg );
         return;
      }

      if( dMob->brains < 1       || dMob->brains > 10
       || dMob->brawn <  1       || dMob->brawn  > 10
       || dMob->senses < 1       || dMob->senses > 10
       || dMob->stamina < 1      || dMob->stamina > 10
       || dMob->coordination < 1 || dMob->coordination > 10
       || dMob->cool < 1         || dMob->cool > 10
       || dMob->luck < 1         || dMob->luck > 10 )
      {
         text_to_socket( dsock, "%c{\"type\":\"ui_command\", \"data\":{\"text\":\"invalid_chargen_stats\"}}%c", (char)2, (char)3 );
         log_string( "Chargen error: stats out of range." );
         free_mobile( dMob );
         return;
      }

      if( dMob->brains + dMob->brawn + dMob->senses + dMob->stamina + dMob->coordination + dMob->cool + dMob->luck != 40 )
      {
         text_to_socket( dsock, "%c{\"type\":\"ui_command\", \"data\":{\"text\":\"invalid_chargen_stats\"}}%c", (char)2, (char)3 );
         log_string( "Chargen error: stats incomplete." );
         free_mobile( dMob );
         return;
      }

      for( size_t i = 0; i < strlen( arg ); i++ )
      {
         arg[i] = tolower( arg[i] );
      }
      arg[0] = toupper( arg[0] ); //Properly format their name
      if( !check_name( dMob->name ) )
      {
         text_to_socket( dsock, "%c{\"type\":\"ui_command\", \"data\":{\"text\":\"invalid_name %s\"}}%c", (char)2, dMob->name, (char)3 );
         log_string( "Chargen error: bad name." );
         free_mobile( dMob );
         return;
      }

      //Looks like we're good to go!
      dsock->player = dMob;
      dMob->socket = dsock;
      dsock->player->level    =   LEVEL_GOD;
      dsock->player->cur_hp   = 100 + ( dsock->player->brawn * 10 );
      dsock->player->max_hp   = 100 + ( dsock->player->brawn * 10 );
      for( int i = BODY_HEAD; i < MAX_BODY; i++ )
      {
         dsock->player->body[i]->max_hp = ( body_hp_mod[i] * dsock->player->max_hp ) / 100;
         dsock->player->body[i]->cur_hp = dsock->player->body[i]->max_hp;
      }
      dsock->player->btc      = 0;
      dsock->player->position = POS_STANDING;

      dsock->player->cur_bandwidth = 0;
      dsock->player->max_bandwidth = 0;
      dsock->player->encumberance  = 0;
      dsock->player->hold_left = NULL;
      dsock->player->hold_right = NULL;

      dsock->player->citizenship = strdup( "Federation" );
      dsock->player->association = strdup( "BitCorp" );
      dsock->player->prompt      = strdup( "#>" );

      dsock->player->socket = dsock;
      AttachToList(dsock->player, dmobile_list);
      AppendToList( strdup( dsock->player->name ), dsock->account->characters );
      save_account( dsock->account );
      dsock->player->room = get_room_by_vnum( FIRST_ROOM );
      AppendToList( dsock->player, dsock->player->room->mobiles );
      echo_around( dsock->player, "text", "The world vibrates slightly and %s flickers into existance.", dsock->player->name );
      log_string("New Player: %s has finished chargen.", dsock->player->name);
      log_string("Player: %s has entered the game.", dsock->player->name);
      save_player( dsock->player );
      text_to_mobile_j( dsock->player, "ui_command", "show_all_windows" );
      text_to_buffer( dsock, " The immigrations control officer sighs, \"Finally finished? The city is"
                             " through the door on your right.\"\r\n\r\n"
                             " A bright LCD screen flashes the following message: \r\n\r\n" );
      text_to_buffer( dsock, "%s\r\n\r\n", motd );
      dsock->state = STATE_PLAYING;
      cmd_look( dsock->player, "" );
      return;
   }

}

static void state_chargen_name( D_SOCKET *dsock, char *arg )
{
   if( dsock->player && dsock->player->name )
   {
      free( dsock->player->name );
      free( dsock->player );
   }
   
   if( !strcasecmp( arg, "cancel" ) )
   {
      text_to_buffer( dsock, mainMenu );
      free_mobile( dsock->player );
      dsock->player = NULL;
      dsock->state = STATE_MAIN_MENU;
      return;
   }
   for( size_t i = 0; i < strlen( arg ); i++ )
   {
      arg[i] = tolower( arg[i] );
   }
   arg[0] = toupper( arg[0] ); //Properly format their name
   
   if( !check_name( arg ) )
   {
      text_to_buffer( dsock, "Input \"%s\" not accepted. Specify personnel name for registration. Input \"Cancel\" to end registration.\r\n", arg );
      return;
   }
   
   if( ( dsock->player = new_mobile() ) == NULL )
   {
      bug( "handle_new_connections::STATE_CHARGEN_NAME : Unable to allocate memory for new player." );
      text_to_buffer( dsock, "Error: The personnel registration system is down for maintenance. Error Code 49527.\r\n" );
      dsock->state = STATE_CLOSED;
      return;
   }
   dsock->player->name = strdup( arg );
   //snprintf( dsock->player->name, MAX_PC_NAME_LENGTH, "%s", arg );
   text_to_buffer( dsock, "Input \"%s\" is accepted.\r\n", dsock->player->name );
   text_to_buffer( dsock, "Specify personnel sex. [M]ale, [F]emale, [N]onbinary\r\nSex: ", dsock->player->name );
   dsock->state = STATE_CHARGEN_GENDER;
   
   return;
}

static void state_chargen_gender( D_SOCKET *dsock, char *arg )
{
   switch( arg[0] )
   {
      default:
      {
         text_to_buffer( dsock, "Invalid input. [M]ale, [F]emale, or [N]onbinary. \"Cancel\" to end personnel registration.\r\nSex: " );
         break;
      }
      case 'm':
      case 'M':
      {
         dsock->player->gender = MALE;
         break;
      }
      case 'f':
      case 'F':
      {
         dsock->player->gender = FEMALE;
         break;
      }
      case 'n':
      case 'N':
      {
         dsock->player->gender = NONBINARY;
         break;
      }
      case 'c':
      case 'C':
      {
         if( strcasecmp( arg, "cancel" ) ) break; //they didn't specifically type 'cancel'
         text_to_buffer( dsock, mainMenu );
         dsock->state = STATE_MAIN_MENU;
         free_mobile( dsock->player );
         dsock->player = NULL;
         return;
      }
   }
   text_to_buffer( dsock, "\r\nSpecify race. Additional information regarding each race can be obtained by typing help <race>\r\n" );
   text_to_buffer( dsock, "Type 'cancel' to end registration.\r\n\r\n\tHuman\tSynthetic\r\n\r\nRace: " );
   dsock->state = STATE_CHARGEN_RACE;
   return;
}

static void state_chargen_race( D_SOCKET *dsock, char *arg )
{
   if( is_prefix( arg, "human" ) )
   {
      dsock->player->race = strdup( "Human" );
   }
   else if( is_prefix( arg, "synthetic" ) )
   {
      dsock->player->race = strdup( "Synthetic" );
   }
   else if( !strcasecmp( arg, "cancel" ) )
   {
      free_mobile( dsock->player );
      text_to_buffer( dsock, mainMenu );
      dsock->state = STATE_MAIN_MENU;
      dsock->player = NULL;
      return;
   }
   else
   {
      text_to_buffer( dsock, "Invalid input. Valid races are Human or Synthetic. 'Cancel' to end registration.\r\nRace: " );
      return;
   }

   text_to_buffer( dsock, " Brains:         5\r\n"
                          " Brawn:          5\r\n"
                          " Senses:         5\r\n"
                          " Stamina:        5\r\n"
                          " Coordination:   5\r\n"
                          " Cool:           5\r\n"
                          " ------------\r\n"
                          " Points:         5\r\n\r\n");
   text_to_buffer( dsock, "Specify physical characteristics. Use +/-<stat> to adjust as necessary.\r\n"
                          "Type help <stat> for additional information. Type 'done' when complete.\r\n\r\n> " );
   dsock->state = STATE_CHARGEN_STATS;
   dsock->player->brains = dsock->player->brawn = dsock->player->coordination = dsock->player->senses = dsock->player->stamina = 5;
   dsock->player->luck = dsock->player->cool = 5;
   return;
}

static void state_chargen_height( D_SOCKET *dsock, char *arg )
{
   /*
   unsigned int height;
   if( sscanf( arg, "%uin", &height ) == 1
       || sscanf( arg, "%u in", &height ) == 1 )
   {
      dsock->player->heightcm = (height * 254)/100;
   }
   else if( sscanf( arg, "%ucm", &height ) == 1
            || sscanf( arg, "%u cm", &height ) == 1 )
   {
      dsock->player->heightcm = height;
   }
   else
   {
      text_to_buffer( dsock, "Invalid input. Input either NNin for height in inches, or NNcm for height in\r\n"
                             "centimeters. Where NN is a positive number.\r\nHeight: " );
      return;
   }

   if( dsock->player->heightcm < 150 || dsock->player->heightcm > 245 )
   {
      text_to_buffer( dsock, "Height must be between 135 cm (4'11\") and 245 cm (8'0\").\r\n" );
      return;
   }
   */
    dsock->state = STATE_CHARGEN_WEIGHT;
    text_to_buffer( dsock, "Input registrant weight. Specify pounds or kilograms via 'lb' or 'kg' postfix.\r\n" );
    text_to_buffer( dsock, "Weight: " );
    return;
}

static void state_chargen_weight( D_SOCKET *dsock, char *arg )
{
   /*
   unsigned int weight;
   if( sscanf( arg, "%ulb", &weight ) == 1
       || sscanf( arg, "%u lb", &weight ) == 1 
       || sscanf( arg, "%ulbs", &weight ) == 1
       || sscanf( arg, "%u lbs", &weight ) == 1 )
   {
      dsock->player->weightkg = (weight * 45)/100;
   }
   else if( sscanf( arg, "%ukg", &weight ) == 1
            || sscanf( arg, "%u kg", &weight ) == 1
            || sscanf( arg, "%ukgs", &weight ) == 1
            || sscanf( arg, "%u kgs", &weight ) == 1 )
   {
      dsock->player->weightkg = weight;
   }
   else
   {
      text_to_buffer( dsock, "Invalid input. Input either NNlbs for weight in pounds, or NNkgs for weight in\r\n"
                             "kilograms. Where NN is a positive number.\r\nHeight: " );
      return;
   }
   if( dsock->player->weightkg < 45 || dsock->player->weightkg > 180 )
   {
      text_to_buffer( dsock, "Weight must be between 30 kg (95 lbs) and 180 kg (400 lbs)\r\n" );
      return;
   }
   */
    dsock->state = STATE_CHARGEN_EYECOLOR;
    text_to_buffer( dsock, "\r\nInput registrant eye color (Black, brown, hazel, green, blue, grey, gold )r\n" );
    text_to_buffer( dsock, "Eye color: " );
    return;
}

static void state_chargen_eyecolor( D_SOCKET *dsock, char *arg )
{
   if( is_prefix( arg, "black" ) )
   {
      dsock->player->eyecolor = strdup( "Black" );
   }
   else if( is_prefix( arg, "brown" ) )
   {
      dsock->player->eyecolor = strdup( "Brown" );
   }
   else if( is_prefix( arg, "hazel" ) )
   {
      dsock->player->eyecolor = strdup( "Hazel" );
   }
    else if( is_prefix( arg, "green" ) )
    {
       dsock->player->eyecolor = strdup( "Green" );
    }

    else if( is_prefix( arg, "blue" ) )
    {
       dsock->player->eyecolor = strdup( "Blue" );
    }
    else if( is_prefix( arg, "grey" ) )
    {
       dsock->player->eyecolor = strdup( "Grey" );
    }
    else if( is_prefix( arg, "gold" ) )
   {
      dsock->player->eyecolor = strdup( "Gold" );
   }
   else
   {
      text_to_buffer( dsock, "Invalid input. Specify registrant eyecolor.\r\n(Black, brown, hazel, green, blue, grey, gold): " );
      return;
   }
   //All done, now start playing!
   dsock->player->level    =   LEVEL_GOD;
   dsock->player->cur_hp   = 100;
   dsock->player->max_hp   = 100;
   /*
   dsock->player->sight    = 100;
   dsock->player->head     = 100;
   dsock->player->body     = 100;
   dsock->player->larm     = 100;
   dsock->player->rarm     = 100;
   dsock->player->lleg     = 100;
   dsock->player->rleg     = 100;
   */
   dsock->player->btc      = 0.0;
   dsock->player->position = POS_STANDING;

   dsock->player->cur_bandwidth = 0;
   dsock->player->max_bandwidth = 0;
   dsock->player->encumberance  = 0;
   dsock->player->hold_left = NULL;
   dsock->player->hold_right = NULL;

   dsock->player->citizenship = strdup( "Federation" );
   dsock->player->association = strdup( "BitCorp" );
   dsock->player->prompt      = strdup( "#>" );

   dsock->player->socket = dsock;
   AttachToList(dsock->player, dmobile_list);
   AppendToList( strdup( dsock->player->name ), dsock->account->characters );
   save_account( dsock->account );
   dsock->player->room = get_room_by_vnum( FIRST_ROOM );
   AppendToList( dsock->player, dsock->player->room->mobiles );
   echo_around( dsock->player, "text", "The world vibrates slightly and %s flickers into existance.", dsock->player->name );
   log_string("New Player: %s has finished chargen.", dsock->player->name);
   log_string("Player: %s has entered the game.", dsock->player->name);
   save_player( dsock->player );
   text_to_mobile_j( dsock->player, "ui_command", "show_all_windows" );
   //init_events_player(dsock->player);
   //strip_event_socket(dsock, EVENT_SOCKET_IDLE);
   text_to_buffer( dsock, " The immigrations control officer sighs, \"Finally finished? The city is"
                          " through the door on your right.\"\r\n\r\n"
                          " A bright LCD screen flashes the following message: \r\n\r\n" );
   text_to_buffer( dsock, "%s\r\n\r\n", motd );
   //text_to_buffer( dsock, "\r\nPress <return> to continue\r\n" );
   dsock->state = STATE_PLAYING;
   cmd_look( dsock->player, "" );
   return;
}

static void state_chargen_stats( D_SOCKET *dsock, char *arg )
{
   D_M *p = dsock->player;
   int points = 40 - ( p->brains + p->brawn + p->coordination + p->senses + p->stamina + p->luck + p->cool);
    if( toupper(arg[0]) == 'H' )
    {
       //Get help info
       return;
    }
    else if( toupper( arg[0] ) == 'D' )
    {
       if( points > 0 )
       {
          text_to_buffer( dsock, "Allocate all points before continuing.\r\n" );
          return;
       }
       text_to_buffer( dsock, "Physical characteristics recorded.\r\n" );
    }
    else if( toupper( arg[0] ) == 'C' )
    {
       if( !strcasecmp( arg, "cancel" ) )
       {
       }
       else
       {
          text_to_buffer( dsock, "Spell out 'cancel' to cancel character creation.\r\n" );
       }
       return;
    }
    else if( arg[0] == '-' )
    {
       if( is_prefix( arg + 1, "brains" ) )
       {
          if( p->brains == 1 )
             return;
          p->brains--;
       }
       else if( is_prefix( arg + 1, "brawn" ) )
       {
          if( p->brawn == 1 )
             return;
          p->brawn--;
       }
       else if( is_prefix( arg + 1, "coordination" ) )
       {
          if( p->coordination == 1 )
             return;
          p->coordination--;
       }
       else if( is_prefix( arg + 1, "senses" ) )
       {
          if( p->senses == 1  )
             return;
          p->senses--;
       }
       else if( is_prefix( arg + 1, "stamina" ) )
       {
          if( p->stamina == 1  )
             return;
          p->stamina--;
       }
       else if( is_prefix( arg + 1, "luck" ) )
       {
          if( p->luck == 1  )
             return;
          p->luck--;
       }
       else if( is_prefix( arg + 1, "cool" ) )
       {
          if( p->cool == 1  )
             return;
          p->cool--;
       }
       else
       {
          text_to_buffer( dsock, "Invalid stat '%s'.\r\n", arg+1 );
       }
       points = 40 - (p->brains + p->brawn + p->coordination + p->senses + p->stamina + p->luck + p->cool); //recalculate points
       text_to_buffer( dsock, " Brains:         %i\r\n"
                        " Brawn:          %i\r\n"
                        " Senses:         %i\r\n"
                        " Stamina:        %i\r\n"
                        " Coordination:   %i\r\n"
                        " Cool:           %i\r\n"
                        " Luck:           %i\r\n"
                        " ------------------\r\n"
                        " Points:         %i\r\n\r\n",
                        p->brains, p->brawn, p->senses, p->stamina, p->coordination, p->cool, p->luck, points );
       text_to_buffer( dsock, "Specify physical characteristics. Use +/-<stat> to adjust as necessary.\r\n"
                          "Type help <stat> for additional information. Type 'done' when complete.\r\n\r\n> " );
       return;
    }
    else if( arg[0] == '+' )
    {
       if( is_prefix( arg + 1, "brains" ) )
       {
          if( p->brains == 10 || points == 0 )
             return;
          p->brains++;
       }
       else if( is_prefix( arg + 1, "brawn" ) )
       {
          if( p->brawn == 10 || points == 0 )
             return;
          p->brawn++;
       }
       else if( is_prefix( arg + 1, "coordination" ) )
       {
          if( p->coordination == 10 || points == 0 )
             return;
          p->coordination++;
       }
       else if( is_prefix( arg + 1, "senses" ) )
       {
          if( p->senses == 10 || points == 0 )
             return;
          p->senses++;
       }
       else if( is_prefix( arg + 1, "stamina" ) )
       {
          if( p->stamina == 10 || points == 0 )
             return;
          p->stamina++;
       }
       else if( is_prefix( arg + 1, "luck" ) )
       {
          if( p->luck == 10 || points == 0 )
             return;
          p->luck++;
       }
       else if( is_prefix( arg + 1, "cool" ) )
       {
          if( p->cool == 10 || points == 0 )
             return;
          p->cool++;
       }
       else
       {
          text_to_buffer( dsock, "Invalid stat '%s'.\r\n", arg+1 );
       }
       points = 40 - (p->brains + p->brawn + p->coordination + p->senses + p->stamina + p->luck + p->cool); //recalculate points
       text_to_buffer( dsock, " Brains:         %i\r\n"
                        " Brawn:          %i\r\n"
                        " Senses:         %i\r\n"
                        " Stamina:        %i\r\n"
                        " Coordination:   %i\r\n"
                        " Cool:           %i\r\n"
                        " Luck:           %i\r\n"
                        " ------------------\r\n"
                        " Points:         %i\r\n\r\n",
                        p->brains, p->brawn, p->senses, p->stamina, p->coordination, p->cool, p->luck, points );
       text_to_buffer( dsock, "Specify physical characteristics. Use +/-<stat> to adjust as necessary.\r\n"
                          "Type help <stat> for additional information. Type 'done' when complete.\r\n\r\n> " );
       return;
    }
    text_to_buffer( dsock, "Input registrant height. Specify inches or centimeters via 'in' or 'cm' postfix\r\n" );
    text_to_buffer( dsock, "Height: " );
    dsock->state = STATE_CHARGEN_HEIGHT;
    return;
}

static void state_new_password( D_SOCKET *dsock, char *arg )
{
   char salt[BCRYPT_HASHSIZE];
   char hash[BCRYPT_HASHSIZE];

   if( strlen( arg ) < 6 )
   {
      text_to_buffer( dsock, "\r\nPassword must be at least 6 characters long.\r\n" );
      text_to_buffer( dsock, "Please enter a new password: " );
      return;
   }
   
   bool l=FALSE, n=FALSE, s=FALSE; //letters, numbers, special characters
   if( strpbrk( arg, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ" ) )
      l=TRUE;
   if( strpbrk( arg, "1234567890" ) )
      n = TRUE;
   if( strpbrk( arg, "`~!@#$%^&*()-_=+[{]}\\|;:'\",<.>/?" ) )
      s=TRUE;
   if( !l || !n || !s )
      text_to_buffer( dsock, "\r\nAlthough we will accept your password, we highly recommend using a secure\r\npassword which contains both letters, numbers, and special characters.\r\n" );

   bcrypt_gensalt( 12, salt );
   bcrypt_hashpw( arg, salt, hash );
   dsock->account->password = strdup( hash );
   text_to_buffer( dsock, "Confirm your password: " );
   dsock->state = STATE_VERIFY_PASSWORD;
   return;
}

static void state_verify_password( D_SOCKET *dsock, char *arg )
{

   if( !bcrypt_checkpw( arg, dsock->account->password ) )
   {
      text_to_buffer( dsock, "Passwords match. Account created.\r\n" );
   text_to_buffer(dsock, (char *) do_echo); 
      text_to_buffer( dsock, mainMenu );
      dsock->state = STATE_MAIN_MENU;
      return;
   }

   text_to_buffer( dsock, "Passwords do not match.\r\nPlease enter a new password: " );
   dsock->state = STATE_NEW_PASSWORD;
   return;
}


void handle_new_connections( D_SOCKET *dsock, char *arg )
{

   switch( dsock->state )
   {
      default:
      {
         bug("Handle_new_connections: Bad state %i", (int)dsock->state );
         break;
      }
      case STATE_GET_ACCOUNT:
         state_get_account( dsock, arg );
         break;
      case STATE_ASK_PASSWORD:
         state_ask_password( dsock, arg );
         break;
      case STATE_NEW_ACCOUNT:
         state_new_account( dsock, arg );
         break;
      case STATE_MAIN_MENU:
         state_main_menu( dsock, arg );
         break;
      case STATE_CHOOSE_CHAR:
         state_choose_char( dsock, arg );
         break;
      case STATE_CHARGEN:
         state_chargen( dsock, arg );
         break;
      case STATE_CHARGEN_NAME:
         state_chargen_name( dsock, arg );
         break;
      case STATE_CHARGEN_GENDER:
         state_chargen_gender( dsock, arg );
         break;
      case STATE_CHARGEN_RACE:
         state_chargen_race( dsock, arg );
         break;
      case STATE_CHARGEN_HEIGHT:
         state_chargen_height( dsock, arg );
         break;
      case STATE_CHARGEN_WEIGHT:
         state_chargen_weight( dsock, arg );
         break;
      case STATE_CHARGEN_EYECOLOR:
         state_chargen_eyecolor( dsock, arg );
         break;
      case STATE_PRESS_ENTER:
         state_press_enter( dsock, arg );
         break;
      case STATE_CHARGEN_STATS:
         state_chargen_stats( dsock, arg );
         break;
      case STATE_NEW_PASSWORD:
         state_new_password( dsock, arg );
         break;
      case STATE_VERIFY_PASSWORD:
         state_verify_password( dsock, arg );
         break;
   }
   return;
}

