/*
 * This is the main headerfile
 */

#ifndef MUD_H
#define MUD_H

#include <zlib.h>
#include <pthread.h>
#include <arpa/telnet.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <math.h>


#include "list.h"
#include "stack.h"
#include "tables.h"
#include <jansson.h>
#include <bcrypt.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "handler_lua.h"

/************************
 * Standard definitions *
 ************************/

/* define TRUE and FALSE */
#ifndef FALSE
#define FALSE   0
#endif
#ifndef TRUE
#define TRUE    1
#endif

#define eTHIN   0
#define eBOLD   1

#define ADMIN_EMAIL "admin@cybermud.com"

/* A few globals */
#define MAX_STRING_LENGTH  10240
#define PULSES_PER_SECOND     4                   /* must divide 1000 : 4, 5 or 8 works */
#define MAX_BUFFER         40960                /* seems like a decent amount         */
#define MAX_OUTPUT         40960                /* well shoot me if it isn't enough   */
#define MAX_HELP_ENTRY     4096                   /* roughly 40 lines of blocktext      */
#define MUDPORT            9009                   /* just set whatever port you want    */
#define FILE_TERMINATOR    "EOF"                  /* end of file marker                 */
#define COPYOVER_FILE      "../txt/copyover.dat"  /* tempfile to store copyover data    */
#define EXE_FILE           "../src/cbm"     /* the name of the mud binary         */
#define FIRST_ROOM            1      /*The room new players start out in */

/* Thread states - please do not change the order of these states    */
#define TSTATE_LOOKUP          0  /* Socket is in host_lookup        */
#define TSTATE_DONE            1  /* The lookup is done.             */
#define TSTATE_WAIT            2  /* Closed while in thread.         */
#define TSTATE_CLOSED          3  /* Closed, ready to be recycled.   */

/* player levels */
#define LEVEL_GUEST            1  /* Dead players and actual guests  */
#define LEVEL_PLAYER           2  /* Almost everyone is this level   */
#define LEVEL_ADMIN            3  /* Any admin without shell access  */
#define LEVEL_GOD              4  /* Any admin with shell access     */

/* Communication Ranges */
#define COMM_LOCAL             0  /* same room only                  */
#define COMM_CHAT              1  /* global OOC                      */
#define COMM_LOG              10  /* admins only                     */

/* define simple types */
//typedef  unsigned char     bool;
typedef  short int         sh_int;


/******************************
 * End of standard definitons *
 ******************************/

/***********************
 * Defintion of Macros *
 ***********************/

#define UMIN(a, b)		((a) < (b) ? (a) : (b))
#define IS_ADMIN(dMob)          ((dMob->level) > LEVEL_PLAYER ? TRUE : FALSE)
#define IS_NPC(x)               ((x->socket) == NULL ? TRUE : FALSE )
#define IS_PC(x)                ((x->socket) == NULL ? FALSE : TRUE )
#define GENDER(x)               ((x->gender) == MALE ? "male" : (x->gender) == FEMALE ? "female" : "nonbinary")
#define POSSESSIVE(x)           ((x->gender) == MALE ? "his"  : (x->gender) == FEMALE ? "her"    : "their")
#define SUBJECTIVE(x)           ((x->gender) == MALE ? "he"   : (x->gender) == FEMALE ? "she"    : "they")
#define OBJECTIVE(x)            ((x->gender) == MALE ? "him"  : (x->gender) == FEMALE ? "her"    : "them")
#define MOBNAME(x)              ((x->socket) == NULL ? (x->sdesc) : (x->name) )
#define GENDERTERN(x, m, f, n)  (( (x->gender) == MALE ? (m) : (x->gender) == FEMALE ? (f) : (n) ))
#define ISCONTAINER(x)          (((x->type) == ITEM_CONTAINER || (x->type) == ITEM_CORPSE ||\
                                (x->type) == ITEM_HOLSTER || (x->type) == ITEM_SHEATH ||\
                                (x->type) == ITEM_MAGAZINE) ? TRUE : FALSE)
#define ISGUN(x)                ((x->type) == ITEM_FIREARM)
#define b_to_e( b )             (((b)>=BODY_HEAD && (b)<MAX_BODY)?b_to_e_table[(b)]:WEAR_NONE)
#define IS_FIGHTING( x )        (((x)->fighting) == NULL ? FALSE : TRUE )
#define atoi(x)                 (strtol((x), NULL, 10))
/***********************
 * End of Macros       *
 ***********************/

