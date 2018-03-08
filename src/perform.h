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

#pragma once

class perform;

#include <vector>
#include "globals.h"
#include "event.h"
#include "midibus.h"
#include "midifile.h"
#include "sequence.h"
#ifndef __WIN32__
#   include <unistd.h>
#endif
#include <pthread.h>

/* if we have jack, include the jack headers */
#ifdef JACK_SUPPORT
#include <jack/jack.h>
#include <jack/transport.h>
#ifdef JACK_SESSION
#include <jack/session.h>
#endif // JACK_SESSION
#endif // JACK_SUPPORT

#undef USE_JACK_BBT_POSITION                // old code could be used for debug

/* class contains sequences that make up a live set */

class midi_control
{
public:

    bool m_active;
    bool m_inverse_active;
    long m_status;
    long m_data;
    long m_min_value;
    long m_max_value;
};

enum mute_op
{
    MUTE_TOGGLE = -1,
    MUTE_OFF = 0,
    MUTE_ON = 1
};

enum ff_rw_type_e
{
    FF_RW_REWIND    = -1,
    FF_RW_RELEASE   =  0,
    FF_RW_FORWARD   =  1
};

struct tempo_mark
{
    uint64_t tick;
    double bpm;
    uint32_t bw;            // not used
    uint32_t bp_measure;    // not used
    uint32_t start;         // calculated frame offset start - jack_nframes_t
    double microseconds_start; // calculated offset for clock display
    
    tempo_mark ( ) : tick ( 0 ), bpm ( 0.0 ), bw ( 0 ), bp_measure ( 0 ), start ( 0 ), microseconds_start( 0.0 )
        {
        }
};

#ifdef JACK_SUPPORT
/*  Bar and beat start at 1. */
struct BBT
{
    unsigned short bar;
    unsigned char beat;
    unsigned short tick;

    BBT ( ) : bar( 0 ), beat( 0 ), tick( 0 )
        {
        }
};


struct position_info
{
    jack_nframes_t frame;

    float tempo;
    int beats_per_bar;
    int beat_type;

    BBT bbt;
};

struct time_sig
{
    int beats_per_bar;
    int beat_type;

    time_sig ( ) : beats_per_bar( 0 ), beat_type( 0 )
        {
        }

    time_sig ( int bpb, int note ) : beats_per_bar( bpb ), beat_type( note )
        {
        }
};
#endif // JACK_SUPPORT

#define STOP_MARKER         0.0
#define STARTING_MARKER     0

const int c_status_replace  = 0x01;
const int c_status_snapshot = 0x02;
const int c_status_queue    = 0x04;

const int c_midi_track_ctrl = c_seqs_in_set * 2;
const int c_midi_control_bpm_up       = c_midi_track_ctrl ;
const int c_midi_control_bpm_dn       = c_midi_track_ctrl + 1;
const int c_midi_control_ss_up        = c_midi_track_ctrl + 2;
const int c_midi_control_ss_dn        = c_midi_track_ctrl + 3;
const int c_midi_control_mod_replace  = c_midi_track_ctrl + 4;
const int c_midi_control_mod_snapshot = c_midi_track_ctrl + 5;
const int c_midi_control_mod_queue    = c_midi_track_ctrl + 6;
//andy midi_control_mod_mute_group
const int c_midi_control_mod_gmute    = c_midi_track_ctrl + 7;
//andy learn_mute_toggle_mode
const int c_midi_control_mod_glearn   = c_midi_track_ctrl + 8;
//andy play only this screen set
const int c_midi_control_play_ss      = c_midi_track_ctrl + 9;
const int c_midi_controls             = c_midi_track_ctrl + 10;//7

struct performcallback
{
    virtual void on_grouplearnchange(bool state) {}
};

class perform
{
public:

