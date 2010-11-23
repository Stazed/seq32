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

#include "userfile.h"
#include <iostream>

userfile::userfile( string a_name ) :
    configfile( a_name )
{
}

userfile::~userfile( )
{

}



bool
userfile::parse( perform *a_perf )
{

    /* file size */
    int file_size = 0;

    /* open binary file */
    ifstream file ( m_name.c_str(), ios::in | ios::ate );

    if( ! file.is_open() )
        return false;

    file_size = file.tellg();

    /* run to start */
    file.seekg( 0, ios::beg );

    line_after( &file, "[user-midi-bus-definitions]" );
    int buses = 0;
    sscanf( m_line, "%d", &buses );
    char bus_num[4];

    for ( int i=0; i<buses; i++ )
    {
        snprintf(bus_num, sizeof(bus_num), "%d", i);
        line_after( &file, "[user-midi-bus-" + string(bus_num) + "]");
        global_user_midi_bus_definitions[i].alias = m_line;
        next_data_line( &file );
        int instruments=0;
        int instrument;
        int channel;

        sscanf( m_line, "%d", &instruments );
        for ( int j=0; j<instruments; j++ )
        {
            next_data_line( &file );
            sscanf( m_line, "%d %d", &channel, &instrument );
            global_user_midi_bus_definitions[i].instrument[channel] = instrument;
            // printf( "%d %d\n", channel, instrument );
        }
    }

    line_after( &file, "[user-instrument-definitions]" );
    int instruments = 0;
    sscanf( m_line, "%d", &instruments );
    char instrument_num[4];

    for ( int i=0; i<instruments; i++ )
    {
        snprintf(instrument_num, sizeof(instrument_num), "%d", i);
        line_after( &file, "[user-instrument-" + string(instrument_num) + "]");
        global_user_instrument_definitions[i].instrument = m_line;
        // printf( "%d %s\n", i, m_line );
        next_data_line( &file );
        int ccs=0;
        int cc=0;

        char cc_name[1024];

        sscanf( m_line, "%d", &ccs );
        for ( int j=0; j<ccs; j++ )
        {
            next_data_line( &file );
            sscanf( m_line, "%d", &cc );
            sscanf( m_line, "%[^\n]", cc_name );
            global_user_instrument_definitions[i].controllers[cc] = string(cc_name);
            global_user_instrument_definitions[i].controllers_active[cc] = true;
            // printf( "[%d] %s\n", cc, cc_name );
        }
    }


#if 0
    line_after( &file, "[midi-control]" );

    unsigned int sequences = 0;
    sscanf( m_line, "%u", &sequences );
    next_data_line( &file );

    for ( unsigned int i=0; i<sequences; ++i ){

        int sequence = 0;

        sscanf( m_line, "%d [ %d %d %ld %ld %ld %ld ] [ %d %d %ld %ld %ld %ld ] [ %d %d %ld %ld %ld %ld ]",

                &sequence,

                &a_perf->get_midi_control_toggle(i)->m_active,
                &a_perf->get_midi_control_toggle(i)->m_inverse_active,
                &a_perf->get_midi_control_toggle(i)->m_status,
                &a_perf->get_midi_control_toggle(i)->m_data,
                &a_perf->get_midi_control_toggle(i)->m_min_value,
                &a_perf->get_midi_control_toggle(i)->m_max_value,

                &a_perf->get_midi_control_on(i)->m_active,
                &a_perf->get_midi_control_on(i)->m_inverse_active,
                &a_perf->get_midi_control_on(i)->m_status,
                &a_perf->get_midi_control_on(i)->m_data,
                &a_perf->get_midi_control_on(i)->m_min_value,
                &a_perf->get_midi_control_on(i)->m_max_value,

                &a_perf->get_midi_control_off(i)->m_active,
                &a_perf->get_midi_control_off(i)->m_inverse_active,
                &a_perf->get_midi_control_off(i)->m_status,
                &a_perf->get_midi_control_off(i)->m_data,
                &a_perf->get_midi_control_off(i)->m_min_value,
                &a_perf->get_midi_control_off(i)->m_max_value );

        next_data_line( &file );
    }
    /* group midi control */
    line_after( &file, "[mute-group]");

    int gtrack = 0;
    sscanf( m_line, "%d", &gtrack );
    next_data_line( &file );

    int mtx[c_seqs_in_set], j=0;
    for (int i=0; i< c_seqs_in_set; i++) {
        a_perf->select_group_mute(j);
        sscanf (m_line, "%d [%d %d %d %d %d %d %d %d] [%d %d %d %d %d %d %d %d] [%d %d %d %d %d %d %d %d] [%d %d %d %d %d %d %d %d]",
            &j,
            &mtx[0], &mtx[1], &mtx[2], &mtx[3],
            &mtx[4], &mtx[5], &mtx[6], &mtx[7],

            &mtx[8], &mtx[9], &mtx[10], &mtx[11],
            &mtx[12], &mtx[13], &mtx[14], &mtx[15],

            &mtx[16], &mtx[17], &mtx[18], &mtx[19],
            &mtx[20], &mtx[21], &mtx[22], &mtx[23],

            &mtx[24], &mtx[25], &mtx[26], &mtx[27],
            &mtx[28], &mtx[29], &mtx[30], &mtx[31]);
        for (int k=0; k< c_seqs_in_set; k++) {
            a_perf->set_group_mute_state(k, mtx[k]);
        }
        j++;
        next_data_line( &file );
    }

    line_after( &file, "[midi-clock]" );
    long buses = 0;
    sscanf( m_line, "%ld", &buses );
    next_data_line( &file );

    for ( int i=0; i<buses; ++i ){

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


    for ( int i=0; i<keys; ++i ){

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


    for ( int i=0; i<groups; ++i ){

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
    sscanf( m_line, "%ld", &a_perf->m_show_ui_sequence_key );
    next_data_line( &file );
    sscanf( m_line, "%ld", &a_perf->m_key_start );
    next_data_line( &file );
    sscanf( m_line, "%ld", &a_perf->m_key_stop );

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
    global_jack_start_mode = (bool) flag;


    line_after( &file, "[midi-input]" );
    buses = 0;
    sscanf( m_line, "%ld", &buses );
    next_data_line( &file );

    for ( int i=0; i<buses; ++i ){

        long bus_on, bus;
        sscanf( m_line, "%ld %ld", &bus, &bus_on );
        a_perf->get_master_midi_bus( )->set_input( bus, (bool) bus_on );
        next_data_line( &file );
    }

    /* midi clock mod  */
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

    /* interaction method  */
    long method = 0;
    line_after( &file, "[interaction-method]" );
    sscanf( m_line, "%ld", &method );
    global_interactionmethod = (interaction_method_e)method;

#endif
    file.close();

    return true;
}


bool
userfile::write( perform *a_perf  )
{
    return false;
}
