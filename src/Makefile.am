 



AM_CXXFLAGS = $(DEPS_CFLAGS) $(GTKMM_CFLAGS) $(JACK_CFLAGS) $(LASH_CFLAGS) -Wall
LDADD = $(DEPS_LIBS)  $(GTKMM_LIBS) $(ALSA_LIBS) $(JACK_LIBS) $(LASH_LIBS)

bin_PROGRAMS = seq24 dump

dump_SOURCES = dump.cpp

seq24_SOURCES = \
event.cpp    mainwid.h  midibus.h  seqdata.cpp  seqevent.cpp  seqroll.cpp  sequence.cpp   \
event.h    mainwnd.cpp  perform.cpp  seqdata.h  seqevent.h  seqroll.h  sequence.h \
globals.h  mainwnd.h  perform.h  seqedit.cpp  seqkeys.cpp   seqtime.cpp \
mainwid.cpp  midibus.cpp  seq24.cpp    seqedit.h  seqkeys.h   seqtime.h \
controllers.h mutex.h mutex.cpp midifile.cpp midifile.h options.h \
options.cpp perfedit.cpp perfedit.h perfnames.cpp perfnames.h \
perfroll.cpp perfroll.h perftime.cpp perftime.h maintime.h maintime.cpp \
font.cpp font.h optionsfile.cpp optionsfile.h seqmenu.h seqmenu.cpp \
lash.h lash.cpp configfile.cpp configfile.h userfile.h userfile.cpp

EXTRA_DIST = *.xpm 

MOSTLYCLEANFILES = *~


