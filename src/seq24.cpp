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

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "perform.h"
#include "mainwnd.h"
#include "midifile.h"
#include "optionsfile.h"
#include "userfile.h"
#include "font.h"
#include "lash.h"

/* struct for command parsing */
static struct 
option long_options[] = {

    {"file",     required_argument, 0, 'f'},
    {"help",     0, 0, 'h'},
    {"showmidi",     0, 0, 's'},
    {"stats",     0, 0, 'S' },
    {"priority", 0, 0, 'p' },
    {"ignore",required_argument, 0, 'i'},
    {"jack_transport",0, 0, 'j'},
    {"jack_master",0, 0, 'J'},
    {"jack_master_cond",0,0,'C'},
    {"jack_start_mode", required_argument, 0, 'M' },
    {"manual_alsa_ports", 0, 0, 'm' },
    {"pass_sysex", 0, 0, 'P'},
    {"show_keys", 0,0,'k'},
    {0, 0, 0, 0}

};

bool global_manual_alsa_ports = false;
bool global_showmidi = false;
bool global_priority = false;
bool global_device_ignore = false;
int global_device_ignore_num = 0;
bool global_stats = false;
bool global_pass_sysex = false;
std::string global_filename = "";
std::string config_filename = ".seq24rc";
std::string user_filename = ".seq24usr";
bool global_print_keys = false;

bool global_with_jack_transport = false;
bool global_with_jack_master = false;
bool global_with_jack_master_cond = false;
bool global_jack_start_mode = true;

user_midi_bus_definition   global_user_midi_bus_definitions[c_maxBuses];
user_instrument_definition global_user_instrument_definitions[c_max_instruments];

font *p_font_renderer;
lash *lash_driver = NULL;


int 
main (int argc, char *argv[])
{
    for ( int i=0; i<c_maxBuses; i++ )
    {
        for ( int j=0; j<16; j++ )
            global_user_midi_bus_definitions[i].instrument[j] = -1;
    }

    for ( int i=0; i<c_max_instruments; i++ )
    {
        for ( int j=0; j<128; j++ )
            global_user_instrument_definitions[i].controllers_active[j] = false;
    }
    
    /* init the lash driver (strips lash specific cmdline arguments */
    lash_driver = new lash(&argc, &argv);

    /* the main performance object */
    perform p; 

    /* all GTK applications must have a gtk_main(). Control ends here
       and waits for an event to occur (like a key press or mouse event). */
    Gtk::Main kit(argc, argv);

    p_font_renderer = new font();


    if ( getenv( "HOME" ) != NULL ){
        
        string home( getenv( "HOME" ));
        std::string total_file = home + "/" + config_filename;
        printf( "Reading [%s]\n", total_file.c_str());

        optionsfile options( total_file );

        if ( !options.parse( &p ) ){
            printf( "Error Reading [%s]\n", total_file.c_str());  
        }

        total_file = home + "/" + user_filename;
        printf( "Reading [%s]\n", total_file.c_str());

        userfile user( total_file );

        if ( !user.parse( &p ) ){
            printf( "Error Reading [%s]\n", total_file.c_str());  
        }
    
    } else {
        
        printf( "Error calling getenv( \"HOME\" )\n" );  
    }

    

    /* parse parameters */
    int c;
    
    midifile *f;

    while (1){
	
        /* getopt_long stores the option index here. */
        int option_index = 0;
        
        c = getopt_long (argc, argv, "p:f:v", long_options, &option_index);
        
        /* Detect the end of the options. */
        if (c == -1)
            break;
        
        switch (c){
            
            case '?':
            case 'h':
                
                printf( "usage: seq24 [options]\n\n" );
                printf( "options:\n" );
                printf( "    --help : show this message\n" );
                printf( "    --file <filename> : load midi file on startup\n" );
                printf( "    --manual_alsa_ports : seq24 won't attach alsa ports\n" );
                printf( "    --showmidi : dumps incoming midi to screen\n" );
                printf( "    --priority : runs higher priority with FIFO scheduler (must be root)\n" );
                printf( "    --pass_sysex : passes any incoming sysex messages to all outputs \n" );
                printf( "    --show_keys : prints pressed key value\n" );
                printf( "    --jack_transport : seq24 will sync to jack transport\n" );
                printf( "    --jack_master : seq24 will try to be jack master\n" );
                printf( "    --jack_master_cond : jack master will fail if there is already a master\n" );
                printf( "    --jack_start_mode <x> : when seq24 is synced to jack, the following play\n" );
                printf( "                          modes are available (0 = live mode)\n");
                printf( "                                              (1 = song mode) (default)\n" );
                printf( "\n\n\n" );
                
                return 0;
                break;
                
            case 'S':
                global_stats = true;
                break;
                
            case 's':
                global_showmidi = true;
                break;
                
            case 'p':
                global_priority = true;
                break;
                
            case 'P':
                global_pass_sysex = true;
                break;
                
            case 'k':
                global_print_keys = true;
                break;
                
            case 'j':
                global_with_jack_transport = true;
                break;
                
            case 'J':
                global_with_jack_master = true;
                break;
                
            case 'C':
                global_with_jack_master_cond = true;
                break;

            case 'M':
                if (atoi( optarg ) > 0) {
                    global_jack_start_mode = true;
                }
                else {
                    global_jack_start_mode = false;
                }
                break;

            case 'm':
                global_manual_alsa_ports = true;
                break;

            case 'f':
                global_filename = std::string( optarg );
               break;
                
            case 'i':
                /* ignore alsa device */
                global_device_ignore = true;
                global_device_ignore_num = atoi( optarg );
                break;
                
                
            default:
                break;
        }
        
    } /* end while */


    p.init();
    
    p.launch_input_thread();
    p.launch_output_thread();
    p.init_jack();
    
    if (global_filename != "") {
        /* import that midi file */
        f = new midifile( global_filename.c_str());
        f->parse( &p, 0 );
        delete f;
    }

    mainwnd seq24_window( &p );

    lash_driver->start( &p );
    kit.run(seq24_window);
    
    p.deinit_jack();
    
    if ( getenv( "HOME" ) != NULL ){
        
        string home( getenv( "HOME" ));
        std::string total_file = home + "/" + config_filename;
        printf( "Writing [%s]\n", total_file.c_str());

        optionsfile options( total_file );

        if ( !options.write( &p ) ){
            printf( "Error writing [%s]\n", total_file.c_str());  
        }
            
    } else {
        
        printf( "Error calling getenv( \"HOME\" )\n" );  
    }

    delete lash_driver;

    return 0;
}
