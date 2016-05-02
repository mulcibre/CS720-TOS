
#include <kernel.h>

static const int videoBufferStart = 0xB8000;
static const char blankSpace = ' ';
static char charUnderCursor = ' ';

/*
– 25 rows (numbered 0-24)
– 80 columns (numbered 0-79)
– top left corner of screen: row 0, column 0
– lower right corner of screen: row 24, column 79
– screen can display 2000 (25 X 80) characters 
*/
void setCharAtCoord(char c, int x, int y)
{
    //  generate memory offset by screen dimensions above
    int addressToPoke = videoBufferStart + (x * 2) + (y * 160);
    //	set desired character
    poke_b(addressToPoke, c);

    //	set desired color
    poke_b(addressToPoke + 1, 0x0F);
}

void setCharAtCoordInWindow(WINDOW* wnd, char c, int x, int y)
{
    //  generate memory offset by screen dimensions above
    // 	add window origin offsets for x and y
    x += wnd->x;
    y += wnd->y;
   	setCharAtCoord(c, x, y);
}

char getCharAtCoord(int x, int y)
{
    //  generate memory offset by screen dimensions above
    int addressToPeek = videoBufferStart + (x * 2) + (y * 160);
    return (char)peek_b((MEM_ADDR) addressToPeek);
}

char getCharAtCoordInWindow(WINDOW* wnd, int x, int y)
{
	x += wnd->x;
    y += wnd->y;
   	getCharAtCoord(x, y);
}

void shiftWindowBuffer(WINDOW* wnd)
{	
    int x, y;
    for(y = 0; y < wnd->height - 1; y++)
    {
        for(x = 0; x < wnd->width; x++)
        {
            setCharAtCoordInWindow(wnd, getCharAtCoordInWindow(wnd, x, y + 1), x, y);
        }
    }
    //  Clear last line
    x = 0;
    for(; x < wnd->width; x++)
    {
        setCharAtCoordInWindow(wnd, blankSpace, x, wnd->height - 1);
    }
    move_cursor(wnd, 0, wnd->height - 1);
}

/* Window Struct
typedef struct {
  int  x, y;
  int  width, height;
  int  cursor_x, cursor_y;
  char cursor_char;
} WINDOW;
*/
void move_cursor(WINDOW* wnd, int x, int y)
{
    /*
      The position of the cursor is set to be (x, y). Note that this
position has to be within the boundaries of the window. The
position is relative to the top-left corner of the window.   
        */
    //char temp = getCharAtCoord(x, y);
    //  set old cursor location to have previous character
    //setCharAtCoord(charUnderCursor, wnd->cursor_x, wnd->cursor_y);
    assert(x < wnd->width && y < wnd->height);
    wnd->cursor_x = x;
    wnd->cursor_y = y;
    //setCharAtCoord(wnd->cursor_char, x, y);
}


void remove_cursor(WINDOW* wnd)
{
    setCharAtCoord(' ', wnd->cursor_x, wnd->cursor_y);
}


void show_cursor(WINDOW* wnd)
{
    /*
    Shows the cursor of the window by displaying cursor_char at
    the current position of the cursor location. 
    */
    setCharAtCoord(wnd->cursor_char, wnd->cursor_x, wnd->cursor_y);
}


void clear_window(WINDOW* wnd)
{
    /*
    Clear the window. Content of the window is erased and the
    cursor is placed at the top left corner of the window. 
    */
    int x = wnd->x;
    int y = wnd->y;
    for(; x < wnd->width; x++)
    {
        for(; y < wnd->height; y++)
        {
            setCharAtCoord(blankSpace, x, y);
        }
    }
    charUnderCursor = blankSpace;
    
    wnd->cursor_x = wnd->x;
    wnd->cursor_y = wnd->y;
}

