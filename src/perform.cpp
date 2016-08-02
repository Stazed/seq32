//----------------------------------------------------------------------------
//
//  This file is part of seq32.
//
//  seq32 is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  seq32 is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with seq32; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//-----------------------------------------------------------------------------

#include "perform.h"
#include "midibus.h"
#include "event.h"
#include <stdio.h>
#ifndef __WIN32__
#  include <time.h>
#endif
#include <sched.h>

//For keys
#include <gtkmm/accelkey.h>


using namespace Gtk;

perform::perform()
{
    for (int i=0; i< c_max_sequence; i++)
    {
        m_seqs[i] = NULL;
        m_seqs_active[i] = false;

        m_was_active_main[i] = false;
        m_was_active_edit[i] = false;
        m_was_active_perf[i] = false;
        m_was_active_names[i] = false;
    }

    m_mute_group_selected = 0;
    m_mode_group = true;
    m_mode_group_learn = false;
    m_looping = false;
    m_reposition = false;
    m_inputing = true;
    m_outputing = true;
    m_tick = 0;
    m_midiclockrunning = false;
    m_usemidiclock = false;
    m_midiclocktick = 0;
    m_midiclockpos = -1;

    thread_trigger_width_ms = c_thread_trigger_width_ms;

    m_left_tick = 0;
    m_right_tick = c_ppqn * 16;
    m_starting_tick = 0;

    midi_control zero = {false,false,0,0,0};

    for ( int i=0; i<c_midi_controls; i++ )
    {
        m_midi_cc_toggle[i] = zero;
        m_midi_cc_on[i] = zero;
        m_midi_cc_off[i] = zero;
    }

    m_show_ui_sequence_key = true;

    set_key_event( GDK_1, 0 );
    set_key_event( GDK_q, 1 );
    set_key_event( GDK_a, 2 );
    set_key_event( GDK_z, 3 );
    set_key_event( GDK_2, 4 );
    set_key_event( GDK_w, 5 );
    set_key_event( GDK_s, 6 );
    set_key_event( GDK_x, 7 );
    set_key_event( GDK_3, 8 );
    set_key_event( GDK_e, 9 );
    set_key_event( GDK_d, 10 );
    set_key_event( GDK_c, 11 );
    set_key_event( GDK_4, 12 );
    set_key_event( GDK_r, 13 );
    set_key_event( GDK_f, 14 );
    set_key_event( GDK_v, 15 );
    set_key_event( GDK_5, 16 );
    set_key_event( GDK_t, 17 );
    set_key_event( GDK_g, 18 );
    set_key_event( GDK_b, 19 );
    set_key_event( GDK_6, 20 );
    set_key_event( GDK_y, 21 );
    set_key_event( GDK_h, 22 );
    set_key_event( GDK_n, 23 );
    set_key_event( GDK_7, 24 );
    set_key_event( GDK_u, 25 );
    set_key_event( GDK_j, 26 );
    set_key_event( GDK_m, 27 );
    set_key_event( GDK_8, 28 );
    set_key_event( GDK_i, 29 );
    set_key_event( GDK_k, 30 );
    set_key_event( GDK_comma, 31 );

    set_key_group( GDK_exclam,  0 );
    set_key_group( GDK_quotedbl,  1  );
    set_key_group( GDK_numbersign,  2  );
    set_key_group( GDK_dollar,  3  );
    set_key_group( GDK_percent,  4  );
    set_key_group( GDK_ampersand,  5  );
    set_key_group( GDK_parenleft,  7  );
    set_key_group( GDK_slash,  6  );
    set_key_group( GDK_semicolon,  31 );
    set_key_group( GDK_A,  16 );
    set_key_group( GDK_B,  28 );
    set_key_group( GDK_C,  26 );
    set_key_group( GDK_D,  18 );
    set_key_group( GDK_E,  10 );
    set_key_group( GDK_F,  19 );
    set_key_group( GDK_G,  20 );
    set_key_group( GDK_H,  21 );
    set_key_group( GDK_I,  15 );
    set_key_group( GDK_J,  22 );
    set_key_group( GDK_K,  23 );
    set_key_group( GDK_M,  30 );
    set_key_group( GDK_N,  29 );
    set_key_group( GDK_Q,  8  );
    set_key_group( GDK_R,  11 );
    set_key_group( GDK_S,  17 );
    set_key_group( GDK_T,  12 );
    set_key_group( GDK_U,  14 );
    set_key_group( GDK_V,  27 );
    set_key_group( GDK_W,  9  );
    set_key_group( GDK_X,  25 );
    set_key_group( GDK_Y,  13 );
    set_key_group( GDK_Z,  24 );

    m_key_bpm_up = GDK_apostrophe;
    m_key_bpm_dn = GDK_semicolon;

    m_key_replace = GDK_Control_L;
    m_key_queue = GDK_Control_R;
    m_key_snapshot_1 = GDK_Alt_L;
    m_key_snapshot_2 = GDK_Alt_R;
    m_key_keep_queue = GDK_backslash;

    m_key_screenset_up = GDK_bracketright;
    m_key_screenset_dn = GDK_bracketleft;
    m_key_set_playing_screenset = GDK_Home;
    m_key_group_on = GDK_igrave;
    m_key_group_off = GDK_apostrophe;
    m_key_group_learn = GDK_Insert;

    m_key_start  = GDK_space;
    m_key_stop   = GDK_Escape;
    m_key_forward   = GDK_f;
    m_key_rewind   = GDK_r;
    m_key_pointer   = GDK_p;

    m_key_song   = GDK_F1;
    m_key_jack   = GDK_F2;
    m_key_menu   = GDK_F3;
    m_key_follow_trans  = GDK_F4;

    m_jack_stop_tick = 0;

    m_offset = 0;
    m_control_status = 0;
    m_screen_set = 0;

    m_jack_running = false;
    m_toggle_jack = false;

    m_jack_master = false;

    m_out_thread_launched = false;
    m_in_thread_launched = false;

    m_playback_mode = false;
    m_follow_transport = true;
    m_start_from_perfedit = false;

    m_bp_measure = 4;
    m_bw = 4;
    m_excell_FF_RW = 1.0;

    m_have_undo = false;
    m_have_redo = false;
}

void perform::init()
{
    m_master_bus.init( );
}

void perform::init_jack()
{

#ifdef JACK_SUPPORT

    if ( global_with_jack_transport  && !m_jack_running)
    {
        m_jack_running = true;
        m_jack_master = true;

        //printf ( "init_jack() m_jack_running[%d]\n", m_jack_running );

        do
        {
            /* become a new client of the JACK server */
#ifdef JACK_SESSION
            if (global_jack_session_uuid.empty())
                m_jack_client = jack_client_open(PACKAGE, JackNullOption, NULL);
            else
                m_jack_client = jack_client_open(PACKAGE, JackSessionID, NULL,
                                                 global_jack_session_uuid.c_str());
#else
            m_jack_client = jack_client_open(PACKAGE, JackNullOption, NULL );
#endif

            if (m_jack_client == 0)
            {
                printf( "JACK server is not running.\n[JACK sync disabled]\n");
                m_jack_running = false;
                break;
            }
            else
                m_jack_frame_rate = jack_get_sample_rate( m_jack_client );

            /*
                The call to jack_timebase_callback() to supply jack with BBT, etc would
                occasionally fail when the *pos information had zero or some garbage in
                the pos.frame_rate variable. This would occur when there was a rapid change
                of frame position by another client... i.e. qjackctl.
                From the jack API:

                "   pos	address of the position structure for the next cycle;
                    pos->frame will be its frame number. If new_pos is FALSE,
                    this structure contains extended position information from the current cycle.
                    If TRUE, it contains whatever was set by the requester.
                    The timebase_callback's task is to update the extended information here."

                The "If TRUE" line seems to be the issue. It seems that qjackctl does not
                always set pos.frame_rate so we get garbage and some strange BBT calculations
                that display in qjackctl. So we need to set it here and just use m_jack_frame_rate
                for calculations instead of pos.frame_rate.
            */

            jack_on_shutdown( m_jack_client, jack_shutdown,(void *) this );

            /* now using jack_process_callback() ca. 7/10/16    */
            /*
                jack_set_sync_callback(m_jack_client, jack_sync_callback,
                                   (void *) this );
            */

            jack_set_process_callback(m_jack_client, jack_process_callback, (void *) this);

#ifdef JACK_SESSION
            if (jack_set_session_callback)
                jack_set_session_callback(m_jack_client, jack_session_callback,
                                          (void *) this );
#endif

            /* true if we want to fail if there is already a master */
            bool cond = global_with_jack_master_cond;

            if ( global_with_jack_master &&
                    jack_set_timebase_callback(m_jack_client, cond,
                                               jack_timebase_callback, this) == 0)
            {
                printf("[JACK transport master]\n");
                m_jack_master = true;
            }
            else
            {
                printf("[JACK transport slave]\n");
                m_jack_master = false;
            }

            if (jack_activate(m_jack_client))
            {
                printf("Cannot register as JACK client\n");
                m_jack_running = false;
                break;
            }
        }
        while (0);
    }
#endif
}

void perform::deinit_jack()
{
#ifdef JACK_SUPPORT

    if ( m_jack_running)
    {
        //printf ( "deinit_jack() m_jack_running[%d]\n", m_jack_running );

        m_jack_running = false;
        m_jack_master = false;

        if ( jack_release_timebase(m_jack_client))
        {
            printf("Cannot release Timebase.\n");
        }

        if (jack_client_close(m_jack_client))
        {
            printf("Cannot close JACK client.\n");
        }
    }

    if ( !m_jack_running )
    {
        printf( "[JACK sync disabled]\n");
    }

#endif
}

bool perform::clear_all()
{
    for (int i=0; i< c_max_sequence; i++ )
    {
        if ( is_active(i) )
        {
            if ( m_seqs[i] != NULL && m_seqs[i]->get_editing() )
            {
                return false;
            }
        }
    }

    reset_sequences();

    for (int i=0; i< c_max_sequence; i++ )
    {
        if ( is_active(i) )
            delete_sequence( i );
    }

    string e( "" );

    for (int i=0; i<c_max_sets; i++ )
    {
        set_screen_set_notepad( i, &e );
    }

    set_have_undo(false);
    set_have_redo(false);

    return true;
}

