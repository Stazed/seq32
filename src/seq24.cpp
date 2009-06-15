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

#ifdef __WIN32__
#    include "configwin32.h"
#else
#    include "config.h"
#endif

#include "font.h"
#ifdef LASH_SUPPORT
#    include "lash.h"
#endif
#include "mainwnd.h"
#include "midifile.h"
#include "optionsfile.h"
#include "perform.h"
#include "userfile.h"

/* struct for command parsing */
static struct 
option long_options[] = {

    {"file",     required_argument, 0, 'f'},
    {"help",     0, 0, 'h'},
    {"showmidi",     0, 0, 's'},
    {"show_keys",     0, 0, 'k' },
    {"stats",     0, 0, 'S' },
    {"priority", 0, 0, 'p' },
    {"ignore",required_argument, 0, 'i'},
    {"interaction_method",required_argument, 0, 'x'},
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
Glib::ustring global_filename = "";
Glib::ustring last_used_dir ="/";
std::string config_filename = ".seq24rc";
std::string user_filename = ".seq24usr";
bool global_print_keys = false;
interaction_method_e global_interactionmethod = e_seq24_interaction;

bool global_with_jack_transport = false;
bool global_with_jack_master = false;
bool global_with_jack_master_cond = false;
bool global_jack_start_mode = true;

user_midi_bus_definition   global_user_midi_bus_definitions[c_maxBuses];
user_instrument_definition global_user_instrument_definitions[c_max_instruments];

font *p_font_renderer;

#ifdef LASH_SUPPORT
lash *lash_driver = NULL;
#endif

#ifdef __WIN32__
#   define HOME "HOMEPATH"
#   define SLASH "\\"
#else
#   define HOME "HOME"
#   define SLASH "/"
#endif


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
#ifdef LASH_SUPPORT
	lash_driver = new lash(&argc, &argv);
#endif

    /* the main performance object */
    perform p; 

    /* all GTK applications must have a gtk_main(). Control ends here
       and waits for an event to occur (like a key press or mouse event). */
    Gtk::Main kit(argc, argv);

    p_font_renderer = new font();


    if ( getenv( HOME ) != NULL ){
        
        Glib::ustring home( getenv( HOME ));
        last_used_dir = home;
        Glib::ustring total_file = home + SLASH + config_filename;
        printf( "Reading [%s]\n", total_file.c_str());

        optionsfile options( total_file );

        if ( !options.parse( &p ) ){
            printf( "Error Reading [%s]\n", total_file.c_str());  
        }

        total_file = home + SLASH + user_filename;
        printf( "Reading [%s]\n", total_file.c_str());

        userfile user( total_file );

        if ( !user.parse( &p ) ){
            printf( "Error Reading [%s]\n", total_file.c_str());  
        }
    
    } else {
        
        printf( "Error calling getenv( \"%s\" )\n", HOME );  
    }

    

    /* parse parameters */
    int c;


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
                printf( "    --interaction_method <number>: see .seq24rc for methods to use\n" );
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
                global_filename = Glib::ustring(optarg);
               break;
                
            case 'i':
                /* ignore alsa device */
                global_device_ignore = true;
                global_device_ignore_num = atoi( optarg );
                break;
                
            case 'x':
                global_interactionmethod = (interaction_method_e)atoi( optarg );
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
        midifile *f = new midifile(global_filename);
        f->parse( &p, 0 );
        delete f;
    }

    mainwnd seq24_window( &p );

#ifdef LASH_SUPPORT
	lash_driver->start( &p );
#endif
    kit.run(seq24_window);
    
    p.deinit_jack();
    
    if ( getenv( HOME ) != NULL ){
        
        string home( getenv( HOME ));
        Glib::ustring total_file = home + SLASH + config_filename;
        printf( "Writing [%s]\n", total_file.c_str());

        optionsfile options( total_file );

        if ( !options.write( &p ) ){
            printf( "Error writing [%s]\n", total_file.c_str());  
        }
            
    } else {
        
        printf( "Error calling getenv( \"%s\" )\n", HOME );  
    }

#ifdef LASH_SUPPORT
	delete lash_driver;
#endif

    return 0;
}


/* declared in globals.h */
char* key2text( unsigned long int val )
{
    unsigned long int s[] = {GDK_Insert, GDK_Escape, GDK_Return, GDK_Delete,
                          GDK_BackSpace, GDK_Pause, GDK_Break, GDK_Tab, GDK_Scroll_Lock,
                          GDK_Left, GDK_Right, GDK_Up, GDK_Down, GDK_Page_Up, GDK_Page_Down,
                          GDK_Break, GDK_Num_Lock, GDK_F1, GDK_F2, GDK_F3, GDK_F4, GDK_F5,
                          GDK_F6, GDK_F7, GDK_F8, GDK_F9, GDK_F10, GDK_F11, GDK_F12,
                          GDK_Shift_L, GDK_Shift_R, GDK_Control_L, GDK_Control_R,
                          GDK_Caps_Lock, GDK_Alt_L, GDK_Alt_R, GDK_Home, GDK_End, '\n', '\t', 0 };
    char* str[] = {"Ins", "Esc", "Ret", "Del",
                   "Bck", "Paus", "Brk", "Tab", "ScLk",
                   "<-", "->", "Up", "Dn", "PgUp", "PgDn",
                   "Brk", "NmLk", "F1", "F2", "F3", "F4", "F5",
                   "F6", "F7", "F8", "F9", "F10", "F11", "F12",
                   "ShL", "ShR", "CtlL", "CtlR",
                   "Caps", "AltL", "AltR", "Home", "End", "Ret", "Tab", 0 };
    unsigned long int* s2 = s;
    char** str2 = str;
    while (*s2 != 0) { if (val == s2[0]) return str2[0];  s2++; str2++; }
    //for ( int x = '!'; x <= '~'; ++x) printf( "ascii: %d  '%c'\n", x, (char)x);

    static char buf[16][16];
    static int rotate = 0; //lets us use this in iostreams for example...
    rotate = (rotate + 1) % 16;
    strcpy( buf[rotate], "??" );

    //printf( "%d %d %d %d - ", val, ' ' <= val, val <= '~', *s2 );
    if (*s2 == 0 && ' ' <= val && val <= '~')
    {
        sprintf( buf[rotate], "'%c'", (char)val );
    }
    //printf( "  ret: %s\n", buf[rotate] );
    return buf[rotate];
}