    //Playlist mode
    void        set_playlist_mode(bool mode);
    bool        get_playlist_mode();
    void        set_playlist_file(const Glib::ustring& fn);
    Glib::ustring get_playlist_current_file();
    int         get_playlist_index();
    bool        set_playlist_index(int index);

    unsigned int        m_key_playlist_next;
    unsigned int        m_key_playlist_prev;
    int                 m_setjump;              // perfedit messages to mainwnd timeout
    bool                m_playlist_stop_mark;
    // end playlist public
private:

    //Playlist mode
    bool m_playlist_mode;
    Glib::ustring m_playlist_file;
    int m_playlist_nfiles;
    int m_playlist_current_idx;
    std::vector<Glib::ustring> m_playlist_fileset;
    // end playlist private

    //andy mute group
    bool m_mute_group[c_gmute_tracks];
    bool m_tracks_mute_state[c_seqs_in_set];
    bool m_mode_group;
    bool m_mode_group_learn;
    int m_mute_group_selected;
    //andy playing screen
    int m_playing_screen;

    /* vector of sequences */
    sequence *m_seqs[c_max_sequence];
    
    bool m_seqs_active[ c_max_sequence ];

    bool m_was_active_main[ c_max_sequence ];
    bool m_was_active_edit[ c_max_sequence ];
    bool m_was_active_perf[ c_max_sequence ];
    bool m_was_active_names[ c_max_sequence ];

    bool m_sequence_state[  c_max_sequence ];

    /* used for undo/redo track number */
    vector<int> undo_vect;
    vector<int> redo_vect;

    /* our midibus */
    mastermidibus m_master_bus;

    /* pthread info */
    pthread_t m_out_thread;
    pthread_t m_in_thread;
    bool m_out_thread_launched;
    bool m_in_thread_launched;

    bool m_inputing;
    bool m_outputing;
    bool m_looping;
    bool m_reposition;

    bool m_playback_mode;
    bool m_follow_transport;
    bool m_start_from_perfedit;

    int thread_trigger_width_ms;

    long m_left_tick;
    long m_right_tick;
    long m_starting_tick;

    long m_tick;
    bool m_usemidiclock;
    bool m_midiclockrunning; // stopped or started
    int  m_midiclocktick;
    long m_midiclockpos;

    int m_bp_measure;
    int m_bw;

    bool m_show_ui_sequence_key;

    void set_playback_mode( bool a_playback_mode );

    string m_screen_set_notepad[c_max_sets];

    midi_control m_midi_cc_toggle[ c_midi_controls ];
    midi_control m_midi_cc_on[ c_midi_controls ];
    midi_control m_midi_cc_off[ c_midi_controls ];

    int m_offset;
    int m_control_status;
    int m_screen_set;

    condition_var m_condition_var;

    // do not access these directly, use set/lookup below
    std::map<unsigned int,long> key_events;
    std::map<unsigned int,long> key_groups;
    std::map<long,unsigned int> key_events_rev; // reverse lookup, keep this in sync!!
    std::map<long,unsigned int> key_groups_rev; // reverse lookup, keep this in sync!!

#ifdef JACK_SUPPORT

    jack_client_t *m_jack_client;
    jack_nframes_t m_jack_frame_current,
                   m_jack_frame_last,
                   m_jack_frame_rate;
    jack_position_t m_jack_pos;
    jack_transport_state_t m_jack_transport_state;
    jack_transport_state_t m_jack_transport_state_last;
    double m_jack_tick;
#ifdef JACK_SESSION
public:
    jack_session_event_t *m_jsession_ev;
    bool jack_session_event();
private:
#endif // JACK_SESSION
#endif // JACK_SUPPORT

    bool m_jack_running;
    bool m_toggle_jack;
    bool m_jack_master;
    long m_jack_stop_tick;
    
    /* Allow continue on stop */
    bool m_continue;
    
    /* For midi control to update start bpm of perfedit tempo mark */
    bool m_update_perf_edit_tempo_markers;
    
