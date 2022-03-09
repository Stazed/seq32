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

#include "perfedit.h"
#include "sequence.h"
#include "mainwnd.h"

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
#include "pixmaps/transportFollow.xpm"
#include "pixmaps/transpose.xpm"

#ifdef JACK_SUPPORT
#include "pixmaps/jack.xpm"
#endif // JACK_SUPPORT

using namespace sigc;

#define add_tooltip( obj, text ) obj->set_tooltip_text( text);

ff_rw_type_e FF_RW_button_type = FF_RW_RELEASE;

perfedit::perfedit( perform *a_perf, mainwnd *a_main )
{
    using namespace Menu_Helpers;

    set_icon(Gdk::Pixbuf::create_from_xpm_data(perfedit_xpm));

    /* set the performance */
    m_snap = c_ppqn / 4;

    m_mainperf = a_perf;
    m_mainwnd = a_main;
    m_tick_time_as_bbt = false;
    m_toggle_time_type = true;      // set true to show clock on start

    /* main window */
    set_title( "seq32 - Song Editor");
    set_size_request(750, 400);

    m_vadjust = Adjustment::create(0,0,1,1,1,1 );
    m_hadjust = Adjustment::create(0,0,1,1,1,1 );

    m_vscroll   =  manage(new VScrollbar( m_vadjust ));
    m_hscroll   =  manage(new HScrollbar( m_hadjust ));

    m_perfnames = manage( new perfnames( m_mainperf, a_main, m_vadjust ));

    m_perfroll = manage( new perfroll( m_mainperf,
                                       this,
                                       m_hadjust,
                                       m_vadjust ));

    m_perftime = manage( new perftime( m_mainperf, this, m_hadjust ));
    m_tempo = manage( new tempo( m_mainperf, this, m_hadjust ));

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
    
    m_tick_time = manage(new Gtk::Label(""));
    m_button_time_type = manage(new Gtk::Button("HMS"));
    Gtk::HBox * hbox4 = manage(new Gtk::HBox(false, 0));
    m_tick_time->set_justify(Gtk::JUSTIFY_LEFT);
    
    m_button_time_type->set_focus_on_click(false);
    
    m_button_time_type->signal_clicked().connect
    (
        mem_fun(*this, &perfedit::toggle_time_format)
    );
    add_tooltip
    (
        m_button_time_type,
        "Toggles between B:B:T and H:M:S format, showing the selected format."
    );

    hbox4->pack_start(*m_button_time_type, false, false, 5);
    hbox4->pack_start(*m_tick_time, false, false, 5);

    m_table->attach( *m_hlbox,  0, 3, 0, 1,  Gtk::FILL, Gtk::SHRINK, 2, 0 ); // shrink was 0

    m_table->attach( *m_perfnames,    0, 1, 3, 4, Gtk::SHRINK, Gtk::FILL );
    m_table->attach( *m_tempo, 1, 2, 1, 2, Gtk::FILL, Gtk::SHRINK );
    
    m_table->attach( *hbox4, 0,1,1,3, Gtk::FILL, Gtk::SHRINK);
    
    m_table->attach( *m_perftime, 1, 2, 2, 3, Gtk::FILL, Gtk::SHRINK );
    m_table->attach( *m_perfroll, 1, 2, 3, 4,
                     Gtk::FILL | Gtk::SHRINK,
                     Gtk::FILL | Gtk::SHRINK );

    m_table->attach( *m_vscroll, 2, 3, 3, 4, Gtk::SHRINK, Gtk::FILL | Gtk::EXPAND  );

    m_table->attach( *m_hbox,  0, 1, 4, 5,  Gtk::FILL, Gtk::SHRINK, 0, 2 );
    m_table->attach( *m_hscroll, 1, 2, 4, 5, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK  );
    m_table->attach( *m_button_grow, 2, 3, 4, 5, Gtk::SHRINK, Gtk::SHRINK  );

    m_menu_snap =   manage( new Menu());

    m_snap_menu_items.resize(25);

    m_snap_menu_items[0].set_label("1/1");
    m_snap_menu_items[0].signal_activate().connect(sigc::bind(mem_fun(*this,&perfedit::set_snap), 1 ));
    m_menu_snap->append(m_snap_menu_items[0]);

    m_snap_menu_items[1].set_label("1/2");
    m_snap_menu_items[1].signal_activate().connect(sigc::bind(mem_fun(*this,&perfedit::set_snap), 2 ));
    m_menu_snap->append(m_snap_menu_items[1]);

    m_snap_menu_items[2].set_label("1/4");
    m_snap_menu_items[2].signal_activate().connect(sigc::bind(mem_fun(*this,&perfedit::set_snap), 4 ));
    m_menu_snap->append(m_snap_menu_items[2]);

    m_snap_menu_items[3].set_label("1/8");
    m_snap_menu_items[3].signal_activate().connect(sigc::bind(mem_fun(*this,&perfedit::set_snap), 8 ));
    m_menu_snap->append(m_snap_menu_items[3]);

    m_snap_menu_items[4].set_label("1/16");
    m_snap_menu_items[4].signal_activate().connect(sigc::bind(mem_fun(*this,&perfedit::set_snap), 16 ));
    m_menu_snap->append(m_snap_menu_items[4]);

    m_snap_menu_items[5].set_label("1/32");
    m_snap_menu_items[5].signal_activate().connect(sigc::bind(mem_fun(*this,&perfedit::set_snap), 32 ));
    m_menu_snap->append(m_snap_menu_items[5]);

    m_menu_snap->append(m_menu_separator6);

    m_snap_menu_items[6].set_label("1/3");
    m_snap_menu_items[6].signal_activate().connect(sigc::bind(mem_fun(*this,&perfedit::set_snap), 3 ));
    m_menu_snap->append(m_snap_menu_items[6]);

    m_snap_menu_items[7].set_label("1/6");
    m_snap_menu_items[7].signal_activate().connect(sigc::bind(mem_fun(*this,&perfedit::set_snap), 6 ));
    m_menu_snap->append(m_snap_menu_items[7]);

    m_snap_menu_items[8].set_label("1/12");
    m_snap_menu_items[8].signal_activate().connect(sigc::bind(mem_fun(*this,&perfedit::set_snap), 12 ));
    m_menu_snap->append(m_snap_menu_items[8]);

    m_snap_menu_items[9].set_label("1/24");
    m_snap_menu_items[9].signal_activate().connect(sigc::bind(mem_fun(*this,&perfedit::set_snap), 24 ));
    m_menu_snap->append(m_snap_menu_items[9]);

    m_menu_snap->append(m_menu_separator7);

    m_snap_menu_items[10].set_label("1/5");
    m_snap_menu_items[10].signal_activate().connect(sigc::bind(mem_fun(*this,&perfedit::set_snap), 5 ));
    m_menu_snap->append(m_snap_menu_items[10]);

    m_snap_menu_items[11].set_label("1/10");
    m_snap_menu_items[11].signal_activate().connect(sigc::bind(mem_fun(*this,&perfedit::set_snap), 10 ));
    m_menu_snap->append(m_snap_menu_items[11]);

    m_snap_menu_items[12].set_label("1/20");
    m_snap_menu_items[12].signal_activate().connect(sigc::bind(mem_fun(*this,&perfedit::set_snap), 20 ));
    m_menu_snap->append(m_snap_menu_items[12]);

    m_snap_menu_items[13].set_label("1/40");
    m_snap_menu_items[13].signal_activate().connect(sigc::bind(mem_fun(*this,&perfedit::set_snap), 40 ));
    m_menu_snap->append(m_snap_menu_items[13]);

    m_menu_snap->append(m_menu_separator8);

    m_snap_menu_items[14].set_label("1/7");
    m_snap_menu_items[14].signal_activate().connect(sigc::bind(mem_fun(*this,&perfedit::set_snap), 7 ));
    m_menu_snap->append(m_snap_menu_items[14]);

    m_snap_menu_items[15].set_label("1/9");
    m_snap_menu_items[15].signal_activate().connect(sigc::bind(mem_fun(*this,&perfedit::set_snap), 9 ));
    m_menu_snap->append(m_snap_menu_items[15]);

    m_snap_menu_items[16].set_label("1/11");
    m_snap_menu_items[16].signal_activate().connect(sigc::bind(mem_fun(*this,&perfedit::set_snap), 11 ));
    m_menu_snap->append(m_snap_menu_items[16]);

    m_snap_menu_items[17].set_label("1/13");
    m_snap_menu_items[17].signal_activate().connect(sigc::bind(mem_fun(*this,&perfedit::set_snap), 13 ));
    m_menu_snap->append(m_snap_menu_items[17]);

    m_snap_menu_items[18].set_label("1/14");
    m_snap_menu_items[18].signal_activate().connect(sigc::bind(mem_fun(*this,&perfedit::set_snap), 14 ));
    m_menu_snap->append(m_snap_menu_items[18]);

    m_snap_menu_items[19].set_label("1/15");
    m_snap_menu_items[19].signal_activate().connect(sigc::bind(mem_fun(*this,&perfedit::set_snap), 15 ));
    m_menu_snap->append(m_snap_menu_items[19]);

    m_snap_menu_items[20].set_label("1/18");
    m_snap_menu_items[20].signal_activate().connect(sigc::bind(mem_fun(*this,&perfedit::set_snap), 18 ));
    m_menu_snap->append(m_snap_menu_items[20]);

    m_snap_menu_items[21].set_label("1/22");
    m_snap_menu_items[21].signal_activate().connect(sigc::bind(mem_fun(*this,&perfedit::set_snap), 22 ));
    m_menu_snap->append(m_snap_menu_items[21]);

    m_snap_menu_items[22].set_label("1/26");
    m_snap_menu_items[22].signal_activate().connect(sigc::bind(mem_fun(*this,&perfedit::set_snap), 26 ));
    m_menu_snap->append(m_snap_menu_items[22]);

    m_snap_menu_items[23].set_label("1/28");
    m_snap_menu_items[23].signal_activate().connect(sigc::bind(mem_fun(*this,&perfedit::set_snap), 28 ));
    m_menu_snap->append(m_snap_menu_items[23]);

    m_snap_menu_items[24].set_label("1/30");
    m_snap_menu_items[24].signal_activate().connect(sigc::bind(mem_fun(*this,&perfedit::set_snap), 30 ));
    m_menu_snap->append(m_snap_menu_items[24]);

    /* snap */
    m_button_snap = manage( new Button());
    m_button_snap->add( *manage( new Image(Gdk::Pixbuf::create_from_xpm_data( snap_xpm ))));
    m_button_snap->signal_clicked().connect(  sigc::bind<Menu *>( mem_fun( *this, &perfedit::popup_menu), m_menu_snap  ));
    add_tooltip( m_button_snap, "Grid snap. (Fraction of Measure Length)" );
    m_entry_snap = manage( new Entry());
    m_entry_snap->set_width_chars(4);
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

        MenuItem * menu_item = new MenuItem(num);
        menu_item->signal_activate().connect(sigc::bind(mem_fun(*this,&perfedit::xpose_button_callback), i ));
        m_menu_xpose->insert(*menu_item, 0);
    }

    m_button_xpose = manage( new Button());
    m_button_xpose->add( *manage( new Image(Gdk::Pixbuf::create_from_xpm_data( transpose_xpm ))));
    m_button_xpose->signal_clicked().connect(  sigc::bind<Menu *>( mem_fun( *this, &perfedit::popup_menu), m_menu_xpose  ));
    add_tooltip( m_button_xpose, "Song transpose" );
    m_entry_xpose = manage( new Entry());
    m_entry_xpose->set_width_chars(4);
    m_entry_xpose->set_editable( false );

    m_menu_bp_measure = manage( new Menu() );
    m_menu_bw = manage( new Menu() );

    m_bw_menu_items.resize(5);

    m_bw_menu_items[0].set_label("1");
    m_bw_menu_items[0].signal_activate().connect(sigc::bind(mem_fun(*this, &perfedit::bw_button_callback), 1 ));
    m_menu_bw->append(m_bw_menu_items[0]);

    m_bw_menu_items[1].set_label("2");
    m_bw_menu_items[1].signal_activate().connect(sigc::bind(mem_fun(*this, &perfedit::bw_button_callback), 2 ));
    m_menu_bw->append(m_bw_menu_items[1]);

    m_bw_menu_items[2].set_label("4");
    m_bw_menu_items[2].signal_activate().connect(sigc::bind(mem_fun(*this, &perfedit::bw_button_callback), 4 ));
    m_menu_bw->append(m_bw_menu_items[2]);

    m_bw_menu_items[3].set_label("8");
    m_bw_menu_items[3].signal_activate().connect(sigc::bind(mem_fun(*this, &perfedit::bw_button_callback), 8 ));
    m_menu_bw->append(m_bw_menu_items[3]);

    m_bw_menu_items[4].set_label("16");
    m_bw_menu_items[4].signal_activate().connect(sigc::bind(mem_fun(*this, &perfedit::bw_button_callback), 16 ));
    m_menu_bw->append(m_bw_menu_items[4]);

    char b[20];

    for( int i=0; i<16; i++ )
    {
        snprintf( b, sizeof(b), "%d", i+1 );

        MenuItem * menu_item = new MenuItem(b);
        menu_item->signal_activate().connect(sigc::bind(mem_fun(*this, &perfedit::bp_measure_button_callback), i + 1 ));
        m_menu_bp_measure->append(*menu_item);
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
    
    m_button_continue = manage( new ToggleButton( "S" ) );
    m_button_continue->signal_toggled().connect(  mem_fun( *this, &perfedit::set_continue_callback ));
    add_tooltip( m_button_continue, "Toggle to set 'Stop/Pause' button method.\n"
            "If set to 'S', stopping transport will reposition to 'L' mark.\n"
            "If set to 'P', stopping transport will not reposition to 'L' mark." );
    
    m_button_continue->set_active( false );
    
    m_button_stop = manage( new Button() );
    m_button_stop->add(*manage( new Image(Gdk::Pixbuf::create_from_xpm_data( stop_xpm ))));
    m_button_stop->signal_clicked().connect( mem_fun( *this, &perfedit::stop_playing));
    add_tooltip( m_button_stop, "Stop/Pause playing." );

    m_button_play = manage( new Button() );
    m_button_play->add(*manage( new Image(Gdk::Pixbuf::create_from_xpm_data( play2_xpm ))));
    m_button_play->signal_clicked().connect(  mem_fun( *this, &perfedit::start_playing));
    add_tooltip( m_button_play, "Begin/Continue playing." );

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

    m_hlbox->pack_start( *m_button_continue, false, false );
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
        if ( a_ev->keyval == GDK_KEY_z || a_ev->keyval == GDK_KEY_Z )
        {
            undo();
            return true;
        }
        /* Ctrl-R: Redo */
        if ( a_ev->keyval == GDK_KEY_r || a_ev->keyval == GDK_KEY_R )
        {
            redo();
            return true;
        }
    }

    if (a_ev->keyval == GDK_KEY_Z)         /* zoom in              */
    {
        set_zoom(m_perfroll->m_zoom / 2);
        return true;
    }
    else if (a_ev->keyval == GDK_KEY_0)         /* reset to normal zoom */
    {
        set_zoom(c_perf_scale_x);
        return true;
    }
    else if (a_ev->keyval == GDK_KEY_z)         /* zoom out             */
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

        if ( a_ev->keyval ==  m_mainperf->m_key_forward )
        {
            fast_forward(true);
            return true;
        }

        if ( a_ev->keyval ==  m_mainperf->m_key_rewind )
        {
            rewind(true);
            return true;
        }

        if(m_mainperf->get_playlist_mode())
        {
            if(a_ev->keyval == m_mainperf->m_key_playlist_prev)
            {
                m_mainperf->m_setjump = -1;
                return true;
            }

            if(a_ev->keyval == m_mainperf->m_key_playlist_next)
            {
                m_mainperf->m_setjump = 1;
                return true;
            }
        }