void perform::set_mode_group_mute ()
{
    m_mode_group = true;
}

void perform::unset_mode_group_mute ()
{
    m_mode_group = false;
}

void perform::set_group_mute_state (int a_g_track, bool a_mute_state)
{
    if (a_g_track < 0)
        a_g_track = 0;
    if (a_g_track > c_seqs_in_set)
        a_g_track = c_seqs_in_set -1;
    m_mute_group[a_g_track + m_mute_group_selected * c_seqs_in_set] = a_mute_state;
}

bool perform::get_group_mute_state (int a_g_track)
{
    if (a_g_track < 0)
        a_g_track = 0;
    if (a_g_track > c_seqs_in_set)
        a_g_track = c_seqs_in_set -1;
    return m_mute_group[a_g_track + m_mute_group_selected * c_seqs_in_set];
}

void perform::select_group_mute (int a_g_mute)
{
    int j = (a_g_mute * c_seqs_in_set);
    int k = m_playing_screen * c_seqs_in_set;
    if (a_g_mute < 0)
        a_g_mute = 0;
    if (a_g_mute > c_seqs_in_set)
        a_g_mute = c_seqs_in_set -1;
    if (m_mode_group_learn)
        for (int i = 0; i < c_seqs_in_set; i++)
        {
            if (is_active(i + k))
            {
                assert(m_seqs[i + k]);
                m_mute_group[i + j] = m_seqs[i + k]->get_playing();
            }
        }
    m_mute_group_selected = a_g_mute;
}

void perform::set_mode_group_learn ()
{
    set_mode_group_mute();
    m_mode_group_learn = true;
    for (size_t x = 0; x < m_notify.size(); ++x)
        m_notify[x]->on_grouplearnchange( true );
}

void perform::unset_mode_group_learn ()
{
    for (size_t x = 0; x < m_notify.size(); ++x)
        m_notify[x]->on_grouplearnchange( false );
    m_mode_group_learn = false;
}

void perform::select_mute_group ( int a_group )
{
    int j = (a_group * c_seqs_in_set);
    int k = m_playing_screen * c_seqs_in_set;
    if (a_group < 0)
        a_group = 0;
    if (a_group > c_seqs_in_set)
        a_group = c_seqs_in_set -1;
    m_mute_group_selected = a_group;
    for (int i = 0; i < c_seqs_in_set; i++)
    {
        if ((m_mode_group_learn) && (is_active(i + k)))
        {
            assert(m_seqs[i + k]);
            m_mute_group[i + j] = m_seqs[i + k]->get_playing();
        }
        m_tracks_mute_state[i] = m_mute_group[i + m_mute_group_selected * c_seqs_in_set];
    }
}

void perform::mute_group_tracks ()
{
    if (m_mode_group)
    {
        for (int i=0; i< c_seqs_in_set; i++)
        {
            for (int j=0; j < c_seqs_in_set; j++)
            {
                if ( is_active(i * c_seqs_in_set + j) )
                {
                    if ((i == m_playing_screen) && (m_tracks_mute_state[j]))
                    {
                        sequence_playing_on (i * c_seqs_in_set + j);
                    }
                    else
                    {
                        sequence_playing_off (i * c_seqs_in_set + j);
                    }
                }
            }
        }
    }
}

void perform::select_and_mute_group (int a_g_group)
{
    select_mute_group(a_g_group);
    mute_group_tracks();
}

void perform::set_reposition(bool a_pos_type)
{
    m_reposition = a_pos_type;
}

void perform::set_song_mute( mute_op op  )
{
    for (int i=0; i< c_max_sequence; i++ )
    {
        if ( is_active(i) )
        {
            if(op == MUTE_ON)
            {
                m_seqs[i]->set_song_mute( true );
            }
            else if(op == MUTE_OFF)
            {
                m_seqs[i]->set_song_mute( false );
            }
            else if(op == MUTE_TOGGLE)
            {
                m_seqs[i]->set_song_mute( ! m_seqs[i]->get_song_mute() );
            }
            m_seqs[i]->set_dirty_mp();
        }
    }
}

perform::~perform()
{
    m_inputing = false;
    m_outputing = false;

    m_condition_var.signal();

    if (m_out_thread_launched )
        pthread_join( m_out_thread, NULL );

    if (m_in_thread_launched )
        pthread_join( m_in_thread, NULL );

    for (int i=0; i< c_max_sequence; i++ )
    {
        if ( is_active(i) )
        {
            delete m_seqs[i];
        }
    }
}

void
perform::start_playing()
{
    if(global_song_start_mode || m_start_from_perfedit)
    {
        // song mode
        if(m_jack_master)
        {
           if(!m_reposition)    // allow to start at key-p position if set
                position_jack(true, m_left_tick);     // for cosmetic reasons - to stop transport line flicker on start
        }
        start_jack( );
        start( true );           // true for setting song m_playback_mode = true
    }
    else
    {
        // live mode
        if(m_jack_master)
            position_jack(false, 0);   // for cosmetic reasons - to stop transport line flicker on start
        start( false );
        start_jack( );
    }
}

void
perform::stop_playing()
{
    stop_jack();
    stop();
}

void
perform::FF_rewind()
{
    if(FF_RW_button_type == 0)
        return;

    long a_tick = 0;
    long measure_ticks = (c_ppqn * 4) * m_bp_measure / m_bw;
    measure_ticks /= 4;
    measure_ticks *= m_excell_FF_RW;

    if(FF_RW_button_type < 0)  // rewind
    {
        a_tick = m_tick - measure_ticks;
        if(a_tick < 0)
            a_tick = 0;
    }
    if(FF_RW_button_type > 0)  // Fast Forward
        a_tick = m_tick + measure_ticks;

    if(m_jack_running)
    {
        position_jack(true, a_tick);
    }
    else
    {
        set_starting_tick(a_tick);  // this will set progress line
        set_reposition();
    }
}

void perform::set_start_from_perfedit( bool a_start )
{
    m_start_from_perfedit = a_start;
}

void perform::toggle_song_mode()
{
    if(global_song_start_mode)
        global_song_start_mode = false;
    else
    {
        global_song_start_mode = true;
    }
}

void perform::toggle_jack_mode()
{
    set_jack_mode(!m_jack_running);
}

void perform::set_jack_mode(bool a_mode)
{
    m_toggle_jack = a_mode;
}

bool perform::get_toggle_jack()
{
    return m_toggle_jack;
}

bool perform::is_jack_running()
{
    return m_jack_running;
}

void perform::set_follow_transport(bool a_set)
{
    m_follow_transport = a_set;
}

bool perform::get_follow_transport()
{
    return m_follow_transport;
}

void perform::toggle_follow_transport()
{
    set_follow_transport(!m_follow_transport);
}

void perform::set_jack_stop_tick(long a_tick)
{
    m_jack_stop_tick = a_tick;
}

void perform::set_left_tick( long a_tick )
{
    m_left_tick = a_tick;
    m_starting_tick = a_tick;

    if(m_jack_master) // don't use in slave mode
    {
        position_jack(true, a_tick);
    }
    else if (!m_jack_running)
    {
        m_tick = a_tick;
    }

    m_reposition = false;

    if ( m_left_tick >= m_right_tick )
        m_right_tick = m_left_tick + c_ppqn * 4;
}

long perform::get_left_tick()
{
    return m_left_tick;
}

void perform::set_starting_tick( long a_tick )
{
    m_starting_tick = a_tick;
    m_tick = a_tick;    // set progress line
}

long perform::get_starting_tick()
{
    return m_starting_tick;
}

void perform::set_right_tick( long a_tick )
{
    if ( a_tick >= c_ppqn * 4 )
    {
        m_right_tick = a_tick;

        if ( m_right_tick <= m_left_tick )
        {
            m_left_tick = m_right_tick - c_ppqn * 4;
            m_starting_tick = m_left_tick;

            if(m_jack_master && m_jack_running)
                position_jack(true, m_left_tick);
            else
                m_tick = m_left_tick;

            m_reposition = false;
        }
    }
}

long perform::get_right_tick()
{
    return m_right_tick;
}

void perform::add_sequence( sequence *a_seq, int a_pref )
{
    /* check for preferred */
    if ( a_pref < c_max_sequence &&
            is_active(a_pref) == false &&
            a_pref >= 0 )
    {
        m_seqs[a_pref] = a_seq;
        set_active(a_pref, true);
    }
    else if(a_pref >= 0)
    {
        for (int i=a_pref; i< c_max_sequence; i++ )
        {
            if ( is_active(i) == false )
            {
                m_seqs[i] = a_seq;
                set_active(i,true);
                break;
            }
        }
    }
}

void perform::set_active( int a_sequence, bool a_active )
{
    if ( a_sequence < 0 || a_sequence >= c_max_sequence )
        return;

    //printf ("set_active %d\n", a_active );

    if ( m_seqs_active[ a_sequence ] == true && a_active == false )
    {
        set_was_active(a_sequence);
    }

    m_seqs_active[ a_sequence ] = a_active;
}

void perform::set_was_active( int a_sequence )
{
    if ( a_sequence < 0 || a_sequence >= c_max_sequence )
        return;

    //printf( "was_active true\n" );

    m_was_active_main[ a_sequence ] = true;
    m_was_active_edit[ a_sequence ] = true;
    m_was_active_perf[ a_sequence ] = true;
    m_was_active_names[ a_sequence ] = true;
}

bool perform::is_active( int a_sequence )
{
    if ( a_sequence < 0 || a_sequence >= c_max_sequence )
        return false;

    return m_seqs_active[ a_sequence ];
}

bool perform::is_dirty_main (int a_sequence)
{
    if ( a_sequence < 0 || a_sequence >= c_max_sequence )
        return false;

    if ( is_active(a_sequence) )
    {
        return m_seqs[a_sequence]->is_dirty_main();
    }

    bool was_active = m_was_active_main[ a_sequence ];
    m_was_active_main[ a_sequence ] = false;

    return was_active;
}

