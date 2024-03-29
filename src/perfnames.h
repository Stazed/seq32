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

#include "perform.h"
#include "sequence.h"
#include "seqmenu.h"
#include "globals.h"

using namespace Gtk;

/* forward declaration */
class mainwnd;

/* holds the left side piano */
class perfnames : public virtual Gtk::DrawingArea, public virtual seqmenu
{
private:
    
    Cairo::RefPtr<Cairo::ImageSurface> m_surface;
    Glib::RefPtr<Gdk::Pixbuf> m_pixbuf;

    perform      *m_mainperf;

    Glib::RefPtr<Adjustment> m_vadjust;

    bool m_redraw_tracks;

    int m_window_x, m_window_y;

    int          m_sequence_offset;
    int          m_names_y;
    float        m_vertical_zoom;

    bool         m_sequence_active[c_total_seqs];
    
    sequence     m_clipboard;
    sequence     m_moving_seq;
    
    bool         m_button_down;
    bool         m_moving;
    int          m_old_seq;

    void on_realize() override;
    bool on_button_press_event(GdkEventButton* a_ev) override;
    bool on_button_release_event(GdkEventButton* a_ev) override;
    bool on_motion_notify_event(GdkEventMotion* a_ev) override;
    void on_size_allocate(Gtk::Allocation& ) override;
    bool on_scroll_event( GdkEventScroll* a_ev ) override;

    void convert_y( int a_y, int *a_note);

    void draw_sequence( int a_sequence );

    void change_vert();

    void redraw( int a_sequence ) override;
    
    void check_global_solo_tracks();
    
    void sequence_is_being_edited();

protected:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);

public:

    void redraw_dirty_sequences();
    void set_vertical_zoom (float a_zoom);

    perfnames( perform *a_perf, mainwnd *a_main,
               Glib::RefPtr<Adjustment> a_vadjust );

};
