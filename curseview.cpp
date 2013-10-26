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

#include <stdlib.h>
#include <curses.h>
#include <signal.h>
#include <unistd.h>
#include <limits.h>

#include "dba_sqlite.h"
#include "misc.h"
#include "ftimer.h"
#include "curseview.h"

extern DBSqlite DBA;
const char * __DontGetIfNotExist( const char * s, DBRow * R );
extern basicString_t browser_path;
extern basicString_t text_browser_path;
const char * javascript_now();
const char * get_last_forwarded_url( const char * uri );
int get_url_with_curl( const char * url, basicString_t& returnData, bool =true );
basicString_t squish_sqldate_us( basicString_t& date );
extern int highest_feed_id();
extern const char * podcast_detection_types[];
basicString_t squish_sqldate_us( basicString_t& date );
const char * last_report_ids();
extern bool config_strip_html_on;
int rss_mark( int, int, const char *, const char * , const char * );
extern basicString_t instapaper_username;
extern basicString_t instapaper_password;
extern basicString_t config_path;
extern int slideshow_speed;
extern int rss_deleteBookmark( int );
extern int rss_deleteItem( int );


// javascript:q=location.href;p=document.title;void(t=open('https://pinboard.in/add?later=yes&noui=yes&jump=close&url='+encodeURIComponent(q)+'&title='+encodeURIComponent(p),'Pinboard','toolbar=no,width=100,height=100'));t.blur();
// javascript:q=location.href;if(document.getSelection){d=document.getSelection();}else{d='';};p=document.title;void(open('https://pinboard.in/add?url='+encodeURIComponent(q)+'&description='+encodeURIComponent(d)+'&title='+encodeURIComponent(p),'Pinboard','toolbar=no,width=700,height=350'));
// javascript:if(document.getSelection){s=document.getSelection();}else{s='';};document.location='https://pinboard.in/add?next=same&url='+encodeURIComponent(location.href)+'&description='+encodeURIComponent(s)+'&title='+encodeURIComponent(document.title)
//basicString_t pinboard( "https://pinboard.in/add?url=%_HREF_%&title=%_TITLE_%" );
basicString_t pinboard( "https://pinboard.in/add?url=%_HREF_%&title=%_TITLE_%&noui=yes&jump=close" );

// javascript:function iprl5(){var d=document,z=d.createElement('scr'+'ipt'),b=d.body,l=d.location;try{if(!b)throw(0);d.title='(Saving...) '+d.title;z.setAttribute('src',l.protocol+'//www.instapaper.com/j/oX2zmk0unU1o?u='+encodeURIComponent(l.href)+'&t='+(new Date().getTime()));b.appendChild(z);}catch(e){alert('Please wait until the page has loaded.');}}iprl5();void(0)" );
basicString_t instapaper( "http://www.instapaper.com/api/add?url=%_HREF_%&title=%_TITLE_%&username=%_USERNAME_%&password=%_PASSWORD_%&redirect=close" );


bookmarklet_t bookmarklets[] = {
{ BOOKMARKLET_PINBOARD, pinboard.str },
{ BOOKMARKLET_INSTAPAPER, instapaper.str },
{ BOOKMARKLET_NONE, 0 }
};


static void sig_catch( int signo ) {
    endwin();
    const char *sig = 0;
    char buf[20];
    switch(signo) {
    case 1: sig = "SIGHUP (Term) Hangup detected on controlling terminal or death of controlling process"; break;
    case 2: sig = 
       "SIGINT        2       (Term)    Interrupt from keyboard";break;
    case 3: sig = 
       "SIGQUIT       3       (Core)    Quit from keyboard";break;
    case 4: sig =
       "SIGILL        4       (Core)    Illegal Instruction";break;
    case 6: sig =
       "SIGABRT       6       (Core)    Abort signal from abort(3)";break;
    case 8: sig =
       "SIGFPE        8       (Core)    Floating point exception";break;
    case 9: sig =
       "SIGKILL       9       (Term)    Kill signal";break;
    case 11: sig =
       "SIGSEGV      11       (Core)    Invalid memory reference";break;
    default: 
        sprintf(buf, "%d",signo);
        sig = buf;
        break;
    }

printf( "in sig_catch. you must've done something stupid to get here. The death causing signal is: %s\n", sig );
fflush(stdout);
    exit(EXIT_SUCCESS);
}

void atexit_catch( void ) {
    if ( isendwin() )
        return;
    endwin();
}


// globals, variable, static components of ncurses_basic_setup
WINDOW * root_window = 0;
// sets size of message area by specifying first row to draw feed results
const unsigned int ncurses_basic_setup_t::FIRST_DISPLAY_ROW = 7;


// globals to use SIGALRM 
cursesSlideShow_t * sshow_p = 0;
int _delay = 0;

static void slideshow_alrm( int signo ) 
{
    if ( sshow_p ) {
        sshow_p->next_slide();
        sshow_p->draw_slide();
    }

    alarm( _delay ); // _delay now turns slideshow on/off
}

static void redraw_alrm( int signo )
{
    // redraw slide
    if ( sshow_p ) {
        sshow_p->draw_slide();
    }

    // was in slide show, restart it
    if ( _delay ) {
        signal( SIGALRM, slideshow_alrm );
        alarm( _delay ); //
    }
}


// each time KEY_UP | KEY_DOWN is pressed, if timer is off, it is started, 
// else, if a repeated key press exceeds this many usec between, rep_cnt is reset
// else rep_cnt is incremented until rep_max is reached, rep_lines[rep_cnt] are scrolled, 
// continue to scroll rep_max until timer check fails, or any other key is pressed
//
// returns num lines to scroll
unsigned int key_accelerator( int key, int& last_key, int& cnt, utimer_t& timer )
{
    // max usec between key inputs to continue repeat 
    long int rtime = 133333; 
    // scrolls up to this many lines, must match high value of rep_lines[]
    unsigned int rep_max = 18; 
    // scrolling schedule for repeated key-inputs (key held down) 
    unsigned int rep_lines[] = { 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 4, 4, 6, 10, 14, 18, 0 }; 

    // start timer from beginning
    if ( timer.timeup() || key != last_key ) 
    {
        timer.set( rtime ); 
        last_key = key;
        cnt = 0;
        return rep_lines[cnt];
    }

    timer.set( rtime );

    if ( rep_lines[cnt] == rep_max )
    {
        return rep_max;
    }

    return rep_lines[++cnt];
}
    

/********************************************************
 * editable field
 ********************************************************/
editable_t::editable_t() 
{
    loc[0] = loc[1] = 0;
    buf = (char*)malloc(100*sizeof(char));
    memset(buf, 0, 100*sizeof(char));
    buflen = 100;
    place = 0;
    active = 0;
    color = active_color = COLOR_PAIR(1); // just hope these are sane
    curs_color = 0;
}

editable_t::editable_t(int x, int y) 
{
    loc[0] = x;
    loc[1] = y;
    buf = (char*)malloc(100*sizeof(char));
    memset( buf, 0, 100 * sizeof(char) );
    buflen = 100;
    place = 0;
    active = 0;
    color = active_color = COLOR_PAIR(1); // just hope these are sane
    curs_color = 0; // will not display cursor if left set to 0
}

editable_t::~editable_t() {
    free( buf );
}

void editable_t::erase() {
    memset(buf, 0, sizeof(char)*buflen);
    place = 0;
}

// returns 1 onchange, 0 no change
int editable_t::keyPress( int k ) 
{
    if ( KEY_BACKSPACE == k || k == 8 || k == KEY_DC || k == 127 ) {
        if ( 0 == place )
            return 0;
        buf[--place] = 0;
        return 1;
    }
    else if ( k >= 32 && k <= 126 )
    {
        if ( place == buflen-1 )
            return 0;
        buf[place++] = k;
        buf[place] = 0;
        return 1;
    }
    return 0;
}

void editable_t::draw( WINDOW * w ) 
{
    if ( !w )
        w = stdscr;

    // get window current color so we can restore it
    attr_t dummy;
    short orig_color;
    wattr_get(w, &dummy, &orig_color, 0);

    if ( active ) 
        wattron( w, active_color );
    else
        wattron( w, color );

    mvwprintw( w, loc[1], loc[0], "%s", buf );

    if ( active && curs_color ) {
        // draw cursor
        wattron( w, curs_color );
        wattron( w, A_ALTCHARSET );
        mvwprintw( w, loc[1], loc[0]+place, "%c", 'a' );
        wattroff( w, A_ALTCHARSET );
    }

    wcolor_set( w, orig_color, 0 );
}

/********************************************************
 * searchableResult
 ********************************************************/
DBRow & searchableResult_t::operator[] ( unsigned int i )
{
    if ( i >= rows.count() )
        return *rows[0];
    return *rows[i];
}

unsigned int searchableResult_t::numRows()
{
    return rows.count();
}

// re-index rows, filtering by string, case-insenstitive
void searchableResult_t::updateFilter( basicString_t& filter )
{
    rows.reset();
    res->resetRowCount();
    basicString_t title;

    DBRow * row;
    while ( (row = res->NextRow()) )
    {
        DBValue * v = row->FindByName( "title" );
        const char *_title = v && v->getString() ? v->getString() : 0;
        if ( _title ) {
            title = _title;
            if ( title.stristr( filter.str ) )
                rows.push_back( row );        
        }
    }
}

// index rows
void searchableResult_t::setResult(DBResult* _res)
{
    // FIXME: handle empty or null results correctly
    Assert ( _res && _res->numRows() && "set to an empty result" );
        
    rows.reset();
    res = _res;
    res->resetRowCount();

    DBRow * row;
    while ( (row = res->NextRow()) )
    {
        rows.push_back( row );        
    }
}

void searchableResult_t::restoreResult()
{
    if ( !res )
        return;
    rows.reset();
    res->resetRowCount();
    DBRow * row;
    while ( (row = res->NextRow()) )
    {
        rows.push_back( row );        
    }
}

/***********************************************************************************************
 *
 *
 *      ncurses_basic_setup_t
 *
 *          - static class, helper methods
 *
 **********************************************************************************************/

