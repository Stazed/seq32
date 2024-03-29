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
#include "seqkeys.h"
#include "globals.h"
#include "seqdata.h"
#include "seqevent.h"
#include "perform.h"

using namespace Gtk;

class rect
{
public:
    int x, y, height, width;
};

class seqroll;
struct FruitySeqRollInput
{
    FruitySeqRollInput() : m_adding( false ), m_canadd( true ), m_erase_painting( false ),
    m_drag_paste_start_pos()
    {}
    bool on_button_press_event(GdkEventButton* a_ev, seqroll& ths);
    bool on_button_release_event(GdkEventButton* a_ev, seqroll& ths);
    bool on_motion_notify_event(GdkEventMotion* a_ev, seqroll& ths);
    void updateMousePtr(seqroll& ths);
    bool m_adding;
    bool m_canadd;
    bool m_erase_painting;
    long m_drag_paste_start_pos[2];
};

struct Seq32SeqRollInput
{
    Seq32SeqRollInput() : m_adding( false )
    {}
    bool on_button_press_event(GdkEventButton* a_ev, seqroll& ths);
    bool on_button_release_event(GdkEventButton* a_ev, seqroll& ths);
    bool on_motion_notify_event(GdkEventMotion* a_ev, seqroll& ths);
    void set_adding( bool a_adding, seqroll& ths );
    bool m_adding;
};

/* piano roll */
class seqroll : public Gtk::DrawingArea
{

private:
    friend struct FruitySeqRollInput;
    FruitySeqRollInput m_fruity_interaction;

    friend struct Seq32SeqRollInput;
    Seq32SeqRollInput m_seq32_interaction;
    
    Cairo::RefPtr<Cairo::Context> m_surface_window;
    Cairo::RefPtr<Cairo::ImageSurface> m_surface_edit;
    Cairo::RefPtr<Cairo::ImageSurface> m_surface_background;

    rect         m_old;
    rect         m_selected;

    sequence     * const m_seq;
    sequence     * m_clipboard;
    perform      * const m_perform;
    seqdata      * const m_seqdata_wid;
    seqevent     * const m_seqevent_wid;
    seqkeys      * const m_seqkeys_wid;

    ToggleButton *m_toggle_play;

    int m_pos;

    /* one pixel == m_zoom ticks */
    int          m_zoom;
    int          m_snap;
    int          m_note_length;

    int          m_scale;
    int          m_chord;
    int          m_key;

    /* Vertical Zoom */
    int         m_key_y;
    int         m_rollarea_y;

    int m_window_x, m_window_y;

    /* what is the data window currently editing ? */
    unsigned char m_status;
    unsigned char m_cc;

    /* when highlighting a bunch of events */
    bool m_selecting;
    bool m_moving;
    bool m_moving_init;
    bool m_growing;
    bool m_painting;
    bool m_paste;
    bool m_is_drag_pasting;
    bool m_is_drag_pasting_start;
    bool m_justselected_one;

    /* where the dragging started */
    int m_drop_x;
    int m_drop_y;
    int m_move_delta_x;
    int m_move_delta_y;
    int m_current_x;
    int m_current_y;

    int m_move_snap_offset_x;

    int m_old_progress_x;

    Glib::RefPtr<Adjustment> const m_vadjust;
    Glib::RefPtr<Adjustment> const m_hadjust;

    int m_scroll_offset_ticks;
    int m_scroll_offset_key;

    int m_scroll_offset_x;
    int m_scroll_offset_y;

    int m_scroll_page;

    bool transport_follow;
    bool trans_button_press;

    int m_background_sequence;
    bool m_drawing_background_seq;

    bool m_ignore_redraw;
    bool m_expanded_recording;
    bool m_have_realize;
    bool m_redraw_window;

    void on_realize();
    bool on_button_press_event(GdkEventButton* a_ev);
    bool on_button_release_event(GdkEventButton* a_ev);
    bool on_motion_notify_event(GdkEventMotion* a_ev);

    bool on_focus_in_event(GdkEventFocus*);
    bool on_focus_out_event(GdkEventFocus*);
    bool on_scroll_event( GdkEventScroll* a_ev);

    bool on_leave_notify_event	(GdkEventCrossing* a_p0);
    bool on_enter_notify_event	(GdkEventCrossing* a_p0);

    void convert_xy( int a_x, int a_y, long *a_ticks, int *a_note);
    void convert_tn( long a_ticks, int a_note, int *a_x, int *a_y);

    void snap_y( int *a_y );
    void snap_x( int *a_x );

    void xy_to_rect( int a_x1,  int a_y1,
                     int a_x2,  int a_y2,
                     int *a_x,  int *a_y,
                     int *a_w,  int *a_h );

    void convert_tn_box_to_rect( long a_tick_s, long a_tick_f,
                                 int a_note_h, int a_note_l,
                                 int *a_x, int *a_y,
                                 int *a_w, int *a_h );

    void on_size_allocate(Gtk::Allocation& );

    void change_horz();
    void change_vert();

    void force_draw();

protected:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);

public:

    bool on_key_press_event(GdkEventKey* a_p0); // called from seqedit
    void reset();
    void redraw();
    void redraw_events();
    void set_zoom( int a_zoom );
    void set_vertical_zoom( int key_y, int rollarea_y);
    void set_snap( int a_snap );
    void set_note_length( int a_note_length );
    void set_ignore_redraw(bool a_ignore);

    void set_scale( int a_scale );
    void set_chord( int a_chord );
    void set_key( int a_key );

    void update_sizes();
    void update_background();

    void draw_background_on_surface();
    void draw_events_on_surface();
    void draw_selection_on_window(const Cairo::RefPtr<Cairo::Context>& cr);
    void update_surface();

    void draw_progress_on_window();
    void follow_progress();

    void start_paste( );
    void set_expanded_recording(bool a_record);
    bool get_expanded_record();

    void set_background_sequence( bool a_state, int a_seq );

    seqroll( perform *a_perf,
             sequence *a_seq, int a_zoom, int a_snap,
             seqdata *a_seqdata_wid,
             seqevent *a_seqevent_wid,
             seqkeys *a_seqkeys_wid,
             int a_pos,
             Glib::RefPtr<Adjustment> a_hadjust,
             Glib::RefPtr<Adjustment> a_vadjust,
             ToggleButton *a_toggle_play);

    void set_data_type( unsigned char a_status, unsigned char a_control  );

    ~seqroll( );

};
