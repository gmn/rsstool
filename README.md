RSS Power Tool
=======


RSS Power Tool is a fast, powerful, lightweight tool for manipulating
rss (and atom) feeds on your system.  Data from feed updates is stored 
locally in a small sqlite3 database file.  It is easily searchable and 
viewable by using multiple methods. You can edit your feeds, merge them,
export them, update them, read updates, and a few other things. It should
import vanilla .opml of the type exported by Google Reader and other readers,
but this hasn't yet been extensively tested.

It runs in the commandline on Un*X operating systems. 
It has been tested recently on Ubuntu 12.10 and OS X 10.8.

Dependencies:
- libcurl
- sqlite3

Much more development is planned. The ncurses portion of the code (rss vis) is 
currently incomplete.  It is lacking some features, and is in the middle 
of an architecture re-write which should improve rendering and performance.  
That said, it works fine and is currently the best way to browse items after
updating.

Help documents for getting started and installation will be forthcoming.
Right now usage information is built into the application. Each sub-command
provides a --help (or -h) switch which will explain what it does and how it
can be used. A masterlist of commands can be accessed by a simple: "rss -h"

Planned
- generate .deb
- create proper man page
- finish ncurses rewrite
- finish bookmarks support (currently does not import bookmarks file)
- web pages with various help information 
- automatic data-culling, and size-limits to prevent db super-bloat
- proper time-zone support
- pthread support to speed up certain sections
- turn main function set into proper library with API
- tagging support
- misc..

All code herein is claimed Copyright 2013 by Gregory Naughton unless
otherwise noted, and is released under the GPLv3. 

Greg Naughton, May 2013