bool perform::is_dirty_edit (int a_sequence)
{
    if ( a_sequence < 0 || a_sequence >= c_max_sequence )
        return false;

    if ( is_active(a_sequence) )
    {
        return m_seqs[a_sequence]->is_dirty_edit();
    }

    bool was_active = m_was_active_edit[ a_sequence ];
    m_was_active_edit[ a_sequence ] = false;

    return was_active;
}

bool perform::is_dirty_perf (int a_sequence)
{
    if ( a_sequence < 0 || a_sequence >= c_max_sequence )
        return false;

    if ( is_active(a_sequence) )
    {
        return m_seqs[a_sequence]->is_dirty_perf();
    }

    bool was_active = m_was_active_perf[ a_sequence ];
    m_was_active_perf[ a_sequence ] = false;

    return was_active;
}

bool perform::is_dirty_names (int a_sequence)
{
    if ( a_sequence < 0 || a_sequence >= c_max_sequence )
        return false;

    if ( is_active(a_sequence) )
    {
        return m_seqs[a_sequence]->is_dirty_names();
    }

    bool was_active = m_was_active_names[ a_sequence ];
    m_was_active_names[ a_sequence ] = false;

    return was_active;
}

sequence* perform::get_sequence( int a_sequence )
{
    return m_seqs[a_sequence];
}

mastermidibus* perform::get_master_midi_bus( )
{
    return &m_master_bus;
}

void perform::set_bpm(int a_bpm)
{
    if ( a_bpm < 5 )  a_bpm = 5;
    if ( a_bpm > 500 ) a_bpm = 500;

    if ( ! (m_jack_running && global_is_running ))
    {
        m_master_bus.set_bpm( a_bpm );
    }
}

int  perform::get_bpm()
{
    return  m_master_bus.get_bpm( );
}

void perform::set_bp_measure(int a_bp_mes)
{
    m_bp_measure = a_bp_mes;
}

int perform::get_bp_measure()
{
    return m_bp_measure;
}

void perform::set_bw(int a_bw)
{
    m_bw = a_bw;
}

int perform::get_bw( )
{
    return m_bw;
}

void perform::delete_sequence( int a_num )
{
    set_active(a_num, false);

    if ( m_seqs[a_num] != NULL &&
            !m_seqs[a_num]->get_editing() )
    {
        m_seqs[a_num]->set_playing( false );
        delete m_seqs[a_num];
        global_is_modified = true;
    }
}

bool perform::is_sequence_in_edit( int a_num )
{
    return ( m_seqs[a_num] != NULL &&
             m_seqs[a_num]->get_editing());
}

void perform::new_sequence( int a_sequence )
{
    m_seqs[ a_sequence ] = new sequence();
    m_seqs[ a_sequence ]->set_master_midi_bus( &m_master_bus );
    set_active(a_sequence, true);
    global_is_modified = true;
}

midi_control * perform::get_midi_control_toggle( unsigned int a_seq )
{
    if ( a_seq >= (unsigned int) c_midi_controls )
        return NULL;
    return &m_midi_cc_toggle[a_seq];
}

midi_control * perform::get_midi_control_on( unsigned int a_seq )
{
    if ( a_seq >= (unsigned int) c_midi_controls )
        return NULL;
    return &m_midi_cc_on[a_seq];
}

midi_control* perform::get_midi_control_off( unsigned int a_seq )
{
    if ( a_seq >= (unsigned int) c_midi_controls )
        return NULL;
    return &m_midi_cc_off[a_seq];
}

void perform::print()
{
//    for( int i=0; i<m_numSeq; i++ )
//    {
//        printf("Sequence %d\n", i);
//        m_seqs[i]->print();
//    }

//      m_master_bus.print();
}

void perform::set_screen_set_notepad( int a_screen_set, string *a_notepad )
{
    if ( a_screen_set < c_max_sets )
        m_screen_set_notepad[a_screen_set] = *a_notepad;
}

string * perform::get_screen_set_notepad( int a_screen_set )
{
    return &m_screen_set_notepad[a_screen_set];
}

void perform::set_screenset( int a_ss )
{
    m_screen_set = a_ss;

    if ( m_screen_set < 0 )
        m_screen_set = c_max_sets - 1;

    if ( m_screen_set >= c_max_sets )
        m_screen_set = 0;
}

int perform::get_screenset()
{
    return m_screen_set;
}

void perform::set_playing_screenset ()
{
    for (int j, i = 0; i < c_seqs_in_set; i++)
    {
        j = i + m_playing_screen * c_seqs_in_set;
        if ( is_active(j) )
        {
            assert( m_seqs[j] );
            m_tracks_mute_state[i] = m_seqs[j]->get_playing();
        }
    }
    m_playing_screen = m_screen_set;
    mute_group_tracks();
}

int perform::get_playing_screenset ()
{
    return m_playing_screen;
}

void perform::set_offset( int a_offset )
{
    m_offset = a_offset  * c_mainwnd_rows * c_mainwnd_cols;
}

void perform::play( long a_tick )
{
    /* just run down the list of sequences and have them dump */

    //printf( "play [%d]\n", a_tick );

    m_tick = a_tick;
    for (int i=0; i< c_max_sequence; i++ )
    {
        if ( is_active(i) )
        {
            assert( m_seqs[i] );

            if ( m_seqs[i]->get_queued() &&
                    m_seqs[i]->get_queued_tick() <= a_tick )
            {
                m_seqs[i]->play( m_seqs[i]->get_queued_tick() - 1, m_playback_mode );
                m_seqs[i]->toggle_playing();
            }

            m_seqs[i]->play( a_tick, m_playback_mode );
        }
    }

    /* flush the bus */
    m_master_bus.flush();
}

void perform::set_orig_ticks( long a_tick  )
{
    for (int i=0; i< c_max_sequence; i++ )
    {
        if ( is_active(i) == true )
        {
            assert( m_seqs[i] );
            m_seqs[i]->set_orig_tick( a_tick );
        }
    }
}

void perform::clear_sequence_triggers( int a_seq  )
{
    if ( is_active(a_seq) == true )
    {
        assert( m_seqs[a_seq] );
        m_seqs[a_seq]->clear_triggers( );
    }
}

void perform::move_triggers( bool a_direction )
{
    if ( m_left_tick < m_right_tick )
    {
        long distance = m_right_tick - m_left_tick;

        for (int i=0; i< c_max_sequence; i++ )
        {
            if ( is_active(i) == true )
            {
                assert( m_seqs[i] );
                m_seqs[i]->move_triggers( m_left_tick, distance, a_direction );
            }
        }
    }
}

void perform::push_trigger_undo(int a_track)
{
    undo_vect.push_back(a_track);

    if(a_track < 0) // -1 is all tracks
    {
        for (int i=0; i< c_max_sequence; i++ )
        {
            if ( is_active(i) == true )
            {
                assert( m_seqs[i] );
                m_seqs[i]->push_trigger_undo( );
            }
        }
    }
    else
    {
        if ( is_active(a_track) == true )
        {
            assert( m_seqs[a_track] );
            m_seqs[a_track]->push_trigger_undo( );
        }
    }

    set_have_undo(true);
}

void perform::pop_trigger_undo()
{
    int a_track = undo_vect[undo_vect.size()-1];
    undo_vect.pop_back();
    redo_vect.push_back(a_track);

    if(a_track < 0)
    {
        for (int i=0; i< c_max_sequence; i++ )
        {
            if ( is_active(i) == true )
            {
                assert( m_seqs[i] );
                m_seqs[i]->pop_trigger_undo( );
            }
        }
    }
    else
    {
        if ( is_active(a_track) == true )
        {
            assert( m_seqs[a_track] );
            m_seqs[a_track]->pop_trigger_undo( );
        }
    }

    if(undo_vect.size() == 0)
        set_have_undo(false);
    else
        set_have_undo(true);

    if(redo_vect.size() == 0)
        set_have_redo(false);
    else
        set_have_redo(true);
}

void perform::pop_trigger_redo()
{
    int a_track = redo_vect[redo_vect.size()-1];
    redo_vect.pop_back();
    undo_vect.push_back(a_track);

    if(a_track < 0)
    {
        for (int i=0; i< c_max_sequence; i++ )
        {
            if ( is_active(i) == true )
            {
                assert( m_seqs[i] );
                m_seqs[i]->pop_trigger_redo( );
            }
        }
    }
    else
    {
        if ( is_active(a_track) == true )
        {
            assert( m_seqs[a_track] );
            m_seqs[a_track]->pop_trigger_redo( );
        }
    }

    if(redo_vect.size() == 0)
        set_have_redo(false);
    else
        set_have_redo(true);

    if(undo_vect.size() == 0)
        set_have_undo(false);
    else
        set_have_undo(true);
}

void perform::set_have_undo(bool a_undo)
{
    m_have_undo = a_undo;
    global_is_modified = true; // once set always set - unless cleared by save file
}

void perform::set_have_redo(bool a_redo)
{
    m_have_redo = a_redo;
}

/* copies between L and R -> R */
void perform::copy_triggers( )
{
    if ( m_left_tick < m_right_tick )
    {
        long distance = m_right_tick - m_left_tick;

        for (int i=0; i< c_max_sequence; i++ )
        {
            if ( is_active(i) == true )
            {
                assert( m_seqs[i] );
                m_seqs[i]->copy_triggers( m_left_tick, distance );
            }
        }
    }
}

void perform::start_jack()
{
    //printf( "perform::start_jack()\n" );
#ifdef JACK_SUPPORT
    if ( m_jack_running)
        jack_transport_start (m_jack_client );
#endif
}

void perform::stop_jack()
{
    //printf( "perform::stop_jack()\n" );
#ifdef JACK_SUPPORT
    if( m_jack_running )
        jack_transport_stop (m_jack_client);
#endif
}

