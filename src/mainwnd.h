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
#include "perform.h"
#include "sequence.h"
#include "event.h"
#include "maintime.h"
#include "perfedit.h"
#include "options.h"

#pragma once

#ifdef NSM_SUPPORT
#include "nsm.h"
#endif

#include <map>
#include <gtkmm.h>
#include <string>

#include "globals.h"

using namespace Gtk;

using namespace Menu_Helpers;

class mainwnd : public Gtk::Window, public performcallback
{
    /* notification handler for learn mode toggle */
    virtual void on_grouplearnchange(bool state);

private:

    perform  *m_mainperf;
    Glib::RefPtr<Gtk::Application> m_app;

    bool      m_menu_mode;
    static int m_sigpipe[2];

    MenuBar  *m_menubar;
    Menu     *m_menu_file;
    Menu     *m_menu_recent;        /* File/Recent menu popup.    */
    Menu     *m_menu_edit;
    Menu     *m_menu_help;

    std::vector<MenuItem> m_file_menu_items;
    std::vector<MenuItem> m_edit_menu_items;
    MenuItem m_help_menu_item;
    MenuItem m_help_submenu_item;
    Glib::RefPtr<Gtk::AccelGroup> m_accelgroup;
    SeparatorMenuItem   m_menu_separator1;
    SeparatorMenuItem   m_menu_separator2;
    SeparatorMenuItem   m_menu_separator3;
    SeparatorMenuItem   m_menu_separator4;
    SeparatorMenuItem   m_menu_separator5;
    
    Gtk::HBox *tophbox;
    Gtk::Image * m_image_seq32;     /* Image for Mainwindow logo.   */

    mainwid  *m_main_wid;
    maintime *m_main_time;

    perfedit *m_perf_edit;
    options *m_options;

    Button      *m_button_learn;

    Button      *m_button_stop;
    Button      *m_button_play;
    Button      *m_button_perfedit;

    ToggleButton *m_button_mode;
    ToggleButton *m_button_menu;

    SpinButton  *m_spinbutton_bpm;
    Glib::RefPtr<Adjustment> m_adjust_bpm;

    Button      *m_button_tap;

    SpinButton  *m_spinbutton_ss;
    Glib::RefPtr<Adjustment> m_adjust_ss;

    SpinButton  *m_spinbutton_load_offset;
    Glib::RefPtr<Adjustment> m_adjust_load_offset;

    Entry       *m_entry_notes;

    sigc::connection   m_timeout_connect;
    
    /* tap button - From sequencer64 */
    int m_current_beats; // value is displayed in the button.
    long m_base_time_ms; // Indicates the first time the tap button was ... tapped.
    long m_last_time_ms; // Indicates the last time the tap button was tapped.

    void set_song_mode();
    void toggle_song_mode();
    void set_menu_mode();
    void toggle_menu_mode();

    void file_import_dialog();
    void options_dialog();
    void about_dialog();

    void adj_callback_ss( );
    void adj_callback_bpm( );
    void edit_callback_notepad( );
    bool timer_callback( );

    void start_playing();
    void stop_playing();
    void learn_toggle();
    void open_performance_edit();
    void sequence_key( int a_seq );
public:
    void update_window_title();
private:
    void update_window_xpm();
    void toLower(basic_string<char>&);
    void file_new();
    void file_open();
    void file_open_playlist();

#ifdef NSM_SUPPORT
    bool m_nsm_optional_gui;
    bool m_nsm_visible;
    bool m_dirty_flag;
    nsm_client_t *m_nsm;
public:
    void set_nsm_client(nsm_client_t *nsm, bool optional_gui);
private:
#endif

public:
    void file_save();
private:
    void file_save_as( file_type_e type, int a_seq );
    void file_exit();
    void new_open_error_dialog();
    void new_file();
    bool save_file();
    void export_midi(const Glib::ustring& fn, file_type_e type, int a_seq);
    void choose_file(bool playlist_mode = false);
    int query_save_changes();
    bool is_save();
    void update_recent_files_menu ();
    void load_recent_file (int index);
    
    static void handle_signal(int sig);
    bool install_signal_handlers();
    bool signal_action(Glib::IOCondition condition);

    void popup_menu (Menu * a_menu);
    void apply_song_transpose ();
    void set_song_mute(mute_op op);

    /* From  sequencer64 tap button */
    void tap ();
    void set_tap_button (int beats);
    double update_bpm ();

public:

    mainwnd(perform *a_p, Glib::RefPtr<Gtk::Application> app);
    ~mainwnd();

    bool open_file(const Glib::ustring&);
    void export_seq_track_trigger(file_type_e type, int a_seq);
    bool on_delete_event(GdkEventAny *a_e);
    bool on_key_press_event(GdkEventKey* a_ev);
    bool on_key_release_event(GdkEventKey* a_ev);
    bool playlist_jump(int jmp, bool a_verify = false);
    bool verify_playlist_dialog();
    void playlist_verify();
    void load_tempo_list();
    void set_bp_measure(int bp_measure);
    void set_bw(int bw);
    void update_start_BPM(double bpm);
    
};
