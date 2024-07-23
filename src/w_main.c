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
// DESCRIPTION:
//     Common code to parse command line, identifying WAD files to load.
//

#include <stdlib.h>

#include "config.h"
#include "d_iwad.h"
#include "i_glob.h"
#include "i_system.h"
#include "m_argv.h"
#include "w_main.h"
#include "w_merge.h"
#include "w_wad.h"
#include "z_zone.h"

// Parse the command line, merging WAD files that are sppecified.
// Returns true if at least one file was added.
boolean W_ParseCommandLine(void)
{
    boolean modifiedgame = false;
    int p;

    // Merged PWADs are loaded first, because they are supposed to be 
    // modified IWADs.

    //!
    // @arg <files>
    // @category mod
    //
    // Simulates the behavior of deutex's -merge option, merging a PWAD
    // into the main IWAD.  Multiple files may be specified.
    //

    p = M_CheckParmWithArgs("-merge", 1);

    if (p > 0)
    {
        for (p = p + 1; p<myargc && myargv[p][0] != '-'; ++p)
        {
            char *filename;

            modifiedgame = true;

            filename = D_TryFindWADByName(myargv[p]);

            printf("  merging %s\n", filename);
            W_MergeFile(filename);
            free(filename);
        }
    }

    //!
    // @arg <files>
    // @vanilla
    //
    // Load the specified PWAD files.
    //

    p = M_CheckParmWithArgs ("-file", 1);
    if (p)
    {
	// the parms after p are wadfile/lump names,
	// until end of parms or another - preceded parm
	modifiedgame = true;            // homebrew levels
	while (++p != myargc && myargv[p][0] != '-')
        {
            char *filename;

            filename = D_TryFindWADByName(myargv[p]);

            // [crispy] always merge arguments of "-file" parameter
            printf("  merging %s\n", filename);
	    W_MergeFile(filename);
            free(filename);
        }
    }

    return modifiedgame;
}

// Load all WAD files from the given directory.
void W_AutoLoadWADs(const char *path)
{
    glob_t *glob;
    const char *filename;

    glob = I_StartMultiGlob(path, GLOB_FLAG_NOCASE|GLOB_FLAG_SORTED,
                            "*.wad", "*.lmp", NULL);
    for (;;)
    {
        filename = I_NextGlob(glob);
        if (filename == NULL)
        {
            break;
        }
        printf("  [autoload]  merging %s\n", filename);
        W_MergeFile(filename);
    }

    I_EndGlob(glob);
}