/******************************
 * New structures             *
 ******************************/

/* type defintions */
typedef struct  dSocket       D_SOCKET;
typedef struct  dAccount      D_ACCOUNT;
typedef struct  dRoom         D_ROOM;
typedef struct  dMobile       D_MOBILE;
typedef struct  dExit         D_EXIT;
typedef struct  dArea         D_AREA;
typedef struct  dObject       D_OBJECT;
typedef struct  dOffer        D_OFFER;
typedef struct  help_data     HELP_DATA;
typedef struct  lookup_data   LOOKUP_DATA;
typedef struct  event_data    EVENT_DATA;
typedef struct  skills        SKILLS;
typedef struct  dBodypart     D_BODYPART;
typedef struct  dEquipment    D_EQUIPMENT;
typedef struct  dReset        D_RESET;
typedef struct  command_data  D_COMMAND;
typedef struct  mAffects      D_MAFFECT;

/* the actual structures */
struct dRoom
{
   D_AREA           * area;
   unsigned int       vnum;
   char             * name;
   char             * description;
   unsigned int       sectortype;
   unsigned int       light;

   LIST             * scripts;
   LIST             * mobiles;
   LIST             * objects;
   LIST             * exits; 
};

struct dReset
{
   enum reset_type_tb type;
   int                location; //room vnum
   json_t           * data;
   void             * what; //pointer to what is reset, if NULL then the reset needs to be fired again.
};

struct dExit
{
   char             * name;
   char             * farside_name; //what people on the other side see when the exit is used.
   D_EXIT           * farside_exit;
   D_ROOM           * to_room;
   unsigned int       to_vnum;

   unsigned int       door_level; //Ranging from a hollow wood bedroom door (0) to a reinforced steel/kevlar blast vault whatever door (10)
   unsigned int       lock_level; //Ranging from the littl push-button lock on a bathroom door (0) to something you would find on a bank safe (10)
   enum lock_type_tb  lock_type;  //Pin-tumbler, combo/dial, electric pin, fingerprint, remote
   
   enum lock_state_tb  lock;
   enum exit_state_tb  exit;
};

/**
 * ivar guide:
 * armor:    1- stopping power
 *           2- material (0 = leather, 1 = steel, 2 = alloy, 3 = kevlar, 4 = Composite(Carbon Fiber/Graphene composite) 
 * firearms: 1- caliber
 *           2- fire rate 0 is bolt action, 1 is semi-auto, 2 is burst 3 is full auto
 *           3- feed mechanism (0=magazine, 1=single shot, 2+=internal magazine/tube and size)
 *           4- # of hands (1 or 2)
 * keys:     1- pins
 *           2- lock number
 * magazine: 1- caliber 
 *           2- capacity
 * bullet:   1- caliber
 *           2- bullet type( 1=ball, 2=JHP, 3=AP, 4=API, 5=explosive)
 */
struct dObject
{
   D_ROOM          * in_room;
   D_OBJECT        * in_object;
   D_MOBILE        * carried_by;

   unsigned int      vnum;
   char            * name;
   char            * sdesc;
   char            * ldesc;
   char            * guid;
   unsigned int      capacity_cm3;
   unsigned int      volume_cm3;
   unsigned int      weight_g;//weight in grams
   unsigned int      repair;//percent/100
   bool              can_get;

   int               ivar1, ivar2, ivar3, ivar4, ivar5, ivar6;
   char             *svar1, *svar2, *svar3, *svar4, *svar5, *svar6;

   enum wear_pos_tb  wear_pos;
   enum item_type_tb type;

   LIST            * contents;
   LIST            * scripts;
};

struct dArea
{
   char *name;
   char *author;
   char *filename;
   char *reset_script;
   time_t last_reset;
   unsigned int reset_interval;

   unsigned int r_low;
   unsigned int r_hi;
   unsigned int m_low;
   unsigned int m_hi;
   unsigned int o_low;
   unsigned int o_hi;

   LIST *rooms;
   LIST *mobiles;
   LIST *objects;
   LIST *resets;
};

struct dAccount
{
   char   *name;
   char   *password;
   char   *email;
   LIST   * characters;
   bool    acceptANSI;
};

