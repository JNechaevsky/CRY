add_library(doom STATIC
            am_map.c        am_map.h
            ct_chat.c       ct_chat.h
                            d_englsh.h
            d_main.c        d_main.h
            d_net.c
            doomdef.c       doomdef.h
            doomstat.c      doomstat.h
                            d_player.h
                            d_think.h
            f_finale.c      f_finale.h
            f_wipe.c        f_wipe.h
            g_game.c        g_game.h
            info.c          info.h
            id_func.c       id_func.h
            m_menu.c        m_menu.h
            m_random.c      m_random.h
            p_ceilng.c
            p_doors.c
            p_enemy.c
            p_floor.c
            p_inter.c
            p_lights.c
                            p_local.h
            p_map.c
            p_maputl.c
            p_mobj.c
            p_plats.c
            p_pspr.c
            p_saveg.c
            p_setup.c
            p_sight.c
            p_spec.c
            p_switch.c
            p_telept.c
            p_tick.c
            p_user.c
            r_bmaps.c
            r_bsp.c
            r_collit.c      r_collit.h
            r_data.c
            r_draw.c
                            r_local.h
            r_main.c
            r_plane.c
            r_segs.c
            r_swirl.c
            r_things.c
            s_sound.c       s_sound.h
            sounds.c        sounds.h
            st_bar.c        st_bar.h
            wi_stuff.c      wi_stuff.h)

target_include_directories(doom PRIVATE "../" "${CMAKE_CURRENT_BINARY_DIR}/../../")
target_link_libraries(doom SDL2::SDL2 miniz::miniz)
if(ENABLE_SDL2_MIXER)
    target_link_libraries(doom SDL2_mixer::SDL2_mixer)
endif()
if(ENABLE_SDL2_NET)
    target_link_libraries(doom SDL2_net::SDL2_net)
endif()