void ncurses_basic_setup_t::erase_screen_brute_force( WINDOW * w =0, int rows =0 )
{
    basicString_t line;
    for ( int i = 0 ; i < COLS; i++ ) {
        line.append( " ", 1 );
    }

    // if rows is given, dont do root_window
    if ( !rows ) {
        for ( int j = 0 ; j < LINES; j++ ) {
            mvwprintw( root_window, j, 0, "%s", line.str );
        }
        wrefresh( root_window );
    }

    // if window is not given, dont do sub window
    if ( !w )
        return;

    int subwin_rows = rows ? rows : LINES;

    for ( int j = 0 ; j < subwin_rows; j++ ) {
        mvwprintw( w, j, 0, "%s", line.str );
    }
    wrefresh( w );
}

void ncurses_basic_setup_t::erase_win_brute_force( WINDOW * w )
{
    int row, col;
    getmaxyx( w, row, col ); 

    static basicString_t line;
    for ( int i = 0 ; !line.length() && i < col; i++ ) {
        line.append( " ", 1 );
    }
    
    for ( int j = 0 ; j < row; j++ ) {
        mvwprintw( w, j, 0, "%s", line.str );
    }
    wrefresh( w );
}

void ncurses_basic_setup_t::top_border( WINDOW * win =0, int height =0, int width =COLS )
{
    if ( !win )
        win = stdscr;

    /* j: ┘ k: ┐ l: ┌ m: └ q: ─ t: ├ u: ┤ v: ┴ w: ┬ x: │ */
    attron( A_ALTCHARSET );
    mvwprintw( win, height, 0, "%c", 'l' );
    mvwprintw( win, height, width-1, "%c", 'k' );
    for ( int i = 1; i < width-1; i++ ) {
        mvwprintw( win, height, i, "%c", 'q' );
    }
    attroff( A_ALTCHARSET );
}

void ncurses_basic_setup_t::bot_border( WINDOW * win =0, int height =LINES-1, int width =COLS )
{
    if ( !win )
        win = stdscr;

    /* j: ┘ k: ┐ l: ┌ m: └ q: ─ t: ├ u: ┤ v: ┴ w: ┬ x: │ */
    attron( A_ALTCHARSET );
    mvwprintw( win, height, 0, "%c", 'm' );
    mvwprintw( win, height, width-1, "%c", 'j' );
    for ( int i = 1; i < width-1; i++ ) {
        mvwprintw( win, height, i, "%c", 'q' );
    }
    attroff( A_ALTCHARSET );
}

void ncurses_basic_setup_t::left_border( WINDOW * win =0, int height =LINES, int col =0, int start =1 )
{
    if ( !win )
        win = stdscr;

    /* j: ┘ k: ┐ l: ┌ m: └ q: ─ t: ├ u: ┤ v: ┴ w: ┬ x: │ */
    attron( A_ALTCHARSET );
    mvwprintw( win, 0,        col, "%c", 'l' );
    mvwprintw( win, height-1, col, "%c", 'm' );
    for ( int i = start; i < height-1; i++ ) {
        mvwprintw( win, i, col, "%c", 'x' );
    }
    attroff( A_ALTCHARSET );
}

void ncurses_basic_setup_t::right_border( WINDOW * win =0, int height =LINES, int col =COLS-1, int start =1 )
{
    if ( !win )
        win = stdscr;

    /* j: ┘ k: ┐ l: ┌ m: └ q: ─ t: ├ u: ┤ v: ┴ w: ┬ x: │ */
    attron( A_ALTCHARSET );
    mvwprintw( win, 0,        col, "%c", 'k' );
    mvwprintw( win, height-1, col, "%c", 'j' );
    for ( int i = start; i < height-1; i++ ) {
        mvwprintw( win, i, col, "%c", 'x' );
    }
    attroff( A_ALTCHARSET );
}



void ncurses_basic_setup_t::stop_curses()
{
    if ( ! isendwin() )
        endwin();

    if ( root_window )
        delwin( root_window );
    root_window = 0;
}

void ncurses_basic_setup_t::start_curses()
{
    if ( root_window )
        return;

    //term = set_term( 0 );

    /* Initialize curses */
    root_window = initscr();

    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set( 0 );

    if ( !getenv("ESCDELAY") )
        set_escdelay(15);

/*  COLOR_BLACK
    COLOR_RED
    COLOR_GREEN
    COLOR_YELLOW
    COLOR_BLUE
    COLOR_MAGENTA
    COLOR_CYAN
    COLOR_WHITE */
    init_pair( 1, COLOR_RED, COLOR_BLACK);
    init_pair( 2, COLOR_MAGENTA, COLOR_BLACK);
    init_pair( 3, COLOR_GREEN, COLOR_BLACK );
    init_pair( 4, COLOR_BLACK, COLOR_GREEN );
    init_pair( 5, COLOR_BLUE, COLOR_BLACK );
    init_pair( 6, COLOR_BLACK, COLOR_YELLOW );
    init_pair( 7, COLOR_WHITE, COLOR_BLACK );
    init_pair( 8, COLOR_BLACK, COLOR_BLUE );
    init_pair( 9, COLOR_CYAN, COLOR_BLACK );
    init_pair( 10, COLOR_BLACK, COLOR_WHITE );
    init_pair( 11, COLOR_WHITE, COLOR_WHITE );
    init_pair( 12, COLOR_BLUE, COLOR_BLUE );
    init_pair( 13, COLOR_YELLOW, COLOR_BLACK );
    init_pair( 14, COLOR_CYAN, COLOR_CYAN );
    init_pair( 15, COLOR_GREEN, COLOR_GREEN );
    init_pair( 16, COLOR_WHITE, COLOR_RED ); // '/' search color

    // initial
    attron( COLOR_PAIR( 3 ) );

    // catch unexpected exits to ensure a hopefully clean shutdown
    signal( SIGHUP, sig_catch ) ;
    signal( SIGINT, sig_catch ) ;
    signal( SIGQUIT, sig_catch ) ;
    signal( SIGILL, sig_catch ) ;
    signal( SIGABRT, sig_catch ) ;
    signal( SIGFPE, sig_catch ) ;
    signal( SIGKILL, sig_catch ) ;
    signal( SIGSEGV, sig_catch ) ;
    atexit( atexit_catch );
};

void ncurses_basic_setup_t::draw_box( int start_x, int start_y, int cols, int rows, WINDOW * win =0 )
{
    if ( !win )
        win = root_window;

    /* j: ┘ k: ┐ l: ┌ m: └ q: ─ t: ├ u: ┤ v: ┴ w: ┬ x: │ */
    attron( A_ALTCHARSET );

    int i = start_x;
    int j = start_y;

    for ( i = start_x+1; i < start_x + cols; i++ )
    {
        mvwprintw( win, start_y, i, "%c", 'q' );
        mvwprintw( win, start_y+rows-1, i, "%c", 'q' );
    }
    for ( j = start_y+1; j < start_y + rows; j++ )
    {
        mvwprintw( win, j, start_x, "%c", 'x' );
        mvwprintw( win, j, start_x+cols-1, "%c", 'x' );
    }
    mvwprintw( win, start_y, start_x, "%c", 'l' );
    mvwprintw( win, start_y+rows-1, start_x, "%c", 'm' );
    mvwprintw( win, start_y, start_x+cols-1, "%c", 'k' );
    mvwprintw( win, start_y+rows-1, start_x+cols-1, "%c", 'j' );

    attroff( A_ALTCHARSET );
}

void ncurses_basic_setup_t::horz_divider( int start_x, int cols, int height, WINDOW * win =0 )
{
    if ( !win )
        win = root_window;
    int i;
    attron( A_ALTCHARSET );
    for ( i = start_x+1; i < start_x + cols-1; i++ )
    {
        mvprintw( height, i, "%c", 'q' );
    }
    mvwprintw( win, height, start_x, "%c", 't' );
    mvwprintw( win, height, start_x+cols-1, "%c", 'u' );
    attroff( A_ALTCHARSET );
}


/***********************************************************************************************
 *
 *
 *      curses_msgbox_t
 *
 *
 **********************************************************************************************/
curses_msgbox_t::curses_msgbox_t() : msgbox_pane(0), help_pane(0), lr_padding(0), tb_padding(0), msg_blocking(false)
{
}

void curses_msgbox_t::msgbox_init() 
{
    if ( !msgbox_pane ) {
        msgbox_pane = newwin( FIRST_DISPLAY_ROW-2-2*tb_padding, COLS-2-2*lr_padding, 1+tb_padding, 1+lr_padding );
    }
}

void curses_msgbox_t::shutdown()
{
    if ( msgbox_pane )
        delwin( msgbox_pane );
    msgbox_pane = 0;
}

void curses_msgbox_t::push_msg( const char * fmt, ... )
{
    char buf[ _VA_BUF_SZ ];
    char * buffer = &buf[0];
    va_list argptr;
    va_start( argptr, fmt );
    int len_actual = vsnprintf( buffer, sizeof( buf ), fmt, argptr );
    va_end( argptr );

    basicString_t *str_p = new basicString_t;

    if ( len_actual >= _VA_BUF_SZ )
    {  
        buffer = (char*) malloc( len_actual+1 );
        va_start( argptr, fmt );
        vsnprintf( buffer, len_actual+1, fmt, argptr );
        va_end( argptr );
        str_p->set( buffer );
        free( buffer );
    }
    else
        str_p->set( buffer );

    messages.push_back( str_p );
}

void curses_msgbox_t::push_msg_immed_timed( int sec, const char * fmt, ... )
{
    /////////////////////////////////////////
    // dupe code of push_msg
    char buf[ _VA_BUF_SZ ];
    char * buffer = &buf[0];
    va_list argptr;
    va_start( argptr, fmt );
    int len_actual = vsnprintf( buffer, sizeof( buf ), fmt, argptr );
    va_end( argptr );

    basicString_t *str_p = new basicString_t;

    if ( len_actual >= _VA_BUF_SZ )
    {  
        buffer = (char*) malloc( len_actual+1 );
        va_start( argptr, fmt );
        vsnprintf( buffer, len_actual+1, fmt, argptr );
        va_end( argptr );
        str_p->set( buffer );
        free( buffer );
    }
    else
        str_p->set( buffer );

    messages.push_back( str_p );
    /////////////////////////////////////////


    werase(msgbox_pane);
    draw_msgbox();
    wrefresh(msgbox_pane);

    // will dispaly msg for a time, then redraw normally, erasing msgbox
    signal( SIGALRM, redraw_alrm );
    alarm( sec );

    // adjust _delay, to difference, if slideshow has been running
    if ( _delay ) {
        _delay = _delay - sec;
        if ( _delay < 1 ) _delay = 1;
    }

    // this sets a timer that tells draw_slide what to draw
    msg_timer.set( sec * 1000000 );
}

