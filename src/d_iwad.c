//
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
//     Search for and locate an IWAD file, and initialize according
//     to the IWAD type.
//

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "doomkeys.h"
#include "d_iwad.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_misc.h"
#include "w_wad.h"
#include "z_zone.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif


// Array of locations to search for IWAD files
//
// "128 IWAD search directories should be enough for anybody".

#define MAX_IWAD_DIRS 128

static boolean iwad_dirs_built = false;
static const char *iwad_dirs[MAX_IWAD_DIRS];
static int num_iwad_dirs = 0;

static void AddIWADDir(const char *dir)
{
    if (num_iwad_dirs < MAX_IWAD_DIRS)
    {
        iwad_dirs[num_iwad_dirs] = dir;
        ++num_iwad_dirs;
    }
}

// Returns true if the specified path is a path to a file
// of the specified name.

static boolean DirIsFile(const char *path, const char *filename)
{
    return strchr(path, DIR_SEPARATOR) != NULL
        && !strcasecmp(M_BaseName(path), filename);
}

// Add IWAD directories parsed from splitting a path string containing
// paths separated by PATH_SEPARATOR. 'suffix' is a string to concatenate
// to the end of the paths before adding them.
static void AddIWADPath(const char *path, const char *suffix)
{
    char *left, *p, *dup_path;

    dup_path = M_StringDuplicate(path);

    // Split into individual dirs within the list.
    left = dup_path;

    for (;;)
    {
        p = strchr(left, PATH_SEPARATOR);
        if (p != NULL)
        {
            // Break at the separator and use the left hand side
            // as another iwad dir
            *p = '\0';

            AddIWADDir(M_StringJoin(left, suffix, NULL));
            left = p + 1;
        }
        else
        {
            break;
        }
    }

    AddIWADDir(M_StringJoin(left, suffix, NULL));

    free(dup_path);
}

#ifndef _WIN32
// Add standard directories where IWADs are located on Unix systems.
// To respect the freedesktop.org specification we support overriding
// using standard environment variables. See the XDG Base Directory
// Specification:
// <http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html>
static void AddXdgDirs(void)
{
    const char *env;
    char *tmp_env;

    // Quote:
    // > $XDG_DATA_HOME defines the base directory relative to which
    // > user specific data files should be stored. If $XDG_DATA_HOME
    // > is either not set or empty, a default equal to
    // > $HOME/.local/share should be used.
    env = getenv("XDG_DATA_HOME");
    tmp_env = NULL;

    if (env == NULL)
    {
        const char *homedir = getenv("HOME");
        if (homedir == NULL)
        {
            homedir = "/";
        }

        tmp_env = M_StringJoin(homedir, "/.local/share", NULL);
        env = tmp_env;
    }

    // We support $XDG_DATA_HOME/games/doom (which will usually be
    // ~/.local/share/games/doom) as a user-writeable extension to
    // the usual /usr/share/games/doom location.
    AddIWADDir(M_StringJoin(env, "/games/doom", NULL));
    free(tmp_env);

    // Quote:
    // > $XDG_DATA_DIRS defines the preference-ordered set of base
    // > directories to search for data files in addition to the
    // > $XDG_DATA_HOME base directory. The directories in $XDG_DATA_DIRS
    // > should be seperated with a colon ':'.
    // >
    // > If $XDG_DATA_DIRS is either not set or empty, a value equal to
    // > /usr/local/share/:/usr/share/ should be used.
    env = getenv("XDG_DATA_DIRS");
    if (env == NULL)
    {
        // (Trailing / omitted from paths, as it is added below)
        env = "/usr/local/share:/usr/share";
    }

    // The "standard" location for IWADs on Unix that is supported by most
    // source ports is /usr/share/games/doom - we support this through the
    // XDG_DATA_DIRS mechanism, through which it can be overridden.
    AddIWADPath(env, "/games/doom");
    AddIWADPath(env, "/doom");

    // The convention set by RBDOOM-3-BFG is to install Doom 3: BFG
    // Edition into this directory, under which includes the Doom
    // Classic WADs.
    AddIWADPath(env, "/games/doom3bfg/base/wads");
}

#endif // !_WIN32

//
// Build a list of IWAD files
//

static void BuildIWADDirList(void)
{
    char *env;

    if (iwad_dirs_built)
    {
        return;
    }

    // Look in the current directory.  Doom always does this.
    AddIWADDir(".");

    // Next check the directory where the executable is located. This might
    // be different from the current directory.
    AddIWADDir(M_DirName(myargv[0]));

    // Add DOOMWADDIR if it is in the environment
    env = M_getenv("DOOMWADDIR");
    if (env != NULL)
    {
        AddIWADDir(env);
    }

    // Add dirs from DOOMWADPATH:
    env = M_getenv("DOOMWADPATH");
    if (env != NULL)
    {
        AddIWADPath(env, "");
    }

#ifdef _WIN32


#else
    AddXdgDirs();
#endif

    // Don't run this function again.

    iwad_dirs_built = true;
}

//
// Searches WAD search paths for an WAD with a specific filename.
// 

char *D_FindWADByName(const char *name)
{
    char *path;
    char *probe;
    int i;
    
    // Absolute path?

    probe = M_FileCaseExists(name);
    if (probe != NULL)
    {
        return probe;
    }

    BuildIWADDirList();

    // Search through all IWAD paths for a file with the given name.

    for (i=0; i<num_iwad_dirs; ++i)
    {
        // As a special case, if this is in DOOMWADDIR or DOOMWADPATH,
        // the "directory" may actually refer directly to an IWAD
        // file.

        probe = M_FileCaseExists(iwad_dirs[i]);
        if (DirIsFile(iwad_dirs[i], name) && probe != NULL)
        {
            return probe;
        }
        free(probe);

        // Construct a string for the full path

        path = M_StringJoin(iwad_dirs[i], DIR_SEPARATOR_S, name, NULL);

        probe = M_FileCaseExists(path);
        if (probe != NULL)
        {
            return probe;
        }

        free(path);
    }

    // File not found

    return NULL;
}

//
// D_TryWADByName
//
// Searches for a WAD by its filename, or returns a copy of the filename
// if not found.
//

char *D_TryFindWADByName(const char *filename)
{
    char *result;

    result = D_FindWADByName(filename);

    if (result != NULL)
    {
        return result;
    }
    else
    {
        return M_StringDuplicate(filename);
    }
}

//
// FindIWAD
// Checks availability of IWAD files by name,
// to determine whether registered/commercial features
// should be executed (notably loading PWADs).
//

char *D_FindIWAD(int mask, GameMission_t *mission)
{
    char *result = D_FindWADByName("yaguar-doom.wad");;

    return result;
}
