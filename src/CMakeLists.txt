#CMake file for Seq32

project (seq32)

set (ProgSources
    configfile.cpp
    event.cpp
    keybindentry.cpp
    lfownd.cpp
    maintime.cpp
    mainwid.cpp
    mainwnd.cpp
    midibus.cpp
    midibus_portmidi.cpp
    midifile.cpp
    mutex.cpp
    options.cpp
    optionsfile.cpp
    perfedit.cpp
    perfnames.cpp
    perform.cpp
    perfroll.cpp
    perfroll_input.cpp
    perftime.cpp
    seqdata.cpp
    seqedit.cpp
    seqevent.cpp
    seqkeys.cpp
    seqmenu.cpp
    seqroll.cpp
    seqtime.cpp
    sequence.cpp
    tempo.cpp
    tempopopup.cpp
    userfile.cpp
)


set(ExternLibraries
    ${GTKMM_LIBRARIES}
    ${ALSA_LIBRARIES}
    ${JACK_LIBRARIES}
    ${LIBLO_LIBRARIES}
    rt
)

add_executable (seq32 ${ProgSources} seq32.cpp)

target_include_directories (
    seq32 PRIVATE
    ${GTKMM_INCLUDE_DIRS}
    ${ALSA_INCLUDE_DIRS}
    ${JACK_INCLUDE_DIRS}
    ${LIBLO_INCLUDE_DIRS}
    "${CMAKE_CURRENT_SOURCE_DIR}/pixmaps"
    "${CMAKE_CURRENT_BINARY_DIR}/pixmaps"
)

target_link_libraries (seq32 ${ExternLibraries})


install (TARGETS seq32 RUNTIME DESTINATION bin)
