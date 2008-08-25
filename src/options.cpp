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

#include "options.h"
#include <sstream>


const int c_status = 0;
const int c_status_inv = 1;
const int c_d1 = 2;
const int c_d2 = 3;
const int c_d3 = 4;

options::options (Gtk::Window & parent, perform * a_p):
    Gtk::Dialog ("Options", parent, true, true)
{
    m_perf = a_p;
    VBox *vbox = NULL;
    
    HBox *hbox = manage (new HBox ());
    get_vbox ()->pack_start (*hbox, false, false);

    get_action_area ()->set_border_width (2);
    hbox->set_border_width (6);

    m_button_ok = manage (new Button (" Ok "));
    get_action_area ()->pack_end (*m_button_ok, false, false);
    m_button_ok->signal_clicked ().connect (mem_fun (*this, &options::hide));


    m_notebook = manage (new Notebook ());
    hbox->pack_start (*m_notebook);

    // Clock  Buses
    int buses = m_perf->get_master_midi_bus ()->get_num_out_buses ();
    //Notebook *clock_notebook = manage( new Notebook());
    //clock_notebook->set_scrollable(true);

    vbox = manage (new VBox ());
    m_notebook->pages().push_back(Notebook_Helpers::TabElem (*vbox, "MIDI Clock"));

    CheckButton *check;
    Label *label;
    
    Gtk::Tooltips * tooltips = manage (new Tooltips ());

    for (int i = 0; i < buses; i++)
    {  
        HBox *hbox2 = manage (new HBox ());
        label = manage( new Label(m_perf->get_master_midi_bus ()->
                                            get_midi_out_bus_name (i), 0));

        hbox2->pack_start (*label, false, false);
        
        
        Gtk::RadioButton * rb_off = manage (new RadioButton ("Off"));
        tooltips->set_tip (*rb_off,
                "Midi Clock will be disabled.");

        
        Gtk::RadioButton * rb_on = manage (new RadioButton ("On (Pos)"));
        tooltips->set_tip (*rb_on,
                "Midi Clock will be sent. Midi Song Position and Midi Continue will be sent if starting greater than tick 0 in song mode, otherwise Midi Start is sent.");

        Gtk::RadioButton * rb_mod = manage (new RadioButton ("On (Mod)"));
        tooltips->set_tip (*rb_mod, "Midi Clock will be sent.  Midi Start will be sent and clocking will begin once the song position has reached the modulo of the specified Size. (Used for gear that doesn't respond to Song Position)");

        Gtk::RadioButton::Group group = rb_off->get_group ();
        rb_on->set_group (group);
        rb_mod->set_group (group);

        rb_off->signal_toggled().connect (sigc::bind(mem_fun (*this, &options::clock_callback_off), i, rb_off ));
        rb_on->signal_toggled ().connect (sigc::bind(mem_fun (*this, &options::clock_callback_on),  i, rb_on  ));
        rb_mod->signal_toggled().connect (sigc::bind(mem_fun (*this, &options::clock_callback_mod), i, rb_mod ));
        
        hbox2->pack_end (*rb_mod, false, false ); 
        hbox2->pack_end (*rb_on, false, false);
        hbox2->pack_end (*rb_off, false, false);

        vbox->pack_start( *hbox2, false, false );
       
        switch ( m_perf->get_master_midi_bus ()->get_clock (i))
        {
            case e_clock_off: rb_off->set_active(1); break;
            case e_clock_pos: rb_on->set_active(1); break;
            case e_clock_mod: rb_mod->set_active(1); break;
        }
                              
        // SET DEFAULT STATES check->set_active (m_perf->get_master_midi_bus ()->get_clock (i));
    }

    Adjustment *clock_mod_adj = new Adjustment( midibus::get_clock_mod(), 1, 16 << 10, 1 );
    SpinButton *clock_mod_spin = new SpinButton( *clock_mod_adj );

    HBox *hbox2 = manage (new HBox ());
    
    //m_spinbutton_bpm->set_editable( false );
    hbox2->pack_start(*(manage( new Label( " Clock Start Modulo (1/16 Notes) " ))), false, false, 4);
    hbox2->pack_start(*clock_mod_spin, false, false );

    vbox->pack_start( *hbox2, false, false );
    
    clock_mod_adj->signal_value_changed().connect( sigc::bind(mem_fun(*this,&options::clock_mod_callback),clock_mod_adj));


    // Input Buses
    buses = m_perf->get_master_midi_bus ()->get_num_in_buses ();

    vbox = manage (new VBox ());
    m_notebook->pages ().
        push_back (Notebook_Helpers::TabElem (*vbox, "MIDI Input"));

    for (int i = 0; i < buses; i++)
    {

        check =
            manage (new
                    CheckButton (m_perf->get_master_midi_bus ()->
                        get_midi_in_bus_name (i), 0));
        check->signal_toggled ().
            connect (bind (mem_fun (*this, &options::input_callback), i, check));
        check->set_active (m_perf->get_master_midi_bus ()->get_input (i));

        vbox->pack_start (*check, false, false);
    }

    // Jack
    VBox *vbox2 = manage (new VBox ());
    vbox2->set_border_width (4);
    m_notebook->pages ().
        push_back (Notebook_Helpers::TabElem (*vbox2, "Jack Sync"));


    check = manage (new CheckButton ("Jack Transport"));
    check->set_active (global_with_jack_transport);
    tooltips->set_tip (*check, "Enable sync with JACK Transport.");
    check->signal_toggled ().
        connect (bind
                (mem_fun (*this, &options::transport_callback), e_jack_transport,
                 check));
    vbox2->pack_start (*check, false, false);

    check = manage (new CheckButton ("Transport Master"));
    check->set_active (global_with_jack_master);
    tooltips->set_tip (*check, "Seq24 will attempt to serve as JACK Master.");
    check->signal_toggled ().
        connect (bind
                (mem_fun (*this, &options::transport_callback), e_jack_master,
                 check));

    vbox2->pack_start (*check, false, false);

    check = manage (new CheckButton ("Master Conditional"));
    check->set_active (global_with_jack_master_cond);
    tooltips->set_tip (*check,
            "Seq24 will fail to be master if there is already a master set.");
    check->signal_toggled ().
        connect (bind
                (mem_fun (*this, &options::transport_callback), e_jack_master_cond,
                 check));

    vbox2->pack_start (*check, false, false);


    Gtk::RadioButton * rb_live = manage (new RadioButton ("Live Mode"));
    tooltips->set_tip (*rb_live,
            "Playback will be in live mode.  Use this to allow muting and unmuting of loops.");

    Gtk::RadioButton * rb_perform = manage (new RadioButton ("Song Mode"));
    tooltips->set_tip (*rb_perform, "Playback will use the song editors data.");

    Gtk::RadioButton::Group group = rb_live->get_group ();
    rb_perform->set_group (group);

    if (global_jack_start_mode)
    {
        rb_perform->set_active (true);
    }
    else
    {
        rb_live->set_active (true);
    }

    rb_perform->signal_toggled ().
        connect (bind
                (mem_fun (*this, &options::transport_callback),
                 e_jack_start_mode_song, rb_perform));

    vbox2->pack_start (*rb_live, false, false);
    vbox2->pack_start (*rb_perform, false, false);


    Gtk::Button * button = manage (new Button ("Connect"));
    tooltips->set_tip (*button, "Connect to Jack.");
    button->signal_clicked ().
        connect (bind
                (mem_fun (*this, &options::transport_callback), e_jack_connect,
                 button));
    vbox2->pack_start (*button, false, false);

    button = manage (new Button ("Disconnect"));
    tooltips->set_tip (*button, "Disconnect Jack.");
    button->signal_clicked ().
        connect (bind
                (mem_fun (*this, &options::transport_callback), e_jack_disconnect,
                 button));
    vbox2->pack_start (*button, false, false);


    /* show everything */
    show_all_children ();
}