void output_char(WINDOW* wnd, unsigned char c)
{
    /*
    ‘ch’ is displayed at the current cursor location of the window.
    The cursor is advanced to the next location. 
    */
    // newline check, or carriage return
    if(c == '\n' || c == 13)
    {
       if(wnd->cursor_y >= wnd->height - 1)
       {
           //cursor is on last line, scroll window
           shiftWindowBuffer(wnd);
       	}
       	else
       	{
       		move_cursor(wnd, 0, wnd->cursor_y + 1);
   		}
    }
    //backspace
    else if(c == '\b')
    {
    	if (wnd->cursor_x != 0) 
    	{
	    	wnd->cursor_x--;
		}
    }
    else
    {
        //  set desired character at cursor position
        setCharAtCoordInWindow(wnd, c, wnd->cursor_x, wnd->cursor_y);
        if(wnd->cursor_x >= wnd->width - 1)
        {
            if(wnd->cursor_y >= wnd->height - 1)
            {
                //cursor is on last line, scroll window
                shiftWindowBuffer(wnd);
            }
            else
            {
            // cursor was at end of line? move to next line
            move_cursor(wnd, 0, wnd->cursor_y + 1);
        	}
        }
        else
        {	
            move_cursor(wnd, wnd->cursor_x + 1, wnd->cursor_y);
        }
    }
}



void output_string(WINDOW* wnd, const char* str)
{
    /*
    ‘str’ is a string that is displayed in the window. The cursor is
    advanced accordingly. 
    */
    int index = 0;
    while(str[index] != 0)
    {
        output_char(wnd, str[index]);
        index++;
    }
}

/*
 * There is not need to make any changes to the code below,
 * however, you are encouraged to at least look at it!
 */
#define MAXBUF (sizeof(long int) * 8)		 /* enough for binary */

char *printnum(char *b, unsigned int u, int base,
	       BOOL negflag, int length, BOOL ladjust,
	       char padc, BOOL upcase)
{
    char	buf[MAXBUF];	/* build number here */
    char	*p = &buf[MAXBUF-1];
    int		size;
    char	*digs;
    static char up_digs[] = "0123456789ABCDEF";
    static char low_digs[] = "0123456789abcdef";
    
    digs = upcase ? up_digs : low_digs;
    do {
	    *p-- = digs[ u % base ];
	    u /= base;
    } while( u != 0 );
    
    if (negflag)
	*b++ = '-';
    
    size = &buf [MAXBUF - 1] - p;
    
    if (size < length && !ladjust) {
	while (length > size) {
	    *b++ = padc;
	    length--;
	}
    }
    
    while (++p != &buf [MAXBUF])
	*b++ = *p;
    
    if (size < length) {
	/* must be ladjust */
	while (length > size) {
	    *b++ = padc;
	    length--;
	}
    }
    return b;
}


/*
 *  This version implements therefore following printf features:
 *
 *	%d	decimal conversion
 *	%u	unsigned conversion
 *	%x	hexadecimal conversion
 *	%X	hexadecimal conversion with capital letters
 *	%o	octal conversion
 *	%c	character
 *	%s	string
 *	%m.n	field width, precision
 *	%-m.n	left adjustment
 *	%0m.n	zero-padding
 *	%*.*	width and precision taken from arguments
 *
 *  This version does not implement %f, %e, or %g.  It accepts, but
 *  ignores, an `l' as in %ld, %lo, %lx, and %lu, and therefore will not
 *  work correctly on machines for which sizeof(long) != sizeof(int).
 *  It does not even parse %D, %O, or %U; you should be using %ld, %o and
 *  %lu if you mean long conversion.
 *
 *  This version implements the following nonstandard features:
 *
 *	%b	binary conversion
 *
 */


#define isdigit(d) ((d) >= '0' && (d) <= '9')
#define ctod(c) ((c) - '0')


