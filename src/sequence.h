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

class sequence;

#include <string>
#include <list>
#include <stack>

#include "event.h"
#include "midibus.h"
#include "globals.h"
#include "mutex.h"

enum draw_type
{
    DRAW_FIN = 0,
    DRAW_NORMAL_LINKED,
    DRAW_NOTE_ON,
    DRAW_NOTE_OFF
};

enum note_play_state
{
    NOTE_PLAY = 0,
    NOTE_MUTE,
    NOTE_SOLO
};

enum trigger_edit
{
    GROW_START = 0, //grow the start of the trigger
    GROW_END = 1, //grow the end of the trigger
    MOVE = 2 //move the entire trigger block
};


using std::list;

/* used in playback */
class trigger
{
public:

    long m_tick_start;
    long m_tick_end;

    bool m_selected;

    long m_offset;

    trigger ()
    {
        m_tick_start = 0;
        m_tick_end = 0;
        m_offset = 0;
        m_selected = false;
    };

    bool operator< (trigger rhs) const
    {
        if (m_tick_start < rhs.m_tick_start)
            return true;

        return false;
    };
};

class sequence
{

private:

    /* holds the events */
    list < event > m_list_event;
    static list < event > m_list_clipboard;

    list < event > m_list_undo_hold; // lfo and seqdata

    list < trigger > m_list_trigger;
    trigger m_trigger_clipboard;
    trigger *m_export_trigger; 

    stack < list < event > >m_list_undo;
    stack < list < event > >m_list_redo;
    stack < list < trigger > >m_list_trigger_undo;
    stack < list < trigger > >m_list_trigger_redo;

    /* markers */
    list < event >::iterator m_iterator_play;
    list < event >::iterator m_iterator_draw;

    list < trigger >::iterator m_iterator_play_trigger;
    list < trigger >::iterator m_iterator_draw_trigger;

    /* contains the proper midi channel */
    char m_midi_channel;
    char m_bus;

    /* song playback mode solo/mute */
    bool m_song_mute;
    bool m_song_solo;

    bool m_transposable;

    /* polyphonic step edit note counter */
    int m_notes_on;

    /* outputs to sequence to this Bus on midichannel */
    mastermidibus *m_masterbus;

    /* map for noteon, used when muting, to shut off current
       messages */
    int m_playing_notes[c_midi_notes];
    
    /* flags for solo/mute notes */
    bool m_have_solo;
    note_play_state m_mute_solo_notes[c_midi_notes];

    /* states */
    bool m_was_playing;
    bool m_playing;
    bool m_recording;
    bool m_quanized_rec;
    bool m_thru;
    bool m_queued;
    bool m_overwrite_recording;
    
    /* flag to indicate play marker has gone to beginning of sequence on looping */
    bool m_loop_reset;

    bool m_trigger_copied;

    /* flag indicates that contents has changed from
       a recording */
    bool m_dirty_main;
    bool m_dirty_edit;
    bool m_dirty_perf;
    bool m_dirty_names;

    /* anything editing currently ? */
    bool m_editing;
    bool m_raise;

    /* named sequence */
    string m_name;

    /* where were we */
    long m_last_tick;
    long m_queued_tick;

    long m_trigger_offset;

    /* length of sequence in pulses
       should be powers of two in bars */
    long m_length;
    long m_snap_tick;
    long m_unit_measure;

    /* these are just for the editor to mark things
       in correct time */
    //long m_length_measures;
    long m_time_beats_per_measure;
    long m_time_beat_width;
    int m_rec_vol;

    /* locking */
    seq32::mutex m_mutex;

    /* used to idenfity which events are ours in the out queue */
    //unsigned char m_tag;

    /* takes an event this sequence is holding and
       places it on our midibus */
    void put_event_on_bus (event * a_e);

    /* resets the location counters */
    void reset_loop ();

    /* remove all events from sequence */
    void remove_all ();

    /* mutex */
    void lock ();
    void unlock ();

    /* sets m_trigger_offset and wraps it to length */
    void set_trigger_offset (long a_trigger_offset);
    void split_trigger( trigger &trig, long a_split_tick);
    void adjust_trigger_offsets_to_legnth( long a_new_len );
    long adjust_offset( long a_offset );
    void remove( list<event>::iterator i );
    void remove(const event* e );

public:

    sequence ();
    ~sequence ();

    /* seqdata & lfownd hold for undo */
    void set_hold_undo (bool a_hold);
    int get_hold_undo ();

    bool m_have_undo;
    bool m_have_redo;

    void push_undo (bool a_hold = false);
    void pop_undo ();
    void pop_redo ();

    void push_trigger_undo ();
    void pop_trigger_undo ();
    void pop_trigger_redo ();

    //
    //  Gets and Sets
    //

    void set_have_undo();
    void set_have_redo();

    /* name */
    void set_name (const string &a_name);
    void set_name (char *a_name);

    void set_unit_measure ();
    long get_unit_measure ();

    void set_bp_measure (long a_beats_per_measure);
    long get_bp_measure ();

