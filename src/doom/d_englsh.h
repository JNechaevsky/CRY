//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2018-2019 Julia Nechaevskaya
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//


#ifndef __D_ENGLSH__
#define __D_ENGLSH__


// -----------------------------------------------------------------------------
// D_Main.C strings
// -----------------------------------------------------------------------------

#define D_DEVSTR    "Development mode ON.\n"
#define D_CDROM     "CD-ROM Version: default.cfg from c:\\doomdata\n"

// -----------------------------------------------------------------------------
// M_Menu.C
// -----------------------------------------------------------------------------

#define PRESSKEY    "press a key."
#define PRESSYN     "(press \"y\" or \"n\")"
#define QUITMSG     "are you sure you want to quit?"
#define QSAVESPOT   "you haven't picked a quicksave slot yet!\n\n"PRESSKEY
#define SAVEDEAD    "you can't save if you aren't playing!\n\n"PRESSKEY

#define SAVEGAMENAME "doomsav"

#define MSGON       "Messages ON"
#define MSGOFF      "Messages OFF"

#define ENDGAME     "are you sure you want to end the game?\n\n"PRESSYN

#define DETAILHI    "High detail"
#define DETAILLO    "Low detail"

#define GAMMA_OFF   "Gamma correction OFF"
#define GAMMA_05    "Gamma correction level 0.5"
#define GAMMA_1     "Gamma correction level 1.0"
#define GAMMA_15    "Gamma correction level 1.5"
#define GAMMA_2     "Gamma correction level 2.0"
#define GAMMA_25    "Gamma correction level 2.5"
#define GAMMA_3     "Gamma correction level 3.0"
#define GAMMA_35    "Gamma correction level 3.5"
#define GAMMA_4     "Gamma correction level 4.0"

#define EMPTYSTRING	"empty slot"

// -----------------------------------------------------------------------------
// Health and Armor
// -----------------------------------------------------------------------------

#define GOTHTHBONUS "Picked up a health bonus."
#define GOTARMBONUS "Picked up an armor bonus."
#define GOTSTIM     "Picked up a stimpack."
#define GOTMEDIKIT  "Picked up a medikit."
#define GOTMEDINEED "Picked up a medikit that you REALLY need!"
#define GOTSUPER    "Supercharge!"
#define GOTARMOR    "Picked up the armor."
#define GOTMEGA     "Picked up the MegaArmor!"


// -----------------------------------------------------------------------------
// Weapons
// -----------------------------------------------------------------------------

#define GOTCHAINSAW	"A chainsaw! Find some meat!"
#define GOTSHOTGUN	"You got the shotgun!"
#define GOTCHAINGUN	"You got the chaingun!"
#define GOTLAUNCHER	"You got the rocket launcher!"
#define GOTPLASMA	"You got the plasma gun!"
#define GOTBFG9000	"You got the BFG9000!  Oh, yes."


// -----------------------------------------------------------------------------
// Ammo
// -----------------------------------------------------------------------------

#define GOTCLIP     "Picked up a clip."
#define GOTCLIPBOX  "Picked up a box of bullets."
#define GOTROCKET   "Picked up a rocket."
#define GOTROCKET2  "Picked up a two rockets."
#define GOTROCKBOX  "Picked up a box of rockets."
#define GOTCELL     "Picked up an energy cell."
#define GOTCELLBOX  "Picked up an energy cell pack."
#define GOTSHELLS   "Picked up 4 shotgun shells."
#define GOTSHELLS8  "Picked up 8 shotgun shells."
#define GOTSHELLBOX "Picked up a box of shotgun shells."
#define GOTBACKPACK "Picked up a backpack full of ammo!"


// -----------------------------------------------------------------------------
// Powerups
// -----------------------------------------------------------------------------

#define GOTINVUL    "Invulnerability!"
#define GOTBERSERK  "Berserk!"
#define GOTSUIT     "Radiation Shielding Suit"
#define GOTMAP      "Computer Area Map"


