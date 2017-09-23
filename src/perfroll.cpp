//----------------------------------------------------------------------------
//
//  This file is part of seq32.
//
//  seq32 is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General)mm Public License as published by
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
#include "event.h"
#include "perfroll.h"

perfroll::perfroll( perform *a_perf,
                    perfedit * a_perf_edit,
                    Adjustment * a_hadjust,
                    Adjustment * a_vadjust  ) :
    m_window(NULL),
    m_black(Gdk::Color("black")),
    m_white(Gdk::Color("white")),
    m_grey(Gdk::Color("grey")),
    m_lt_grey(Gdk::Color("light grey")),

    m_pixmap(NULL),
    m_mainperf(a_perf),
    m_perfedit(a_perf_edit),

    m_perf_scale_x(c_perf_scale_x),       // 32 ticks per pixel

    m_old_progress_ticks(0),

    m_4bar_offset(0),
    m_sequence_offset(0),
    m_roll_length_ticks(0),
    m_drop_y(0),
    m_drop_sequence(0),

    m_vadjust(a_vadjust),
    m_hadjust(a_hadjust),

    m_moving(false),
    m_growing(false),

    have_button_press(false),
    transport_follow(true),
    trans_button_press(false),
    m_zoom(c_perf_scale_x)
{
    Glib::RefPtr<Gdk::Colormap> colormap = get_default_colormap();
    colormap->alloc_color( m_black );
    colormap->alloc_color( m_white );
    colormap->alloc_color( m_grey );
    colormap->alloc_color( m_lt_grey );

    //m_text_font_6_12 = Gdk_Font( c_font_6_12 );

    add_events( Gdk::BUTTON_PRESS_MASK |
                Gdk::BUTTON_RELEASE_MASK |
                Gdk::POINTER_MOTION_MASK |
                Gdk::KEY_PRESS_MASK |
                Gdk::KEY_RELEASE_MASK |
                Gdk::FOCUS_CHANGE_MASK |
                Gdk::SCROLL_MASK );

    set_size_request( 10, 10 );

    set_double_buffered( false );

    for( int i=0; i<c_total_seqs; ++i )
        m_sequence_active[i]=false;

    switch (global_interactionmethod)
    {
    case e_fruity_interaction:
        m_interaction = new FruityPerfInput;
        break;
    default:
    case e_seq32_interaction:
        m_interaction = new Seq32PerfInput;
        break;
    }
}

perfroll::~perfroll( )
{
    delete m_interaction;
}

void
perfroll::change_horz()
{
    if ( m_4bar_offset != (int) m_hadjust->get_value() )
    {
        m_4bar_offset = (int) m_hadjust->get_value();
        queue_draw();
    }
}

void
perfroll::change_vert()
{
    if ( m_sequence_offset != (int) m_vadjust->get_value() )
    {
        /*   must adjust m_drop_y or perfroll_input unselect_triggers will not work if   */
        /*   scrolled up or down to a new location - see note in on_button_press_event() */
        /*   in perfroll_input.cpp */

        m_drop_y += ((m_sequence_offset - (int) m_vadjust->get_value()) *c_names_y);

        m_sequence_offset = (int) m_vadjust->get_value();
        queue_draw();
    }
}

void
perfroll::on_realize()
{
    // we need to do the default realize
    Gtk::DrawingArea::on_realize();

    set_flags( Gtk::CAN_FOCUS );

    // Now we can allocate any additional resources we need
    m_window = get_window();
    m_gc = Gdk::GC::create( m_window );
    m_window->clear();

    update_sizes();

    m_hadjust->signal_value_changed().connect( mem_fun( *this, &perfroll::change_horz ));
    m_vadjust->signal_value_changed().connect( mem_fun( *this, &perfroll::change_vert ));

    /*
        This creation of m_background needs to be set to the max width for proper drawing of zoomed
        measures or they will get truncated with high beats per measure and low beat width. Since this
        is a constant size, it cannot be adjusted later for zoom. The constant c_perfroll_background_x
        is set to the max amount by default for use here. The drawing functions fill_background_pixmap()
        and draw_background_on() which use c_perfroll_background_x also, could be adjusted by zoom with
        a substituted variable. Not sure if there is any benefit to doing the adjustment...
        Perhaps a small benefit in speed? Maybe FIXME if really, really bored...
    */

    m_background = Gdk::Pixmap::create( m_window,
                                        c_perfroll_background_x,
                                        c_names_y, -1 );

    /* and fill the background ( dotted lines n' such ) */
    fill_background_pixmap();
}

