//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2024 Julia Nechaevskaya
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

#pragma once


//
//	M_Menu.C
//
#define PRESSKEY 	"press a key."
#define PRESSYN 	"press y or n."
#define QUITMSG	"are you sure you want to\nquit this great game?"
#define QSAVESPOT	"you haven't picked a quicksave slot yet!\n\n"PRESSKEY
#define SAVEDEAD 	"you can't save if you aren't playing!\n\n"PRESSKEY
#define QSPROMPT 	"quicksave over your game named\n\n'%s'?\n\n"PRESSYN
#define QLPROMPT	"do you want to quickload the game named\n\n'%s'?\n\n"PRESSYN

#define MSGOFF	"Messages OFF"
#define MSGON		"Messages ON"
#define ENDGAME	"are you sure you want to end the game?\n\n"PRESSYN

#define DOSY		"(press y to quit.)"

#define DETAILHI	"High detail"
#define DETAILLO	"Low detail"
#define DETAILNA	"Not available in 1x resolution"

#define GAMMALVL05  "GAMMA CORRECTION LEVEL 0.50"
#define GAMMALVL055 "GAMMA CORRECTION LEVEL 0.55"
#define GAMMALVL06  "GAMMA CORRECTION LEVEL 0.60"
#define GAMMALVL065 "GAMMA CORRECTION LEVEL 0.65"
#define GAMMALVL07  "GAMMA CORRECTION LEVEL 0.70"
#define GAMMALVL075 "GAMMA CORRECTION LEVEL 0.75"
#define GAMMALVL08  "GAMMA CORRECTION LEVEL 0.80"
#define GAMMALVL085 "GAMMA CORRECTION LEVEL 0.85"
#define GAMMALVL09  "GAMMA CORRECTION LEVEL 0.90"
#define GAMMALVL095 "GAMMA CORRECTION LEVEL 0.95"
#define GAMMALVL0   "GAMMA CORRECTION OFF"
#define GAMMALVL1   "GAMMA CORRECTION LEVEL 1"
#define GAMMALVL2   "GAMMA CORRECTION LEVEL 2"
#define GAMMALVL3   "GAMMA CORRECTION LEVEL 3"
#define GAMMALVL4   "GAMMA CORRECTION LEVEL 4"

#define EMPTYSTRING	"empty slot"

//
//	P_inter.C
//
#define GOTARMOR	"You pick up the armor."
#define GOTMEGA	"You got the MegaArmor!"
#define GOTHTHBONUS	"You pick up a health bonus."
#define GOTARMBONUS	"You pick up an armor bonus."
#define GOTSTIM	"You pick up a stimpack."
#define GOTMEDINEED	"You pick up a medikit that you REALLY need!"
#define GOTMEDIKIT	"You pick up a medikit."
#define GOTSUPER	"Supercharge!"

#define GOTBLUECARD	"You pick up a blue keycard."
#define GOTYELWCARD	"You pick up a yellow keycard."
#define GOTREDCARD	"You pick up a red keycard."
#define GOTBLUESKUL	"You pick up a blue skull key."
#define GOTYELWSKUL	"You pick up a yellow skull key."
#define GOTREDSKULL	"You pick up a red skull key."

#define GOTINVUL	"Invulnerability!"
#define GOTBERSERK	"Berserk!"
#define GOTINVIS	"Partial Invisibility"
#define GOTSUIT	"Radiation Shielding Suit"
#define GOTMAP	"Computer Area Map"
#define GOTVISOR	"Light Amplification Visor"

#define GOTCLIP	"You pick up a clip."
#define GOTCLIPBOX	"You pick up a box of bullets."
#define GOTROCKET	"You pick up a rocket."
#define GOTROCKBOX	"You pick up a box of rockets."
#define GOTCELL	"You pick up an energy cell."
#define GOTCELLBOX	"You pick up an energy cell pack."
#define GOTSHELLS	"You pick up 4 shotgun shells."
#define GOTSHELLBOX	"You pick up a box of shotgun shells."
#define GOTBACKPACK	"You pick up a backpack full of ammo!"

#define GOTBFG9000	"You got the BFG9000!  Oh, yes."
#define GOTCHAINGUN	"You got the chaingun!"
#define GOTCHAINSAW	"A chainsaw!  Find some meat!"
#define GOTLAUNCHER	"You got the rocket launcher!"
#define GOTPLASMA	"You got the plasma gun!"
#define GOTSHOTGUN	"You got the shotgun!"

