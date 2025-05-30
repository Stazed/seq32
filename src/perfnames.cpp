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
#include "perfnames.h"
#include "font.h"
#include "mainwnd.h"
#include "themes.h"
#include "pixmaps/track_play.xpm"
#include "pixmaps/track_solo.xpm"
#include "pixmaps/track_mute.xpm"

perfnames::perfnames( perform *a_perf, mainwnd *a_main, Glib::RefPtr<Adjustment> a_vadjust ):
    seqmenu(a_perf, a_main ),
    m_mainperf(a_perf),
    m_vadjust(a_vadjust),
    m_redraw_tracks(false),
    m_sequence_offset(0),
    m_names_y(c_names_y),
    m_vertical_zoom(c_default_vertical_zoom),
    m_button_down(false),
    m_moving(false),
    m_old_seq(0)
{
    Gtk::Allocation allocation = get_allocation();
    m_surface = Cairo::ImageSurface::create(
        Cairo::Format::FORMAT_ARGB32,
        allocation.get_width(),
        allocation.get_height()
    );

    add_events( Gdk::BUTTON_PRESS_MASK |
                Gdk::BUTTON_RELEASE_MASK |
                Gdk::BUTTON_MOTION_MASK |
                Gdk::SCROLL_MASK );

    /* set default size */
    set_size_request( c_names_x, 100 );

    m_vadjust->signal_value_changed().connect( mem_fun( *(this), &perfnames::change_vert ));

    set_double_buffered( false );

    for( int i=0; i<c_total_seqs; ++i )
        m_sequence_active[i]=false;
}

void
perfnames::on_realize()
{
    // we need to do the default realize
    Gtk::DrawingArea::on_realize();
}

void
perfnames::change_vert()
{
    if ( m_sequence_offset != (int) m_vadjust->get_value() )
    {
        m_sequence_offset = (int) m_vadjust->get_value();
        m_redraw_tracks = true;
        queue_draw();
    }
}

void
perfnames::redraw( int sequence )
{
    draw_sequence( sequence);
}

