//----------------------------------------------------------------------------
//
//  This file is part of seq24.
//
//  seq24 is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  seq24 is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with seq24; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//-----------------------------------------------------------------------------

#include "perfedit.h"
#include "sequence.h"

#include "pixmaps/snap.xpm"
#include "pixmaps/play2.xpm"
#include "pixmaps/stop.xpm"
#include "pixmaps/expand.xpm"
#include "pixmaps/collapse.xpm"
#include "pixmaps/loop.xpm"
#include "pixmaps/copy.xpm"
#include "pixmaps/undo.xpm"
#include "pixmaps/redo.xpm"
#include "pixmaps/down.xpm"
#include "pixmaps/perfedit.xpm"
#include "pixmaps/jack.xpm"
#include "pixmaps/transportFollow.xpm"
#include "pixmaps/transpose.xpm"

using namespace sigc;

// tooltip helper, for old vs new gtk...
#if GTK_MINOR_VERSION >= 12
#   define add_tooltip( obj, text ) obj->set_tooltip_text( text);
#else
#   define add_tooltip( obj, text ) m_tooltips->set_tip( *obj, text );
#endif

perfedit::perfedit( perform *a_perf )
{
    using namespace Menu_Helpers;

    set_icon(Gdk::Pixbuf::create_from_xpm_data(perfedit_xpm));

    /* set the performance */
    m_snap = c_ppqn / 4;

    m_mainperf = a_perf;

    /* main window */
    set_title( "seq24 - Song Editor");
    set_size_request(700, 400);

    /* tooltips */
    m_tooltips = manage( new Tooltips( ) );

    m_vadjust = manage( new Adjustment(0,0,1,1,1,1 ));
    m_hadjust = manage( new Adjustment(0,0,1,1,1,1 ));

    m_vscroll   =  manage(new VScrollbar( *m_vadjust ));
    m_hscroll   =  manage(new HScrollbar( *m_hadjust ));

    m_perfnames = manage( new perfnames( m_mainperf, m_vadjust ));

    m_perfroll = manage( new perfroll( m_mainperf,
                                       this,
                                       m_hadjust,
                                       m_vadjust ));

    m_perftime = manage( new perftime( m_mainperf, this, m_hadjust ));

    /* init table, viewports and scroll bars */
    m_table     = manage( new Table( 6, 3, false));
    m_table->set_border_width( 2 );

    m_hbox      = manage( new HBox( false, 2 ));
    m_hlbox     = manage( new HBox( false, 2 ));

    m_hlbox->set_border_width( 2 );

    m_button_grow = manage( new Button());
    m_button_grow->add( *manage( new Arrow( Gtk::ARROW_RIGHT, Gtk::SHADOW_OUT )));
    m_button_grow->signal_clicked().connect( mem_fun( *this, &perfedit::grow));
    add_tooltip( m_button_grow, "Increase size of Grid." );

    /* fill table */

    m_table->attach( *m_hlbox,  0, 3, 0, 1,  Gtk::FILL, Gtk::SHRINK, 2, 0 ); // shrink was 0

    m_table->attach( *m_perfnames,    0, 1, 2, 3, Gtk::SHRINK, Gtk::FILL );

    m_table->attach( *m_perftime, 1, 2, 1, 2, Gtk::FILL, Gtk::SHRINK );
    m_table->attach( *m_perfroll, 1, 2, 2, 3,
                     Gtk::FILL | Gtk::SHRINK,
                     Gtk::FILL | Gtk::SHRINK );

    m_table->attach( *m_vscroll, 2, 3, 2, 3, Gtk::SHRINK, Gtk::FILL | Gtk::EXPAND  );

    m_table->attach( *m_hbox,  0, 1, 3, 4,  Gtk::FILL, Gtk::SHRINK, 0, 2 );
    m_table->attach( *m_hscroll, 1, 2, 3, 4, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK  );
    m_table->attach( *m_button_grow, 2, 3, 3, 4, Gtk::SHRINK, Gtk::SHRINK  );

    m_menu_snap =   manage( new Menu());
    m_menu_snap->items().push_back(MenuElem("1/1",   sigc::bind(mem_fun(*this,&perfedit::set_snap), 1  )));
    m_menu_snap->items().push_back(MenuElem("1/2",   sigc::bind(mem_fun(*this,&perfedit::set_snap), 2  )));
    m_menu_snap->items().push_back(MenuElem("1/4",   sigc::bind(mem_fun(*this,&perfedit::set_snap), 4  )));
    m_menu_snap->items().push_back(MenuElem("1/8",   sigc::bind(mem_fun(*this,&perfedit::set_snap), 8  )));
    m_menu_snap->items().push_back(MenuElem("1/16",  sigc::bind(mem_fun(*this,&perfedit::set_snap), 16  )));
    m_menu_snap->items().push_back(MenuElem("1/32",  sigc::bind(mem_fun(*this,&perfedit::set_snap), 32  )));
    m_menu_snap->items().push_back(SeparatorElem());
    m_menu_snap->items().push_back(MenuElem("1/3",   sigc::bind(mem_fun(*this,&perfedit::set_snap), 3  )));
    m_menu_snap->items().push_back(MenuElem("1/6",   sigc::bind(mem_fun(*this,&perfedit::set_snap), 6  )));
    m_menu_snap->items().push_back(MenuElem("1/12",   sigc::bind(mem_fun(*this,&perfedit::set_snap), 12  )));
    m_menu_snap->items().push_back(MenuElem("1/24",   sigc::bind(mem_fun(*this,&perfedit::set_snap), 24  )));
    m_menu_snap->items().push_back(SeparatorElem());
    m_menu_snap->items().push_back(MenuElem("1/5",   sigc::bind(mem_fun(*this,&perfedit::set_snap), 5  )));
    m_menu_snap->items().push_back(MenuElem("1/10",   sigc::bind(mem_fun(*this,&perfedit::set_snap), 10  )));
    m_menu_snap->items().push_back(MenuElem("1/20",   sigc::bind(mem_fun(*this,&perfedit::set_snap), 20  )));
    m_menu_snap->items().push_back(MenuElem("1/40",   sigc::bind(mem_fun(*this,&perfedit::set_snap), 40  )));
    m_menu_snap->items().push_back(SeparatorElem());
    m_menu_snap->items().push_back(MenuElem("1/7",   sigc::bind(mem_fun(*this,&perfedit::set_snap), 7  )));
    m_menu_snap->items().push_back(MenuElem("1/9",   sigc::bind(mem_fun(*this,&perfedit::set_snap), 9  )));
    m_menu_snap->items().push_back(MenuElem("1/11",   sigc::bind(mem_fun(*this,&perfedit::set_snap), 11  )));
    m_menu_snap->items().push_back(MenuElem("1/13",   sigc::bind(mem_fun(*this,&perfedit::set_snap), 13  )));
    m_menu_snap->items().push_back(MenuElem("1/14",   sigc::bind(mem_fun(*this,&perfedit::set_snap), 14  )));
    m_menu_snap->items().push_back(MenuElem("1/15",   sigc::bind(mem_fun(*this,&perfedit::set_snap), 15  )));

    /* snap */
    m_button_snap = manage( new Button());
    m_button_snap->add( *manage( new Image(Gdk::Pixbuf::create_from_xpm_data( snap_xpm ))));
    m_button_snap->signal_clicked().connect(  sigc::bind<Menu *>( mem_fun( *this, &perfedit::popup_menu), m_menu_snap  ));
    add_tooltip( m_button_snap, "Grid snap. (Fraction of Measure Length)" );
    m_entry_snap = manage( new Entry());
    m_entry_snap->set_size_request( 40, -1 );
    m_entry_snap->set_editable( false );

    m_menu_xpose =   manage( new Menu());
    char num[11];
    for ( int i=-12; i<=12; ++i)
    {
        if (i)
        {
            snprintf(num, sizeof(num), "%+d [%s]", i, c_interval_text[abs(i)]);
        }
        else
        {
            snprintf(num, sizeof(num), "0 [normal]");
        }
        m_menu_xpose->items().push_front( MenuElem( num,
                                          sigc::bind(mem_fun(*this,&perfedit::xpose_button_callback),
                                                  i )));
    }

    m_button_xpose = manage( new Button());
    m_button_xpose->add( *manage( new Image(Gdk::Pixbuf::create_from_xpm_data( transpose_xpm ))));
    m_button_xpose->signal_clicked().connect(  sigc::bind<Menu *>( mem_fun( *this, &perfedit::popup_menu), m_menu_xpose  ));
    add_tooltip( m_button_xpose, "Song transpose" );
    m_entry_xpose = manage( new Entry());
    m_entry_xpose->set_size_request( 30, -1 );
    m_entry_xpose->set_editable( false );

    m_menu_bp_measure = manage( new Menu() );
    m_menu_bw = manage( new Menu() );

    /* bw */
    m_menu_bw->items().push_back(MenuElem("1", sigc::bind(mem_fun(*this,&perfedit::bw_button_callback), 1  )));
    m_menu_bw->items().push_back(MenuElem("2", sigc::bind(mem_fun(*this,&perfedit::bw_button_callback), 2  )));
    m_menu_bw->items().push_back(MenuElem("4", sigc::bind(mem_fun(*this,&perfedit::bw_button_callback), 4  )));
    m_menu_bw->items().push_back(MenuElem("8", sigc::bind(mem_fun(*this,&perfedit::bw_button_callback), 8  )));
    m_menu_bw->items().push_back(MenuElem("16", sigc::bind(mem_fun(*this,&perfedit::bw_button_callback), 16 )));

    char b[20];

    for( int i=0; i<16; i++ )
    {
        snprintf( b, sizeof(b), "%d", i+1 );

        /* length */
        m_menu_bp_measure->items().push_back(MenuElem(b,
                                             sigc::bind(mem_fun(*this,&perfedit::bp_measure_button_callback),
                                                     i+1 )));
    }

    /* beats per measure */
    m_button_bp_measure = manage( new Button());
    m_button_bp_measure->add( *manage( new Image(Gdk::Pixbuf::create_from_xpm_data( down_xpm  ))));
    m_button_bp_measure->signal_clicked().connect(  sigc::bind<Menu *>( mem_fun( *this, &perfedit::popup_menu), m_menu_bp_measure  ));
    add_tooltip( m_button_bp_measure, "Time Signature. Beats per Measure" );
    m_entry_bp_measure = manage( new Entry());
    m_entry_bp_measure->set_width_chars(2);
    m_entry_bp_measure->set_editable( false );

    /* beat width */
    m_button_bw = manage( new Button());
    m_button_bw->add( *manage( new Image(Gdk::Pixbuf::create_from_xpm_data( down_xpm  ))));
    m_button_bw->signal_clicked().connect(  sigc::bind<Menu *>( mem_fun( *this, &perfedit::popup_menu), m_menu_bw  ));
    add_tooltip( m_button_bw, "Time Signature.  Length of Beat" );
    m_entry_bw = manage( new Entry());
    m_entry_bw->set_width_chars(2);
    m_entry_bw->set_editable( false );

    /* undo */
    m_button_undo = manage( new Button());
    m_button_undo->add( *manage( new Image(Gdk::Pixbuf::create_from_xpm_data( undo_xpm  ))));
    m_button_undo->signal_clicked().connect(  mem_fun( *this, &perfedit::undo));
    add_tooltip( m_button_undo, "Undo." );

    /* redo */
    m_button_redo = manage( new Button());
    m_button_redo->add( *manage( new Image(Gdk::Pixbuf::create_from_xpm_data( redo_xpm  ))));
    m_button_redo->signal_clicked().connect(  mem_fun( *this, &perfedit::redo));
    add_tooltip( m_button_redo, "Redo." );

    /* expand */
    m_button_expand = manage( new Button());
    m_button_expand->add(*manage( new Image(Gdk::Pixbuf::create_from_xpm_data( expand_xpm ))));
    m_button_expand->signal_clicked().connect(  mem_fun( *this, &perfedit::expand));
    add_tooltip( m_button_expand, "Expand between L and R markers." );

    /* collapse */
    m_button_collapse = manage( new Button());
    m_button_collapse->add(*manage( new Image(Gdk::Pixbuf::create_from_xpm_data( collapse_xpm ))));
    m_button_collapse->signal_clicked().connect(  mem_fun( *this, &perfedit::collapse));
    add_tooltip( m_button_collapse, "Collapse between L and R markers." );

    /* copy */
    m_button_copy = manage( new Button());
    m_button_copy->add(*manage( new Image(Gdk::Pixbuf::create_from_xpm_data( copy_xpm ))));
    m_button_copy->signal_clicked().connect(  mem_fun( *this, &perfedit::copy ));
    add_tooltip( m_button_copy, "Expand and copy between L and R markers." );

    m_button_loop = manage( new ToggleButton() );
    m_button_loop->add(*manage( new Image(Gdk::Pixbuf::create_from_xpm_data( loop_xpm ))));
    m_button_loop->signal_toggled().connect(  mem_fun( *this, &perfedit::set_looped ));
    add_tooltip( m_button_loop, "Play looped between L and R." );

    m_button_stop = manage( new Button() );
    m_button_stop->add(*manage( new Image(Gdk::Pixbuf::create_from_xpm_data( stop_xpm ))));
    m_button_stop->signal_clicked().connect( mem_fun( *this, &perfedit::stop_playing));
    add_tooltip( m_button_stop, "Stop playing." );

    m_button_play = manage( new Button() );
    m_button_play->add(*manage( new Image(Gdk::Pixbuf::create_from_xpm_data( play2_xpm ))));
    m_button_play->signal_clicked().connect(  mem_fun( *this, &perfedit::start_playing));
    add_tooltip( m_button_play, "Begin playing at L marker." );

#ifdef JACK_SUPPORT
    m_button_jack = manage( new ToggleButton() );
    m_button_jack->add(*manage( new Image(Gdk::Pixbuf::create_from_xpm_data( jack_xpm ))));
    m_button_jack->signal_toggled().connect(  mem_fun( *this, &perfedit::set_jack_mode ));
    add_tooltip( m_button_jack, "Toggle Jack sync connection" );
    if(global_with_jack_transport)
    {
        m_button_jack->set_active( true );
    }
#endif // JACK_SUPPORT

    m_button_follow = manage( new ToggleButton() );
    m_button_follow->add(*manage( new Image(Gdk::Pixbuf::create_from_xpm_data( transportFollow_xpm ))));
    m_button_follow->signal_clicked().connect(  mem_fun( *this, &perfedit::set_follow_transport ));
    add_tooltip( m_button_follow, "Follow transport" );
    m_button_follow->set_active(true);

    m_hlbox->pack_end( *m_button_copy, false, false );
    m_hlbox->pack_end( *m_button_expand, false, false );
    m_hlbox->pack_end( *m_button_collapse, false, false );
    m_hlbox->pack_end( *m_button_redo, false, false );
    m_hlbox->pack_end( *m_button_undo, false, false );

    m_hlbox->pack_start( *m_button_stop, false, false );
    m_hlbox->pack_start( *m_button_play, false, false );
    m_hlbox->pack_start( *m_button_loop, false, false );

    m_hlbox->pack_start( *(manage(new VSeparator( ))), false, false, 4);

    m_hlbox->pack_start( *m_button_bp_measure, false, false );
    m_hlbox->pack_start( *m_entry_bp_measure, false, false );

    m_hlbox->pack_start( *(manage(new Label( "/" ))), false, false, 4);

    m_hlbox->pack_start( *m_button_bw, false, false );
    m_hlbox->pack_start( *m_entry_bw, false, false );

    m_hlbox->pack_start( *(manage(new Label( "x" ))), false, false, 4);

    m_hlbox->pack_start( *m_button_snap, false, false );
    m_hlbox->pack_start( *m_entry_snap, false, false );

    m_hlbox->pack_start(*m_button_xpose, false, false );
    m_hlbox->pack_start(*m_entry_xpose, false, false );

    m_hlbox->pack_start( *(manage(new VSeparator( ))), false, false, 4);

#ifdef JACK_SUPPORT
    m_hlbox->pack_start( *m_button_jack, false, false );
#endif // JACK_SUPPORT

    m_hlbox->pack_start( *m_button_follow, false, false );

    /* add table */
    this->add( *m_table );

    m_snap = 8;
    m_bp_measure = 4;
    m_bw = 4;

    set_snap( 8 );
    set_bp_measure( 4 );
    set_bw( 4 );
    set_xpose( 0 );

    add_events( Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK );
}

