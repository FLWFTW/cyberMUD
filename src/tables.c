/**
 * @file tables.c
 * @author Will Sayin
 * @version 1.0
 *
 * @section DESCRIPTION
 * Tables and table conversions.
 */
#include "mud.h"

const char *item_type[] = {    [ITEM_CLOTHING]    = "clothing",
                               [ITEM_ARMOR]       = "armor",
                               [ITEM_FIREARM]     = "firearm",
                               [ITEM_BLADE]       = "blade",
                               [ITEM_BLUDGEON]    = "bludgeon",
                               [ITEM_ELECTRONICS] = "electronics",
                               [ITEM_COMPUTER]    = "computer",
                               [ITEM_MACHINE]     = "machine",
                               [ITEM_SCRAP]       = "scrap",
                               [ITEM_FURNITURE]   = "furniture",
                               [ITEM_LOCKPICK]    = "lockpick",
                               [ITEM_SHEATH]      = "sheath",
                               [ITEM_HOLSTER]     = "holster",
                               [ITEM_CONTAINER]   = "container",
                               [ITEM_BEVERAGE]    = "beverage",
                               [ITEM_MAGAZINE]    = "magazine",
                               [ITEM_BANDOLIER]   = "bandolier",
                               [ITEM_BULLET]      = "bullet",
                               [ITEM_BOOK]        = "book",
                               [ITEM_MEMORY]      = "memory",
                               [ITEM_CHIP]        = "chip",
                               [ITEM_KEY]         = "key",
                               [ITEM_CORPSE]      = "corpse",
                               [ITEM_DRUG]        = "drugs",
                               [ITEM_UNKNOWN]     = "unknown",
                               [MAX_ITEM]         = 0 };

const char *wear_pos[] = {     [WEAR_HEAD]      = "head",
                               [WEAR_EYES]      = "eyes",
                               [WEAR_FACE]      = "face",
                               [WEAR_NECK]      = "neck",
                               [WEAR_BODY]      = "body",
                               [WEAR_SHOULDERS] = "shoulders",
                               [WEAR_ARMS]      = "arms", 
                               [WEAR_HANDS]     = "hands",
                               [WEAR_WAIST]     = "waist",
                               [WEAR_LEGS]      = "legs",
                               [WEAR_FEET]      = "feet",
                               [WEAR_SLUNG]     = "slung",
                               [WEAR_NONE]      = "none",
                               [MAX_WEAR]       = 0 };

size_t body_aim_mod[]    = {   [BODY_HEAD]    = 15,
                               [BODY_EYES]    = 5,
                               [BODY_FACE]    = 10,
                               [BODY_NECK]    = 10,
                               [BODY_LARM]    = 25,
                               [BODY_RARM]    = 25,
                               [BODY_LHAND]   = 10,
                               [BODY_RHAND]   = 10,
                               [BODY_TORSO]   = 90,
                               [BODY_GROIN]   = 70,
                               [BODY_LLEG]    = 25,
                               [BODY_RLEG]    = 25,
                               [BODY_LFOOT]   = 10,
                               [BODY_RFOOT]   = 10,
                               [MAX_BODY]     = 100 };

const char *body_parts[] = {   [BODY_HEAD]    = "head", 
                               [BODY_EYES]    = "eyes",
                               [BODY_FACE]    = "face",
                               [BODY_NECK]    = "neck",
                               [BODY_LARM]    = "left arm",
                               [BODY_RARM]    = "right arm",
                               [BODY_LHAND]   = "left hand",
                               [BODY_RHAND]   = "right hand",
                               [BODY_TORSO]   = "torso",
                               [BODY_GROIN]   = "groin",
                               [BODY_LLEG]    = "left leg",
                               [BODY_RLEG]    = "right leg",
                               [BODY_LFOOT]   = "left foot",
                               [BODY_RFOOT]   = "right foot",
                               [MAX_BODY]     = 0 };

size_t get_bodypart_code( const char *str )
{
   size_t i;
   for( i = 0; i < MAX_BODY; i++ )
   {
      if( !strcasecmp( body_parts[i], str ) )
         break;
   }

   return i;
}

const char *wound_trauma[] = { [TRAUMA_NONE]     = "none",
                               [TRAUMA_MILD]     = "leaking",
                               [TRAUMA_MODERATE] = "bleeding",
                               [TRAUMA_SEVERE]   = "pouring",
                               [TRAUMA_CRITICAL] = "severed",
                               [MAX_TRAUMA]      = 0};

const char *blunt_trauma[] = { [TRAUMA_NONE]     = "none",
                               [TRAUMA_MILD]     = "bruised",
                               [TRAUMA_MODERATE] = "battered",
                               [TRAUMA_SEVERE]   = "smashed",
                               [TRAUMA_CRITICAL] = "broken",
                               [MAX_TRAUMA]      = 0};

