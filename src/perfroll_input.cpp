#include "perform.h"
#include "perfroll_input.h"
#include "perfroll.h"

void FruityPerfInput::updateMousePtr( perfroll& ths )
{
    // context sensitive mouse
    long drop_tick;
    int drop_sequence;
    ths.convert_xy( m_current_x, m_current_y, &drop_tick, &drop_sequence );
    if (ths.m_mainperf->is_active( drop_sequence ))
    {
        long start, end;
        if (ths.m_mainperf->get_sequence(drop_sequence)->intersectTriggers( drop_tick, start, end ))
        {
            if (start <= drop_tick && drop_tick <= start + (c_perfroll_size_box_click_w * c_perf_scale_x) &&
                    (m_current_y % c_names_y) <= c_perfroll_size_box_click_w + 1)
            {
                ths.get_window()->set_cursor( Gdk::Cursor( Gdk::RIGHT_PTR ));
            }
            else if (end - (c_perfroll_size_box_click_w * c_perf_scale_x) <= drop_tick && drop_tick <= end &&
                     (m_current_y % c_names_y) >= c_names_y - c_perfroll_size_box_click_w - 1)
            {
                ths.get_window()->set_cursor( Gdk::Cursor( Gdk::LEFT_PTR ));
            }
            else
            {
                ths.get_window()->set_cursor( Gdk::Cursor( Gdk::CENTER_PTR ));
            }
        }
        else
        {
            ths.get_window()->set_cursor( Gdk::Cursor( Gdk::PENCIL ));
        }
    }
    else
    {
        ths.get_window()->set_cursor( Gdk::Cursor( Gdk::CROSSHAIR ));
    }
}

bool FruityPerfInput::on_button_press_event(GdkEventButton* a_ev, perfroll& ths)
{
    ths.grab_focus( );

    if ( ths.m_mainperf->is_active( ths.m_drop_sequence ))
    {
        ths.m_mainperf->get_sequence( ths.m_drop_sequence )->unselect_triggers( );
        ths.draw_background_on( ths.m_pixmap, ths.m_drop_sequence );
        ths.draw_sequence_on( ths.m_pixmap, ths.m_drop_sequence );

        /* ths.m_drop_y will be adjusted by perfroll.cpp change_vert() for any        */
        /* scroll after it was originally selected. Below call to draw_drawable_row   */
        /* will have the wrong y location and un-select will not occur if the user    */
        /* scrolls the track up or down to a new y location if not adjusted.          */

        ths.draw_drawable_row( ths.m_window, ths.m_pixmap, ths.m_drop_y);
    }

    ths.m_drop_x = (int) a_ev->x;
    ths.m_drop_y = (int) a_ev->y;
    m_current_x = (int) a_ev->x;
    m_current_y = (int) a_ev->y;

    ths.convert_xy( ths.m_drop_x, ths.m_drop_y, &ths.m_drop_tick, &ths.m_drop_sequence );

    /*      left mouse button     */
    if ( a_ev->button == 1)
    {
        on_left_button_pressed(a_ev, ths);
    }
    /*     right mouse button      */
    if ( a_ev->button == 3 )
    {
        on_right_button_pressed(a_ev, ths);
    }
    /* middle: split or paste */
    if ( a_ev->button == 2 )
    {
        if ( ths.m_mainperf->is_active( ths.m_drop_sequence ))
        {
            long tick = ths.m_drop_tick;
            tick = tick - tick % ths.m_snap; // grid snap

            bool state = ths.m_mainperf->get_sequence( ths.m_drop_sequence )->get_trigger_state(tick);

            if ( state )    // clicked on trigger - split trigger
            {
                ths.split_trigger(ths.m_drop_sequence, tick);
            }
            else    // clicked on track (off trigger) - paste
            {
                ths.m_mainperf->push_trigger_undo(ths.m_drop_sequence);
                ths.m_mainperf->get_sequence( ths.m_drop_sequence )->paste_trigger(tick);
            }
        }
    }

    updateMousePtr( ths );
    return true;
}