    void set_bw (long a_beat_width);
    long get_bw ();
    void set_rec_vol (int a_rec_vol);

    void set_song_mute (bool a_mute);
    bool get_song_mute ();
    
    void set_song_solo (bool a_solo);
    bool get_song_solo ();

    void set_transposable (bool a_xpose);
    bool get_transposable ();

    /* returns string of name */
    const char *get_name ();

    void set_editing (bool a_edit)
    {
        m_editing = a_edit;
    };
    bool get_editing ()
    {
        return m_editing;
    };
    void set_raise (bool a_edit)
    {
        m_raise = a_edit;
    };
    bool get_raise ()
    {
        return m_raise;
    };

    /* length in ticks */
    void set_length (long a_len, bool a_adjust_triggers = true);
    long get_length ();

    /* returns last tick played..  used by
       editors idle function */
    long get_last_tick ();

    /* sets state.  when playing,
       and sequencer is running, notes
       get dumped to the alsa buffers */
    void set_playing (bool);
    bool get_playing ();
    void toggle_playing ();
    
    /* for solo / muting of notes */
    bool check_any_solo_notes();
    void set_solo_note(int a_note);
    void set_mute_note(int a_note);
    bool is_note_solo(int a_note);
    bool is_note_mute(int a_note);

    void toggle_queued ();
    void off_queued ();
    bool get_queued ();
    long get_queued_tick ();

    void set_recording (bool);
    bool get_recording ();
    void set_snap_tick( int a_st );
    void set_quanized_rec( bool a_qr );
    bool get_quanidez_rec( );
    void set_overwrite_rec( bool a_ov );
    bool get_overwrite_rec( );    
    void set_loop_reset( bool a_reset);
    bool get_loop_reset( );

    void set_thru (bool);
    bool get_thru ();

    /* singals that a redraw is needed from recording */
    /* resets flag on call */
    bool is_dirty_main ();
    bool is_dirty_edit ();
    bool is_dirty_perf ();
    bool is_dirty_names ();

    void set_dirty_mp();
    void set_dirty();

    /* midi channel */
    unsigned char get_midi_channel ();
    void set_midi_channel (unsigned char a_ch);

    /* dumps contents to stdout */
    void print ();
    void print_triggers();

    /* dumps notes from tick and prebuffers to
       ahead.  Called by sequencer thread - performance */
    void play (long a_tick, bool a_playback_mode);
    void send_note_to_bus(int transpose, event transposed_event, event note);
    
    void set_orig_tick (long a_tick);

    //
    //  Selection and Manipulation
    //

    /* adds event to internal list */
    void add_event (const event * a_e);
    
    /* for speed on file load these are used to great benefit */
    void add_event_no_sort( const event *a_e );     // all events are added first
    void sort_events();                             // called after all events added, once

    void add_trigger (long a_tick, long a_length, long a_offset = 0, bool a_adjust_offset = true);
    void split_trigger( long a_tick );
    void grow_trigger (long a_tick_from, long a_tick_to, long a_length);
    void del_trigger (long a_tick );
    bool get_trigger_state (long a_tick);
    bool select_trigger(long a_tick);
    bool unselect_triggers ();

    int get_trigger_count ()
    {
        return m_list_trigger.size();
    }

    void get_sequence_triggers(std::vector<trigger> &trig_vect);

    bool intersectTriggers( long position, long& start, long& end );
    bool intersectNotes( long position, long position_note, long& start, long& end, long& note );
    bool intersectEvents( long posstart, long posend, long status, long& start );

    void del_selected_trigger();
    void cut_selected_trigger();
    void copy_selected_trigger();
    void paste_trigger(long a_tick = -1); // -1 default behavior no paste tick
    void set_trigger_export();
    trigger *get_trigger_export();

    void move_selected_triggers_to(long a_tick, bool a_adjust_offset, trigger_edit editMode = MOVE);
    long get_selected_trigger_start_tick();
    long get_selected_trigger_end_tick();

    long get_max_trigger ();

    void move_triggers (long a_start_tick, long a_distance, bool a_direction);
    void paste_triggers (long a_start_tick, long a_distance, long a_offset = 0);
    void clear_triggers ();

    long get_trigger_offset ();

    /* sets the midibus to dump to */
    void set_midi_bus (char a_mb);
    char get_midi_bus ();

    void set_master_midi_bus (mastermidibus * a_mmb);

    enum select_action_e
    {
        e_select,
        e_select_one,
        e_is_selected,
        e_would_select,
        e_deselect, // deselect under cursor
        e_toggle_selection, // sel/deselect under cursor
        e_remove_one // remove one note under cursor
    };

    /* select note events in range, returns number
       selected */
    int select_note_events (long a_tick_s, int a_note_h,
                            long a_tick_f, int a_note_l, select_action_e a_action );

    /* select events in range, returns number
       selected */
    int select_events (long a_tick_s, long a_tick_f,
                       unsigned char a_status, unsigned char a_cc, select_action_e a_action);

    int select_linked (long a_tick_s, long a_tick_f, unsigned char a_status);

