// C Source File
// Created 20/05/2017; 13:21:29

// Delete or comment out the items you do not need.
#define COMMENT_STRING         "Xtase terminal."
#define COMMENT_PROGRAM_NAME   "XtsTerm"
#define COMMENT_VERSION_STRING "1.4"
#define COMMENT_VERSION_NUMBER 1,4,0,0 /* major, minor, revision, subrevision */
#define COMMENT_AUTHORS        "Xtase - fgalliat"

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 128

//#include <tigcclib.h>
#include "fargodef.h" // replacement for tigcclib.h
#include "utils.h"

#define KEY_QUIT 4360
#define KEY_ESC  264

#define FONT_WIDTH 4
#define FONT_HEIGHT 6

#define LOCAL_ECHO false
#define LOCAL_DBGKEY false

char chs[2] = {0x00, 0x00};


#define byteBuffLen 128
volatile char bytes[byteBuffLen+1];
//volatile int cursor = 0;
volatile bool textDirty = false;

const char* PRGM_VERSION = "XtsTiTerm 1.4.3ov";

void printSeg(char* str, int x, int y, int ll) {
  if ( ll <= 0 ) { return; }
	if ( y < 0 ) { return; }
	if ( y+FONT_HEIGHT >= SCREEN_HEIGHT ) { return; }

	if ( x+(ll*FONT_WIDTH) >= SCREEN_WIDTH ) {
	 //return;	
	}
	
	// +8 is for appTitle line
  DrawStr(x, y+8, str, A_NORMAL);
}


void low_cls() {
	xts_cls();
	DrawStr(0, 0, PRGM_VERSION, A_NORMAL);
	xts_drawHorizLine( 1, 6, SCREEN_WIDTH-2, true );
	xts_drawHorizLine( 0, 6+1, SCREEN_WIDTH-0, true );
}

// _______________________________________________
// the text (chars) matrix

// SCREEN_WIDTH / FONT_WIDTH
#define ttyWidth  60 
// ~ SCREEN_HEIGHT / FONT_HEIGHT
#define ttyHeight 20

#define TTY_MAX_HEIGHT (ttyHeight - 5)

#define ttyLen (ttyWidth * TTY_MAX_HEIGHT)
char ttyMatrix[ttyLen];
int ttyCursor = 0;

void tty_clear() {
	memset(ttyMatrix, 0x00, ttyLen);
	ttyCursor = 0;
	low_cls();
}

void tty_setCharAt(char ch, int addr) {
	ttyMatrix[ addr ] = ch;
}

// void tty_setChar(char ch, int x, int y) { tty_setCharAt(ch, (y*ttyWidth)+x ); }

int tty_cursX() { return ttyCursor % ttyWidth; }
int tty_cursY() { return ttyCursor / ttyWidth; }

int tty_setCurs(int x, int y) { ttyCursor = (y*ttyWidth)+x; return ttyCursor; };


void tty_scrollup() {
	textDirty = false;	

	// characters moving
  int base_addr = 0;
	for(int i=0; i < TTY_MAX_HEIGHT-1; i++) {
		base_addr = ( i*ttyWidth );
  	for(int xx=0; xx < ttyWidth; xx++) {
  		ttyMatrix[ base_addr + xx ] = ttyMatrix[ ( base_addr+ttyWidth ) + xx ];
  	}
  }	
  
  // last line clear
  base_addr = ( (TTY_MAX_HEIGHT-1) * ttyWidth );
 	for(int xx=0; xx < ttyWidth; xx++) {
 		ttyMatrix[ base_addr + xx ] = 0x00;
 	}

	// screen moving (from y=8px)
 	xts_scrollup( 8, FONT_HEIGHT, true );
 	
 	// move cursor
 	tty_setCurs( 0, TTY_MAX_HEIGHT-1 );
 	
 	textDirty = true;
}

// _______________________________________________

void cls() {
	low_cls();
	tty_clear();
}

char line[ttyWidth+1];

