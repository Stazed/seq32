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

#ifdef __WIN32__
#include "configwin32.h"
#else
#include "config.h"
#endif

#ifdef LASH_SUPPORT     // after config above which defines this
#include "perform.h"

class mainwnd;
#include <lash/lash.h>


/* all the ifdef skeleton work is done in this class in such a way that any
 * other part of the code can use this class whether or not lash support is
 * actually built in (the functions will just do nothing) */

class lash
{
private:
    perform       *m_perform;
    mainwnd       *m_mainwnd;
    lash_client_t *m_client;

    bool process_events();
    void handle_event(lash_event_t* conf);
    void handle_config(lash_config_t* conf);


public:
    lash(int *argc, char ***argv);
    void set_mainwnd(mainwnd * a_main);
    void set_alsa_client_id(int id);
    void start(perform* perform);
};


/* global lash driver, defined in seq32.cpp and used in midibus.cpp*/
extern lash *lash_driver;
#endif // LASH_SUPPORT