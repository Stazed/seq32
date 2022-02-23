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

#include "maintime.h"


maintime::maintime( ):
    m_tick(0)
{
}

void
maintime::on_realize()
{
    // we need to do the default realize
    Gtk::DrawingArea::on_realize();

    // Now we can allocate any additional resources we need
    m_window = get_window();
    m_window->clear();

    /* set default size */
    set_size_request( c_maintime_x, c_maintime_y );
}

int
maintime::idle_progress( long a_ticks )
{
    m_tick = a_ticks;
    
    if(get_realized())
    {
        on_draw();
    }
   // queue_draw();

    return true;
}

bool
maintime::on_expose_event(GdkEventExpose* a_e)
{
    idle_progress( m_tick );
    return true;
}

bool
maintime::on_draw(/* const Cairo::RefPtr<Cairo::Context>& cr */)
{
    Cairo::RefPtr<Cairo::Context> cr = m_window->create_cairo_context();

    cr->set_operator(Cairo::OPERATOR_CLEAR);
    cr->rectangle(-1, -1, (c_maintime_x - 1), (c_maintime_y - 1));
    cr->paint_with_alpha(0.0);
    cr->set_operator(Cairo::OPERATOR_OVER);

    /* clear the window */
    cr->set_source_rgb(0.8, 0.8, 0.8);          // light grey FIXME
    cr->rectangle(0.0, 0.0, (c_maintime_x - 1), (c_maintime_y - 1));
    cr->stroke_preserve();
    cr->fill();

    cr->set_source_rgb(0.0, 0.0, 0.0);          // black FIXME
    cr->set_line_width(1.0);
    cr->rectangle(0.0, 0.0, (c_maintime_x - 1), (c_maintime_y - 1));
    cr->stroke();

    int width = c_maintime_x - 1 - c_pill_width;

    int tick_x = ((m_tick % c_ppqn) * (c_maintime_x - 1) ) / c_ppqn ;
    int beat_x = (((m_tick / 4) % c_ppqn) * width) / c_ppqn ;
    int bar_x = (((m_tick / 16) % c_ppqn) * width) / c_ppqn ;

    if ( tick_x <= (c_maintime_x / 4 ))
    {
        cr->set_source_rgb(0.6, 0.6, 0.6);    // grey FIXME
        cr->rectangle(2.0, 2.0, (c_maintime_x - 4), (c_maintime_y - 4));
        cr->stroke_preserve();
        cr->fill();
    }

    cr->set_source_rgb(0.0, 0.0, 0.0);        // black FIXME
    cr->rectangle((beat_x + 2), 2.0, c_pill_width, (c_maintime_y - 4));
    cr->stroke_preserve();
    cr->fill();

    cr->rectangle((bar_x + 2), 2.0, c_pill_width, (c_maintime_y - 4));
    cr->stroke_preserve();
    cr->fill();

    return true;
}
