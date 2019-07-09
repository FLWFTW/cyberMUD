/*
 * This file contains the socket code, used for accepting
 * new connections as well as reading and writing to
 * sockets, and closing down unused sockets.
 */

#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <sys/ioctl.h>
#include <errno.h>

/* including main header file */
#include "mud.h"

/* global variables */
fd_set      fSet;                  /* the socket list for polling       */
STACK     * dsock_free = NULL;     /* the socket free list              */
LIST      * dsock_list = NULL;     /* the linked list of active sockets */
STACK     * dmobile_free = NULL;   /* the mobile free list              */
LIST      * dmobile_list = NULL;   /* the mobile list of active mobiles */
LIST      * droom_list = NULL;     /* linked list of loaded rooms       */
LIST      * darea_list = NULL;     /* linked list of loaded rooms       */
LIST      * dobject_list = NULL;   /* linked list of loaded objects     */
LIST      * object_protos = NULL;
LIST      * mobile_protos = NULL;
lua_State * globalLuaState;

/* mccp support */
const unsigned char compress_will   [] = { IAC, WILL, TELOPT_COMPRESS,  '\0' };
const unsigned char compress_will2  [] = { IAC, WILL, TELOPT_COMPRESS2, '\0' };
const unsigned char do_echo         [] = { IAC, WONT, TELOPT_ECHO,      '\0' };
const unsigned char dont_echo       [] = { IAC, WILL, TELOPT_ECHO,      '\0' };
/* local procedures */
void GameLoop         ( int control );

/* intialize shutdown state */
bool shut_down = FALSE;
int  control;
D_ROOM *froom;
/*
 * This is where it all starts, nothing special.
 */
int main(int argc, char **argv)
{
  bool fCopyOver;

  /* get the current time */
  current_time = time(NULL);
  boot_time = time( NULL );

  json_object_seed( 0 );

  /* allocate memory for socket and mobile lists'n'stacks */
  dsock_free =    AllocStack();
  dsock_list =    AllocList();
  dmobile_free =  AllocStack();
  dmobile_list =  AllocList();
  droom_list =    AllocList();
  darea_list =    AllocList();
  dobject_list =  AllocList();
  object_protos =  AllocList();
  mobile_protos =  AllocList();

  globalLuaState = init_lua();

  froom = new_room();
  froom->name = strdup( "/dev/null/ $");
  froom->description = strdup( "You shouldn't be here. Yet you are." );
  froom->vnum = 0;
  AttachToList( froom, droom_list );
  /* note that we are booting up */
  log_string("Program starting.");

  /* initialize the event queue - part 1 */
  //init_event_queue(1);

  if (argc > 2 && !strcmp(argv[argc-1], "copyover") && atoi(argv[argc-2]) > 0)
  {
    fCopyOver = TRUE;
    control = atoi(argv[argc-2]);
  }
  else fCopyOver = FALSE;

  /* initialize the socket */
  if (!fCopyOver)
    control = init_socket();

  /* load all external data */
  load_muddata(fCopyOver);

  load_areas();
  link_exits();

  check_areas( TRUE ); //initial reset

  /* initialize the event queue - part 2*/
  //init_event_queue(2);

  /* main game loop */
  GameLoop(control);

  log_string("MUD is shutting down...");
  close_lua( globalLuaState );
  /* close down the socket */
  close(control);
  log_string( "Socket closed" );

  /* terminated without errors */
  log_string("Program terminated without errors.");

  /* and we are done */
  return 0;
}

