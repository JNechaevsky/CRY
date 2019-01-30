//
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2014 Fabian Greffrath
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


#include <stdio.h>
#include <string.h>
#include "deh_defs.h"
#include "deh_io.h"
#include "deh_main.h"
#include "d_englsh.h"


typedef struct {
    char *macro;
    char *string;
} bex_string_t;

// mnemonic keys table
static const bex_string_t bex_stringtable[] = {
    // [Julia] Nothing here, the project is not Dehacked compatible.
};

static void *DEH_BEXStrStart(deh_context_t *context, char *line)
{
    char s[10];

    if (sscanf(line, "%9s", s) == 0 || strncmp("[STRINGS]", s, sizeof(s)))
    {
	DEH_Warning(context, "Parse error on section start");
    }

    return NULL;
}

static void DEH_BEXStrParseLine(deh_context_t *context, char *line, void *tag)
{
    char *variable_name, *value;
    int i;

    if (!DEH_ParseAssignment(line, &variable_name, &value))
    {
	DEH_Warning(context, "Failed to parse assignment");
	return;
    }

    for (i = 0; i < arrlen(bex_stringtable); i++)
    {
	if (!strcmp(bex_stringtable[i].macro, variable_name))
	{
	    DEH_AddStringReplacement(bex_stringtable[i].string, value);
	}
    }
}

deh_section_t deh_section_bexstr =
{
    "[STRINGS]",
    NULL,
    DEH_BEXStrStart,
    DEH_BEXStrParseLine,
    NULL,
    NULL,
};