#ifdef JACK_SUPPORT
        if ( a_ev->keyval ==  m_mainperf->m_key_jack )
        {
            toggle_jack();
            return true;
        }
#endif // JACK_SUPPORT
        if ( a_ev->keyval ==  m_mainperf->m_key_export_trigger )
        {
            if(m_mainperf->is_active(m_perfroll->get_drop_sequence()))
            {
                m_mainperf->get_sequence(m_perfroll->get_drop_sequence())->set_trigger_export();
                m_mainwnd->export_seq_track_trigger(E_MIDI_SOLO_TRIGGER, m_perfroll->get_drop_sequence());
            }
        }
    }

    if(!event_was_handled)
    {
        return Gtk::Window::on_key_press_event(a_ev);
    }

    return false;
}

bool
perfedit::on_key_release_event(GdkEventKey* a_ev)
{
    if ( a_ev->type == GDK_KEY_RELEASE )
    {
        if ( a_ev->keyval ==  m_mainperf->m_key_forward )
        {
            fast_forward(false);
            return true;
        }
        if ( a_ev->keyval ==  m_mainperf->m_key_rewind )
        {
            rewind(false);
            return true;
        }
    }

    /* For CTRL-L paste trigger */
    return m_perftime->on_key_release_event(a_ev);
}