struct dSocket
{
   D_MOBILE      * player;
   D_ACCOUNT     * account;
   LIST          * events;
   char          * hostname;
   char            inbuf[MAX_BUFFER];
   char            outbuf[MAX_OUTPUT];
   char            next_command[MAX_BUFFER];
   bool            bust_prompt;
   sh_int          lookup_status;
   enum state_tb   state;
   sh_int          control;
   sh_int          top_output;
   unsigned char   compressing;                 /* MCCP support */
   z_stream      * out_compress;                /* MCCP support */
   unsigned char * out_compress_buf;            /* MCCP support */

   char            loginName[MAX_BUFFER];

   char          * edit_buffer;  /* The PC's text editor */
   char          **edit_pointer; /* The text the PC is editing */
};

struct skills
{
   unsigned int combat;
   unsigned int engineering;
   unsigned int subterfuge;
   unsigned int medicine;
   unsigned int personality;
};

struct dBodypart
{
   int cur_hp;
   int max_hp;
   enum trauma_level_tb wound_trauma;
   enum trauma_level_tb blunt_trauma;
   enum trauma_level_tb burn_trauma;
};

struct dEquipment
{
   D_OBJECT   * worn[2];
};

struct mAffects
{
   bool               blind;
   bool               deaf;
   bool               mute;
   bool               poisoned;
};

struct dMobile
{
   D_SOCKET         * socket;
   D_ROOM           * room;
   D_RESET          * reset;
   LIST             * events;
   char             * name;
   char             * password;
   char             * prompt;
   sh_int             level;
   enum gender_tb      gender;
   unsigned int       vnum;
   enum position_tb    position;
   SKILLS           * skills;
   D_MOBILE         * fighting;
   D_MAFFECT          affects;
   bool               quit;

   int                cur_hp;
   int                max_hp;

   char             * sdesc;
   char             * ldesc;
   char             * guid;

   unsigned int       brains;
   unsigned int       brawn;
   unsigned int       senses;
   unsigned int       stamina;
   unsigned int       coordination;
   unsigned int       cool;
   unsigned int       luck;
 
   char             * race;
   char             * eyecolor;
   char             * eyeshape;
   char             * haircolor;
   char             * hairstyle;
   char             * skincolor;
   char             * build;
   char             * height;
   unsigned int       age;
   float              btc;
   unsigned int       signal;
   unsigned int       cur_bandwidth;
   unsigned int       max_bandwidth;
   unsigned int       encumberance;

   D_BODYPART       * body[MAX_BODY];
 
   D_OFFER          * offer_left;
   D_OFFER          * offer_right;

   D_OBJECT         * hold_right;
   D_OBJECT         * hold_left;
 
   D_EQUIPMENT      * equipment[WEAR_NONE];
 
   char             * citizenship;
   char             * association;

   LIST          * com_cmd_list;
   LIST          * act_cmd_list;
   LIST          * ooc_cmd_list;
   LIST          * wiz_cmd_list;
   LIST          * scripts;

};

struct help_data
{
   time_t              load_time;
   char              * keyword;
   char              * text;
   int                 level;
};

struct lookup_data
{
  D_SOCKET          * dsock;   /* the socket we wish to do a hostlookup on */
  char              * buf;     /* the buffer it should be stored in        */
};

struct typCmd
{
  char              * cmd_name;
  void             (* cmd_funct)(D_MOBILE *dMOb, char *arg);
  sh_int              level;
  enum cmd_type_tb     type;
};

struct command_data
{
   void           (* func)(D_MOBILE *dMob, char *arg);
   D_MOBILE        * dMob;
   char            arg[MAX_STRING_LENGTH];
};

struct dOffer
{
   D_OBJECT         * what;
   D_MOBILE         * to;
   time_t             when;
};

typedef struct buffer_type
{
  char              * data;        /* The data                      */
  int                 len;         /* The current len of the buffer */
  int                 size;        /* The allocated size of data    */
} BUFFER;

/* here we include external structure headers */
#include "event.h"

/******************************
 * End of new structures      *
 ******************************/

/***************************
 * Global Variables        *
 ***************************/

