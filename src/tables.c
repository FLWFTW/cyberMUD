#include "mud.h"

const char *item_type[] = { "clothing", "armor", "blade", "club", "pistol", "smg",
                            "rifle", "shotgun", "electronics", "computer", "scrap",
                            "furniture", "lockpicks", "sheath", "holster", "container",
                            "magazine", "bullet",
                            "unknown", 0 };

const char *wear_pos[] = { "head", "eyes", "face", "neck", "body", "shoulders", "arms", 
                           "hands", "waist", "legs", "feet", "slung", "none",
                           0 };

const char *body_parts[] = { "head", "eyes", "face", "neck", "left arm", "right arm",
                             "left hand", "right hand", "torso", "groin", "left leg",
                             "right leg", "left foot", "right foot",
                             0 };

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

const int b_to_e_table[] = { WEAR_HEAD, WEAR_EYES, WEAR_FACE, WEAR_NECK,
   WEAR_ARMS, WEAR_ARMS, WEAR_HANDS, WEAR_HANDS, WEAR_BODY, WEAR_WAIST,
   WEAR_LEGS, WEAR_LEGS, WEAR_FEET, WEAR_FEET, WEAR_NONE };

int b_to_e( int b )
{
   if( b < WEAR_HEAD || b > WEAR_FEET )
      return WEAR_NONE;

   return b_to_e_table[b];
}