void GameLoop(int icontrol)   
{
  D_SOCKET *dsock;
  ITERATOR Iter;
  static struct timeval tv;
  struct timeval last_time, new_time;
  fd_set rFd;
  long secs, usecs;

  /* set this for the first loop */
  gettimeofday(&last_time, NULL);

  /* clear out the file socket set */
  FD_ZERO(&fSet);

  /* add control to the set */
  FD_SET(icontrol, &fSet);

  /* copyover recovery */
  AttachIterator(&Iter, dsock_list);
  while ((dsock = (D_SOCKET *) NextInList(&Iter)) != NULL)
    FD_SET(dsock->control, &fSet);
  DetachIterator(&Iter);

  /* do this untill the program is shutdown */
  while (!shut_down)
  {
    /* set current_time */
    current_time = time(NULL);

    /* copy the socket set */
    memcpy(&rFd, &fSet, sizeof(fd_set));

    /* wait for something to happen */
    if (select(FD_SETSIZE, &rFd, NULL, NULL, &tv) < 0)
      continue;

    /* check for new connections */
    if (FD_ISSET(icontrol, &rFd))
    {
      struct sockaddr_in sock;
      unsigned int socksize;
      int newConnection;

      socksize = sizeof(sock);
      if ((newConnection = accept(icontrol, (struct sockaddr*) &sock, &socksize)) >=0)
        new_socket(newConnection);
    }

    /* poll sockets in the socket list */
    AttachIterator(&Iter ,dsock_list);
    while ((dsock = (D_SOCKET *) NextInList(&Iter)) != NULL)
    {
      /*
       * Close sockects we are unable to read from.
       */
      if (FD_ISSET(dsock->control, &rFd) && !read_from_socket(dsock))
      {
        close_socket(dsock, FALSE);
        continue;
      }

      /* Ok, check for a new command */
      next_cmd_from_buffer(dsock);

      /* Is there a new command pending ? */
      if (dsock->next_command[0] != '\0')
      {
        /* figure out how to deal with the incoming command */
        switch(dsock->state)
        {
           default:
              bug( "Invalid state: %i", dsock->state );
              break;
           case STATE_GET_ACCOUNT:
           case STATE_NEW_ACCOUNT:
           case STATE_ASK_PASSWORD:
           case STATE_MAIN_MENU:
           case STATE_CHOOSE_CHAR:
           case STATE_CHARGEN_NAME:
           case STATE_CHARGEN_GENDER:
           case STATE_NEW_NAME:
           case STATE_NEW_PASSWORD:
           case STATE_VERIFY_PASSWORD:
           case STATE_CHARGEN:
           case STATE_CHARGEN_RACE:
           case STATE_CHARGEN_STATS:
           case STATE_CHARGEN_HEIGHT:
           case STATE_CHARGEN_WEIGHT:
           case STATE_CHARGEN_EYECOLOR:
           case STATE_PRESS_ENTER:
              handle_new_connections(dsock, dsock->next_command);
              break;
          case STATE_PLAYING:
              if( dsock->player == NULL )
                 break;
              handle_cmd_input(dsock->player, dsock->next_command);
              break;
        }

        dsock->next_command[0] = '\0';
      }

      /* if the player quits or gets disconnected */
      if (dsock->state == STATE_CLOSED) continue;

      /* Send all new data to the socket and close it if any errors occour */
      if (!flush_output(dsock))
        close_socket(dsock, FALSE);
    }
    DetachIterator(&Iter);

    /* call the event queue */
    //heartbeat();
    
    /*Iterate through mobiles (This includes players)*/
    check_mobiles();

    /*Iterate through objects*/
    check_objects();

    /*Iterate through rooms*/
    check_rooms();

    /*Iterate through areas*/
    check_areas( FALSE );

    /*
     * Here we sleep out the rest of the pulse, thus forcing
     * SocketMud(tm) to run at PULSES_PER_SECOND pulses each second.
     */
    gettimeofday(&new_time, NULL);

    /* get the time right now, and calculate how long we should sleep */
    usecs = (int) (last_time.tv_usec -  new_time.tv_usec) + 1000000 / PULSES_PER_SECOND;
    secs  = (int) (last_time.tv_sec  -  new_time.tv_sec);

    /*
     * Now we make sure that 0 <= usecs < 1.000.000
     */
    while (usecs < 0)
    {
      usecs += 1000000;
      secs  -= 1;
    }
    while (usecs >= 1000000)
    {
      usecs -= 1000000;
      secs  += 1;
    }

    /* if secs < 0 we don't sleep, since we have encountered a laghole */
    if (secs > 0 || (secs == 0 && usecs > 0))
    {
      struct timeval sleep_time;

      sleep_time.tv_usec = usecs;
      sleep_time.tv_sec  = secs;

      if (select(0, NULL, NULL, NULL, &sleep_time) < 0)
        continue;
    }

    /* reset the last time we where sleeping */
    gettimeofday(&last_time, NULL);

    /* recycle sockets */
    recycle_sockets();
  }
}

/*
 * Init_socket()
 *
 * Used at bootup to get a free
 * socket to run the server from.
 */
int init_socket()
{
  struct sockaddr_in my_addr;
  int sockfd, reuse = 1;

  /* let's grab a socket */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  /* setting the correct values */
  my_addr.sin_family = AF_INET;
  my_addr.sin_addr.s_addr = INADDR_ANY;
  my_addr.sin_port = htons(MUDPORT);

  /* this actually fixes any problems with threads */
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1)
  {
    perror("Error in setsockopt()");
    exit(1);
  } 

  /* bind the port */
  bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr));

  /* start listening already :) */
  listen(sockfd, 3);

  /* return the socket */
  return sockfd;
}

/* 
 * New_socket()
 *
 * Initializes a new socket, get's the hostname
 * and puts it in the active socket_list.
 */