extern  D_ROOM      *   froom;
extern  STACK       *   dsock_free;       /* the socket free list               */
extern  LIST        *   dsock_list;       /* the linked list of active sockets  */
extern  STACK       *   dmobile_free;     /* the mobile free list               */
extern  LIST        *   dmobile_list;     /* the mobile list of active mobiles  */
extern  LIST        *   droom_list;       /* linked list of all loaded rooms    */
extern  LIST        *   darea_list;       /* linked list of all loaded areas    */
extern  LIST        *   object_protos;    /* linked list of all object prototypes */
extern  LIST        *   mobile_protos;    /* linked list of all mobile prototypes */
extern  LIST        *   dobject_list;     /* linked list of all loaded objects  */
extern  LIST        *   help_list;        /* the linked list of help files      */
extern  const struct    typCmd tabCmd[];  /* the command table                  */
extern  bool            shut_down;        /* used for shutdown                  */
extern  char        *   greeting;         /* the welcome greeting               */
extern  char        *   motd;             /* the MOTD help file                 */
extern  char        *   death_message;    /* what a player sees when they die   */
extern  int             control;          /* boot control socket thingy         */
extern  time_t          current_time;     /* let's cut down on calls to time()  */
extern  time_t          boot_time;        /* What time the MUD booted up        */
extern  const int       b_to_e_table[];   /* bodypart to equipment wear position */
extern  D_MOBILE    *   current_mob;      /* the mobile currently being worked on */

/*************************** 
 * End of Global Variables *
 ***************************/

/***********************
 *    MCCP support     *
 ***********************/

extern const unsigned char compress_will[];
extern const unsigned char compress_will2[];
extern const unsigned char do_echo[];
extern const unsigned char dont_echo[];

#define TELOPT_COMPRESS       85
#define TELOPT_COMPRESS2      86
#define COMPRESS_BUF_SIZE   8192

/***********************
 * End of MCCP support *
 ***********************/

/***********************************
 * Prototype function declerations *
 ***********************************/

/* more compact */
#define  D_S         D_SOCKET
#define  D_M         D_MOBILE

#define  buffer_new(size)             __buffer_new     ( size)
#define  buffer_strcat(buffer,text)   __buffer_strcat  ( buffer, text )

char  *crypt                  ( const char *key, const char *salt );

/*
 * socket.c
 */
int   init_socket             ( void );
bool  new_socket              ( int sock );
void  close_socket            ( D_S *dsock, bool reconnect );
bool  read_from_socket        ( D_S *dsock );
bool  text_to_socket          ( D_S *dsock, const char *txt, ... );  /* sends the output directly */
void  text_to_buffer          ( D_S *dsock, const char *txt, ... );  /* buffers the output        */
void  send_json_m             ( D_M *dMob, const char *txt, ... );   /* Sends json data to a mobile */
void  text_to_mobile          ( D_M *dMob, const char *txt, ... );   /* buffers the output        */
void  text_to_mobile_j        ( D_M *dMob, const char *type, const char *txt, ... );   /* buffers the output        */
int   next_cmd_from_buffer    ( D_S *dsock );
bool  flush_output            ( D_S *dsock );
void  clear_socket            ( D_S *sock_new, int sock );
void  recycle_sockets         ( void );
void *lookup_address          ( void *arg );

/*
 * handler_connections.c
 */
void  handle_new_connections  ( D_S *dsock, char *arg );

/*
 * interpret.c
 */
void  handle_cmd_input        ( D_MOBILE *dMob, char *arg );
void  handle_edit_input       ( D_SOCKET *dsock, char *arg );

/*
 * io.c
 */
void    log_string            ( const char *txt, ... );
void    bug                   ( const char *txt, ... );
time_t  last_modified         ( char *helpfile );
char   *read_help_entry       ( const char *helpfile );     /* pointer         */
char   *fread_line            ( FILE *fp );                 /* pointer         */
char   *fread_string          ( FILE *fp );                 /* allocated data  */
char   *fread_word            ( FILE *fp );                 /* pointer         */
int     fread_number          ( FILE *fp );                 /* just an integer */

/* 
 * strings.c
 */
bool   is_number              ( char *str );
int    isascii                ( int c );
char   *one_arg               ( char *fStr, char *bStr );
char   *strdup                ( const char *s );
char   *strdupf               ( const char *s, ... );
int     strcasecmp            ( const char *s1, const char *s2 );
char   *strcasestr            ( const char *s1, const char *s2 );
bool    is_prefix             ( const char *aStr, const char *bStr );
char   *capitalize            ( char *txt );
BUFFER *__buffer_new          ( int size );
void    __buffer_strcat       ( BUFFER *buffer, const char *text );
void    buffer_free           ( BUFFER *buffer );
void    buffer_clear          ( BUFFER *buffer );
int     bprintf               ( BUFFER *buffer, char *fmt, ... );

