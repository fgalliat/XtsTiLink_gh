// Header File
// Created 26/05/2021; 16:38:31
#define bool unsigned char
#define true 1
#define false 0

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 128

// old from Fargo ...
// #define LCD_MEM tios__001C
// #define LCD_MEM_SIZE 0x0F00
// volatile unsigned char* lcd = (unsigned char*)&LCD_MEM;

#define LCD_MEM_SIZE LCD_SIZE
volatile unsigned char* lcd = (unsigned char*)LCD_MEM;

void xts_cls() {
  // memset( lcd, 0x00, LCD_MEM_SIZE);	
  clrscr ();
}

// ex. yStart = topMost line
//     nbOfLines = 1 // will scroll-up by one line
void xts_scrollup(int yStart, int nbOfLines, bool clearRemaining) {
	
	volatile int srcOffset = (yStart*SCREEN_WIDTH/8); // 1bpp
	volatile int len       = ((SCREEN_HEIGHT-(yStart+nbOfLines))*SCREEN_WIDTH/8); // 1bpp
	volatile int dstOffset = ((yStart+nbOfLines)*SCREEN_WIDTH/8); // 1bpp

  int i;
	for(i= 0; i < len; i++) {
		lcd[ i + srcOffset ] = lcd[ i + dstOffset ];
	}
	
	if ( clearRemaining ) {
		
	  /*
		for(int i= len; i < LCD_MEM_SIZE; i++) {
			lcd[ i+dstOffset ] = 0x00;
		}
		*/
	}

}

void xts_drawHorizLine(int x, int y, int x2, bool black) {
	
	// mandatory black ?
	MoveTo( x, y );
	DrawTo( x2, y );
}


void xts_drawLine(int x, int y, int x2, int y2) {
	MoveTo( x, y );
	DrawTo( x2, y2 );
}

void xts_drawRect(int x, int y, int w, int h) {
	xts_drawLine(x,y,x+w,y);
	xts_drawLine(x+w,y,x+w,y+h);
	xts_drawLine(x+w,y+h,x,y+h);
	xts_drawLine(x,y+h,x,y);
}

void xts_drawRoundedRect(int x, int y, int w, int h, int radius) {
	xts_drawRect(x,y,w,h);
	// rounded corner isn't impl. yet
}

// =========================================================================


// assumes only posiive values
/*
int atoi(char* str, int len) {
    int res = 0;
    for (int i=0; i < len; i++) {
        res = (res*10) + (str[i]-'0');
    }
    return res;
}
*/

int atoi_increment(int result, char ch) {
	result = (result *10) + (ch-'0');
	return result;
}