void
perfedit::undo()
{
    m_mainperf->pop_trigger_undo();
    m_perfroll->redraw_all_tracks();
}

void
perfedit::redo()
{
    m_mainperf->pop_trigger_redo();
    m_perfroll->redraw_all_tracks();
}

void
perfedit::start_playing()
{
    m_mainperf->set_start_from_perfedit(true);
    m_mainperf->start_playing();
}

void
perfedit::set_continue_callback()
{
    m_mainperf->set_continue(m_button_continue->get_active());
    bool is_active = m_button_continue->get_active();
    
    std::string label = is_active ? "P" : "S";
    Gtk::Label * lblptr(dynamic_cast<Gtk::Label *>
    (
         m_button_continue->get_child())
    );
    if (lblptr != NULL)
        lblptr->set_text(label);
}

void
perfedit::stop_playing()
{
    m_mainperf->stop_playing();
}

void
perfedit::rewind(bool a_press)
{
    if(a_press)
    {
        if(FF_RW_button_type == FF_RW_REWIND) // for key repeat, just ignore repeat
            return;
        
        FF_RW_button_type = FF_RW_REWIND;
    }
    else
        FF_RW_button_type = FF_RW_RELEASE;

    g_timeout_add(120,FF_RW_timeout,m_mainperf);
}