bool
perfedit::on_key_press_event(GdkEventKey* a_ev)
{
    bool event_was_handled = false;

    // control and modifier key combinations matching
    if ( a_ev->state & GDK_CONTROL_MASK )
    {
        /* Ctrl-Z: Undo */
        if ( a_ev->keyval == GDK_z || a_ev->keyval == GDK_Z )
        {
            undo();
            return true;
        }
        /* Ctrl-R: Redo */
        if ( a_ev->keyval == GDK_r || a_ev->keyval == GDK_R )
        {
            redo();
            return true;
        }
    }

    if (a_ev->keyval == GDK_Z)         /* zoom in              */
    {
        set_zoom(m_perfroll->m_zoom / 2);
        return true;
    }
    else if (a_ev->keyval == GDK_0)         /* reset to normal zoom */
    {
        set_zoom(c_perf_scale_x);
        return true;
    }
    else if (a_ev->keyval == GDK_z)         /* zoom out             */
    {
        set_zoom(m_perfroll->m_zoom * 2);
        return true;
    }

    if ( a_ev->type == GDK_KEY_PRESS )
    {
        if ( global_print_keys )
        {
            printf( "key_press[%d] == %s\n", a_ev->keyval, gdk_keyval_name( a_ev->keyval ) );
        }
        // the start/end key may be the same key (i.e. SPACE)
        // allow toggling when the same key is mapped to both triggers (i.e. SPACEBAR)
        bool dont_toggle = m_mainperf->m_key_start != m_mainperf->m_key_stop;
        if ( a_ev->keyval == m_mainperf->m_key_start && (dont_toggle || !global_is_running) )
        {
            start_playing();
            return true;
        }
        else if ( a_ev->keyval == m_mainperf->m_key_stop && (dont_toggle || global_is_running) )
        {
            stop_playing();
            return true;
        }

        if(a_ev->keyval == m_mainperf->m_key_start || a_ev->keyval == m_mainperf->m_key_stop)
            event_was_handled = true;

        if ( a_ev->keyval ==  m_mainperf->m_key_follow_trans )
        {
            toggle_follow_transport();
            return true;
        }

#ifdef JACK_SUPPORT
        if ( a_ev->keyval ==  m_mainperf->m_key_jack )
        {
            toggle_jack();
            return true;
        }
#endif // JACK_SUPPORT
    }

    if(!event_was_handled)
    {
        return Gtk::Window::on_key_press_event(a_ev);
    }

    return false;
}

