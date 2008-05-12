//----------------------------------------------------------------------------
//
//  This file is part of seq24.
//
//  seq24 is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  seq24 is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with seq24; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//-----------------------------------------------------------------------------

#ifndef SEQ24_PERFORM
#define SEQ24_PERFORM

class perform;

#include "globals.h"
#include "event.h"
#include "midibus.h"
#include "midifile.h"
#include "sequence.h"
#include <unistd.h>
#include <pthread.h>


/* if we have jack, include the jack headers */
#ifdef JACK_SUPPORT
#include <jack/jack.h>
#include <jack/transport.h>
#endif


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

const int c_status_replace  = 0x01;
const int c_status_snapshot = 0x02;
const int c_status_queue    = 0x04;

const int c_midi_control_bpm_up       = c_seqs_in_set ;
const int c_midi_control_bpm_dn       = c_seqs_in_set + 1;
const int c_midi_control_ss_up        = c_seqs_in_set + 2;
const int c_midi_control_ss_dn        = c_seqs_in_set + 3;
const int c_midi_control_mod_replace  = c_seqs_in_set + 4;
const int c_midi_control_mod_snapshot = c_seqs_in_set + 5;
const int c_midi_control_mod_queue    = c_seqs_in_set + 6;
const int c_midi_controls             = c_seqs_in_set + 7;

class perform
{
 private:

    /* vector of sequences */
    sequence *m_seqs[c_max_sequence];

    bool m_seqs_active[ c_max_sequence ];

    bool m_was_active_main[ c_max_sequence ];
    bool m_was_active_edit[ c_max_sequence ];
    bool m_was_active_perf[ c_max_sequence ];
    bool m_was_active_names[ c_max_sequence ];
    
    bool m_sequence_state[  c_max_sequence ];

    /* our midibus */
    mastermidibus m_master_bus;

    /* pthread info */
    pthread_t m_out_thread;
    pthread_t m_in_thread;
    bool m_out_thread_launched;
    bool m_in_thread_launched;

    bool m_running;
    bool m_inputing;
    bool m_outputing;
    bool m_looping;

    bool m_playback_mode;

    int thread_trigger_width_ms; 

    long m_left_tick;
    long m_right_tick;
    long m_starting_tick;
    
    long m_tick;
   
    void set_running( bool a_running );
    bool is_running();

    void set_playback_mode( bool a_playback_mode );

    string m_screen_set_notepad[c_max_sets];

    midi_control m_midi_cc_toggle[ c_midi_controls ];
    midi_control m_midi_cc_on[ c_midi_controls ];
    midi_control m_midi_cc_off[ c_midi_controls ];
    
    int m_offset;
    int m_control_status;
    int m_screen_set;

    condition_var m_condition_var;

    std::map<long,long> key_events;

#ifdef JACK_SUPPORT
    
    jack_client_t *m_jack_client;
    jack_nframes_t m_jack_frame_current,
                   m_jack_frame_last;
    jack_position_t m_jack_pos;
    jack_transport_state_t m_jack_transport_state;
    jack_transport_state_t m_jack_transport_state_last;
    double m_jack_tick;
    
#endif
    
    bool m_jack_running;
    bool m_jack_master;

    void inner_start( bool a_state );
    void inner_stop();

 public:

    long m_key_bpm_up;
    long m_key_bpm_dn;

    long m_key_replace;
    long m_key_queue;
    long m_key_snapshot_1;
    long m_key_snapshot_2;

    long m_key_screenset_up;
    long m_key_screenset_dn;

    long m_key_start; 
    long m_key_stop;


    perform();
    ~perform();

    void init( void );

    void clear_all( void );
    
    void launch_input_thread( void );
    void launch_output_thread( void );
    void init_jack( void );
    void deinit_jack( void );
    
    void add_sequence( sequence *a_seq, int a_perf );
    void delete_sequence( int a_num );
    bool is_sequence_in_edit( int a_num );
    
    void clear_sequence_triggers( int a_seq  );


    long get_tick( ) { return m_tick; };

    void set_left_tick( long a_tick );
    long get_left_tick( void );

    void set_starting_tick( long a_tick );
    long get_starting_tick( void );

    void set_right_tick( long a_tick );
    long get_right_tick( void );

    void move_triggers( bool a_direction );
    void copy_triggers(  );
    
    void push_trigger_undo( void );
    void pop_trigger_undo( void );

    void print();

    midi_control *get_midi_control_toggle( unsigned int a_seq );
    midi_control *get_midi_control_on( unsigned int a_seq );
    midi_control *get_midi_control_off( unsigned int a_seq );

    void handle_midi_control( int a_control, bool a_state );

    void set_screen_set_notepad( int a_screen_set, string *a_note );
    string *get_screen_set_notepad( int a_screen_set );

    void set_screenset( int a_ss );
    int get_screenset( void );
    
    void start( bool a_state );
    void stop();

    void start_jack();
    void stop_jack();
    void position_jack( bool a_state );

    void off_sequences( void );

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

    sequence * get_sequence( int a_sequence );

    void reset_sequences( void );

    void set_bpm(int a_bpm);
    int  get_bpm( );

    void set_looping( bool a_looping ){ m_looping = a_looping; };
 
    void set_sequence_control_status( int a_status );
    void unset_sequence_control_status( int a_status );

    void sequence_playing_toggle( int a_sequence );
    void sequence_playing_on( int a_sequence );
    void sequence_playing_off( int a_sequence );
    
    void mute_all_tracks( void );

    mastermidibus* get_master_midi_bus( );
    
    void output_func();
    void input_func();
    
    long get_max_trigger( void );

    void set_offset( int a_offset );
    
    void save_playing_state( void );
    void restore_playing_state( void );
    
    
    std::map<long,long> *get_key_events( void ){ return &key_events; };


    friend class midifile;
    friend class optionsfile;

#ifdef JACK_SUPPORT

    friend int jack_sync_callback(jack_transport_state_t state, 
                              jack_position_t *pos, void *arg);
    friend void jack_shutdown(void *arg);
    friend void jack_timebase_callback(jack_transport_state_t state, jack_nframes_t nframes, 
                                       jack_position_t *pos, int new_pos, void *arg);
#endif

};

/* located in perform.C */
extern void *output_thread_func(void *a_p);
extern void *input_thread_func(void *a_p);



#ifdef JACK_SUPPORT

int jack_sync_callback(jack_transport_state_t state, 
					   jack_position_t *pos, void *arg);
void print_jack_pos( jack_position_t* jack_pos );
void jack_shutdown(void *arg);
void jack_timebase_callback(jack_transport_state_t state, jack_nframes_t nframes, 
                            jack_position_t *pos, int new_pos, void *arg);
#endif


#endif
