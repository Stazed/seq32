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
#include "mainwnd.h"
#include "perform.h"
#include "midifile.h"
#include "perfedit.h"

#include "play2.xpm"
#include "stop.xpm"
#include "perfedit.xpm"
#include "seq24.xpm"
 
mainwnd::mainwnd(perform *a_p)
{


    /* set the performance */
    m_mainperf = a_p;

    /* main window */
    set_window_title_filename( global_filename );

    m_main_wid = manage( new mainwid(  m_mainperf ));
    m_main_time = manage( new maintime( ));

    m_menubar   = manage(new MenuBar());
    m_menu_file = manage(new Menu());
    m_menu_control = manage( new Menu());
    m_menu_help    = manage( new Menu());
    
    /* fill with items */
    m_menu_file->items().push_back(MenuElem("New", mem_fun(*this,&mainwnd::file_new_dialog)));
    m_menu_file->items().push_back(SeparatorElem());
    m_menu_file->items().push_back(MenuElem("Open...",   mem_fun(*this,&mainwnd::file_open_dialog)));
    m_menu_file->items().push_back(MenuElem("Import...", mem_fun(*this,&mainwnd::file_import_dialog)));
    m_menu_file->items().push_back(SeparatorElem());
    m_menu_file->items().push_back(MenuElem("Save", mem_fun(*this,&mainwnd::file_save_dialog)));
    m_menu_file->items().push_back(MenuElem("Save As...", mem_fun(*this,&mainwnd::file_saveas_dialog)));
    m_menu_file->items().push_back(SeparatorElem());
    m_menu_file->items().push_back(MenuElem("Options...", mem_fun(*this,&mainwnd::options_dialog)));
    m_menu_file->items().push_back(SeparatorElem());
    m_menu_file->items().push_back(MenuElem("Exit", mem_fun(*this,&mainwnd::file_exit_dialog)));

    m_menu_help->items().push_back(MenuElem("About", mem_fun(*this,&mainwnd::about_dialog)));
 
    m_menubar->items().push_front(MenuElem("File", *m_menu_file));
    m_menubar->items().push_back(MenuElem("Help", *m_menu_help));

    HBox *hbox = manage( new HBox( false, 2 ) );

    m_button_stop = manage( new Button( ));
    m_button_stop->add( *manage( new Image(Gdk::Pixbuf::create_from_xpm_data( stop_xpm ))));
    m_button_stop->signal_clicked().connect( mem_fun(*this,&mainwnd::stop_playing));
    hbox->pack_start(*m_button_stop, false, false);

    m_button_play = manage( new Button() );
    m_button_play->add( *manage( new Image(Gdk::Pixbuf::create_from_xpm_data( play2_xpm  ))));
    m_button_play->signal_clicked().connect(  mem_fun( *this, &mainwnd::start_playing));
    hbox->pack_start(*m_button_play, false, false);

    //m_button_test = manage( new Button("test") );
    //m_button_test->signal_clicked().connect(  mem_fun( *this, &mainwnd::test));
    //hbox->pack_start(*m_button_test, false, false);

    m_button_perfedit = manage( new Button(  ));
    m_button_perfedit->add( *manage( new Image(Gdk::Pixbuf::create_from_xpm_data( perfedit_xpm  ))));
    m_button_perfedit->signal_clicked().connect( mem_fun( *this, &mainwnd::open_performance_edit ));
    hbox->pack_end(*m_button_perfedit, false, false, 4);

    m_adjust_bpm = manage( new Adjustment(  m_mainperf->get_bpm(), 20, 500, 1 ));
    m_spinbutton_bpm = manage( new SpinButton( *m_adjust_bpm ));
    m_spinbutton_bpm->set_editable( false );
    hbox->pack_start(*(manage( new Label( "  bpm " ))), false, false, 4);
    hbox->pack_start(*m_spinbutton_bpm, false, false );
  
    m_adjust_ss = manage( new Adjustment( 0, 0, c_max_sets - 1, 1 ));
    m_spinbutton_ss = manage( new SpinButton( *m_adjust_ss ));
    m_spinbutton_ss->set_editable( false );
    m_spinbutton_ss->set_wrap( true );
    hbox->pack_end(*m_spinbutton_ss, false, false );
    hbox->pack_end(*(manage( new Label( "  set " ))), false, false, 4);

    m_adjust_bpm->signal_value_changed().connect( mem_fun(*this,&mainwnd::adj_callback_bpm ));
    m_adjust_ss->signal_value_changed().connect( mem_fun(*this,&mainwnd::adj_callback_ss ));
 
    m_entry_notes = manage( new Entry());
    m_entry_notes->signal_changed().connect( mem_fun(*this,&mainwnd::edit_callback_notepad ));

    hbox->pack_start( *m_entry_notes, true, true );

    /* 2nd hbox */
    HBox *hbox2 = manage( new HBox( false, 0 ) );
    hbox2->pack_start( *manage(  new Image(Gdk::Pixbuf::create_from_xpm_data( seq24_xpm ))), false, false );
    hbox2->pack_end( *m_main_time, false, false );

    /* set up a vbox, put the menu in it, and add it */
    VBox *vbox = new VBox();
    vbox->set_border_width( 10 );
    vbox->pack_start(*hbox2, false, false );
    vbox->pack_start(*m_main_wid, true, true, 10 );
    vbox->pack_start(*hbox, false, false ); 
 

    VBox *ovbox = new VBox();
 
    ovbox->pack_start(*m_menubar, false, false );
    ovbox->pack_start( *vbox );

    /* add box */
    this->add (*ovbox);
  
    /* show everything */
    show_all();

    add_events( Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK );

    m_entry_notes->set_text( * m_mainperf->get_screen_set_notepad(  m_mainperf->get_screenset() )); 

    m_timeout_connect = Glib::signal_timeout().connect(mem_fun(*this,&mainwnd::timer_callback), 25);
    
    m_quit = false;

    m_perf_edit = new perfedit( m_mainperf );
    m_options = NULL;
  
}

 
mainwnd::~mainwnd()
{
    if ( m_perf_edit != NULL )
        delete m_perf_edit;
    if ( m_options != NULL )
        delete m_options;
}


