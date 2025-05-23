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

#include "mainwid.h"
#include "font.h"
#include "seqedit.h"
#include "themes.h"

const char mainwid::m_seq_to_char[c_seqs_in_set] =
{
    '1', 'Q', 'A', 'Z',
    '2', 'W', 'S', 'X',
    '3', 'E', 'D', 'C',
    '4', 'R', 'F', 'V',
    '5', 'T', 'G', 'B',
    '6', 'Y', 'H', 'N',
    '7', 'U', 'J', 'M',
    '8', 'I', 'K', ','
};

// Constructor

mainwid::mainwid( perform *a_p, mainwnd *a_main ):
    seqmenu( a_p, a_main ),
    m_screenset(0),

    m_mainperf(a_p),

    m_window_x(c_mainwid_x),
    m_window_y(c_mainwid_y),
    m_text_x(c_text_x),
    m_text_y(c_text_y),
    m_seqarea_x(c_seqarea_x),
    m_seqarea_y(c_seqarea_y),
    m_seqarea_seq_x(c_text_x * 13),
    m_seqarea_seq_y(c_text_y * 2),
    m_resize_ratio_x(c_default_horizontal_zoom),
    m_resize_ratio_y(c_default_vertical_zoom),

    m_button_down(false),
    m_moving(false),
    m_need_redraw(false),
    m_have_realize(false),
    m_progress_tick(0),
    m_background_color(COLOR_WHITE),
    m_foreground_color(COLOR_BLACK),
    m_drop_x(),
    m_drop_y(),
    m_current_x(),
    m_current_y(),
    m_old_seq(),
    m_last_tick_x(),
    m_last_playing()
{
    Gtk::Allocation allocation = get_allocation();
    m_surface = Cairo::ImageSurface::create(
        Cairo::Format::FORMAT_ARGB32,
        allocation.get_width(),
        allocation.get_height()
    );

    using namespace Menu_Helpers;

    set_size_request( m_window_x, m_window_y );

    add_events( Gdk::BUTTON_PRESS_MASK |
                Gdk::BUTTON_RELEASE_MASK |
                Gdk::KEY_PRESS_MASK |
                Gdk::BUTTON_MOTION_MASK |
                Gdk::FOCUS_CHANGE_MASK  );

    set_double_buffered( false );
}

// GTK, on realize of window, init shiz
void
mainwid::on_realize()
{
    // we need to do the default realize
    Gtk::DrawingArea::on_realize();

    set_can_focus();
    
    if (m_window_x != m_surface->get_width() || m_window_y != m_surface->get_height())
    {
        m_surface = Cairo::ImageSurface::create(
            Cairo::Format::FORMAT_ARGB32,
                m_window_x,  m_window_y
        );
    }

    fill_background_window();
    m_need_redraw = true;
}

// Fills Pixmap
void
mainwid::draw_sequences_on_surface()
{
    for ( int i = 0; i < c_mainwnd_rows *  c_mainwnd_cols; i++ )
    {
        int a_seq = i + (m_screenset  * c_mainwnd_rows * c_mainwnd_cols );
        draw_sequence_on_surface( a_seq );
        draw_sequence_surface_on_window(a_seq);
        m_last_tick_x[ i + (m_screenset  * c_mainwnd_rows * c_mainwnd_cols)  ] = 0;
    }
}

// updates background
void
mainwid::fill_background_window()
{
    /* clear background - the whole sequence drawing area */
    Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(m_surface);
    
    cr->set_source_rgb(c_back_x_dark_grey.r, c_back_x_dark_grey.g, c_back_x_dark_grey.b);
    cr->rectangle(0, 0, m_window_x, m_window_y);
    cr->fill();
}


