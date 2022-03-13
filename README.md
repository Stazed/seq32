seq32 README
------------

Screenshot
----------

![screenshot](https://raw.github.com/Stazed/seq32/wip/icons/seq32-2.0.0.png "Seq32 release-2.0.0")

What is seq32?
--------------
It's a fork of [seq24](https://launchpad.net/seq24) (which is a fork of the [original seq24](http://filter24.org/seq24/)).

This a GTKMM-3 version with many enhancements.

Version 2.0.0 changes:

Beginning with version 2.0.0 seq32 has been ported to Gtkmm-3. 

New colors.

The build system has been moved to CMAKE.

Additional editing features.

NSM support now has optional gui, and dirty flag.

See the SEQ32 document for additional information.

Install
-------

The dependencies are:

*   cmake
*   Gtkmm-3.0
*   Jackd
*   libasound2
*   liblo   (NSM support)
*   librt

Additional dependencies required by Gtkmm-3
*   sigc++-2.0
*   glibmm-2.4
*   cairomm-1.0
*   pangomm-1.4
*   atkmm-1.6

For compiling you will also need the development packages for the above dependencies.

What to do with a fresh repository checkout?
--------------------------------------------
Using CMAKE:
```bash
    mkdir build
    cd build
    cmake ..
    make
    make install (as root)
```
To remove:
```bash
    make uninstall (as root)
```

--------------------------------------------
Using old autotools:

Apply "autoreconf -i" to get a configure script, then:
Run the usual "./configure" then "make".

To install: As root, "make install".
Or you can copy the "seq42" binary (in the "src" directory) wherever you like.

Re: macro `AM_PATH_ALSA' not found in library

Please run 'aclocal -I m4' to regenerate the aclocal.m4 file.
You need aclocal from GNU automake 1.5 (or newer) to do this.
Then run 'autoreconf -i' to regenerate the configure file.