void
perfnames::draw_sequence( int sequence )
{
    int i = sequence - m_sequence_offset;
    Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(m_surface);

    cr->set_operator(Cairo::OPERATOR_CLEAR);
    cr->rectangle(0, (m_names_y * i), m_window_x - 1, m_names_y);
    cr->paint_with_alpha(0.0);
    cr->set_operator(Cairo::OPERATOR_OVER);

    if ( sequence < c_total_seqs )
    {
        cr->set_source_rgb(c_back_black.r, c_back_black.g, c_back_black.b);
        cr->rectangle(0, (m_names_y * i), m_window_x - 1, m_names_y);
        cr->stroke_preserve();
        cr->fill();

        if ( sequence % c_seqs_in_set == 0 )
        {
            char screen_set[10];
            snprintf(screen_set, sizeof(screen_set), "%2d", sequence / c_seqs_in_set );

            /* The black background */
            cr->set_source_rgb(c_back_black.r, c_back_black.g, c_back_black.b);
            cr->rectangle(0, (m_names_y * i) + 2, c_perf_ss_width, m_names_y - 5 );
            cr->stroke_preserve();
            cr->fill();

            /* print the screen_set number */
            p_font_renderer->render_string_on_drawable(1, (m_names_y * i) + 6, cr, screen_set, font::WHITE);
        }
        else    // no screen set number
        {
            cr->set_source_rgb(c_back_medium_grey.r, c_back_medium_grey.g, c_back_medium_grey.b);
            cr->rectangle(1, (m_names_y * i) + 3, c_perf_ss_width, m_names_y - 5 );
            cr->stroke_preserve();
            cr->fill();
        }

        /* Track name background color based on focus */
        bool is_focus = false;
        if ( m_mainperf->is_active( sequence ))
        {
            if( m_mainperf->is_focus_track( sequence ))
            {
                is_focus = true;
                cr->set_source_rgba(c_track_color.r, c_track_color.g, c_track_color.b, 1.0);
            }
            else
            {
                cr->set_source_rgba(c_track_color.r, c_track_color.g, c_track_color.b, 0.5);
            }
        }
        else    // inactive
        {
            cr->set_source_rgb(c_back_light_grey.r, c_back_light_grey.g, c_back_light_grey.b);
        }

        cr->set_line_width(2.0);
        cr->rectangle(c_perf_ss_width + 3,
                        (m_names_y * i) + 3,
                        m_window_x - 4 - c_perf_ss_width,
                        m_names_y - 5  );
        cr->stroke_preserve();
        cr->fill();

        if ( m_mainperf->is_active( sequence ))
        {
            m_sequence_active[sequence]=true;

            /* names */
            char name[50];
            snprintf(name, sizeof(name), "%-20.20s",
                     m_mainperf->get_sequence(sequence)->get_name());

            /* Set the label color based on focus */
            font::Color font_color = font::WHITE;

            if(is_focus)
            {
                font_color = font::BLACK;
            }

            /* Print the sequence name */
            p_font_renderer->render_string_on_drawable(5 + c_perf_ss_width,
                                                       (m_names_y * i) + 2,
                                                       cr, name, font_color,
                                                       c_default_horizontal_zoom,
                                                       m_vertical_zoom);

            /* Buses */
            char str[20];
            snprintf(str, sizeof(str),
                     "%d-%d %ld/%ld",
                     m_mainperf->get_sequence(sequence)->get_midi_bus(),
                     m_mainperf->get_sequence(sequence)->get_midi_channel()+1,
                     m_mainperf->get_sequence(sequence)->get_bp_measure(),
                     m_mainperf->get_sequence(sequence)->get_bw() );

            /* Print the sequence bus, etc */
            p_font_renderer->render_string_on_drawable(5 + c_perf_ss_width,
                                                       m_names_y * i + (12 * m_vertical_zoom),
                                                       cr, str, font_color,
                                                       c_default_horizontal_zoom,
                                                       m_vertical_zoom);

            bool solo = m_mainperf->get_sequence(sequence)->get_song_solo();
            bool muted = m_mainperf->get_sequence(sequence)->get_song_mute();

            if(solo)
            {
                m_pixbuf = Gdk::Pixbuf::create_from_xpm_data(track_solo_xpm);
                m_pixbuf = m_pixbuf->scale_simple(c_mute_x, (int) (c_mute_y * m_vertical_zoom), Gdk::INTERP_BILINEAR);
                Gdk::Cairo::set_source_pixbuf (cr, m_pixbuf, m_window_x - 17, (m_names_y * i) + 3);
                cr->paint();
            }
            else if (muted)
            {
                m_pixbuf = Gdk::Pixbuf::create_from_xpm_data(track_mute_xpm);
                m_pixbuf = m_pixbuf->scale_simple(c_mute_x, (int) (c_mute_y * m_vertical_zoom), Gdk::INTERP_BILINEAR);
                Gdk::Cairo::set_source_pixbuf (cr, m_pixbuf, m_window_x - 17, (m_names_y * i) + 3);
                cr->paint();
            }
            else
            {
                m_pixbuf = Gdk::Pixbuf::create_from_xpm_data(track_play_xpm);
                m_pixbuf = m_pixbuf->scale_simple(c_mute_x, (int) (c_mute_y * m_vertical_zoom), Gdk::INTERP_BILINEAR);
                Gdk::Cairo::set_source_pixbuf (cr, m_pixbuf, m_window_x - 17, (m_names_y * i) + 3);
                cr->paint();
            }
        }   // Active sequence
    }
    else    // if you scroll down to the very bottom
    {
        cr->set_source_rgb(c_back_black.r, c_back_black.g, c_back_black.b);
        cr->rectangle(0, (m_names_y * i) + 1, m_window_x, m_names_y);
        cr->stroke_preserve();
        cr->fill();
    }
}

bool
perfnames::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();

    // resize handler
    if (width != m_surface->get_width() || height != m_surface->get_height())
    {
        m_surface = Cairo::ImageSurface::create(
            Cairo::Format::FORMAT_ARGB32,
            allocation.get_width(),
            allocation.get_height()
        );
        m_redraw_tracks = true;
    }

    if (m_redraw_tracks)
    {
        m_redraw_tracks = false;
        int trks = (m_window_y / m_names_y) + 1;

        for ( int i = 0; i < trks; i++ )
        {
            int seq = i + m_sequence_offset;
            draw_sequence(seq);
        }
    }

    /* Draw the new background */
    cr->set_source(m_surface, 0.0, 0.0);
    cr->paint();

    return true;
}