void curses_msgbox_t::push_msg_blocking( const char *fmt, ... )
{
    /////////////////////////////////////////
    // dupe code of push_msg
    char buf[ _VA_BUF_SZ ];
    char * buffer = &buf[0];
    va_list argptr;
    va_start( argptr, fmt );
    int len_actual = vsnprintf( buffer, sizeof( buf ), fmt, argptr );
    va_end( argptr );

    basicString_t *str_p = new basicString_t;

    if ( len_actual >= _VA_BUF_SZ )
    {  
        buffer = (char*) malloc( len_actual+1 );
        va_start( argptr, fmt );
        vsnprintf( buffer, len_actual+1, fmt, argptr );
        va_end( argptr );
        str_p->set( buffer );
        free( buffer );
    }
    else
        str_p->set( buffer );

    messages.push_back( str_p );
    /////////////////////////////////////////

    werase(msgbox_pane);
    draw_msgbox();
    wrefresh(msgbox_pane);
    alarm(0);

    // block for a day
    msg_timer.set( LONG_MAX );

    // tells input loop to eat next input and reset timer
    msg_blocking = true;
}


void curses_msgbox_t::cancel_timed()
{
    alarm(0);
}

void curses_msgbox_t::draw_msgbox()
{
    attron( COLOR_PAIR( 3 ) ); // green border

    // draw boxes in the main window. Sub-windows are for containing inline text only,
    // subwins cause correctly line-break behavior
    // -looks like it's actually drawing to FIRST_DISPLAY_ROW-1 !
    //draw_box( 0, 0, COLS, FIRST_DISPLAY_ROW, root_window );

    if ( messages.length() > 0 ) 
    {
        wattron( msgbox_pane, COLOR_PAIR( 7 ) ); // white

        // print last four messages
        unsigned int start = messages.length() >= FIRST_DISPLAY_ROW-2 ? messages.length()-(FIRST_DISPLAY_ROW-2) : 0;

        for ( unsigned int i = 0; i < FIRST_DISPLAY_ROW-1 && start + i < messages.length(); i++ )
        {
            if ( 0 == i && messages[ start + i ]->strstr( " feed        :") ) { // hack!
                wattron( msgbox_pane, COLOR_PAIR( 2 ) ); 
            } else if ( 3 == i && messages[ start + i ]->strstr( " description :") ) { // hack!
                wattron( msgbox_pane, COLOR_PAIR( 9 ) ); 
            } else {
                wattron( msgbox_pane, COLOR_PAIR( 7 ) ); 
            }
            mvwprintw( msgbox_pane, i, 0, "%s", messages[ start + i ]->str );
        }
    }

#if 0
    // green line
    attron( COLOR_PAIR( 6 ) );
    for ( int j = 0; j < COLS; j++ ) 
        mvprintw( FIRST_DISPLAY_ROW-1, j, " " );
#endif
}

void curses_msgbox_t::clear_messages()
{
    messages.erase();
}

void curses_msgbox_t::redraw_msgbox()
{
    werase(msgbox_pane);
    erase_screen_brute_force( msgbox_pane, FIRST_DISPLAY_ROW-2-2*tb_padding );
    draw_msgbox();
    wrefresh(msgbox_pane);
}



const char * help_list[] = {
" <<-- RSS Power Tool -->>"," ", 
"o      [o]pen item in browser",
"m      open [m]edia url in browser",
"l      open page in [l]ynx",
"p      [p]inboard it",
"i      [i]nstapaper it",
"b      [b]ookmark this item for later",
"r      [r]emove bookmark",
"d/g    [d]ownload/[g]et a page and display contents in main window",
"space  start/stop slideshow",
"enter  switch between single-page mode and item-view mode",
"+/-    adjust slideshow speed",
"0-9    set slideshow speed to this many seconds",
"arrow  left/right/up/down - change slide, or scroll text",
"q/esc  exit",
"/      search feeds",
"?/h    this menu",
0 };

void curses_msgbox_t::help_menu()
{
    const char ** s;
    int j;
    if ( ! help_pane )
    {
        j = 0;
        s = help_list;
        while(*s++)
            ++j;
        
        help_pane = newwin( j+2, COLS - 4, 1, 2 );
    }

    WINDOW * w = help_pane;
    werase(w);
    wattron(w,COLOR_PAIR(3));
    box(w,0,0);

    wattron(w,COLOR_PAIR(7));
    j = 0;
    s = help_list; 
    while(*s)
        mvwprintw(w, ++j, 2, " %s", *s++ ); 

    // turn off any timed events
    alarm(0);

    // block
    msg_blocking = true; 
}


/***********************************************************************************************
 *
 *
 *      cursesFeedView_t
 *
 *
 **********************************************************************************************/

void cursesFeedView_t::shutdown() 
{
    curses_msgbox_t::shutdown();
}

cursesFeedView_t::cursesFeedView_t() : res(0), cursor_row(0), rows_shift(0)
{
}

cursesFeedView_t::cursesFeedView_t( result_t * res ) 
                    : res(0), cursor_row(0), rows_shift(0)
{
    setResult( res );
}

void cursesFeedView_t::setResult( result_t * _res ) 
{
    if ( res == _res ) 
        return;

    // previous set extant, needs delete
    if ( _res ) {
        for ( unsigned int i = 0; i < results.length(); i++ ) {
            if ( results[i] ) {
                delete results[i];
            }
        }
    }

    // make sure set this
    res = _res;

    // result pointers, 0 tells us we need to allocate
    for ( unsigned int i = 0; i < res->numRows(); i++ ) {
        results[i] = 0; 
        //filtres[i] = &(results[i]); // these point to these
    }
}

cursesFeedView_t::~cursesFeedView_t()
{
    for ( unsigned int i = 0; i < results.length(); i++ ) {
        if ( results[i] ) {
            delete results[i];
            results[i] = 0;
        }
    }
    shutdown();
}

void cursesFeedView_t::print_rows()
{

    unsigned int display_row = FIRST_DISPLAY_ROW;

    int cval = rows_shift & 0x1 ? 3 : 5; // 0b011 | 0b101


    for ( unsigned int i = rows_shift; i < res->numRows(); i++ )
    {
        // alternate color
        attron( COLOR_PAIR( cval ^= 6 ) );

        // highlight cursor
        if ( display_row == FIRST_DISPLAY_ROW + cursor_row-rows_shift )
            attron( COLOR_PAIR( 4 ) );

        // get stuff we want to print
        DBRow & row = (*res)[i]; 
        DBValue * v = row.FindByName( "id" );
        int feed_id = v ? v->getInt() : 0;
        v = row.FindByName( "title" );
        const char * title = v ? v->getString() : 0;
        v = row.FindByName( "last_updated" );
        basicString_t sqldate = v && v->getString() ? v->getString() : 0;
        basicString_t us_date = squish_sqldate_us( sqldate );

        char fid[24];
        sprintf( fid, "[%d]", feed_id );

        mvprintw( display_row++, 0, " %-6.6s %-40.40s   %s", fid, title, us_date.str );

        if ( display_row >= (unsigned) LINES )
            return; // we're done. no point is printing outside of screen boundary
    }
}



void cursesFeedView_t::draw_feed()
{
    // green border
    attron( COLOR_PAIR( 3 ) ); 
    wattron( stdscr, COLOR_PAIR( 3 ) ); 

    // erase
    erase();
    werase( msgbox_pane );
    werase( root_window );
    erase_win_brute_force( msgbox_pane );
    erase_win_brute_force( stdscr );
    //erase_screen_brute_force( msgbox_pane );

    // box around entire screen
    draw_box( 0, 0, COLS, FIRST_DISPLAY_ROW, root_window );

    // feeds
    print_rows();

    // top messages
    draw_msgbox();

    // refresh in this order!
    wrefresh( root_window );
    wrefresh( msgbox_pane );

    if ( msg_blocking ) 
        wrefresh( help_pane );

    if ( in_search_field ) {
        attron( COLOR_PAIR(16) );
        basicString_t line;
        for ( int i = 1 ; i < COLS ; i++ ) 
            line.append( " ", 1 );
        mvprintw( FIRST_DISPLAY_ROW-1, 0, "%s", line.str );
        field.draw();
    }
}


void cursesFeedView_t::show_feed_info()
{
    if ( 0 == res->numRows() )
        return;
    DBRow & row = (*res)[ cursor_row ];

    basicString_t id = __DontGetIfNotExist( "id", &row );
    basicString_t title = __DontGetIfNotExist( "title", &row );
    basicString_t xmlUrl = __DontGetIfNotExist( "xmlUrl", &row );
    basicString_t htmlUrl = __DontGetIfNotExist( "htmlUrl", &row );
    basicString_t description = __DontGetIfNotExist( "description", &row );
    basicString_t last_updated = __DontGetIfNotExist( "last_updated", &row );
    basicString_t type = __DontGetIfNotExist( "type", &row );
    basicString_t timeouts = __DontGetIfNotExist( "timeouts", &row );
    basicString_t errmsg = __DontGetIfNotExist( "errmsg", &row );
    basicString_t disabled = __DontGetIfNotExist( "disabled", &row );

    basicString_t fmt;
    fmt.erase();
    int clip = COLS-2-2*lr_padding-21; 
    fmt.sprintf ( " feed        : [%%s] %%-%d.%ds", clip, clip );
    push_msg( fmt.str, id.str, title.str );

        push_msg( " xmlUrl      : %s", xmlUrl.length() ? xmlUrl.str : "" );

        push_msg( " htmlUrl     : %s", htmlUrl.length() ? htmlUrl.str : "" );

    fmt.erase();
    clip = COLS-2-2*lr_padding-14 ;
    fmt.sprintf ( " description : %%-%d.%ds", clip, clip );
    push_msg( fmt.str, description.length() ? description.str : "" );

    DBValue * v;
    DBResult * R = DBA( fmt.sprintf("select count(item.id) as count from item,item_feeds where item_feeds.item_id = item.id and item_feeds.feed_id = %s;", id.str ).str );
    v = R->FindByNameFirstRow( "count" );
    int num = v ? v->getInt() : 0;
    R = DBA( fmt.sprintf("select sqldate from item,item_feeds where item_feeds.item_id = item.id and item_feeds.feed_id = %s order by sqldate asc limit 1;", id.str ).str );
    v = R->FindByNameFirstRow( "sqldate" );
    basicString_t oldest = v ? v->getString() : 0;
    R = DBA( fmt.sprintf("select sqldate from item,item_feeds where item_feeds.item_id = item.id and item_feeds.feed_id = %s order by sqldate desc limit 1;", id.str ).str );
    v = R->FindByNameFirstRow( "sqldate" );
    basicString_t newest = v ? v->getString() : 0;

    push_msg( " newest      : %s, items: %d, oldest: %s, type: %s, disab: %s, timeouts: %s", squish_sqldate_us(newest).str, num, squish_sqldate_us(oldest).str, type.str, disabled.str, timeouts.str );
}