void perform::position_jack( bool a_state, long a_tick )
{
    //printf( "perform::position_jack()\n" );

#ifdef JACK_SUPPORT

    long current_tick = 0;

    if(a_state) // master in song mode
    {
        current_tick = a_tick;
    }

    current_tick *= 10;

    /*  This jack_frame calculation is all that is needed to change jack position
        The BBT calc can be sent but will be overridden by the first call to
        jack_timebase_callback() of any master set. If no master is set, then the
        BBT will display the new position but will not change even if the transport
        is rolling. There is no need to send BBT on position change - the fact that
        the function jack_transport_locate() exists and only uses the frame position
        is proof that BBT is not needed! Upon further reflection, why not send BBT?
        Because other programs do not.... lets follow convention.
        The below calculation for jack_transport_locate(), works, is simpler and
        does not send BBT. The calc for jack_transport_reposition() will be commented
        out again....
    */

    int ticks_per_beat = c_ppqn * 10; // 192 * 10 = 1920
    int beats_per_minute =  m_master_bus.get_bpm();

    uint64_t tick_rate = ((uint64_t)m_jack_frame_rate * current_tick * 60.0);
    long tpb_bpm = ticks_per_beat * beats_per_minute / (m_bw / 4.0 );
    uint64_t jack_frame = tick_rate / tpb_bpm;

    jack_transport_locate(m_jack_client,jack_frame);

#if 0
    /* The below BBT call to jack_BBT_position() is not necessary to change jack position!!! */

    jack_position_t pos;
    double jack_tick = current_tick * (m_bw / 4.0 );

    /* gotta set these here since they are set in timebase */
    pos.ticks_per_beat = c_ppqn * 10; // 192 * 10 = 1920
    pos.beats_per_minute =  m_master_bus.get_bpm();

    jack_BBT_position(pos, jack_tick);

    /* this calculates jack frame to put into pos.frame.
       it is what really matters for position change */

    uint64_t tick_rate = ((uint64_t)pos.frame_rate * current_tick * 60.0);
    long tpb_bpm = pos.ticks_per_beat * pos.beats_per_minute / (pos.beat_type / 4.0 );
    pos.frame = tick_rate / tpb_bpm; // pos.frame is all that is needed for position change!

    /*
       ticks * 10 = jack ticks;
       jack ticks / ticks per beat = num beats;
       num beats / beats per minute = num minutes
       num minutes * 60 = num seconds
       num secords * frame_rate  = frame */

    jack_transport_reposition( m_jack_client, &pos );
#endif // 0

    if(global_is_running)
        m_reposition = false;

#endif // JACK_SUPPORT
}

void perform::start(bool a_state)
{
    if (m_jack_running)
    {
        return;
    }

    inner_start(a_state);
}

/*
    stop(); This function's sole purpose was to prevent inner_stop() from being called
    internally when jack was running...potentially twice?. inner_stop() was called by output_func()
    when jack sent a JackTransportStopped message. If seq42 initiated the stop, then
    stop_jack() was called which then triggered the JackTransportStopped message
    to output_func() which then triggered the bool stop_jack to call inner_stop().
    The output_func() call to inner_stop() is only necessary when some other jack
    client sends a jack_transport_stop message to jack, not when it is initiated
    by seq42.  The method of relying on jack to call inner_stop() when internally initiated
    caused a (very) obscure apparent freeze if you press and hold the start/stop key
    if set to toggle. This occurs because of the delay between JackTransportStarting and
    JackTransportStopped if both triggered in rapid succession by holding the toggle key
    down.  The variable global_is_running gets set false by a delayed inner_stop()
    from jack after the start (true) is already sent. This means the global is set to true
    when jack is actually off (false). Any subsequent presses to the toggle key send a
    stop message because the global is set to true. Because jack is not running,
    output_func() is not running to send the inner_stop() call which resets the global
    to false. Thus an apparent freeze as the toggle key endlessly sends a stop, but
    inner_stop() never gets called to reset. Whoo! So, to fix this we just need to call
    inner_stop() directly rather than wait for jack to send a delayed stop, only when
    running. This makes the whole purpose of this stop() function unneeded. The check
    for m_jack_running is commented out and this function could be removed. It is
    being left for future generations to ponder!!!
*/
void perform::stop()
{
//    if (m_jack_running)
//    {
//        return;
//    }

    inner_stop();
}

void perform::inner_start(bool a_state)
{
    m_condition_var.lock();

    if (!global_is_running)
    {
        set_playback_mode( a_state );

        if (a_state)
            off_sequences();

        global_is_running = true;
        m_condition_var.signal();
    }

    m_condition_var.unlock();
}

void perform::inner_stop(bool a_midi_clock)
{
    set_start_from_perfedit(false);
    global_is_running = false;
    reset_sequences();
    m_usemidiclock = a_midi_clock;
}

void perform::off_sequences()
{
    for (int i = 0; i < c_max_sequence; i++)
    {
        if (is_active(i))
        {
            assert(m_seqs[i]);
            m_seqs[i]->set_playing(false);
        }
    }
}

void perform::all_notes_off()
{
    for (int i=0; i< c_max_sequence; i++)
    {
        if (is_active(i))
        {
            assert(m_seqs[i]);
            m_seqs[i]->off_playing_notes();
        }
    }
    /* flush the bus */
    m_master_bus.flush();
}

void perform::reset_sequences()
{
    for (int i=0; i< c_max_sequence; i++)
    {
        if (is_active(i))
        {
            assert( m_seqs[i] );

            bool state = m_seqs[i]->get_playing();

            m_seqs[i]->off_playing_notes();
            m_seqs[i]->set_playing(false);
            m_seqs[i]->zero_markers();

            if (!m_playback_mode)
                m_seqs[i]->set_playing(state);
        }
    }
    /* flush the bus */
    m_master_bus.flush();
}

void perform::launch_output_thread()
{
    int err;

    err = pthread_create(&m_out_thread, NULL, output_thread_func, this);
    if (err != 0)
    {
        /*TODO: error handling*/
    }
    else
        m_out_thread_launched= true;
}

void perform::set_playback_mode( bool a_playback_mode )
{
    m_playback_mode = a_playback_mode;
}

void perform::launch_input_thread()
{
    int err;

    err = pthread_create(&m_in_thread, NULL, input_thread_func, this);
    if (err != 0)
    {
        /*TODO: error handling*/
    }
    else
        m_in_thread_launched = true;
}

long perform::get_max_trigger()
{
    long ret = 0, t;

    for (int i=0; i< c_max_sequence; i++ )
    {
        if ( is_active(i) == true )
        {
            assert( m_seqs[i] );

            t = m_seqs[i]->get_max_trigger( );
            if ( t > ret )
                ret = t;
        }
    }

    return ret;
}

void* output_thread_func(void *a_pef )
{
    /* set our performance */
    perform *p = (perform *) a_pef;
    assert(p);

    struct sched_param schp;
    /*
     * set the process to realtime privs
     */

    if ( global_priority )
    {
        memset(&schp, 0, sizeof(sched_param));
        schp.sched_priority = 1;

#ifndef __WIN32__
        // Not in MinGW RCB
        if (sched_setscheduler(0, SCHED_FIFO, &schp) != 0)
        {
            printf("output_thread_func: couldnt sched_setscheduler"
                   " (FIFO), you need to be root.\n");
            pthread_exit(0);
        }
#endif
    }

#ifdef __WIN32__
    timeBeginPeriod(1);
#endif
    p->output_func();
#ifdef __WIN32__
    timeEndPeriod(1);
#endif

    return 0;
}

#ifdef JACK_SUPPORT

/*
    This process callback is called by jack whether stopped or rolling.
    Assuming every jack cycle...
    "...client supplied function that is called by the engine anytime there is work to be done".
    There seems to be no definition of '...work to be done'.
    nframes = buffer_size -- is not used.
*/

int jack_process_callback(jack_nframes_t nframes, void* arg)
{
    perform *m_mainperf = (perform *) arg;

    /* For start or FF/RW/ key-p when not running */
    if(!global_is_running)
    {
        jack_transport_state_t state = jack_transport_query( m_mainperf->m_jack_client, nullptr );

        /* we are stopped, do we need to start? */
        if(state == JackTransportRolling || state == JackTransportStarting )
        {
            /* we need to start */
            //printf("JackTransportState [%d]\n",state);
            m_mainperf->m_jack_transport_state_last = JackTransportStarting;

            if(m_mainperf->m_start_from_perfedit)
            {
                m_mainperf->inner_start( m_mainperf->m_start_from_perfedit );
            }
            else
            {
                m_mainperf->inner_start( global_song_start_mode );
            }

            //printf("JackTransportState [%d]\n",m_mainperf->m_jack_transport_state);
        }
        /* we don't need to start - just reposition transport marker */
        else
        {
            long tick = get_current_jack_position((void *)m_mainperf);
            long diff = tick - m_mainperf->get_jack_stop_tick();

            if(diff != 0)
            {
                m_mainperf->set_reposition();
                m_mainperf->set_starting_tick(tick);
                m_mainperf->set_jack_stop_tick(tick);
            }
        }
    }

    return 0;
}

#if 0
/* former slow sync callback - no longer used - now using jack_process_callback() - ca. 7/10/16 */
int jack_sync_callback(jack_transport_state_t state,
                       jack_position_t *pos, void *arg)
{
    perform *p = (perform *) arg;

    p->m_jack_transport_state_last =
        p->m_jack_transport_state =
            state;

    switch (state)
    {
    case JackTransportStopped:
        //printf( "[JackTransportStopped]\n" );
        break;

    case JackTransportRolling:
        //printf( "[JackTransportRolling]\n" );
        break;

    case JackTransportStarting:
        //printf( "[JackTransportStarting]\n" );
        if(p->m_start_from_perfedit)
            p->inner_start( p->m_start_from_perfedit );
        else
            p->inner_start( global_song_start_mode );
        break;

    case JackTransportLooping:
        //printf( "[JackTransportLooping]" );
        break;

    default:
        break;
    }

    //printf( "starting frame[%d] tick[%8.2f]\n", p->m_jack_frame_current, p->m_jack_tick );

    print_jack_pos( pos );
    return 1;
}
#endif // 0

#ifdef JACK_SESSION