void
perfnames::convert_y( int a_y, int *a_seq)
{
    *a_seq = a_y / m_names_y;
    *a_seq  += m_sequence_offset;

    if ( *a_seq >= c_total_seqs )
        *a_seq = c_total_seqs - 1;

    if ( *a_seq < 0 )
        *a_seq = 0;
}

bool
perfnames::on_button_press_event(GdkEventButton *a_e)
{
    int sequence;
    int y = (int) a_e->y;
    convert_y( y, &sequence );

    m_current_seq = sequence;
    
    /*      left mouse button     */
    if ( a_e->button == 1 &&  m_mainperf->is_active( sequence ) )
    {
        m_button_down = true;
    }

    /* Middle mouse button toggle solo track */
    if ( a_e->button == 2 &&  m_mainperf->is_active( sequence ) )
    {
        bool solo = m_mainperf->get_sequence(sequence)->get_song_solo();
        m_mainperf->get_sequence(sequence)->set_song_solo( !solo );
        /* we want to shut off mute if we are setting solo, so if solo was
         * off (false) then we are turning it on(toggle), so unset the mute. */
        if (!solo)
            m_mainperf->get_sequence(sequence)->set_song_mute( solo );
        
        check_global_solo_tracks();
    }

    return true;
}

bool
perfnames::on_button_release_event(GdkEventButton* p0)
{
    int sequence;
    int y = (int) p0->y;

    convert_y( y, &sequence );
    m_current_seq = sequence;
    
    m_button_down = false;
    
    /* left mouse button & not moving - toggle mute  */
    if ( p0->button == 1 && m_mainperf->is_active( m_current_seq ) && !m_moving )
    {
        bool muted = m_mainperf->get_sequence(m_current_seq)->get_song_mute();
        m_mainperf->get_sequence(m_current_seq)->set_song_mute( !muted );
       /* we want to shut off solo if we are setting mute, so if mute was
         * off (false) then we are turning it on(toggle), so unset the solo. */
        if (!muted)
            m_mainperf->get_sequence(m_current_seq)->set_song_solo( muted );
        
        check_global_solo_tracks();
    }
    
    /* Left button and moving */
    if ( p0->button == 1 && m_moving )
    {
        m_moving = false;
        
        /* If we land on in-active track, then move or copy to new location */
        if ( ! m_mainperf->is_active( m_current_seq ) )
        {
            m_mainperf->new_sequence( m_current_seq  );
            *(m_mainperf->get_sequence( m_current_seq )) = m_moving_seq;
            m_mainperf->get_sequence( m_current_seq )->unselect_triggers( );
            m_mainperf->get_sequence(m_current_seq)->set_dirty();

            /* Alt key, put the original back to old location - since this is a copy, not move */
            if ( p0->state & GDK_MOD1_MASK)
            {
                m_mainperf->new_sequence( m_old_seq  );
                *(m_mainperf->get_sequence( m_old_seq )) = m_moving_seq;
                m_mainperf->get_sequence(m_old_seq)->set_dirty();
            }
        }
        /* If we did land on an active track and it is not being edited, then swap places. */
        else if ( !m_mainperf->is_sequence_in_edit( m_current_seq ) )
        {
            m_clipboard = *(m_mainperf->get_sequence( m_current_seq ));      // hold the current for swap to old location
            m_mainperf->new_sequence( m_old_seq  );                          // The old location
            *(m_mainperf->get_sequence( m_old_seq )) = m_clipboard;          // put the current track into the old location
            m_mainperf->get_sequence( m_old_seq )->unselect_triggers( );
            m_mainperf->get_sequence(m_old_seq)->set_dirty();

            m_mainperf->delete_sequence( m_current_seq );                    // delete the current for replacement
            m_mainperf->new_sequence( m_current_seq  );                      // add a new blank one
            *(m_mainperf->get_sequence( m_current_seq )) = m_moving_seq;     // replace with the old
            m_mainperf->get_sequence( m_current_seq )->unselect_triggers( );
            m_mainperf->get_sequence(m_current_seq)->set_dirty();
        }
       /* They landed on another track but it is being edited, so ignore the move 
         * and put the old track back to original location. */
        else
        {
            m_mainperf->new_sequence( m_old_seq  );
            *(m_mainperf->get_sequence( m_old_seq )) = m_moving_seq;
            m_mainperf->get_sequence(m_old_seq)->set_dirty();
            sequence_is_being_edited();
        }  
    }
    
    /* launch menu - right mouse button  */
    if ( p0->button == 3 )
    {
        popup_menu();
    }

    return false;
}