void
perfedit::undo()
{
    m_mainperf->pop_trigger_undo();
    m_perfroll->queue_draw();
}

void
perfedit::redo()
{
    m_mainperf->pop_trigger_redo();
    m_perfroll->queue_draw();
}

void
perfedit::start_playing()
{
    m_mainperf->set_start_from_perfedit(true);
    m_mainperf->start_playing();
}

void
perfedit::stop_playing()
{
    m_mainperf->stop_playing();
}

void
perfedit::collapse()
{
    m_mainperf->push_trigger_undo();
    m_mainperf->move_triggers( false );
    m_perfroll->queue_draw();
}

void
perfedit::copy()
{
    m_mainperf->push_trigger_undo();
    m_mainperf->copy_triggers(  );
    m_perfroll->queue_draw();
}

void
perfedit::expand()
{
    m_mainperf->push_trigger_undo();
    m_mainperf->move_triggers( true );
    m_perfroll->queue_draw();
}

void
perfedit::set_looped()
{
    m_mainperf->set_looping( m_button_loop->get_active());
}

#ifdef JACK_SUPPORT
void
perfedit::set_jack_mode ()
{
    if(m_button_jack->get_active() && !global_is_running)
        m_mainperf->init_jack ();

    if(!m_button_jack->get_active() && !global_is_running)
        m_mainperf->deinit_jack ();

    if(m_mainperf->is_jack_running())
        m_button_jack->set_active(true);
    else
        m_button_jack->set_active(false);

    m_mainperf->set_jack_mode(m_mainperf->is_jack_running()); // for seqroll keybinding
}