bool perform::jack_session_event()
{
    Glib::ustring fname( m_jsession_ev->session_dir );
    fname += "file.mid";

    Glib::ustring cmd( "seq32 \"${SESSION_DIR}file.mid\" --jack_session_uuid " );
    cmd += m_jsession_ev->client_uuid;

    midifile f(fname);
    f.write(this);

    m_jsession_ev->command_line = strdup( cmd.c_str() );

    jack_session_reply( m_jack_client, m_jsession_ev );

    if( m_jsession_ev->type == JackSessionSaveAndQuit )
        Gtk::Main::quit();

    jack_session_event_free (m_jsession_ev);

    return false;
}

void jack_session_callback(jack_session_event_t *event, void *arg )
{
    perform *p = (perform *) arg;
    p->m_jsession_ev = event;
    Glib::signal_idle().connect( sigc::mem_fun( *p, &perform::jack_session_event) );
}

#endif // JACK_SESSION
#endif // JACK_SUPPORT

void perform::output_func()
{
    while (m_outputing)
    {
        //printf ("waiting for signal\n");

        m_condition_var.lock();

        while (!global_is_running)
        {
            m_condition_var.wait();

            /* if stopping, then kill thread */
            if (!m_outputing)
                break;
        }

        m_condition_var.unlock();

        //printf( "signaled [%d]\n", m_playback_mode );

#ifndef __WIN32__
        /* begning time */
        struct timespec last;
        /* current time */
        struct timespec current;

        struct timespec stats_loop_start;
        struct timespec stats_loop_finish;

        /* difference between last and current */
        struct timespec delta;
#else
        /* begning time */
        long last;
        /* current time */
        long current;

        long stats_loop_start = 0;
        long stats_loop_finish = 0;

        /* difference between last and current */
        long delta;
#endif

        /* tick and tick fraction */
        double current_tick   = 0.0;
        double total_tick   = 0.0;
        long clock_tick = 0;
        long delta_tick_frac = 0;

        long stats_total_tick = 0;

        long stats_loop_index = 0;
        long stats_min = 0x7FFFFFFF;
        long stats_max = 0;
        long stats_avg = 0;
        long stats_last_clock_us = 0;
        long stats_clock_width_us = 0;

        long stats_all[100];
        long stats_clock[100];

        bool jack_stopped = false;
        bool dumping = false;

        bool init_clock = true;

#ifdef JACK_SUPPORT
        double jack_ticks_converted = 0.0;
        double jack_ticks_converted_last = 0.0;
        double jack_ticks_delta = 0.0;
        if(m_jack_running && m_jack_master && m_playback_mode) // song mode master start left tick marker
        {
            if(!m_reposition)                                  // allow to start if key-p set
                position_jack(true, m_left_tick);
            else
                m_reposition = false;
        }

        if(m_jack_running && m_jack_master && !m_playback_mode)// live mode master start at zero
            position_jack(false, 0);
#endif
        for( int i=0; i<100; i++ )
        {
            stats_all[i] = 0;
            stats_clock[i] = 0;
        }

        /* if we are in the performance view, we care
           about starting from the offset */
        if ( m_playback_mode && !m_jack_running)
        {
            current_tick = m_starting_tick;
            clock_tick = m_starting_tick;
            set_orig_ticks( m_starting_tick );
        }

        int ppqn = m_master_bus.get_ppqn();
#ifndef __WIN32__
        /* get start time position */
        clock_gettime(CLOCK_REALTIME, &last);

        if ( global_stats )
            stats_last_clock_us= (last.tv_sec * 1000000) + (last.tv_nsec / 1000);
#else
        /* get start time position */
        /* timeGetTime() returns a "DWORD" type (= unsigned long)*/
        last = timeGetTime();

        if ( global_stats )
            stats_last_clock_us= last * 1000;
#endif

        while( global_is_running )
        {
            /************************************

              Get delta time ( current - last )
              Get delta ticks from time
              Add to current_ticks
              Compute prebuffer ticks
              play from current tick to prebuffer

             **************************************/

            if ( global_stats )
            {
#ifndef __WIN32__
                clock_gettime(CLOCK_REALTIME, &stats_loop_start);
#else
                stats_loop_start = timeGetTime();
#endif
            }

            /* delta time */
#ifndef __WIN32__
            clock_gettime(CLOCK_REALTIME, &current);
            delta.tv_sec  =  (current.tv_sec  - last.tv_sec  );
            delta.tv_nsec =  (current.tv_nsec - last.tv_nsec );
            long delta_us = (delta.tv_sec * 1000000) + (delta.tv_nsec / 1000);
#else
            current = timeGetTime();
            //printf( "current [0x%x]\n", current );
            delta = current - last;
            long delta_us = delta * 1000;
            //printf( "  delta [0x%x]\n", delta );
#endif

            /* delta time to ticks */
            /* bpm */
            int bpm  = m_master_bus.get_bpm() * ( 4.0 / m_bw);

            /* get delta ticks, delta_ticks_f is in 1000th of a tick */
            long long delta_tick_num = bpm * ppqn * delta_us + delta_tick_frac;
            long long delta_tick_denom = 60000000;
            long delta_tick = (long)(delta_tick_num / delta_tick_denom);
            delta_tick_frac = (long)(delta_tick_num % delta_tick_denom);

            if (m_usemidiclock)
            {
                delta_tick = m_midiclocktick;
                m_midiclocktick = 0;
            }

            if (0 <= m_midiclockpos)
            {
                delta_tick = 0;
                clock_tick     = m_midiclockpos;
                current_tick   = m_midiclockpos;
                total_tick     = m_midiclockpos;
                m_midiclockpos = -1;
            }

            //printf ( "    delta_tick[%lf]\n", delta_tick  );
#ifdef JACK_SUPPORT

            // no init until we get a good lock

            if ( m_jack_running )
            {
                init_clock = false;

                /*
                    Another note about jack....
                    If another jack client is supplying tempo/BBT info that is different from seq42 (as Master),
                    the perfroll grid will be incorrect. Perfroll uses internal temp/BBT and cannot update on
                    the fly. Even if seq42 could support tempo/BBT changes, all info would have to be available
                    before the transport start, to work. For this reason, the tempo/BBT info will be plugged from
                    the seq42 internal settings here... always. This is the method used by probably all other jack
                    clients with some sort of time-line. The jack API indicates that BBT is optional and AFIK,
                    other sequencers only use frame & frame_rate from jack for internal calculations. The tempo
                    and BBT info is always internal. Also, if there is no Master set, then we would need to plug
                    it here to follow the jack frame anyways.
                */

                m_jack_transport_state = jack_transport_query( m_jack_client, &m_jack_pos );

                m_jack_pos.beats_per_bar = m_bp_measure;
                m_jack_pos.beat_type = m_bw;
                m_jack_pos.ticks_per_beat = c_ppqn * 10;
                m_jack_pos.beats_per_minute =  m_master_bus.get_bpm();

                if ( m_jack_transport_state_last  ==  JackTransportStarting &&
                        m_jack_transport_state       == JackTransportRolling )
                {
                    m_jack_frame_current =  jack_get_current_transport_frame( m_jack_client );
                    m_jack_frame_last = m_jack_frame_current;

                    //printf ("[Start Playback]\n" );
                    dumping = true;
                    m_jack_tick =
                        m_jack_pos.frame *
                        m_jack_pos.ticks_per_beat *
                        m_jack_pos.beats_per_minute / (m_jack_frame_rate * 60.0);

                    /* convert ticks */
                    jack_ticks_converted =
                        m_jack_tick * ((double) c_ppqn /
                                       (m_jack_pos.ticks_per_beat *
                                        m_jack_pos.beat_type / 4.0  ));

                    set_orig_ticks( (long) jack_ticks_converted );
                    current_tick = clock_tick = total_tick = jack_ticks_converted_last = jack_ticks_converted;
                    init_clock = true;

                    if ( m_looping && m_playback_mode )
                    {
                        //printf( "left[%lf] right[%lf]\n", (double) get_left_tick(), (double) get_right_tick() );

                        if ( current_tick >= get_right_tick() )
                        {
                            while ( current_tick >= get_right_tick() )
                            {
                                double size = get_right_tick() - get_left_tick();
                                current_tick = current_tick - size;

                                //printf( "> current_tick[%lf]\n", current_tick );
                            }
                            off_sequences();
                            set_orig_ticks( (long)current_tick );

                        }
                    }
                }

                if ( m_jack_transport_state_last  ==  JackTransportRolling &&
                        m_jack_transport_state  == JackTransportStopped )
                {
                    m_jack_transport_state_last = JackTransportStopped;
                    //printf ("[Stop Playback]\n" );
                    jack_stopped = true;
                }

                //-----  Jack transport is Rolling Now ---------

                /* transport is in a sane state if dumping == true */
                if ( dumping )
                {
                    m_jack_frame_current =  jack_get_current_transport_frame( m_jack_client );

                    //printf( " frame[%7d]", m_jack_pos.frame );
                    //printf( " current_transport_frame[%7d]", m_jack_frame_current );

                    // if we are moving ahead
                    if ( (m_jack_frame_current > m_jack_frame_last))
                    {
                        m_jack_tick +=
                            (m_jack_frame_current - m_jack_frame_last)  *
                            m_jack_pos.ticks_per_beat *
                            m_jack_pos.beats_per_minute / (m_jack_frame_rate * 60.0);

                        //printf ( "m_jack_tick += (m_jack_frame_current[%lf] - m_jack_frame_last[%lf]) *\n",
                        //        (double) m_jack_frame_current, (double) m_jack_frame_last );
                        //printf(  "m_jack_pos.ticks_per_beat[%lf] * m_jack_pos.beats_per_minute[%lf] / \n(m_jack_frame_rate[%lf] * 60.0\n", (double) m_jack_pos.ticks_per_beat, (double) m_jack_pos.beats_per_minute, (double) m_jack_frame_rate);

                        m_jack_frame_last = m_jack_frame_current;
                    }

                    /* convert ticks */
                    jack_ticks_converted =
                        m_jack_tick * ((double) c_ppqn /
                                       (m_jack_pos.ticks_per_beat *
                                        m_jack_pos.beat_type / 4.0  ));

                    //printf ( "jack_ticks_conv[%lf] = \n",  jack_ticks_converted );
                    //printf ( "    m_jack_tick[%lf] * ((double) c_ppqn[%lf] / \n", m_jack_tick, (double) c_ppqn );
                    //printf ( "   (m_jack_pos.ticks_per_beat[%lf] * m_jack_pos.beat_type[%lf] / 4.0  )\n",
                    //        m_jack_pos.ticks_per_beat, m_jack_pos.beat_type );

                    jack_ticks_delta = jack_ticks_converted - jack_ticks_converted_last;

                    clock_tick     += jack_ticks_delta;
                    current_tick   += jack_ticks_delta;
                    total_tick     += jack_ticks_delta;

                    m_jack_transport_state_last = m_jack_transport_state;
                    jack_ticks_converted_last = jack_ticks_converted;

                    /* printf( "current_tick[%lf] delta[%lf]\n", current_tick, jack_ticks_delta ); */

                } /* end if dumping / sane state */
            } /* if jack running */
            else
            {
#endif // JACK_SUPPORT
                /* default if jack is not compiled in, or not running */
                /* add delta to current ticks */
                clock_tick     += delta_tick;
                current_tick   += delta_tick;
                total_tick     += delta_tick;
                dumping = true;

                /* if we reposition key-p from perfroll
                   then reset to adjusted starting  */
                if ( m_playback_mode && !m_jack_running && !m_usemidiclock && m_reposition)
                {
                    current_tick = m_starting_tick; // reposition sets m_starting_tick
                    set_orig_ticks( m_starting_tick );
                    m_starting_tick = m_left_tick;  // restart at left marker
                    m_reposition = false;
                }

#ifdef JACK_SUPPORT
            }
#endif // JACK_SUPPORT

            /* init_clock will be true when we run for the first time, or
             * as soon as jack gets a good lock on playback */

            if (init_clock)
            {
                m_master_bus.init_clock( clock_tick );
                init_clock = false;
            }

            if (dumping)
            {
                if ( m_looping && m_playback_mode )
                {
                    static bool jack_position_once = false;
                    if ( current_tick >= get_right_tick() )
                    {
#ifdef JACK_SUPPORT
                        if(m_jack_running && m_jack_master && !jack_position_once)
                        {
                            position_jack(true, m_left_tick);
                            jack_position_once = true;
                        }
#endif // JACK_SUPPORT
                        double leftover_tick = current_tick - (get_right_tick());

#ifdef JACK_SUPPORT     // don't play during JackTransportStarting to avoid xruns on FF or rewind
                        if(m_jack_running && m_jack_transport_state != JackTransportStarting)
                            play( get_right_tick() - 1 );
#endif // JACK_SUPPORT
                        if(!m_jack_running)
                            play( get_right_tick() - 1 );

                        reset_sequences();

                        set_orig_ticks( get_left_tick() );
                        current_tick = (double) get_left_tick() + leftover_tick;
                    }
                    else
                        jack_position_once = false;
                }

                /* play */
#ifdef JACK_SUPPORT // don't play during JackTransportStarting to avoid xruns on FF or rewind
                if(m_jack_running && m_jack_transport_state != JackTransportStarting)
                    play( (long) current_tick );
#endif // JACK_SUPPORT
                if(!m_jack_running)
                    play( (long) current_tick );
                //printf( "play[%d]\n", current_tick );

                /* midi clock */
                m_master_bus.clock( clock_tick );

                if ( global_stats )
                {
                    while ( stats_total_tick <= total_tick )
                    {
                        /* was there a tick ? */
                        if ( stats_total_tick % (c_ppqn / 24) == 0 )
                        {
#ifndef __WIN32__
                            long current_us = (current.tv_sec * 1000000) + (current.tv_nsec / 1000);
#else
                            long current_us = current * 1000;
#endif
                            stats_clock_width_us = current_us - stats_last_clock_us;
                            stats_last_clock_us = current_us;

                            int index = stats_clock_width_us / 300;
                            if ( index >= 100 ) index = 99;
                            stats_clock[index]++;
                        }
                        stats_total_tick++;
                    }
                }
            }

            /***********************************

              Figure out how much time
              we need to sleep, and do it

             ************************************/

            /* set last */
            last = current;

#ifndef __WIN32__
            clock_gettime(CLOCK_REALTIME, &current);
            delta.tv_sec  =  (current.tv_sec  - last.tv_sec  );
            delta.tv_nsec =  (current.tv_nsec - last.tv_nsec );
            long elapsed_us = (delta.tv_sec * 1000000) + (delta.tv_nsec / 1000);
            //printf( "elapsed_us[%ld]\n", elapsed_us );
#else
            current = timeGetTime();
            delta = current - last;
            long elapsed_us = delta * 1000;
            //printf( "        elapsed_us[%ld]\n", elapsed_us );
#endif
            /* now, we want to trigger every c_thread_trigger_width_ms,
               and it took us delta_us to play() */

            delta_us = (c_thread_trigger_width_ms * 1000) - elapsed_us;
            //printf( "sleeping_us[%ld]\n", delta_us );

            /* check midi clock adjustment */

            double next_total_tick = (total_tick + (c_ppqn / 24.0));
            double next_clock_delta   = (next_total_tick - total_tick - 1);

            double next_clock_delta_us =  (( next_clock_delta ) * 60000000.0f / c_ppqn  / bpm );

            if ( next_clock_delta_us < (c_thread_trigger_width_ms * 1000.0f * 2.0f) )
            {
                delta_us = (long)next_clock_delta_us;
            }

#ifndef __WIN32__
            if ( delta_us > 0.0 )
            {
                delta.tv_sec =  (delta_us / 1000000);
                delta.tv_nsec = (delta_us % 1000000) * 1000;

                //printf("sleeping() ");
                nanosleep( &delta, NULL );
            }
#else
            if ( delta_us > 0 )
            {
                delta =  (delta_us / 1000);

                //printf("           sleeping() [0x%x]\n", delta);
                Sleep(delta);
            }
#endif
            else
            {
                if ( global_stats )
                    printf ("underrun\n" );
            }

            if ( global_stats )
            {
#ifndef __WIN32__
                clock_gettime(CLOCK_REALTIME, &stats_loop_finish);
#else
                stats_loop_finish = timeGetTime();
#endif
            }

            if ( global_stats )
            {

#ifndef __WIN32__
                delta.tv_sec  =  (stats_loop_finish.tv_sec  - stats_loop_start.tv_sec  );
                delta.tv_nsec =  (stats_loop_finish.tv_nsec - stats_loop_start.tv_nsec );
                long delta_us = (delta.tv_sec * 1000000) + (delta.tv_nsec / 1000);
#else
                delta = stats_loop_finish - stats_loop_start;
                long delta_us = delta * 1000;
#endif
                int index = delta_us / 100;
                if ( index >= 100  ) index = 99;

                stats_all[index]++;

                if ( delta_us > stats_max )
                    stats_max = delta_us;

                if ( delta_us < stats_min )
                    stats_min = delta_us;

                stats_avg += delta_us;
                stats_loop_index++;

                if ( stats_loop_index > 200 )
                {
                    stats_loop_index = 0;
                    stats_avg /= 200;

                    printf("stats_avg[%ld]us stats_min[%ld]us"
                           " stats_max[%ld]us\n", stats_avg,
                           stats_min, stats_max);

                    stats_min = 0x7FFFFFFF;
                    stats_max = 0;
                    stats_avg = 0;
                }
            }

            if (jack_stopped)
                inner_stop();
        }

        if (global_stats)
        {
            printf("\n\n-- trigger width --\n");
            for (int i=0; i<100; i++ )
            {
                printf( "[%3d][%8ld]\n", i * 100, stats_all[i] );
            }
            printf("\n\n-- clock width --\n" );
            int bpm  = m_master_bus.get_bpm();

            printf("optimal: [%d]us\n", ((c_ppqn / 24)* 60000000 / c_ppqn / bpm));

            for ( int i=0; i<100; i++ )
            {
                printf( "[%3d][%8ld]\n", i * 300, stats_clock[i] );
            }
        }

         /* m_tick is the progress play tick that displays the progress line */
#ifdef JACK_SUPPORT
        if(m_playback_mode && m_jack_master) // master in song mode
        {
            position_jack(m_playback_mode,m_left_tick);
        }
        if(!m_playback_mode && m_jack_running && m_jack_master) // master in live mode
        {
            position_jack(m_playback_mode,0);
        }
#endif // JACK_SUPPORT
        if(!m_usemidiclock) // will be true if stopped by midi event
        {
            if(m_playback_mode && !m_jack_running) // song mode default
                m_tick = m_left_tick;

            if(!m_playback_mode && !m_jack_running) // live mode default
                m_tick = 0;
        }

        /* this means we leave m_tick at stopped location if in slave mode or m_usemidiclock = true */

        m_master_bus.flush();
        m_master_bus.stop();

#ifdef JACK_SUPPORT
        if(m_jack_running)
            m_jack_stop_tick = get_current_jack_position((void *)this);
#endif // JACK_SUPPORT
    }

    pthread_exit(0);
}