// Draws a specific sequence on the surface
void
mainwid::draw_sequence_on_surface( int a_seq )
{
    Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(m_surface);

    cr->set_operator(Cairo::OPERATOR_DEST);
    cr->set_operator(Cairo::OPERATOR_OVER);

    if ( a_seq >= (m_screenset  * c_mainwnd_rows * c_mainwnd_cols ) &&
            a_seq <  ((m_screenset+1)  * c_mainwnd_rows * c_mainwnd_cols ))
    {
        float i =  (a_seq / c_mainwnd_rows) % c_mainwnd_cols;
        float j =  a_seq % c_mainwnd_rows;

        float base_x = (c_mainwid_border +
                      (m_seqarea_x + c_mainwid_spacing) * i);
        float base_y = (c_mainwid_border +
                      (m_seqarea_y + c_mainwid_spacing) * j);

        /*int local_seq = a_seq % c_seqs_in_set;*/

        /* The background of the sequences */
        cr->set_source_rgb(c_back_black.r, c_back_black.g, c_back_black.b);
        cr->rectangle( (int) base_x + 1, (int) base_y + 1, (int) m_seqarea_x - 2, (int) m_seqarea_y - 2);
        cr->fill();
        
        /* Outline of the whole window */
        cr->set_source_rgb(c_fore_white.r, c_fore_white.g, c_fore_white.b);
        cr->rectangle(0, 0, m_window_x, m_window_y );
        cr->stroke();

        if ( m_mainperf->is_active( a_seq ))
        {
            sequence *seq = m_mainperf->get_sequence( a_seq );

            if ( seq->get_playing() )
            {
                m_last_playing[a_seq] = true;
                m_background_color = COLOR_BLACK;
                m_foreground_color = COLOR_WHITE;
            }
            else
            {
                m_last_playing[a_seq] = false;
                m_background_color = COLOR_WHITE;
                m_foreground_color = COLOR_BLACK;
            }

            if( m_background_color == COLOR_BLACK)
            {
                cr->set_source_rgba(c_track_color.r, c_track_color.g, c_track_color.b, 1.0);
            }
            else
            {
                cr->set_source_rgba(c_track_color.r, c_track_color.g, c_track_color.b, 0.5);
            }

            cr->rectangle( (int) base_x + 1,
                            (int) base_y + 1,
                            (int) m_seqarea_x - 2,
                            (int) m_seqarea_y - 2 );
            cr->fill();

            if( m_foreground_color == COLOR_BLACK)
            {
                cr->set_source_rgb(c_fore_white.r, c_fore_white.g, c_fore_white.b);
            }
            else
            {
                cr->set_source_rgb(c_back_black.r, c_back_black.g, c_back_black.b);
            }

            float rectangle_x = base_x + m_text_x - 1;
            float rectangle_y = base_y + m_text_y + (m_text_y * 0.5);

            if ( seq->get_queued() )
            {
                cr->set_source_rgb(c_back_black.r, c_back_black.g, c_back_black.b);
                cr->rectangle( (int) rectangle_x + 1,
                                         (int) rectangle_y - 1,
                                         (int) m_seqarea_seq_x + 1,
                                         (int) m_seqarea_seq_y + 3 );
                cr->fill();

                m_foreground_color = COLOR_BLACK;
                
                cr->set_source_rgba(c_track_color.r, c_track_color.g, c_track_color.b, 0.5);
            }
            
            /* The box around the note, queue area */
            cr->rectangle( (int) rectangle_x + 1,
                                    (int) rectangle_y - 1,
                                    (int) m_seqarea_seq_x + 1,
                                    (int) m_seqarea_seq_y + 3 );
            cr->stroke();

            int lowest_note = seq->get_lowest_note_event( );
            int highest_note = seq->get_highest_note_event( );

            int height = highest_note - lowest_note;
            height += 2;

            int length = seq->get_length( );
            
            long tick_s;
            long tick_f;
            int note;

            bool selected;
            
            float scale_font_w = (float) c_font_width * m_resize_ratio_x;

            int velocity;
            draw_type dt;

            seq->reset_draw_marker();

            while ( (dt = seq->get_next_note_event( &tick_s, &tick_f, &note,
                                                    &selected, &velocity )) != DRAW_FIN )
            {
                int note_y = m_seqarea_seq_y -
                             (m_seqarea_seq_y  * (note + 1 - lowest_note)) / height ;

                int tick_s_x = (tick_s * m_seqarea_seq_x)  / length;
                int tick_f_x = (tick_f * m_seqarea_seq_x)  / length;

                if ( dt == DRAW_NOTE_ON || dt == DRAW_NOTE_OFF )
                    tick_f_x = tick_s_x + 1;
                if ( tick_f_x <= tick_s_x )
                    tick_f_x = tick_s_x + 1;

                /* The drawn note color */
                if (m_foreground_color == COLOR_BLACK)
                {
                    cr->set_source_rgb(c_fore_white.r, c_fore_white.g, c_fore_white.b);
                }
                else
                {
                    cr->set_source_rgb(c_back_black.r, c_back_black.g, c_back_black.b);
                }
                
                cr->move_to( (int) rectangle_x + tick_s_x + 1,
                                   (int) rectangle_y + note_y);
                cr->line_to( (int) rectangle_x + tick_f_x + 1,
                                   (int) rectangle_y + note_y);
                cr->stroke();
            }
            
            /* The sequence names, bus, channel, key, text color */
            font::Color font_color;

            if ( seq->get_playing() )
            {
                font_color = font::BLACK;
            }
            else
            {
                font_color = font::WHITE;
            }

            /* Sequence name */
            char name[20];
            snprintf( name, sizeof name, "%.13s", seq->get_name() );

            p_font_renderer->render_string_on_drawable((base_x + m_text_x),
                                                       base_y + 2, cr,
                                                       name, font_color,
                                                       m_resize_ratio_x,
                                                       m_resize_ratio_y);

            /* midi channel + key + timesig */
            /*char key =  m_seq_to_char[local_seq];*/
            char str[20];

            if (m_mainperf->show_ui_sequence_key())
            {
                snprintf( str, sizeof str, "%c", (char)m_mainperf->lookup_keyevent_key( a_seq ) );

                p_font_renderer->render_string_on_drawable((base_x + m_seqarea_x - 5) - scale_font_w,
                                                            base_y + m_text_y * 4,
                                                            cr, str, font_color,
                                                            m_resize_ratio_x,
                                                            m_resize_ratio_y);
            }

            snprintf( str, sizeof str,
                      "%d-%d %ld/%ld",
                      seq->get_midi_bus(),
                      seq->get_midi_channel()+1,
                      seq->get_bp_measure(), seq->get_bw() );

            p_font_renderer->render_string_on_drawable( (base_x + m_text_x),
                                                        base_y + m_text_y * 4,
                                                        cr, str, font_color,
                                                        m_resize_ratio_x,
                                                        m_resize_ratio_y);
        }
        else    /* not active */
        {
            cr->set_source_rgb(c_back_black.r, c_back_black.g, c_back_black.b);
            cr->rectangle((int) base_x + 4, (int) base_y, (int) m_seqarea_x - 8, (int) m_seqarea_y + 1);
            cr->fill();
            
            cr->rectangle((int) base_x + 1, (int) base_y + 1, (int) m_seqarea_x - 2, (int) m_seqarea_y - 2);
            cr->fill();
        }
    }
}