void
perfroll::init_before_show( )
{
    m_roll_length_ticks = m_mainperf->get_max_trigger();
    m_roll_length_ticks = m_roll_length_ticks -
                          ( m_roll_length_ticks % ( c_ppqn * 16 ));
    m_roll_length_ticks +=  c_ppqn * 4096;
}

void
perfroll::update_sizes()
{
    int h_bars         = m_roll_length_ticks / (c_ppqn * 16);
    int h_bars_visable = (m_window_x * m_perf_scale_x) / (c_ppqn * 16);

    m_hadjust->set_lower( 0 );
    m_hadjust->set_upper( h_bars );
    m_hadjust->set_page_size( h_bars_visable );
    m_hadjust->set_step_increment( 1 );
    m_hadjust->set_page_increment( 1 );

    int h_max_value = h_bars - h_bars_visable;

    if ( m_hadjust->get_value() > h_max_value )
    {
        m_hadjust->set_value( h_max_value );
    }

    m_vadjust->set_lower( 0 );
    m_vadjust->set_upper( c_total_seqs );
    m_vadjust->set_page_size( m_window_y / c_names_y );
    m_vadjust->set_step_increment( 1 );
    m_vadjust->set_page_increment( 1 );

    int v_max_value = c_total_seqs - (m_window_y / c_names_y);

    if ( m_vadjust->get_value() > v_max_value )
    {
        m_vadjust->set_value(v_max_value);
    }

    if ( is_realized() )
    {
        m_pixmap = Gdk::Pixmap::create( m_window,
                                        m_window_x,
                                        m_window_y, -1 );
    }

    queue_draw();
}

void
perfroll::increment_size()
{
    m_roll_length_ticks += (c_ppqn * 512);
    update_sizes( );
}

/* updates background */
void
perfroll::fill_background_pixmap()
{
    /* clear background */
    m_gc->set_foreground(m_white);
    m_background->draw_rectangle(m_gc,true,
                                 0,
                                 0,
                                 c_perfroll_background_x,
                                 c_names_y );

    /* draw horz grey lines */
    m_gc->set_foreground(m_grey);

    gint8 dash = 1;
    m_gc->set_dashes( 0, &dash, 1 );

    m_gc->set_line_attributes( 1,
                               Gdk::LINE_ON_OFF_DASH,
                               Gdk::CAP_NOT_LAST,
                               Gdk::JOIN_MITER );

    m_background->draw_line(m_gc,
                            0,
                            0,
                            c_perfroll_background_x,
                            0 );

    int beats = m_measure_length / m_beat_length;

    /* draw vert lines */
    for ( int i=0; i< beats ; )
    {
        if ( i == 0 )
        {
            m_gc->set_line_attributes( 1,
                                       Gdk::LINE_SOLID,
                                       Gdk::CAP_NOT_LAST,
                                       Gdk::JOIN_MITER );
        }
        else
        {
            m_gc->set_line_attributes( 1,
                                       Gdk::LINE_ON_OFF_DASH,
                                       Gdk::CAP_NOT_LAST,
                                       Gdk::JOIN_MITER );
        }

        m_gc->set_foreground(m_grey);

        /* solid line on every beat */
        m_background->draw_line(m_gc,
                                i * m_beat_length / m_perf_scale_x,
                                0,
                                i * m_beat_length / m_perf_scale_x,
                                c_names_y );

        // jump 2 if 16th notes
        if ( m_beat_length < c_ppqn/2 )
        {
            i += (c_ppqn / m_beat_length);
        }
        else
        {
            ++i;
        }
    }

    /* reset line style */

    m_gc->set_line_attributes( 1,
                               Gdk::LINE_SOLID,
                               Gdk::CAP_NOT_LAST,
                               Gdk::JOIN_MITER );
}

/* simply sets the snap member */
void
perfroll::set_guides( int a_snap, int a_measure, int a_beat )
{
    m_snap = a_snap;
    m_measure_length = a_measure;
    m_beat_length = a_beat;

    if ( is_realized() )
    {
        fill_background_pixmap();
    }

    queue_draw();
}