void
perfedit::fast_forward(bool a_press)
{
    if(a_press)
    {
        if(FF_RW_button_type == FF_RW_FORWARD) // for key repeat, just ignore repeat
            return;
        
        FF_RW_button_type = FF_RW_FORWARD;
    }
    else
        FF_RW_button_type = FF_RW_RELEASE;

    g_timeout_add(120,FF_RW_timeout,m_mainperf);
}

void
perfedit::collapse()
{
    m_mainperf->push_trigger_undo(-1); //  all tracks
    m_mainperf->move_triggers( false );
    m_perfroll->redraw_all_tracks();
}

void
perfedit::copy()
{
    m_mainperf->push_trigger_undo(-1); // all tracks
    m_mainperf->copy_triggers(  );
    m_perfroll->redraw_all_tracks();
}

void
perfedit::expand()
{
    m_mainperf->push_trigger_undo(-1); // all tracks
    m_mainperf->move_triggers( true );
    m_perfroll->redraw_all_tracks();
}

void
perfedit::paste_triggers(long paste_tick) // all tracks
{
    m_mainperf->push_trigger_undo(-1);
    m_mainperf->paste_triggers ( paste_tick );
    m_perfroll->redraw_all_tracks();
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

    // for setting the transport tick to display in the correct location
    // FIXME currently does not work for slave from disconnected - need jack position
    if(global_song_start_mode)
    {
        m_mainperf->set_starting_tick(m_mainperf->get_left_tick());
 
        if(m_mainperf->is_jack_running() && m_mainperf->is_jack_master() )
            m_mainperf->position_jack(true, m_mainperf->get_left_tick());

        m_mainperf->set_reposition();
    }
    else
        m_mainperf->set_starting_tick(m_mainperf->get_tick());

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
    a_menu->show_all();
    a_menu->popup_at_pointer(NULL);
}

