#ifndef __TABLES_H__
#define __TABLES_H__

/* FUNCTIONS */
int b_to_e( int b );

/* TABLES */

/* Connection states */
enum state_tb { STATE_GET_ACCOUNT = 0, STATE_NEW_ACCOUNT, STATE_ASK_PASSWORD,
               STATE_MAIN_MENU, STATE_CHOOSE_CHAR, STATE_CHARGEN, STATE_CHARGEN_NAME,
               STATE_CHARGEN_GENDER, STATE_CHARGEN_RACE, STATE_CHARGEN_STATS,
               STATE_CHARGEN_HEIGHT, STATE_CHARGEN_WEIGHT, STATE_CHARGEN_EYECOLOR,
               STATE_PRESS_ENTER,
               STATE_NEW_NAME,
               STATE_NEW_PASSWORD, STATE_VERIFY_PASSWORD, STATE_PLAYING, STATE_WRITING,
               STATE_CLOSED };

enum wear_pos_tb { WEAR_HEAD = 0, WEAR_EYES, WEAR_FACE, WEAR_NECK,
                  WEAR_BODY, WEAR_SHOULDERS, WEAR_ARMS, WEAR_HANDS, WEAR_WAIST,
                  WEAR_LEGS, WEAR_FEET, WEAR_SLUNG, WEAR_NONE, MAX_WEAR };
extern const char *wear_pos[];

enum bodyparts_tb { BODY_HEAD = 0, BODY_EYES, BODY_FACE, BODY_NECK, BODY_LARM,
                   BODY_RARM, BODY_LHAND, BODY_RHAND, BODY_TORSO, BODY_GROIN, BODY_LLEG,
                   BODY_RLEG, BODY_LFOOT, BODY_RFOOT,
                   MAX_BODY };
extern const char *body_parts[];
extern size_t      body_aim_mod[];
extern size_t      body_hp_mod[];

enum gender_tb { FEMALE=0, MALE, NONBINARY };

enum position_tb { POS_UNCONSCIOUS = 0, POS_SLEEPING, POS_RESTRAINED, POS_RESTING, 
                  POS_SITTING, POS_PRONE, POS_KNEELING, POS_STANDING,
                  MAX_POS };
extern const char *positions[];

enum item_type_tb { ITEM_CLOTHING = 0, ITEM_ARMOR, ITEM_ELECTRONICS, ITEM_COMPUTER, ITEM_MACHINE,
                   ITEM_SCRAP, ITEM_FURNITURE, ITEM_LOCKPICK, ITEM_SHEATH, ITEM_HOLSTER,
                   ITEM_CONTAINER, ITEM_BEVERAGE, ITEM_MAGAZINE, ITEM_BULLET, ITEM_BOOK, ITEM_MEMORY,
                   ITEM_CHIP, ITEM_KEY, ITEM_CORPSE, ITEM_FIREARM, ITEM_BLADE, ITEM_BLUDGEON, ITEM_DRUG,
                   ITEM_UNKNOWN, ITEM_BANDOLIER,  MAX_ITEM };
extern const char *item_type[];

enum ammo_caliber_tb { AMMO_38SPC = 0, AMMO_9x19, AMMO_357MAG, AMMO_45APC, AMMO_10MM, AMMO_44MAG, AMMO_50AE,
                       AMMO_556NATO, AMMO_762x39, AMMO_762x51, AMMO_762x54R, AMMO_127x108, AMMO_145x114,
                       AMMO_10GAUGE, AMMO_12GAUGE, AMMO_20GAUGE, MAX_AMMO };
extern const char *ammo_caliber[];
extern const char *ammo_dice[];

extern const char *ammo_type[];

enum amomo_type_tb { AMMO_FMJ = 0, AMMO_JHP, AMMO_AP, AMMO_API, AMMO_EXP, MAX_AMMOTYPE };

enum damage_type_tb  { DAMAGE_PIERCE = 0, DAMAGE_CUT, DAMAGE_BLUNT, DAMAGE_BURN, DAMAGE_PROJECTILE, DAMAGE_EXPLOSIVE, DAMAGE_OTHER, MAX_DT };
enum trauma_level_tb { TRAUMA_NONE = 0, TRAUMA_MILD, TRAUMA_MODERATE, TRAUMA_SEVERE, TRAUMA_CRITICAL, MAX_TRAUMA };
extern const char *wound_trauma[];
extern const char *blunt_trauma[];
extern const char *burn_trauma[];

enum exit_state_tb { EXIT_FREE = 0, EXIT_JAMMED, EXIT_CLOSED, EXIT_OPEN, EXIT_BROKEN_OPEN, MAX_EXITSTATE };
enum lock_state_tb { LOCK_FREE = 0, LOCK_JAMMED, LOCK_LOCKED, LOCK_UNLOCKED, MAX_LOCKSTATE };
enum lock_type_tb  { LOCK_PINTUMBLER = 0, LOCK_COMBO, LOCK_ELECTRICPIN, LOCK_FINGERPRINT, LOCK_REMOTE, MAX_LOCKTYPE };
extern const char *exit_state[];
extern const char *lock_state[];
extern const char *lock_type[];

enum reset_type_tb { RESET_EXIT = 0, RESET_MOB, RESET_OBJ, MAX_RESET };
enum cmd_type_tb   { CMD_ACT = 0, CMD_COM, CMD_OOC, CMD_WIZ, MAX_CMD };
extern const char *command_types[];

enum sector_type_tb { SECTOR_INSIDE = 0, SECTOR_CITY, SECTOR_STREET, SECTOR_ROOFTOP, SECTOR_SEWER, SECTOR_TUNNEL,
                      SECTOR_GRASS, SECTOR_FIELD, SECTOR_DIRT, SECTOR_GRAVEL, SECTOR_FOREST, SECTOR_MOUNTAIN, SECTOR_OCEAN, SECTOR_SWAMP,
                      SECTOR_JUNKYARD, SECTOR_SNOW, SECTOR_DESERT, SECTOR_BEACH, SECTOR_ICE, SECTOR_MUD, SECTOR_CONCRETE,
                      SECTOR_WATER, SECTOR_UNDERWATER, SECTOR_FLYING, MAX_SECTOR };
extern const char *sector_types[];
#endif
