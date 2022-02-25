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

#include "seqmenu.h"
#include "seqedit.h"
#include "font.h"
#include "mainwnd.h"

// Constructor

seqmenu::seqmenu( perform *a_p, mainwnd *a_main ) :
    m_menu(NULL),
    m_mainperf(a_p),
    m_mainwnd(a_main)
{
    using namespace Menu_Helpers;

    // init the clipboard, so that we don't get a crash
    // on paste with no previous copy...
    m_clipboard.set_master_midi_bus( a_p->get_master_midi_bus() );
}

void
seqmenu::popup_menu()
{
    using namespace Menu_Helpers;

    delete m_menu;

    m_menu = manage( new Menu());
    
    m_menu_items.clear();
    m_menu_items.resize(12);

    if ( m_mainperf->is_active( m_current_seq ))
    {
#ifdef GTKMM_3_SUPPORT
        m_menu_items[0].set_label("Edit");
        m_menu_items[0].signal_activate().connect(mem_fun(*this, &seqmenu::seq_edit) );
        m_menu->append(m_menu_items[0]);
#else
        m_menu->items().push_back(MenuElem("Edit...",
                                          mem_fun(*this, &seqmenu::seq_edit)));
#endif
    }
    else
    {
#ifdef GTKMM_3_SUPPORT
        m_menu_items[1].set_label("New");
        m_menu_items[1].signal_activate().connect(mem_fun(*this, &seqmenu::seq_edit) );
        m_menu->append(m_menu_items[1]);
#else
        m_menu->items().push_back(MenuElem("New",
                                           mem_fun(*this, &seqmenu::seq_edit)));
#endif
    }
    
    MenuItem * menu_separator1 = new SeparatorMenuItem();
    m_menu->append(*menu_separator1);

    if ( m_mainperf->is_active( m_current_seq ))
    {
#ifdef GTKMM_3_SUPPORT
        m_menu_items[2].set_label("Cut");
        m_menu_items[2].signal_activate().connect(mem_fun(*this, &seqmenu::seq_cut) );
        m_menu->append(m_menu_items[2]);

        m_menu_items[3].set_label("Copy");
        m_menu_items[3].signal_activate().connect(mem_fun(*this, &seqmenu::seq_copy) );
        m_menu->append(m_menu_items[3]);

        m_menu_items[4].set_label("Export sequence");
        m_menu_items[4].signal_activate().connect(mem_fun(*this, &seqmenu::seq_export) );
        m_menu->append(m_menu_items[4]);
    
        m_menu_items[5].set_label("Export track");
        m_menu_items[5].signal_activate().connect(mem_fun(*this, &seqmenu::track_export) );
        m_menu->append(m_menu_items[5]);
#else
        m_menu->items().push_back(MenuElem("Cut", mem_fun(*this,&seqmenu::seq_cut)));
        m_menu->items().push_back(MenuElem("Copy", mem_fun(*this,&seqmenu::seq_copy)));
        m_menu->items().push_back(MenuElem("Export sequence", mem_fun(*this,&seqmenu::seq_export)));
        m_menu->items().push_back(MenuElem("Export track", mem_fun(*this,&seqmenu::track_export)));     
#endif
    }
    else
    {
#ifdef GTKMM_3_SUPPORT
        m_menu_items[6].set_label("Paste");
        m_menu_items[6].signal_activate().connect(mem_fun(*this, &seqmenu::seq_paste) );
        m_menu->append(m_menu_items[6]);
#else
        m_menu->items().push_back(MenuElem("Paste", mem_fun(*this,&seqmenu::seq_paste)));
#endif
    }

    MenuItem * menu_separator2 = new SeparatorMenuItem();
    m_menu->append(*menu_separator2);

#ifdef GTKMM_3_SUPPORT
    Menu *menu_song = manage( new Menu() );
    m_menu_items[7].set_label("Song");
    m_menu_items[7].set_submenu(*menu_song);
    m_menu->append(m_menu_items[7]);
#else
    m_menu->items().push_back( MenuElem( "Song", *menu_song) );
#endif

    if ( m_mainperf->is_active( m_current_seq ))
    {
#ifdef GTKMM_3_SUPPORT
        MenuItem * menu_item = new MenuItem("Clear Song Data");
        menu_item->signal_activate().connect(mem_fun(*this,&seqmenu::seq_clear_perf));
        menu_song->append(*menu_item);
#else
        menu_song->items().push_back(MenuElem("Clear Song Data", mem_fun(*this,&seqmenu::seq_clear_perf)));
#endif
    }

#ifdef GTKMM_3_SUPPORT
    m_menu_items[8].set_label("Mute all tracks");
    m_menu_items[8].signal_activate().connect(sigc::bind(mem_fun(*this, &seqmenu::set_song_mute), MUTE_ON));
    menu_song->append(m_menu_items[8]);
    
    m_menu_items[9].set_label("Unmute all tracks");
    m_menu_items[9].signal_activate().connect(sigc::bind(mem_fun(*this, &seqmenu::set_song_mute), MUTE_OFF));
    menu_song->append(m_menu_items[9]);
    
    m_menu_items[10].set_label("Unmute all tracks");
    m_menu_items[10].signal_activate().connect(sigc::bind(mem_fun(*this, &seqmenu::set_song_mute), MUTE_TOGGLE));
    menu_song->append(m_menu_items[10]);
#else

    menu_song->items().push_back(MenuElem("Mute all tracks",
                                            sigc::bind(mem_fun(*this, &seqmenu::set_song_mute), MUTE_ON)));

    menu_song->items().push_back(MenuElem("Unmute all tracks",
                                            sigc::bind(mem_fun(*this, &seqmenu::set_song_mute), MUTE_OFF)));

    menu_song->items().push_back(MenuElem("Toggle mute for all tracks",
                                            sigc::bind(mem_fun(*this, &seqmenu::set_song_mute), MUTE_TOGGLE)));
#endif
    if ( m_mainperf->is_active( m_current_seq ))
    {
#ifdef GTKMM_3_SUPPORT
        MenuItem * menu_separator3 = new SeparatorMenuItem();
        m_menu->append(*menu_separator3);
        
        Menu *menu_buses = manage( new Menu() );
        m_menu_items[11].set_label("Midi Bus");
        m_menu_items[11].set_submenu(*menu_buses);
        m_menu->append(m_menu_items[11]);
#else
        m_menu->items().push_back(SeparatorElem());
        Menu *menu_buses = manage( new Menu() );

        m_menu->items().push_back( MenuElem( "Midi Bus", *menu_buses) );
#endif
        /* midi buses */
        mastermidibus *masterbus = m_mainperf->get_master_midi_bus();
        for ( int i=0; i< masterbus->get_num_out_buses(); i++ )
        {
            Menu *menu_channels = manage( new Menu() );
#ifdef GTKMM_3_SUPPORT
            MenuItem * menu_item = new MenuItem(masterbus->get_midi_out_bus_name(i));
            menu_item->set_submenu(*menu_channels);
            menu_buses->append(*menu_item);
#else
            menu_buses->items().push_back(MenuElem( masterbus->get_midi_out_bus_name(i),
                                                    *menu_channels ));
#endif
            char b[4];

            /* midi channel menu */
            for( int j=0; j<16; j++ )
            {
                snprintf(b, sizeof(b), "%d", j + 1);
                std::string name = string(b);
                int instrument = global_user_midi_bus_definitions[i].instrument[j];
                if ( instrument >= 0 && instrument < c_maxBuses )
                {
                    name = name + (string(" (") +
                                   global_user_instrument_definitions[instrument].instrument +
                                   string(")") );
                }
#ifdef GTKMM_3_SUPPORT
                MenuItem * menu_item = new MenuItem(name);
                menu_item->signal_activate().connect(sigc::bind(mem_fun(*this,&seqmenu::set_bus_and_midi_channel), i, j));
                menu_channels->append(*menu_item);
#else
                menu_channels->items().push_back(MenuElem(name,
                                                 sigc::bind(mem_fun(*this,&seqmenu::set_bus_and_midi_channel),
                                                         i, j )));
#endif
            }
        }
    }
#ifdef GTKMM_3_SUPPORT
    m_menu->show_all();
    m_menu->popup_at_pointer(NULL);
#else
    m_menu->popup(0,0);
#endif
}

