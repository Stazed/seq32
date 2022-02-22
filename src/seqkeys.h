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

#include "globals.h"

enum
{
    White_Note = 0,
    Green_Note,
    Red_Note
};

/* holds the left side piano */
class seqkeys : public Gtk::DrawingArea
{
private:

    Glib::RefPtr<Gdk::Window> m_window;
    Cairo::RefPtr<Cairo::Context>  m_surface_window;

    Cairo::RefPtr<Cairo::ImageSurface> m_surface;

    sequence *m_seq;

    Gtk::Adjustment   *m_vadjust;

    int m_scroll_offset_key;
    int m_scroll_offset_y;

    int m_window_x;
    int m_window_y;

    void on_realize();
    bool on_expose_event(GdkEventExpose* a_ev);
    bool on_button_press_event(GdkEventButton* a_ev);
    bool on_button_release_event(GdkEventButton* a_ev);
    bool on_motion_notify_event(GdkEventMotion* a_p0);
    bool on_leave_notify_event(GdkEventCrossing* p0);
    bool on_enter_notify_event(GdkEventCrossing* p0);
    bool on_scroll_event( GdkEventScroll* a_ev);

    void update_surface();
    void convert_y( int a_y, int *a_note);

    bool m_hint_state;
    int m_hint_key;
    bool m_enter_piano_roll;

    bool m_keying;
    int m_keying_note;

    int          m_scale;
    int          m_key;

    void draw_key( int a_key, bool a_state );
    void on_size_allocate(Gtk::Allocation&);

    void change_vert();
    void update_sizes();
    void reset();

protected:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);

public:
    
    void idle_draw();

    /* listen to note on add or move */
    void set_listen_button_press(GdkEventButton* a_ev);
    void set_listen_button_release(GdkEventButton* a_ev);
    void set_listen_motion_notify(GdkEventMotion* a_p0);

    /* sets key to red */
    void set_hint_key( int a_key );

    /* true == on, false == off */
    void set_hint_state( bool a_state );

    seqkeys( sequence *a_seq,
             Gtk::Adjustment *a_vadjust );

    void set_scale( int a_scale );
    void set_key( int a_key );

};