bool new_socket(int sock)
{
  struct sockaddr_in   sock_addr;
  pthread_attr_t       attr;
  pthread_t            thread_lookup;
  LOOKUP_DATA        * lData;
  D_SOCKET           * sock_new;
  int                  argp = 1;
  socklen_t            size;

  /* initialize threads */
  pthread_attr_init(&attr);   
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  /*
   * allocate some memory for a new socket if
   * there is no free socket in the free_list
   */
  if (StackSize(dsock_free) <= 0)
  {
    if ((sock_new = malloc(sizeof(*sock_new))) == NULL)
    {
      bug("New_socket: Cannot allocate memory for socket.");
      abort();
    }
  }
  else
  {
    sock_new = (D_SOCKET *) PopStack(dsock_free);
  }

  /* attach the new connection to the socket list */
  FD_SET(sock, &fSet);

  /* clear out the socket */
  clear_socket(sock_new, sock);

  /* set the socket as non-blocking */
  ioctl(sock, FIONBIO, &argp);

  /* update the linked list of sockets */
  AttachToList(sock_new, dsock_list);

  /* do a host lookup */
  size = sizeof(sock_addr);
  if (getpeername(sock, (struct sockaddr *) &sock_addr, &size) < 0)
  {
    perror("New_socket: getpeername");
    sock_new->hostname = strdup("unknown");
  }
  else
  {
    /* set the IP number as the temporary hostname */
    sock_new->hostname = strdup(inet_ntoa(sock_addr.sin_addr));

    if (strcasecmp(sock_new->hostname, "127.0.0.1"))
    {
      /* allocate some memory for the lookup data */
      if ((lData = malloc(sizeof(*lData))) == NULL)
      {
        bug("New_socket: Cannot allocate memory for lookup data.");
        abort();
      }

      /* Set the lookup_data for use in lookup_address() */
      lData->buf    =  strdup((char *) &sock_addr.sin_addr);
      lData->dsock  =  sock_new;

      /* dispatch the lookup thread */
      pthread_create(&thread_lookup, &attr, &lookup_address, (void*) lData);
    }
    else sock_new->lookup_status++;
  }

  /* negotiate compression */
  text_to_buffer(sock_new, (char *) compress_will2);
  text_to_buffer(sock_new, (char *) compress_will);

  /* send the greeting */
  text_to_buffer(sock_new, greeting);
  text_to_buffer(sock_new, "Enter your account name, or 'new' to register for a new account: ");

  /* initialize socket events */
  //init_events_socket(sock_new);

  /* everything went as it was supposed to */
  return TRUE;
}

/*
 * Close_socket()
 *
 * Will close one socket directly, freeing all
 * resources and making the socket availably on
 * the socket free_list.
 */
void close_socket(D_SOCKET *dsock, bool reconnect)
{
  //EVENT_DATA *pEvent;
  //ITERATOR Iter;

  if (dsock->lookup_status > TSTATE_DONE) return;
  dsock->lookup_status += 2;

  /* remove the socket from the polling list */
  FD_CLR(dsock->control, &fSet);

  if (dsock->state == STATE_PLAYING)
  {
    if (reconnect)
      text_to_socket(dsock, "This connection has been taken over.\n\r");
    else if (dsock->player)
    {
      dsock->player->socket = NULL;
      log_string("Closing link to %s", dsock->player->name);
    }
  }
  else if (dsock->player)
    free_mobile(dsock->player);

  /* dequeue all events for this socket 
  AttachIterator(&Iter, dsock->events);
  while ((pEvent = (EVENT_DATA *) NextInList(&Iter)) != NULL)
    dequeue_event(pEvent);
  DetachIterator(&Iter);
*/
  /* set the closed state */
  dsock->state = STATE_CLOSED;
}

/* 
 * Read_from_socket()
 *
 * Reads one line from the socket, storing it
 * in a buffer for later use. Will also close
 * the socket if it tries a buffer overflow.
 */
bool read_from_socket(D_SOCKET *dsock)
{
  int size;

  /* check for buffer overflows, and drop connection in that case */
  size = strlen(dsock->inbuf);
  if (size >= sizeof(dsock->inbuf) - 2)
  {
    text_to_socket(dsock, "\n\r!!!! Input Overflow !!!!\n\r");
    return FALSE;
  }

  /* start reading from the socket */
  for (;;)
  {
    int sInput;
    int wanted = sizeof(dsock->inbuf) - 2 - size;

    sInput = read(dsock->control, dsock->inbuf + size, wanted);

    if (sInput > 0)
    {
      size += sInput;

      if (dsock->inbuf[size-1] == '\n' || dsock->inbuf[size-1] == '\r')
        break;
    }
    else if (sInput == 0)
    {
      log_string("Read_from_socket: EOF");
      return FALSE;
    }
    else if (errno == EAGAIN || sInput == wanted)
      break;
    else
    {
      perror("Read_from_socket");
      return FALSE;
    }     
  }
  dsock->inbuf[size] = '\0';
  return TRUE;
}

