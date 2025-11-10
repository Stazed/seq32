
install (FILES hicolor/48x48/apps/seq32.png
    hicolor/scalable/apps/seq32.svg
    DESTINATION share/pixmaps)

install(DIRECTORY hicolor/ DESTINATION share/icons/hicolor)

install (FILES seq32.desktop
    DESTINATION share/applications)