void cursesFeedView_t::key_down( int mv, unsigned int rows_sz )
{
    // clamp move
    if ( cursor_row + mv >= rows_sz ) {
        mv = (rows_sz-1) - cursor_row;
    }

    // do move
    cursor_row += mv;

    // adjust rowshift
    unsigned int display_rows = LINES - 1 - FIRST_DISPLAY_ROW;
    if ( cursor_row > display_rows + rows_shift )
        rows_shift = cursor_row - display_rows;
}

void cursesFeedView_t::key_up( int mv ) 
{
    int cursor = cursor_row;

    // clip the move
    if ( cursor - mv < 0 ) {
        mv = cursor;
    }

    // set the cursor
    cursor_row -= mv;

    // off the screen?
    if ( cursor_row < rows_shift )
        rows_shift = cursor_row;
}

void cursesFeedView_t::selection()
{
    if ( 0 == res->numRows() )
        return;

    // 
    DBRow & row = (*res)[ cursor_row ];

    DBValue * v = row.FindByName( "title" );
    const char * title = v ? v->getString() : 0;
    v = row.FindByName( "id" );
    int feed_id = v ? v->getInt() : 0;

    push_msg( " " );
    push_msg( " selected:  [%d] %s", feed_id, title );

    //
    // execute slideshow mode
    cursesSlideShow_t sshow;


    // FIXME: use filtres instead
    // if ( ! *filtres[ cursor_row ] ) {
    //     *filtres[ cursor_row ] = new ItemResult( &DBA, 50 );
    //  }
    
    if ( !results[ cursor_row ] ) {
        results[ cursor_row ] = new ItemResult( &DBA, 50 );
    }

    basicString_t buf;

    results[ cursor_row ]->setClause( buf.sprintf("(feed.id = %d)", feed_id).str );
    sshow.run( results[ cursor_row ] );
}

void cursesFeedView_t::updateSearchField()
{
    if ( strlen(field.buf) < 2 )
        return;

    basicString_t filter = &field.buf[1];

    searchableResult_t * sres = dynamic_cast<searchableResult_t*>(res);
    if ( sres ) {
        sres->updateFilter( filter );
    }

    cursor_row = 0; // move to the top every time result set updated
    rows_shift = 0;

    // rebuild filtres instead, and use filtres in selection()
    

    // saved results get nuked by a search
    for ( unsigned int i = 0; i < results.length(); i++ ) {
        if ( results[i] ) {
            delete results[i];
            results[i] = 0;
        }
    }
}

void cursesFeedView_t::run( result_t * res )
{
    if ( !res || res->numRows() == 0 ) {
        printf ( "nothing to display \n" );
        return;
    }

    start_curses();
    msgbox_init();

    setResult(res);
    field.setColor(COLOR_PAIR(16));
    field.loc[0] = 0;
    field.loc[1] = FIRST_DISPLAY_ROW - 1;
    in_search_field = false;


    // key accelerator stuff
    int rep_cnt = 0;  
    utimer_t keydown_timer;
    int rep_key = 0; // KEY_UP, KEY_DOWN are only ones repeated
    int repeater_lines = 1;


    int key = 1000;
    bool done = false;

    // display once before we go into key-loop
    show_feed_info();
    draw_feed();


    while ( !done && (key = getch()) )
    {

        if ( msg_blocking ) {
            msg_timer.reset();
            msg_blocking = false;
            key = 1000;
        }

        if ( KEY_DOWN == key || KEY_UP == key ) 
        {
            repeater_lines = key_accelerator( key, rep_key, rep_cnt, keydown_timer );
        }
        else
        {
            keydown_timer.reset();
            repeater_lines = 1;
        }

        // search-filter result set
        if ( in_search_field ) 
        {
            if ( field.keyPress(key) )
                updateSearchField();

            // 0-length cancels; ESCAPE cancels 
            if ( strlen(field.buf) == 0 || key == 27 ) {
                in_search_field = false;
                key = 1000;
                searchableResult_t * r = dynamic_cast<searchableResult_t*>(res);
                if ( r ) {
                    r->restoreResult();
                }
                field.erase();
            }
        }
        else
        {
            // letters
            switch( key )
            {
            case 'h':
                help_menu();
                break;
            case '?': 
                help_menu();
                break;
            case '/':
                if ( strlen(field.buf) == 0 )
                    field.keyPress('/');
                in_search_field = true;
                break;
            case 'q':
                key = 27; // quit
                break;
            }
        }

        // non-letters
        switch( key )
        {
        case KEY_DOWN:
            key_down( repeater_lines, res->numRows() );
            show_feed_info();
            break;
        case KEY_NPAGE:
            key_down( LINES - 1 - FIRST_DISPLAY_ROW, res->numRows() );
            show_feed_info();
            break;
        case KEY_UP:
            key_up( repeater_lines );
            show_feed_info();
            break;
        case KEY_PPAGE:
            key_up( LINES - 1 - FIRST_DISPLAY_ROW );
            show_feed_info();
            break;
        case KEY_LEFT:
        case KEY_RIGHT:
            break;
        case KEY_ENTER:
        case 10:
            selection();
            break;
        default:
            break;
        }

        if ( key == 27 )
            break;

        draw_feed();
    }
}


/***********************************************************************************************
 *
 *
 *      cursesSlideshow_t
 *
 *
 **********************************************************************************************/

void cursesSlideShow_t::shutdown() 
{
    // item view
    for ( unsigned int i = 0; i < item_win.count(); i++ ) {
        if ( item_win[i] ) { 
            delwin ( item_win[i] );
            item_win[i] = 0;
        }
    }
    item_win.reset();

    // slide view
    if ( text_pane )
        delwin( text_pane );
    text_pane = 0;
    if ( item_pane )
        delwin( item_pane );
    item_pane = 0;

    // 
    curses_msgbox_t::shutdown();
}

cursesSlideShow_t::~cursesSlideShow_t()
{
    shutdown();
}

void cursesSlideShow_t::flipJustification()
{
    if ( 0 == remainder_item_height )
        return;

    // swap the end windows
    WINDOW *& s = item_win[ 0 ];
    WINDOW *& e = item_win[ item_win.count()-1 ];
    WINDOW * T = e;
    e = s;
    s = T;

    // get our new identity
    item_justification ^= 1;

    const unsigned int ROW_HEIGHT = FIRST_DISPLAY_ROW - 1;

    //
    // relocate the windows in the terminal
    //

    // top justified
    if ( !item_justification )
    {
        for ( unsigned int i = 0; i < item_win.count(); i++ ) {
            int begin_y = 1 + ROW_HEIGHT * i;
            int begin_x = 1 + lr_padding;
            mvwin( item_win[i], begin_y, begin_x );
        }
    }

    // bottom justified
    else
    {
        int begin_y = 0;
        int begin_x = 1 + lr_padding;
        mvwin( item_win[0], begin_y, begin_x );

        for ( unsigned int i = 1; i < item_win.count(); i++ ) {
            int begin_y = 1 + remainder_item_height + ROW_HEIGHT * (i-1);
            int begin_x = 1 + lr_padding;
            mvwin( item_win[i], begin_y, begin_x );
        }
    }
}

void cursesSlideShow_t::key_up( int mv ) 
{
    // keys change meaning depending on mode
    if ( !in_itemView ) {
        return ss_scroll_up(mv);
    }

    // in itemView 
    ss_prev_item(1);

    // move the cursor
    int row = cursor_row;
    if ( --row < 0 )
        row = 0;

    // special case, flip item_justification bit
    if ( row == 0 && cursor_row == 1 && item_justification )
        flipJustification();

    cursor_row = row;
}

void cursesSlideShow_t::key_down( int mv ) 
{
    if ( !in_itemView ) {
        return ss_scroll_down(mv);
    }

    // in itemView 
    ss_next_item(1);

    // cursor clamps [0>= X < num_view_items]
    unsigned int row = cursor_row;
    if ( ++row >= num_view_items )
        row = num_view_items - 1;

    // special case, flip align
    if ( row == num_view_items - 1 && cursor_row == num_view_items - 2 && !item_justification )
        flipJustification();

    cursor_row = row;
}

void cursesSlideShow_t::key_left( int mv ) 
{
    if ( !in_itemView ) {
        return ss_prev_item(mv);
    }

    // in itemView, no action
}

void cursesSlideShow_t::key_right( int mv ) 
{
    if ( !in_itemView ) {
        return ss_next_item(mv);
    }

    // in itemView, no action
}

void cursesSlideShow_t::ss_prev_item( int mv ) 
{
    // clamp move
    int cur = cur_item;
    if ( (cur -= mv) < 0 )
        cur_item = tot_items - 1;
    else
        cur_item = cur;

    // reset scroll
    content_line_ofst = 0; 

    // if slideshow in progress
    if ( _delay && slide_change_stops_slideshow ) {
        _delay = 0; // stop slideshow
        push_msg_immed_timed( 1, " key-press detected. stopping slideshow" );
    }

    // nuke content to signal rebuild for next slide
    content_processed.erase();
}

void cursesSlideShow_t::ss_next_item( int mv ) 
{
    // clamp move
    if ( cur_item + mv >= tot_items )
        cur_item = 0;
    else
        cur_item += mv;

    // reset line scroll
    content_line_ofst = 0;

    // if slideshow in progress
    if ( _delay && slide_change_stops_slideshow ) {
        _delay = 0; // stop slideshow
        push_msg_immed_timed( 1, " key-press detected. stopping slideshow" );
    }

    // nuke content to signal rebuild for next slide
    content_processed.erase();
}

void cursesSlideShow_t::next_slide() {
    if ( ++cur_item >= tot_items )
        cur_item = 0;

    content_line_ofst = 0; // reset scroll each time we change slide

    // if sshow is on, reset delay every slide in case it's been altered
    if ( _delay )
        _delay = ss_delay; 

    // signal rebuild
    content_processed.erase();
}



