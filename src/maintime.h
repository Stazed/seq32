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

const int c_maintime_x = 300;
const int c_maintime_y = 10;
const int c_pill_width = 8;

/* main time*/
class maintime: public Gtk::DrawingArea
{

private:

    void on_realize();

    long m_tick;
    
protected:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);

public:

    int idle_progress( long a_ticks );
    maintime();

};
