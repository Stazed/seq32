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

#include <iostream>
#include <fstream>
#include <string>

#include "optionsfile.h"

extern Glib::ustring last_used_dir;

optionsfile::optionsfile(const Glib::ustring& a_name) :
    configfile( a_name )
{
}

bool
optionsfile::parse( perform *a_perf )
{
    /* open binary file */
    ifstream file ( m_name.c_str(), ios::in | ios::ate );

    if( ! file.is_open() )
        return false;

    /* run to start */
    file.seekg( 0, ios::beg );

    line_after( &file, "[midi-control]" );

    unsigned int sequences = 0;
    sscanf( m_line, "%u", &sequences );
    next_data_line( &file );

    for (unsigned int i = 0; i < sequences; ++i)
    {
        int sequence = 0;
        int tog[6], on[6], off[6];
        sscanf(m_line,
               "%d [ %d %d %d %d %d %d ]"
                 " [ %d %d %d %d %d %d ]"
                 " [ %d %d %d %d %d %d ]",

                &sequence,
                &tog[0], &tog[1], &tog[2], &tog[3], &tog[4], &tog[5],
                &on[0], &on[1], &on[2], &on[3], &on[4], &on[5],
                &off[0], &off[1], &off[2], &off[3], &off[4], &off[5]
            );
        
        a_perf->get_midi_control_toggle(i)->m_active            =  tog[0];
        a_perf->get_midi_control_toggle(i)->m_inverse_active    =  tog[1];
        a_perf->get_midi_control_toggle(i)->m_status            =  tog[2];
        a_perf->get_midi_control_toggle(i)->m_data              =  tog[3];
        a_perf->get_midi_control_toggle(i)->m_min_value         =  tog[4];
        a_perf->get_midi_control_toggle(i)->m_max_value         =  tog[5];
        
        a_perf->get_midi_control_on(i)->m_active                =  on[0];
        a_perf->get_midi_control_on(i)->m_inverse_active        =  on[1];
        a_perf->get_midi_control_on(i)->m_status                =  on[2];
        a_perf->get_midi_control_on(i)->m_data                  =  on[3];
        a_perf->get_midi_control_on(i)->m_min_value             =  on[4];
        a_perf->get_midi_control_on(i)->m_max_value             =  on[5];
        
        a_perf->get_midi_control_off(i)->m_active               =  off[0];
        a_perf->get_midi_control_off(i)->m_inverse_active       =  off[1];
        a_perf->get_midi_control_off(i)->m_status               =  off[2];
        a_perf->get_midi_control_off(i)->m_data                 =  off[3];
        a_perf->get_midi_control_off(i)->m_min_value            =  off[4];
        a_perf->get_midi_control_off(i)->m_max_value            =  off[5];
        
        next_data_line(&file);
    }
    /* group midi control */
    line_after( &file, "[mute-group]");

    int gtrack = 0;
    sscanf( m_line, "%d", &gtrack );
    next_data_line( &file );

    int mtx[c_seqs_in_set], j=0;
    for (int i=0; i< c_seqs_in_set; i++)
    {
        a_perf->select_group_mute(j);
        sscanf(m_line, "%d [%d %d %d %d %d %d %d %d]"
               " [%d %d %d %d %d %d %d %d]"
               " [%d %d %d %d %d %d %d %d]"
               " [%d %d %d %d %d %d %d %d]",
               &j,
               &mtx[0], &mtx[1], &mtx[2], &mtx[3],
               &mtx[4], &mtx[5], &mtx[6], &mtx[7],

               &mtx[8], &mtx[9], &mtx[10], &mtx[11],
               &mtx[12], &mtx[13], &mtx[14], &mtx[15],

               &mtx[16], &mtx[17], &mtx[18], &mtx[19],
               &mtx[20], &mtx[21], &mtx[22], &mtx[23],

               &mtx[24], &mtx[25], &mtx[26], &mtx[27],
               &mtx[28], &mtx[29], &mtx[30], &mtx[31]);
        for (int k=0; k< c_seqs_in_set; k++)
        {
            a_perf->set_group_mute_state(k, mtx[k]);
        }
        j++;
        next_data_line( &file );
    }

    line_after( &file, "[midi-clock]" );
    long buses = 0;
    sscanf( m_line, "%ld", &buses );
    next_data_line( &file );

    for ( int i=0; i<buses; ++i )
    {
        long bus_on, bus;
        sscanf( m_line, "%ld %ld", &bus, &bus_on );
        a_perf->get_master_midi_bus( )->set_clock( bus, (clock_e) bus_on );
        next_data_line( &file );
    }

    line_after( &file, "[keyboard-control]" );
    long keys = 0;
    sscanf( m_line, "%ld", &keys );
    next_data_line( &file );

    a_perf->key_events.clear();

    for ( int i=0; i<keys; ++i )
    {
        long key = 0, seq = 0;
        sscanf( m_line, "%ld %ld", &key, &seq );
        a_perf->set_key_event( key, seq );
        next_data_line( &file );
    }
    line_after( &file, "[keyboard-group]" );
    long groups = 0;
    sscanf( m_line, "%ld", &groups );
    next_data_line( &file );

    a_perf->key_groups.clear();

    for ( int i=0; i<groups; ++i )
    {
        long key = 0, group = 0;
        sscanf( m_line, "%ld %ld", &key, &group );
        a_perf->set_key_group( key, group );
        next_data_line( &file );
    }

    sscanf( m_line, "%u %u", &a_perf->m_key_bpm_up,
            &a_perf->m_key_bpm_dn );
    next_data_line( &file );
    sscanf( m_line, "%u %u %u", &a_perf->m_key_screenset_up,
            &a_perf->m_key_screenset_dn,
            &a_perf->m_key_set_playing_screenset);

    next_data_line( &file );

    sscanf( m_line, "%u %u %u", &a_perf->m_key_group_on,
            &a_perf->m_key_group_off,
            &a_perf->m_key_group_learn);

    next_data_line( &file );

    sscanf( m_line, "%u %u %u %u %u",
            &a_perf->m_key_replace,
            &a_perf->m_key_queue,
            &a_perf->m_key_snapshot_1,
            &a_perf->m_key_snapshot_2,
            &a_perf->m_key_keep_queue);
    next_data_line( &file );

    int show_key = 0;
    sscanf(m_line, "%d", &show_key);
    a_perf->m_show_ui_sequence_key = (bool) show_key;
    next_data_line( &file );

    sscanf( m_line, "%u", &a_perf->m_key_start );
    next_data_line( &file );

    sscanf( m_line, "%u", &a_perf->m_key_stop );

    line_after( &file, "[New-keys]" );
    sscanf( m_line, "%u", &a_perf->m_key_song );
    next_data_line( &file );

    sscanf( m_line, "%u", &a_perf->m_key_menu );
    next_data_line( &file );

    sscanf( m_line, "%u", &a_perf->m_key_follow_trans );

#ifdef JACK_SUPPORT
    next_data_line( &file );
    sscanf( m_line, "%u", &a_perf->m_key_jack );
#endif // JACK_SUPPORT
    
    next_data_line( &file );
    sscanf( m_line, "%u", &a_perf->m_key_tap_bpm );
    
    next_data_line( &file );
    sscanf( m_line, "%u", &a_perf->m_key_rewind );
    
    next_data_line( &file );
    sscanf( m_line, "%u", &a_perf->m_key_forward );
    
    next_data_line( &file );
    sscanf( m_line, "%u", &a_perf->m_key_pointer );
    
    next_data_line( &file );
    sscanf( m_line, "%u", &a_perf->m_key_playlist_next );
    
    next_data_line( &file );
    sscanf( m_line, "%u", &a_perf->m_key_playlist_prev );
    
    next_data_line( &file );
    sscanf( m_line, "%u", &a_perf->m_key_export_trigger );

    line_after( &file, "[jack-transport]" );
    long flag = 0;

    sscanf( m_line, "%ld", &flag );
    global_with_jack_transport = (bool) flag;

    next_data_line( &file );
    sscanf( m_line, "%ld", &flag );
    global_with_jack_master = (bool) flag;

    next_data_line( &file );
    sscanf( m_line, "%ld", &flag );
    global_with_jack_master_cond = (bool) flag;

    next_data_line( &file );
    sscanf( m_line, "%ld", &flag );
    global_song_start_mode = (bool) flag;

    line_after( &file, "[midi-input]" );
    buses = 0;
    sscanf( m_line, "%ld", &buses );
    next_data_line( &file );

    for ( int i=0; i<buses; ++i )
    {
        long bus_on, bus;
        sscanf( m_line, "%ld %ld", &bus, &bus_on );
        a_perf->get_master_midi_bus( )->set_input( bus, (bool) bus_on );
        next_data_line( &file );
    }

    /* midi clock mod */
    long ticks = 64;
    line_after( &file, "[midi-clock-mod-ticks]" );
    sscanf( m_line, "%ld", &ticks );
    midibus::set_clock_mod(ticks);

    /* manual alsa ports */
    line_after( &file, "[manual-alsa-ports]" );
    sscanf( m_line, "%ld", &flag );
    global_manual_alsa_ports = (bool) flag;

    /* last used dir */
    line_after( &file, "[last-used-dir]" );
    //FIXME: check for a valid path is missing
    if (m_line[0] == '/')
        last_used_dir.assign(m_line);

    line_after( &file, "[recent-files]" );
    {
        int count;
        sscanf(m_line, "%d", &count);
        for (int i = 0; i < count; ++i)
        {
            next_data_line( &file );
            if (strlen(m_line) > 0)
                a_perf->add_recent_file(std::string(m_line));
        }
    }    

    /* interaction method  */
    long method = 0;
    line_after( &file, "[interaction-method]" );
    sscanf( m_line, "%ld", &method );
    global_interactionmethod = (interaction_method_e)method;

    file.close();

    return true;
}