void
perfroll::draw_progress()
{
    long tick = m_mainperf->get_tick();
    long tick_offset = m_4bar_offset * c_ppqn * 16;

    int progress_x =     ( tick - tick_offset ) / m_perf_scale_x ;
    int old_progress_x = ( m_old_progress_ticks - tick_offset ) / m_perf_scale_x ;

    /* draw old */
    m_window->draw_drawable
    (
        m_gc,
        m_pixmap,
        old_progress_x-1, 0,
        old_progress_x-1, 0,
        2, m_window_y
    );

    m_gc->set_line_attributes( 2,
                           Gdk::LINE_SOLID,
                           Gdk::CAP_NOT_LAST,
                           Gdk::JOIN_MITER );

    m_gc->set_foreground(m_black);

    m_window->draw_line
    (
        m_gc,
        progress_x, 0,
        progress_x, m_window_y
    );
    // reset
    m_gc->set_line_attributes( 1,
                           Gdk::LINE_SOLID,
                           Gdk::CAP_NOT_LAST,
                           Gdk::JOIN_MITER );
    m_old_progress_ticks = tick;

    auto_scroll_horz();
}

void perfroll::draw_sequence_on( Glib::RefPtr<Gdk::Drawable> a_draw, int a_sequence )
{
    long tick_on;
    long tick_off;
    long offset;
    bool selected;

    long tick_offset = m_4bar_offset * c_ppqn * 16;
    long x_offset = tick_offset / m_perf_scale_x;

    if ( a_sequence < c_total_seqs )
    {
        if ( m_mainperf->is_active( a_sequence ))
        {
            m_sequence_active[a_sequence] = true;

            sequence *seq =  m_mainperf->get_sequence( a_sequence );

            seq->reset_draw_trigger_marker();

            a_sequence -= m_sequence_offset;

            long sequence_length = seq->get_length();
            int length_w = sequence_length / m_perf_scale_x;

            while ( seq->get_next_trigger( &tick_on, &tick_off, &selected, &offset  ))
            {
                if ( tick_off > 0 )
                {
                    long x_on  = tick_on  / m_perf_scale_x;
                    long x_off = tick_off / m_perf_scale_x;
                    int  w     = x_off - x_on + 1;

                    int x = x_on;
                    int y = c_names_y * a_sequence + 1;  // + 2
                    int h = c_names_y - 2; // - 4

                    // adjust to screen corrids
                    x = x - x_offset;

                    if ( selected )
                        m_gc->set_foreground(m_grey);
                    else
                        m_gc->set_foreground(m_white);

                    /* main trigger box */
                    a_draw->draw_rectangle(m_gc,true,
                                           x,
                                           y,
                                           w,
                                           h );
                    /* trigger outline */
                    m_gc->set_foreground(m_black);
                    a_draw->draw_rectangle(m_gc,false,
                                           x,
                                           y,
                                           w,
                                           h );

                    /* little seq grab handle - left hand side */
                    m_gc->set_foreground(m_black);
                    a_draw->draw_rectangle(m_gc,false,
                                           x,
                                           y,
                                           c_perfroll_size_box_w,
                                           c_perfroll_size_box_w );

                    /* seq grab handle - right side */
                    a_draw->draw_rectangle(m_gc,false,
                                           x+w-c_perfroll_size_box_w,
                                           y+h-c_perfroll_size_box_w,
                                           c_perfroll_size_box_w,
                                           c_perfroll_size_box_w );

                    m_gc->set_foreground(m_black);

                    long length_marker_first_tick = ( tick_on - (tick_on % sequence_length) + (offset % sequence_length) - sequence_length);

                    long tick_marker = length_marker_first_tick;

                    while ( tick_marker < tick_off )
                    {
                        long tick_marker_x = (tick_marker / m_perf_scale_x) - x_offset;

                        if ( tick_marker > tick_on )
                        {
                            m_gc->set_foreground(m_lt_grey);
                            a_draw->draw_rectangle(m_gc,true,
                                                   tick_marker_x,
                                                   y+4,
                                                   1,
                                                   h-8 );
                        }

                        int lowest_note = seq->get_lowest_note_event( );
                        int highest_note = seq->get_highest_note_event( );

                        int height = highest_note - lowest_note;
                        height += 2;

                        int length = seq->get_length( );

                        long tick_s;
                        long tick_f;
                        int note;

                        bool selected;

                        int velocity;
                        draw_type dt;

                        seq->reset_draw_marker();

                        m_gc->set_foreground(m_black);
                        while ( (dt = seq->get_next_note_event( &tick_s, &tick_f, &note,
                                                                &selected, &velocity )) != DRAW_FIN )
                        {
                            int note_y = ((c_names_y-6) -
                                          ((c_names_y-6)  * (note - lowest_note)) / height) + 1;

                            int tick_s_x = ((tick_s * length_w)  / length) + tick_marker_x;
                            int tick_f_x = ((tick_f * length_w)  / length) + tick_marker_x;

                            if ( dt == DRAW_NOTE_ON || dt == DRAW_NOTE_OFF )
                                tick_f_x = tick_s_x + 1;
                            if ( tick_f_x <= tick_s_x )
                                tick_f_x = tick_s_x + 1;

                            if ( tick_s_x < x )
                            {
                                tick_s_x = x;
                            }

                            if ( tick_f_x > x + w )
                            {
                                tick_f_x = x + w;
                            }

                            /*
                                    [           ]
                             -----------
                                             ---------
                                   ----------------
                            ------                      ------
                            */

                            if ( tick_f_x >= x && tick_s_x <= x+w )
                                m_pixmap->draw_line(m_gc, tick_s_x,
                                                    y + note_y,
                                                    tick_f_x,
                                                    y + note_y );
                        }

                        tick_marker += sequence_length;
                    }
                }
            }
        }
    }
}

