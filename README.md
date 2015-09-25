RSS Power Tool
=======

**RSS Power Tool** is a fast, powerful, lightweight tool for manipulating
rss (and atom) feeds on your system.  Data from feed updates is stored
locally in a small sqlite3 database file.  It is easily searchable and
viewable by using multiple methods. You can edit your feeds, merge them,
export them, update them, read updates, and a few other things. It should
import vanilla .opml of the type exported by Google Reader and other readers,
but this hasn't yet been extensively tested.

[![image](https://camo.githubusercontent.com/8fe2d0d124abf929ee00b33f18070bcdbf32b64e/687474703a2f2f6d7974686d616368696e652e6f72672f696d616765732f7270745f6974656d5f766965772e706e67)](https://github.com/gmn/rsstool/wiki/RSS-Power-Tool---Main-Page "Click for more Images and Instructions")

It runs in the commandline on Unix operating systems.
It has been tested recently on Ubuntu 12.10, 13.10, 14.04 and OS X 10.8.

Dependencies:
- libcurl
- sqlite3

# Purpose

The purpose of **RSS Power Tool** is to provide a more powerful interface for reading, searching, colating and managing their RSS and ATOM feeds. It exports bookmarks XML feed, allows for fast, configurable searching, and supports some browser, Instapaper and Pinboard integration for fast saving links.

[![image](https://camo.githubusercontent.com/cbc902f133a70f7363ca0b68c5738ef00ccb736a/687474703a2f2f6d7974686d616368696e652e6f72672f696d616765732f7270745f6974656d5f6c6973745f766965772e706e67)](https://github.com/gmn/rsstool/wiki/RSS-Power-Tool---Main-Page "Click here for instructions")

# dev miscellanea

Much more development is planned. The ncurses portion of the code (rss vis) is
currently incomplete.  It is lacking some features, and is in the middle
of an architecture re-write which should improve rendering and performance.
That said, it works fine and is currently the best way to browse items after
updating.

Help documents for getting started and installation will be forthcoming.
Right now usage information is built into the application. Each sub-command
provides a --help (or -h) switch which will explain what it does and how it
can be used. A masterlist of commands can be accessed by a simple: "rss -h"

TODO
- finish bookmarks support (currently does not import bookmarks file)
- bookmarks save path set in config, so we can automatically save and import
    bookmarks from a convenient location such as Dropbox, to automatically
    sync them across clients

- generate .deb
- create proper man page
- finish ncurses rewrite
- web pages with various help information
- automatic data-culling, and size-limits to prevent db super-bloat
- proper time-zone support
- pthread support to speed up certain sections
- turn main function set into proper library with API
- tagging support
- misc..

All code herein is claimed Copyright 2013 by Gregory Naughton unless
otherwise noted, and is released under the GPLv3.

Greg Naughton, May 2013, greg@naughton.org