void
options::clock_callback_off (int a_bus, RadioButton *a_button)
{  
    if (a_button->get_active ())
        m_perf->get_master_midi_bus ()->set_clock(a_bus, e_clock_off );
}

void
options::clock_callback_on (int a_bus, RadioButton *a_button)
{  
    if (a_button->get_active ())
        m_perf->get_master_midi_bus ()->set_clock(a_bus, e_clock_pos );
}

void
options::clock_callback_mod (int a_bus, RadioButton *a_button)
{  
    if (a_button->get_active ())
        m_perf->get_master_midi_bus ()->set_clock(a_bus, e_clock_mod );
}

void 
options::clock_mod_callback( Adjustment *adj )
{
    midibus::set_clock_mod((int)adj->get_value());
}
 

    void
options::input_callback (int a_bus, Button * i_button)
{
    CheckButton *a_button = (CheckButton *) i_button;
    bool input = a_button->get_active ();
    m_perf->get_master_midi_bus ()->set_input (a_bus, input);
}


    void
options::transport_callback (button a_type, Button * a_check)
{
    CheckButton *check = (CheckButton *) a_check;

    switch (a_type)
    {

        case e_jack_transport:
            {
                global_with_jack_transport = check->get_active ();
            }
            break;

        case e_jack_master:
            {
                global_with_jack_master = check->get_active ();
            }
            break;

        case e_jack_master_cond:
            {
                global_with_jack_master_cond = check->get_active ();
            }
            break;

        case e_jack_start_mode_song:
            {
                //printf( "toggle %d\n" ,  check->get_active() );
                global_jack_start_mode = check->get_active ();
            }
            break;

        case e_jack_connect:
            {
                m_perf->init_jack ();
            }
            break;


        case e_jack_disconnect:
            {
                m_perf->deinit_jack ();
            }
            break;

        default:
            break;
    }
}