// This is the GTK timer callback, used to draw our current time and bpm
// on the main window
bool
mainwnd::timer_callback(  )
{
    long ticks = m_mainperf->get_tick();
	
    m_main_time->idle_progress( ticks );
    m_main_wid->update_markers( ticks );

    if ( m_adjust_bpm->get_value() != m_mainperf->get_bpm()){
        m_adjust_bpm->set_value( m_mainperf->get_bpm());
    }

    if ( m_adjust_ss->get_value() !=  m_mainperf->get_screenset() )
    {
        m_main_wid->set_screenset(  m_mainperf->get_screenset() );  
        m_adjust_ss->set_value( m_mainperf->get_screenset()  );	
        m_entry_notes->set_text( * m_mainperf->get_screen_set_notepad(m_mainperf->get_screenset()  )); 
    }

    return true;

}


void 
mainwnd::open_performance_edit( void )
{
    m_perf_edit->init_before_show();
    m_perf_edit->show_all();
}


void 
mainwnd::options_dialog( void )
{
    if ( m_options != NULL )
        delete m_options;
    m_options = new options( *this,  m_mainperf ); 
    m_options->show_all(); 
}

void 
mainwnd::start_playing( void )
{
    m_mainperf->position_jack( false );  
    m_mainperf->start( false );
    m_mainperf->start_jack( );
}

void 
mainwnd::stop_playing( void )
{
    m_mainperf->stop_jack();
    m_mainperf->stop();
    m_main_wid->update_sequences_on_window();
}