void perfroll::draw_background_on( Glib::RefPtr<Gdk::Drawable> a_draw, int a_sequence )
{
    long tick_offset = m_4bar_offset * c_ppqn * 16;
    long first_measure = tick_offset / m_measure_length;

    a_sequence -= m_sequence_offset;

    int y = c_names_y * a_sequence;
    int h = c_names_y;

    m_gc->set_foreground(m_white);
    a_draw->draw_rectangle(m_gc,true,
                           0,
                           y,
                           m_window_x,
                           h );

    m_gc->set_foreground(m_black);
    for ( int i = first_measure;
            i < first_measure +
            (m_window_x * m_perf_scale_x /
             (m_measure_length)) + 1;
            i++ )
    {
        int x_pos = ((i * m_measure_length) - tick_offset) / m_perf_scale_x;

        a_draw->draw_drawable(m_gc, m_background,
                              0,
                              0,
                              x_pos,
                              y,
                              c_perfroll_background_x,
                              c_names_y );
    }
}

bool
perfroll::on_expose_event(GdkEventExpose* e)
{
    int y_s = e->area.y / c_names_y;
    int y_f = (e->area.y  + e->area.height) / c_names_y;

    for ( int y=y_s; y<=y_f; y++ )
    {
        /*
        for ( int x=x_s; x<=x_f; x++ ){

            m_pixmap->draw_drawable(m_gc, m_background,
        			 0,
        			 0,
        			 x * c_perfroll_background_x,
        			 c_names_y * y,
        			 c_perfroll_background_x,
        			 c_names_y );
        }

               */

        draw_background_on(m_pixmap, y + m_sequence_offset );
        draw_sequence_on(m_pixmap, y + m_sequence_offset );
    }

    m_window->draw_drawable( m_gc, m_pixmap,
                             e->area.x,
                             e->area.y,
                             e->area.x,
                             e->area.y,
                             e->area.width,
                             e->area.height );
    return true;
}

void
perfroll::redraw_dirty_sequences()
{
    bool draw = false;

    int y_s = 0;
    int y_f = m_window_y / c_names_y;

    for ( int y=y_s; y<=y_f; y++ )
    {
        int seq = y + m_sequence_offset; // 4am

        bool dirty = (m_mainperf->is_dirty_perf(seq ));

        if (dirty)
        {
            draw_background_on(m_pixmap,seq );
            draw_sequence_on(m_pixmap,seq );
            draw = true;
        }
    }

    if ( draw )
        m_window->draw_drawable( m_gc, m_pixmap,
                                 0,
                                 0,
                                 0,
                                 0,
                                 m_window_x,
                                 m_window_y );
}

void
perfroll::draw_drawable_row( Glib::RefPtr<Gdk::Drawable> a_dest, Glib::RefPtr<Gdk::Drawable> a_src,  long a_y )
{
    if( a_y < 0) // if user scrolled up off the window
        return;

    int s = a_y / c_names_y;
    a_dest->draw_drawable(m_gc, a_src,
                          0,
                          c_names_y * s,
                          0,
                          c_names_y * s,
                          m_window_x,
                          c_names_y );
}