    /**
     *  Holds a few MIDI file-names most recently used.  Although this is a
     *  vector, we do not let it grow past c_max_recent_files.
     *
     *  New feature from Oli Kester's kepler34 project via sequencer64
     */

    std::vector<std::string> m_recent_files;


    void inner_start( bool a_state );
    void inner_stop(bool a_midi_clock = false);

public:
    bool is_learn_mode() const
    {
        return m_mode_group_learn;
    }
    
    void set_update_perfedit_markers(bool a_set)
    {
        m_update_perf_edit_tempo_markers = a_set;
    }
    
    bool get_update_perfedit_markers()
    {
        return m_update_perf_edit_tempo_markers;
    }

    // can register here for events...
    std::vector<performcallback*> m_notify;
    
    /* m_list_play_marker is used to trigger bpm or stops when running in play().
     * As each marker is encountered, it's value is used, then it is erased.
     * It is reset at stop, or when any new marker is set or removed by user.
     * Reset value = m_list_marker in the tempo() class. */
    list < tempo_mark > m_list_play_marker;
    
    /* m_list_total_marker contains all markers including stops.
     * Should always = m_list_marker in the tempo() class.
     * Adjusted when new marker is set or removed by user or
     * from file loading. Updated from the tempo class function,
     * reset_tempo_list() and perform set_start_tempo(). */
    list < tempo_mark > m_list_total_marker;
    
    /* m_list_no_stop_markers contains only playing markers (no stops).
     * It is used by the render_tempomap() (jack) when playing and position_jack().
     * Also used in output_func() when jack is running.
     * Only adjusted when new marker is set or removed by user */
    list < tempo_mark > m_list_no_stop_markers;
    
    /* m_list_file_load_marker is used exclusively for file loading to
     * update the tempo class tempo map > m_list_marker. The items loaded
     * come from the midifile class, parse() function. */
    list < tempo_mark > m_list_file_load_marker;

    /* for undo/redo */
    stack < list < tempo_mark > >m_list_undo;
    stack < list < tempo_mark > >m_list_redo;


    unsigned int m_key_bpm_up;
    unsigned int m_key_bpm_dn;
    unsigned int m_key_tap_bpm;

    unsigned int m_key_replace;
    unsigned int m_key_queue;
    unsigned int m_key_keep_queue;
    unsigned int m_key_snapshot_1;
    unsigned int m_key_snapshot_2;

    unsigned int m_key_screenset_up;
    unsigned int m_key_screenset_dn;
    unsigned int m_key_set_playing_screenset;

    unsigned int m_key_group_on;
    unsigned int m_key_group_off;
    unsigned int m_key_group_learn;

    unsigned int m_key_start;
    unsigned int m_key_stop;
    unsigned int m_key_forward;
    unsigned int m_key_rewind;
    unsigned int m_key_pointer;

    unsigned int m_key_song;
    unsigned int m_key_jack;
    unsigned int m_key_menu;
    unsigned int m_key_follow_trans;
    unsigned int m_key_export_trigger;

    bool show_ui_sequence_key() const
    {
        return m_show_ui_sequence_key;
    }
    
    perform();
    ~perform();

    void start_playing();
    void stop_playing();
    void set_continue(bool a_set);

    void FF_rewind();

    void set_start_from_perfedit(bool a_start);
    bool get_start_from_perfedit();
    void toggle_song_mode();
    void toggle_jack_mode();
    void set_jack_mode(bool a_mode);
    bool get_toggle_jack();
    bool is_jack_running();
    bool is_jack_master();

    void set_follow_transport(bool a_set);
    bool get_follow_transport();
    void toggle_follow_transport();

    void init();

    bool clear_all();

    void launch_input_thread();
    void launch_output_thread();
    void init_jack();
    void deinit_jack();

    void add_sequence( sequence *a_seq, int a_perf );
    void delete_sequence( int a_num );
    bool is_sequence_in_edit( int a_num );