void
mainwnd::file_new_dialog( void )
{


    Gtk::MessageDialog dialog(*this,
                              "Clear Sequences ?",
                              false,
                              Gtk::MESSAGE_QUESTION,
                              (Gtk::ButtonsType)(BUTTONS_OK_CANCEL),
                              true );
    
    int result = dialog.run();

    //Handle the response:
    switch(result)
    {
        case(Gtk::RESPONSE_OK):
        {
            m_mainperf->clear_all();
            
            m_main_wid->reset();
            m_entry_notes->set_text( * m_mainperf->get_screen_set_notepad( m_mainperf->get_screenset() ));
            
            global_filename = "untitled";
            set_window_title_filename( global_filename );
            
            break;
        }
        case(Gtk::RESPONSE_CANCEL):
        {   
            break;
        }
        default:
        { 
            break;
        }
    }
}


void 
mainwnd::file_save_dialog( void )
{
    bool result = false;
    
    midifile f( global_filename.c_str() );
    result = f.write( m_mainperf );

    if ( !result ){
        
        Gtk::MessageDialog errdialog(*this,
                                     "Error writing file.",
                                     false,
                                     Gtk::MESSAGE_ERROR,
                                     (Gtk::ButtonsType)(Gtk::BUTTONS_OK),
                                     true);
        errdialog.run();
    }
    
}

void 
mainwnd::file_saveas_dialog( void )
{

    FileSelection dialog( "Save As..." );
    int result = dialog.run();
    
    //Handle the response:
    switch(result)
    {
        case(Gtk::RESPONSE_OK):
        {
            bool result = false;
            
            midifile f( dialog.get_filename() );
            result = f.write( m_mainperf );

            if ( !result ){
                
                Gtk::MessageDialog errdialog(*this,
                                              false,
                                             "Error writing file.",
                                             Gtk::MESSAGE_ERROR,
                                             (Gtk::ButtonsType)(Gtk::BUTTONS_OK),
                                             true );
                errdialog.run();
            }
            
            global_filename = std::string( dialog.get_filename());
            set_window_title_filename( global_filename );
            
            break;
        }
        case(Gtk::RESPONSE_CANCEL):
        {
            break;
        }
        default:
        {
            break;
        }
    }


}

void 
mainwnd::file_open_dialog( void )
{
    FileSelection dialog( "Open..." );
    int result = dialog.run();
    
    //Handle the response:
    switch(result)
    {
        case(Gtk::RESPONSE_OK):
        {
            bool result = false;
            
            m_mainperf->clear_all();
            
            midifile f( dialog.get_filename() );
            result = f.parse( m_mainperf, 0 );

            if ( !result ){
                
                Gtk::MessageDialog errdialog(*this,
                                             "Error reading file.", false,
                                             Gtk::MESSAGE_ERROR,
                                             (Gtk::ButtonsType)(Gtk::BUTTONS_OK),
                                             true);
                errdialog.run();
            }

            global_filename = std::string(dialog.get_filename());
            set_window_title_filename( global_filename );
            
            m_main_wid->reset();
            m_entry_notes->set_text( * m_mainperf->get_screen_set_notepad( m_mainperf->get_screenset() )); 
            m_adjust_bpm->set_value( m_mainperf->get_bpm() );

            
            break;
        }
        case(Gtk::RESPONSE_CANCEL):
        {
            break;
        }
        default:
        {
            break;
        }
    }
  
}

