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
#include <gtkmm/adjustment.h>

using namespace Gtk;

#include "globals.h"

/* forward declaration */
class mainwnd;

/* holds the left side piano */
class perfnames : public virtual Gtk::DrawingArea, public virtual seqmenu
{
private:

    Glib::RefPtr<Gdk::Window>   m_window;
    Cairo::RefPtr<Cairo::Context>  m_surface_window;
    
    Cairo::RefPtr<Cairo::ImageSurface> m_surface;

    perform      *m_mainperf;

    Adjustment   *m_vadjust;

    bool m_redraw_tracks;

    int m_window_x, m_window_y;

    int          m_sequence_offset;

    bool         m_sequence_active[c_total_seqs];
    
    sequence     m_clipboard;
    sequence     m_moving_seq;
    
    bool         m_button_down;
    bool         m_moving;
    int          m_old_seq;

    void on_realize();
    bool on_expose_event(GdkEventExpose* a_ev);
    bool on_button_press_event(GdkEventButton* a_ev);
    bool on_button_release_event(GdkEventButton* a_ev);
    bool on_motion_notify_event(GdkEventMotion* a_ev);
    void on_size_allocate(Gtk::Allocation& );
    bool on_scroll_event( GdkEventScroll* a_ev ) ;

    void convert_y( int a_y, int *a_note);

    void draw_sequence( int a_sequence );

    void change_vert();

    void redraw( int a_sequence );
    
    void check_global_solo_tracks();

protected:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);

public:

    void idle_redraw();
    void redraw_dirty_sequences();

    perfnames( perform *a_perf, mainwnd *a_main,
               Adjustment *a_vadjust   );

};
