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

#include <cctype>
#include <csignal>
#include <cerrno>
#include <cstring>

#include "mainwnd.h"
#include "perform.h"
#include "midifile.h"
#include "perfedit.h"

#include "pixmaps/play2.xpm"
#include "pixmaps/stop.xpm"
#include "pixmaps/learn.xpm"
#include "pixmaps/learn2.xpm"
#include "pixmaps/perfedit.xpm"
#include "pixmaps/seq32.xpm"
#include "pixmaps/seq32_32.xpm"
#include "pixmaps/seq32_playlist.xpm"
#include "pixmaps/menu.xpm"

bool global_is_running = false;
bool global_is_modified = false;

#define add_tooltip( obj, text ) obj->set_tooltip_text( text);

mainwnd::mainwnd(perform *a_p, Glib::RefPtr<Gtk::Application> app):
    m_mainperf(a_p),
    m_app(app),
    m_menu_mode(false),
    m_perf_edit(NULL),
    m_options(NULL)
#ifdef NSM_SUPPORT
    ,m_nsm_visible(true)
    ,m_dirty_flag(false)
    ,m_nsm(NULL)
#endif
{
    set_icon(Gdk::Pixbuf::create_from_xpm_data(seq32_32_xpm));

    /* register for notification */
    m_mainperf->m_notify.push_back( this );

    /* main window */
    update_window_title();

    m_accelgroup = Gtk::AccelGroup::create();
    add_accel_group(m_accelgroup);

    m_main_wid = manage( new mainwid( m_mainperf, this));
    m_main_time = manage( new maintime( ));

    m_menubar = manage(new MenuBar());

    m_menu_file = manage(new Menu());

    m_menu_recent = nullptr;

    m_menu_edit = manage(new Menu());

    m_menu_help = manage(new Menu());

    m_file_menu_items.resize(8);
    m_edit_menu_items.resize(8);

    m_file_menu_items[0].set_label("_File");
    m_file_menu_items[0].set_use_underline(true);
    m_file_menu_items[0].set_submenu(*m_menu_file);

    m_menubar->append(m_file_menu_items[0]);

    m_edit_menu_items[0].set_label("_Edit");
    m_edit_menu_items[0].set_use_underline(true);
    m_edit_menu_items[0].set_submenu(*m_menu_edit);

    m_menubar->append(m_edit_menu_items[0]);

    m_help_submenu_item.set_label("_Help");
    m_help_submenu_item.set_use_underline(true);
    m_help_submenu_item.set_submenu(*m_menu_help);

    m_menubar->append(m_help_submenu_item);

    /* file menu items */
    m_file_menu_items[1].set_label("_New");
    m_file_menu_items[1].set_use_underline(true);
    m_file_menu_items[1].add_accelerator("activate", m_accelgroup, 'n', Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
    m_file_menu_items[1].signal_activate().connect(mem_fun(*this, &mainwnd::file_new));
    m_menu_file->append(m_file_menu_items[1]);

    m_file_menu_items[2].set_label("_Open");
    m_file_menu_items[2].set_use_underline(true);
    m_file_menu_items[2].add_accelerator("activate", m_accelgroup, 'o', Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
    m_file_menu_items[2].signal_activate().connect(mem_fun(*this, &mainwnd::file_open));
    m_menu_file->append(m_file_menu_items[2]);

    /* Add the recent files submenu */
    update_recent_files_menu();

    m_file_menu_items[3].set_label("Open _playlist...");
    m_file_menu_items[3].set_use_underline(true);
    m_file_menu_items[3].signal_activate().connect(mem_fun(*this, &mainwnd::file_open_playlist));
    m_menu_file->append(m_file_menu_items[3]);

    m_menu_file->append(m_menu_separator1);

    m_file_menu_items[4].set_label("_Save");
    m_file_menu_items[4].set_use_underline(true);
    m_file_menu_items[4].add_accelerator("activate", m_accelgroup, 's', Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
    m_file_menu_items[4].signal_activate().connect(mem_fun(*this, &mainwnd::file_save));
    m_menu_file->append(m_file_menu_items[4]);

    m_file_menu_items[5].set_label("Save _as...");
    m_file_menu_items[5].set_use_underline(true);
    m_file_menu_items[5].add_accelerator("activate", m_accelgroup, 's', Gdk::CONTROL_MASK | Gdk::SHIFT_MASK, Gtk::ACCEL_VISIBLE);
    m_file_menu_items[5].signal_activate().connect(sigc::bind(mem_fun(*this, &mainwnd::file_save_as), E_MIDI_SEQ32_FORMAT, c_no_export_sequence));
    m_menu_file->append(m_file_menu_items[5]);

    m_menu_file->append(m_menu_separator2);

    m_file_menu_items[6].set_label("O_ptions...");
    m_file_menu_items[6].set_use_underline(true);
    m_file_menu_items[6].signal_activate().connect(mem_fun(*this, &mainwnd::options_dialog));
    m_menu_file->append(m_file_menu_items[6]);

    m_menu_file->append(m_menu_separator3);

    m_file_menu_items[7].set_label("E_xit");
    m_file_menu_items[7].set_use_underline(true);
    m_file_menu_items[7].add_accelerator("activate", m_accelgroup, 'q', Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
    m_file_menu_items[7].signal_activate().connect(mem_fun(*this, &mainwnd::file_exit));
    m_menu_file->append(m_file_menu_items[7]);

    /* edit menu items */
    m_edit_menu_items[1].set_label("_Song_Editor");
    m_edit_menu_items[1].set_use_underline(true);
    m_edit_menu_items[1].add_accelerator("activate", m_accelgroup, 'e', Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);
    m_edit_menu_items[1].signal_activate().connect(mem_fun(*this, &mainwnd::open_performance_edit));
    m_menu_edit->append(m_edit_menu_items[1]);

    m_edit_menu_items[2].set_label("_Apply song transpose");
    m_edit_menu_items[2].set_use_underline(true);
    m_edit_menu_items[2].signal_activate().connect(mem_fun(*this, &mainwnd::apply_song_transpose));
    m_menu_edit->append(m_edit_menu_items[2]);

    m_menu_edit->append(m_menu_separator4);

    m_edit_menu_items[3].set_label("_Mute all tracks");
    m_edit_menu_items[3].set_use_underline(true);
    m_edit_menu_items[3].signal_activate().connect(sigc::bind(mem_fun(*this, &mainwnd::set_song_mute), MUTE_ON));
    m_menu_edit->append(m_edit_menu_items[3]);

    m_edit_menu_items[4].set_label("_Unmute all tracks");
    m_edit_menu_items[4].set_use_underline(true);
    m_edit_menu_items[4].signal_activate().connect(sigc::bind(mem_fun(*this, &mainwnd::set_song_mute), MUTE_OFF));
    m_menu_edit->append(m_edit_menu_items[4]);

    m_edit_menu_items[5].set_label("_Toggle mute all tracks");
    m_edit_menu_items[5].set_use_underline(true);
    m_edit_menu_items[5].signal_activate().connect(sigc::bind(mem_fun(*this, &mainwnd::set_song_mute), MUTE_TOGGLE));
    m_menu_edit->append(m_edit_menu_items[5]);

    m_menu_edit->append(m_menu_separator5);

    m_edit_menu_items[6].set_label("_Import midi");
    m_edit_menu_items[6].set_use_underline(true);
    m_edit_menu_items[6].signal_activate().connect(mem_fun(*this, &mainwnd::file_import_dialog));
    m_menu_edit->append(m_edit_menu_items[6]);

    m_edit_menu_items[7].set_label("Midi export _song");
    m_edit_menu_items[7].set_use_underline(true);
    m_edit_menu_items[7].signal_activate().connect(sigc::bind(mem_fun(*this, &mainwnd::file_save_as), E_MIDI_SONG_FORMAT, c_no_export_sequence));
    m_menu_edit->append(m_edit_menu_items[7]);

    /* help menu items */
    m_help_menu_item.set_label("_About...");
    m_help_menu_item.set_use_underline(true);
    m_help_menu_item.signal_activate().connect(mem_fun(*this, &mainwnd::about_dialog));
    m_menu_help->append(m_help_menu_item);

    /* top line items */
    tophbox = manage( new HBox( false, 0 ) );
    
    m_image_seq32 = manage(new Image( Gdk::Pixbuf::create_from_xpm_data(seq32_xpm)));
    tophbox->pack_start(*m_image_seq32, false, false);

    m_button_mode = manage( new ToggleButton( " Live " ) );
    m_button_mode->set_can_focus(false);
    m_button_mode->signal_toggled().connect(  mem_fun( *this, &mainwnd::set_song_mode ));
    add_tooltip( m_button_mode, "Toggle song mode (or live/sequence mode)." );
    if(global_song_start_mode)
    {
        m_button_mode->set_active( true );
    }
    tophbox->pack_start(*m_button_mode, false, false );

    m_button_menu = manage( new ToggleButton() );
    m_button_menu->add(*manage( new Image(Gdk::Pixbuf::create_from_xpm_data( menu_xpm ))));
    m_button_menu->signal_toggled().connect(  mem_fun( *this, &mainwnd::set_menu_mode ));
    add_tooltip( m_button_menu, "Toggle to disable/enable menu\n"
                 "when sequencer is NOT running.\n\n"
                 "The menu is disabled when the\n"
                 "sequencer IS running.");
    tophbox->pack_start(*m_button_menu, false, false );

    // adjust placement...
    VBox *vbox_b = manage( new VBox() );
    HBox *hbox3 = manage( new HBox( false, 0 ) );
    vbox_b->pack_start( *hbox3, false, false );
    tophbox->pack_end( *vbox_b, false, false );
    hbox3->set_spacing( 10 );

    /* timeline */
    hbox3->pack_start( *m_main_time, false, false );

    /* group learn button */
    m_button_learn = manage( new Button( ));
    m_button_learn->set_focus_on_click( false );
    m_button_learn->set_can_focus(false);   // FIXME check this
  //  m_button_learn->set_flags( m_button_learn->get_flags() & ~Gtk::CAN_FOCUS );
    m_button_learn->set_image(*manage(new Image(
                                          Gdk::Pixbuf::create_from_xpm_data( learn_xpm ))));
    m_button_learn->signal_clicked().connect(
        mem_fun(*this, &mainwnd::learn_toggle));
    add_tooltip( m_button_learn, "Mute Group Learn\n\n"
                 "Click 'L' then press a mutegroup key to store the mute state of "
                 "the sequences in that key.\n\n"
                 "(see File/Options/Keyboard for available mutegroup keys "
                 "and the corresponding hotkey for the 'L' button)" );
    hbox3->pack_end( *m_button_learn, false, false );

    /* bottom line items */
    HBox *bottomhbox = manage( new HBox(false, 10));

    /* container for start+stop buttons */
    HBox *startstophbox = manage(new HBox(false, 4));
    bottomhbox->pack_start(*startstophbox, Gtk::PACK_SHRINK);

    /* stop button */
    m_button_stop = manage( new Button());
    m_button_stop->add(*manage(new Image(
                                   Gdk::Pixbuf::create_from_xpm_data( stop_xpm ))));
    m_button_stop->signal_clicked().connect(
        mem_fun(*this, &mainwnd::stop_playing));
    add_tooltip( m_button_stop, "Stop playing MIDI sequence" );
    startstophbox->pack_start(*m_button_stop, Gtk::PACK_SHRINK);

    /* play button */
    m_button_play = manage(new Button() );
    m_button_play->add(*manage(new Image(
                                   Gdk::Pixbuf::create_from_xpm_data( play2_xpm ))));
    m_button_play->signal_clicked().connect(
        mem_fun( *this, &mainwnd::start_playing));
    add_tooltip( m_button_play, "Play MIDI sequence" );
    startstophbox->pack_start(*m_button_play, Gtk::PACK_SHRINK);

    /* bpm spin button with label*/
    HBox *bpmhbox = manage(new HBox(false, 4));
    bottomhbox->pack_start(*bpmhbox, Gtk::PACK_SHRINK);

    m_adjust_bpm = Adjustment::create(c_bpm, c_bpm_minimum, c_bpm_maximum, 1);
    m_spinbutton_bpm = manage( new SpinButton( m_adjust_bpm ));
    m_spinbutton_bpm->set_name( "BPM Edit" );
    m_spinbutton_bpm->set_editable( true );
    m_spinbutton_bpm->set_digits(2);                    // 2 = two decimal precision
    m_spinbutton_bpm->signal_value_changed().connect(mem_fun(*this, &mainwnd::adj_callback_bpm ));

    add_tooltip( m_spinbutton_bpm, "Adjust beats per minute (BPM) value");

    Label* bpmlabel = manage(new Label("_BPM", true));
    bpmlabel->set_mnemonic_widget(*m_spinbutton_bpm);
    bpmhbox->pack_start(*bpmlabel, Gtk::PACK_SHRINK);
    bpmhbox->pack_start(*m_spinbutton_bpm, Gtk::PACK_SHRINK);

    /* bpm tap tempo button - sequencer64 */
    m_button_tap = manage(new Button("0"));
    m_button_tap->signal_clicked().connect(mem_fun(*this, &mainwnd::tap));
    add_tooltip
    (
        m_button_tap,
        "Tap in time to set the beats per minute (BPM) value. "
        "After 5 seconds of no taps, the tap-counter will reset to 0. "
        "Also see the File / Options / Keyboard / Tap BPM key assignment."
    );
    bpmhbox->pack_start( *m_button_tap, false, false );

    /* screen set name edit line */
    HBox *notebox = manage(new HBox(false, 4));
    bottomhbox->pack_start(*notebox, Gtk::PACK_EXPAND_WIDGET);

    m_entry_notes = manage( new Entry());
    m_entry_notes->set_name( "Screen Name" );
    m_entry_notes->signal_changed().connect(
        mem_fun(*this, &mainwnd::edit_callback_notepad));
    m_entry_notes->set_text(*m_mainperf->get_screen_set_notepad(
                                m_mainperf->get_screenset()));
    add_tooltip( m_entry_notes, "Enter screen set name" );
    Label* notelabel = manage(new Label("_Name", true));
    notelabel->set_mnemonic_widget(*m_entry_notes);
    notebox->pack_start(*notelabel, Gtk::PACK_SHRINK);
    notebox->pack_start(*m_entry_notes, Gtk::PACK_EXPAND_WIDGET);

    /* sequence set spin button */
    HBox *sethbox = manage(new HBox(false, 4));
    bottomhbox->pack_start(*sethbox, Gtk::PACK_SHRINK);

    m_adjust_ss = Adjustment::create( 0, 0, c_max_sets - 1, 1 );
    m_spinbutton_ss = manage( new SpinButton( m_adjust_ss ));
    m_spinbutton_ss->set_editable( true );
    m_spinbutton_ss->set_wrap( true );
    m_spinbutton_ss->signal_value_changed().connect(
        mem_fun(*this, &mainwnd::adj_callback_ss ));
    
    add_tooltip( m_spinbutton_ss, "Select screen set" );
    Label* setlabel = manage(new Label("_Set", true));
    setlabel->set_mnemonic_widget(*m_spinbutton_ss);
    sethbox->pack_start(*setlabel, Gtk::PACK_SHRINK);
    sethbox->pack_start(*m_spinbutton_ss, Gtk::PACK_SHRINK);

    /* song edit button */
    m_button_perfedit = manage( new Button( ));
    m_button_perfedit->add( *manage( new Image(
                                         Gdk::Pixbuf::create_from_xpm_data( perfedit_xpm  ))));
    m_button_perfedit->signal_clicked().connect(
        mem_fun( *this, &mainwnd::open_performance_edit ));
    add_tooltip( m_button_perfedit, "Show or hide song editor window" );
    bottomhbox->pack_end(*m_button_perfedit, Gtk::PACK_SHRINK);

    /* vertical layout container for window content*/
    VBox *contentvbox = new VBox();
    contentvbox->set_spacing(10);
    contentvbox->set_border_width(10);
    contentvbox->pack_start(*tophbox, Gtk::PACK_SHRINK);
    contentvbox->pack_start(*m_main_wid, Gtk::PACK_SHRINK);
    contentvbox->pack_start(*bottomhbox, Gtk::PACK_SHRINK);

    /*main container for menu and window content */
    VBox *mainvbox = new VBox();

    mainvbox->pack_start(*m_menubar, false, false );
    mainvbox->pack_start( *contentvbox );

    m_main_wid->set_can_focus();
    m_main_wid->grab_focus();

    /* add main layout box */
    this->add (*mainvbox);

    /* tap button  */
    m_current_beats = 0;
    m_base_time_ms  = 0;
    m_last_time_ms  = 0;

    /* show everything */
    set_position(Gtk::WIN_POS_CENTER);
    show_all();

    add_events( Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK );

    m_timeout_connect = Glib::signal_timeout().connect(
                            mem_fun(*this, &mainwnd::timer_callback), c_redraw_ms);

    m_perf_edit = new perfedit( m_mainperf, this );
}


mainwnd::~mainwnd()
{
    delete m_perf_edit;
    delete m_options;
}

/*
 * move through the playlist (jmp is 0 on start and 1 if right arrow, -1 for left arrow)
 */
bool mainwnd::playlist_jump(int jmp, bool a_verify)
{
    if(global_is_running)                       // don't allow jump if running
        return false;
    
    bool result = false;
    if(a_verify)                                // we will run through all the files
    {
        m_mainperf->set_playlist_index(0);      // start at zero
        jmp = 0;                                // to get the first one
    }

    while(1)
    {
        if(m_mainperf->set_playlist_index(m_mainperf->get_playlist_index() + jmp))
        {
            if(Glib::file_test(m_mainperf->get_playlist_current_file(), Glib::FILE_TEST_EXISTS))
            {
                if(open_file(m_mainperf->get_playlist_current_file()))
                {
                    if(a_verify)    // verify whole playlist
                    {
                        jmp = 1;    // after the first one set to 1 for jump
                        continue;   // keep going till the end of list
                    }
                    result = true;
                    break;
                }
                else
                {
                    Glib::ustring message = "Playlist file open error\n";
                    message += m_mainperf->get_playlist_current_file();
                    m_mainperf->error_message_gtk(message);
                    m_mainperf->set_playlist_mode(false);    // abandon ship
                    result = false;
                    break;  
                }
            }
            else
            {
                Glib::ustring message = "Midi playlist file does not exist\n";
                message += m_mainperf->get_playlist_current_file();
                m_mainperf->error_message_gtk(message);
                m_mainperf->set_playlist_mode(false);        // abandon ship
                result = false;
                break;  
            }
        }
        else                                                // end of file list
        {
            result = true;   // means we got to the end or beginning, without error
            break;
        }
    }
    
    if(!result)                                             // if errors occured above
    {
        update_window_title();
        update_window_xpm();
    }
    return result;
}

bool
mainwnd::verify_playlist_dialog()
{
    Gtk::MessageDialog warning("Do you wish the verify the playlist?\n",
                       false,
                       Gtk::MESSAGE_WARNING, Gtk::BUTTONS_YES_NO, true);

    auto result = warning.run();

    if (result == Gtk::RESPONSE_NO || result == Gtk::RESPONSE_DELETE_EVENT)
    {
        return false;
    }
    
    return true;
}

void
mainwnd::playlist_verify()
{
    bool result = false;
    
    result = playlist_jump(PLAYLIST_ZERO,true); // true is verify mode
    
    if(result)                                  // everything loaded
    {
        m_mainperf->set_playlist_index(0);      // set to start
        playlist_jump(PLAYLIST_ZERO);                       // load the first file
        printf("Playlist verification was successful!\n");
    }
    else                                        // verify failed somewhere
    {
        new_file();                             // clear and start clean
    }
}

// This is the GTK timer callback, used to draw our current time and bpm
// ondd_events( the main window
bool
mainwnd::timer_callback(  )
{
    long ticks = m_mainperf->get_tick();

    m_main_time->idle_progress( ticks );
    m_main_wid->update_markers( ticks );

    /* this will trigger the button callback */
    if ( m_adjust_bpm->get_value() != m_mainperf->get_bpm())
    {
        m_adjust_bpm->set_value( m_mainperf->get_bpm());
    }
    
    /* For midi control to update perfedit markers on bpm change */
    if(m_mainperf->get_update_perfedit_markers())
    {
        m_mainperf->set_update_perfedit_markers(false);
        m_perf_edit->update_start_BPM(m_mainperf->get_bpm());
    }

    if ( m_adjust_ss->get_value() !=  m_mainperf->get_screenset() )
    {
        m_main_wid->set_screenset(m_mainperf->get_screenset());
        m_adjust_ss->set_value( m_mainperf->get_screenset());
        m_entry_notes->set_text(*m_mainperf->get_screen_set_notepad(
                                    m_mainperf->get_screenset()));
    }
#ifdef JACK_SUPPORT
    /* for seqroll keybinding, this is needed here instead of */
    /* perfedit timeout() since perfedit may not be open */
    if (m_perf_edit->get_toggle_jack() != m_mainperf->get_toggle_jack())
        m_perf_edit->toggle_jack();
#endif // JACK_SUPPORT

#ifdef NSM_SUPPORT
    if(m_nsm)
    {
        nsm_check_nowait( m_nsm );
        if (m_nsm_optional_gui && m_nsm_visible != global_nsm_gui)
        {
            m_nsm_visible = global_nsm_gui;
            if (m_nsm_visible)
            {
                show();
                nsm_send_is_shown( m_nsm );
            }
            else
            {
                m_app->hold();
                close_all_windows();
                hide();
                nsm_send_is_hidden( m_nsm );
            }
        }
        if (m_dirty_flag != global_is_modified)
        {
            m_dirty_flag = global_is_modified;
            if (m_dirty_flag)
            {
                nsm_send_is_dirty ( m_nsm );
            }
            else
            {
                nsm_send_is_clean ( m_nsm );
            }
        }
    }
#endif

    if (m_button_mode->get_active() != global_song_start_mode)
        m_button_mode->set_active(global_song_start_mode);

    if(global_is_running && m_button_mode->get_sensitive())
        m_button_mode->set_sensitive(false);
    else if(!global_is_running && !m_button_mode->get_sensitive())
        m_button_mode->set_sensitive(true);

    if(global_is_running && m_menubar->get_sensitive())
        m_menubar->set_sensitive(false);
    else if(!global_is_running && (m_menubar->get_sensitive() == m_menu_mode ))
        m_menubar->set_sensitive(!m_menu_mode);

    /* Tap button - sequencer64 */
    if (m_current_beats > 0)
    {
        if (m_last_time_ms > 0)
        {
            struct timespec spec;
            clock_gettime(CLOCK_REALTIME, &spec);
            long ms = long(spec.tv_sec) * 1000;     /* seconds to ms        */
            ms += round(spec.tv_nsec * 1.0e-6);     /* nanoseconds to ms    */
            long difference = ms - m_last_time_ms;
            if (difference > 5000L)                 /* 5 second wait        */
            {
                m_current_beats = m_base_time_ms = m_last_time_ms = 0;
                set_tap_button(0);
            }
        }
    }

    /* perfedit left, right arrow keys for playlist */
    if(m_mainperf->m_setjump)
    {
        playlist_jump(m_mainperf->m_setjump);
        m_mainperf->m_setjump=0;
    }
    
    /* when in set list mode, tempo stop markers trigger set file increment.
     We have to let the transport completely stop before doing the 
     file loading or strange things happen*/
    if(m_mainperf->m_playlist_stop_mark && !global_is_running)
    {
        m_mainperf->m_playlist_stop_mark = false;
        playlist_jump(PLAYLIST_NEXT);    // next file
    }
    
    if(m_mainperf->m_playlist_midi_control_set && !global_is_running)
    {
        m_mainperf->m_playlist_midi_control_set = false;
        playlist_jump(m_mainperf->m_playlist_midi_jump_value);
        m_mainperf->m_playlist_midi_jump_value = PLAYLIST_ZERO;
    }
    
    /* Shut off the reposition flag after the reposition */
    if (!global_is_running)
    {
        if((m_mainperf->get_starting_tick() == m_mainperf->get_tick()) && m_mainperf->get_reposition())
        {
            m_perf_edit->update_clock();                // for reposition update
            m_perf_edit->reposition_progress_line();    // for perfroll
            m_mainperf->set_reposition(false);
        }
    }
    
    return true;
}

void
mainwnd::set_song_mode()
{
    global_song_start_mode = m_button_mode->get_active();

    bool is_active = m_button_mode->get_active();

    /*
     * spaces with 'Live' are to keep button width close
     * to the same when changed for cosmetic purposes.
     */

    std::string label = is_active ? "Song" : " Live ";
    Gtk::Label * lblptr(dynamic_cast<Gtk::Label *>
    (
         m_button_mode->get_child())
    );
    if (lblptr != NULL)
        lblptr->set_text(label);
}

void
mainwnd::toggle_song_mode()
{
    // Note that this will trigger the button signal callback.
    if(!global_is_running)
    {
        m_button_mode->set_active( ! m_button_mode->get_active() );
    }
}

void
mainwnd::set_menu_mode()
{
    m_menu_mode = m_button_menu->get_active();
}

void
mainwnd::toggle_menu_mode()
{
    // Note that this will trigger the button signal callback.
    m_button_menu->set_active( ! m_button_menu->get_active() );
}

void
mainwnd::open_performance_edit()
{
    if (m_perf_edit->is_visible())
        m_perf_edit->hide();
    else
    {
        m_perf_edit->init_before_show();
        m_perf_edit->show_all();
    }
}

void
mainwnd::options_dialog()
{
    delete m_options;
    m_options = new options( *this,  m_mainperf );
    m_options->show_all();
}

void
mainwnd::start_playing()
{
    m_mainperf->start_playing();
}

void
mainwnd::stop_playing()
{
    m_mainperf->stop_playing();
    m_main_wid->update_sequences_on_window();
}

void
mainwnd::on_grouplearnchange(bool state)
{
    /* respond to learn mode change from m_mainperf */
    m_button_learn->set_image(*manage(new Image(
                                          Gdk::Pixbuf::create_from_xpm_data( state ? learn2_xpm : learn_xpm))));
}

void
mainwnd::learn_toggle()
{
    if (m_mainperf->is_group_learning())
    {
        m_mainperf->unset_mode_group_learn();
    }
    else
    {
        m_mainperf->set_mode_group_learn();
    }
}

/* callback function */
void mainwnd::file_new()
{
    if (is_save())
        new_file();
}

void mainwnd::new_file()
{
    /* reset everything to default */
    if(m_mainperf->clear_all())
    {
        m_perf_edit->clear_tempo_list();
        m_perf_edit->update_start_BPM(c_bpm);
        m_perf_edit->set_bp_measure(4);
        m_perf_edit->set_bw(4);
        m_perf_edit->set_xpose(0);
        m_mainperf->set_playlist_mode(false);

        m_main_wid->reset();
        m_entry_notes->set_text( * m_mainperf->get_screen_set_notepad(
                                     m_mainperf->get_screenset() ));

        global_filename = "";
        update_window_title();
        update_window_xpm();
        global_is_modified = false;
    }
    else
    {
        new_open_error_dialog();
    }
}

/* callback function */
void mainwnd::file_save()
{
#ifdef NSM_SUPPORT
    // Do not save if in NSM session
    if(m_mainperf->get_playlist_mode() && (m_nsm != NULL))
    {
        fprintf(stderr, "Seq32 playlist mode cannot save!!\n" );
        return;
    }
#endif
    save_file();
}

/* callback function */
void mainwnd::file_save_as( file_type_e type, int a_seq )
{
    Gtk::FileChooserDialog dialog("Save file as",
                                  Gtk::FILE_CHOOSER_ACTION_SAVE);

    switch(type)
    {
    case E_MIDI_SONG_FORMAT:
        dialog.set_title("Midi export song triggers");
        break;

    case E_MIDI_SOLO_SEQUENCE:
        dialog.set_title("Midi export sequence");
        break;

    case E_MIDI_SOLO_TRIGGER:
        dialog.set_title("Midi export solo trigger");
        break;

    case E_MIDI_SOLO_TRACK:
        dialog.set_title("Midi export solo track");
        break;

    default:            // Save file as -- native seq32
        break;
    }

    dialog.set_transient_for(*this);

    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);

    auto filter_midi = Gtk::FileFilter::create();
    filter_midi->set_name("MIDI files");
    filter_midi->add_pattern("*.midi");
    filter_midi->add_pattern("*.MIDI");
    filter_midi->add_pattern("*.mid");
    filter_midi->add_pattern("*.MID");
    dialog.add_filter(filter_midi);

    auto filter_any = Gtk::FileFilter::create();
    filter_any->set_name("Any files");
    filter_any->add_pattern("*");
    dialog.add_filter(filter_any);

    dialog.set_current_folder(last_used_dir);
    int result = dialog.run();

    switch (result)
    {
    case Gtk::RESPONSE_OK:
    {
        std::string fname = dialog.get_filename();
        auto current_filter = dialog.get_filter();

        if ((current_filter) &&
                (current_filter->get_name() == "MIDI files"))
        {
            // check for MIDI file extension; if missing, add .midi
            std::string suffix = fname.substr(
                                     fname.find_last_of(".") + 1, std::string::npos);
            toLower(suffix);
            if ((suffix != "midi") && (suffix != "mid"))
                fname = fname + ".midi";
        }

        if (Glib::file_test(fname, Glib::FILE_TEST_EXISTS))
        {
            Gtk::MessageDialog warning(*this,
                                       "File already exists!\n"
                                       "Do you want to overwrite it?",
                                       false,
                                       Gtk::MESSAGE_WARNING, Gtk::BUTTONS_YES_NO, true);
            auto result = warning.run();

            if (result == Gtk::RESPONSE_NO)
                return;
        }

        if(type == E_MIDI_SEQ32_FORMAT)
        {
            global_filename = fname;
            update_window_title();
            update_window_xpm();
            save_file();
        }
        else                            // export song triggers, solo track or solo trigger
        {
            export_midi(fname, type, a_seq);
        }

        break;
    }

    default:
        break;
    }
}

void mainwnd::export_midi(const Glib::ustring& fn, file_type_e type, int a_seq)
{
    bool result = false;

    midifile f(fn);

    if(type == E_MIDI_SOLO_SEQUENCE)
        result = f.write(m_mainperf, a_seq);            // solo sequence export
    else
        result = f.write_song(m_mainperf, type, a_seq); // export song triggers, solo track or solo trigger


    if (!result)
    {
        Gtk::MessageDialog errdialog
        (
            *this,
            "Error writing file.",
            false,
            Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK,
            true
        );
        errdialog.run();
    }
}

void mainwnd::new_open_error_dialog()
{
    Gtk::MessageDialog errdialog
    (
        *this,
        "All sequence edit windows\nmust be closed before\nopening a new file.",
        false,
        Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK,
        true
    );
    errdialog.run();
}

bool mainwnd::open_file(const Glib::ustring& fn)
{
    bool result;

    /* reset everything to default */
    if(m_mainperf->clear_all())
    {
        m_perf_edit->clear_tempo_list();
        m_perf_edit->set_xpose(0);

        midifile f(fn);
        result = f.parse(m_mainperf, this, 0);

        global_is_modified = !result; /* this means good file = NOT modified and bad = modified?? */

        if (!result)
        {
            Gtk::MessageDialog errdialog(*this,
                                         "Error reading file: " + fn, false,
                                         Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
            errdialog.run();
            global_filename = "";
            new_file();
            return false;
        }

        last_used_dir = fn.substr(0, fn.rfind("/") + 1);
        global_filename = fn;
        
        if(!m_mainperf->get_playlist_mode())           /* don't list files from playlist */
        {
            m_mainperf->add_recent_file(fn);           /* from Oli Kester's Kepler34/Sequencer 64       */
            update_recent_files_menu();
        }
        
        update_window_title();
        update_window_xpm();

        m_main_wid->reset();
        m_entry_notes->set_text(*m_mainperf->get_screen_set_notepad(
                                    m_mainperf->get_screenset()));
    }
    else
    {
        new_open_error_dialog();
        return false;
    }

    return true;
}

void
mainwnd::export_seq_track_trigger(file_type_e type, int a_seq)
{
    file_save_as(type, a_seq);
}

/*callback function*/
void mainwnd::file_open()
{
    if (is_save())
        choose_file();
}

/*callback function*/
void mainwnd::file_open_playlist()
{
    if (is_save())
    {
        choose_file(true);
    }
}

void mainwnd::choose_file(const bool playlist_mode)
{
    Gtk::FileChooserDialog dialog("Open MIDI file",
                                  Gtk::FILE_CHOOSER_ACTION_OPEN);
    dialog.set_transient_for(*this);

    if(playlist_mode)
    	dialog.set_title("Open Playlist file");

    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

    if(!playlist_mode)
    {
        auto filter_midi = Gtk::FileFilter::create();
        filter_midi->set_name("MIDI files");
        filter_midi->add_pattern("*.MIDI");
        filter_midi->add_pattern("*.midi");
        filter_midi->add_pattern("*.MID");
        filter_midi->add_pattern("*.mid");
        dialog.add_filter(filter_midi);
    }

    auto filter_any = Gtk::FileFilter::create();
    filter_any->set_name("Any files");
    filter_any->add_pattern("*");
    dialog.add_filter(filter_any);

    dialog.set_current_folder(last_used_dir);

    int result = dialog.run();

    switch(result)
    {
    case(Gtk::RESPONSE_OK):
        if(playlist_mode)
        {
            m_mainperf->set_playlist_mode(true);
            m_mainperf->set_playlist_file(dialog.get_filename());
            
            if(m_mainperf->get_playlist_mode())  // true means file load with no errors
            {
                if(verify_playlist_dialog())
                {
                    playlist_verify();
                }
                else
                {
                    playlist_jump(PLAYLIST_ZERO);
                }

                update_window_title();
                update_window_xpm();
            }
        }
        else
        {
            m_mainperf->set_playlist_mode(playlist_mode); // playlist_mode is false to clear flag
            if(!open_file(dialog.get_filename()))
            {
                update_window_title();                  // since we cleared flag above but fail does not update
                update_window_xpm();
            }
                
        }
        break;
    default:
        break;
    }
}

bool mainwnd::save_file()
{
    bool result = false;

    if (global_filename == "")
    {
        file_save_as(E_MIDI_SEQ32_FORMAT, c_no_export_sequence);
        return true;
    }

    midifile f(global_filename);

    result = f.write(m_mainperf, c_no_export_sequence);

    if (result && !m_mainperf->get_playlist_mode())            /* don't list files from playlist */
    {
        m_mainperf->add_recent_file(global_filename);
        update_recent_files_menu();
    }
    else if (!result)
    {
        Gtk::MessageDialog errdialog(*this,
                                     "Error writing file.", false,
                                     Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
        errdialog.run();
    }

    global_is_modified = !result; /* means if file is saved then clear the modified flag */

    return result;
}

int mainwnd::query_save_changes()
{
    Glib::ustring query_str;

    if (global_filename == "")
        query_str = "Unnamed file was changed.\nSave changes?";
    else
        query_str = "File '" + global_filename + "' was changed.\n"
                    "Save changes?";

    Gtk::MessageDialog dialog(*this, query_str, false,
                              Gtk::MESSAGE_QUESTION,
                              Gtk::BUTTONS_NONE, true);

    dialog.add_button(Gtk::Stock::YES, Gtk::RESPONSE_YES);
    dialog.add_button(Gtk::Stock::NO, Gtk::RESPONSE_NO);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

    return dialog.run();
}

bool mainwnd::is_save()
{
    bool result = false;

    if (global_is_modified)
    {
        int choice = query_save_changes();
        switch (choice)
        {
        case Gtk::RESPONSE_YES:
            if (save_file())
                result = true;
            break;
        case Gtk::RESPONSE_NO:
            result = true;
            break;
        case Gtk::RESPONSE_CANCEL:
        default:
            break;
        }
    }
    else
        result = true;

    return result;
}

/**
 *  Sets up the recent MIDI files menu.  If the menu already exists, delete it.
 *  Then recreate the new menu named "&Recent MIDI files...".  Add all of the
 *  entries present in the m_minperf->recent_files_count() list.  Hook each entry up
 *  to the open_file() function with each file-name as a parameter.  If there
 *  are none, just add a disabled "<none>" entry.
 */

#define SET_FILE    mem_fun(*this, &mainwnd::load_recent_file)

void
mainwnd::update_recent_files_menu ()
{
#ifdef NSM_SUPPORT
    if(m_nsm != NULL)
        return;
#endif

    if (m_menu_recent != nullptr)
    {
        // Nothing to do??
    }
    else
    {
        m_menu_recent = manage(new Gtk::Menu());
        MenuItem * menu_item = new MenuItem("Recent Files");
        menu_item->set_submenu(*m_menu_recent);
        m_menu_file->append(*menu_item);
    }

    if (m_mainperf->recent_file_count() > 0)
    {
        for (int i = 0; i < m_mainperf->recent_file_count(); ++i)
        {
            std::string filepath = m_mainperf->recent_file(i);     // shortened name
            MenuItem * menu_item = new MenuItem(filepath);
            menu_item->signal_activate().connect(sigc::bind(SET_FILE, i));
            m_menu_recent->append(*menu_item);
        }
    }
    else
    {
        MenuItem * menu_item = new MenuItem("<none>");
        menu_item->signal_activate().connect(sigc::bind(SET_FILE, (-1)));
        m_menu_recent->append(*menu_item);
    }
}

/**
 *  Looks up the desired recent MIDI file and opens it.  This function passes
 *  false as the shorten parameter of m_mainperf::recent_file().
 *
 * \param index
 *      Indicates which file in the list to open, ranging from 0 to the number
 *      of recent files minus 1.  If set to -1, then nothing is done.
 */

void
mainwnd::load_recent_file (int index)
{
    if (index >= 0 and index < m_mainperf->recent_file_count())
    {
        if (is_save())
        {
            std::string filepath = m_mainperf->recent_file(index, false);
            m_mainperf->set_playlist_mode(false);  // shut off playlist when new file is loaded outside of playlist
            open_file(filepath);
        }
    }
}

/* convert string to lower case letters */
void
mainwnd::toLower(basic_string<char>& s)
{
    for (basic_string<char>::iterator p = s.begin();
            p != s.end(); p++)
    {
        *p = tolower(*p);
    }
}

void
mainwnd::file_import_dialog()
{
    Gtk::FileChooserDialog dialog("Import MIDI file",
                                  Gtk::FILE_CHOOSER_ACTION_OPEN);
    dialog.set_transient_for(*this);

    auto filter_midi = Gtk::FileFilter::create();
    filter_midi->set_name("MIDI files");
    filter_midi->add_pattern("*.midi");
    filter_midi->add_pattern("*.MIDI");
    filter_midi->add_pattern("*.mid");
    filter_midi->add_pattern("*.MID");
    dialog.add_filter(filter_midi);

    auto filter_any = Gtk::FileFilter::create();
    filter_any->set_name("Any files");
    filter_any->add_pattern("*");
    dialog.add_filter(filter_any);

    dialog.set_current_folder(last_used_dir);

    ButtonBox *btnbox = dialog.get_action_area();
    HBox hbox( false, 2 );

    m_adjust_load_offset = Adjustment::create( 0, -(c_max_sets - 1),
                                   c_max_sets - 1, 1 );
    m_spinbutton_load_offset = manage( new SpinButton( m_adjust_load_offset ));
    m_spinbutton_load_offset->set_editable( true );
    m_spinbutton_load_offset->set_wrap( true );
    hbox.pack_end(*m_spinbutton_load_offset, false, false );
    hbox.pack_end(*(manage( new Label("Screen Set Offset"))), false, false, 4);

    btnbox->pack_start(hbox, false, false );

    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

    dialog.show_all_children();

    int result = dialog.run();

    //Handle the response:
    switch(result)
    {
    case(Gtk::RESPONSE_OK):
    {
        try
        {
            midifile f( dialog.get_filename() );

            /* True flag in f.parse() below is to indicate imported file. We can't use
             * the offset value since it could be the same as regular 'open' file of 0.
             * The flag is used to trigger the verification pop-up on changed tempo/time-sig
             * for imported files only */

            if(f.parse( m_mainperf, this, (int) m_adjust_load_offset->get_value(), true ))
                last_used_dir = dialog.get_filename().substr(0, dialog.get_filename().rfind("/") + 1);
            else return;
        }
        catch(...)
        {
            Gtk::MessageDialog errdialog(*this,
                                         "Error reading file.", false,
                                         Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
            errdialog.run();
        }

        global_is_modified = true; // OK

        m_main_wid->reset();
        m_entry_notes->set_text(*m_mainperf->get_screen_set_notepad(
                                    m_mainperf->get_screenset() ));

        break;
    }

    case(Gtk::RESPONSE_CANCEL):
        break;

    default:
        break;
    }
}

/*callback function*/
void mainwnd::file_exit()
{
#ifdef NSM_SUPPORT
    if (m_nsm && m_nsm_optional_gui)
    {
        global_nsm_gui = false;
    }
    else
    {
#endif  
        if (is_save())
        {
            if (global_is_running)
                stop_playing();

            hide();
        }
#ifdef NSM_SUPPORT
    }
#endif
}

bool
mainwnd::on_delete_event(GdkEventAny * /* a_e */)
{
#ifdef NSM_SUPPORT
    if (m_nsm && m_nsm_optional_gui)
    {
        // nsm : hide gui instead of closing
        global_nsm_gui = false;
        return true;
    }
#endif

    bool result = is_save();
    if (result && global_is_running)
        stop_playing();

    return !result;
}

void
mainwnd::about_dialog()
{
    Gtk::AboutDialog dialog;
    dialog.set_transient_for(*this);
    dialog.set_name(PACKAGE);
    dialog.set_version(VERSION);
    dialog.set_comments("Interactive MIDI Sequencer\n");

    dialog.set_copyright(
        "(C) 2015 - present Stazed\n"
        "(C) 2008 - 2010 Seq24team\n"
        "(C) 2002 - 2006 Rob C. Buse");
    dialog.set_website(WEBSITE);

    dialog.set_logo(Gdk::Pixbuf::create_from_xpm_data( seq32_xpm  ));

    Glib::ustring null_license;
    dialog.set_license(null_license);

    dialog.set_license_type(LICENSE_GPL_3_0);

    std::vector<Glib::ustring> list_authors;

    list_authors.push_back("Rob C. Buse <rcb@filter24.org>");
    list_authors.push_back("Ivan Hernandez <ihernandez@kiusys.com>");
    list_authors.push_back("Guido Scholz <guido.scholz@bayernline.de>");
    list_authors.push_back("Jaakko Sipari <jaakko.sipari@gmail.com>");
    list_authors.push_back("Peter Leigh <pete.leigh@gmail.com>");
    list_authors.push_back("Anthony Green <green@redhat.com>");
    list_authors.push_back("Daniel Ellis <mail@danellis.co.uk>");
    list_authors.push_back("Sebastien Alaiwan <sebastien.alaiwan@gmail.com>");
    list_authors.push_back("Kevin Meinert <kevin@subatomicglue.com>");
    list_authors.push_back("Andrea delle Canne <andreadellecanne@gmail.com>");
    list_authors.push_back("Stazed <stazed@mapson.com>");
    dialog.set_authors(list_authors);

    dialog.show();
    dialog.run();
}

void
mainwnd::popup_menu(Menu *a_menu)
{
    a_menu->popup(0,0);
}

void
mainwnd::apply_song_transpose()
{
    if(global_is_running)
        return;

    if(m_mainperf->get_master_midi_bus()->get_transpose() != 0)
    {
        m_mainperf->apply_song_transpose();
        m_perf_edit->set_xpose(0);
    }
}

void
mainwnd::set_song_mute(mute_op op)
{
    m_mainperf->set_song_mute(op);
    global_is_modified = true;
}

void
mainwnd::tap ()
{
    double bpm = update_bpm();
    set_tap_button(m_current_beats);
    if (m_current_beats > 1)                    /* first one is useless */
        m_adjust_bpm->set_value(double(bpm));
}

void
mainwnd::set_tap_button (int beats)
{
    Gtk::Label * tapptr(dynamic_cast<Gtk::Label *>(m_button_tap->get_child()));
    if (tapptr != nullptr)
    {
        char temp[8];
        snprintf(temp, sizeof(temp), "%d", beats);
        tapptr->set_text(temp);
    }
}

double
mainwnd::update_bpm ()
{
    double bpm = 0.0;
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    long ms = long(spec.tv_sec) * 1000;     /* seconds to milliseconds      */
    ms += round(spec.tv_nsec * 1.0e-6);     /* nanoseconds to milliseconds  */
    if (m_current_beats == 0)
    {
        m_base_time_ms = ms;
        m_last_time_ms = 0;
    }
    else if (m_current_beats >= 1)
    {
        int diffms = ms - m_base_time_ms;
        bpm = m_current_beats * 60000.0 / diffms;
        m_last_time_ms = ms;
    }
    ++m_current_beats;
    return bpm;
}

void
mainwnd::adj_callback_ss( )
{
    m_mainperf->set_screenset( (int) m_adjust_ss->get_value());
    m_main_wid->set_screenset( m_mainperf->get_screenset());
    m_entry_notes->set_text(*m_mainperf->get_screen_set_notepad(
                                m_mainperf->get_screenset()));
}

void
mainwnd::adj_callback_bpm( )
{
    if(m_mainperf->get_bpm() !=  m_adjust_bpm->get_value())
    {
        //printf("adj_callback_bpm - mainwnd\n");

        /* update_start_BPM() will set perform midibus. */
        m_perf_edit->update_start_BPM(m_adjust_bpm->get_value());
        global_is_modified = true;
    }
}

bool
mainwnd::on_key_release_event(GdkEventKey* a_ev)
{
    if ( a_ev->keyval == m_mainperf->m_key_replace )
        m_mainperf->unset_sequence_control_status( c_status_replace );

    if (a_ev->keyval == m_mainperf->m_key_queue )
        m_mainperf->unset_sequence_control_status( c_status_queue );

    if ( a_ev->keyval == m_mainperf->m_key_snapshot_1 ||
            a_ev->keyval == m_mainperf->m_key_snapshot_2 )
        m_mainperf->unset_sequence_control_status( c_status_snapshot );

    if ( a_ev->keyval == m_mainperf->m_key_group_learn )
    {
        m_mainperf->unset_mode_group_learn();
    }

    return false;
}

void
mainwnd::edit_callback_notepad( )
{
    string text = m_entry_notes->get_text();
    m_mainperf->set_screen_set_notepad( m_mainperf->get_screenset(),
                                        &text );
    global_is_modified = true; // OK
}

bool
mainwnd::on_key_press_event(GdkEventKey* a_ev)
{
    if ( a_ev->type == GDK_KEY_PRESS )
    {
        if(!global_is_running && !m_menu_mode) // only allow menu when not running & menu button not pressed
        {
            if ( a_ev->state & GDK_MOD1_MASK ) // alt key
            {
                if ((a_ev->keyval == GDK_KEY_b || a_ev->keyval == GDK_KEY_B) || // bpm
                        (a_ev->keyval == GDK_KEY_f || a_ev->keyval == GDK_KEY_F) || // file
                        (a_ev->keyval == GDK_KEY_h || a_ev->keyval == GDK_KEY_H) || // help
                        (a_ev->keyval == GDK_KEY_n || a_ev->keyval == GDK_KEY_N) || // name
                        (a_ev->keyval == GDK_KEY_s || a_ev->keyval == GDK_KEY_S) || // set
                        (a_ev->keyval == GDK_KEY_v || a_ev->keyval == GDK_KEY_V))   // view
                {
                    return Gtk::Window::on_key_press_event(a_ev); // return = don't do anything else
                }
            }

            if ( a_ev->state & GDK_CONTROL_MASK ) // ctrl key
            {
                if ((a_ev->keyval == GDK_KEY_e || a_ev->keyval == GDK_KEY_E) || // song editor
                        (a_ev->keyval == GDK_KEY_n || a_ev->keyval == GDK_KEY_N) || // new file
                        (a_ev->keyval == GDK_KEY_o || a_ev->keyval == GDK_KEY_O) || // open file
                        (a_ev->keyval == GDK_KEY_q || a_ev->keyval == GDK_KEY_Q) || // quit
                        (a_ev->keyval == GDK_KEY_s || a_ev->keyval == GDK_KEY_S) )  // save
                {
                    return Gtk::Window::on_key_press_event(a_ev); // return =  don't do anything else
                }
            }
        }
        // screen name - this must go before the shift_mask & tab below
        // or we get duplicate entries on caps and reverse tab
        if(get_focus()->get_name() == "Screen Name")      // if we are on the screen name
            return Gtk::Window::on_key_press_event(a_ev); // return = don't do anything else

        if(get_focus()->get_name() == "BPM Edit")        // if we are on the BPM spin button - allow editing
           return Gtk::Window::on_key_press_event(a_ev); // return = don't do anything else

        // stop, start, song, jack, menu, edit ,L
        if(a_ev->keyval == GDK_KEY_Tab || a_ev->keyval == GDK_KEY_Return) // use it for buttons only
            return Gtk::Window::on_key_press_event(a_ev); // return = don't do anything else

        // bpm, screen set
        if(a_ev->keyval == GDK_KEY_Up || a_ev->keyval == GDK_KEY_Down) // use it for spin buttons only
            return Gtk::Window::on_key_press_event(a_ev); // return = don't do anything else

        if (a_ev->state & GDK_SHIFT_MASK ) // use it for reverse tab
            Gtk::Window::on_key_press_event(a_ev); // no return = let others use it for caps (mute groups)

        // control and modifier key combinations matching

        if ( global_print_keys )
        {
            printf( "key_press[%d]\n", a_ev->keyval );
            fflush( stdout );
        }

        if ( a_ev->keyval == m_mainperf->m_key_bpm_dn )
        {
            /* update_start_BPM() will set tempo markers, set perform midibus
             * and trigger mainwnd timeout to update the mainwnd bpm spinner */
            m_perf_edit->update_start_BPM(m_mainperf->get_bpm() -1);
        }

        if ( a_ev->keyval ==  m_mainperf->m_key_bpm_up )
        {
            /* update_start_BPM() will set tempo markers, set perform midibus
             * and trigger mainwnd timeout to update the mainwnd bpm spinner */
            m_perf_edit->update_start_BPM(m_mainperf->get_bpm() +1);
        }

        if (a_ev->keyval  == m_mainperf->m_key_tap_bpm )
        {
            tap();
        }

        if ( a_ev->keyval == m_mainperf->m_key_replace )
        {
            m_mainperf->set_sequence_control_status( c_status_replace );
        }

        if ((a_ev->keyval ==  m_mainperf->m_key_queue )
                || (a_ev->keyval == m_mainperf->m_key_keep_queue ))
        {
            m_mainperf->set_sequence_control_status( c_status_queue );
        }

        if ( a_ev->keyval == m_mainperf->m_key_snapshot_1 ||
                a_ev->keyval == m_mainperf->m_key_snapshot_2 )
        {
            m_mainperf->set_sequence_control_status( c_status_snapshot );
        }

        if ( a_ev->keyval == m_mainperf->m_key_screenset_dn )
        {
            m_mainperf->set_screenset(  m_mainperf->get_screenset() - 1 );
            m_main_wid->set_screenset(  m_mainperf->get_screenset() );
            m_adjust_ss->set_value( m_mainperf->get_screenset()  );
            m_entry_notes->set_text(*m_mainperf->get_screen_set_notepad(
                                        m_mainperf->get_screenset()));
        }

        if ( a_ev->keyval == m_mainperf->m_key_screenset_up )
        {
            m_mainperf->set_screenset(  m_mainperf->get_screenset() + 1 );
            m_main_wid->set_screenset(  m_mainperf->get_screenset() );
            m_adjust_ss->set_value( m_mainperf->get_screenset()  );
            m_entry_notes->set_text(*m_mainperf->get_screen_set_notepad(
                                        m_mainperf->get_screenset()));
        }

        if ( a_ev->keyval == m_mainperf->m_key_set_playing_screenset )
        {
            m_mainperf->set_playing_screenset();
        }

        if ( a_ev->keyval == m_mainperf->m_key_group_on )
        {
            m_mainperf->set_mode_group_mute();
        }

        if ( a_ev->keyval == m_mainperf->m_key_group_off )
        {
            m_mainperf->unset_mode_group_mute();
        }

        if ( a_ev->keyval == m_mainperf->m_key_group_learn )
        {
            m_mainperf->set_mode_group_learn();
        }

        // activate mute group key
        if (m_mainperf->get_key_groups()->count( a_ev->keyval ) != 0 )
        {
            m_mainperf->select_and_mute_group(
                m_mainperf->lookup_keygroup_group(a_ev->keyval));
        }

        // mute group learn
        if (m_mainperf->is_learn_mode() &&
                a_ev->keyval != m_mainperf->m_key_group_learn)
        {
            if (a_ev->state & GDK_SHIFT_MASK )
            {
                if( m_mainperf->get_key_groups()->count( a_ev->keyval ) != 0 )
                {
                    std::ostringstream os;
                    os << "Key \""
                       << gdk_keyval_name(a_ev->keyval)
                       << "\" (code = "
                       << a_ev->keyval
                       << ") successfully mapped.";

                    Gtk::MessageDialog dialog(*this,
                                              "MIDI mute group learn success", false,
                                              Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
                    dialog.set_secondary_text(os.str(), false);
                    dialog.run();

                    // we miss the keyup msg for learn, force set it off
                    m_mainperf->unset_mode_group_learn();
                    global_is_modified = true;
                }
                else
                {
                    std::ostringstream os;
                    os << "Key \""
                       << gdk_keyval_name(a_ev->keyval)
                       << "\" (code = "
                       << a_ev->keyval
                       << ") is not one of the configured mute-group keys.\n"
                       << "To change this see File/Options menu or .seq32rc";

                    Gtk::MessageDialog dialog(*this,
                                              "MIDI mute group learn failed", false,
                                              Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);

                    dialog.set_secondary_text(os.str(), false);
                    dialog.run();
                    // we miss the keyup msg for learn, force set it
                    m_mainperf->unset_mode_group_learn();
                }
            }
        }

        if ( a_ev->keyval ==  m_mainperf->m_key_song )
        {
            toggle_song_mode();
            return true;
        }

        if ( a_ev->keyval ==  m_mainperf->m_key_menu )
        {
            toggle_menu_mode();
            return true;
        }

#ifdef JACK_SUPPORT
        if ( a_ev->keyval ==  m_mainperf->m_key_jack )
        {
            m_perf_edit->toggle_jack();
            return true;
        }
#endif // JACK_SUPPORT

        // the start/end key may be the same key (i.e. SPACE)
        // allow toggling when the same key is mapped to both
        // triggers (i.e. SPACEBAR)
        bool dont_toggle = m_mainperf->m_key_start
                           != m_mainperf->m_key_stop;

        if ( a_ev->keyval == m_mainperf->m_key_start
                && (dont_toggle || !global_is_running))
        {
            start_playing();
        }
        else if ( a_ev->keyval == m_mainperf->m_key_stop
                  && (dont_toggle || global_is_running))
        {
            stop_playing();
        }

        /* toggle sequence mute/unmute using keyboard keys... */
        if (m_mainperf->get_key_events()->count( a_ev->keyval) != 0)
        {
            sequence_key(m_mainperf->lookup_keyevent_seq( a_ev->keyval));
        }

        if(m_mainperf->get_playlist_mode())
        {
            if ( a_ev->keyval == m_mainperf->m_key_playlist_prev )
            {
            	playlist_jump(PLAYLIST_PREVIOUS);
                return true;
            }
            if ( a_ev->keyval == m_mainperf->m_key_playlist_next )
            {
            	playlist_jump(PLAYLIST_NEXT);
                return true;
            }
        }
    }

    return false;
}

void
mainwnd::sequence_key( int a_seq )
{
    int offset = m_mainperf->get_screenset() * c_mainwnd_rows * c_mainwnd_cols;

    if ( m_mainperf->is_active( a_seq + offset ) )
    {
        m_mainperf->sequence_playing_toggle( a_seq + offset );
    }
}

void
mainwnd::update_window_title()
{
    std::string title;

    if(m_mainperf->get_playlist_mode())
    {
    	char num[20];
    	sprintf(num,"%02d",m_mainperf->get_playlist_index() +1);
    	title =
    		( PACKAGE )
			+ string(" - Playlist, Song ")
			+ num
			+ string(" - [")
            + Glib::filename_to_utf8(global_filename)
            + string( "]" );
    }
    else
    {

        if (global_filename == "")
            title = ( PACKAGE ) + string( " - [unnamed]" );
        else
            title =
                ( PACKAGE )
                + string( " - [" )
                + Glib::filename_to_utf8(global_filename)
                + string( "]" );

    }

    set_title ( title.c_str());
    
    if(m_perf_edit != nullptr)
    {
        if(m_mainperf->get_playlist_mode())
            m_perf_edit->set_title( title.c_str());
        else
            m_perf_edit->set_title("seq32 - Song Editor");
    }  
}

/* This changes the main window logo .xpm when in playlist mode and back */
void
mainwnd::update_window_xpm()
{
    tophbox->remove(*m_image_seq32);
    
    if(m_mainperf->get_playlist_mode())
    {
        m_image_seq32 = manage(new Image( Gdk::Pixbuf::create_from_xpm_data(seq32_xpm_playlist)));
    }
    else
    {
        m_image_seq32 = manage(new Image( Gdk::Pixbuf::create_from_xpm_data(seq32_xpm)));
    }

    if(m_image_seq32 != NULL)
    {
        tophbox->pack_start(*m_image_seq32, false, false);
        tophbox->reorder_child(*m_image_seq32, 0);
        m_image_seq32->show();
    }
}

/* update tempo class thru perfedit for file loading of tempo map */
void
mainwnd::load_tempo_list()
{
    m_perf_edit->load_tempo_list();
}

void
mainwnd::set_bp_measure(int bp_measure)
{
    m_perf_edit->set_bp_measure(bp_measure);
}

void
mainwnd::set_bw(int bw)
{
    m_perf_edit->set_bw(bw);
}

void
mainwnd::update_start_BPM(double bpm)
{
    m_perf_edit->update_start_BPM(bpm);
}

#ifdef NSM_SUPPORT
void
mainwnd::close_all_windows()
{
    m_closing_windows = true;

    for(unsigned i = 0; i < m_vector_windows.size(); ++i)
    {
        m_vector_windows[i]->close();
    }

    m_vector_windows.clear();
    
    if ( m_options != NULL )
        m_options->hide();

    if (m_perf_edit != NULL)
    {
        m_perf_edit->hide_tempo_popup();
        m_perf_edit->hide();
    }

    m_closing_windows = false;
}

void
mainwnd::set_window_pointer(Gtk::Window * a_win)
{
    m_vector_windows.push_back(a_win);
}

void
mainwnd::remove_window_pointer(Gtk::Window * a_win)
{
    if (m_closing_windows)
        return;
    
    for(unsigned i = 0; i < m_vector_windows.size(); ++i)
    {
        if (m_vector_windows[i] == a_win)
        {
            m_vector_windows.erase(m_vector_windows.begin() + i);
        }
    }
}

void
mainwnd::set_nsm_client(nsm_client_t *nsm, bool optional_gui)
{
    m_nsm = nsm;
    m_nsm_optional_gui = optional_gui;
    if(m_menu_file != nullptr)
    {
        m_file_menu_items[1].set_sensitive(false);  // New
        m_file_menu_items[2].set_sensitive(false);  // Open
        /* We allow loading of play list - but we prevent saving when using it */
        // m_file_menu_items[3].set_sensitive(false);  // Open playlist
        m_file_menu_items[5].set_sensitive(false);  // Save As

        m_menu_recent->set_sensitive(false);        // Recent files submenu
        m_file_menu_items[7].set_label("Hide");     // Exit
    }
}
#endif
