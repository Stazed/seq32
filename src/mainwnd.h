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


#include "mainwid.h"
#include "perform.h"
#include "sequence.h"
#include "event.h"
#include "options.h"
#include "maintime.h"
#include "perfedit.h"
#include "options.h"



#ifndef SEQ24_MAINWINDOW
#define SEQ24_MAINWINDOW

#include <gtkmm/button.h>
#include <gtkmm/window.h>
#include <gtkmm/accelgroup.h>
#include <gtkmm/box.h>
#include <gtkmm/main.h>
#include <gtkmm/menu.h>
#include <gtkmm/menubar.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/window.h>
#include <gtkmm/table.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/label.h>
#include <gtkmm/viewport.h> 
#include <gtkmm/fileselection.h>
#include <gtkmm/dialog.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/textview.h>
#include <gtkmm/arrow.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/progressbar.h>
#include <gtkmm/dialog.h>
#include <gtkmm/label.h>
#include <gdkmm/cursor.h>
#include <gtkmm/image.h>


#include "globals.h"

#include <map>


using namespace Gtk;


using namespace Menu_Helpers;


class mainwnd : public Gtk::Window
{
   
 private:

    bool      m_quit;
    
    MenuBar  *m_menubar;
    Menu     *m_menu_file;
    Menu     *m_menu_control;
    Menu     *m_menu_help;

    perform  *m_mainperf;

    mainwid  *m_main_wid;
    maintime *m_main_time;

    perfedit *m_perf_edit;
    options *m_options;

    Gdk::Cursor   m_main_cursor;
    
    Button      *m_button_stop;
    Button      *m_button_play;
    Button      *m_button_test;

    Button      *m_button_perfedit;

    SpinButton  *m_spinbutton_bpm;
    Adjustment  *m_adjust_bpm;

    SpinButton  *m_spinbutton_ss;
    Adjustment  *m_adjust_ss;

    SpinButton  *m_spinbutton_load_offset;
    Adjustment  *m_adjust_load_offset;

    Entry       *m_entry_notes;

    SigC::Connection   m_timeout_connect;

    void file_new_dialog( void );
    void file_save_dialog( void );
    void file_saveas_dialog( void );
    void file_open_dialog( void );
    void file_import_dialog( void );
    void file_exit_dialog( void );

    void options_dialog( void );
    void about_dialog( void );


    void adj_callback_ss( );
    void adj_callback_bpm( );

    void edit_callback_notepad( );

    bool timer_callback( );

    void start_playing();
    void stop_playing();
    void test();

    void open_performance_edit( );

    void sequence_key( int a_seq );

    void set_window_title_filename( std::string a_file );

 public:

    mainwnd(perform *a_p);
    ~mainwnd();

    bool on_delete_event(GdkEventAny *a_e);
    bool on_key_press_event(GdkEventKey* a_ev);
    bool on_key_release_event(GdkEventKey* a_ev);


};


#endif
