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

#pragma once

#include "globals.h"
#include "perform.h"

/* forward declarations */
class seqedit;
class mainwnd;


using namespace Gtk;

class seqmenu : public virtual Glib::ObjectBase
{

private:

    Menu         *m_menu;
    perform      *m_mainperf;
    sequence     m_clipboard;
    
    std::vector<MenuItem> m_menu_items;

    void on_realize();

    void seq_edit();
    void seq_new();

    void seq_copy();
    void seq_export();
    void track_export();
    void seq_cut();
    void seq_paste();

    void seq_clear_perf();

    void set_bus_and_midi_channel( int a_bus, int a_ch );
    void set_song_mute(mute_op op);

    virtual void redraw( int a_sequence ) = 0;

protected:

    mainwnd      *m_mainwnd;
    int m_current_seq;
    void popup_menu();

public:

    seqmenu( perform *a_p, mainwnd *a_main );
    virtual ~seqmenu( ) { };
};