void
perfedit::set_guides()
{
    long measure_ticks = (c_ppqn * 4) * m_bp_measure / m_bw;
    long snap_ticks =  measure_ticks / m_snap;
    long beat_ticks = (c_ppqn * 4) / m_bw;
    m_perfroll->set_guides( snap_ticks, measure_ticks, beat_ticks );
    m_perftime->set_guides( snap_ticks, measure_ticks );
    m_tempo->set_guides( snap_ticks, measure_ticks);
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
    if(a_beats_per_measure < 1 || a_beats_per_measure > 16)
        a_beats_per_measure = 4;

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
    if(a_beat_width < 1 || a_beat_width > 16)
        a_beat_width = 4;

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
perfedit::update_start_BPM(double bpm)
{
    m_tempo->set_start_BPM(bpm);
}

void
perfedit::clear_tempo_list()
{
    m_tempo->clear_tempo_list();
}

void
perfedit::set_zoom (int z)
{
    m_perfroll->set_zoom(z);
    m_perftime->set_zoom(z);
    m_tempo->set_zoom(z);
}

void
perfedit::set_follow_transport()
{
    m_mainperf->set_follow_transport(m_button_follow->get_active());
    m_perfroll->have_stopped_reposition();
}

void
perfedit::toggle_follow_transport()
{
    // Note that this will trigger the button signal callback.
    m_button_follow->set_active( ! m_button_follow->get_active() );
}

/* File loading of tempo class tempo markers */
void 
perfedit::load_tempo_list()
{
    m_tempo->load_tempo_list();
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
    
    /* Calculate the current time/BBT, and display it. */
    if (global_is_running /*|| m_mainperf->get_reposition() */|| m_toggle_time_type)
    {
        m_toggle_time_type = false;
        update_clock();
    }

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

void 
perfedit::update_clock()
{
    long ticks = m_mainperf->get_tick();
    if (m_tick_time_as_bbt)
    {
        std::string t = "<b><span foreground=\"#7FFE00\" size=\"14000\">";
        t += tick_to_measurestring(ticks);
        t += "</span></b>";
        m_tick_time->set_markup(t);
    }
    else
    {
        std::string t = "<b><span foreground=\"#7FFE00\" size=\"14000\">";
        t += tick_to_timestring(ticks);
        t += "</span></b>";
        m_tick_time->set_markup(t);
    }
}

/**
 * Needed when transport is not running if reposition by FF, RW or P pointer.
 * Updates the perfroll progress line.
 */
void
perfedit::reposition_progress_line()
{
    m_perfroll->have_stopped_reposition();
}

double
perfedit::tempo_map_microseconds(unsigned long a_tick)
{
    /* live mode - we ignore tempo changes so use the first tempo only */
    if(!global_song_start_mode && !m_mainperf->get_start_from_perfedit() && !m_mainperf->get_reposition())
    {
        tempo_mark first_tempo = (* m_mainperf->m_list_no_stop_markers.begin());
        return ticks_to_delta_time_us (a_tick, first_tempo.bpm, c_ppqn);
    }
    
    /* song mode - cycle through tempo map */
    double hold_microseconds = 0;
    
    list<tempo_mark>::iterator i;
    tempo_mark last_tempo = (*--m_mainperf->m_list_no_stop_markers.end());
    
    for ( i = ++m_mainperf->m_list_no_stop_markers.begin(); i != m_mainperf->m_list_no_stop_markers.end(); ++i )
    {
        if( a_tick >= (*i).tick )
        {
            hold_microseconds = (*i).microseconds_start;
        }
        else
        {
            last_tempo = (*--i);
            break;
        }
    }
    
    uint64_t end_tick = a_tick - last_tempo.tick;
   
    return hold_microseconds + ticks_to_delta_time_us (end_tick, last_tempo.bpm, c_ppqn);
}

std::string
perfedit::tick_to_timestring (long a_tick)
{
    unsigned long microseconds = tempo_map_microseconds(a_tick);
    int seconds = int(microseconds / 1000000UL);
    int minutes = seconds / 60;
    int hours = seconds / (60 * 60);
    minutes -= hours * 60;
    seconds -= (hours * 60 * 60) + (minutes * 60);
    microseconds -= (hours * 60 * 60 + minutes * 60 + seconds) * 1000000UL;

    char tmp[32];
    snprintf(tmp, sizeof tmp, "%03d:%d:%02d", hours, minutes, seconds);
    return std::string(tmp);
}

void
perfedit::tick_to_midi_measures ( long a_tick, int &measures, int &beats, int &divisions )
{
    static const double s_epsilon = 0.000001;   /* HMMMMMMMMMMMMMMMMMMMMMMM */
    int W = m_mainperf->get_bw();
    int P = c_ppqn;
    int B = m_mainperf->get_bp_measure();
    bool result = (W > 0) && (P > 0) && (B > 0);
    if (result)
    {
        double m = a_tick * W / (4.0 * P * B);       /* measures, whole.frac     */
        double m_whole = floor(m);              /* holds integral measures  */
        m -= m_whole;                           /* get fractional measure   */
        double b = m * B;                       /* beats, whole.frac        */
        double b_whole = floor(b);              /* get integral beats       */
        b -= b_whole;                           /* get fractional beat      */
        double pulses_per_beat = 4 * P / W;     /* pulses/qn * qn/beat      */
        measures = (int(m_whole + s_epsilon) + 1);
        beats = (int(b_whole + s_epsilon) + 1);
        divisions = (int(b * pulses_per_beat + s_epsilon));
    }
}

std::string
perfedit::tick_to_measurestring (long a_tick )
{
    int measures = 0;
    int beats = 0;
    int divisions = 0;

    char tmp[32];

    tick_to_midi_measures( a_tick, measures, beats, divisions );
    snprintf
    (
        tmp, sizeof tmp, "%03d:%d:%03d",
        measures, beats, divisions
    );
    return std::string(tmp);
}

void
perfedit::toggle_time_format ()
{
    m_tick_time_as_bbt = ! m_tick_time_as_bbt;
    std::string label = m_tick_time_as_bbt ? "BBT" : "HMS" ;
    Gtk::Label * lbl(dynamic_cast<Gtk::Label *>(m_button_time_type->get_child()));
    if (lbl != NULL)
    {
        lbl->set_text(label);
        m_toggle_time_type = true;
    }
}

void
perfedit::hide_tempo_popup()
{
    m_tempo->hide_tempo_popup();
}

void
perfedit::set_perfroll_marker_change(bool a_change)
{
    m_perfroll->set_marker_changed(a_change);
    m_perfroll->redraw_all_tracks();
}

void
perfedit::set_marker_line_selection(uint64_t a_tick)
{
    m_perfroll->set_marker_line_selection(a_tick);
    m_perfroll->redraw_all_tracks();
}

int
FF_RW_timeout(void *arg)
{
    perform *p = (perform *) arg;

    if(FF_RW_button_type != FF_RW_RELEASE)
    {
        p->FF_rewind();
        if(p->m_excell_FF_RW < 60.0f)
            p->m_excell_FF_RW *= 1.1f;
        return (TRUE);
    }

    p->m_excell_FF_RW = 1.0;
    return (FALSE);
}