    void clear_sequence_triggers( int a_seq  );

    long get_tick( )
    {
        return m_tick;
    };

    void set_jack_stop_tick(long a_tick);
    long get_jack_stop_tick( )
    {
        return m_jack_stop_tick;
    };

    void set_left_tick( long a_tick );
    long get_left_tick();

    void set_starting_tick( long a_tick );
    long get_starting_tick();

    void set_right_tick( long a_tick );
    long get_right_tick();

    void move_triggers( bool a_direction );
    void copy_triggers(  );

    void push_trigger_undo(int a_track);
    void pop_trigger_undo();
    void pop_trigger_redo();
    void set_have_undo(bool a_undo);
    void set_have_redo(bool a_redo);
    bool m_have_undo;
    bool m_have_redo;

    float m_excell_FF_RW;

    void print();
    void error_message_gtk( Glib::ustring message);

    midi_control *get_midi_control_toggle( unsigned int a_seq );
    midi_control *get_midi_control_on( unsigned int a_seq );
    midi_control *get_midi_control_off( unsigned int a_seq );

    void handle_midi_control( int a_control, bool a_state );

    void set_screen_set_notepad( int a_screen_set, string *a_note );
    string *get_screen_set_notepad( int a_screen_set );

    void set_screenset( int a_ss );
    int get_screenset();
    void set_playing_screenset();
    int get_playing_screenset();
    void mute_group_tracks ();
    void select_and_mute_group (int a_g_group);
    void set_mode_group_mute ();
    void select_group_mute (int a_g_mute);
    void set_mode_group_learn ();
    void unset_mode_group_learn ();
    bool is_group_learning(void)
    {
        return m_mode_group_learn;
    }
    void select_mute_group ( int a_group );
    void unset_mode_group_mute ();
    void start( bool a_state );
    void stop();
 
    void reset_tempo_play_marker_list();
    double get_start_tempo();
    
    void start_jack();
    void stop_jack();
    void position_jack( bool a_state, long a_tick );

    void off_sequences();
    void all_notes_off();

    void set_active(int a_sequence, bool a_active);
    void set_was_active( int a_sequence );
    bool is_active(int a_sequence);
    bool is_dirty_main (int a_sequence);
    bool is_dirty_edit (int a_sequence);
    bool is_dirty_perf (int a_sequence);
    bool is_dirty_names (int a_sequence);

    void new_sequence( int a_sequence );

    /* plays all notes to Curent tick */
    void play( long a_tick );
    void set_orig_ticks( long a_tick  );
    
    void tempo_change();

    sequence * get_sequence( int a_sequence );

    void reset_sequences();

    void set_bpm(double a_bpm);
    double  get_bpm( );

    void set_bp_measure(int a_bp_mes);
    int get_bp_measure( );

    void set_bw(int a_bw);
    int get_bw( );

    void set_looping( bool a_looping )
    {
        m_looping = a_looping;
    };

    void set_reposition(bool a_pos_type = true);
    bool get_reposition();

    void set_sequence_control_status( int a_status );
    void unset_sequence_control_status( int a_status );

    void sequence_playing_toggle( int a_sequence );
    void sequence_playing_on( int a_sequence );
    void sequence_playing_off( int a_sequence );
    void set_group_mute_state (int a_g_track, bool a_mute_state);
    bool get_group_mute_state (int a_g_track);

    void set_song_mute( mute_op op  );

    mastermidibus* get_master_midi_bus( );

    void output_func();
    void input_func();

    unsigned short combine_bytes(unsigned char First, unsigned char Second);
    void parse_sysex(event a_e);

    long get_max_trigger();

    void set_offset( int a_offset );

    void save_playing_state();
    void restore_playing_state();

    const std::map<unsigned int,long> *get_key_events(void) const
    {
        return &key_events;
    };
    const std::map<unsigned int,long> *get_key_groups(void) const
    {
        return &key_groups;
    };