/*
 * help.c
 */
HELP_DATA *new_help           ( void );
bool  check_help              ( D_M *dMob, char *helpfile );
void  load_helps              ( void );
void  cmd_hedit               ( D_MOBILE *dMob, char *arg );

/*
 * objects.c
 */
D_OBJECT *get_object_by_vnum       ( unsigned int vnum );
void free_object              ( D_OBJECT *object );
void object_to_room           ( D_OBJECT *object, D_ROOM *room );
void object_to_mobile         ( D_OBJECT *object, D_MOBILE *dMob );
void object_to_object         ( D_OBJECT *object, D_OBJECT *container );
void object_from_object       ( D_OBJECT *object, D_OBJECT *container );
void object_from_room         ( D_OBJECT *object, D_ROOM *room );
void object_from_mobile       ( D_OBJECT *object, D_MOBILE *dMob );
bool equip_object             ( D_MOBILE *dMob, D_OBJECT *obj );
D_OBJECT *get_object_list     ( const char *name, LIST *list );
D_OBJECT *spawn_object        ( unsigned int vnum );
D_OBJECT *new_object          ();
size_t total_volume           ( D_OBJECT *obj );
bool object_can_fit           ( D_OBJECT *object, D_OBJECT *container );
void check_objects            ();
D_OBJECT *get_object_mob      ( D_MOBILE *dMob, char *name );
bool can_lift                 ( D_MOBILE *dMob, D_OBJECT *dObj );

/*
 * utils.c
 */
bool is_crippled( D_MOBILE *dMob, enum bodyparts_tb loc );
void socket_cleanup( LIST *list );
void account_cleanup( LIST *list );
void mobile_cleanup( LIST *list );
void object_cleanup( LIST *list );
void exit_cleanup( LIST *list );
void room_cleanup( LIST *list );
void area_cleanup( LIST *list );
unsigned int calc_brains      ( D_MOBILE *dMob );
unsigned int calc_brawn       ( D_MOBILE *dMob );
unsigned int calc_stamina     ( D_MOBILE *dMob );
unsigned int calc_senses      ( D_MOBILE *dMob );
unsigned int calc_cool        ( D_MOBILE *dMob );
unsigned int calc_coordination( D_MOBILE *dMob );
unsigned int calc_luck        ( D_MOBILE *dMob );
D_OBJECT *get_armor_pos( D_MOBILE *dMob, enum wear_pos_tb pos );
char *gen_guid();
D_EXIT *get_exit_by_name      ( D_ROOM *room, char *name );
bool  check_name              ( const char *name );
void  clear_mobile            ( D_M *dMob );
void  communicate             ( D_M *dMob, char *txt, int range );
void  load_muddata            ( bool fCopyOver );
char *get_time                ( void );
void  copyover_recover        ( void );
D_M  *check_reconnect         ( char *player );
void  free_account            ( D_ACCOUNT *account );
D_ACCOUNT *new_account        ();
void free_mobile              (D_MOBILE *dMob);
void free_mobile_proto        (D_MOBILE *dMob);
void free_room                ( D_ROOM *room );
void free_reset               ( D_RESET *reset );
void free_exit                ( D_EXIT *exit );
D_EXIT *new_exit              ();
D_ROOM *new_room              ();
D_AREA *new_area              ();
D_MOBILE *new_mobile          ();
D_RESET *new_reset            ();
D_ROOM *get_room_by_vnum      ( unsigned int vnum );
D_MOBILE *spawn_mobile        ( unsigned int vnum );
D_MOBILE *get_mobile_by_vnum  ( unsigned int vnum );
D_MOBILE *get_mobile_list     ( const char *name, LIST *list );
D_ROOM *mob_to_room           ( D_MOBILE *dMob, D_ROOM *to );
D_ROOM *obj_to_room           ( D_OBJECT *dObj, D_ROOM *to );
char *proper                  ( const char *word );
void sentence_case            ( char *sentence );
void show_mob_obj_list        ( D_MOBILE *dMob, LIST *list, size_t indent );
void check_mobiles            ();
void check_rooms              ();
void check_areas              ( bool force_reset );
bool is_name                  ( char *name, char *namelist );
D_OBJECT *make_corpse         ( D_MOBILE *dMob );
size_t roll                   ( size_t min, size_t max );
size_t dice                   ( char *str );