void render(int dirtyLineNb) {
  if ( !textDirty )	{ return; }
	
  int cc=0, xx=0;

	for(int i=0; i < TTY_MAX_HEIGHT; i++) {
		memset( line, 0x00, ttyWidth );

		cc=0;
		for(xx=0; xx < ttyWidth; xx++) { 
		  line[xx] = ttyMatrix[ (i*ttyWidth)+xx ];
		  if ( ttyMatrix[ (i*ttyWidth)+xx ] == 0) { cc=xx; break; } 
		}
		if ( cc > 0 ) {
		  printSeg(line, 0, (i*FONT_HEIGHT), cc);
		}

		// kept because dirty can covers multiple lines
	  if ( i >= dirtyLineNb+1 )	{ break; }

	}
	
	textDirty = false;
}


void low_br() {
	tty_setCharAt( 0x00, ttyCursor );
	
	if ( tty_cursY()+1 >= TTY_MAX_HEIGHT ) {
		tty_scrollup();
		tty_setCurs( 0, TTY_MAX_HEIGHT-1 );		
	} else {
		tty_setCurs( 0, tty_cursY()+1 );		
	}

}

void disp(char* str, int len) {
  if ( len == 0 )	{ return; }
  //if ( len < 0 )  { len = strlen( str ); }
  //if ( len > byteBuffLen ) {
  //	len = byteBuffLen;
  //}

  //v3 of impl.	
  for(int i=0; i < len; i++) {
    if ( str[i] == 13 ) { continue; }	
    
  	if ( str[i] == 10 ) {
 			low_br();
			render( tty_cursY() );
  	}
  	
  	else if (str[i] == 27 && len > i+1 && str[i+1] == '[') {
  		// this is a VT100 ESCAPE Cmd
  		
  		if ( len > i+2 && str[i+2] == 'J' ) {
  			cls();
	  		i+=3;
  		} else if ( len > i+3 && str[i+2] == '2' && str[i+3] == 'J') {
  			// not tested
  			cls();
  			i+=4;
  		} else if ( len > i+3 && str[i+2] >= '0' && str[i+2] <= '9' ) {
  			i+=2;
	  		// character attributes
	  		// ex. ^[0m      end of char attribs
	  		// ex. ^[05;01;32;44m 
	  		// blink 05 - bold 01 - green fg 32 ; blue bg 44
	  		while( str[i] != 'm' && i < len ) { i++; }
	  	} else {
	  		i+=2;
	  		ttyMatrix[ ttyCursor++ ] = '^';
	  		ttyMatrix[ ttyCursor++ ] = '[';
	  		for(; i < len; i++) {
		  		ttyMatrix[ ttyCursor++ ] = str[i];
	  		}
  		}
  	}
  	
  	else {
  		
  		// auto wraps line
  		
  		if ( str[i] == 0x09 ) { // TAB
  		  ttyMatrix[ ttyCursor++ ] = ' ';
  		  ttyMatrix[ ttyCursor++ ] = ' ';
  		  ttyMatrix[ ttyCursor++ ] = ' ';
  		  ttyMatrix[ ttyCursor++ ] = ' ';
  		} else {
	  		ttyMatrix[ ttyCursor++ ] = str[i];
  		}		
  		
  		if ( ttyCursor >= ttyLen ) {
  			tty_scrollup();
  		}

  		ttyMatrix[ ttyCursor   ] = 0x00; // to keep clean lines
  		
  	}
  	
 	}

}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// TODO : refactor
void dispOneChar(char ch) {
	if ( ch == '\r' ) { return; }
	if ( ch == '\n' ) { 
	  low_br();
		render( tty_cursY() );
	} else if ( ch == '\t' ) { 
		ttyMatrix[ ttyCursor++ ] = ' ';
  	ttyMatrix[ ttyCursor++ ] = ' ';
  	ttyMatrix[ ttyCursor++ ] = ' ';
  	ttyMatrix[ ttyCursor++ ] = ' ';
	} else if ( ch == '\b' ) { 
	  ttyMatrix[ ttyCursor   ] = 0x00;
	} else if ( ch == (char)27 ) {
		ttyMatrix[ ttyCursor++ ] = '^';
	} else if ( ch == (char)3 ) { 
		ttyMatrix[ ttyCursor++ ] = 'C';
  	ttyMatrix[ ttyCursor++ ] = 't';
  	ttyMatrix[ ttyCursor++ ] = 'l';
  	ttyMatrix[ ttyCursor++ ] = '-';
  	ttyMatrix[ ttyCursor++ ] = 'C';
	} else {
		ttyMatrix[ ttyCursor++ ] = ch;
	}
	
	ttyMatrix[ ttyCursor   ] = 0x00; // to keep clean lines
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~




	char KEYval[8];
	char HANDSHAKEseq[1] = { 0x06 }; 


void sleepSec(int sec) {
	//OSFreeTimer (USER_TIMER);
	//OSRegisterTimer (USER_TIMER, sec*20);
	//while (!OSTimerExpired (USER_TIMER));
	
	//OSTimerRestart(USER_TIMER);
	//while (!OSTimerExpired (USER_TIMER));
}



// Main Function
void _main(void) {
/*
	// Install timekeeping TSR (FiftyMSecTick) into the vector table.
  // This is intentionally NOT removed at the end of the program.
  unsigned long OldInt5=(unsigned long)GetIntVec(AUTO_INT_5);
  if (OldInt5!=0x44) {
    long *p=(long*)0x40044;
    *(p++)=0x52b90004; // addq.l #1,0x4...
    *(p++)=0x00404ef9; // ...0040; jmp ...
    *(long*)p=OldInt5; // ... [OldInt5].l
    SetIntVec(AUTO_INT_5,(INT_HANDLER)0x44);
  } // necessary ??
*/

	// seems that global delaracted variables
	// doesn't reset @ PRGM re-launch.....
	FontSetSys( F_4x6 );
	cls();
	tty_clear();
		
	// ========== that code prevents from "#### LINE 1111 : Emulator #####" ==
	// Ugly hack to find LIO_SendData and LIO_RecvData...
  LIO_SendData=(void*)ERD_dialog;
  while (*(void**)LIO_SendData!=OSLinkTxQueueInquire) LIO_SendData+=2;
  LIO_RecvData=(void*)LIO_SendData;
  LIO_SendData-=28;
  while (*(void**)LIO_RecvData!=OSReadLinkBlock) LIO_RecvData+=2;
  LIO_RecvData-=150;
  // =======================================================================
	
	char* beginSession = (char*)"X:?begin\n";
  char* endSession   = (char*)"X:?end\n";
  //beginSession[2] = 0xFF; // not applied !!
  //endSession[2] = 0xFF; // not applied !!
	
	LIO_SendData(beginSession,2+6+1);
	
	// just a try ....
	char inLengthBuf[2]; // no more than 64KB
	unsigned int inLength;
	char inputBuf[byteBuffLen+1];
	
	int idleCpt = 0;
	
	while( true ) {
		
	  // ,1 -> byte
  	// ,2 -> short ...	
	 	//if ( LIO_RecvData(inByte,1,4) ) {
	 		
 		if ( LIO_RecvData(inputBuf,1,1) == 0 ) {
			//disp( inputBuf, 1 );
			dispOneChar( inputBuf[0] );
			textDirty = true;
 		} else {
 			
 		  //if ( idleCpt++ > 2 ) {
 			  render( tty_cursY() );
 			  idleCpt=0;
 			//}
 		}
 		
	
		// read from KBD
		//ngetchx();
		//short key = GetKeyInput();
		if ( kbhit() ) {
			short key = ngetchx();
			
			// @ this time : avoid key-repeat
			while( kbhit() ) {}
			
			memset( KEYval, 0x00, 8);
			sprintf( KEYval, "K:%d\n", key );
			LIO_SendData(KEYval,strlen(KEYval));	// send keystroke code
			//LIO_SendData(&key,2);	// send keystroke
			
			if ( key == KEY_QUIT ) {
			  break;				
			} else {
				
			  if ( LOCAL_ECHO ) {
					// local echo
					/*
				  chs[0] = (char)key;	
					if ( chs[0] == 13 ) { br(); }
			 		else {
			 			DrawStr(x, y, chs, A_NORMAL);
			 			x += FONT_WIDTH;
			 			if ( x >= SCREEN_WIDTH-10 ) { br(); }
			 		}
			 		*/
		 		}
		 		
		 		if (LOCAL_DBGKEY) {
		 			// local keystroke display code	
		 			/*
			 		DrawStr(x, y, KEYval, A_NORMAL);
			 		x += FONT_WIDTH *5; 
			 		if ( x >= SCREEN_WIDTH-10 ) { br(); }
			 		*/
		 		}
		 		
			}
		} // if kbhit()
		
	} // end while
	
	LIO_SendData(endSession,2+4+1);
}