bool
perfroll::on_button_press_event(GdkEventButton* a_ev)
{
    if(!trans_button_press) // to avoid double button press on normal seq32 method
    {
        transport_follow = m_mainperf->get_follow_transport();
        m_mainperf->set_follow_transport(false);
        trans_button_press = true;
    }

    return m_interaction->on_button_press_event(a_ev, *this);
}

bool
perfroll::on_button_release_event(GdkEventButton* a_ev)
{
    bool result;
    result = m_interaction->on_button_release_event(a_ev, *this);

    m_mainperf->set_follow_transport(transport_follow);
    trans_button_press = false;

    return result;
}

void
perfroll::auto_scroll_horz()
{
    if(!m_mainperf->get_follow_transport())
        return;

    if(m_zoom >= c_perf_scale_x)
    {
        double progress = (double)m_mainperf->get_tick()/m_zoom/c_ppen;

        int zoom_ratio = m_zoom/c_perf_scale_x;

        progress *= zoom_ratio;

        int offset = zoom_ratio;
        if(zoom_ratio != 1)
            offset *= -2;

        double page_size_adjust = (m_hadjust->get_page_size()/zoom_ratio)/2;
        double get_value_adjust = m_hadjust->get_value()*zoom_ratio;

        if(progress > page_size_adjust || (get_value_adjust > progress))
            m_hadjust->set_value(progress - page_size_adjust + offset);

        return;
    }

    long progress_tick = m_mainperf->get_tick();
    long tick_offset = m_4bar_offset * c_ppqn * 16;

    int progress_x =     ( progress_tick - tick_offset ) / m_zoom  + 100;
    int page = progress_x / m_window_x;

    if (page != 0 || progress_x < 0)
    {
        double left_tick = (double) progress_tick /m_zoom/c_ppen;

        switch(m_zoom)
        {
        case 8:
            m_hadjust->set_value(left_tick / 4);
            break;
        case 16:
            m_hadjust->set_value(left_tick / 2 );
            break;
/*        case 32:
            m_hadjust->set_value(left_tick );
            break;
        case 64:
            m_hadjust->set_value(left_tick * 2 );
            break;
        case 128:
            m_hadjust->set_value(left_tick * 4 );
            break;*/
        default:
            break;
        }
    }
}

bool
perfroll::on_scroll_event( GdkEventScroll* a_ev )
{
    guint modifiers;    // Used to filter out caps/num lock etc.
    modifiers = gtk_accelerator_get_default_mod_mask ();

    if ((a_ev->state & modifiers) == GDK_CONTROL_MASK)
    {
        if (a_ev->direction == GDK_SCROLL_DOWN)
        {
            m_perfedit->set_zoom(m_zoom*2);
        }
        else if (a_ev->direction == GDK_SCROLL_UP)
        {
            m_perfedit->set_zoom(m_zoom/2);
        }
        return true;
    }

    if ((a_ev->state & modifiers) == GDK_SHIFT_MASK)
    {
        double val = m_hadjust->get_value();

        if ( a_ev->direction == GDK_SCROLL_UP )
        {
            val -= m_hadjust->get_step_increment();
        }
        else if ( a_ev->direction == GDK_SCROLL_DOWN )
        {
            val += m_hadjust->get_step_increment();
        }

        m_hadjust->clamp_page(val, val + m_hadjust->get_page_size());
    }
    else
    {
        double val = m_vadjust->get_value();

        if ( a_ev->direction == GDK_SCROLL_UP )
        {
            val -= m_vadjust->get_step_increment();
        }
        else if ( a_ev->direction == GDK_SCROLL_DOWN )
        {
            val += m_vadjust->get_step_increment();
        }

        m_vadjust->clamp_page(val, val + m_vadjust->get_page_size());
    }
    return true;
}

bool
perfroll::on_motion_notify_event(GdkEventMotion* a_ev)
{
    return m_interaction->on_motion_notify_event(a_ev, *this);
}