bool
perfedit::get_toggle_jack()
{
    return m_button_jack->get_active();
}

void
perfedit::toggle_jack()
{
    // Note that this will trigger the button signal callback.
    m_button_jack->set_active( ! m_button_jack->get_active() );
}
#endif // JACK_SUPPORT

void
perfedit::xpose_button_callback( int a_xpose)
{
    if(m_mainperf->get_master_midi_bus()->get_transpose() != a_xpose)
    {
        set_xpose(a_xpose);
    }
}

void
perfedit::set_xpose( int a_xpose  )
{
    char b[11];
    snprintf( b, sizeof(b), "%+d", a_xpose );
    m_entry_xpose->set_text(b);

    m_mainperf->all_notes_off();
    m_mainperf->get_master_midi_bus()->set_transpose(a_xpose);
}

void
perfedit::popup_menu(Menu *a_menu)
{
    a_menu->popup(0,0);
}

void
perfedit::set_guides()
{
    long measure_ticks = (c_ppqn * 4) * m_bp_measure / m_bw;
    long snap_ticks =  measure_ticks / m_snap;
    long beat_ticks = (c_ppqn * 4) / m_bw;
    m_perfroll->set_guides( snap_ticks, measure_ticks, beat_ticks );
    m_perftime->set_guides( snap_ticks, measure_ticks );
}

