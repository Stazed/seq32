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

#include "sequence.h"
#include "perform.h"

#include <gtkmm/adjustment.h>
#include <gtkmm/button.h>
#include <gtkmm/window.h>
#include <gtkmm/accelgroup.h>
#include <gtkmm/box.h>
#include <gtkmm/main.h>
#include <gtkmm/menu.h>
#include <gtkmm/menubar.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/window.h>
#include <gtkmm/table.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/widget.h>
#include <gtkmm/scrollbar.h>
#include <gtkmm/viewport.h>
#include <gtkmm/combo.h>
#include <gtkmm/label.h>
#include <gtkmm/toolbar.h>
#include <gtkmm/optionmenu.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/invisible.h>
#include <gtkmm/separator.h>
#include <gtkmm/tooltips.h>
#include <gtkmm/invisible.h>
#include <gtkmm/arrow.h>
#include <gtkmm/image.h>

#include <sigc++/bind.h>

#include <list>
#include <string>

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

    Adjustment *m_vadjust;
    Adjustment *m_hadjust;

    perfnames *m_perfnames;
    perfroll *m_perfroll;
    perftime *m_perftime;
    tempo *m_tempo;

    /* time signature, beats per measure, beat width */
    Menu        *m_menu_bp_measure;
    Menu        *m_menu_bw;

    Menu        *m_menu_snap;
    Menu        *m_menu_xpose;

    Button      *m_button_snap;
    Entry       *m_entry_snap;

    Button      *m_button_xpose;
    Entry       *m_entry_xpose;

    Button      *m_button_stop;
    Button      *m_button_play;

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

    Tooltips *m_tooltips;

    /* set snap to in pulses */
    int m_snap;
    int m_bp_measure;
    int m_bw;

    void bp_measure_button_callback(int a_beats_per_measure);
    void bw_button_callback(int a_beat_width);

    void set_snap (int a_snap);

    void set_guides();

    void grow ();

    void on_realize ();

    void start_playing ();
    void stop_playing ();
    void rewind(bool a_press);
    void fast_forward(bool a_press);

    void set_looped ();
    void set_jack_mode();

    void xpose_button_callback( int a_xpose);

    void expand ();
    void collapse ();
    void copy ();
    void undo ();
    void redo ();

    void popup_menu (Menu * a_menu);

    bool timeout ();

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
    void update_start_BPM();

    void set_xpose (int a_xpose);
    void set_zoom (int z);

    bool get_toggle_jack();
    void toggle_jack();
    void set_follow_transport ();
    void toggle_follow_transport();
    
    void load_tempo_list();
    void reset_tempo_list(bool play_list_only);

    friend int FF_RW_timeout(void *arg);

    perfedit (perform * a_perf, mainwnd *a_main);
    ~perfedit ();
};