void FruityPerfInput::on_left_button_pressed(GdkEventButton* a_ev, perfroll& ths)
{
    /* left-ctrl: split or paste */
    if ( a_ev->state & GDK_CONTROL_MASK )
    {
        if ( ths.m_mainperf->is_active( ths.m_drop_sequence ))
        {
            long tick = ths.m_drop_tick;
            tick = tick - tick % ths.m_snap; // grid snap

            bool state = ths.m_mainperf->get_sequence( ths.m_drop_sequence )->get_trigger_state(tick);

            if ( state )    // clicked on trigger - split
            {
                ths.split_trigger(ths.m_drop_sequence, tick);
            }
            else // clicked off trigger for paste
            {
                ths.m_mainperf->push_trigger_undo(ths.m_drop_sequence);
                ths.m_mainperf->get_sequence( ths.m_drop_sequence )->paste_trigger(tick);
            }
        }
    }
    else
    {
        long tick = ths.m_drop_tick;

        /* add a new note if we didnt select anything */
        //if (m_adding)
        {
            m_adding_pressed = true;

            if ( ths.m_mainperf->is_active( ths.m_drop_sequence ))
            {
                long seq_length = ths.m_mainperf->get_sequence( ths.m_drop_sequence )->get_length( );

                bool state = ths.m_mainperf->get_sequence( ths.m_drop_sequence )->get_trigger_state( tick );

                // resize the event, or move it, depending on where clicked.
                if ( state )
                {
                    //m_adding = false;
                    m_adding_pressed = false;
                    // flag to tell motion notify to push_trigger_undo
                    ths.have_button_press = ths.m_mainperf->get_sequence( ths.m_drop_sequence )->select_trigger( tick );

                    long start_tick = ths.m_mainperf->get_sequence( ths.m_drop_sequence )->get_selected_trigger_start_tick();
                    long end_tick = ths.m_mainperf->get_sequence( ths.m_drop_sequence )->get_selected_trigger_end_tick();

                    if ( tick >= start_tick &&
                            tick <= start_tick + (c_perfroll_size_box_click_w * c_perf_scale_x) &&
                            (ths.m_drop_y % c_names_y) <= c_perfroll_size_box_click_w + 1 )
                    {
                        // clicked left side: begin a grow/shrink for the left side
                        ths.m_growing = true;
                        ths.m_grow_direction = true;
                        ths.m_drop_tick_trigger_offset = ths.m_drop_tick -
                                                         ths.m_mainperf->get_sequence( ths.m_drop_sequence )->
                                                         get_selected_trigger_start_tick( );
                    }
                    else if ( tick >= end_tick - (c_perfroll_size_box_click_w * c_perf_scale_x) &&
                              tick <= end_tick &&
                              (ths.m_drop_y % c_names_y) >= c_names_y - c_perfroll_size_box_click_w - 1 )
                    {
                        // clicked right side: grow/shrink the right side
                        ths.m_growing = true;
                        ths.m_grow_direction = false;
                        ths.m_drop_tick_trigger_offset =
                            ths.m_drop_tick -
                            ths.m_mainperf->get_sequence( ths.m_drop_sequence )->get_selected_trigger_end_tick( );
                    }
                    else
                    {
                        // clicked in the middle - move it
                        ths.m_moving = true;
                        ths.m_drop_tick_trigger_offset = ths.m_drop_tick -
                                                         ths.m_mainperf->get_sequence( ths.m_drop_sequence )->
                                                         get_selected_trigger_start_tick( );

                    }

                    ths.draw_background_on( ths.m_pixmap, ths.m_drop_sequence );
                    ths.draw_sequence_on( ths.m_pixmap, ths.m_drop_sequence );
                    ths.draw_drawable_row( ths.m_window, ths.m_pixmap, ths.m_drop_y);
                }

                // add an event:
                else
                {
                    // snap to length of sequence
                    tick = tick - (tick % seq_length);

                    ths.m_mainperf->push_trigger_undo(ths.m_drop_sequence);
                    ths.m_mainperf->get_sequence( ths.m_drop_sequence )->add_trigger( tick, seq_length );
                    ths.draw_background_on( ths.m_pixmap, ths.m_drop_sequence );
                    ths.draw_sequence_on( ths.m_pixmap, ths.m_drop_sequence );
                    ths.draw_drawable_row( ths.m_window, ths.m_pixmap, ths.m_drop_y);

                    //m_drop_tick_last = (m_drop_tick + seq_length - 1);
                }
            }
        }
    }
}