/*
 * action_safe.c
 */
void  cmd_lock                ( D_M *dMob, char *arg );
void  cmd_force               ( D_M *dMob, char *arg );
void  cmd_unlock              ( D_M *dMob, char *arg );
void  cmd_examine             ( D_M *dMob, char *arg );
void  cmd_draw                ( D_M *dMob, char *arg );
void  cmd_unsling             ( D_M *dMob, char *arg );
void  cmd_holster             ( D_M *dMob, char *arg );
void  cmd_sheath              ( D_M *dMob, char *arg );
void  cmd_skillset            ( D_M *dMob, char *arg );
void  cmd_sling               ( D_M *dMob, char *arg );
void  cmd_areas               ( D_M *dMob, char *arg );
void  cmd_say                 ( D_M *dMob, char *arg );
void  cmd_chat                ( D_M *dMob, char *arg );
void  cmd_goto                ( D_M *dMob, char *arg );
void  cmd_look                ( D_M *dMob, char *arg );
void  cmd_score               ( D_M *dMob, char *arg );
void  cmd_quit                ( D_M *dMob, char *arg );
void  cmd_qui                 ( D_M *dMob, char *arg );
void  cmd_reboot              ( D_M *dMob, char *arg );
void  cmd_reboo               ( D_M *dMob, char *arg );
void  cmd_shutdown            ( D_M *dMob, char *arg );
void  cmd_shutdow             ( D_M *dMob, char *arg );
void  cmd_commands            ( D_M *dMob, char *arg );
void  cmd_who                 ( D_M *dMob, char *arg );
void  cmd_help                ( D_M *dMob, char *arg );
void  cmd_compress            ( D_M *dMob, char *arg );
void  cmd_save                ( D_M *dMob, char *arg );
void  cmd_slay                ( D_M *dMob, char *arg );
void  cmd_copyover            ( D_M *dMob, char *arg );
void  cmd_linkdead            ( D_M *dMob, char *arg );
void  cmd_north               ( D_M *dMob, char *arg );
void  cmd_south               ( D_M *dMob, char *arg );
void  cmd_east                ( D_M *dMob, char *arg );
void  cmd_northeast           ( D_M *dMob, char *arg );
void  cmd_northwest           ( D_M *dMob, char *arg );
void  cmd_west                ( D_M *dMob, char *arg );
void  cmd_southeast           ( D_M *dMob, char *arg );
void  cmd_southwest           ( D_M *dMob, char *arg );
void  cmd_up                  ( D_M *dMob, char *arg );
void  cmd_down                ( D_M *dMob, char *arg );
void  cmd_prompt              ( D_M *dMob, char *arg );
void  cmd_ospawn              ( D_M *dMob, char *arg );
void  cmd_mspawn              ( D_M *dMob, char *arg );
void  cmd_get                 ( D_M *dMob, char *arg );
void  cmd_drop                ( D_M *dMob, char *arg );
void  cmd_wear                ( D_M *dMob, char *arg );
void  cmd_equipment           ( D_M *dMob, char *arg );
void  cmd_ostat               ( D_M *dMob, char *arg );
void  cmd_olist               ( D_M *dMob, char *arg );
void  cmd_mlist               ( D_M *dMob, char *arg );
void  cmd_enter               ( D_M *dMob, char *arg );
void  cmd_open                ( D_M *dMob, char *arg );
void  cmd_close               ( D_M *dMob, char *arg );
void  cmd_inventory           ( D_M *dMob, char *arg );
void  cmd_remove              ( D_M *dMob, char *arg );
void  cmd_put                 ( D_M *dMob, char *arg );
void  cmd_stow                ( D_M *dMob, char *arg );
void  cmd_give                ( D_M *dMob, char *arg );
void  cmd_ungive              ( D_M *dMob, char *arg );
void  cmd_accept              ( D_M *dMob, char *arg );
void  cmd_time                ( D_M *dMob, char *arg );
void  cmd_restore             ( D_M *dMob, char *arg );
void  cmd_kneel               ( D_M *dMob, char *arg );
void  cmd_prone               ( D_M *dMob, char *arg );
void  cmd_rest                ( D_M *dMob, char *arg );
void  cmd_sleep               ( D_M *dMob, char *arg );
void  cmd_sit                 ( D_M *dMob, char *arg );
void  cmd_stand               ( D_M *dMob, char *arg );
void  cmd_peace               ( D_M *dMob, char *arg );