// replaces the content of the slide with whatever is returned by the item_url, filtered
void cursesSlideShow_t::get_and_display_item_url()
{
    DBRow& row = (*res)[ cur_item ];
    DBValue * v = row.FindByName( "item_url" );
    const char * url = v && v->getString() ? v->getString() : 0;
    if ( !url ) {
        push_msg_immed_timed( 1, " couldn't get item url" );
        return;
    }

    basicString_t urlFetch;

    if ( !get_url_with_curl( url, urlFetch ) ) {
        push_msg_immed_timed( 2, " couldn't fetch url: %s", url );
        return;
    }

    process_content( urlFetch );

    clear_messages();
    push_msg(" ");
    push_msg( " fetched: %s" , url );
    push_msg(" ");
    push_msg_immed_timed( 3, " diplaying returned page.." );
    
    // re-render with new stuff
    draw_slide();

    _delay = 0; // stop slideshow when page is fetched by user volition
}

void cursesSlideShow_t::process_content( basicString_t & input )
{
    if ( content_processed.length() )
        content_processed.erase();
    
    // rm all utf8 chars. regular curses hates non-ascii
    strip_utf8_chars( input ); 

    linePointers.setColumnCharWidth( COLS-2-2*c_lr_pad );

    // process html tags: remove, replace, reformat 
    if ( config_strip_html_on )
        detagger.strip( input, content_processed );

    // build pointers from content input
    if ( config_strip_html_on )
        linePointers.process( content_processed );
    else
        linePointers.process( input );
}

void cursesSlideShow_t::print_item_slide()
{
    if ( !res ) {
        push_msg_immed_timed(2, " no DBResult. nothing to display." );
        return;
    }

    attron( COLOR_PAIR( 5 ) ); // blue border

    draw_box( 0, 0, COLS, LINES );
    horz_divider( 0, COLS, FIRST_DISPLAY_ROW-1 );

    //box( item_pane, 0, 0 );//border - use wattron(COLOR_PAIR()) to set color


    attron( COLOR_PAIR( 7 ) ); // white text
    wattron( text_pane, COLOR_PAIR( 7 ) ); // white text
    wattron( item_pane, COLOR_PAIR( 7 ) ); // blue item info
    
    DBRow& row = (*res)[ cur_item ];    

    
    basicString_t feed_id = __DontGetIfNotExist( "feed_id", &row );
    basicString_t ftitle = __DontGetIfNotExist( "ftitle", &row );
    basicString_t title = __DontGetIfNotExist( "title", &row );
    basicString_t date;
    date.strncpy( __DontGetIfNotExist( "sqldate", &row ), 16 );

    basicString_t desc = __DontGetIfNotExist( "description", &row );
    basicString_t content = __DontGetIfNotExist( "content", &row );
    basicString_t A = __DontGetIfNotExist( "author", &row );

    basicString_t media_url = __DontGetIfNotExist( "media_url", &row );
    basicString_t item_url = __DontGetIfNotExist( "item_url", &row );




    char fmt[48];
    unsigned int len = ftitle.length() > 25 ? 25 : ftitle.length() ;
    sprintf( fmt, " %%u/%%u - [%%s] %%-%u.%us - %%s - %%s %%s %%s", len, len );

    if ( msg_timer.timeup() ) 
    {
        strip_utf8_chars( title ); 
        basicString_t title_dehtml;
        detagger.strip( title, title_dehtml );

        mvwprintw( item_pane, 0, 0, fmt, cur_item + 1, res->numRows(), feed_id.str, ftitle.str, date.str, title_dehtml.str, A.length() ? "-":"", A.length() ? A.str : "" );

        if ( media_url.length() )
            mvwprintw( item_pane, 2, 0, " %s", media_url.str );
        if ( item_url.length() )
            mvwprintw( item_pane, 3, 0, " %s", item_url.str );
    }
    else
    {
        draw_msgbox();
    }


    // concat content w/ description
    if ( content.length() ) {
        desc += "<br><br>\nContent:<br>\n";
        desc += content += "<br>\n";
    }

    // there is something to display
    if ( desc.length() ) 
    {
        // first time in this slide, build pointers array to new slide content
        if ( !content_processed.length() )
            process_content( desc );

        mvwprintw( text_pane, 0, 0, "%s", linePointers[content_line_ofst] );

        set_cursor();
    }
}



void cursesSlideShow_t::draw_itemView() 
{
    // erase 
    erase_screen_brute_force();
    erase();
    for ( unsigned int i = 0; i < item_win.count(); i++ ) {
        wattron( item_win[i], COLOR_PAIR( 7 ) );
        werase( item_win[i] );
        erase_win_brute_force( item_win[i] );
    }

    //
    // draw borders - blue border in main win
    //
    attron( COLOR_PAIR( 5 ) ); 

    // top justified
    if ( !item_justification ) 
    {
        left_border(stdscr,LINES+1);
        right_border(stdscr,LINES+1);
        top_border();

        for ( unsigned int i = 1 ; i < item_win.count(); i++ ) 
        {
            unsigned int shift = i * (FIRST_DISPLAY_ROW-1);
            attron( COLOR_PAIR( 5 ) ); 
            horz_divider( 0, COLS, shift );
        }
    }
    // bottom justified
    else 
    {
        left_border(stdscr,LINES,0,0);
        right_border(stdscr,LINES,COLS-1,0);
        bot_border();

        for ( unsigned int i = 1 ; i < item_win.count(); i++ ) 
        {
            unsigned int shift = (LINES-1) - i * (FIRST_DISPLAY_ROW-1);
            attron( COLOR_PAIR( 5 ) ); 
            horz_divider( 0, COLS, shift );
        }
    }


    //
    // cursor border
    //
    //attron( COLOR_PAIR( 9 ) );
    attron( COLOR_PAIR( 2 ) );
    //attron( COLOR_PAIR( 15 ) );
    int j = (FIRST_DISPLAY_ROW-1) * cursor_row;
    int h = FIRST_DISPLAY_ROW;

    // bottom justified is offset
    if ( item_justification ) {
        j = (FIRST_DISPLAY_ROW-1) * (cursor_row-1) + remainder_item_height;
    }
    draw_box( 0, j, COLS, h, root_window );


    //
    unsigned int draw_row = cur_item;

    // adjust the draw_row to the cursor offet so things line up
    if ( cursor_row > 0 ) {

        // if there is no bifurcation on the screen, under the cursor
        int ci = cur_item;
        int cr = cursor_row;
        if ( ci - cr >= 0 ) 
            draw_row -= cursor_row;
        else
        {
            int diff = cr - ci;
            draw_row = ( tot_items - diff ) % tot_items;
        }
    }

    //
    // item text
    //
    for ( unsigned int i = 0 ; i < item_win.count(); i++ )
    {
        WINDOW * win_p = item_win[i];
        DBRow& row = (*res)[ draw_row ];    
        
        basicString_t feed_id = __DontGetIfNotExist( "feed_id", &row );
        basicString_t ftitle = __DontGetIfNotExist( "ftitle", &row );
        basicString_t title = __DontGetIfNotExist( "title", &row );
        basicString_t date;
        date.strncpy( __DontGetIfNotExist( "sqldate", &row ), 16 );
        basicString_t A = __DontGetIfNotExist( "author", &row );
        basicString_t media_url = __DontGetIfNotExist( "media_url", &row );
        basicString_t item_url = __DontGetIfNotExist( "item_url", &row );

        strip_utf8_chars( title ); 
        basicString_t title_dehtml;
        detagger.strip( title, title_dehtml );

        char fmt[48];
        unsigned int len = ftitle.length() > 25 ? 25 : ftitle.length() ;
        sprintf( fmt, " %%u/%%u - [%%s] %%-%u.%us - %%s - %%s %%s %%s", len, len );

        // nudge the top box text in bottom justified orientation
        int F = 0;
        if ( i == 0 && item_justification )
            F = remainder_item_height - 5;

        // white text
        wattron( win_p, COLOR_PAIR( 7 ) ); 

            mvwprintw( win_p, 0+F, 0, fmt, draw_row + 1, res->numRows(), feed_id.str, ftitle.str, date.str, title_dehtml.str, A.length() ? "-":"", A.length() ? A.str : "" );
        if ( media_url.length() )
            mvwprintw( win_p, 2+F, 0, " %s", media_url.str );
        if ( item_url.length() )
            mvwprintw( win_p, 3+F, 0, " %s", item_url.str );


        // should be able to write i = ++i % M;, but gcc complains about seq pt.
        ++draw_row;
        draw_row = draw_row % tot_items ;

        /* FIXME: do this later, when you restrict number of windows create 
        in cases where num items is less than how many we can fit in a window
        if ( i == tot_items - 1 )
            break;
        */
    }

    // freshy freshy
    refresh();
    for ( unsigned int i = 0; i < item_win.count(); i++ ) {
//        wattron( item_win[i], COLOR_PAIR( 11 ) ); 
//        erase_win_brute_force( item_win[i] );
        wrefresh( item_win[i] );
    }
}


void cursesSlideShow_t::draw_slideView() 
{
    erase_screen_brute_force( item_pane );
    werase( item_pane );
    werase( text_pane );
    erase();

    print_item_slide();
    if ( !msg_timer.timeup() ) {
        draw_msgbox();
    }

    refresh();
    wrefresh( root_window );
    wrefresh( text_pane );

    if ( msg_timer.timeup() ) {
        wrefresh( item_pane );
    } else {
        wrefresh( msgbox_pane );
    }
}

void cursesSlideShow_t::draw_slide() 
{
    if ( in_itemView ) 
    {
        draw_itemView();
    }
    else
    {
        draw_slideView();
    }

    if ( msg_blocking ) 
        wrefresh( help_pane );
}

void cursesSlideShow_t::enter()
{
    in_itemView ^= 1;
}



// uses globals as glue to SIGALRM
void cursesSlideShow_t::toggleSlideShow()
{
    if ( !_delay ) {        // turn on
        _delay = ss_delay;
        signal( SIGALRM, slideshow_alrm );
    }
    else                    // turn off 
    {
        _delay = 0;
    }

    alarm( _delay ); 

    // notify user
    if ( _delay )
        push_msg_immed_timed( 1, " starting slideshow on %d second delay", _delay );
    else
        push_msg_immed_timed( 1, " stopping slideshow" );
}

