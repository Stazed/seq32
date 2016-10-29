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


    /*sjh: setlist stuff */
    //TRYING:
    Image 						*m_s24_pic;
    //NOT WORKING:
    Glib::RefPtr<Gdk::GC>       m_gc;
    Glib::RefPtr<Gdk::Window>   m_window;
    Gdk::Color    				m_black, m_white, m_grey;
    Glib::RefPtr<Gdk::Pixmap>   m_pixmap;
    /*TODO: This was an attempt to indicate that we are in setlist mode
     * by changing the main image. Doesn't work though - need to
     * rebuild the hbox and add in the image. I think the method is correct - just
     * needs refining.
     */
    void set_wsetlist_mode(bool mode);
    void setlist_jump(int jmp);

/**************************************************/

    perform  *m_mainperf;
    bool      m_menu_mode;
    static int m_sigpipe[2];

#if GTK_MINOR_VERSION < 12
    Tooltips *m_tooltips;
#endif
    MenuBar  *m_menubar;
    Menu     *m_menu_file;
    Menu     *m_menu_edit;
    Menu     *m_menu_help;

    mainwid  *m_main_wid;
    maintime *m_main_time;

    perfedit *m_perf_edit;
    options *m_options;

    Gdk::Cursor   m_main_cursor;

    Button      *m_button_learn;

    Button      *m_button_stop;
    Button      *m_button_play;
    Button      *m_button_perfedit;

    ToggleButton *m_button_mode;
    ToggleButton *m_button_menu;

    SpinButton  *m_spinbutton_bpm;
    Adjustment  *m_adjust_bpm;

    SpinButton  *m_spinbutton_ss;
    Adjustment  *m_adjust_ss;

    SpinButton  *m_spinbutton_load_offset;
    Adjustment  *m_adjust_load_offset;

    Entry       *m_entry_notes;

    sigc::connection   m_timeout_connect;

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
    void update_window_title();
    void toLower(basic_string<char>&);
    void file_new();
    void file_open();
    void file_open_setlist();//sjh mod
    void file_save();
    void file_save_as( int type = c_seq32_midi );
    void file_exit();
    void new_open_error_dialog();
    void new_file();
    bool save_file();
    void export_midi(const Glib::ustring& fn);
    void choose_file(bool setlist_mode = false);
    int query_save_changes();
    bool is_save();
    static void handle_signal(int sig);
    bool install_signal_handlers();
    bool signal_action(Glib::IOCondition condition);

    void popup_menu (Menu * a_menu);
    void apply_song_transpose ();
    void set_song_mute(mute_op op);

public:

    mainwnd(perform *a_p);
    ~mainwnd();

    bool open_file(const Glib::ustring&, bool setlist_mode = false);
    bool on_delete_event(GdkEventAny *a_e);
    bool on_key_press_event(GdkEventKey* a_ev);
    bool on_key_release_event(GdkEventKey* a_ev);

};