bool
optionsfile::write( perform *a_perf  )
{
    /* open binary file */

    ofstream file ( m_name.c_str(), ios::out | ios::trunc  );
    char outs[1024];

    if( ! file.is_open() )
        return false;

    /* midi control */

    file << "#\n";
    file << "# Seq 32 Init File\n";
    file << "#\n\n\n";

    file << "[midi-control]\n";
    file <<  c_midi_controls << "\n";

    for (int i=0; i< c_midi_controls; i++ )
    {
        switch( i )
        {

        /* 32 mute for channel
           32 group mute */
        case c_seqs_in_set               :
            file << "# mute in group\n";
            break;
        case c_midi_control_bpm_up       :
            file << "# bpm up\n";
            break;
        case c_midi_control_bpm_dn       :
            file << "# bpm down\n";
            break;
        case c_midi_control_ss_up        :
            file << "# screen set up\n";
            break;
        case c_midi_control_ss_dn        :
            file << "# screen set down\n";
            break;
        case c_midi_control_mod_replace  :
            file << "# mod replace\n";
            break;
        case c_midi_control_mod_snapshot :
            file << "# mod snapshot\n";
            break;
        case c_midi_control_mod_queue    :
            file << "# mod queue\n";
            break;
        case c_midi_control_mod_gmute    :
            file << "# mod gmute\n";
            break;
        case c_midi_control_mod_glearn   :
            file << "# mod glearn\n";
            break;
        case c_midi_control_play_ss      :
            file << "# screen set play\n";
            break;

        default:
            break;
        }

        snprintf( outs, sizeof(outs), "%d [%1d %1d %3ld %3ld %3ld %3ld]"
                  " [%1d %1d %3ld %3ld %3ld %3ld]"
                  " [%1d %1d %3ld %3ld %3ld %3ld]",
                  i,
                  a_perf->get_midi_control_toggle(i)->m_active,
                  a_perf->get_midi_control_toggle(i)->m_inverse_active,
                  a_perf->get_midi_control_toggle(i)->m_status,
                  a_perf->get_midi_control_toggle(i)->m_data,
                  a_perf->get_midi_control_toggle(i)->m_min_value,
                  a_perf->get_midi_control_toggle(i)->m_max_value,

                  a_perf->get_midi_control_on(i)->m_active,
                  a_perf->get_midi_control_on(i)->m_inverse_active,
                  a_perf->get_midi_control_on(i)->m_status,
                  a_perf->get_midi_control_on(i)->m_data,
                  a_perf->get_midi_control_on(i)->m_min_value,
                  a_perf->get_midi_control_on(i)->m_max_value,

                  a_perf->get_midi_control_off(i)->m_active,
                  a_perf->get_midi_control_off(i)->m_inverse_active,
                  a_perf->get_midi_control_off(i)->m_status,
                  a_perf->get_midi_control_off(i)->m_data,
                  a_perf->get_midi_control_off(i)->m_min_value,
                  a_perf->get_midi_control_off(i)->m_max_value );

        file << string(outs) << "\n";
    }

    /* group midi control */
    file << "\n\n\n[mute-group]\n";

    int mtx[c_seqs_in_set];
    file <<  c_gmute_tracks << "\n";
    for (int j=0; j < c_seqs_in_set; j++ )
    {
        a_perf->select_group_mute(j);
        for (int i=0; i < c_seqs_in_set; i++)
        {
            mtx[i] = a_perf->get_group_mute_state(i);
        }

        snprintf(outs, sizeof(outs),
                 "%d [%1d %1d %1d %1d %1d %1d %1d %1d]"
                 " [%1d %1d %1d %1d %1d %1d %1d %1d]"
                 " [%1d %1d %1d %1d %1d %1d %1d %1d]"
                 " [%1d %1d %1d %1d %1d %1d %1d %1d]",
                 j,
                 mtx[0], mtx[1], mtx[2], mtx[3],
                 mtx[4], mtx[5], mtx[6], mtx[7],

                 mtx[8], mtx[9], mtx[10], mtx[11],
                 mtx[12], mtx[13], mtx[14], mtx[15],

                 mtx[16], mtx[17], mtx[18], mtx[19],
                 mtx[20], mtx[21], mtx[22], mtx[23],

                 mtx[24], mtx[25], mtx[26], mtx[27],
                 mtx[28], mtx[29], mtx[30], mtx[31]);

        file << string(outs) << "\n";
    }

    /* bus mute/unmute data */
    int buses = a_perf->get_master_midi_bus( )->get_num_out_buses();

    file << "\n\n\n[midi-clock]\n";
    file << buses << "\n";

    for (int i=0; i< buses; i++ )
    {
        file << "# " << a_perf->get_master_midi_bus( )->get_midi_out_bus_name(i) << "\n";
        snprintf(outs, sizeof(outs), "%d %d", i,
                 (char) a_perf->get_master_midi_bus( )->get_clock(i));
        file << outs << "\n";
    }

    /* midi clock mod  */
    file << "\n\n[midi-clock-mod-ticks]\n";
    file << midibus::get_clock_mod() << "\n";

    /* bus input data */
    buses = a_perf->get_master_midi_bus( )->get_num_in_buses();

    file << "\n\n\n[midi-input]\n";
    file << buses << "\n";

    for (int i=0; i< buses; i++ )
    {
        file << "# " << a_perf->get_master_midi_bus( )->get_midi_in_bus_name(i) << "\n";
        snprintf(outs, sizeof(outs), "%d %d", i,
                 (char) a_perf->get_master_midi_bus( )->get_input(i));
        file << outs << "\n";
    }

    /* manual alsa ports */
    file << "\n\n\n[manual-alsa-ports]\n";
    file << "# set to 1 if you want seq32 to create its own alsa ports and\n";
    file << "# not connect to other clients\n";
    file << global_manual_alsa_ports << "\n";

    /* interaction-method */
    int x = 0;
    file << "\n\n\n[interaction-method]\n";
    while (c_interaction_method_names[x] && c_interaction_method_descs[x])
    {
        file << "# " << x << " - '" << c_interaction_method_names[x]
             << "' (" << c_interaction_method_descs[x] << ")\n";
        ++x;
    }
    file << global_interactionmethod << "\n";

    file << "\n\n\n[keyboard-control]\n";
    file << "# Key #, Sequence # \n";
    file << (a_perf->key_events.size() < (size_t)c_seqs_in_set ?
             a_perf->key_events.size() : (size_t)c_seqs_in_set) << "\n";

    for( std::map<unsigned int,long>::const_iterator i = a_perf->key_events.begin();
            i != a_perf->key_events.end(); ++i )
    {
        snprintf(outs, sizeof(outs), "%u  %ld        # %s", i->first,
                 i->second, gdk_keyval_name( i->first ) );
        file << string(outs) << "\n";
    }

    file << "\n\n\n[keyboard-group]\n";
    file << "# Key #, group # \n";
    file << (a_perf->key_groups.size() < (size_t)c_seqs_in_set ?
             a_perf->key_groups.size() : (size_t)c_seqs_in_set) << "\n";

    for( std::map<unsigned int,long>::const_iterator i = a_perf->key_groups.begin();
            i != a_perf->key_groups.end(); ++i )
    {
        snprintf(outs, sizeof(outs), "%u  %ld        # %s", i->first,
                 i->second, gdk_keyval_name(i->first));
        file << string(outs) << "\n";
    }

    file << "# bpm up, down\n"
         << a_perf->m_key_bpm_up << " "
         << a_perf->m_key_bpm_dn << "        # "
         << gdk_keyval_name( a_perf->m_key_bpm_up ) << " "
         << gdk_keyval_name( a_perf->m_key_bpm_dn ) << "\n";

    file << "# screen set up, down, play\n"
         << a_perf->m_key_screenset_up << " "
         << a_perf->m_key_screenset_dn << " "
         << a_perf->m_key_set_playing_screenset << "        # "
         << gdk_keyval_name( a_perf->m_key_screenset_up ) << " "
         << gdk_keyval_name( a_perf->m_key_screenset_dn ) << " "
         << gdk_keyval_name( a_perf->m_key_set_playing_screenset ) << "\n";

    file << "# group on, off, learn\n"
         << a_perf->m_key_group_on << " "
         << a_perf->m_key_group_off << " "
         << a_perf->m_key_group_learn << "        # "
         << gdk_keyval_name( a_perf->m_key_group_on ) << " "
         << gdk_keyval_name( a_perf->m_key_group_off ) << " "
         << gdk_keyval_name( a_perf->m_key_group_learn ) << "\n";

    file << "# replace, queue, snapshot_1, snapshot 2, keep queue\n"
         << a_perf->m_key_replace << " "
         << a_perf->m_key_queue << " "
         << a_perf->m_key_snapshot_1 << " "
         << a_perf->m_key_snapshot_2 << " "
         << a_perf->m_key_keep_queue << "        # "
         << gdk_keyval_name( a_perf->m_key_replace ) << " "
         << gdk_keyval_name( a_perf->m_key_queue ) << " "
         << gdk_keyval_name( a_perf->m_key_snapshot_1 ) << " "
         << gdk_keyval_name( a_perf->m_key_snapshot_2 ) << " "
         << gdk_keyval_name( a_perf->m_key_keep_queue ) << "\n";

    file << a_perf->m_show_ui_sequence_key
         << "        # show_ui_sequence_key (1=true/0=false)\n";
    file << a_perf->m_key_start << "        # "
         << gdk_keyval_name( a_perf->m_key_start )
         << " start sequencer\n";
    file << a_perf->m_key_stop << "        # "
         << gdk_keyval_name( a_perf->m_key_stop )
         << " stop sequencer\n";

    file << "\n\n\n[New-keys]\n";

    file << a_perf->m_key_song << "        # "
         << gdk_keyval_name( a_perf->m_key_song )
         << " Song mode\n";
    file << a_perf->m_key_menu << "        # "
         << gdk_keyval_name( a_perf->m_key_menu )
         << " Menu mode\n";
    file << a_perf->m_key_follow_trans << "        # "
         << gdk_keyval_name( a_perf->m_key_follow_trans )
         << " follow transport\n";
#ifdef JACK_SUPPORT
    file << a_perf->m_key_jack << "        # "
         << gdk_keyval_name( a_perf->m_key_jack )
         << " jack sync\n";
#endif // JACK_SUPPORT
    
    file << a_perf->m_key_tap_bpm << "        # "
         << gdk_keyval_name( a_perf->m_key_tap_bpm )
         << " tap BPM key\n";
    
    file << a_perf->m_key_rewind << "        # "
         << gdk_keyval_name( a_perf->m_key_rewind )
         << " rewind key\n";
    
    file << a_perf->m_key_forward << "        # "
         << gdk_keyval_name( a_perf->m_key_forward )
         << " fast forward key\n";

    file << a_perf->m_key_pointer << "        # "
         << gdk_keyval_name( a_perf->m_key_pointer )
         << " pointer key\n";

    file << a_perf->m_key_playlist_next << "        # "
         << gdk_keyval_name( a_perf->m_key_playlist_next )
         << " playlist next key\n";
    
    file << a_perf->m_key_playlist_prev << "        # "
         << gdk_keyval_name( a_perf->m_key_playlist_prev )
         << " playlist previous key\n";
    
    file << a_perf->m_key_export_trigger << "        # "
         << gdk_keyval_name( a_perf->m_key_export_trigger )
         << " trigger export key\n";
    
    file << "\n\n\n[jack-transport]\n\n"

         << "# jack_transport - Enable sync with JACK Transport.\n"
         << global_with_jack_transport << "\n\n"

         << "# jack_master - Seq32 will attempt to serve as JACK Master.\n"
         << global_with_jack_master << "\n\n"

         << "# jack_master_cond -  Seq32 will fail to be master if there is already a master set.\n"
         << global_with_jack_master_cond  << "\n\n"

         << "# song_start_mode\n"
         << "# 0 = Playback will be in live mode.  Use this to allow muting and unmuting of loops.\n"
         << "# 1 = Playback will use the song editors data.\n"
         << global_song_start_mode << "\n\n";

    file << "\n\n\n[last-used-dir]\n\n"
         << "# Last used directory.\n"
         << last_used_dir << "\n\n";

    
    /*
     *  New feature from Kepler34 via sequencer64
     */

    int count = a_perf->recent_file_count();
    file << "\n"
        "[recent-files]\n\n"
        "# Holds a list of the last few recently-loaded MIDI files.\n\n"
        << count << "\n\n" ;

    if (count > 0)
    {
        for (int i = 0; i < count; ++i)
            file << a_perf->recent_file(i, false) << "\n";

        file << "\n";
    }
    
    file.close();
    return true;
}