void vsprintf(char *buf, const char *fmt, va_list argp)
{
    char		*p;
    char		*p2;
    int			length;
    int			prec;
    int			ladjust;
    char		padc;
    int			n;
    unsigned int        u;
    int			negflag;
    char		c;
    
    while (*fmt != '\0') {
	if (*fmt != '%') {
	    *buf++ = *fmt++;
	    continue;
	}
	fmt++;
	if (*fmt == 'l')
	    fmt++;	     /* need to use it if sizeof(int) < sizeof(long) */
	
	length = 0;
	prec = -1;
	ladjust = FALSE;
	padc = ' ';
	
	if (*fmt == '-') {
	    ladjust = TRUE;
	    fmt++;
	}
	
	if (*fmt == '0') {
	    padc = '0';
	    fmt++;
	}
	
	if (isdigit (*fmt)) {
	    while (isdigit (*fmt))
		length = 10 * length + ctod (*fmt++);
	}
	else if (*fmt == '*') {
	    length = va_arg (argp, int);
	    fmt++;
	    if (length < 0) {
		ladjust = !ladjust;
		length = -length;
	    }
	}
	
	if (*fmt == '.') {
	    fmt++;
	    if (isdigit (*fmt)) {
		prec = 0;
		while (isdigit (*fmt))
		    prec = 10 * prec + ctod (*fmt++);
	    } else if (*fmt == '*') {
		prec = va_arg (argp, int);
		fmt++;
	    }
	}
	
	negflag = FALSE;
	
	switch(*fmt) {
	case 'b':
	case 'B':
	    u = va_arg (argp, unsigned int);
	    buf = printnum (buf, u, 2, FALSE, length, ladjust, padc, 0);
	    break;
	    
	case 'c':
	    c = va_arg (argp, int);
	    *buf++ = c;
	    break;
	    
	case 'd':
	case 'D':
	    n = va_arg (argp, int);
	    if (n >= 0)
		u = n;
	    else {
		u = -n;
		negflag = TRUE;
	    }
	    buf = printnum (buf, u, 10, negflag, length, ladjust, padc, 0);
	    break;
	    
	case 'o':
	case 'O':
	    u = va_arg (argp, unsigned int);
	    buf = printnum (buf, u, 8, FALSE, length, ladjust, padc, 0);
	    break;
	    
	case 's':
	    p = va_arg (argp, char *);
	    if (p == (char *)0)
		p = "(NULL)";
	    if (length > 0 && !ladjust) {
		n = 0;
		p2 = p;
		for (; *p != '\0' && (prec == -1 || n < prec); p++)
		    n++;
		p = p2;
		while (n < length) {
		    *buf++ = ' ';
		    n++;
		}
	    }
	    n = 0;
	    while (*p != '\0') {
		if (++n > prec && prec != -1)
		    break;
		*buf++ = *p++;
	    }
	    if (n < length && ladjust) {
		while (n < length) {
		    *buf++ = ' ';
		    n++;
		}
	    }
	    break;
	    
	case 'u':
	case 'U':
	    u = va_arg (argp, unsigned int);
	    buf = printnum (buf, u, 10, FALSE, length, ladjust, padc, 0);
	    break;
	    
	case 'x':
	    u = va_arg (argp, unsigned int);
	    buf = printnum (buf, u, 16, FALSE, length, ladjust, padc, 0);
	    break;
	    
	case 'X':
	    u = va_arg (argp, unsigned int);
	    buf = printnum (buf, u, 16, FALSE, length, ladjust, padc, 1);
	    break;
	    
	case '\0':
	    fmt--;
	    break;
	    
	default:
	    *buf++ = *fmt;
	}
	fmt++;
    }
    *buf = '\0';
}



void wprintf(WINDOW* wnd, const char *fmt, ...)
{
    va_list	argp;
    char	buf[160];

    va_start(argp, fmt);
    vsprintf(buf, fmt, argp);
    output_string(wnd, buf);
    va_end(argp);
}




static WINDOW kernel_window_def = {0, 0, 80, 25, 0, 0, ' '};
WINDOW* kernel_window = &kernel_window_def;


void kprintf(const char *fmt, ...)
{
    va_list	  argp;
    char	  buf[160];

    va_start(argp, fmt);
    vsprintf(buf, fmt, argp);
    output_string(kernel_window, buf);
    va_end(argp);
}


