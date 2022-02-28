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

perfnames::perfnames( perform *a_perf, mainwnd *a_main, Glib::RefPtr<Adjustment> a_vadjust ):
    seqmenu(a_perf, a_main ),
    m_mainperf(a_perf),
    m_vadjust(a_vadjust),
    m_redraw_tracks(false),
    m_sequence_offset(0),
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

    Pango::FontDescription font;
    int text_width;
    int text_height;

    font.set_family(c_font);
    font.set_size((c_key_fontsize - 1) * Pango::SCALE);
    font.set_weight(Pango::WEIGHT_NORMAL);

    cr->set_operator(Cairo::OPERATOR_CLEAR);
    cr->rectangle(0, (c_names_y * i), m_window_x - 1, c_names_y);
    cr->paint_with_alpha(0.0);
    cr->set_operator(Cairo::OPERATOR_OVER);

    cr->set_line_width(1.0);

    if ( sequence < c_total_seqs )
    {
        cr->set_source_rgb(0.0, 0.0, 0.0);        // Black FIXME
        cr->rectangle(0, (c_names_y * i), m_window_x, c_names_y + 1);
        cr->stroke_preserve();
        cr->fill();

        if ( sequence % c_seqs_in_set == 0 )
        {
            char screen_set[10];
            snprintf(screen_set, sizeof(screen_set), "%2d", sequence / c_seqs_in_set );

            auto s = create_pango_layout(screen_set);
            s->set_font_description(font);
            s->get_pixel_size(text_width, text_height);
            
            cr->set_source_rgb(0.0, 0.0, 0.0);    // black FIXME
            cr->rectangle(0, c_names_y * i + 2, text_width, text_height );
            cr->stroke_preserve();
            cr->fill();
            
            /* print the screen_set number */
            cr->set_source_rgb(1.0, 1.0, 1.0);    // White FIXME
            cr->move_to(1, c_names_y * i + 2);

            s->show_in_cairo_context(cr);
        }
        else    // no screen set number
        {
            cr->set_source_rgb(1.0, 1.0, 1.0);    // White FIXME
            cr->rectangle(1, (c_names_y * (i)), (6*2), c_names_y - 1 );
            cr->stroke_preserve();
            cr->fill();
        }

        if ( m_mainperf->is_active( sequence ))
        {
            cr->set_source_rgb(1.0, 1.0, 1.0);    // White FIXME
        }
        else
        {
            cr->set_source_rgb(0.6, 0.6, 0.6);    // grey FIXME
        }
        
        cr->rectangle(6 * 2 + 3,
                        (c_names_y * i) + 1,
                        m_window_x - 3 - (6*2),
                        c_names_y - 1  );
        cr->stroke_preserve();
        cr->fill();

        if ( m_mainperf->is_active( sequence ))
        {
            m_sequence_active[sequence]=true;

            /* names */
            char name[50];
            snprintf(name, sizeof(name), "%-20.20s",
                     m_mainperf->get_sequence(sequence)->get_name());

            // set background for name
            auto n = create_pango_layout(name);
            font.set_size((c_key_fontsize - 2) * Pango::SCALE);
            n->set_font_description(font);
            n->get_pixel_size(text_width, text_height);
            cr->set_source_rgb(1.0, 1.0, 1.0);    // White FIXME

            // draw the white background for the name
            cr->rectangle(5 + 6*2, c_names_y * i + 12, text_width, 10.0);
            cr->stroke_preserve();
            cr->fill();

            cr->set_source_rgb(0.0, 0.0, 0.0);    // Black FIXME
            cr->move_to(5 + 6*2, c_names_y * i + 1);

            n->show_in_cairo_context(cr);

            /* Buses */
            char str[20];
            snprintf(str, sizeof(str),
                     "%d-%d %ld/%ld",
                     m_mainperf->get_sequence(sequence)->get_midi_bus(),
                     m_mainperf->get_sequence(sequence)->get_midi_channel()+1,
                     m_mainperf->get_sequence(sequence)->get_bp_measure(),
                     m_mainperf->get_sequence(sequence)->get_bw() );

            // set background for bus
            auto t = create_pango_layout(str);
            t->set_font_description(font);
            t->get_pixel_size(text_width, text_height);

            cr->set_source_rgb(1.0, 1.0, 1.0);    // White FIXME
            cr->set_line_width(1.0);

            // draw the white background for the bus
            cr->rectangle(5 + 6*2, c_names_y * i + 10, text_width, 6.0);
            cr->stroke_preserve();
            cr->fill();

            cr->set_source_rgb(0.0, 0.0, 0.0);    // Black FIXME
            cr->move_to(5 + 6*2, c_names_y * i + 11);

            t->show_in_cairo_context(cr);

            bool fill = false;
            bool solo = m_mainperf->get_sequence(sequence)->get_song_solo();
            bool muted = m_mainperf->get_sequence(sequence)->get_song_mute();
            
            if(solo || muted)
                fill = true;

            cr->set_source_rgb(0.0, 0.0, 0.0);          // Black FIXME
            
            if(muted)
            {
                cr->set_source_rgb(1.0, 0.27, 0.0);     // Red FIXME
            }
            if(solo)
            {
                cr->set_source_rgb(0.5, 0.988, 0.0);    // Green FIXME
            }

            cr->rectangle(m_window_x - 11,
                                     (c_names_y * i) + 1,
                                     11,
                                     c_names_y - 1  );

            if ( fill )
            {
                cr->stroke_preserve();
                cr->fill();
            }
            else
            {
                cr->stroke();
            }

            char smute[5];

            if ( muted )
            {
                snprintf(smute, sizeof(smute), "M" );
                // background
                cr->set_source_rgb(0.0, 0.0, 0.0);    // Black FIXME
            }
            else if ( solo )
            {
                snprintf(smute, sizeof(smute), "S" );
                // background
                cr->set_source_rgb(0.0, 0.0, 0.0);    // Black FIXME
            }
            else
            {
                snprintf(smute, sizeof(smute), "P" );
                // background 
                cr->set_source_rgb(1.0, 1.0, 1.0);    // White FIXME
            }

            auto m = create_pango_layout(smute);
            font.set_size((c_key_fontsize - 1) * Pango::SCALE);
            m->set_font_description(font);
            m->get_pixel_size(text_width, text_height);
            
            // draw the background for the mute label
            cr->rectangle(m_window_x - text_width - 3 , c_names_y * i + (text_height * .5) - 1, text_width + 1, (text_height * .5) + 5 );
            cr->stroke_preserve();
            cr->fill();
            
            // print the mute label
            if ( muted || solo )
            {
                cr->set_source_rgb(1.0, 1.0, 1.0);    // White FIXME
            }
            else
            {
                cr->set_source_rgb(0.0, 0.0, 0.0);    // Black FIXME
            }

            cr->move_to(m_window_x - text_width - 2, (c_names_y * i) +  ((c_names_y * .5) - (text_height * .5)));

            m->show_in_cairo_context(cr);
            
        }   // Active sequence
    }
    else    // if you scroll down to the very bottom
    {
        cr->set_source_rgb(0.6, 0.6, 0.6);    // grey FIXME
        cr->rectangle(0, (c_names_y * i) + 1, m_window_x, c_names_y);
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
        int trks = (m_window_y / c_names_y) + 1;

        for ( int i = 0; i < trks; i++ )
        {
            int seq = i + m_sequence_offset;
            draw_sequence(seq);
        }
    }

   /* Clear previous background */
    cr->set_source_rgb(1.0, 1.0, 1.0);  // White FIXME
    cr->rectangle (0.0, 0.0, width, height);
    cr->stroke_preserve();
    cr->fill();

    /* Draw the new background */
    cr->set_source(m_surface, 0.0, 0.0);
    cr->paint();

    return true;
}

void
perfnames::convert_y( int a_y, int *a_seq)
{
    *a_seq = a_y / c_names_y;
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
        
        /* If we did not land on another active track, then move to new location */
        if ( ! m_mainperf->is_active( m_current_seq ) )
        {
            m_mainperf->new_sequence( m_current_seq  );
            *(m_mainperf->get_sequence( m_current_seq )) = m_moving_seq;
            m_mainperf->get_sequence( m_current_seq )->unselect_triggers( );
            m_mainperf->get_sequence(m_current_seq)->set_dirty();
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
  //          printf("Cannot swap with a sequence that is being edited!!!\n");
            m_mainperf->new_sequence( m_old_seq  );
            *(m_mainperf->get_sequence( m_old_seq )) = m_moving_seq;
            m_mainperf->get_sequence(m_old_seq)->set_dirty();
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
    int y_f = m_window_y / c_names_y;

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