void 
mainwnd::file_import_dialog( void )
{

   FileSelection dialog( "Import..." );


   HBox *abox = dialog.get_action_area(); 
   HBox hbox( false, 2 );
   
   m_adjust_load_offset = manage( new Adjustment( 0, -(c_max_sets - 1) , c_max_sets - 1, 1 ));
   m_spinbutton_load_offset = manage( new SpinButton( *m_adjust_load_offset ));
   m_spinbutton_load_offset->set_editable( false );
   m_spinbutton_load_offset->set_wrap( true );
   hbox.pack_end(*m_spinbutton_load_offset, false, false );
   hbox.pack_end(*(manage( new Label( "Screen Set Offset" ))), false, false, 4);
   
   abox->pack_end(hbox, false, false );  
   
   dialog.show_all_children();
   
   int result = dialog.run();
   
   //Handle the response:
   switch(result)
   {
       case(Gtk::RESPONSE_OK):
       {

           try{
               midifile f( dialog.get_filename() );
               f.parse( m_mainperf, (int) m_adjust_load_offset->get_value() );

           }
           catch(...){
               
               Gtk::MessageDialog errdialog(*this, 
                                            "Error reading file.", false,
                                            Gtk::MESSAGE_ERROR,
                                            (Gtk::ButtonsType)(Gtk::BUTTONS_OK),
                                            true);
                errdialog.run();
               
           }

           global_filename = std::string(dialog.get_filename());
           set_window_title_filename( global_filename );
           
           m_main_wid->reset();
           m_entry_notes->set_text( * m_mainperf->get_screen_set_notepad( m_mainperf->get_screenset() )); 
           m_adjust_bpm->set_value( m_mainperf->get_bpm() );

           break;
       }
       case(Gtk::RESPONSE_CANCEL):
       {
           break;
       }
       default:
       {
           break;
       }
   }

}


void
mainwnd::file_exit_dialog( void )
{
    Gtk::MessageDialog dialog(*this,
                              "Quit seq24 ?", false,
                              Gtk::MESSAGE_QUESTION,
                              (Gtk::ButtonsType)(Gtk::BUTTONS_OK_CANCEL),
                              true);
    
    int result = dialog.run();

    //Handle the response:
    switch(result)
    {
        case(Gtk::RESPONSE_OK):
        {
            m_quit = true;
            Gtk::Main::quit();
            
            break;
        }
        case(Gtk::RESPONSE_CANCEL):
        {   
            break;
        }
        default:
        { 
            break;
        }
    }
    
}

bool
mainwnd::on_delete_event(GdkEventAny *a_e)
{
    if ( ! m_quit ){
        file_exit_dialog();
        return true;
    }
    else {
        
        midifile f( "autosave.mid" );
        f.write( m_mainperf );

        stop_playing();
        return false;
    }
}



void 
mainwnd::about_dialog( void )
{
    Dialog dialog( "About", *this, true, true   );

    dialog.set_size_request( 450, 400 );

    dialog.add_button( " Ok ", Gtk::RESPONSE_OK  );

    Glib::RefPtr<TextBuffer> buffer = TextBuffer::create();
    buffer->set_text( c_about );
    
    TextView text( buffer ); 
    text.set_editable( false );

    dialog.get_vbox()->set_border_width( 10 );
    dialog.get_vbox()->pack_start( text, true, true, 0);

    dialog.show_all_children();
    dialog.run(); 
}





void 
mainwnd::adj_callback_ss( )
{
    m_mainperf->set_screenset( (int) m_adjust_ss->get_value()); 
    m_main_wid->set_screenset( m_mainperf->get_screenset());
    m_entry_notes->set_text( * m_mainperf->get_screen_set_notepad(m_mainperf->get_screenset()));
}


void 
mainwnd::adj_callback_bpm( )
{
    m_mainperf->set_bpm( (int) m_adjust_bpm->get_value()); 
}


bool
mainwnd::on_key_release_event(GdkEventKey* a_ev)
{
	if ( a_ev->keyval == m_mainperf->m_key_replace )
		{
			m_mainperf->unset_sequence_control_status( c_status_replace );
		}
	
	if (a_ev->keyval == m_mainperf->m_key_queue )
		{
			m_mainperf->unset_sequence_control_status( c_status_queue );
		}
	
	if ( a_ev->keyval == m_mainperf->m_key_snapshot_1 ||
		 a_ev->keyval == m_mainperf->m_key_snapshot_2 )
		{
			m_mainperf->unset_sequence_control_status( c_status_snapshot );
		}
	
    return false;
}

