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


#include "globals.h"
#include "perform.h"
#include "seqmenu.h"

/* forward declarations */
class seqedit;
class mainwnd;

#pragma once

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

using namespace Gtk;

enum
{
    COLOR_BLACK = 0,
    COLOR_WHITE
};

/* The main window grid of sequences */
class mainwid : public Gtk::DrawingArea, public seqmenu
{

private:

    Cairo::RefPtr<Cairo::Context>  m_surface_window;
    Cairo::RefPtr<Cairo::ImageSurface> m_surface;

    int          m_screenset;

    perform      * const m_mainperf;

    sequence     m_clipboard;
    sequence     m_moving_seq;

    int          m_window_x,
                 m_window_y;

    float        m_text_x;
    float        m_text_y;
    float        m_seqarea_x;
    float        m_seqarea_y;

    /* The draw progress area size */
    int         m_seqarea_seq_x = c_text_x * 13;
    int         m_seqarea_seq_y = c_text_y * 2;
    
    /* The resize ratio for font resizing */
    float       m_resize_ratio_x;
    float       m_resize_ratio_y;

    bool         m_button_down;
    bool         m_moving;
    
    bool         m_need_redraw;
    bool         m_have_realize;
    int          m_progress_tick;
    
    int m_background_color, m_foreground_color;

    /* when highlighting a bunch of events */

    /* where the dragging started */
    int m_drop_x;
    int m_drop_y;
    int m_current_x;
    int m_current_y;

    int m_old_seq;

    long m_last_tick_x[c_max_sequence];
    bool m_last_playing[c_max_sequence];
    static const char m_seq_to_char[c_seqs_in_set];

    void on_realize() override;

    bool on_button_press_event(GdkEventButton* a_ev) override;
    bool on_button_release_event(GdkEventButton* a_ev) override;
    bool on_motion_notify_event(GdkEventMotion* a_p0) override;
    bool on_focus_in_event(GdkEventFocus*) override;
    bool on_focus_out_event(GdkEventFocus*) override;
    void on_size_allocate(Gtk::Allocation& ) override;

    void draw_sequence_on_surface( int a_seq );
    void draw_sequences_on_surface();

    void fill_background_window();
    void draw_sequence_surface_on_window( int a_seq );

    int seq_from_xy( int a_x, int a_y );

    void redraw( int a_seq ) override;
    void sequence_is_being_edited();
    
protected:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;

public:

    mainwid( perform *a_p, mainwnd *a_main );
    ~mainwid( );

    void reset();

    //int get_screenset( );
    void set_screenset( int a_ss );

    void update_sequence_on_window( int a_seq  );
    void update_sequences_on_window( );

    void update_markers( int a_ticks );
    void draw_marker_on_sequence( int a_seq, int a_tick );

};
