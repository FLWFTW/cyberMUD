#ifndef __TABLES_H__
#define __TABLES_H__

/* FUNCTIONS */
int b_to_e( int b );

/* TABLES */

/* Connection states */
enum state_t { STATE_GET_ACCOUNT = 0, STATE_NEW_ACCOUNT, STATE_ASK_PASSWORD,
               STATE_MAIN_MENU, STATE_CHOOSE_CHAR, STATE_CHARGEN, STATE_CHARGEN_NAME,
               STATE_CHARGEN_GENDER, STATE_CHARGEN_RACE, STATE_CHARGEN_STATS,
               STATE_CHARGEN_HEIGHT, STATE_CHARGEN_WEIGHT, STATE_CHARGEN_EYECOLOR,
               STATE_PRESS_ENTER,
               STATE_NEW_NAME,
               STATE_NEW_PASSWORD, STATE_VERIFY_PASSWORD, STATE_PLAYING, STATE_WRITING,
               STATE_CLOSED };

enum wear_pos_t { WEAR_HEAD = 0, WEAR_EYES, WEAR_FACE, WEAR_NECK,
                  WEAR_BODY, WEAR_SHOULDERS, WEAR_ARMS, WEAR_HANDS, WEAR_WAIST,
                  WEAR_LEGS, WEAR_FEET, WEAR_SLUNG, WEAR_NONE, MAX_WEAR };
extern const char *wear_pos[];

enum bodyparts_t { BODY_HEAD = 0, BODY_EYES, BODY_FACE, BODY_NECK, BODY_LARM,
                   BODY_RARM, BODY_LHAND, BODY_RHAND, BODY_TORSO, BODY_GROIN, BODY_LLEG,
                   BODY_RLEG, BODY_LFOOT, BODY_RFOOT,
                   MAX_BODY };
extern const char *body_parts[];
extern size_t      body_aim_mod[];
extern size_t      body_hp_mod[];

enum gender_t { FEMALE=0, MALE, NONBINARY };

enum position_t { POS_UNCONSCIOUS = 0, POS_SLEEPING, POS_RESTRAINED, POS_RESTING, 
                  POS_SITTING, POS_PRONE, POS_KNEELING, POS_STANDING,
                  MAX_POS };
extern const char *positions[];

enum item_type_t { ITEM_CLOTHING = 0, ITEM_ARMOR, ITEM_ELECTRONICS, ITEM_COMPUTER, ITEM_MACHINE,
                   ITEM_SCRAP, ITEM_FURNITURE, ITEM_LOCKPICK, ITEM_SHEATH, ITEM_HOLSTER,
                   ITEM_CONTAINER, ITEM_BEVERAGE, ITEM_MAGAZINE, ITEM_BULLET, ITEM_BOOK, ITEM_MEMORY,
                   ITEM_CHIP, ITEM_KEY, ITEM_CORPSE, ITEM_FIREARM, ITEM_BLADE, ITEM_BLUDGEON, ITEM_DRUG,
                   ITEM_UNKNOWN, MAX_ITEM };
extern const char *item_type[];

enum ammo_type_t { AMMO_38SPC = 0, AMMO_9x19, AMMO_357MAG, AMMO_45APC, AMMO_10MM, AMMO_44MAG, AMMO_50AE,
                   AMMO_556NATO, AMMO_762x39, AMMO_762x51, AMMO_762x54R, AMMO_127x108, AMMO_145x114,
                   AMMO_10GAUGE, AMMO_12GAUGE, AMMO_20GAUGE, MAX_AMMO };
extern const char *ammo_type[];
extern const char *ammo_dice[];

enum damage_type_t  { DAMAGE_PIERCE = 0, DAMAGE_CUT, DAMAGE_BLUNT, DAMAGE_BURN, DAMAGE_PROJECTILE, DAMAGE_OTHER, MAX_DT };
enum trauma_level_t { TRAUMA_NONE = 0, TRAUMA_MILD, TRAUMA_MODERATE, TRAUMA_SEVERE, TRAUMA_CRITICAL, MAX_TRAUMA };
extern const char *wound_trauma[];
extern const char *blunt_trauma[];
extern const char *burn_trauma[];

enum exit_state_t { EXIT_FREE = 0, EXIT_JAMMED, EXIT_CLOSED, EXIT_OPEN, EXIT_BROKEN_OPEN, MAX_EXITSTATE };
enum lock_state_t { LOCK_FREE = 0, LOCK_JAMMED, LOCK_LOCKED, LOCK_UNLOCKED, MAX_LOCKSTATE };
enum lock_type_t  { LOCK_PINTUMBLER = 0, LOCK_COMBO, LOCK_ELECTRICPIN, LOCK_FINGERPRINT, LOCK_REMOTE, MAX_LOCKTYPE };
extern const char *exit_state[];
extern const char *lock_state[];
extern const char *lock_type[];

enum reset_type_t { RESET_EXIT = 0, RESET_MOB, RESET_OBJ, MAX_RESET };
enum cmd_type_t   { CMD_ACT = 0, CMD_COM, CMD_OOC, CMD_WIZ, MAX_CMD };

#endif
