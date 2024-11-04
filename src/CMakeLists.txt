foreach(SUBDIR doom)
    add_subdirectory("${SUBDIR}")
endforeach()

# Common source files used by absolutely everything:

set(COMMON_SOURCE_FILES
    i_main.c
    i_system.c           i_system.h
    id_vars.c            id_vars.h
    m_argv.c             m_argv.h
    m_misc.c             m_misc.h)

# Source files used by the game binaries (chocolate-doom, etc.)

set(GAME_SOURCE_FILES
    d_event.c           d_event.h
                        doomkeys.h
                        doomtype.h
    d_iwad.c            d_iwad.h
    d_loop.c            d_loop.h
    d_mode.c            d_mode.h
                        d_ticcmd.h
    i_glob.c            i_glob.h
    i_input.c           i_input.h
    i_joystick.c        i_joystick.h
                        i_swap.h
    i_oplmusic.c
    i_sdlmusic.c
    i_sdlsound.c
    i_sound.c           i_sound.h
    i_timer.c           i_timer.h
    i_video.c           i_video.h
    midifile.c          midifile.h
    mus2mid.c           mus2mid.h
                        m_array.h
    m_bbox.c            m_bbox.h
    m_config.c          m_config.h
    m_controls.c        m_controls.h
    m_fixed.c           m_fixed.h
    memio.c             memio.h
    tables.c            tables.h
    v_video.c           v_video.h
                        v_patch.h
    v_trans.c           v_trans.h
    w_main.c            w_main.h
    w_wad.c             w_wad.h
    w_file.c            w_file.h
    w_file_stdc.c
    w_merge.c           w_merge.h
    z_zone.c            z_zone.h)

# Platform-dependent source files:

set(GAME_INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/../")

if(WIN32)
    list(APPEND GAME_SOURCE_FILES i_winmusic.c)
    list(APPEND GAME_SOURCE_FILES midifallback.c    midifallback.h)
    list(APPEND GAME_SOURCE_FILES w_file_win32.c)
    list(APPEND GAME_SOURCE_FILES
        "../win32/win_opendir.c" "../win32/win_opendir.h")
    list(APPEND GAME_INCLUDE_DIRS
         "${PROJECT_SOURCE_DIR}/win32/")
elseif(POSIX)
    list(APPEND GAME_SOURCE_FILES w_file_posix.c)
endif()


set(SOURCE_FILES ${COMMON_SOURCE_FILES} ${GAME_SOURCE_FILES})

set(EXTRA_LIBS SDL2::SDL2main SDL2::SDL2 miniz::miniz opl)
if(ENABLE_SDL2_MIXER)
    list(APPEND EXTRA_LIBS SDL2_mixer::SDL2_mixer)
endif()
if(ENABLE_SDL2_NET)
    list(APPEND EXTRA_LIBS SDL2_net::SDL2_net)
endif()
if(SampleRate_FOUND)
    list(APPEND EXTRA_LIBS SampleRate::samplerate)
endif()
#if(FluidSynth_FOUND)
#    list(APPEND EXTRA_LIBS FluidSynth::libfluidsynth)
#endif()
if(WIN32)
	list(APPEND EXTRA_LIBS winmm shlwapi)
endif()

if(WIN32)
    add_executable("${PROGRAM_PREFIX}" WIN32 ${SOURCE_FILES} "${CMAKE_CURRENT_BINARY_DIR}/doom-res.rc")
else()
    add_executable("${PROGRAM_PREFIX}" ${SOURCE_FILES})
endif()

target_include_directories("${PROGRAM_PREFIX}" PRIVATE ${GAME_INCLUDE_DIRS})
target_link_libraries("${PROGRAM_PREFIX}" doom ${EXTRA_LIBS} m)

if(MSVC)
    set_target_properties("${PROGRAM_PREFIX}" PROPERTIES
                          LINK_FLAGS "/MANIFEST:NO")
endif()

set_target_properties("${PROGRAM_PREFIX}" PROPERTIES
                      INTERPROCEDURAL_OPTIMIZATION ${ENABLE_LTO})