void
perfedit::set_snap( int a_snap  )
{
    char b[10];
    snprintf( b, sizeof(b), "1/%d", a_snap );
    m_entry_snap->set_text(b);

    m_snap = a_snap;
    set_guides();
}

void perfedit::bp_measure_button_callback(int a_beats_per_measure)
{
    if(m_bp_measure != a_beats_per_measure )
    {
        set_bp_measure(a_beats_per_measure);
        global_is_modified = true;
    }
}

void perfedit::set_bp_measure( int a_beats_per_measure )
{
    m_mainperf->set_bp_measure(a_beats_per_measure);

    if(a_beats_per_measure <= 7)
        set_snap(a_beats_per_measure * 2);
    else
        set_snap(a_beats_per_measure);

    char b[10];
    snprintf(b, sizeof(b), "%d", a_beats_per_measure );
    m_entry_bp_measure->set_text(b);

    m_bp_measure = a_beats_per_measure;
    set_guides();
}

int perfedit::get_bp_measure()
{
    return m_bp_measure;
}

void perfedit::bw_button_callback(int a_beat_width)
{
    if(m_bw != a_beat_width )
    {
        set_bw(a_beat_width);
        global_is_modified = true;
    }
}

void perfedit::set_bw( int a_beat_width )
{
    m_mainperf->set_bw(a_beat_width);
    char b[10];
    snprintf(b, sizeof(b), "%d", a_beat_width );
    m_entry_bw->set_text(b);

    m_bw = a_beat_width;
    set_guides();
}