void cursesSlideShow_t::inc_timer()
{
    ++ss_delay;

    if ( _delay )
        _delay = ss_delay; 

    if ( _delay ) {
        push_msg_immed_timed( 1, " delay increased to %d", ss_delay );
    } else {
        push_msg_immed_timed( 1, " delay increased to %d. Slideshow is currently off. Spacebar to start." , ss_delay );
    }
}

void cursesSlideShow_t::dec_timer()
{
    if ( --ss_delay < 1 )
        ss_delay = 1;

    if ( _delay )
        _delay = ss_delay; 

    if ( _delay ) {
        push_msg_immed_timed( 1, " delay decreased to %d", ss_delay );
    } else {
        push_msg_immed_timed( 1, " delay decreased to %d. Slideshow is currently off. Spacebar to start." , ss_delay );
    }
}

void cursesSlideShow_t::set_cursor()
{
    if ( !linePointers.count() )
        return;

    attron( COLOR_PAIR( 3 ) ); // green, baby

    float frac = ((float)content_line_ofst) / ((float)linePointers.count());
    
    unsigned int text_lines = LINES-2-FIRST_DISPLAY_ROW-2*c_tb_pad; 

    unsigned int line = (unsigned int) (float)(frac * ((float)text_lines));

    attron( A_ALTCHARSET );
    mvprintw( FIRST_DISPLAY_ROW+line, COLS-2, "%c", 'a' );
    attroff( A_ALTCHARSET );
}

void cursesSlideShow_t::ss_scroll_down( unsigned int mv =1 )
{
    if ( ! linePointers.count() )
        return;

    content_line_ofst += mv;

    // 
    while ( content_line_ofst >= linePointers.count() )
        content_line_ofst--;
}

void cursesSlideShow_t::ss_scroll_up( unsigned int mv =1 )
{
    if ( ! linePointers.count() )
        return;

    int ctmp = content_line_ofst;
    ctmp -= mv;
    if ( ctmp < 0 )
        ctmp = 0;
    content_line_ofst = ctmp;
}


void cursesSlideShow_t::mark_item()
{
    DBRow& row = (*res)[ cur_item ];
    // item_url
    DBValue * v = row.FindByName( "item_url" );
    const char * iurl = v && v->getString() ? v->getString() : 0;
    // media_url
    v = row.FindByName( "media_url" );
    const char * murl = v && v->getString() ? v->getString() : 0;
    // feed_id
    v = row.FindByName( "feed_id" );
    int feed_id = v ? v->getInt() : 0;
    // item_id
    v = row.FindByName( "id" );
    int item_id = v ? v->getInt() : 0;
    // title
    v = row.FindByName( "title" );
    const char * title = v && v->getString() ? v->getString() : 0;

    if ( !iurl && !murl ) {
        push_msg_immed_timed( 2, " item has no urls" );
        return ;
    }

    // save & post message
    if ( !rss_mark( feed_id, item_id, title, iurl, murl ) ) {
        push_msg_immed_timed( 4, " Already have it" );
    } else { 
        push_msg_immed_timed( 2, " Item saved." );
    } 
}


void cursesSlideShow_t::open_in_lynx()
{
    if ( text_browser_path.length() == 0 ) {
        push_msg_immed_timed( 2, " no text browser configured" );
        return;
    }

    DBRow& row = (*res)[ cur_item ];
    // item_url
    DBValue * v = row.FindByName( "item_url" );
    const char * iurl = v && v->getString() ? v->getString() : 0;
    if ( !iurl ) {
        v = row.FindByName( "media_url" );
        iurl = v && v->getString() ? v->getString() : 0;
    }
    if ( !iurl ) {
        push_msg_immed_timed( 2, " no url to open" );
        return;
    }

    const char * argv0;
    if ( (argv0 = strrchr( text_browser_path.str, '/')) )
        argv0++;
    else
        argv0 = text_browser_path.str;
    
    if ( execl( text_browser_path.str, argv0, iurl, (char*) 0 ) < 0) {
        push_msg_immed_timed( 2, " opening text browser failed" );
        return;
    }
}

void cursesSlideShow_t::open_in_browser()
{
    if ( browser_path.length() == 0 ) {
        push_msg_immed_timed( 2, " no web browser configured" );
        return;
    }

    DBRow& row = (*res)[ cur_item ];
    // item_url
    DBValue * v = row.FindByName( "item_url" );
    const char * iurl = v && v->getString() ? v->getString() : 0;
    if ( !iurl ) {
        v = row.FindByName( "media_url" );
        iurl = v && v->getString() ? v->getString() : 0;
    }
    if ( !iurl ) {
        push_msg_immed_timed( 2, " no url to open" );
        return;
    }
    
    basicString_t open_cmd = browser_path;
    open_cmd += ' ';
    open_cmd += iurl;
    open_cmd += " >/dev/null 2>&1";

    if ( -1 == system( open_cmd.str ) ) 
    {
        push_msg_immed_timed( 2, " opening in browser failed" );
        return;
    }

    open_cmd = " opening: ";
    open_cmd += iurl;
    push_msg_immed_timed( 2, "%s", open_cmd.str );
}

void cursesSlideShow_t::open_media_url()
{
    if ( browser_path.length() == 0 ) {
        push_msg_immed_timed( 2, " no web browser configured" );
        return;
    }

    DBRow& row = (*res)[ cur_item ];
    // item_url
    DBValue * v = row.FindByName( "media_url" );
    const char * murl = v && v->getString() ? v->getString() : 0;
    if ( !murl ) {
        push_msg_immed_timed( 2, " no media url to open" );
        return;
    }
    
    basicString_t open_cmd = browser_path;
    open_cmd += ' ';
    open_cmd += murl;
    open_cmd += " >/dev/null 2>&1";

    if ( -1 == system( open_cmd.str ) ) 
    {
        push_msg_immed_timed( 2, " opening in browser failed" );
        return;
    }

    open_cmd = " opening media_url: ";
    open_cmd += murl;
    push_msg_immed_timed( 2, "%s", open_cmd.str );
}

// TODO: you could do this in curl, but there is the issue of Login so this will work for now 
void cursesSlideShow_t::run_bookmarklet( bookmarklet_enum_t type )
{
    if ( browser_path.length() == 0 ) {
        push_msg_immed_timed( 2, " no web browser configured" );
        return;
    }

    DBRow& row = (*res)[ cur_item ];
    // item_url
    DBValue * v = row.FindByName( "item_url" );
    const char * _iurl = v && v->getString() ? v->getString() : 0;
    if ( !_iurl ) {
        v = row.FindByName( "media_url" );
        _iurl = v && v->getString() ? v->getString() : 0;
    }
    if ( !_iurl ) {
        push_msg_immed_timed( 2, " no url to save" );
        return;
    }
    basicString_t iurl = get_last_forwarded_url( _iurl );
    basicString_t enc_uri = encodeURIComponent( iurl );

    // title
    v = row.FindByName( "title" );
    basicString_t _title = v && v->getString() ? v->getString() : 0;
    basicString_t title = encodeURIComponent( _title );

    // time
    //const char * time = javascript_now();


    basicString_t url;
    const char * name = 0;

    switch ( type ) 
    {
    case BOOKMARKLET_PINBOARD:
        name = "pinboard";
        url = pinboard;
        url.replace( "%_HREF_%", enc_uri.str ).replace( "%_TITLE_%", title.str );
        break;
    case BOOKMARKLET_INSTAPAPER:
        name = "instapaper";
        if ( !(instapaper_username.length() && instapaper_password.length()) ) 
        {
            push_msg_immed_timed( 5, "Instapaper username/password are not set. See configuration file: \"%s\"", config_path.str );
            return;
        }
        url = instapaper;
        url.replace( "%_HREF_%", enc_uri.str ).replace( "%_TITLE_%", title.str );
        url.replace( "%_USERNAME_%", instapaper_username.str );
        url.replace( "%_PASSWORD_%", instapaper_password.str );
        break;
    default:
        return;
    }
    

    basicString_t open_cmd = browser_path;
    open_cmd += " \"";
    open_cmd += url;
    open_cmd += "\" >/dev/null 2>&1";

    if ( -1 == system( open_cmd.str ) ) 
    {
        push_msg_immed_timed( 3, " %s: saving failed", name );
        return;
    }

    open_cmd.sprintf( "%s: \"%s\" bookmarkleted", name, _title.str );
    push_msg_immed_timed( 8, " %s", open_cmd.str );
}


// 
void cursesSlideShow_t::setupItemWindows()
{
    if ( item_win.count() )
        return;

    const unsigned int TOP_ROW_HEIGHT = FIRST_DISPLAY_ROW;
    const unsigned int ROW_HEIGHT = FIRST_DISPLAY_ROW - 1;

    // TOP_ROW_HEIGHT = 1-row  w/ remainder LINES - TOP_ROW_HEIGHT
    // remainder divided by normal ROW_HEIGHT round down = N rows
    // if remainder > 0 after that , 1 more row

    // number of windows we need for item view
    num_view_items = (LINES - TOP_ROW_HEIGHT) / ROW_HEIGHT + 1;
    remainder_item_height = (LINES - TOP_ROW_HEIGHT) % ROW_HEIGHT;
    if ( remainder_item_height >= 1 )
        num_view_items++;
    item_height = ROW_HEIGHT - 1;


    // typically, one of the windows will be fractional size because the 
    //  total windows height not dividing evenly into the terminal lines
    unsigned int normal_items = remainder_item_height ? num_view_items-1 : num_view_items;

    unsigned int i = 0;
    for ( ; i < normal_items; i++ )
    {
        int nlines  = item_height;
        int ncols   = COLS - 2 - 1 -2 * lr_padding;
        int begin_y = 1 + ROW_HEIGHT * i;
        int begin_x = 1 + lr_padding;
        WINDOW * win_p = newwin( nlines, ncols, begin_y, begin_x );
        Assert( win_p && "Oh shit. newwin() returned NULL" );
        item_win[ i ] = win_p;
    }

    if ( remainder_item_height ) {
        int nlines  = remainder_item_height;
        int ncols   = COLS - 2 - 1 -2 * lr_padding;
        int begin_y = 1 + ROW_HEIGHT * i;
        int begin_x = 1 + lr_padding;
        WINDOW * win_p = newwin( nlines, ncols, begin_y, begin_x );
        Assert( win_p && "Oh shit. newwin() returned NULL" );
        item_win[ i ] = win_p;
    }
}


