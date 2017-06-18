#define bool unsigned char
#define true 1
#define false 0

#define Q(ti89value,ti92pv200value) (ti92pv200value)

#define calc_off() asm("trap #4")

/* ngetchx() equivalent with support for APD, 2nd+ON, DIAMOND+ON and
   grayscale-compatible low-power mode */
short GetKeyInput(void)
{
  while (1) {
    pokeIO(0x600005,0b10111); /* low power mode, wake up CPU only on AI 1
                                 (grayscale), 2 (keypress), 3 (AMS clock), 5
                                 (AMS timer base, needed for APD) and 6 (ON,
                                 always wakes up the CPU) */
    /* Checking the APD here makes sure it will get checked at least each time
       AI1 or AI5 is triggered. */
    if (OSTimerExpired(APD_TIMER)) { // APD expired
      calc_off(); // turn calculator off
      OSTimerRestart(APD_TIMER); // restart APD
      continue; // reenter low power mode
    }
    if (!kbhit()) continue; // if no keypress, reenter low power mode
    OSTimerRestart(APD_TIMER); // restart APD
    unsigned short keypress=ngetchx();
    if (keypress == KEY_OFF /*2nd+ON*/ || keypress == KEY_OFF2 /*DIAMOND+ON*/)
      calc_off();
    else return keypress;
  }
}

// =========================================================================

//tios::globals		equ	tios@001C
//tios::main_lcd		equ	tios::globals+$0000	; $F00 bytes
//LCD_MEM			equ	tios::main_lcd

#define LCD_MEM tios__001C
#define LCD_MEM_SIZE 0x0F00

volatile unsigned char* lcd = (unsigned char*)&LCD_MEM;

void xts_cls() {
  memset( lcd, 0x00, LCD_MEM_SIZE);	
}

// ex. yStart = topMost line
//     nbOfLines = 1 // will scroll-up by one line
void xts_scrollup(int yStart, int nbOfLines, bool clearRemaining) {
	
	volatile int srcOffset = (yStart*SCREEN_WIDTH/8); // 1bpp
	volatile int len       = ((SCREEN_HEIGHT-(yStart+nbOfLines))*SCREEN_WIDTH/8); // 1bpp
	volatile int dstOffset = ((yStart+nbOfLines)*SCREEN_WIDTH/8); // 1bpp

	for(int i= 0; i < len; i++) {
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
int atoi(char* str, int len) {
    int res = 0;
    for (int i=0; i < len; i++) {
        res = (res*10) + (str[i]-'0');
    }
    return res;
}

int atoi_increment(int result, char ch) {
	result = (result *10) + (ch-'0');
	return result;
}


// =========================================================================

// The LIO functions in the TI-92 ROM functions are not exported by Fargo.
unsigned short (*LIO_SendData) (const void *src, unsigned long size); 
unsigned short (*LIO_RecvData) (void *dest, unsigned long size, unsigned long WaitDelay); 

