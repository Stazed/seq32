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
                    Glib::RefPtr<Adjustment> a_hadjust,
                    Glib::RefPtr<Adjustment> a_vadjust  ) :
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
    m_redraw_tracks(false),
    m_zoom(c_perf_scale_x)
{
    Gtk::Allocation allocation = get_allocation();
    m_surface_track = Cairo::ImageSurface::create(
        Cairo::Format::FORMAT_ARGB32,
        allocation.get_width(),
        allocation.get_height()
    );

    m_surface_background = Cairo::ImageSurface::create(
        Cairo::Format::FORMAT_ARGB32,
        allocation.get_width(),
        allocation.get_height()
    );

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
        m_redraw_tracks = true;
    }
}

void
perfroll::change_vert()
{
    if ( m_sequence_offset != (int) m_vadjust->get_value() )
    {
        m_sequence_offset = (int) m_vadjust->get_value();
        m_redraw_tracks = true;
    }
}

void
perfroll::on_realize()
{
    // we need to do the default realize
    Gtk::DrawingArea::on_realize();

    set_can_focus();
    update_sizes();

    m_hadjust->signal_value_changed().connect( mem_fun( *this, &perfroll::change_horz ));
    m_vadjust->signal_value_changed().connect( mem_fun( *this, &perfroll::change_vert ));

    // resize handler
    if (c_perfroll_background_x != m_surface_background->get_width() || c_names_y != m_surface_background->get_height())
    {
        m_surface_background = Cairo::ImageSurface::create(
            Cairo::Format::FORMAT_ARGB32,
            c_perfroll_background_x, c_names_y
        );
    }
    
    if (m_window_x != m_surface_track->get_width() || m_window_y != m_surface_track->get_height())
    {
        m_surface_track = Cairo::ImageSurface::create(
            Cairo::Format::FORMAT_ARGB32,
                m_window_x,  m_window_y
        );
    }

    /* and fill the background ( dotted lines n' such ) */
    fill_background_surface();
    m_redraw_tracks = true;
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
    
    if ( get_realized() )
    {
        m_surface_track = Cairo::ImageSurface::create(
            Cairo::Format::FORMAT_ARGB32,
            m_window_x,  m_window_y
        );
    }

    m_redraw_tracks = true;
}

void
perfroll::increment_size()
{
    m_roll_length_ticks += (c_ppqn * 512);
    update_sizes( );
}