void
mainwid::draw_sequence_surface_on_window( int a_seq )
{
    if ( a_seq >= (m_screenset  * c_mainwnd_rows * c_mainwnd_cols ) &&
            a_seq <  ((m_screenset+1)  * c_mainwnd_rows * c_mainwnd_cols ))
    {
        float i =  (a_seq / c_mainwnd_rows) % c_mainwnd_cols;
        float j =  a_seq % c_mainwnd_rows;

        float base_x = (c_mainwid_border +
                      (m_seqarea_x + c_mainwid_spacing) * i);
        float base_y = (c_mainwid_border +
                      (m_seqarea_y + c_mainwid_spacing) * j);

        m_surface_window->set_source(m_surface, 0, 0);
        m_surface_window->rectangle((int) base_x, (int) base_y, (int) m_seqarea_x, (int) m_seqarea_y);
        m_surface_window->stroke_preserve();
        m_surface_window->fill();
    }
}

void
mainwid::redraw( int a_sequence )
{
    update_sequence_on_window( a_sequence );
}

/**
 * From main window timeout
 * @param a_ticks
 */
void
mainwid::update_markers( int a_ticks )
{
    m_progress_tick = a_ticks;
    
    if (m_need_redraw)
    {
        if(m_have_realize)
        {
            m_need_redraw = false;
            fill_background_window();
            draw_sequences_on_surface();
        }
        else
            return; // if we don't have realize yet, then poll until we do
    }
    
    for ( int i = 0; i < (c_mainwnd_rows * c_mainwnd_cols); i++ )
    {
        int a_seq = i + (m_screenset  * c_mainwnd_rows * c_mainwnd_cols );

        if ( m_mainperf->is_dirty_main(a_seq ) )
        {
            update_sequence_on_window( a_seq );
        }

        if (global_is_running)
        {
            draw_marker_on_sequence(a_seq, m_progress_tick);
        }
    }
}

bool
mainwid::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    m_surface_window = cr;

    m_have_realize = true;
    m_need_redraw = true;
    
    update_markers(m_progress_tick);

    return true;
}