bool
perfnames::on_motion_notify_event(GdkEventMotion* a_ev)
{
    int sequence;
    int y = (int) a_ev->y;

    convert_y( y, &sequence );
    
    /* If we are dragging off the original sequence then we are trying to move. */
    if ( m_button_down )
    {
        if ( sequence != m_current_seq && !m_moving &&
                !m_mainperf->is_sequence_in_edit( m_current_seq ) )
        {
            if ( m_mainperf->is_active( m_current_seq ))
            {
                m_old_seq = m_current_seq;
                m_moving = true;

                m_moving_seq = *(m_mainperf->get_sequence( m_current_seq ));
                m_mainperf->delete_sequence( m_current_seq );
            }
        }
    }  

    return true;
}

bool
perfnames::on_scroll_event( GdkEventScroll* a_ev )
{
    guint modifiers;    // Used to filter out caps/num lock etc.
    modifiers = gtk_accelerator_get_default_mod_mask ();

    /* Vertical zoom ALT key */
    if ((a_ev->state & modifiers) == GDK_MOD1_MASK)
    {
        if (a_ev->direction == GDK_SCROLL_DOWN)
        {
            m_mainwnd->set_vertical_zoom(m_vertical_zoom + c_vertical_zoom_step);
        }
        else if (a_ev->direction == GDK_SCROLL_UP)
        {
            m_mainwnd->set_vertical_zoom(m_vertical_zoom - c_vertical_zoom_step);
        }
        return true;
    }
    
    double val = m_vadjust->get_value();

    if (  a_ev->direction == GDK_SCROLL_UP )
    {
        val -= m_vadjust->get_step_increment();
    }
    if (  a_ev->direction == GDK_SCROLL_DOWN )
    {
        val += m_vadjust->get_step_increment();
    }

    m_vadjust->clamp_page( val, val + m_vadjust->get_page_size());
    return true;
}

void
perfnames::on_size_allocate(Gtk::Allocation &a_r )
{
    Gtk::DrawingArea::on_size_allocate( a_r );

    m_window_x = a_r.get_width();
    m_window_y = a_r.get_height();
}

void
perfnames::redraw_dirty_sequences()
{
    int y_s = 0;
    int y_f = m_window_y / m_names_y;

    for ( int y=y_s; y<=y_f; y++ )
    {
        int seq = y + m_sequence_offset;

        if ( seq < c_total_seqs)
        {
            bool dirty = (m_mainperf->is_dirty_names( seq ));

            if (dirty)
            {
                draw_sequence( seq );
                queue_draw();
            }
        }
    }
}

void
perfnames::set_vertical_zoom (float a_zoom)
{
    if (m_mainwnd->zoom_check_vertical(a_zoom))
    {
        m_vertical_zoom = a_zoom;
        m_names_y = (int) (c_names_y * m_vertical_zoom);
        m_redraw_tracks = true;
        queue_draw();
    }
}

void
perfnames::check_global_solo_tracks()
{
    global_solo_track_set = false;
    for (int i = 0; i < c_max_sequence; i++)
    {
        if (m_mainperf->is_active(i))
        {
            if (m_mainperf->get_sequence(i)->get_song_solo())
            {
                global_solo_track_set = true;
                break;
            }
        }
    }
}

void
perfnames::sequence_is_being_edited()
{
    Glib::ustring query_str = "Cannot swap sequences if being edited!";
    
    Gtk::MessageDialog dialog( query_str, false,
                              Gtk::MESSAGE_INFO,
                              Gtk::BUTTONS_OK, true);

    dialog.run();
}