void* input_thread_func(void *a_pef )
{
    /* set our performance */
    perform *p = (perform *) a_pef;
    assert(p);

    struct sched_param schp;
    /*
     * set the process to realtime privs
     */

    if ( global_priority )
    {
        memset(&schp, 0, sizeof(sched_param));
        schp.sched_priority = 1;

#ifndef __WIN32__
        // MinGW RCB
        if (sched_setscheduler(0, SCHED_FIFO, &schp) != 0)
        {
            printf("input_thread_func: couldnt sched_setscheduler"
                   " (FIFO), you need to be root.\n");
            pthread_exit(0);
        }
#endif
    }
#ifdef __WIN32__
    timeBeginPeriod(1);
#endif

    p->input_func();

#ifdef __WIN32__
    timeEndPeriod(1);
#endif

    return 0;
}

void perform::handle_midi_control( int a_control, bool a_state )
{
    switch (a_control)
    {
    case c_midi_control_bpm_up:
        //printf ( "bpm up\n" );
        set_bpm( get_bpm() + 1 );
        break;

    case c_midi_control_bpm_dn:
        //printf ( "bpm dn\n" );
        set_bpm( get_bpm() - 1 );
        break;

    case c_midi_control_ss_up:
        //printf ( "ss up\n" );
        set_screenset( get_screenset() + 1 );
        break;

    case c_midi_control_ss_dn:
        //printf ( "ss dn\n" );
        set_screenset( get_screenset() - 1 );
        break;

    case c_midi_control_mod_replace:
        //printf ( "replace\n" );
        if ( a_state )
            set_sequence_control_status( c_status_replace );
        else
            unset_sequence_control_status( c_status_replace );
        break;

    case c_midi_control_mod_snapshot:
        //printf ( "snapshot\n" );
        if ( a_state )
            set_sequence_control_status( c_status_snapshot );
        else
            unset_sequence_control_status( c_status_snapshot );
        break;

    case c_midi_control_mod_queue:
        //printf ( "queue\n" );
        if ( a_state )
            set_sequence_control_status( c_status_queue );
        else
            unset_sequence_control_status( c_status_queue );

    //andy cases
    case c_midi_control_mod_gmute:
        printf ( "gmute\n" );
        if (a_state)
            set_mode_group_mute();
        else
            unset_mode_group_mute();
        break;

    case c_midi_control_mod_glearn:
        //printf ( "glearn\n" );
        if (a_state)
            set_mode_group_learn();
        else
            unset_mode_group_learn();
        break;

    case c_midi_control_play_ss:
        //printf ( "play_ss\n" );
        set_playing_screenset();
        break;

    default:
        if ((a_control >= c_seqs_in_set) && (a_control < c_midi_track_ctrl))
        {
            //printf ( "group mute\n" );
            select_and_mute_group(a_control - c_seqs_in_set);
        }
        break;
    }
}