// -----------------------------------------------------------------------------
// Keys and Skulls
// -----------------------------------------------------------------------------

#define GOTBLUECARD "Picked up a blue keycard."
#define GOTYELWCARD "Picked up a yellow keycard."
#define GOTREDCARD  "Picked up a red keycard."
#define GOTBLUESKUL "Picked up a blue skull key."
#define GOTYELWSKUL "Picked up a yellow skull key."
#define GOTREDSKULL "Picked up a red skull key."


// -----------------------------------------------------------------------------
// Doors
// -----------------------------------------------------------------------------

#define PD_BLUEC    "You need a blue keycard to open this door"
#define PD_REDC     "You need a red keycard to open this door"
#define PD_YELLOWC  "You need a yellow keycard to open this door"
#define PD_BLUES    "You need a blue skull key to open this door"
#define PD_REDS     "You need a red skull key to open this door"
#define PD_YELLOWS  "You need a yellow skull key to open this door" 


// -----------------------------------------------------------------------------
// HUD messages
// -----------------------------------------------------------------------------

#define GGSAVED     "game saved."
#define GGLOADED    "game loaded."
#define HUSTR_MSGU	"[Message unsent]"


// -----------------------------------------------------------------------------
// DOOM for Atari Jaguar map names
// -----------------------------------------------------------------------------

#define JHUSTR_1    "level 1: Hangar"
#define JHUSTR_2    "level 2: Plant"
#define JHUSTR_3    "level 3: Toxin Refinery"
#define JHUSTR_4    "level 4: Command Control"
#define JHUSTR_5    "level 5: Phobos Lab"
#define JHUSTR_6    "level 6: Central Processing"
#define JHUSTR_7    "level 7: Computer Station"
#define JHUSTR_8    "level 8: Phobos Anomaly"
#define JHUSTR_9    "level 9: Deimos Anomaly"
#define JHUSTR_10   "level 10: Containment Area"
#define JHUSTR_11   "level 11: Refinery"
#define JHUSTR_12   "level 12: Deimos Lab"
#define JHUSTR_13   "level 13: Command Center"
#define JHUSTR_14   "level 14: Halls of the Damned"
#define JHUSTR_15   "level 15: Spawning Vats"
#define JHUSTR_16   "level 16: Tower of Babel"
#define JHUSTR_17   "level 17: Hell Keep"
#define JHUSTR_18   "level 18: Pandemonium"
#define JHUSTR_19   "level 19: House of Pain"
#define JHUSTR_20   "level 20: Unholy Cathedral"
#define JHUSTR_21   "level 21: Mt. Erebus"
#define JHUSTR_22   "level 22: Limbo"
#define JHUSTR_23   "level 23: Dis"
#define JHUSTR_24   "level 24: Military Base"
#define JHUSTR_25   "level 25: Extra Map"

// -----------------------------------------------------------------------------
// Short map names for intermission screen
// -----------------------------------------------------------------------------

#define JAGLVL_01   "Hangar"
#define JAGLVL_02   "Plant"
#define JAGLVL_03   "Toxin Refinery"
#define JAGLVL_04   "Command Control"
#define JAGLVL_05   "Phobos Lab"
#define JAGLVL_06   "Central Processing"
#define JAGLVL_07   "Computer Station"
#define JAGLVL_08   "Phobos Anomaly"
#define JAGLVL_09   "Deimos Anomaly"
#define JAGLVL_10   "Containment Area"
#define JAGLVL_11   "Refinery"
#define JAGLVL_12   "Deimos Lab"
#define JAGLVL_13   "Command Center"
#define JAGLVL_14   "Halls of the Damned"
#define JAGLVL_15   "Spawning Vats"
#define JAGLVL_16   "Tower of Babel"
#define JAGLVL_17   "Hell Keep"
#define JAGLVL_18   "Pandemonium"
#define JAGLVL_19   "House of Pain"
#define JAGLVL_20   "Unholy Cathedral"
#define JAGLVL_21   "Mt. Erebus"
#define JAGLVL_22   "Limbo"
#define JAGLVL_23   "Dis"
#define JAGLVL_24   "Military Base"
#define JAGLVL_25   "Extra Map"