void
seqmenu::set_bus_and_midi_channel( int a_bus, int a_ch )
{
    if ( m_mainperf->is_active( m_current_seq ))
    {
        m_mainperf->get_sequence( m_current_seq )->set_midi_bus( a_bus );
        m_mainperf->get_sequence( m_current_seq )->set_midi_channel( a_ch );
        m_mainperf->get_sequence( m_current_seq )->set_dirty();
        global_is_modified = true;
    }
}

void
seqmenu::set_song_mute(mute_op op)
{
    m_mainperf->set_song_mute(op);
    global_is_modified = true;
}

// Menu callback, Lanches Editor Window
void
seqmenu::seq_edit()
{
    if ( m_mainperf->is_active( m_current_seq ))
    {
        if ( !m_mainperf->get_sequence( m_current_seq )->get_editing())
        {
            new seqedit( m_mainperf->get_sequence( m_current_seq ),
                         m_mainperf,
                         m_current_seq
                       );
        }
        else
        {
            m_mainperf->get_sequence( m_current_seq )->set_raise(true);
        }
    }
    else
    {
        this->seq_new();
        new seqedit( m_mainperf->get_sequence( m_current_seq ),
                     m_mainperf,
                     m_current_seq
                   );
    }
    m_mainperf->set_sequence_editing_list(true);
}

