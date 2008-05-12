/* seqdemo.c by Matthias Nagorni */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <alsa/asoundlib.h>


snd_seq_t *open_seq();
void midi_action(snd_seq_t *seq_handle);

snd_seq_t *open_seq() {

  snd_seq_t *seq_handle;
  int portid;

  if (snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_INPUT, 0) < 0) {
    fprintf(stderr, "Error opening ALSA sequencer.\n");
    exit(1);
  }
  snd_seq_set_client_name(seq_handle, "Dump Util");
  if ((portid = snd_seq_create_simple_port(seq_handle, "Dump Util",
            SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
            SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
    fprintf(stderr, "Error creating sequencer port.\n");
    exit(1);
  }
  return(seq_handle);
}

void midi_action(snd_seq_t *seq_handle) {

    snd_seq_event_t *ev;
    snd_midi_event_t *midi_ev;

    snd_midi_event_new( 10, &midi_ev );

    unsigned char buf[10];
    static long clock;    
    do {


        snd_seq_event_input(seq_handle, &ev);
        memset(buf,0,10);
        snd_midi_event_decode (midi_ev, buf, 10, ev);
        printf ("0x%.2x 0x%.2x 0x%.2x\n", buf[0], buf[1], buf[2]  );
   
        if ( buf[0] == 0xf8 )
        {
            clock++; 

            long bar =   clock / 24 / 4;
            long beat = (clock / 24 ) %  4;
            long tick =  clock           % 24;
            
            printf( "clock [%ld] [%ld][%ld][%ld]\n", clock, bar+1, beat+1, tick );
        }
        if ( buf[0] == 0xfa )
        {
            clock=0; 
            printf( "start [%ld]\n", clock );
        }
        if ( buf[0] == 0xfc )
        {
            printf( "stop [%ld]\n", clock );
        }
        if ( buf[0] == 0xfb )
        {
            printf( "continue [%ld]\n", clock );
        }
        if ( buf[0] == 0xf2 )
        {
            clock = ((buf[2] << 7) | buf[1]) * 6; 
            printf( "songpos [%ld]\n", clock );
        }

 
//                clock f8  start fa  stop fc  continue fb
        // songpos f2 xx xx

                
        snd_seq_free_event(ev);

    } while (snd_seq_event_input_pending(seq_handle, 0) > 0);
}

int main(int argc, char *argv[]) {


  snd_seq_t *seq_handle;
  int npfd;
  struct pollfd *pfd;
    
  seq_handle = open_seq();
  npfd = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
  pfd = (struct pollfd *)alloca(npfd * sizeof(struct pollfd));
  snd_seq_poll_descriptors(seq_handle, pfd, npfd, POLLIN);
  while (1) {
    if (poll(pfd, npfd, 100000) > 0) {
      midi_action(seq_handle);
    }  
  }
}