/* updates background */
void
perfroll::fill_background_surface()
{
    Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(m_surface_background);

    cr->set_operator(Cairo::OPERATOR_CLEAR);
    cr->rectangle(0, 0, c_perfroll_background_x,  c_names_y);
    cr->paint_with_alpha(1.0);
    cr->set_operator(Cairo::OPERATOR_OVER);

    /* clear background */
    cr->set_source_rgb(1.0, 1.0, 1.0);    // White FIXME
    cr->set_line_width(1.0);
    cr->rectangle(0.0, 0.0, c_perfroll_background_x, c_names_y);
    cr->stroke_preserve();
    cr->fill();

    /* draw horizontal grey lines */
    cr->set_source_rgb(0.6, 0.6, 0.6);    // Grey  FIXME
    static const std::vector<double> dashed = {1.0};
    static const std::vector<double> clear;
    cr->set_line_width(1.0);
    cr->set_dash(dashed, 1.0);
    cr->set_line_join(Cairo::LINE_JOIN_MITER);
    cr->move_to(0.0, 0.0);
    cr->line_to(c_perfroll_background_x, 0.0);
    cr->stroke();

    /* draw vertical lines */
    int beats = m_measure_length / m_beat_length;
    for ( int i = 0; i < beats ; )
    {
        if ( i == 0 )
        {
            cr->set_dash(clear, 0.0);        // Clear the dashes
            cr->set_line_width(2.0);
            cr->set_line_join(Cairo::LINE_JOIN_MITER);
        }
        else
        {
            cr->set_line_width(1.0);
            cr->set_dash(dashed, 1.0);
            cr->set_line_join(Cairo::LINE_JOIN_MITER);
        }

        /* solid line on every beat */
        cr->move_to(i * m_beat_length / m_perf_scale_x, 0.0);
        cr->line_to(i * m_beat_length / m_perf_scale_x, c_names_y);
        cr->stroke();

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
}

/* simply sets the snap member */
void
perfroll::set_guides( int a_snap, int a_measure, int a_beat )
{
    m_snap = a_snap;
    m_measure_length = a_measure;
    m_beat_length = a_beat;

    if ( get_realized() )
    {
        fill_background_surface();
        m_redraw_tracks = true;
    }
}

void
perfroll::draw_progress()
{
    if (global_is_running)
    {
        queue_draw();
    }
    else if (m_redraw_tracks)
    {
        queue_draw();
    }
}

bool 
perfroll::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    // resize handler
    if (c_perfroll_background_x != m_surface_background->get_width() || c_names_y != m_surface_background->get_height())
    {
        m_surface_background = Cairo::ImageSurface::create(
            Cairo::Format::FORMAT_ARGB32,
            c_perfroll_background_x, c_names_y
        );

        fill_background_surface();
        m_redraw_tracks = true;
    }

    if (m_window_x != m_surface_track->get_width() || m_window_y != m_surface_track->get_height())
    {
        m_surface_track = Cairo::ImageSurface::create(
            Cairo::Format::FORMAT_ARGB32,
                m_window_x,  m_window_y
        );

        m_redraw_tracks = true;
    }

    if (m_redraw_tracks)
    {
        m_redraw_tracks = false;
        int y_s = 0;
        int y_f = m_window_y / c_names_y;

        for ( int y=y_s; y<=y_f; y++ )
        {
            int sequence = y + m_sequence_offset;

            draw_background_on(sequence );
            draw_sequence_on(sequence);
        }
    }

    /* draw progress line */
    long tick = m_mainperf->get_tick();
    long tick_offset = m_4bar_offset * c_ppqn * 16;

    int progress_x = ( tick - tick_offset ) / m_perf_scale_x ;

    cr->set_source(m_surface_track, 0.0, 0.0);
    cr->paint();

    cr->set_source_rgb(0.0, 0.0, 0.0);            // Black  FIXME
    cr->set_line_width(2.0);
    cr->move_to(progress_x, 0.0);
    cr->line_to(progress_x, m_window_y);
    cr->stroke();

    auto_scroll_horz();

    return true;
}

void perfroll::draw_sequence_on( int a_sequence )
{
    long tick_on;
    long tick_off;
    long offset;
    bool selected;

    Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(m_surface_track);
    cr->set_operator(Cairo::OPERATOR_DEST);
    cr->set_operator(Cairo::OPERATOR_OVER);

    cr->set_line_width(1.0);

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
                    {
                        cr->set_source_rgb(0.6, 0.8, 1.0);    // blue FIXME
                    }
                    else
                    {
                        cr->set_source_rgb(1.0, 1.0, 1.0);    // White FIXME
                    }

                    /* main trigger box */
                    cr->rectangle(x, y, w, h);
                    cr->fill();

                    /* trigger outline */
                    cr->set_source_rgb(0.0, 0.0, 0.0);        // black FIXME
                    cr->rectangle(x, y, w, h);
                    cr->stroke();

                    /* resize handle - top left */
                    cr->set_source_rgb(0.0, 0.0, 0.0);        // black FIXME
                    cr->rectangle(x, y, c_perfroll_size_box_w, c_perfroll_size_box_w);
                    cr->stroke();

                    /* resize handle - bottom right */
                    cr->rectangle(x+w-c_perfroll_size_box_w, y+h-c_perfroll_size_box_w, c_perfroll_size_box_w, c_perfroll_size_box_w);
                    cr->stroke();

                    cr->set_source_rgb(0.0, 0.0, 0.0);        // black FIXME

                    long length_marker_first_tick = ( tick_on - (tick_on % sequence_length) + (offset % sequence_length) - sequence_length);

                    long tick_marker = length_marker_first_tick;

                    while ( tick_marker < tick_off )
                    {
                        long tick_marker_x = (tick_marker / m_perf_scale_x) - x_offset;

                        if ( tick_marker > tick_on )
                        {
                            if ( selected )
                            {
                                cr->set_source_rgb(1.0, 1.0, 1.0);    // White FIXME
                            }
                            else
                            {
                                cr->set_source_rgb(0.6, 0.6, 0.6);    // grey FIXME
                            }

                            cr->rectangle(tick_marker_x, (y + 4), 1.0, (h - 8) );
                            cr->fill();
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

                        cr->set_source_rgb(0.0, 0.0, 0.0);      // black FIXME

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
                             {
                                    cr->move_to(tick_s_x, y + note_y);
                                    cr->line_to(tick_f_x, y + note_y);
                                    cr->stroke();
                            }
                        }

                        tick_marker += sequence_length;
                    }
                }
            }
        }
    }
}