void perform::input_func()
{
    event ev;

    while (m_inputing)
    {
        if ( m_master_bus.poll_for_midi() > 0 )
        {
            do
            {
                if (m_master_bus.get_midi_event(&ev) )
                {
                    // only used when starting from the beginning of the song = 0
                    if (ev.get_status() == EVENT_MIDI_START)
                    {
                        //printf("EVENT_MIDI_START\n");
                        start(global_song_start_mode);
                        m_midiclockrunning = true;
                        m_usemidiclock = true;
                        m_midiclocktick = 0;    // start at beginning of song
                        m_midiclockpos = 0;     // start at beginning of song
                    }
                    // midi continue: start from midi song position
                    // this will be sent immediately after  EVENT_MIDI_SONG_POS
                    // and is used for start from other than beginning of the song,
                    // or to start from previous location at EVENT_MIDI_STOP
                    else if (ev.get_status() == EVENT_MIDI_CONTINUE)
                    {
                        //printf("EVENT_MIDI_CONTINUE\n");
                        m_midiclockrunning = true;
                        start(global_song_start_mode);
                    }
                    // should hold the stop position in case the next event is continue
                    else if (ev.get_status() == EVENT_MIDI_STOP)
                    {
                        //printf("EVENT_MIDI_STOP\n");
                        m_midiclockrunning = false;
                        all_notes_off();
                        inner_stop(true);        // true = m_usemidiclock = true, i.e. hold m_tick position(output_func)
                        m_midiclockpos = m_tick; // set position to last location on stop - for continue
                    }
                    else if (ev.get_status() == EVENT_MIDI_CLOCK)
                    {
                        //printf("EVENT_MIDI_CLOCK - m_tick [%ld] \n", m_tick);
                        if (m_midiclockrunning)
                            m_midiclocktick += 8;
                    }
                    else if (ev.get_status() == EVENT_MIDI_SONG_POS)
                    {
                        unsigned char a, b;
                        ev.get_data(&a, &b);

                        m_midiclockpos = combine_bytes(a,b);

                        /*
                            http://www.blitter.com/~russtopia/MIDI/~jglatt/tech/midispec/ssp.htm

                            Example: If a Song Position value of 8 is received,
                            then a sequencer (or drum box) should cue playback to the
                            third quarter note of the song.
                            (8 MIDI beats * 6 MIDI clocks per MIDI beat = 48 MIDI Clocks.
                            Since there are 24 MIDI Clocks in a quarter note,
                            the first quarter occurs on a time of 0 MIDI Clocks,
                            the second quarter note occurs upon the 24th MIDI Clock,
                            and the third quarter note occurs on the 48th MIDI Clock).
                         */

                        m_midiclockpos *= 48;   // 8 MIDI beats * 6 MIDI clocks per MIDI beat = 48 MIDI Clocks.
                        //printf("EVENT_MIDI_SONG_POS - m_midiclockpos[%ld]\n", m_midiclockpos);
                    }

                    /* filter system wide messages */
                    if (ev.get_status() <= EVENT_SYSEX)
                    {
                        if( global_showmidi)
                            ev.print();

                        /* is there at least one sequence set ? */
                        if (m_master_bus.is_dumping())
                        {
                            ev.set_timestamp(m_tick);

                            /* dump to it - possibly multiple sequences set */
                            m_master_bus.dump_midi_input(ev);
                        }

                        /* use it to control our sequencer */
                        else
                        {
                            for (int i = 0; i < c_midi_controls; i++)
                            {
                                unsigned char data[2] = {0,0};
                                unsigned char status = ev.get_status();

                                ev.get_data( &data[0], &data[1] );

                                if (get_midi_control_toggle(i)->m_active &&
                                        status  == get_midi_control_toggle(i)->m_status &&
                                        data[0] == get_midi_control_toggle(i)->m_data )
                                {
                                    if (data[1] >= get_midi_control_toggle(i)->m_min_value &&
                                            data[1] <= get_midi_control_toggle(i)->m_max_value )
                                    {
                                        if ( i <  c_seqs_in_set )
                                            sequence_playing_toggle( i + m_offset );
                                    }
                                }

                                if ( get_midi_control_on(i)->m_active &&
                                        status  == get_midi_control_on(i)->m_status &&
                                        data[0] == get_midi_control_on(i)->m_data )
                                {
                                    if ( data[1] >= get_midi_control_on(i)->m_min_value &&
                                            data[1] <= get_midi_control_on(i)->m_max_value )
                                    {
                                        if ( i <  c_seqs_in_set )
                                            sequence_playing_on( i  + m_offset);
                                        else
                                            handle_midi_control( i, true );

                                    }
                                    else if (  get_midi_control_on(i)->m_inverse_active )
                                    {
                                        if ( i <  c_seqs_in_set )
                                            sequence_playing_off(  i + m_offset );
                                        else
                                            handle_midi_control( i, false );
                                    }
                                }

                                if ( get_midi_control_off(i)->m_active &&
                                        status  == get_midi_control_off(i)->m_status &&
                                        data[0] == get_midi_control_off(i)->m_data )
                                {
                                    if ( data[1] >= get_midi_control_off(i)->m_min_value &&
                                            data[1] <= get_midi_control_off(i)->m_max_value )
                                    {
                                        if ( i <  c_seqs_in_set )
                                            sequence_playing_off(  i + m_offset );
                                        else
                                            handle_midi_control( i, false );
                                    }
                                    else if ( get_midi_control_off(i)->m_inverse_active )
                                    {
                                        if ( i <  c_seqs_in_set )
                                            sequence_playing_on(  i + m_offset );
                                        else
                                            handle_midi_control( i, true );
                                    }
                                }
                            }
                        }
                    }

                    if (ev.get_status() == EVENT_SYSEX)
                    {
                        if (global_showmidi)
                            ev.print();

                        if (global_pass_sysex)
                            m_master_bus.sysex(&ev);
                    }
                }
            }
            while (m_master_bus.is_more_input());
        }
    }
    pthread_exit(0);
}

/*
    http://www.blitter.com/~russtopia/MIDI/~jglatt/tech/midispec/wheel.htm
    Two data bytes follow the status. The two bytes should be combined together
    to form a 14-bit value. The first data byte's bits 0 to 6 are bits 0 to 6 of
    the 14-bit value. The second data byte's bits 0 to 6 are really bits 7 to 13
    of the 14-bit value. In other words, assuming that a C program has the first
    byte in the variable First and the second data byte in the variable Second,
    here's how to combine them into a 14-bit value (actually 16-bit since most
    computer CPUs deal with 16-bit, not 14-bit, integers):
*/
unsigned short perform::combine_bytes(unsigned char First, unsigned char Second)
{
   unsigned short _14bit;
   _14bit = (unsigned short)Second;
   _14bit <<= 7;
   _14bit |= (unsigned short)First;
   return(_14bit);
}

void perform::save_playing_state()
{
    for( int i=0; i<c_total_seqs; i++ )
    {
        if ( is_active(i) == true )
        {
            assert( m_seqs[i] );
            m_sequence_state[i] = m_seqs[i]->get_playing();
        }
        else
            m_sequence_state[i] = false;
    }
}

void perform::restore_playing_state()
{
    for( int i=0; i<c_total_seqs; i++ )
    {
        if ( is_active(i) == true )
        {
            assert( m_seqs[i] );
            m_seqs[i]->set_playing( m_sequence_state[i] );
        }
    }
}

void perform::set_sequence_control_status( int a_status )
{
    if ( a_status & c_status_snapshot )
    {
        save_playing_state(  );
    }

    m_control_status |= a_status;
}

void perform::unset_sequence_control_status( int a_status )
{
    if ( a_status & c_status_snapshot )
    {
        restore_playing_state(  );
    }

    m_control_status &= (~a_status);
}

void perform::sequence_playing_toggle( int a_sequence )
{
    if ( is_active(a_sequence) == true )
    {
        assert( m_seqs[a_sequence] );

        if ( m_control_status & c_status_queue )
        {
            m_seqs[a_sequence]->toggle_queued();
        }
        else
        {
            if (  m_control_status & c_status_replace )
            {
                unset_sequence_control_status( c_status_replace );
                off_sequences( );
            }

            m_seqs[a_sequence]->toggle_playing();
        }
    }
}