void
mainwid::draw_marker_on_sequence( int a_seq, int a_tick )
{
    if ( m_mainperf->is_active( a_seq ))
    {
        sequence *seq = m_mainperf->get_sequence( a_seq );

        float i =  (a_seq / c_mainwnd_rows) % c_mainwnd_cols;
        float j =  a_seq % c_mainwnd_rows;

        float base_x = (c_mainwid_border +
                      (m_seqarea_x + c_mainwid_spacing) * i);
        float base_y = (c_mainwid_border +
                      (m_seqarea_y + c_mainwid_spacing) * j);

        float rectangle_x = base_x + m_text_x - 1;
        float rectangle_y = base_y + m_text_y + (m_text_y * 0.5);

        int length = seq->get_length( );
        a_tick += (length - seq->get_trigger_offset( ));
        a_tick %= length;

        long tick_x = a_tick * m_seqarea_seq_x / length;
        
        /* Redraws the previous progress line background to clear previous line */
        m_surface_window->set_source(m_surface, 0, 0);
        m_surface_window->rectangle( (int) rectangle_x + m_last_tick_x[a_seq],
                                    (int) rectangle_y,
                                    3,
                                    m_seqarea_seq_y + 1 );

        m_surface_window->stroke_preserve();
        m_surface_window->fill();

        m_last_tick_x[a_seq] = tick_x;

        if ( seq->get_playing() )
        {
            m_surface_window->set_source_rgb(c_back_black.r, c_back_black.g, c_back_black.b);
        }
        else
        {
            m_surface_window->set_source_rgb(c_fore_white.r, c_fore_white.g, c_fore_white.b);
        }

        if ( seq->get_queued())
        {
            m_surface_window->set_source_rgb(c_fore_white.r, c_fore_white.g, c_fore_white.b);
        }

        /* Draw the progress line */
        m_surface_window->set_line_width(2.0);

        m_surface_window->move_to( (int) rectangle_x + tick_x + 1, (int) rectangle_y + 1);
        m_surface_window->line_to( (int) rectangle_x + tick_x + 1, (int) rectangle_y + m_seqarea_seq_y );
        m_surface_window->stroke();
    }
}

void
mainwid::update_sequences_on_window()
{
    m_need_redraw = true;
}

void
mainwid::update_sequence_on_window( int a_seq   )
{
    draw_sequence_on_surface( a_seq );
    draw_sequence_surface_on_window( a_seq );
}

// Translates XY corridinates to a sequence number
int
mainwid::seq_from_xy( int a_x, int a_y )
{
    /* adjust for border */
    int x = a_x - c_mainwid_border - (2 * m_resize_ratio_x);
    int y = a_y - c_mainwid_border;

    /* is it in the box ? */
    if ( x < 0
            || x >= ((m_seqarea_x + c_mainwid_spacing ) * c_mainwnd_cols )
            || y < 0
            || y >= ((m_seqarea_y + c_mainwid_spacing ) * c_mainwnd_rows ))
    {
        return -1;
    }

    /* gives us in box corrdinates */
    int box_test_x = x % (int) (m_seqarea_x + c_mainwid_spacing);
    int box_test_y = y % (int) (m_seqarea_y + c_mainwid_spacing);

    /* right inactive side of area */
    if ( box_test_x > m_seqarea_x
            || box_test_y > m_seqarea_y )
    {
        return -1;
    }

    x /= (int) (m_seqarea_x + c_mainwid_spacing);
    y /= (int) (m_seqarea_y + c_mainwid_spacing);

    int sequence =  ( (x * c_mainwnd_rows + y)
                      + ( m_screenset * c_mainwnd_rows * c_mainwnd_cols ));

    return sequence;
}

// press a mouse button
bool
mainwid::on_button_press_event(GdkEventButton* a_p0)
{
    grab_focus();

    m_current_seq = seq_from_xy( (int) a_p0->x, (int) a_p0->y );

    if ( m_current_seq != -1  && a_p0->button == 1 )
    {
        m_button_down = true;
    }

    return true;
}

