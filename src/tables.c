#include "mud.h"

const char *item_type[] = {   [ITEM_CLOTHING]    = "clothing",
                              [ITEM_ARMOR]       = "armor",
                              [ITEM_FIREARM]     = "firearm",
                              [ITEM_BLADE]       = "blade",
                              [ITEM_BLUDGEON]    = "bludgeon",
                              [ITEM_ELECTRONICS] = "electronics",
                              [ITEM_COMPUTER]    = "computer",
                              [ITEM_SCRAP]       = "scrap",
                              [ITEM_FURNITURE]   = "furniture",
                              [ITEM_LOCKPICK]    = "lockpick",
                              [ITEM_SHEATH]      = "sheath",
                              [ITEM_HOLSTER]     = "holster",
                              [ITEM_CONTAINER]   = "container",
                              [ITEM_MAGAZINE]    = "magazine",
                              [ITEM_BULLET]      = "bullet",
                              [ITEM_BOOK]        = "book",
                              [ITEM_MEMORY]      = "memory",
                              [ITEM_CHIP]        = "chip",
                              [ITEM_KEY]         = "key",
                              [ITEM_CORPSE]      = "corpse",
                              [ITEM_UNKNOWN]     = "unknown",
                              [MAX_ITEM]         = 0 };

const char *wear_pos[] = { [WEAR_HEAD]      = "head",
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

size_t body_aim_mod[]    = { [BODY_HEAD]    = 15,
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

const char *body_parts[] = { [BODY_HEAD]    = "head", 
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

const char *wound_trauma[] = { [TRAUMA_NONE]     = "none",
                               [TRAUMA_MILD]     = "leaking",
                               [TRAUMA_MODERATE] = "bleeding",
                               [TRAUMA_SEVERE]   = "pouring",
                               [TRAUMA_CRITICAL] = "spraying",
                               [MAX_TRAUMA]      = 0};

const char *blunt_trauma[] = { [TRAUMA_NONE]     = "none",
                               [TRAUMA_MILD]     = "bruised",
                               [TRAUMA_MODERATE] = "fractured",
                               [TRAUMA_SEVERE]   = "shattered",
                               [TRAUMA_CRITICAL] = "compounded fracture",
                               [MAX_TRAUMA]      = 0};

const char *burn_trauma[]  = { [TRAUMA_NONE]     = "none",
                               [TRAUMA_MILD]     = "irritated",
                               [TRAUMA_MODERATE] = "blistered",
                               [TRAUMA_SEVERE]   = "burnt",
                               [TRAUMA_CRITICAL] = "charred",
                               [MAX_TRAUMA]      = 0};

const char *reset_types[]  = { [RESET_EXIT]      = "exit",
                               [RESET_MOB]       = "mob",
                               [RESET_OBJ]       = "obj",
                               [MAX_RESET]       = 0};

const char *positions[]    = { [POS_UNCONSCIOUS] = "unconscious",
                               [POS_RESTRAINED]  = "restrained",
                               [POS_SLEEPING]    = "sleeping",
                               [POS_RESTING]     = "resting",
                               [POS_SITTING]     = "sitting",
                               [POS_PRONE]       = "prone",
                               [POS_KNEELING]    = "kneeling",
                               [POS_STANDING]    = "standing",
                               [POS_FIGHTING]    = "fighting",
                               [MAX_POS]         = 0};

const int b_to_e_table[] = { WEAR_HEAD, WEAR_EYES, WEAR_FACE, WEAR_NECK,
   WEAR_ARMS, WEAR_ARMS, WEAR_HANDS, WEAR_HANDS, WEAR_BODY, WEAR_WAIST,
   WEAR_LEGS, WEAR_LEGS, WEAR_FEET, WEAR_FEET, WEAR_NONE };

int b_to_e( int b )
{
   if( b < WEAR_HEAD || b > WEAR_FEET )
      return WEAR_NONE;

   return b_to_e_table[b];
}