void  cmd_fill                ( D_M *dMob, char *arg );
/*
 * accounts.c
 */
D_ACCOUNT *load_account( const char *name, const char *password );
void save_account( D_ACCOUNT *account );

/*
 * mccp.c
 */
bool  compressStart           ( D_S *dsock, unsigned char teleopt );
bool  compressEnd             ( D_S *dsock, unsigned char teleopt, bool forced );

/*
 * save.c
 */
void  save_player             ( D_M *dMob );
D_M  *load_player             ( const char *name );

/*
 * colors.c
 */
char *strip_color_codes( char *msg );
char *parse_color_codes( char *msg );

/*
 * world.c
 */
void load_areas();
void link_exits();

/*
 * handler_json.c
 */
D_ACCOUNT *json_to_account( json_t *json );
D_RESET   *json_to_reset( json_t *json );
D_ROOM *json_to_room( json_t *json );
D_OBJECT *json_to_object( json_t *json );
D_MOBILE *json_to_mobile( json_t *json );
HELP_DATA *json_to_help( json_t *json );
json_t *help_to_json( HELP_DATA *help );
json_t *exit_to_json( D_EXIT *exit );
json_t *mobile_to_json( D_MOBILE *dMob, bool equipment );
json_t *mobile_to_json_cli( D_MOBILE *dMob );
json_t *player_to_json( D_MOBILE *dMob, bool equipment );
json_t *object_to_json( D_OBJECT *dObj );
json_t *object_to_json_cli( D_OBJECT *obj );
json_t *json_from_keys( json_t *parent, int n, ... );
json_t *room_to_json( D_ROOM *room );
json_t *reset_to_json( D_RESET *reset );
json_t *areaheader_to_json( D_AREA *area );

/*
 * comm.c
 */
void echo_room( D_ROOM *room, char *type, char *txt, ... );
void echo_around( D_MOBILE *dMob, char *type, char *txt, ... );
void echo_around_two( D_MOBILE *one, D_MOBILE *two, char *type, char *txt, ... );

/*
 * reset.c
 */
json_t *object_to_reset( D_OBJECT *obj );
json_t *mobile_to_reset( D_MOBILE *dMob );
D_OBJECT *reset_to_object( json_t *data );
D_MOBILE *reset_to_mobile( json_t *data );
void reset_area( D_AREA *pArea );

/*
 * action_wiz.c
 */
void cmd_resetarea( D_MOBILE *dMob, char *arg );
void cmd_astat( D_MOBILE *dMob, char *arg );

/*
 * building.c
 */
void cmd_mstat( D_MOBILE *dMob, char *arg );
void cmd_guiredit( D_MOBILE *dMob, char *arg );
void cmd_instazone( D_MOBILE *dMob, char *arg );
void cmd_oset( D_MOBILE *dMOb, char *arg );
void cmd_eset( D_MOBILE *dMOb, char *arg );
void cmd_redit( D_MOBILE *dMob, char *arg );
void cmd_savearea( D_MOBILE *dMob, char *arg );
void cmd_ocreate( D_MOBILE *dMob, char *arg );
void cmd_mcreate( D_MOBILE *dMob, char *arg );
void cmd_mset( D_MOBILE *dMob, char *arg );
void cmd_makearea( D_MOBILE *dMob, char *arg );
void cmd_aset( D_MOBILE *dMob, char *arg );

/*
 * action_combat.c
 */
void cmd_load( D_MOBILE *dMob, char *arg );
void cmd_fire( D_MOBILE *dMob, char *arg );

/*
 * combat.c
 */
void fire( D_MOBILE *shooter, D_MOBILE *target, D_OBJECT *firearm, enum bodyparts_tb aim );
int damage( D_MOBILE *target, int amount, enum bodyparts_tb location, enum damage_type_tb type );
void kill( D_MOBILE *dMob );

/*
 * table translation functions
 */

size_t get_bodypart_code( const char *str );
size_t get_damagetype_code( const char *str );

/*
 * skills.c
 */
void load_skill_list();
void save_skill_list();

/*******************************
 * End of prototype declartion *
 *******************************/

#endif  /* MUD_H */