bool
mainwid::on_button_release_event(GdkEventButton* a_p0)
{
    m_current_seq = seq_from_xy( (int) a_p0->x, (int) a_p0->y );

    m_button_down = false;

    /* it hit a sequence ? */
    // toggle play mode of sequence (left button)

    if ( m_current_seq != -1  && a_p0->button == 1 && !m_moving )
    {
        if ( m_mainperf->is_active( m_current_seq ))
        {
            //sequence *seq = m_mainperf->get_sequence(  m_current_seq );
            //seq->set_playing( !seq->get_playing() );

            m_mainperf->sequence_playing_toggle( m_current_seq );

            update_sequence_on_window( m_current_seq );
        }
    }

    if ( a_p0->button == 1 && m_moving )
    {
        m_moving = false;

        /* If we land on in-active track, then move or copy to new location */
        if ( ! m_mainperf->is_active( m_current_seq ) && m_current_seq != -1 )
        {
            m_mainperf->new_sequence( m_current_seq  );
            *(m_mainperf->get_sequence( m_current_seq )) = m_moving_seq;
            m_mainperf->get_sequence( m_current_seq )->unselect_triggers();

            update_sequence_on_window( m_current_seq );
            
            /* Alt key, put the original back to old location - since this is a copy, not move */
            if ( a_p0->state & GDK_MOD1_MASK)
            {
                m_mainperf->new_sequence( m_old_seq  );
                *(m_mainperf->get_sequence( m_old_seq )) = m_moving_seq;

                update_sequence_on_window( m_old_seq );
            }
        }
        /* If we did land on another sequence and it is not being edited, then swap places. */
        else if ( !m_mainperf->is_sequence_in_edit( m_current_seq ) )
        {
            m_clipboard = *(m_mainperf->get_sequence( m_current_seq ));     // hold the current for swap to old location
            m_mainperf->new_sequence( m_old_seq  );                         // The old location
            *(m_mainperf->get_sequence( m_old_seq )) = m_clipboard;         // put the current seq into the old location

            m_mainperf->delete_sequence( m_current_seq );                   // delete the current for replacement
            m_mainperf->new_sequence( m_current_seq  );                     // add a new blank one
            *(m_mainperf->get_sequence( m_current_seq )) = m_moving_seq;    // replace with the old
            
            m_mainperf->get_sequence( m_current_seq )->unselect_triggers();
            m_mainperf->get_sequence( m_old_seq )->unselect_triggers();
            
            update_sequence_on_window( m_old_seq );
            
            update_sequence_on_window( m_current_seq );
        }
        /* They landed on another sequence but it is being edited, so ignore the move 
         * and put the old sequence back to original location. */
        else
        {
            m_mainperf->new_sequence( m_old_seq  );
            *(m_mainperf->get_sequence( m_old_seq )) = m_moving_seq;

            update_sequence_on_window( m_old_seq );
            sequence_is_being_edited();
        }
    }
    // launch menu (right button)
    if (  m_current_seq != -1 && a_p0->button == 3  )
    {
        popup_menu();
    }

    return true;
}

bool
mainwid::on_motion_notify_event(GdkEventMotion* a_p0)
{
    int seq = seq_from_xy( (int) a_p0->x, (int) a_p0->y );

    if ( m_button_down )
    {
        if ( seq != m_current_seq && !m_moving &&
                !m_mainperf->is_sequence_in_edit( m_current_seq ) )
        {
            if ( m_mainperf->is_active( m_current_seq ))
            {
                m_old_seq = m_current_seq;
                m_moving = true;

                m_moving_seq = *(m_mainperf->get_sequence( m_current_seq ));
                m_mainperf->delete_sequence( m_current_seq );
                update_sequence_on_window( m_current_seq );
            }
        }
    }

    return true;
}

// redraws everything, queues redraw
void
mainwid::reset( )
{
    m_need_redraw = true;
}

void
mainwid::set_screenset( int a_ss )
{
    m_screenset = a_ss;

    if ( m_screenset < 0 )
        m_screenset = c_max_sets - 1;

    if ( m_screenset >= c_max_sets )
        m_screenset = 0;

    m_mainperf->set_offset(m_screenset);

    reset();
}

mainwid::~mainwid( )
{

}

bool
mainwid::on_focus_in_event(GdkEventFocus*)
{
    grab_focus();
    return false;
}

bool
mainwid::on_focus_out_event(GdkEventFocus*)
{
    return false;
}

void
mainwid::on_size_allocate(Gtk::Allocation &a_r )
{
    Gtk::DrawingArea::on_size_allocate( a_r );

    m_window_x = a_r.get_width();
    m_window_y = a_r.get_height();
    
    // resize handler
    if ( m_window_x != m_surface->get_width() || m_window_y != m_surface->get_height())
    {
        m_surface = Cairo::ImageSurface::create(
            Cairo::Format::FORMAT_ARGB32,
            m_window_x,
            m_window_y
        );

        m_text_x = ( (float)  m_window_x / 8.0) / 15.0;
        m_text_y = ( (float) m_window_y / 4.0) / 5.0;
        
        m_seqarea_x = m_text_x * 15.0;
        m_seqarea_y =  m_text_y * 5.0;
        
        m_seqarea_seq_x = m_text_x * 13.0;
        m_seqarea_seq_y = m_text_y * 2.0;
        
        m_resize_ratio_x = (float) m_window_x / (float) c_mainwid_x;
        m_resize_ratio_y = (float) m_window_y / (float) c_mainwid_y;;
    }
}

void
mainwid::sequence_is_being_edited()
{
    Glib::ustring query_str = "Cannot swap sequences if being edited!";
    
    Gtk::MessageDialog dialog( query_str, false,
                              Gtk::MESSAGE_INFO,
                              Gtk::BUTTONS_OK, true);

    dialog.run();
}