    int select_event_handle( long a_tick_s, long a_tick_f,
                         unsigned char a_status,
                         unsigned char a_cc, int a_data_s);


    int get_num_selected_notes ();
    int get_num_selected_events (unsigned char a_status, unsigned char a_cc);

    void select_all ();

    /* given a note length (in ticks) and a boolean indicating even or odd,
    select all notes where the note on even occurs exactly on an even (or odd)
    multiple of note length.
    Example use: select every note that starts on an even eighth note beat.
    */
    int select_even_or_odd_notes(int note_len, bool even);

    void copy_selected ();
    void paste_selected (long a_tick, int a_note);

    /* returns the 'box' of selected items */
    void get_selected_box (long *a_tick_s, int *a_note_h,
                           long *a_tick_f, int *a_note_l);

    /* returns the 'box' of selected items */
    void get_clipboard_box (long *a_tick_s, int *a_note_h,
                            long *a_tick_f, int *a_note_l);

    /* removes and adds readds selected in position */
    void move_selected_notes (long a_delta_tick, int a_delta_note);

    /* adds a single note on / note off pair */
    void add_note (long a_tick, long a_length, int a_note, bool a_paint = false);

    void add_event (long a_tick,
                    unsigned char a_status,
                    unsigned char a_d0, unsigned char a_d1, bool a_paint = false);

    bool stream_event (const event * a_ev);

    /* changes velocities in a ramping way from vel_s to vel_f  */
    void change_event_data_range (long a_tick_s, long a_tick_f,
                                  unsigned char a_status,
                                  unsigned char a_cc,
                                  int a_d_s, int a_d_f);
    //unsigned char a_d_s, unsigned char a_d_f);

    /* lfo tool */
    void change_event_data_lfo(double a_value, double a_range,
                               double a_speed, double a_phase, int a_wave,
                               unsigned char a_status, unsigned char a_cc);

    /* moves note off event */
    void increment_selected (unsigned char a_status, unsigned char a_control);
    void decrement_selected (unsigned char a_status, unsigned char a_control);

    void randomize_selected( unsigned char a_status, unsigned char a_control, int a_plus_minus );

    void adjust_data_handle( unsigned char a_status, int a_data );

    /* moves note off event */
    void grow_selected (long a_delta_tick);
    void stretch_selected(long a_delta_tick);

    /* deletes events */
    void remove_marked();
    bool mark_selected();
    void unpaint_all();

    /* unselects every event */
    void unselect ();

    /* verfies state, all noteons have an off,
       links noteoffs with their ons */
    void verify_and_link ();
    void link_new ();

    /* resets everything to zero, used when
       sequencer stops */
    void zero_markers ();

    /* flushes a note to the midibus to preview its
       sound, used by the virtual paino */
    void play_note_on (int a_note);
    void play_note_off (int a_note);

    /* send a note off for all active notes */
    void off_playing_notes ();

    //
    // Drawing functions
    //

    /* resets draw marker so calls to getNextnoteEvent
       will start from the first */
    void reset_draw_marker ();
    void reset_draw_trigger_marker ();

    /* each call seqdata( sequence *a_seq, int a_scale );fills the passed refrences with a
       events elements, and returns true.  When it
       has no more events, returns a false */
    draw_type get_next_note_event (long *a_tick_s,
                                   long *a_tick_f,
                                   int *a_note,
                                   bool * a_selected, int *a_velocity);

    int get_lowest_note_event ();
    int get_highest_note_event ();

    bool get_next_event (unsigned char a_status,
                         unsigned char a_cc,
                         long *a_tick,
                         unsigned char *a_D0,
                         unsigned char *a_D1, bool * a_selected, int type = ALL_EVENTS);

    bool get_next_event (unsigned char *a_status, unsigned char *a_cc);

    bool get_next_trigger (long *a_tick_on,
                           long *a_tick_off,
                           bool * a_selected, long *a_tick_offset);

    sequence & operator= (const sequence & a_rhs);

    void fill_list (list < char >*a_list, int a_pos, bool write_triggers = true);
    void seq_number_fill_list( list<char> *a_list, int a_pos );
    void seq_name_fill_list( list<char> *a_list );
    void fill_proprietary_list(list < char >*a_list);
    void meta_track_end( list<char> *a_list, long delta_time);
    long song_fill_list_seq_event( list<char> *a_list, trigger *a_trig, long prev_timestamp, file_type_e type );
    void song_fill_list_seq_trigger( list<char> *a_list, trigger *a_trig, long a_length, long prev_timestamp );

    void select_events (unsigned char a_status, unsigned char a_cc,
                        bool a_inverse = false);
    void quanize_events (unsigned char a_status, unsigned char a_cc,
                         long a_snap_tick, int a_divide, bool a_linked =
                             false);
    void transpose_notes (int a_steps, int a_scale);
    void apply_song_transpose ();
    void shift_notes (int a_ticks);  // move selected notes later/earlier in time
    void multiply_pattern( float a_multiplier );
    void reverse_pattern();
    void calulate_reverse(event &a_e);
};
