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

#include "lash.h"       // for config file which defines LASH_SUPPORT

#ifdef LASH_SUPPORT
#include <string>
#include <sigc++/slot.h>
#include <gtkmm.h>
#include "midifile.h"

lash::lash(int *argc, char ***argv)
{
    m_perform = NULL;

    m_client = lash_init(lash_extract_args(argc, argv), PACKAGE_NAME,
                         LASH_Config_File, LASH_PROTOCOL(2, 0));

    if (m_client == NULL)
    {
        fprintf(stderr, "Failed to connect to LASH.  "
                "Session management will not occur.\n");
    }
    else
    {
        lash_event_t* event = lash_event_new_with_type(LASH_Client_Name);
        lash_event_set_string(event, "Seq32");
        lash_send_event(m_client, event);
        printf("[Connected to LASH]\n");
    }
}

void
lash::set_mainwnd(mainwnd * a_main)
{
    m_mainwnd = a_main;
}

void
lash::set_alsa_client_id(int id)
{
    lash_alsa_client_id(m_client, id);
}

void
lash::start(perform* perform)
{
    m_perform = perform;
    /* Process any LASH events every 250 msec (arbitrarily chosen interval) */
    Glib::signal_timeout().connect(sigc::mem_fun(*this, &lash::process_events), 250);
}

bool
lash::process_events()
{
    lash_event_t *ev = NULL;

    // Process events
    while ((ev = lash_get_event(m_client)) != NULL)
    {
        handle_event(ev);
        lash_event_destroy(ev);
    }

    return true;
}

void
lash::handle_event(lash_event_t* ev)
{
    LASH_Event_Type type   = lash_event_get_type(ev);
    const char      *c_str = lash_event_get_string(ev);
    std::string     str    = (c_str == NULL) ? "" : c_str;

    if (type == LASH_Save_File)
    {
        midifile f(str + "/seq32.mid");
        f.write(m_perform, c_no_export_sequence);
        lash_send_event(m_client, lash_event_new_with_type(LASH_Save_File));
    }
    else if (type == LASH_Restore_File)
    {
        midifile f(str + "/seq32.mid");
        f.parse(m_perform, m_mainwnd, 0 );
        lash_send_event(m_client, lash_event_new_with_type(LASH_Restore_File));
    }
    else if (type == LASH_Quit)
    {
        m_client = NULL;
        Gtk::Main::quit();
    }
    else
    {
        fprintf(stderr, "Warning:  Unhandled LASH event.\n");
    }
}

void
lash::handle_config(lash_config_t* conf)
{
    const char *key     = NULL;
    const void *val     = NULL;
    size_t     val_size = 0;

    key      = lash_config_get_key(conf);
    val      = lash_config_get_value(conf);
    val_size = lash_config_get_value_size(conf);
}

#endif // LASH_SUPPORT
