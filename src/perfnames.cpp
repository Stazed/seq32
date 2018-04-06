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

perfnames::perfnames( perform *a_perf, mainwnd *a_main, Adjustment *a_vadjust ):
    seqmenu(a_perf, a_main ),
    m_black(Gdk::Color( "black" )),
    m_white(Gdk::Color( "white" )),
    m_grey(Gdk::Color( "grey" )),
    m_orange(Gdk::Color( "Orange Red")),    // mute
    m_green(Gdk::Color( "Lawn Green")),     // solo
    m_mainperf(a_perf),
    m_vadjust(a_vadjust),
    m_sequence_offset(0),
    m_button_down(false),
    m_moving(false),
    m_old_seq(0)
{
    add_events( Gdk::BUTTON_PRESS_MASK |
                Gdk::BUTTON_RELEASE_MASK |
                Gdk::BUTTON_MOTION_MASK |
                Gdk::SCROLL_MASK );

    /* set default size */
    set_size_request( c_names_x, 100 );

    // in the constructor you can only allocate colors,
    // get_window() returns 0 because we have not be realized
    Glib::RefPtr<Gdk::Colormap>  colormap= get_default_colormap();
    colormap->alloc_color( m_black );
    colormap->alloc_color( m_white );
    colormap->alloc_color( m_grey );
    colormap->alloc_color( m_orange );
    colormap->alloc_color( m_green );

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

    // Now we can allocate any additional resources we need
    m_window = get_window();
    m_gc = Gdk::GC::create( m_window );
    m_window->clear();

    m_pixmap = Gdk::Pixmap::create(m_window,
                                   c_names_x,
                                   c_names_y  * c_total_seqs + 1,
                                   -1);
}

void
perfnames::change_vert()
{
    if ( m_sequence_offset != (int) m_vadjust->get_value() )
    {
        m_sequence_offset = (int) m_vadjust->get_value();
        queue_draw();
    }
}

void
perfnames::update_pixmap()
{

}

void
perfnames::draw_area()
{

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

    if ( sequence < c_total_seqs )
    {
        m_gc->set_foreground(m_black);
        m_window->draw_rectangle(m_gc,true,
                                 0,
                                 (c_names_y * i),
                                 c_names_x,
                                 c_names_y + 1 );

        if ( sequence % c_seqs_in_set == 0 )
        {
            char ss[10];
            snprintf(ss, sizeof(ss), "%2d", sequence / c_seqs_in_set );

            m_gc->set_foreground(m_white);

            p_font_renderer->render_string_on_drawable(m_gc,
                    2,
                    c_names_y * i + 2,
                    m_window, ss, font::WHITE );
        }

        else
        {
            m_gc->set_foreground(m_white);
            m_window->draw_rectangle(m_gc,true,
                                     1,
                                     (c_names_y * (i)),
                                     (6*2) + 1,
                                     c_names_y );
        }

        if ( m_mainperf->is_active( sequence ))
            m_gc->set_foreground(m_white);
        else
            m_gc->set_foreground(m_grey);

        m_window->draw_rectangle(m_gc,true,
                                 6 * 2 + 3,
                                 (c_names_y * i) + 1,
                                 c_names_x - 3 - (6*2),
                                 c_names_y - 1  );

        if ( m_mainperf->is_active( sequence ))
        {
            m_sequence_active[sequence]=true;

            /* names */
            char name[50];
            snprintf(name, sizeof(name), "%-14.14s   %2d",
                     m_mainperf->get_sequence(sequence)->get_name(),
                     m_mainperf->get_sequence(sequence)->get_midi_channel() + 1);

            p_font_renderer->render_string_on_drawable(m_gc,
                    5 + 6*2,
                    c_names_y * i + 2,
                    m_window, name, font::BLACK );

            char str[20];
            snprintf(str, sizeof(str),
                     "%d-%d %ld/%ld",
                     m_mainperf->get_sequence(sequence)->get_midi_bus(),
                     m_mainperf->get_sequence(sequence)->get_midi_channel()+1,
                     m_mainperf->get_sequence(sequence)->get_bp_measure(),
                     m_mainperf->get_sequence(sequence)->get_bw() );

            p_font_renderer->render_string_on_drawable(m_gc,
                    5 + 6*2,
                    c_names_y * i + 12,
                    m_window, str, font::BLACK );

            bool fill = false;
            bool solo = m_mainperf->get_sequence(sequence)->get_song_solo();
            bool muted = m_mainperf->get_sequence(sequence)->get_song_mute();
            
            if(solo || muted)
                fill = true;

            m_gc->set_foreground(m_black);
            
            if(muted)
                m_gc->set_foreground(m_orange);
            if(solo)
                m_gc->set_foreground(m_green);
            
            m_window->draw_rectangle(m_gc,fill,
                                     6*2 + 6 * 20 + 2,
                                     (c_names_y * i),
                                     10,
                                     c_names_y  );

            if ( muted )
            {
                p_font_renderer->render_string_on_drawable(m_gc,
                        5 + 6*2 + 6 * 20,
                        c_names_y * i + 2,
                        m_window, "M", font::WHITE );
            }
            else if ( solo )
            {
                p_font_renderer->render_string_on_drawable(m_gc,
                        5 + 6*2 + 6 * 20,
                        c_names_y * i + 2,
                        m_window, "S", font::WHITE );
            }
            else
            {
                p_font_renderer->render_string_on_drawable(m_gc,
                        5 + 6*2 + 6 * 20,
                        c_names_y * i + 2,
                        m_window, "P", font::BLACK );
            }
        }
    }
    else
    {
        m_gc->set_foreground(m_grey);
        m_window->draw_rectangle(m_gc,true,
                                 0,
                                 (c_names_y * i) + 1,
                                 c_names_x,
                                 c_names_y );

    }
}

bool
perfnames::on_expose_event(GdkEventExpose* a_e)
{
    int seqs = (m_window_y / c_names_y) + 1;

    for ( int i=0; i< seqs; i++ )
    {
        int sequence = i + m_sequence_offset;
        draw_sequence(sequence);
    }
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
        queue_draw();
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
        queue_draw();
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
            m_mainperf->get_sequence(m_current_seq)->set_dirty();
        }
        /* If we did land on an active track and it is not being edited, then swap places. */
        else if ( !m_mainperf->is_sequence_in_edit( m_current_seq ) )
        {
            m_clipboard = *(m_mainperf->get_sequence( m_current_seq ));      // hold the current for swap to old location
            m_mainperf->new_sequence( m_old_seq  );                          // The old location
            *(m_mainperf->get_sequence( m_old_seq )) = m_clipboard;          // put the current track into the old location
            m_mainperf->get_sequence(m_old_seq)->set_dirty();

            m_mainperf->delete_sequence( m_current_seq );                    // delete the current for replacement
            m_mainperf->new_sequence( m_current_seq  );                      // add a new blank one
            *(m_mainperf->get_sequence( m_current_seq )) = m_moving_seq;     // replace with the old
            m_mainperf->get_sequence(m_current_seq)->set_dirty();
        }
       /* They landed on another track but it is being edited, so ignore the move 
         * and put the old track back to original location. */
        else
        {
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
        int seq = y + m_sequence_offset; // 4am

        if ( seq < c_total_seqs)
        {
            bool dirty = (m_mainperf->is_dirty_names( seq ));

            if (dirty)
            {
                draw_sequence( seq );
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