void perfroll::draw_background_on( int a_sequence )
{
    long tick_offset = m_4bar_offset * c_ppqn * 16;
    long first_measure = tick_offset / m_measure_length;

    a_sequence -= m_sequence_offset;

    int y = c_names_y * a_sequence;
    
    Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(m_surface_track);
    cr->set_operator(Cairo::OPERATOR_DEST);
    cr->set_operator(Cairo::OPERATOR_OVER);

    for ( int i = first_measure;
            i < first_measure +
            (m_window_x * m_perf_scale_x /
             (m_measure_length)) + 1;
            i++ )
    {
        int x_pos = ((i * m_measure_length) - tick_offset) / m_perf_scale_x;

        cr->set_source(m_surface_background, x_pos, y);
        cr->paint();
    }
}

void
perfroll::redraw_dirty_sequences()
{
    int y_s = 0;
    int y_f = m_window_y / c_names_y;

    for ( int y=y_s; y<=y_f; y++ )
    {
        int seq = y + m_sequence_offset; // 4am

        bool dirty = (m_mainperf->is_dirty_perf(seq ));

        if (dirty)
        {
            draw_background_on( seq );
            draw_sequence_on( seq );
            m_redraw_tracks = true;
        }
    }
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
            if ( a_p0->keyval ==  GDK_KEY_Delete || a_p0->keyval == GDK_KEY_BackSpace )
            {
                m_mainperf->push_trigger_undo(m_drop_sequence);
                m_mainperf->get_sequence( m_drop_sequence )->del_selected_trigger();

                ret = true;
            }

            if ( a_p0->state & GDK_CONTROL_MASK )
            {
                /* cut */
                if ( a_p0->keyval == GDK_KEY_x || a_p0->keyval == GDK_KEY_X )
                {
                    m_mainperf->push_trigger_undo(m_drop_sequence);
                    m_mainperf->get_sequence( m_drop_sequence )->cut_selected_trigger();
                    ret = true;
                }
                /* copy */
                if ( a_p0->keyval == GDK_KEY_c || a_p0->keyval == GDK_KEY_C )
                {
                    m_mainperf->get_sequence( m_drop_sequence )->copy_selected_trigger();
                    ret = true;
                }

                /* paste */
                if ( a_p0->keyval == GDK_KEY_v || a_p0->keyval == GDK_KEY_V )
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
        fill_background_surface();
        m_redraw_tracks = true;
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
    grab_focus();
    return false;
}

bool
perfroll::on_focus_out_event(GdkEventFocus*)
{
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

    draw_background_on( a_sequence );
    draw_sequence_on( a_sequence );
    m_redraw_tracks = true;
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

        fill_background_surface();
        update_sizes();
    }
}

int 
perfroll::get_drop_sequence()
{
    return m_drop_sequence;
}