void next_cmd_from_buffer(D_SOCKET *dsock)
{
  int size = 0, i = 0, j = 0, telopt = 0;

  /* if there's already a command ready, we return */
  if (dsock->next_command[0] != '\0')
    return;

  /* if there is nothing pending, then return */
  if (dsock->inbuf[0] == '\0')
    return;

  /* check how long the next command is */
  while (dsock->inbuf[size] != '\0' && dsock->inbuf[size] != '\n' && dsock->inbuf[size] != '\r')
    size++;

  /* we only deal with real commands */
  if (dsock->inbuf[size] == '\0')
    return;

  /* copy the next command into next_command */
  for ( ; i < size; i++)
  {
    if (dsock->inbuf[i] == (signed char) IAC)
    {
      telopt = 1;
    }
    else if (telopt == 1 && (dsock->inbuf[i] == (signed char) DO || dsock->inbuf[i] == (signed char) DONT))
    {
      telopt = 2;
    }
    else if (telopt == 2)
    {
      telopt = 0;

      if (dsock->inbuf[i] == (signed char) TELOPT_COMPRESS)         /* check for version 1 */
      {
        if (dsock->inbuf[i-1] == (signed char) DO)                  /* start compressing   */
          compressStart(dsock, TELOPT_COMPRESS);
        else if (dsock->inbuf[i-1] == (signed char) DONT)           /* stop compressing    */
          compressEnd(dsock, TELOPT_COMPRESS, FALSE);
      }
      else if (dsock->inbuf[i] == (signed char) TELOPT_COMPRESS2)   /* check for version 2 */
      {
        if (dsock->inbuf[i-1] == (signed char) DO)                  /* start compressing   */
          compressStart(dsock, TELOPT_COMPRESS2);
        else if (dsock->inbuf[i-1] == (signed char) DONT)           /* stop compressing    */
          compressEnd(dsock, TELOPT_COMPRESS2, FALSE);
      }
    }
    else if (isprint(dsock->inbuf[i]) && isascii(dsock->inbuf[i]))
    {
      dsock->next_command[j++] = dsock->inbuf[i];
    }
  }
  dsock->next_command[j] = '\0';

  /* skip forward to the next line */
  while (dsock->inbuf[size] == '\n' || dsock->inbuf[size] == '\r')
  {
    dsock->bust_prompt = TRUE;   /* seems like a good place to check */
    size++;
  }

  /* use i as a static pointer */
  i = size;

  /* move the context of inbuf down */
  while (dsock->inbuf[size] != '\0')
  {
    dsock->inbuf[size - i] = dsock->inbuf[size];
    size++;
  }
  dsock->inbuf[size - i] = '\0';
}

void clear_socket(D_SOCKET *sock_new, int sock)
{
  memset(sock_new, 0, sizeof(*sock_new));

  sock_new->control        =  sock;
  sock_new->state          =  STATE_GET_ACCOUNT;
  sock_new->lookup_status  =  TSTATE_LOOKUP;
  sock_new->player         =  NULL;
  sock_new->top_output     =  0;
  sock_new->events         =  AllocList();
}

/* does the lookup, changes the hostname, and dies */
void *lookup_address(void *arg)
{
  LOOKUP_DATA *lData = (LOOKUP_DATA *) arg;
  struct hostent *from;

  /* do the lookup and store the result at &from */
  if( (from = gethostbyaddr(lData->buf, sizeof(lData->buf), AF_INET ) ) != NULL )
  {
     free( lData->dsock->hostname );
     lData->dsock->hostname = strdup( from->h_name );
  }

  /* set it ready to be closed or used */
  lData->dsock->lookup_status++;

  /* free the lookup data */
  free(lData->buf);
  free(lData);

  /* and kill the thread */
  pthread_exit(0);
}

void recycle_sockets()
{
  D_SOCKET *dsock;
  ITERATOR Iter;

  AttachIterator(&Iter, dsock_list);
  while ((dsock = (D_SOCKET *) NextInList(&Iter)) != NULL)
  {
    if (dsock->lookup_status != TSTATE_CLOSED) continue;

    /* remove the socket from the socket list */
    DetachFromList(dsock, dsock_list);

    /* close the socket */
    close(dsock->control);

    /* free the memory */
    free(dsock->hostname);

    /* free the list of events */
    //FreeList(dsock->events);

    /* stop compression */
    compressEnd(dsock, dsock->compressing, TRUE);

    /* put the socket in the free stack */
    PushStack(dsock, dsock_free);
  }
  DetachIterator(&Iter);
}