// -----------------------------------------------------------------------------
// Player names
// -----------------------------------------------------------------------------

#define HUSTR_PLRGREEN      "Green: "
#define HUSTR_PLRINDIGO     "Indigo: "
#define HUSTR_PLRBROWN      "Brown: "
#define HUSTR_PLRRED        "Red: "

// -----------------------------------------------------------------------------
// Automap
// -----------------------------------------------------------------------------

#define AMSTR_FOLLOWON      "Follow Mode ON"
#define AMSTR_FOLLOWOFF     "Follow Mode OFF"
#define AMSTR_GRIDON        "Grid ON"
#define AMSTR_GRIDOFF       "Grid OFF"
#define AMSTR_MARKEDSPOT    "Marked Spot"
#define AMSTR_MARKSCLEARED  "All Marks Cleared"

// -----------------------------------------------------------------------------
// Status bar
// -----------------------------------------------------------------------------

#define STSTR_MUS           "Music Change"
#define STSTR_NOMUS         "IMPOSSIBLE SELECTION"
#define STSTR_DQDON         "Degreelessness Mode On"
#define STSTR_DQDOFF        "Degreelessness Mode Off"

#define STSTR_KFAADDED      "Very Happy Ammo Added"
#define STSTR_FAADDED       "Ammo (no keys) Added"
#define STSTR_KAADDED       "Keys Added"

#define STSTR_NCON          "No Clipping Mode ON"
#define STSTR_NCOFF         "No Clipping Mode OFF"

#define STSTR_CHOPPERS      "... doesn't suck - GM"
#define STSTR_CLEV          "Changing Level..."

#define STSTR_BEHOLD        "inVuln, Str, Rad or Allmap"
#define STSTR_BEHOLDX       "Power-up Activated"
#define STSTR_BEHOLDZ       "Power-up Deactivated"


// -----------------------------------------------------------------------------
// Final text
// -----------------------------------------------------------------------------

#define JAGENDING               \
    "     id software\n"        \
    "     salutes you!\n"       \
    "\n"                        \
    "  the horrors of hell\n"   \
    "  could not kill you.\n"   \
    "  their most cunning\n"    \
    "  traps were no match\n"   \
    "  for you. you have\n"     \
    "  proven yourself the\n"   \
    "  best of all!\n"          \
    "\n"                        \
    "  congratulations!"
    
    
// -----------------------------------------------------------------------------
// Casting screen
// -----------------------------------------------------------------------------

#define CC_ZOMBIE   "zombieman"
#define CC_SHOTGUN  "shotgun guy"
#define CC_IMP      "imp"
#define CC_DEMON    "demon"
#define CC_LOST     "lost soul"
#define CC_CACO     "cacodemon"
#define CC_BARON    "baron of hell"
#define CC_HERO     "our hero"


// -----------------------------------------------------------------------------
// Port-specific strings
// -----------------------------------------------------------------------------


#define STSTR_TESTCTRLS "Press escape to quit."
#define STSTR_MASSACRE  "All monsters killed"

#define STSTR_SCRNSHT   "Screen shot"

#define STSTR_ALWAYSRUN "Always run "
#define STSTR_ALWRUNON  "ON"
#define STSTR_ALWRUNOFF "OFF"

#define STSRT_MOUSELOOK "Mouse look "
#define STSTR_MLOOK_ON  "ON"
#define STSTR_MLOOK_OFF "OFF"

#define STSRT_CROSSHAIR     "Crosshair "
#define STSTR_CROSSHAIR_ON  "ON"
#define STSTR_CROSSHAIR_OFF "OFF"


#endif