bool
perfroll::on_key_press_event(GdkEventKey* a_p0)
{
    if (a_p0->keyval == m_mainperf->m_key_pointer)         /* Move to mouse position */
    {
        int x = 0;
        int y = 0;

        long a_tick = 0;

        get_pointer(x, y);
        if(x < 0)
            x = 0;
        snap_x(&x);
        convert_x(x, &a_tick);

        if(m_mainperf->is_jack_running())
        {
            m_mainperf->set_reposition();
            m_mainperf->set_starting_tick(a_tick);
            m_mainperf->position_jack(true, a_tick);
        }
        else
        {
            m_mainperf->set_reposition();
            m_mainperf->set_starting_tick(a_tick);
        }

        return true;
    }

    bool ret = false;

    if ( m_mainperf->is_active( m_drop_sequence))
    {
        if ( a_p0->type == GDK_KEY_PRESS )
        {
            if ( a_p0->keyval ==  GDK_Delete || a_p0->keyval == GDK_BackSpace )
            {
                m_mainperf->push_trigger_undo(m_drop_sequence);
                m_mainperf->get_sequence( m_drop_sequence )->del_selected_trigger();

                ret = true;
            }

            if ( a_p0->state & GDK_CONTROL_MASK )
            {
                /* cut */
                if ( a_p0->keyval == GDK_x || a_p0->keyval == GDK_X )
                {
                    m_mainperf->push_trigger_undo(m_drop_sequence);
                    m_mainperf->get_sequence( m_drop_sequence )->cut_selected_trigger();
                    ret = true;
                }
                /* copy */
                if ( a_p0->keyval == GDK_c || a_p0->keyval == GDK_C )
                {
                    m_mainperf->get_sequence( m_drop_sequence )->copy_selected_trigger();
                    ret = true;
                }

                /* paste */
                if ( a_p0->keyval == GDK_v || a_p0->keyval == GDK_V )
                {
                    m_mainperf->push_trigger_undo(m_drop_sequence);
                    m_mainperf->get_sequence( m_drop_sequence )->paste_trigger();
                    ret = true;
                }
            }
        }
    }

    if ( ret == true )
    {
        fill_background_pixmap();
        queue_draw();
        return true;
    }
    else
        return false;
}

/* performs a 'snap' on x */
void
perfroll::snap_x( int *a_x )
{
    // snap = number pulses to snap to
    // m_scale = number of pulses per pixel
    //	so snap / m_scale  = number pixels to snap to

    int mod = (m_snap / m_perf_scale_x );

    if ( mod <= 0 )
        mod = 1;

    *a_x = *a_x - (*a_x % mod );
}

void
perfroll::convert_x( int a_x, long *a_tick )
{
    long tick_offset = m_4bar_offset * c_ppqn * 16;
    *a_tick = a_x * m_perf_scale_x;
    *a_tick += tick_offset;
}

void
perfroll::convert_xy( int a_x, int a_y, long *a_tick, int *a_seq)
{
    long tick_offset = m_4bar_offset * c_ppqn * 16;

    *a_tick = a_x * m_perf_scale_x;
    *a_seq = a_y / c_names_y;

    *a_tick += tick_offset;
    *a_seq  += m_sequence_offset;

    if ( *a_seq >= c_total_seqs )
        *a_seq = c_total_seqs - 1;

    if ( *a_seq < 0 )
        *a_seq = 0;
}

bool
perfroll::on_focus_in_event(GdkEventFocus*)
{
    set_flags(Gtk::HAS_FOCUS);
    return false;
}

bool
perfroll::on_focus_out_event(GdkEventFocus*)
{
    unset_flags(Gtk::HAS_FOCUS);
    return false;
}

void
perfroll::on_size_allocate(Gtk::Allocation& a_r )
{
    Gtk::DrawingArea::on_size_allocate( a_r );

    m_window_x = a_r.get_width();
    m_window_y = a_r.get_height();

    update_sizes();
}

void
perfroll::on_size_request(GtkRequisition* a_r )
{
}

void
perfroll::split_trigger( int a_sequence, long a_tick )
{
    m_mainperf->push_trigger_undo(a_sequence);
    m_mainperf->get_sequence( a_sequence )->split_trigger( a_tick );

    draw_background_on( m_pixmap, a_sequence );
    draw_sequence_on( m_pixmap, a_sequence );
    draw_drawable_row( m_window, m_pixmap, m_drop_y);
}

void
perfroll::set_zoom (int a_zoom)
{
    if (m_perfedit->zoom_check(a_zoom))
    {
        m_zoom = a_zoom;
        m_perf_scale_x = m_zoom;

        if (m_perf_scale_x == 0)
            m_perf_scale_x = 1;

        fill_background_pixmap();
        update_sizes();
    }
}

int 
perfroll::get_drop_sequence()
{
    return m_drop_sequence;
}