const char *burn_trauma[]  = { [TRAUMA_NONE]     = "none",
                               [TRAUMA_MILD]     = "irritated",
                               [TRAUMA_MODERATE] = "blistered",
                               [TRAUMA_SEVERE]   = "burnt",
                               [TRAUMA_CRITICAL] = "charred",
                               [MAX_TRAUMA]      = 0};

const char *damage_type[]  = { [DAMAGE_PIERCE]      = "piercing",
                               [DAMAGE_CUT]         = "cutting",
                               [DAMAGE_BLUNT]       = "blunt",
                               [DAMAGE_BURN]        = "burning",
                               [DAMAGE_PROJECTILE]  = "projectile",
                               [DAMAGE_OTHER]       = "other",
                               [MAX_DT]             = 0};

size_t get_damagetype_code( const char *str )
{
   size_t i;
   for( i = 0; i < MAX_DT; i++ )
      if( !strcasecmp( str, damage_type[i] ) )
         break;
   return i;
}

const char *reset_types[]  = { [RESET_EXIT]      = "exit",
                               [RESET_MOB]       = "mob",
                               [RESET_OBJ]       = "obj",
                               [MAX_RESET]       = 0};

const char *positions[]    = { [POS_UNCONSCIOUS] = "unconscious",
                               [POS_SLEEPING]    = "sleeping",
                               [POS_RESTRAINED]  = "restrained",
                               [POS_RESTING]     = "resting",
                               [POS_SITTING]     = "sitting",
                               [POS_PRONE]       = "prone",
                               [POS_KNEELING]    = "kneeling",
                               [POS_STANDING]    = "standing",
                               [MAX_POS]         = 0};

const int b_to_e_table[] = {   [BODY_HEAD]       = WEAR_HEAD,
                               [BODY_EYES]       = WEAR_EYES,
                               [BODY_FACE]       = WEAR_FACE,
                               [BODY_NECK]       = WEAR_NECK,
                               [BODY_LARM]       = WEAR_ARMS,
                               [BODY_RARM]       = WEAR_ARMS,
                               [BODY_LHAND]      = WEAR_HANDS,
                               [BODY_RHAND]      = WEAR_HANDS,
                               [BODY_TORSO]      = WEAR_BODY,
                               [BODY_GROIN]      = WEAR_WAIST,
                               [BODY_LLEG]       = WEAR_LEGS,
                               [BODY_RLEG]       = WEAR_LEGS,
                               [BODY_LFOOT]      = WEAR_FEET,
                               [BODY_RFOOT]      = WEAR_FEET,
                               [MAX_BODY]        = WEAR_BODY };

const char *ammo_caliber[]= { [AMMO_38SPC]       = "38 Special",
                              [AMMO_9x19]        = "9mm Luger",
                              [AMMO_357MAG]      = ".357 Magnum",
                              [AMMO_45APC]       = ".45 APC",
                              [AMMO_10MM]        = "10mm Auto",
                              [AMMO_44MAG]       = ".44 Magnum",
                              [AMMO_50AE]        = ".50 AE",
                              [AMMO_556NATO]     = "5.56mm NATO",
                              [AMMO_762x39]      = "7.62 Kalashnikov",
                              [AMMO_762x51]      = "7.62 NATO",
                              [AMMO_762x54R]     = "7.62 Nagant",
                              [AMMO_127x108]     = "12.7x108mm",
                              [AMMO_145x114]     = "14.5x114mm",
                              [AMMO_10GAUGE]     = "10 Gauge",
                              [AMMO_12GAUGE]     = "12 Gauge",
                              [AMMO_20GAUGE]     = "20 Gauge",
                              [MAX_AMMO]         = 0};

const char *ammo_type[] = {   [AMMO_FMJ]         = "FMJ", /* Full Metal Jacket */
                              [AMMO_JHP]         = "JHP", /* Jacketed Hollow Point */
                              [AMMO_AP]          = "AP",  /* Armor Piercing */
                              [AMMO_API]         = "API", /* Armor Piercing, Incindiery */
                              [AMMO_EXP]         = "EXP", /* Explosive */
                              [MAX_AMMOTYPE]     = 0};

const char *ammo_dice[]   = { [AMMO_38SPC]       = "6d2+6", //0 between 12-18
                              [AMMO_9x19]        = "6d2+9", //1 between 15-21
                              [AMMO_45APC]       = "8d2+8", //3 between 16-24
                              [AMMO_357MAG]      = "8d2+10",//2 between 18-26
                              [AMMO_10MM]        = "8d2+12",//4 between 20-26
                              [AMMO_44MAG]       = "8d2+16",//5 between 24-28
                              [AMMO_50AE]        = "8d2+20",//6 between 28-36
                              [AMMO_556NATO]     = "6d2+24",//7 between 30-36
                              [AMMO_762x39]      = "6d2+24",//8 between 30-36
                              [AMMO_762x51]      = "6d2+28",//9 between 34-40
                              [AMMO_762x54R]     = "6d2+30",//10 between 36-42
                              [AMMO_127x108]     = "1d10+40",//11 between 40-50
                              [AMMO_145x114]     = "1d10+45",//12 between 45-55
                              [AMMO_10GAUGE]     = "2d10+40",//13 between 42-60
                              [AMMO_12GAUGE]     = "2d10+35",//14 between 37-55
                              [AMMO_20GAUGE]     = "2d10+30",//15 between 32-50
                              [MAX_AMMO]         = 0};

