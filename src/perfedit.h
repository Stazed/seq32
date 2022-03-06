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

#include <list>
#include <string>

#include "sequence.h"
#include "perform.h"
#include "globals.h"
#include "mainwid.h"
#include "perfnames.h"
#include "perfroll.h"
#include "perftime.h"
#include "tempo.h"

using namespace Gtk;

/* forward declarations */
class perfroll;
class perftime;
class tempo;

/* has a seqroll and paino roll */
class perfedit:public Gtk::Window
{

private:

    perform *m_mainperf;
    mainwnd *m_mainwnd;

    Table *m_table;

    VScrollbar *m_vscroll;
    HScrollbar *m_hscroll;

    Glib::RefPtr<Adjustment> m_vadjust;
    Glib::RefPtr<Adjustment> m_hadjust;

    perfnames *m_perfnames;
    perfroll *m_perfroll;
    perftime *m_perftime;
    tempo *m_tempo;

    /* time signature, beats per measure, beat width */
    Menu        *m_menu_bp_measure;
    Menu        *m_menu_bw;

    Menu        *m_menu_snap;
    Menu        *m_menu_xpose;
    
    std::vector<MenuItem> m_snap_menu_items;
    std::vector<MenuItem> m_bw_menu_items;
    SeparatorMenuItem   m_menu_separator6;
    SeparatorMenuItem   m_menu_separator7;
    SeparatorMenuItem   m_menu_separator8;

    Button      *m_button_snap;
    Entry       *m_entry_snap;

    Button      *m_button_xpose;
    Entry       *m_entry_xpose;

    Button      *m_button_stop;
    Button      *m_button_play;

    ToggleButton *m_button_continue;
    ToggleButton *m_button_loop;
    ToggleButton *m_button_jack;
    ToggleButton *m_button_follow;

    Button      *m_button_expand;
    Button      *m_button_collapse;
    Button      *m_button_copy;

    Button      *m_button_grow;
    Button      *m_button_undo;
    Button      *m_button_redo;

    Button      *m_button_bp_measure;
    Entry       *m_entry_bp_measure;

    Button      *m_button_bw;
    Entry       *m_entry_bw;

    HBox *m_hbox;
    HBox *m_hlbox;


    /* set snap to in pulses */
    int m_snap;
    int m_bp_measure;
    int m_bw;
    
    /** From sequencer64
     *  Shows the current time into the song performance.
     */

    Gtk::Label * m_tick_time;
    /**
     *  This button will toggle the m_tick_time_as_bbt member.
     */

    Gtk::Button * m_button_time_type;

    /**
     *  Indicates whether to show the time as bar:beats:ticks or as
     *  hours:minutes:seconds.  The default is true:  bar:beats:ticks.
     */

    bool m_tick_time_as_bbt;
    
    /* Flag used when time type is toggled when stopped to update gui */
    bool m_toggle_time_type;

    void bp_measure_button_callback(int a_beats_per_measure);
    void bw_button_callback(int a_beat_width);

    void set_snap (int a_snap);

    void set_guides();

    void grow ();

    void on_realize ();

    void start_playing ();
    void set_continue_callback();
    void stop_playing ();
    void rewind(bool a_press);
    void fast_forward(bool a_press);

    void set_looped ();
    void set_jack_mode();

    void xpose_button_callback( int a_xpose);
    
    void toggle_time_format( );

    void expand ();
    void collapse ();
    void copy ();
    void undo ();
    void redo ();

    void popup_menu (Menu * a_menu);

    bool timeout ();
  
    double tempo_map_microseconds(unsigned long a_tick);
    
    /* Sequencer64 */
    inline double
    ticks_to_delta_time_us (long delta_ticks, double bpm, int ppqn)
    {
        return double(delta_ticks) * pulse_length_us(bpm, ppqn);
    }
    
    inline double
    pulse_length_us (double bpm, int ppqn)
    {
        return 60000000.0 / ppqn / bpm * ( (double) m_bw / 4.0 );
    }
    
    std::string tick_to_timestring(long a_tick);
    std::string tick_to_measurestring (long a_tick);
    void tick_to_midi_measures (long a_tick, int &measures, int &beats, int &divisions);

    bool on_delete_event (GdkEventAny * a_event);
    bool on_key_press_event(GdkEventKey* a_ev);
    bool on_key_release_event(GdkEventKey* a_ev);

public:

    static bool zoom_check (int z)
    {
        return z >= c_perf_max_zoom && z <= (4 * c_perf_scale_x);
    }

    void init_before_show ();
    void set_bp_measure( int a_beats_per_measure );
    int get_bp_measure();
    void set_bw( int a_beat_width );
    int get_bw();
    void update_start_BPM(double bpm);
    void clear_tempo_list();

    void set_xpose (int a_xpose);
    void set_zoom (int z);

    bool get_toggle_jack();
    void toggle_jack();
    void set_follow_transport ();
    void toggle_follow_transport();
    
    void load_tempo_list();
    
    void update_clock();
    void reposition_progress_line();
    void hide_tempo_popup();
    void set_perfroll_marker_change(bool a_change);
    void set_tempo_marker_change(uint64_t a_tick = 0);

    friend int FF_RW_timeout(void *arg);

    perfedit (perform * a_perf, mainwnd *a_main);
    ~perfedit ();
};