void FruityPerfInput::on_right_button_pressed(GdkEventButton* a_ev, perfroll& ths)
{
    //set_adding( false );

    long tick = ths.m_drop_tick;

    if ( ths.m_mainperf->is_active( ths.m_drop_sequence ))
    {
        //long seq_length = ths.m_mainperf->get_sequence( ths.m_drop_sequence )->get_length();

        bool state = ths.m_mainperf->get_sequence( ths.m_drop_sequence )->get_trigger_state( tick );

        if ( state )
        {
            ths.m_mainperf->push_trigger_undo(ths.m_drop_sequence);
            ths.m_mainperf->get_sequence( ths.m_drop_sequence )->del_trigger( tick );
        }
    }
}

bool FruityPerfInput::on_button_release_event(GdkEventButton* a_ev, perfroll& ths)
{
    m_current_x = (int) a_ev->x;
    m_current_y = (int) a_ev->y;

    if ( a_ev->button == 1 || a_ev->button == 3 )
    {
        m_adding_pressed = false;
    }

    ths.m_moving = false;
    ths.m_growing = false;
    m_adding_pressed = false;

    if ( ths.m_mainperf->is_active( ths.m_drop_sequence  ))
    {
        ths.draw_background_on( ths.m_pixmap, ths.m_drop_sequence );
        ths.draw_sequence_on( ths.m_pixmap, ths.m_drop_sequence );
        ths.draw_drawable_row( ths.m_window, ths.m_pixmap, ths.m_drop_y );
    }

    updateMousePtr( ths );
    return true;
}

bool FruityPerfInput::on_motion_notify_event(GdkEventMotion* a_ev, perfroll& ths)
{
    long tick;
    int x = (int) a_ev->x;
    m_current_x = (int) a_ev->x;
    m_current_y = (int) a_ev->y;

    if (  m_adding_pressed )
    {
        ths.convert_x( x, &tick );

        if ( ths.m_mainperf->is_active( ths.m_drop_sequence ))
        {
            long seq_length = ths.m_mainperf->get_sequence( ths.m_drop_sequence )->get_length( );
            tick = tick - (tick % seq_length);

            /*long min_tick = (tick < m_drop_tick) ? tick : m_drop_tick;*/
            long length = seq_length;

            ths.m_mainperf->get_sequence( ths.m_drop_sequence )
            ->grow_trigger( ths.m_drop_tick, tick, length);
            ths.draw_background_on( ths.m_pixmap, ths.m_drop_sequence );
            ths.draw_sequence_on( ths.m_pixmap, ths.m_drop_sequence );
            ths.draw_drawable_row( ths.m_window, ths.m_pixmap, ths.m_drop_y);
        }
    }
    else if ( ths.m_moving || ths.m_growing )
    {
        if ( ths.m_mainperf->is_active( ths.m_drop_sequence))
        {
            if(ths.have_button_press)
            {
                // this is necessary to ensure no push unless we have motion notify
                ths.m_mainperf->push_trigger_undo(ths.m_drop_sequence);
                ths.have_button_press = false;
            }

            ths.convert_x( x, &tick );
            tick -= ths.m_drop_tick_trigger_offset;

            tick = tick - tick % ths.m_snap;

            if ( ths.m_moving )
            {
                ths.m_mainperf->get_sequence( ths.m_drop_sequence )
                ->move_selected_triggers_to( tick, true );
            }
            if ( ths.m_growing )
            {
                if ( ths.m_grow_direction )
                    ths.m_mainperf->get_sequence( ths.m_drop_sequence )
                    ->move_selected_triggers_to( tick, false, GROW_START );
                else
                    ths.m_mainperf->get_sequence( ths.m_drop_sequence )
                    ->move_selected_triggers_to( tick-1, false, GROW_END );
            }

            ths.draw_background_on( ths.m_pixmap, ths.m_drop_sequence );
            ths.draw_sequence_on( ths.m_pixmap, ths.m_drop_sequence );
            ths.draw_drawable_row( ths.m_window, ths.m_pixmap, ths.m_drop_y);
        }
    }

    updateMousePtr( ths );
    return true;
}

/* popup menu calls this */
void Seq32PerfInput::set_adding( bool a_adding, perfroll& ths )
{
    if ( a_adding )
    {
        ths.get_window()->set_cursor(  Gdk::Cursor( Gdk::PENCIL ));
        m_adding = true;
    }
    else
    {
        ths.get_window()->set_cursor( Gdk::Cursor( Gdk::LEFT_PTR ));
        m_adding = false;
    }
}