void perform::sequence_playing_on( int a_sequence )
{
    if ( is_active(a_sequence) == true )
    {
        if (m_mode_group && (m_playing_screen == m_screen_set)
                && (a_sequence >= (m_playing_screen * c_seqs_in_set))
                && (a_sequence < ((m_playing_screen + 1) * c_seqs_in_set)))
            m_tracks_mute_state[a_sequence - m_playing_screen * c_seqs_in_set] = true;
        assert( m_seqs[a_sequence] );
        if (!(m_seqs[a_sequence]->get_playing()))
        {
            if (m_control_status & c_status_queue )
            {
                if (!(m_seqs[a_sequence]->get_queued()))
                    m_seqs[a_sequence]->toggle_queued();
            }
            else
                m_seqs[a_sequence]->set_playing(true);
        }
        else
        {
            if ((m_seqs[a_sequence]->get_queued()) && (m_control_status & c_status_queue ))
                m_seqs[a_sequence]->toggle_queued();
        }
    }
}

void perform::sequence_playing_off( int a_sequence )
{
    if ( is_active(a_sequence) == true )
    {
        if (m_mode_group && (m_playing_screen == m_screen_set)
                && (a_sequence >= (m_playing_screen * c_seqs_in_set))
                && (a_sequence < ((m_playing_screen + 1) * c_seqs_in_set)))
            m_tracks_mute_state[a_sequence - m_playing_screen * c_seqs_in_set] = false;
        assert( m_seqs[a_sequence] );
        if (m_seqs[a_sequence]->get_playing())
        {
            if (m_control_status & c_status_queue )
            {
                if (!(m_seqs[a_sequence]->get_queued()))
                    m_seqs[a_sequence]->toggle_queued();
            }
            else
                m_seqs[a_sequence]->set_playing(false);
        }
        else
        {
            if ((m_seqs[a_sequence]->get_queued()) && (m_control_status & c_status_queue ))
                m_seqs[a_sequence]->toggle_queued();
        }
    }
}

void perform::set_key_event( unsigned int keycode, long sequence_slot )
{
    // unhook previous binding...
    std::map<unsigned int,long>::iterator it1 = key_events.find( keycode );
    if (it1 != key_events.end())
    {
        std::map<long,unsigned int>::iterator i = key_events_rev.find( it1->second );
        if (i != key_events_rev.end())
            key_events_rev.erase( i );
        key_events.erase( it1 );
    }
    std::map<long,unsigned int>::iterator it2 = key_events_rev.find( sequence_slot );
    if (it2 != key_events_rev.end())
    {
        std::map<unsigned int,long>::iterator i = key_events.find( it2->second );
        if (i != key_events.end())
            key_events.erase( i );
        key_events_rev.erase( it2 );
    }
    // set
    key_events[keycode] = sequence_slot;
    key_events_rev[sequence_slot] = keycode;
}

void perform::set_key_group( unsigned int keycode, long group_slot )
{
    // unhook previous binding...
    std::map<unsigned int,long>::iterator it1 = key_groups.find( keycode );
    if (it1 != key_groups.end())
    {
        std::map<long,unsigned int>::iterator i = key_groups_rev.find( it1->second );
        if (i != key_groups_rev.end())
            key_groups_rev.erase( i );
        key_groups.erase( it1 );
    }
    std::map<long,unsigned int>::iterator it2 = key_groups_rev.find( group_slot );
    if (it2 != key_groups_rev.end())
    {
        std::map<unsigned int,long>::iterator i = key_groups.find( it2->second );
        if (i != key_groups.end())
            key_groups.erase( i );
        key_groups_rev.erase( it2 );
    }
    // set
    key_groups[keycode] = group_slot;
    key_groups_rev[group_slot] = keycode;
}

#ifdef JACK_SUPPORT

/*  called by jack_timebase_callback() & position_jack()>(debug)  */
void perform::jack_BBT_position(jack_position_t &pos, double jack_tick)
{
    pos.valid = JackPositionBBT;
    pos.beats_per_bar = m_bp_measure;
    pos.beat_type = m_bw;
    /* these are set in the timebase callback since they are needed for jack_tick */
    //pos.ticks_per_beat = c_ppqn * 10; // 192 * 10 = 1920
    //pos.beats_per_minute =  m_master_bus.get_bpm();

    pos.frame_rate =  m_jack_frame_rate; // comes from init_jack()

    /* Compute BBT info from frame number.  This is relatively
     * simple here, but would become complex if we supported tempo
     * or time signature changes at specific locations in the
     * transport timeline. */

    long ptick = 0, pbeat = 0, pbar = 0;
    pbar  = (long) ((long) jack_tick / (pos.ticks_per_beat *  pos.beats_per_bar ));
    pbeat = (long) ((long) jack_tick % (long) (pos.ticks_per_beat *  pos.beats_per_bar ));
    pbeat = pbeat / (long) pos.ticks_per_beat;
    ptick = (long) jack_tick % (long) pos.ticks_per_beat;

    pos.bar = pbar + 1;
    pos.beat = pbeat + 1;
    pos.tick = ptick;
    pos.bar_start_tick = pos.bar * pos.beats_per_bar *
                          pos.ticks_per_beat;

    //printf( " bbb [%2d:%2d:%4d] jack_tick [%f]\n", pos.bar, pos.beat, pos.tick, jack_tick );
}

/*
    This callback is only called by jack when seq32 is Master and is used to supply jack
    with BBT information based on frame position and frame_rate.
*/
void jack_timebase_callback(jack_transport_state_t state,
                            jack_nframes_t nframes,
                            jack_position_t *pos, int new_pos, void *arg)
{
    double jack_tick;
    jack_nframes_t current_frame;

    perform *p = (perform *) arg;
    current_frame = jack_get_current_transport_frame( p->m_jack_client );

    pos->beats_per_minute = p->get_bpm();
    pos->ticks_per_beat = c_ppqn * 10;

    //printf( "jack_timebase_callback() [%d] [%d] [%d]\n", state, new_pos, current_frame);

    jack_tick =
        (current_frame) *
        pos->ticks_per_beat  *
        pos->beats_per_minute / ( p->m_jack_frame_rate* 60.0);

    p->jack_BBT_position(*pos, jack_tick);
}

long get_current_jack_position(void *arg)
{
    perform *p = (perform *) arg;
    jack_nframes_t current_frame;
    double jack_tick;
    double ticks_per_beat = c_ppqn * 10; // 192 * 10 = 1920
    double beats_per_minute =  p->get_bpm();
    double beat_type = p->get_bw();

    current_frame = jack_get_current_transport_frame( p->m_jack_client );

    jack_tick =
        (current_frame) *
        ticks_per_beat  *
        beats_per_minute / ( p->m_jack_frame_rate* 60.0);


    /* convert ticks */
    return jack_tick * ((double) c_ppqn /
                    (ticks_per_beat *
                     beat_type / 4.0  ));
}

void jack_shutdown(void *arg)
{
    perform *p = (perform *) arg;
    p->m_jack_running = false;

    printf("JACK shut down.\nJACK sync Disabled.\n");
}

void print_jack_pos( jack_position_t* jack_pos )
{
    return;
    printf( "print_jack_pos()\n" );
    printf( "    bar  [%d]\n", jack_pos->bar  );
    printf( "    beat [%d]\n", jack_pos->beat );
    printf( "    tick [%d]\n", jack_pos->tick );
    printf( "    bar_start_tick   [%lf]\n", jack_pos->bar_start_tick );
    printf( "    beats_per_bar    [%f]\n", jack_pos->beats_per_bar );
    printf( "    beat_type        [%f]\n", jack_pos->beat_type );
    printf( "    ticks_per_beat   [%lf]\n", jack_pos->ticks_per_beat );
    printf( "    beats_per_minute [%lf]\n", jack_pos->beats_per_minute );
    printf( "    frame_time       [%lf]\n", jack_pos->frame_time );
    printf( "    next_time        [%lf]\n", jack_pos->next_time );
}

#if 0

int main ()
{
    jack_client_t *client;

    /* become a new client of the JACK server */
    if ((client = jack_client_new("transport tester")) == 0)
    {
        fprintf(stderr, "jack server not running?\n");
        return 1;
    }

    jack_on_shutdown(client, jack_shutdown, 0);
    jack_set_sync_callback(client, jack_sync_callback, NULL);

    if (jack_activate(client))
    {
        fprintf(stderr, "cannot activate client");
        return 1;
    }

    bool cond = false; /* true if we want to fail if there is already a master */
    if (jack_set_timebase_callback(client, cond, timebase, NULL) != 0)
    {
        printf("Unable to take over timebase or there is already a master.\n");
        exit(1);
    }

    jack_position_t pos;

    pos.valid = JackPositionBBT;

    pos.bar = 0;
    pos.beat = 0;
    pos.tick = 0;

    pos.beats_per_bar = time_beats_per_bar;
    pos.beat_type = time_beat_type;
    pos.ticks_per_beat = time_ticks_per_beat;
    pos.beats_per_minute = time_beats_per_minute;
    pos.bar_start_tick = 0.0;

    //jack_transport_reposition( client, &pos );

    jack_transport_start (client);

    //void jack_transport_stop (jack_client_t *client);

    int bob;
    scanf ("%d", &bob);

    jack_transport_stop (client);
    jack_release_timebase(client);
    jack_client_close(client);

    return 0;
}

#endif // 0
#endif // JACK_SUPPORT

bool
perform::sequence_is_song_exportable(int a_seq)
{
    if (is_active(a_seq) &&                                 // active?
        get_sequence(a_seq)->get_trigger_count() > 0 &&     // does it have triggers?
        !get_sequence(a_seq)->get_song_mute())              // is it NOT muted?
    {
        return true;                                        // then it is good.
    }
    return false;                                           // somewhere we failed above
}


void
perform::apply_song_transpose()
{
    for (int i=0; i< c_max_sequence; i++ )
    {
        if ( is_active(i) )
        {
            get_sequence(i)->apply_song_transpose();
        }
    }
}