/* Effective range of each round in meters */
const int ammo_range[]    = { [AMMO_38SPC]       = 10,
                              [AMMO_9x19]        = 18,
                              [AMMO_45APC]       = 20,
                              [AMMO_357MAG]      = 12,
                              [AMMO_10MM]        = 20,
                              [AMMO_44MAG]       = 14,
                              [AMMO_50AE]        = 15,
                              [AMMO_556NATO]     = 100,
                              [AMMO_762x39]      = 75,
                              [AMMO_762x51]      = 150,
                              [AMMO_762x54R]     = 125,
                              [AMMO_127x108]     = 500,
                              [AMMO_145x114]     = 750,
                              [AMMO_10GAUGE]     = 8,
                              [AMMO_12GAUGE]     = 6,
                              [AMMO_20GAUGE]     = 4,
                              [MAX_AMMO]         = 0};

const char *exit_state[]  = { [EXIT_FREE]        = "free",
                              [EXIT_JAMMED]      = "jammed",
                              [EXIT_CLOSED]      = "closed",
                              [EXIT_OPEN]        = "open",
                              [EXIT_BROKEN_OPEN] = "broken",
                              [MAX_EXITSTATE]    = 0};
                  
const char *lock_state[]  = { [LOCK_FREE]        = "free",
                              [LOCK_JAMMED]      = "jammed",
                              [LOCK_LOCKED]      = "locked",
                              [LOCK_UNLOCKED]    = "unlocked",
                              [MAX_LOCKSTATE]    = 0};

const char *lock_type[]   = { [LOCK_PINTUMBLER]  = "pintumbler",
                              [LOCK_COMBO]       = "combo",
                              [LOCK_ELECTRICPIN] = "electric",
                              [LOCK_FINGERPRINT] = "fingerprint",
                              [LOCK_REMOTE]      = "remote",
                              [MAX_LOCKTYPE]     = 0};

size_t body_hp_mod[]     = {   [BODY_HEAD]    = 15,
                               [BODY_EYES]    = 2,
                               [BODY_FACE]    = 10,
                               [BODY_NECK]    = 10,
                               [BODY_LARM]    = 25,
                               [BODY_RARM]    = 25,
                               [BODY_LHAND]   = 3,
                               [BODY_RHAND]   = 3,
                               [BODY_TORSO]   = 50,
                               [BODY_GROIN]   = 10,
                               [BODY_LLEG]    = 25,
                               [BODY_RLEG]    = 25,
                               [BODY_LFOOT]   = 3,
                               [BODY_RFOOT]   = 3,
                               [MAX_BODY]     = 100 };

const char *command_types[] = {[CMD_ACT]      = "action",
                               [CMD_COM]      = "communication",
                               [CMD_OOC]      = "ooc",
                               [CMD_WIZ]      = "wiz",
                               [MAX_CMD]      = 0};

const char *sector_types[] = { [SECTOR_INSIDE]     = "inside",
                               [SECTOR_CITY]       = "city",
                               [SECTOR_STREET]     = "street",
                               [SECTOR_ROOFTOP]    = "rooftop",
                               [SECTOR_SEWER]      = "sewer",
                               [SECTOR_TUNNEL]     = "tunnel",
                               [SECTOR_GRASS]      = "grass",
                               [SECTOR_FIELD]      = "field",
                               [SECTOR_DIRT]       = "dirt",
                               [SECTOR_GRAVEL]     = "gravel",
                               [SECTOR_FOREST]     = "forest",
                               [SECTOR_MOUNTAIN]   = "mountain",
                               [SECTOR_OCEAN]      = "ocean",
                               [SECTOR_SWAMP]      = "swamp",
                               [SECTOR_JUNKYARD]   = "junkyard",
                               [SECTOR_SNOW]       = "snow",
                               [SECTOR_DESERT]     = "desert",
                               [SECTOR_BEACH]      = "beach",
                               [SECTOR_WATER]      = "water",
                               [SECTOR_UNDERWATER] = "underwater",
                               [SECTOR_FLYING]     = "flying",
                               [SECTOR_ICE]        = "ice",
                               [SECTOR_MUD]        = "mud",
                               [SECTOR_CONCRETE]   = "concrete",
                               [MAX_SECTOR]        = 0};