void cursesSlideShow_t::delete_item()
{
    DBRow& row = (*res)[ cur_item ];
    int saved_id = row.getInt( "saved_id" );
    int item_id = row.getInt( "item_id" );
    int status = 0;

    // TODO: prompt user YESNO
    
    if ( bookmark_mode ) {
        if ( (status = rss_deleteBookmark( saved_id )) )
            push_msg_immed_timed( 2, "bookmark deleted" );
    } else {
        if ( (status = rss_deleteItem( item_id )) )
            push_msg_immed_timed( 2, "item deleted" );
    }

    if ( !status ) {
        push_msg_immed_timed( 2, "nothing removed" );
        return;
    }

    // update the results
    if ( bookmark_mode ) 
    {
        DBResult * dbres = dynamic_cast<DBResult*>( res );
        if ( !dbres )
            return;
        basicString_t query = dbres->getQueryString();
        dbres->freeResultData(); // nuke it
        res = DBA( query.str ); // and get again
    }
    else
    {
        /* ItemResult makes this more complicated

        DBResult * ires = dynamic_cast<ItemResult*>( res );
        if ( !ires )
            return;
        basicString_t query = ires->getQueryString();
        ires->freeResultData(); // nuke it
        res = DBA( query.str ); // and get again
        */
    }

    if ( cur_item >= res->numRows() )
        cur_item = res->numRows()-1;

    tot_items = res->numRows();

    // unset this and it will be rebuilt
    content_processed.erase();
}

void cursesSlideShow_t::run( result_t * _res, unsigned int cur_item_advance ) 
{
    if ( !_res || _res->numRows() == 0 ) {
        return;
    }

    start_curses();
    msgbox_init();

    // these MUST be set right away 
    sshow_p = this; 
    this->res = _res;
    tot_items = _res->numRows();

    // slide show display areas
    if ( !item_pane ) {
        item_pane = newwin( 
                            /* rows = -2 for top+bot border - 2 * pad */
                            FIRST_DISPLAY_ROW - 2 - 2*tb_padding, 
                            /* cols = -2 for left-right border - 2 * pad */
                            COLS - 2 - 1/*cheat right-pad*/ - 2 * lr_padding, 
                            1+tb_padding, 
                            1+lr_padding 
                        );
    }

    if ( !text_pane ) {
        text_pane = newwin( LINES-2-FIRST_DISPLAY_ROW-2*c_tb_pad, 
                            COLS-2-2*c_lr_pad, 
                            FIRST_DISPLAY_ROW+1+c_tb_pad, 
                            1+c_lr_pad 
                        );
    }

    // item view windows
    setupItemWindows();


    // optionally, skip ahead
    if ( 0 != cur_item_advance ) {
        cur_item = cur_item_advance - 1;
        ss_next_item( 1 ); // ensures sane cur_item value
    }

    // half-page
    unsigned int pgdn_lines = (LINES-2-FIRST_DISPLAY_ROW-2*c_tb_pad) / 2;


    // key accelerator stuff
    int rep_cnt = 0;  
    utimer_t keydown_timer;
    int rep_key = 0; // KEY_UP, KEY_DOWN are only ones repeated
    int repeater_lines = 1;


    //
    ss_delay = slideshow_speed; // global setting
 

    int key = 1000;
    bool done = false;

    draw_slide();

    while ( !done && (key = getch()) )
    {
        if ( msg_blocking ) {
            msg_timer.reset();
            msg_blocking = false;
            key = 1000;
            //clear_messages();
        }

        if ( key >= '1' && key <= '9' ) { 
            ss_delay = key - '0';
            if ( _delay ) 
                _delay = ss_delay; 
            push_msg_immed_timed( 1, " delay changed to %d", ss_delay );
        }

        if ( KEY_DOWN == key || KEY_UP == key ) 
        {
            repeater_lines = key_accelerator( key, rep_key, rep_cnt, keydown_timer );
        }
        else
        {
            keydown_timer.reset();
            repeater_lines = 1;
        }


        switch( key )
        {
        case KEY_DOWN:
            key_down( repeater_lines );
            break;
        case KEY_UP:
            key_up( repeater_lines );
            break;
        case KEY_NPAGE:
            key_down( pgdn_lines );
            break;
        case KEY_PPAGE:
            key_up( pgdn_lines );
            break;
        case KEY_LEFT:
            key_left( 1 );
            break;
        case KEY_RIGHT:
            key_right( 1 );
            break;
        case KEY_ENTER:
        case 10:
            enter(); // toggles itemView slideView
            break;
        case 's':
        case 'S':
            break;
        case ' ': /* SPACEBAR */
            toggleSlideShow();
            break;
        case '+':
        case '=':
            inc_timer();
            break;
        case '_':
        case '-':
            dec_timer();
            break;
        case 'g':
        case 'd':
            get_and_display_item_url();
            break;
        case 'b':
            mark_item();
            break;
        case 'r':
            delete_item();
            break;
        case '?':
        case 'h':
            help_menu();
            break;
        case 'l':
        case 'L':
            open_in_lynx();
            break;
        case 'o':
        case 'O':
            open_in_browser();
            break;
        case 'i':
            run_bookmarklet( BOOKMARKLET_INSTAPAPER );
            break;
        case 'p':
            run_bookmarklet( BOOKMARKLET_PINBOARD );
            break;
        case 'm':
            open_media_url();
            break;
        case KEY_HOME:
            content_line_ofst = 0;
            break;
        case KEY_END:
            if ( linePointers.count() )
                content_line_ofst = linePointers.count()-1;
            break;
        default:
            break;
        }


        if ( key == 27 || key == 'q' )
            break;

        draw_slide();
    }

    // this can't go off outside SlideShow
    _delay = 0;
    alarm(_delay);
}



/***********************************************************************************************
 *
 *
 *      cursesMenu_t
 *
 *
 ***********************************************************************************************/
void cursesMenu_t::shutdown() {
    curses_msgbox_t::shutdown();
}
cursesMenu_t::~cursesMenu_t() {
    shutdown();
}

void cursesMenu_t::_menu_show_feeds_f()
{
    if ( !feedmenu_res )
        feedmenu_res = DBA( "select * from feed order by id asc;" );

    searchableResult_t sres(feedmenu_res);

    feedview.run( &sres );
}

void cursesMenu_t::_menu_all_posts_f()
{
    allitem_res.setDBHandle( &DBA );
    all_posts_slideshow.run( &allitem_res, item_skip );
    item_skip = 0;
}


void cursesMenu_t::_menu_default_query_f()
{
}

void cursesMenu_t::_menu_edit_stored_queries_f()
{
}


// TODO: - change to item-view; 
//       - show only items w/ podcast media types in them, 
//          so for insance, BoingBoing wont show up when its not a podcast
void cursesMenu_t::_menu_podcasts_f()
{
    basicString_t& podcast_clause = podcasts_res.getClause();
    if ( ! podcast_clause.length() )
    {
        const char ** pp = podcast_detection_types;
        basicString_t buf;
        podcast_clause = "(";
        do
        {  
            if ( pp != podcast_detection_types )
                podcast_clause += " or";
            podcast_clause += buf.sprintf( " media_url like '%%%s%%'", *pp );
        }
        while ( *++pp );
        podcast_clause += ")";

        // also set this the first time
        podcasts_res.setDBHandle( &DBA );

        // set it to start in item view first time
        podcast_itemView.itemViewMode();
    }

    podcast_itemView.run( &podcasts_res );
}

void cursesMenu_t::_menu_unread_items_f()
{
}

void cursesMenu_t::_menu_bookmarked_items_f()
{

    if ( bookmark_res )
        bookmark_res->freeResultData(); // nuke old query memory

    // do query every time because it may have changed
    bookmark_res = DBA( "select distinct feed.title as ftitle, item_feeds.feed_id, item.*,saved_links.id as saved_id from feed,item_feeds,item,saved_links where item_feeds.item_id = item.id and item_feeds.feed_id = feed.id and saved_links.feed_id = item_feeds.feed_id and saved_links.item_id = item_feeds.item_id order by saved_links.timestamp desc;" );

    cursesSlideShow_t sshow;
    sshow.bookmarkMode();
    sshow.run ( bookmark_res ) ;
}

void cursesMenu_t::_menu_search_f()
{
}

void cursesMenu_t::_menu_last_update_f()
{
    static const char * report_ids;

    basicString_t& clause = last_update_res.getClause();
    if ( ! clause.length() )
    {
        report_ids = last_report_ids();
        if ( !report_ids ) 
            clause = "( feed.id = -1 )";
        else 
            clause = report_ids;

        last_update_res.setDBHandle( &DBA );
        last_update_res.setDistinct(1);
        last_update_itemView.itemViewMode(0); // mix it up a little
    }

    if ( !report_ids ) {
        push_msg( " **No new items in last update" );
    } else {
        last_update_itemView.run( &last_update_res );
    }
}

void cursesMenu_t::initialize_menu()
{
    int i = -1;
    menu_items[++i].label = "Feeds";
    menu_items[i].run = &cursesMenu_t::_menu_show_feeds_f;

    menu_items[++i].label = "All Items";
    menu_items[i].run = &cursesMenu_t::_menu_all_posts_f;

    menu_items[++i].label = "Podcasts";
    menu_items[i].run = &cursesMenu_t::_menu_podcasts_f;

    menu_items[++i].label = "Items from most recent update";
    menu_items[i].run = &cursesMenu_t::_menu_last_update_f;

    menu_items[++i].label = "Bookmarked items";
    menu_items[i].run = &cursesMenu_t::_menu_bookmarked_items_f;

    menu_items[++i].label = "Search";
    menu_items[i].run = &cursesMenu_t::_menu_search_f;

/*
    menu_items[++i].label = "Run Default Query";
    menu_items[i].run = &cursesMenu_t::_menu_default_query_f;  

    menu_items[++i].label = "Edit/Create stored queries";
    menu_items[i].run = &cursesMenu_t::_menu_edit_stored_queries_f;

    menu_items[++i].label = "Unread items";
    menu_items[i].run = &cursesMenu_t::_menu_unread_items_f;
*/
}

void cursesMenu_t::up_cmd()
{
    int row = cursor_row;
    if ( --row < 0 )
        row = 0;
    cursor_row = row;
}

void cursesMenu_t::down_cmd()
{
    if ( ++cursor_row >= menu_items.count() )
        cursor_row = menu_items.count() - 1;
}

void cursesMenu_t::enter_key()
{
    if ( menu_items[cursor_row].run ) { 
        void (cursesMenu_t::*pointer)(void) = menu_items[cursor_row].run;
        (*this.*pointer)();
    }
}