//
// P_Doors.C
//
#define PD_BLUEO	"You need a blue key to activate this object"
#define PD_REDO	"You need a red key to activate this object"
#define PD_YELLOWO	"You need a yellow key to activate this object"
#define PD_BLUEK	"You need a blue key to open this door"
#define PD_REDK	"You need a red key to open this door"
#define PD_YELLOWK	"You need a yellow key to open this door"

//
//	G_game.C
//
#define GGSAVED	"game saved."

//
//	AM_map.C
//

#define AMSTR_FOLLOWON	"Follow Mode ON"
#define AMSTR_FOLLOWOFF	"Follow Mode OFF"

#define AMSTR_GRIDON	"Grid ON"
#define AMSTR_GRIDOFF	"Grid OFF"

#define AMSTR_MARKEDSPOT	"Marked Spot"
#define AMSTR_MARKCLEARED	"Cleared Spot"
#define AMSTR_MARKSCLEARED	"All Marks Cleared"

//
//	ST_stuff.C
//

#define STSTR_MUS		"Music Change"
#define STSTR_NOMUS		"IMPOSSIBLE SELECTION"
#define STSTR_DQDON		"Degreelessness Mode On"
#define STSTR_DQDOFF	"Degreelessness Mode Off"

#define STSTR_KFAADDED	"Very Happy Ammo Added"
#define STSTR_FAADDED	"Ammo (no keys) Added"
#define STSTR_KAADDED	"KEYS ADDED"

#define STSTR_NCON		"No Clipping Mode ON"
#define STSTR_NCOFF		"No Clipping Mode OFF"

#define STSTR_BEHOLD	"inVuln, Str, Inviso, Rad, Allmap, or Lite-amp"
#define STSTR_BEHOLDX	"Power-up Toggled"

#define STSTR_CHOPPERS	"... doesn't suck - GM"
#define STSTR_CLEV		"Changing Level..."

//
// F_Finale.C
//

#define CC_ZOMBIE   "zombieman"
#define CC_SHOTGUN  "shotgun guy"
#define CC_IMP      "imp"
#define CC_DEMON    "demon"
#define CC_SPECTRE  "spectre"
#define CC_LOST     "lost soul"
#define CC_CACO     "cacodemon"
#define CC_BARON    "baron of hell"
#define CC_HERO     "our hero"

//
// [JN] ID-specific strings.
//

#define ID_RESET \
"GRAPHICAL, AUDIO AND GAMEPLAY SETTINGS\n" \
"WILL BE RESET TO DEFAULT VALUES.\n\n" \
"ARE YOU SURE WANT TO CONTINUE?\n\n" \
PRESSYN

#define ID_EXTHUD_ON       "EXTENDED HUD ON"
#define ID_EXTHUD_OFF      "EXTENDED HUD OFF"

#define ID_SPECTATOR_ON    "SPECTATOR MODE ON"
#define ID_SPECTATOR_OFF   "SPECTATOR MODE OFF"

#define ID_FREEZE_ON       "FREEZE MODE ON"
#define ID_FREEZE_OFF      "FREEZE MODE OFF"

#define ID_NOTARGET_ON     "NOTARGET MODE ON"
#define ID_NOTARGET_OFF    "NOTARGET MODE OFF"

#define ID_BUDDHA_ON        "BUDDHA MODE ON"
#define ID_BUDDHA_OFF       "BUDDHA MODE OFF"

#define ID_AUTOMAPROTATE_ON     "ROTATE MODE ON"
#define ID_AUTOMAPROTATE_OFF    "ROTATE MODE OFF"
#define ID_AUTOMAPOVERLAY_ON    "OVERLAY MODE ON"
#define ID_AUTOMAPOVERLAY_OFF   "OVERLAY MODE OFF"

#define ID_AUTORUN_ON      "ALWAYS RUN ON"
#define ID_AUTORUN_OFF     "ALWAYS RUN OFF"

#define ID_MLOOK_ON        "MOUSE LOOK ON"
#define ID_MLOOK_OFF       "MOUSE LOOK OFF"

#define ID_NOVERT_ON       "VERTICAL MOUSE MOVEMENT OFF"
#define ID_NOVERT_OFF      "VERTICAL MOUSE MOVEMENT ON"

#define ID_SECRET_FOUND    "YOU FOUND A SECRET!"