void
mainwnd::edit_callback_notepad( )
{
    string text = m_entry_notes->get_text();
    m_mainperf->set_screen_set_notepad( m_mainperf->get_screenset(), 
				        &text ); 
}



bool
mainwnd::on_key_press_event(GdkEventKey* a_ev)
{

    if( m_entry_notes->has_focus()){
        m_entry_notes->event( (GdkEvent*) a_ev );
        return false;
    }
    else
    {
        if ( a_ev->type == GDK_KEY_PRESS ){

            if ( global_print_keys ){
                printf( "key_press[%d]\n", a_ev->keyval ); 
            }
            
            if ( a_ev->keyval == m_mainperf->m_key_bpm_dn ){
                m_mainperf->set_bpm( m_mainperf->get_bpm() - 1 );  
                m_adjust_bpm->set_value(  m_mainperf->get_bpm() );
            }
            
            if ( a_ev->keyval ==  m_mainperf->m_key_bpm_up ){
                m_mainperf->set_bpm( m_mainperf->get_bpm() + 1 );   
                m_adjust_bpm->set_value(  m_mainperf->get_bpm() );
            }
            
            if ( a_ev->keyval == m_mainperf->m_key_replace )
            {
                m_mainperf->set_sequence_control_status( c_status_replace );
            }
            
            if (a_ev->keyval ==  m_mainperf->m_key_queue )
            {
                m_mainperf->set_sequence_control_status( c_status_queue );
            }
            
            if ( a_ev->keyval == m_mainperf->m_key_snapshot_1 ||
                 a_ev->keyval == m_mainperf->m_key_snapshot_2 )
            {
                m_mainperf->set_sequence_control_status( c_status_snapshot );
            }
            
            if ( a_ev->keyval == m_mainperf->m_key_screenset_dn ){

                m_mainperf->set_screenset(  m_mainperf->get_screenset() - 1 );  
                m_main_wid->set_screenset(  m_mainperf->get_screenset() );  
                m_adjust_ss->set_value( m_mainperf->get_screenset()  );	
                m_entry_notes->set_text( * m_mainperf->get_screen_set_notepad(m_mainperf->get_screenset()  )); 
            }
            
            if ( a_ev->keyval == m_mainperf->m_key_screenset_up ){

                m_mainperf->set_screenset(  m_mainperf->get_screenset() + 1 );  
                m_main_wid->set_screenset(  m_mainperf->get_screenset() );  
                m_adjust_ss->set_value( m_mainperf->get_screenset()  );	
                m_entry_notes->set_text( * m_mainperf->get_screen_set_notepad(m_mainperf->get_screenset()  )); 
            }
            
            if ( a_ev->keyval == m_mainperf->m_key_start )
            {
                start_playing();            
            }

            if ( a_ev->keyval == m_mainperf->m_key_stop )
            {
                stop_playing();            
            }


            if( m_mainperf->get_key_events()->count( a_ev->keyval) != 0 ){
                
                sequence_key(  (*m_mainperf->get_key_events())[a_ev->keyval] );
            }
        }
    }
    
    return false;
}


void 
mainwnd::sequence_key( int a_seq )
{
    int offset = m_mainperf->get_screenset() * c_mainwnd_rows * c_mainwnd_cols;
	
    if ( m_mainperf->is_active( a_seq + offset ) ){
		m_mainperf->sequence_playing_toggle( a_seq + offset );
    }
}


void
mainwnd::set_window_title_filename( std::string a_file )
{
    std::string title =
        ( PACKAGE )
        + string( "-" )
        + string( VERSION )
        + string( "  " )
        + a_file;
    
    set_title ( title.c_str());
}





void 
mainwnd::test( )
{
    m_mainperf->get_master_midi_bus( )->flush(); 
}