//
void cursesMenu_t::draw_menu()
{
    erase_screen_brute_force();

    // green border
    attron( COLOR_PAIR( 3 ) ); 
    draw_box( 0, 0, COLS, FIRST_DISPLAY_ROW, root_window );

    if ( messages.count() > 2 ) {
        clear_messages();
    }
    if ( messages.count() != 1 && messages.count() != 2 ) {
        push_msg( " Welcome to RSS Power Tool.  Please select an option." );
    }

    // msgbox
    wattron( msgbox_pane, COLOR_PAIR( 7 ) ); // white
    draw_msgbox();

    // green menu text
    attron( COLOR_PAIR( 3 ) );
    for ( unsigned int i = 0; i < menu_items.count(); i++ ) 
    {
        if ( cursor_row == i )
            attron( COLOR_PAIR( 10 ) );
        else
            attron( COLOR_PAIR( 3 ) );
        mvprintw( FIRST_DISPLAY_ROW + 1 + i * 2, 3, "* %s", menu_items[i].label.str );
    }
    attron( COLOR_PAIR( 3 ) );
    
    wrefresh( root_window );
    wrefresh( msgbox_pane );

    if ( msg_blocking ) 
        wrefresh( help_pane );
}

void cursesMenu_t::escape_screen()
{
    const char ** s;
    int j;
    if ( ! help_pane )
    {
        j = 0;
        s = help_list;
        while(*s++)
            ++j;
        
        help_pane = newwin( j+2, COLS - 4, 1, 2 );
    }

    WINDOW * w = help_pane;
    werase(w);
    wattron(w,COLOR_PAIR(3));
    box(w,0,0);

    wattron(w,COLOR_PAIR(7));
    basicString_t msg( "Exit RSS Power Tool?" );
    basicString_t msg2( "('y' to exit. Any other key returns)" );
    
    int row, col;
    getmaxyx( w, row, col ); 
    mvwprintw(w, (row-1)/2-1, col/2-msg.length()/2, "%s", msg.str );
    mvwprintw(w, (row-1)/2+1, col/2-msg2.length()/2, "%s", msg2.str );
    alarm(0);
    msg_blocking = true;
}


// start a pre-specified run state, before executing menu proper
void cursesMenu_t::execute_run_state()
{

// void (cursesMenu_t::*)() 
// set cursor_row
#define SET_CURSOR( X ) { \
    for ( unsigned int i = 0; i < menu_items.count(); i++ ) { \
        if ( menu_items[i].run == X ) { \
            cursor_row = i; \
            break; \
        } \
    } \
}

    // run sub-routine
    switch ( run_state )
    {
    case RUN_FEEDVIEW:
        SET_CURSOR( &cursesMenu_t::_menu_show_feeds_f );
        _menu_show_feeds_f();
        break;
    case RUN_LAST_UPDATE:
        SET_CURSOR( &cursesMenu_t::_menu_last_update_f );
        _menu_last_update_f();
        break;
    case RUN_ALL_ITEMS:
        SET_CURSOR( &cursesMenu_t::_menu_all_posts_f );
        _menu_all_posts_f();
        break;
    case RUN_BOOKMARKS:
        SET_CURSOR( &cursesMenu_t::_menu_bookmarked_items_f );
        _menu_bookmarked_items_f();
        break;
    case RUN_PODCASTS:
        SET_CURSOR( &cursesMenu_t::_menu_podcasts_f );
        _menu_podcasts_f();
        break;
    case RUN_NOT_SET:
    default:
        break;
    }

#undef SET_CURSOR
}


//
void cursesMenu_t::run()
{
    start_curses();
    msgbox_init();
    initialize_menu();

    int key = 1000;
    bool done = false;

    //
    // descend into specific run state, instead of starting with menu
    //
    execute_run_state();


    draw_menu();

    bool in_escape_screen = false;

    while ( !done && (key = getch()) )
    {
        if ( in_escape_screen && ( key == 'y' || key == 'Y' ) ) {
            break;
        }

        in_escape_screen = false;

        if ( msg_blocking ) {
            msg_timer.reset();
            msg_blocking = false;
            key = 1000;
        }

        switch( key )
        {
        case KEY_DOWN:
        case KEY_NPAGE:
            down_cmd();
            break;
        case KEY_PPAGE:
        case KEY_UP:
            up_cmd();
            break;
        case KEY_LEFT:
            break;
        case KEY_RIGHT:
            break;
        case KEY_ENTER:
        case 10:
            enter_key();
            break;
        case 's':
        case 'S':
            break;
        case '?':
        case 'h':
            help_menu();
            break;
        case 27:
        case 'q':
            in_escape_screen = true;
            escape_screen();
            break;
        default:
            break;
        }

        draw_menu();
    }

    // stop curses and cleanup
    shutdown();
    stop_curses();
}

#if 0
void cursesMenu_t::run()
{
    start_curses();
    msgbox_init();
    initialize_menu();

    bool done = false;
    int key;

    //
    // descend into specific run state, instead of starting with menu
    //
    execute_run_state();

    draw_menu();

    // 
    do
    {
        // fill input fifo
        while ( (key = getch()) != ERR )
        {
            input.add( key );
        }

        // process input fifo
        if ( input.length() )
        {  
            // system
            timed.handleInput( input );

            // handler
            Obj& = getHandler();
            Obj.keyHandler( input );
        }

        // execute timer frames, cull queue of
        //timed.runEvents();

        // update
        if ( now >= last_update_msec + update_frame_sz ) {
            Obj& = getHandler();
            Obj.update();

            timed.updateEvents();
        }

        // draw
        if ( now >= last_draw_msec + frame_sz ) {
            // handler
            Obj& = getHandler();
            Obj.draw();

            // system
            timed.draw();
        }

        if ( done ) 
            break;

        maybe_sleep();
    }
    while(1);

    // stop curses and cleanup
    shutdown();
    stop_curses();
}
#endif




/***************************************************************************
 *
 *
 *  timerHandler, timedEvent_t  and other timer code
 *
 *
 ***************************************************************************/

// iface to post message, even if run loop not started
// this message handler is not a linked-list. It is a Windows/Netscape-style
//  pop-up message box that auto-sizes in the center of the page. 
// There are N varieties:
//  - MSG_BLOCKING: any key cancels it
//  - MSG_TIMED, no key. just displays until timer up. doesn't block input.
//                   but doesn't go away when key is pressed.
//  - MSG_TIMED_ANY_KEY, any key: displays for 1-N, as soon as key hit it dissapears
//                    can disappear before N seconds timer up.
//  - MSG_YES_OR_NO yes-and-no blocking, blocks until receives Y or N result, 
//  - MSG_YES_OR_ANY_KEY, [yY] returns 1, any other key 0
timedEvent_t * timerHandler_t::postMessage( int flag, int msec, const char * fmt, ... )
{
    char buf[ _VA_BUF_SZ ];
    char * buffer = &buf[0];
    va_list argptr;
    va_start( argptr, fmt );
    int len_actual = vsnprintf( buffer, sizeof( buf ), fmt, argptr );
    va_end( argptr );

    basicString_t string;

    if ( len_actual < _VA_BUF_SZ )
    {  
        string.set( buffer );
    }
    else
    {
        buffer = (char*) malloc( len_actual+1 );
        va_start( argptr, fmt );
        vsnprintf( buffer, len_actual+1, fmt, argptr );
        va_end( argptr );
        string.set( buffer );
        free( buffer );
    }
    
    timedEvent_t * te = new msgboxTimed_t( string.str, msec );
    te->win = msgboxwin;
    te->flag = flag;
    events.add( te );
    return te;
}

void timerHandler_t::msgTimed( const char * msg, int t ) {
    postMessage( MSG_TIMED, t, "%s", msg );
}

void timerHandler_t::msgTimedKey( const char * msg, int t ) {
    timedEvent_t * te = postMessage( MSG_TIMED_ANY_KEY, t, "%s", msg );
    te->responder = new keyResponder_t( &te->_remove );
}

void timerHandler_t::msgBlocking( const char * msg ) {
    timedEvent_t * te = postMessage( MSG_BLOCKING, 0, "%s", msg );
    te->responder = new keyResponder_t( &te->_remove );
}

void timerHandler_t::msgYesNo( const char * msg ) {
    timedEvent_t * te = postMessage( MSG_YES_OR_NO, 0, "%s", msg );
    te->responder = new yesNoKeyResponder_t( &te->_remove );
}

void timerHandler_t::msgYesKey( const char * msg ) {
    timedEvent_t * te = postMessage( MSG_YES_OR_ANY_KEY, 0, "%s [Y/n]? ", msg  );
    te->responder = new yesAnyKeyResponder_t( &te->_remove );
}


void timerHandler_t::runEvents()
{
    node_t<timedEvent_t*> * ev = events.gethead();
    while ( ev )
    {
        // events may get marked for removal; that is done here
        if ( ev->val->remove() ) {
            node_t<timedEvent_t*> * next = ev->next;
            events.popnode( ev );
            ev = next;
            continue;
        }

        if ( ev->val->timeup() )
            ev->val->timeupMethod();
        else 
            ev->val->update();
        

        ev = ev->next;
    }
}

void timerHandler_t::draw()
{
    node_t<timedEvent_t*> * ev = events.gethead();
    while ( ev )
    {
        ev->val->draw();
        ev = ev->next;
    }
}

void timerHandler_t::handleInput( list_t<int>& input )
{
    if ( !input.count() )
        return;

    node_t<timedEvent_t*> * ev = events.gethead();
    while ( ev )
    {
        timedEvent_t * E = ev->val;

        // each timer gets a stop function that runs when its completed
        if ( E->flag & TIMER_HANDLES_INPUT )
        {   
            do 
            {
                int key = input.peek_first();
                int consume = E->handleKey( key );

                // didn't consume the key
                if ( !consume )
                    break;

                // consume key
                input.shift();

                // out of keys to process
                if ( !input.count() )
                    return;
            } 
            while(1);
        }

        if ( !input.count() )
            return;

        ev = ev->next;
    }
}


int bookmarkResponder_t::answer( int i ) {
    if ( bookmark_id == 0 || i == 0 )
        return 0;
    basicString_t buf;
    DBResult * r = DBA( buf.sprintf( "delete from saved_links where id = %d", bookmark_id ).str );
    return r->rowsUpdated();
}


