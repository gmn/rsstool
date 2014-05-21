/*
======================================================================

RSS Power Tool Source Code
Copyright (C) 2013 Gregory Naughton

This file is part of RSS Power Tool

RSS Power Tool is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RSS Power Tool is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RSS Power Tool  If not, see <http://www.gnu.org/licenses/>.

======================================================================
*/

#ifndef __CURSEVIEW_H__
#define __CURSEVIEW_H__

#include "dba_sqlite.h"
#include "ftimer.h"
#include "item_result.h"

#include <curses.h>



enum bookmarklet_enum_t {
    BOOKMARKLET_NONE,
    BOOKMARKLET_PINBOARD,
    BOOKMARKLET_INSTAPAPER,
    BOOKMARKLET_FACEBOOK,
    BOOKMARKLET_TWITTER,
    BOOKMARKLET_PINTAREST,
    BOOKMARKLET_INSTAGRAM,
    BOOKMARKLET_GOOGLE_PLUS,
    BOOKMARKLET_STUMBLEUPON,
    BOOKMARKLET_DELICIOUS
};

struct bookmarklet_t {
    bookmarklet_enum_t type;
    const char * run;
};


/******************************************************************************
 *
 * timerHandler_t, timedEvent_t, keyResponder_t
 *
 ******************************************************************************/
class runtimeHandler_t
{
    virtual void key( int ) = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void update() = 0;
    virtual void draw() = 0;
};


enum 
{
    // means timedEvent will consumed this key
    TIMER_HANDLES_INPUT = BIT(1),

    // waits for any key to cancel
    MSG_BLOCKING        = (BIT(2) | TIMER_HANDLES_INPUT),

    // ignores input, goes away after N seconds
    MSG_TIMED           = BIT(3),

    // goes away after N seconds, OR if key pressed
    MSG_TIMED_ANY_KEY   = (BIT(4) | TIMER_HANDLES_INPUT),

    SSHOW_COUNTER       = BIT(5),


    // *not impl
    // blocks until Y or N received
    MSG_YES_OR_NO       = (BIT(6) | TIMER_HANDLES_INPUT),
    // same: Y=yes, any other key = No
    MSG_YES_OR_ANY_KEY  = (BIT(7) | TIMER_HANDLES_INPUT),
};

struct keyResponder_t
{
    int * i_type;

    keyResponder_t() : i_type(0)
    { }

    keyResponder_t( int * p ) : i_type(p)
    { }

    virtual int answer( int i ) {
        if ( i_type )
            *i_type = i;
        return i;
    }

    virtual ~keyResponder_t()
    { }

    int operator()( int x ) {
        return answer(x);
    }
};

struct bookmarkResponder_t : public keyResponder_t
{
    int bookmark_id;
    bookmarkResponder_t( int b ) : bookmark_id(b)
    { }

    int answer( int i ) ;
};


struct yesNoKeyResponder_t : public keyResponder_t
{
    yesNoKeyResponder_t( int * p ) : keyResponder_t(p)
    { }
};

struct yesAnyKeyResponder_t : public keyResponder_t
{
    yesAnyKeyResponder_t( int * p ) : keyResponder_t(p)
    { }
};

struct timedEvent_t
{
    int flag;
    WINDOW * win;
    int _remove;
    keyResponder_t * responder;

    timedEvent_t() : flag(0), win(0), _remove(0), responder(0)
    { }

    mtimer_t timer;
    int timeup() { return timer.timeup(); }


    virtual ~timedEvent_t() 
    { 
        if ( responder )
            delete responder;          
        responder = 0;
    }
    virtual void update() 
    { }
    virtual void draw( void )
    { 
        if ( win )
            wrefresh( win );
    }
    // returns 0 for not consume key, anything else consumes key
    virtual int handleKey( int k ) {
        return 0;
    }
    // called when timer times out
    virtual void timeupMethod( void ) 
    { }


    int remove() { return _remove; }
    void markForRemoval( int m =1 ) { _remove = m; }
};

