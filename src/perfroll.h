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

#include "globals.h"
#include "perform.h"
#include "mutex.h"
#include "perfedit.h"
#include "perfroll_input.h"

using namespace Gtk;

const int c_perfroll_background_x = (c_ppqn * 4 * 16) / c_perf_max_zoom ;
const int c_perfroll_size_box_w = 6;
const int c_perfroll_size_box_click_w = c_perfroll_size_box_w+1 ;

class perfedit;

/* performance roll */
class perfroll : public Gtk::DrawingArea
{

private:
    friend class FruityPerfInput;
    friend class Seq32PerfInput;

    AbstractPerfInput* m_interaction;

    Cairo::RefPtr<Cairo::ImageSurface> m_surface_track;
    Cairo::RefPtr<Cairo::ImageSurface> m_surface_background;

    perform        * const m_mainperf;
    perfedit       * const m_perfedit;

    int          m_snap;
    int          m_measure_length;
    int          m_beat_length;

    int          m_perf_scale_x; // for zoom - replace c_perf_scale_x

    int          m_window_x, m_window_y;

    long         m_old_progress_ticks;

    int          m_4bar_offset;
    int          m_sequence_offset;

    int          m_roll_length_ticks;

    int          m_drop_x, m_drop_y;

    long         m_drop_tick;
    long         m_drop_tick_trigger_offset;
    int          m_drop_sequence;

    bool         m_sequence_active[c_total_seqs];

    Glib::RefPtr<Adjustment> m_vadjust;
    Glib::RefPtr<Adjustment> m_hadjust;

    bool m_moving;
    bool m_growing;
    bool m_grow_direction;

    void on_realize();
    bool on_button_press_event(GdkEventButton* a_ev);
    bool on_button_release_event(GdkEventButton* a_ev);
    bool on_motion_notify_event(GdkEventMotion* a_ev);
    bool on_scroll_event( GdkEventScroll* a_ev ) ;

    bool on_focus_in_event(GdkEventFocus*);
    bool on_focus_out_event(GdkEventFocus*);

    void on_size_request(GtkRequisition* );
    void on_size_allocate(Gtk::Allocation& );

    bool on_key_press_event(GdkEventKey* a_p0);

    void convert_xy( int a_x, int a_y, long *a_ticks, int *a_seq);
    void convert_x( int a_x, long *a_ticks);
    void snap_x( int *a_x );

    void draw_sequence_on( int a_sequence );
    void draw_background_on( int a_sequence );

    void change_horz();
    void change_vert();

    void split_trigger( int a_sequence, long a_tick );

    bool have_button_press;
    bool transport_follow;
    bool trans_button_press;
    bool m_redraw_tracks;


protected:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);

public:

    void auto_scroll_horz();
    int  m_zoom;
    void set_zoom (int a_zoom);
    void set_guides( int a_snap, int a_measure, int a_beat );

    void update_sizes();
    void init_before_show( );
    void fill_background_surface();

    void increment_size();

    void draw_progress();

    void redraw_dirty_sequences();
    void redraw_all_tracks(){m_redraw_tracks = true;}
    
    int get_drop_sequence();

    perfroll( perform *a_perf,
              perfedit *a_perf_edit,
              Glib::RefPtr<Adjustment> a_hadjust,
              Glib::RefPtr<Adjustment> a_vadjust);

    ~perfroll( );
};