    void set_key_event( unsigned int keycode, long sequence_slot );
    void set_key_group( unsigned int keycode, long group_slot );

    // getters of keyboard mapping for sequence and groups,
    // if not found, returns something "safe" (so use get_key()->count() to see if it's there first)
    unsigned int lookup_keyevent_key( long seqnum )
    {
        if (key_events_rev.count( seqnum )) return key_events_rev[seqnum];
        else return '?';
    }
    long lookup_keyevent_seq( unsigned int keycode )
    {
        if (key_events.count( keycode )) return key_events[keycode];
        else return 0;
    }
    unsigned int lookup_keygroup_key( long groupnum )
    {
        if (key_groups_rev.count( groupnum )) return key_groups_rev[groupnum];
        else return '?';
    }
    long lookup_keygroup_group( unsigned int keycode )
    {
        if (key_groups.count( keycode )) return key_groups[keycode];
        else return 0;
    }

    friend class midifile;
    friend class optionsfile;
    friend class options;

    bool sequence_is_song_exportable(int a_seq);
    void apply_song_transpose ();
    
    std::string recent_file (int index, bool shorten = true) const;
    void add_recent_file (const std::string & filename);
    /**
     * \getter m_recent_files.size()
     */

    int recent_file_count () const
    {
        return int(m_recent_files.size());
    }

#ifdef JACK_SUPPORT
#ifdef USE_JACK_BBT_POSITION
    void jack_BBT_position(jack_position_t &pos, double jack_tick);
    friend int jack_sync_callback(jack_transport_state_t state,
                              jack_position_t *pos, void *arg);
#endif // USE_JACK_BBT_POSITION
    friend position_info solve_tempomap ( jack_nframes_t frame, void *arg );
    friend position_info render_tempomap( jack_nframes_t start, jack_nframes_t length, void *cb, void *arg );
    friend jack_nframes_t tick_to_jack_frame(uint64_t a_tick, double a_bpm, void *arg);
    friend long convert_jack_frame_to_s32_tick(jack_nframes_t a_frame, double a_bpm, void *arg);

    friend void jack_shutdown(void *arg);
    friend void jack_timebase_callback(jack_transport_state_t state, jack_nframes_t nframes,
                                       jack_position_t *pos, int new_pos, void *arg);
    friend int jack_process_callback(jack_nframes_t nframes, void* arg);
    friend long get_current_jack_position(jack_nframes_t a_frame, void *arg);
#endif // JACK_SUPPORT
};

/* located in perform.C */
extern void *output_thread_func(void *a_p);
extern void *input_thread_func(void *a_p);

/* located in perfedit.h */
extern ff_rw_type_e FF_RW_button_type;

#ifdef JACK_SUPPORT
#ifdef USE_JACK_BBT_POSITION
int jack_sync_callback(jack_transport_state_t state,
                       jack_position_t *pos, void *arg);
#endif // USE_JACK_BBT_POSITION

position_info solve_tempomap ( jack_nframes_t frame, void *arg );
position_info render_tempomap( jack_nframes_t start, jack_nframes_t length, void *cb, void *arg );
jack_nframes_t tick_to_jack_frame(uint64_t a_tick, double a_bpm, void *arg);
long convert_jack_frame_to_s32_tick(jack_nframes_t a_frame, double a_bpm, void *arg);

void print_jack_pos( jack_position_t* jack_pos );
void jack_shutdown(void *arg);
void jack_timebase_callback(jack_transport_state_t state, jack_nframes_t nframes,
                            jack_position_t *pos, int new_pos, void *arg);
int jack_process_callback(jack_nframes_t nframes, void* arg);
long get_current_jack_position(jack_nframes_t a_frame, void *arg);
#ifdef JACK_SESSION
void jack_session_callback(jack_session_event_t *ev, void *arg);
#endif // JACK_SESSION
#endif // JACK_SUPPORT