struct sshowCounter_t : public timedEvent_t
{
    int x, y;
    int count;
    int initial;

    void timeupMethod() {
        draw();
        if ( 0 == --count ) 
            count = initial;
        timer.set( 1000 );
    }

    void start( int _y, int _x, int dur, WINDOW * w =0 ) {
        x = _x; y = _y;
        count = initial = dur;
        win = w ? w : stdscr;
        timer.set( 1000 );
        draw();
    }

    void draw() {
        mvwprintw( win, y, x, "%d", count );
        timedEvent_t::draw();
    }
};

struct msgboxTimed_t : public timedEvent_t
{
    basicString_t msg;

    msgboxTimed_t() 
    { }

    msgboxTimed_t( const char * str, int msec ) 
    {
        msg = str;
        timer.set( msec );
    }

    void timeupMethod() {
        _remove = 1;
    }

    void draw() {
        mvwprintw( win, 0, 0, "%s", msg.str );
        timedEvent_t::draw();
    }
};


struct timerHandler_t
{
    WINDOW * msgboxwin;

    timerHandler_t() {
        int x, y;
        getmaxyx( stdscr, y, x );
        msgboxwin = newwin( y/2+2, x/2+2, y/4-2, x/4-2 );
    }

    ~timerHandler_t() {
        if ( msgboxwin )
            delwin(msgboxwin);
    }
    
    list_t<timedEvent_t*> events;

    void runEvents();

    void draw();

    void handleInput( list_t<int>& input ); // fifo

    void updateEvents();


    // Message Boxes
    timedEvent_t * postMessage( int flag, int msec, const char *, ... ); 

    void msgTimed( const char * msg, int t );
    void msgTimedKey( const char * msg, int t );
    void msgBlocking( const char * msg );
    void msgYesNo( const char * msg );
    void msgYesKey( const char * msg );
}; // timerHandler_t



/******************************************************************************
 *
 * editable_t   -- editable field
 *
 ******************************************************************************/
struct editable_t
{
    int loc[2]; // x,y
    char * buf;
    int buflen;
    int place;
    int active;
    chtype color, curs_color;
    chtype active_color;
    
    editable_t();
    editable_t(int x, int y) ;
    ~editable_t() ;

    void erase() ;

    void setActive(int set =1) { active = set; }

    // returns 1 onchange, 0 no change
    int keyPress( int k ) ;

    void draw( WINDOW * w =0 ) ;
    void setColor( chtype c ) { color = c; }
    void setCursorColor( chtype c ) { curs_color = c; }
    void setActiveColor( chtype c ) { active_color = c; }
}; // editable_t


/******************************************************************************
 *
 * searchableResult_t   -- 
 *
 ******************************************************************************/
class searchableResult_t : public result_t
{
    DBResult * res;
    cppbuffer_t<DBRow*> rows;

public:
    searchableResult_t() : res(0)
    { }
    searchableResult_t(DBResult * _r)
    { setResult(_r); }

    virtual DBRow & operator[] ( unsigned int );
    virtual unsigned int numRows();

    void updateFilter( basicString_t& );
    void setResult(DBResult* _res);

    ~searchableResult_t()
    { }

    void restoreResult();
}; // searchableResult_t


/******************************************************************************
 *
 * ncurses_basic_setup_t
 *
 *  static class, helper methods & boilerplate only
 *
 ******************************************************************************/
class ncurses_basic_setup_t
{
public:
    // sets the size of the message area; needed by all classes to
    //  determine the size of their top area partitions
    // is const static to unify presentation across different, unconnected
    //  display routines
    static const unsigned int FIRST_DISPLAY_ROW ;           
    static void stop_curses();

    static void erase_screen_brute_force( WINDOW *, int );
    static void erase_win_brute_force( WINDOW * );

    static void top_border(WINDOW*,int,int);
    static void bot_border(WINDOW*,int,int);
    static void left_border(WINDOW*,int,int,int);
    static void right_border(WINDOW*,int,int,int);