bool Seq32PerfInput::on_button_press_event(GdkEventButton* a_ev, perfroll& ths)
{
    ths.grab_focus( );

    if ( ths.m_mainperf->is_active( ths.m_drop_sequence ))
    {
        ths.m_mainperf->get_sequence( ths.m_drop_sequence )->unselect_triggers( );
        ths.draw_background_on( ths.m_pixmap, ths.m_drop_sequence );
        ths.draw_sequence_on( ths.m_pixmap, ths.m_drop_sequence );

        /* ths.m_drop_y will be adjusted by perfroll.cpp change_vert() for any        */
        /* scroll after it was originally selected. Below call to draw_drawable_row   */
        /* will have the wrong y location and un-select will not occur if the user    */
        /* scrolls the track up or down to a new y location if not adjusted.          */

        ths.draw_drawable_row( ths.m_window, ths.m_pixmap, ths.m_drop_y);
    }

    ths.m_drop_x = (int) a_ev->x;
    ths.m_drop_y = (int) a_ev->y;

    ths.convert_xy( ths.m_drop_x, ths.m_drop_y, &ths.m_drop_tick, &ths.m_drop_sequence );

    /*      left mouse button     */
    if ( a_ev->button == 1 )
    {
        long tick = ths.m_drop_tick;

        /* add a new note if we didnt select anything */
        if (  m_adding )
        {
            m_adding_pressed = true;

            if ( ths.m_mainperf->is_active( ths.m_drop_sequence ))
            {
                long seq_length = ths.m_mainperf->get_sequence( ths.m_drop_sequence )->get_length( );

                bool state = ths.m_mainperf->get_sequence( ths.m_drop_sequence )->get_trigger_state( tick );

                if ( state )
                {
                    ths.m_mainperf->push_trigger_undo(ths.m_drop_sequence);
                    ths.m_mainperf->get_sequence( ths.m_drop_sequence )->del_trigger( tick );
                }
                else
                {
                    // snap to length of sequence
                    tick = tick - (tick % seq_length);
                    //m_adding_pressed_state = true;

                    ths.m_mainperf->push_trigger_undo(ths.m_drop_sequence);
                    ths.m_mainperf->get_sequence( ths.m_drop_sequence )->add_trigger( tick, seq_length );
                    ths.draw_background_on( ths.m_pixmap, ths.m_drop_sequence );
                    ths.draw_sequence_on( ths.m_pixmap, ths.m_drop_sequence );
                    ths.draw_drawable_row( ths.m_window, ths.m_pixmap, ths.m_drop_y);

                    //m_drop_tick_last = (m_drop_tick + seq_length - 1);
                }
            }
        }
        else
        {
            if ( ths.m_mainperf->is_active( ths.m_drop_sequence ))
            {
                // flag to tell motion notify to push_trigger_undo
                ths.have_button_press = ths.m_mainperf->get_sequence( ths.m_drop_sequence )->select_trigger( tick );

                long start_tick = ths.m_mainperf->get_sequence( ths.m_drop_sequence )->get_selected_trigger_start_tick();
                long end_tick = ths.m_mainperf->get_sequence( ths.m_drop_sequence )->get_selected_trigger_end_tick();

                if ( tick >= start_tick &&
                        tick <= start_tick + (c_perfroll_size_box_click_w * c_perf_scale_x) &&
                        (ths.m_drop_y % c_names_y) <= c_perfroll_size_box_click_w + 1 )
                {
                    ths.m_growing = true;
                    ths.m_grow_direction = true;
                    ths.m_drop_tick_trigger_offset = ths.m_drop_tick -
                                                     ths.m_mainperf->get_sequence( ths.m_drop_sequence )->
                                                     get_selected_trigger_start_tick( );
                }
                else if ( tick >= end_tick - (c_perfroll_size_box_click_w * c_perf_scale_x) &&
                          tick <= end_tick &&
                          (ths.m_drop_y % c_names_y) >= c_names_y - c_perfroll_size_box_click_w - 1 )
                {
                    ths.m_growing = true;
                    ths.m_grow_direction = false;
                    ths.m_drop_tick_trigger_offset =
                        ths.m_drop_tick -
                        ths.m_mainperf->get_sequence( ths.m_drop_sequence )->get_selected_trigger_end_tick( );
                }
                else
                {
                    ths.m_moving = true;
                    ths.m_drop_tick_trigger_offset = ths.m_drop_tick -
                                                     ths.m_mainperf->get_sequence( ths.m_drop_sequence )->
                                                     get_selected_trigger_start_tick( );

                }

                ths.draw_background_on( ths.m_pixmap, ths.m_drop_sequence );
                ths.draw_sequence_on( ths.m_pixmap, ths.m_drop_sequence );
                ths.draw_drawable_row( ths.m_window, ths.m_pixmap, ths.m_drop_y);
            }
        }
    }

    /*     right mouse button      */
    if ( a_ev->button == 3 )
    {
        set_adding( true, ths );
    }

    /* middle, split :or setting the paste location of copy/paste */
    if ( a_ev->button == 2 )
    {
        if ( ths.m_mainperf->is_active( ths.m_drop_sequence ))
        {
            long tick = ths.m_drop_tick;
            tick = tick - tick % ths.m_snap; // grid snap

            bool state = ths.m_mainperf->get_sequence( ths.m_drop_sequence )->get_trigger_state(tick);

            if ( state )     // clicked on trigger for split
            {
                ths.split_trigger(ths.m_drop_sequence, tick);
            }
            else    // clicked off trigger for paste
            {
                ths.m_mainperf->push_trigger_undo(ths.m_drop_sequence);
                ths.m_mainperf->get_sequence( ths.m_drop_sequence )->paste_trigger(tick);
            }
        }
    }
    return true;
}