int
perfedit::get_bw()
{
    return m_bw;
}

void
perfedit::set_zoom (int z)
{
    m_perfroll->set_zoom(z);
    m_perftime->set_zoom(z);
}

void
perfedit::set_follow_transport()
{
    m_mainperf->set_follow_transport(m_button_follow->get_active());
}

void
perfedit::toggle_follow_transport()
{
    // Note that this will trigger the button signal callback.
    m_button_follow->set_active( ! m_button_follow->get_active() );
}

void
perfedit::on_realize()
{
    // we need to do the default realize
    Gtk::Window::on_realize();

    Glib::signal_timeout().connect(mem_fun(*this,&perfedit::timeout ), c_redraw_ms);
}

void
perfedit::grow()
{
    m_perfroll->increment_size();
    m_perftime->increment_size();
}

void
perfedit::init_before_show()
{
    m_perfroll->init_before_show();
}

bool
perfedit::timeout()
{
    m_perfroll->redraw_dirty_sequences();
    m_perfroll->draw_progress();
    m_perfnames->redraw_dirty_sequences();

    if (m_button_follow->get_active() != m_mainperf->get_follow_transport())
        m_button_follow->set_active(m_mainperf->get_follow_transport());

#ifdef JACK_SUPPORT
    if(global_is_running)
        m_button_jack->set_sensitive(false);
    else
        m_button_jack->set_sensitive(true);
#endif // JACK_SUPPORT

    if(m_mainperf->m_have_undo)
        m_button_undo->set_sensitive(true);
    else
        m_button_undo->set_sensitive(false);

    if(m_mainperf->m_have_redo)
        m_button_redo->set_sensitive(true);
    else
        m_button_redo->set_sensitive(false);

    return true;
}

perfedit::~perfedit()
{

}

bool
perfedit::on_delete_event(GdkEventAny *a_event)
{
    return false;
}