    static void start_curses();

    // static, just helper functions
    static void draw_box( int, int , int, int, WINDOW * );
    static void horz_divider( int, int, int, WINDOW * );

}; // ncurses_basic_setup_t


/******************************************************************************
 *
 * curses_msgbox_t
 *
 * DEPRECATED
 *  each class has their own instance
 *  
 ******************************************************************************/
class curses_msgbox_t : public ncurses_basic_setup_t
{
protected:

    stringbuffer_t messages;

    WINDOW * msgbox_pane;
    WINDOW * help_pane;

    int lr_padding;
    int tb_padding;
    utimer_t msg_timer;
    bool msg_blocking;


    void push_msg( const char * fmt, ... );
    void push_msg_immed_timed( int, const char * fmt, ... );

    void draw_msgbox(); 
    void cancel_timed();

    virtual void shutdown();
    void msgbox_init();

    void clear_messages();

    virtual void help_menu();


public:

    void redraw_msgbox(); 

    curses_msgbox_t();

    ~curses_msgbox_t() {
        shutdown();
    }

    // blocks for any key press
    void push_msg_blocking( const char * fmt, ... );

}; //curses_msgbox_t


/******************************************************************************
 *
 * cursesFeedView_t
 *
 ******************************************************************************/
class cursesFeedView_t : public curses_msgbox_t
{
    cppbuffer_t<ItemResult*> results; // individual feed result pointers
    //cppbuffer_t<ItemResult**> filtres; // points to results pointers, rebuilt when search filter is run

    result_t * res;
    unsigned int cursor_row, rows_shift;
    editable_t field;
    bool in_search_field;

    void print_rows();

    void draw_feed();

    void key_down( int mv, unsigned int rows_sz );

    void key_up( int mv );

    void show_feed_info();

    void selection();
    void updateSearchField();

public:

    cursesFeedView_t();
    cursesFeedView_t(result_t *);

    ~cursesFeedView_t();

    void setResult( result_t * );

    void run( result_t * res );

    void shutdown();

};// cursesFeedView_t


/******************************************************************************
 *
 * cursesSlideShow_t
 *
 ******************************************************************************/
class cursesSlideShow_t : public curses_msgbox_t
{
    result_t * res; // either DBResult || ItemResult
    unsigned int tot_items;

    unsigned int cur_item;
    WINDOW * text_pane;
    WINDOW * item_pane;

    int ss_delay;
    unsigned int content_line_ofst; // for scrolling

    int c_lr_pad; // content-area padding
    int c_tb_pad;

    bool slide_change_stops_slideshow;

    // toggle modes
    int in_itemView;

    // item view mode
    unsigned int num_view_items;
    unsigned int item_height;
    unsigned int remainder_item_height;
    unsigned int item_justification; // 0 = top-just; 1 = bot-just
    unsigned int cursor_row;
    bool bookmark_mode;

    cppbuffer_t<WINDOW *> item_win;
    // slide content may get optionally: html removed, utf8 removed, 
    HtmlTagStripper detagger;
    // and then array of line pointers built out of processed content
    lineScroller_t linePointers;
    basicString_t content_processed;
    void process_content( basicString_t& input );

    // key handler
    void key_left( int mv );
    void key_right( int mv );
    void key_up( int mv );
    void key_down( int mv );

    void print_item_slide();

    void enter();

    void toggleSlideShow();
    void inc_timer();
    void dec_timer();

    // slideshow manip
    void ss_scroll_down( unsigned int );
    void ss_scroll_up( unsigned int );
    void ss_next_item( int );
    void ss_prev_item( int );
    
    void get_and_display_item_url();
    void mark_item();
    //void help_menu();
    void open_in_lynx();
    void open_in_browser();
    void open_media_url();
    void run_bookmarklet( bookmarklet_enum_t );

    void set_cursor();

    void draw_itemView();
    void draw_slideView();
    void setupItemWindows();

    void flipJustification();
    void delete_item();

public:


    void draw_slide();

    void next_slide();

    cursesSlideShow_t() : res(0), tot_items(0), cur_item(0), text_pane(0), 
                        item_pane(0), ss_delay(6), content_line_ofst(0), c_lr_pad(2), 
                        c_tb_pad(0), slide_change_stops_slideshow(false), in_itemView(1),
                        num_view_items(0), item_height(0), remainder_item_height(0),
                        item_justification(0), cursor_row(0), bookmark_mode(false)
    { }
    
    ~cursesSlideShow_t();

    void run( result_t * , unsigned int =0 );

    void shutdown();

    void setResult( ItemResult * r ) { res = r; }

    void itemViewMode( int set =1 ) { in_itemView = set; }
    void bookmarkMode( bool set =true ) { bookmark_mode = set; }

}; // cursesSlideShow_t


/******************************************************************************
 *
 * cursesMenu_t
 *
 ******************************************************************************/
class cursesMenu_t : public curses_msgbox_t
{
    struct menuItem_t
    {
        basicString_t label;
        void (cursesMenu_t::*run)( void );

        menuItem_t() : run(0)
        { }
    };

    void _menu_show_feeds_f( void );
    void _menu_all_posts_f( void );
    void _menu_posts_constrained_f( void );
    void _menu_podcasts_f( void );
    void _menu_default_query_f( void );
    void _menu_edit_stored_queries_f( void );
    void _menu_unread_items_f( void );
    void _menu_bookmarked_items_f( void );
    void _menu_search_f( void );
    void _menu_last_update_f( void );

    cursesSlideShow_t all_posts_slideshow;
    cursesSlideShow_t some_posts_slideshow;
    cursesSlideShow_t podcast_itemView;
    cursesFeedView_t feedview;
    ItemResult allitem_res;
    ItemResult someitem_res;
    cppbuffer_t<menuItem_t> menu_items;

    unsigned int cursor_row, rows_shift;
    DBResult * feedmenu_res;
    ItemResult podcasts_res;
    ItemResult last_update_res;
    cursesSlideShow_t last_update_itemView;
    int run_state;
    int item_skip;


    void up_cmd();
    void down_cmd();
    void enter_key();


    void draw_menu();
    void initialize_menu();
    void escape_screen();

    void execute_run_state();

    // run states
    enum {
        RUN_NOT_SET,
        RUN_FEEDVIEW,
        RUN_LAST_UPDATE,
        RUN_ALL_ITEMS,
        RUN_BOOKMARKS,
        RUN_PODCASTS,
        RUN_SOME_ITEMS_CONSTRAINED
    };

    WINDOW * msg_win;
    DBResult * bookmark_res;
    basicString_t feed_constraint;
    basicString_t constraint_range;

public:

    cursesMenu_t() : cursor_row(0), rows_shift(0), feedmenu_res(0), 
                    run_state(RUN_NOT_SET), item_skip(0), msg_win(0), bookmark_res(0)
    { }
    ~cursesMenu_t();


    void shutdown();

    // not impl yet
    void startProgressBar( const char * );
    void updateProgressBar( int );


    // entry point
    void run();

    // set semaphore to start specific subroutine
    void setFeedView() { run_state = RUN_FEEDVIEW; }
    void setFeedView( const char *cp ) { run_state = RUN_FEEDVIEW; feed_constraint = cp; }
    void setLastUpdate() { run_state = RUN_LAST_UPDATE; }
    void setAllItems( int skip=0 ) { run_state = RUN_ALL_ITEMS; item_skip = skip; }
    void setAllItems( const char *cp, const char *r ) { 
        run_state = RUN_SOME_ITEMS_CONSTRAINED; 
        feed_constraint = cp; 
        constraint_range = r;
    }
    void setBookmarks() { run_state = RUN_BOOKMARKS; }
    void setPodcasts() { run_state = RUN_PODCASTS; }
}; //cursesMenu_t



#endif /* __CURSEVIEW_H__ */