bool Seq32PerfInput::on_button_release_event(GdkEventButton* a_ev, perfroll& ths)
{
    if ( a_ev->button == 1 )
    {
        if ( m_adding )
        {
            m_adding_pressed = false;
        }
    }

    if ( a_ev->button == 3 )
    {
        m_adding_pressed = false;
        set_adding( false, ths );
    }

    ths.m_moving = false;
    ths.m_growing = false;
    m_adding_pressed = false;

    if ( ths.m_mainperf->is_active( ths.m_drop_sequence  ))
    {
        ths.draw_background_on( ths.m_pixmap, ths.m_drop_sequence );
        ths.draw_sequence_on( ths.m_pixmap, ths.m_drop_sequence );
        ths.draw_drawable_row( ths.m_window, ths.m_pixmap, ths.m_drop_y );
    }

    return true;
}

bool Seq32PerfInput::on_motion_notify_event(GdkEventMotion* a_ev, perfroll& ths)
{
    long tick;
    int x = (int) a_ev->x;

    if (  m_adding && m_adding_pressed )
    {
        ths.convert_x( x, &tick );

        if ( ths.m_mainperf->is_active( ths.m_drop_sequence ))
        {
            long seq_length = ths.m_mainperf->get_sequence( ths.m_drop_sequence )->get_length( );
            tick = tick - (tick % seq_length);

            /*long min_tick = (tick < m_drop_tick) ? tick : m_drop_tick;*/
            long length = seq_length;

            ths.m_mainperf->get_sequence( ths.m_drop_sequence )
            ->grow_trigger( ths.m_drop_tick, tick, length);
            ths.draw_background_on( ths.m_pixmap, ths.m_drop_sequence );
            ths.draw_sequence_on( ths.m_pixmap, ths.m_drop_sequence );
            ths.draw_drawable_row( ths.m_window, ths.m_pixmap, ths.m_drop_y);
        }
    }
    else if ( ths.m_moving || ths.m_growing )
    {
        if ( ths.m_mainperf->is_active( ths.m_drop_sequence))
        {
            if(ths.have_button_press)
            {
                // this is necessary to ensure no push unless we have motion notify
                ths.m_mainperf->push_trigger_undo(ths.m_drop_sequence);
                ths.have_button_press = false;
            }

            ths.convert_x( x, &tick );
            tick -= ths.m_drop_tick_trigger_offset;

            tick = tick - tick % ths.m_snap;

            if ( ths.m_moving )
            {
                ths.m_mainperf->get_sequence( ths.m_drop_sequence )
                ->move_selected_triggers_to( tick, true );
            }
            if ( ths.m_growing )
            {
                if ( ths.m_grow_direction )
                    ths.m_mainperf->get_sequence( ths.m_drop_sequence )
                    ->move_selected_triggers_to( tick, false, GROW_START );
                else
                    ths.m_mainperf->get_sequence( ths.m_drop_sequence )
                    ->move_selected_triggers_to( tick-1, false, GROW_END );
            }

            ths.draw_background_on( ths.m_pixmap, ths.m_drop_sequence );
            ths.draw_sequence_on( ths.m_pixmap, ths.m_drop_sequence );
            ths.draw_drawable_row( ths.m_window, ths.m_pixmap, ths.m_drop_y);
        }
    }

    return true;
}