// Makes a New sequence
void
seqmenu::seq_new()
{
    if ( ! m_mainperf->is_active( m_current_seq ))
    {
        m_mainperf->new_sequence( m_current_seq );
        m_mainperf->get_sequence( m_current_seq )->set_dirty();
    }
}

// Copies selected to clipboard sequence */
void
seqmenu::seq_copy()
{
    if ( m_mainperf->is_active( m_current_seq ))
        m_clipboard = *(m_mainperf->get_sequence( m_current_seq ));
}

// Exports solo sequence to selected midi file
void
seqmenu::seq_export()
{
    if ( m_mainperf->is_active( m_current_seq ))
    {
        m_mainwnd->export_seq_track_trigger(E_MIDI_SOLO_SEQUENCE, m_current_seq);
    }
}

// Exports solo track in song format
void
seqmenu::track_export()
{
    if ( m_mainperf->is_active( m_current_seq ))
    {
        m_mainwnd->export_seq_track_trigger(E_MIDI_SOLO_TRACK, m_current_seq);
    }
}
// Deletes and Copies to Clipboard */
void
seqmenu::seq_cut()
{
    if ( m_mainperf->is_active( m_current_seq ) &&
            !m_mainperf->is_sequence_in_edit( m_current_seq ) )
    {
        m_clipboard = *(m_mainperf->get_sequence( m_current_seq ));
        m_mainperf->delete_sequence( m_current_seq );
        redraw( m_current_seq );
    }
}

// Puts clipboard into location
void
seqmenu::seq_paste()
{
    if ( ! m_mainperf->is_active( m_current_seq ))
    {
        m_mainperf->new_sequence( m_current_seq  );
        *(m_mainperf->get_sequence( m_current_seq )) = m_clipboard;

        m_mainperf->get_sequence( m_current_seq )->set_dirty();
    }
}

void
seqmenu::seq_clear_perf()
{
    if ( m_mainperf->is_active( m_current_seq ))
    {
        m_mainperf->push_trigger_undo(m_current_seq);

        m_mainperf->clear_sequence_triggers( m_current_seq  );
        m_mainperf->get_sequence( m_current_seq )->set_dirty();
    }
}
