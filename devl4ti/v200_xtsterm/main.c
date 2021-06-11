// C Source File
// Created 26/05/2021; 16:09:29

// Delete or comment out the items you do not need.
#define COMMENT_STRING         "Arduino terminal"
#define COMMENT_PROGRAM_NAME   "XtsTiTerm2"
#define COMMENT_VERSION_STRING "Voyage 200 version"
#define COMMENT_VERSION_NUMBER 2,0,0,0 /* major, minor, revision, subrevision */
#define COMMENT_AUTHORS        "Xtase - fgalliat"
#define COMMENT_BW_ICON \
	{0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000}
#define COMMENT_GRAY_ICON \
	{0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000}, \
	{0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000, \
	 0b0000000000000000}

#include <tigcclib.h>

// Weird behavior --> local include (on Win10)
#include "C:/vm_mnt/dev/Arduino/XtsTiLink_gh/devl4ti/v200_xtsterm/utils.h"

#define KEY_QUIT 4360
#define KEY_ESC  264

#define FONT_WIDTH 4
#define FONT_HEIGHT 6

#define LOCAL_ECHO false
#define LOCAL_DBGKEY false

char chs[2] = {0x00, 0x00};


// #define byteBuffLen 128
#define byteBuffLen 32
volatile char bytes[byteBuffLen+1];

volatile bool textDirty = false;
volatile bool textDirtyScroll = false;

const char* PRGM_VERSION = "XtsTiTerm 2.0.0ve";

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
	textDirtyScroll = true;
}

void tty_setCharAt(char ch, int addr) {
	ttyMatrix[ addr ] = ch;
}

int tty_cursX() { return ttyCursor % ttyWidth; }
int tty_cursY() { return ttyCursor / ttyWidth; }

int tty_setCurs(int x, int y) { ttyCursor = (y*ttyWidth)+x; return ttyCursor; };


void tty_scrollup() {
	textDirty = false;	

	// characters moving
  int base_addr = 0;
  int i, xx;
	for(i=0; i < TTY_MAX_HEIGHT-1; i++) {
		base_addr = ( i*ttyWidth );
  	for(xx=0; xx < ttyWidth; xx++) {
  		ttyMatrix[ base_addr + xx ] = ttyMatrix[ ( base_addr+ttyWidth ) + xx ];
  	}
  }	
  
  // last line clear
  base_addr = ( (TTY_MAX_HEIGHT-1) * ttyWidth );
 	for(xx=0; xx < ttyWidth; xx++) {
 		ttyMatrix[ base_addr + xx ] = 0x00;
 	}

	// screen moving (from y=8px)
 	xts_scrollup( 8, FONT_HEIGHT, true );
 	
 	// move cursor
 	tty_setCurs( 0, TTY_MAX_HEIGHT-1 );
 	
 	textDirtyScroll = true;
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
	
  int cc=0, xx=0, addr;

	int start = dirtyLineNb-1; if (start < 0) { start = 0; }
	if ( textDirtyScroll ) { start = 0; textDirtyScroll = false; }
	
  int i;
	for(i=start; i < TTY_MAX_HEIGHT; i++) {

		cc=0;
		for(xx=0; xx < ttyWidth; xx++) { 
		  addr = (i*ttyWidth)+xx;
		  line[xx] = ttyMatrix[ addr ];
		  if ( ttyMatrix[ addr ] == 0) { cc=xx; break; } 
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

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// TODO : refactor

#define vt100EscLen 128
volatile char vt100Esc[vt100EscLen];
volatile int  vt100EscCursor = 0;

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
		vt100EscCursor = 0;
		vt100Esc[ vt100EscCursor++ ] = '^';
	} else if ( ch == (char)3 ) { 
		ttyMatrix[ ttyCursor++ ] = 'C';
  	ttyMatrix[ ttyCursor++ ] = 't';
  	ttyMatrix[ ttyCursor++ ] = 'l';
  	ttyMatrix[ ttyCursor++ ] = '-';
  	ttyMatrix[ ttyCursor++ ] = 'C';
	} else {
		ttyMatrix[ ttyCursor++ ] = ch;

		if ( vt100EscCursor >= vt100EscLen-1 ) {
			vt100EscCursor = 0;
			vt100Esc[ vt100EscCursor ] = 0x00;
		}

		if ( vt100EscCursor > 0 ) {
			vt100Esc[ vt100EscCursor++ ] = ch;
			vt100Esc[ vt100EscCursor ] = 0x00;
			
			if ( vt100Esc[1] == '[' ) { // REGULAR VT100
				// character attributes
				if ( ch == 'm' ) { vt100EscCursor = 0; } // end of char attribute
				//else if ( vt100EscCursor == 3 && strncmp( (const unsigned char*)vt100Esc, "^[J", 3) == 0 ) {  // VT100 - CLS code
				else if ( vt100EscCursor == 3 && ch == 'J' ) {  // VT100 - CLS code
					cls();
					vt100EscCursor = 0;
				}
				//else if ( vt100EscCursor == 3 && strncmp( (const unsigned char*)vt100Esc, "^[H", 3) == 0 ) {  // VT100 - cursor top most code
				else if ( vt100EscCursor == 3 && ch == 'H' ) {  // VT100 - cursor top most code
					vt100EscCursor = 0;
				} 
				/*else if ( vt100EscCursor > 2) {
					// let chars in the tty matrix
					vt100EscCursor = 0;
				}*/
			} 
			
			// else if ( vt100Esc[1] == '(' ) { // MOA EXTENDED VT100
			// 	if ( vt100EscCursor == 3 && ch == 'J' ) {  // Ext VT100 - CLS draws only
			// 		low_cls();
			// 		vt100EscCursor = 0;
			// 	} else if ( vt100EscCursor >= 3 && ch == 'r' ) {  // Ext VT100 - draw RECT
			// 	  printSeg("RECT", 120, 0, 4);
				
			// 	  volatile int x=0,y=0,w=0,h=0,r=0;
			// 	  volatile int i; volatile char cc;
				  
			// 	  for(i=2; i < vt100EscCursor; i++) {
			// 	    cc = vt100Esc[i];
			// 	  	if ( cc == ';' || cc== 'r' ) { i++; break; }
			// 	  	x = atoi_increment( x, cc );
			// 	  }
			// 	  for(; i < vt100EscCursor; i++) {
			// 	    cc = vt100Esc[i];
			// 	  	if ( cc == ';' || cc== 'r' ) { i++; break; }
			// 	  	y = atoi_increment( y, cc );
			// 	  }
			// 	  for(; i < vt100EscCursor; i++) {
			// 	    cc = vt100Esc[i];
			// 	  	if ( cc == ';' || cc== 'r' ) { i++; break; }
			// 	  	w = atoi_increment( w, cc );
			// 	  }
			// 	  for(; i < vt100EscCursor; i++) {
			// 	    cc = vt100Esc[i];
			// 	  	if ( cc == ';' || cc== 'r' ) { i++; break; }
			// 	  	h = atoi_increment( h, cc );
			// 	  }
				  
			// 	  if ( cc != 'r' ) {		// optional corder ~radius~
			// 	  	for(; i < vt100EscCursor; i++) {
			// 		    cc = vt100Esc[i];
			// 		  	if ( cc == ';' || cc== 'r' ) { i++; break; }
			// 		  	r = atoi_increment( r, cc );
			// 		  }
			// 	  }
				  
			// 	  xts_drawRoundedRect(x,y,w,h,r);
				  
			// 	  //volatile char msg[60];
			// 	  //sprintf(msg, "RECT(%d,%d,%d,%d -- %d)", x, y, w, h, r);
			// 	  //printSeg(msg, 120, 0, strlen(msg) );

			// 		vt100EscCursor = 0;
			// 	} else if ( vt100EscCursor >= 3 && ch == 'l' ) {  // Ext VT100 - draw POLYLINE
			// 	  volatile int x=0,y=0,x2=0,y2=0;
			// 	  volatile int i; volatile char cc;
				  
			// 	  i=2;
				  
			// 	  for(i=2; i < vt100EscCursor; i++) {
			// 	    cc = vt100Esc[i];
			// 	  	if ( cc == ';' || cc== 'l' ) { i++; break; }
			// 	  	x = atoi_increment( x, cc );
			// 	  }
			// 	  for(; i < vt100EscCursor; i++) {
			// 	    cc = vt100Esc[i];
			// 	  	if ( cc == ';' || cc== 'l' ) { i++; break; }
			// 	  	y = atoi_increment( y, cc );
			// 	  }
			// 	  for(; i < vt100EscCursor; i++) {
			// 	    cc = vt100Esc[i];
			// 	  	if ( cc == ';' || cc== 'l' ) { i++; break; }
			// 	  	x2 = atoi_increment( x2, cc );
			// 	  }
			// 	  for(; i < vt100EscCursor; i++) {
			// 	    cc = vt100Esc[i];
			// 	  	if ( cc == ';' || cc== 'l' ) { i++; break; }
			// 	  	y2 = atoi_increment( y2, cc );
			// 	  }
			// 	  xts_drawLine(x,y,x2,y2);
				  
			// 	  while( cc != 'l' ) {
			// 	  	x=x2; y=y2;
			// 	  	x2=0;y2=0;
				  	
			// 	  	for(; i < vt100EscCursor; i++) {
			// 		    cc = vt100Esc[i];
			// 		  	if ( cc == ';' || cc== 'l' ) { i++; break; }
			// 		  	x2 = atoi_increment( x2, cc );
			// 		  }
			// 		  for(; i < vt100EscCursor; i++) {
			// 		    cc = vt100Esc[i];
			// 		  	if ( cc == ';' || cc== 'l' ) { i++; break; }
			// 		  	y2 = atoi_increment( y2, cc );
			// 		  }
			// 		  xts_drawLine(x,y,x2,y2);
			// 	  }
				  
			// 		vt100EscCursor = 0;
			// 	} else if ( vt100EscCursor >= 3 && ch == 't' ) {  // Ext VT100 - draw text
			// 	  volatile int x=0,y=0,attr=0;
			// 	  volatile int i; volatile char cc;
			// 	  volatile char txt[60]; volatile int cursor = 0;
			// 	  i=2;
				  
			// 	  for(i=2; i < vt100EscCursor; i++) {
			// 	    cc = vt100Esc[i];
			// 	  	if ( cc == ';' || cc== 't' ) { i++; break; }
			// 	  	x = atoi_increment( x, cc );
			// 	  }
			// 	  for(; i < vt100EscCursor; i++) {
			// 	    cc = vt100Esc[i];
			// 	  	if ( cc == ';' || cc== 't' ) { i++; break; }
			// 	  	y = atoi_increment( y, cc );
			// 	  }
			// 	  for(; i < vt100EscCursor; i++) {
			// 	    cc = vt100Esc[i];
			// 	  	if ( cc == ';' ) { i++; break; }
			// 	  	txt[ cursor++ ] = cc;
			// 	  }
				  
			// 	  cc = vt100Esc[i];
			// 	  if ( cc >= '0' && cc <= '9' ) {
			// 	  	attr = cc-'0';
			// 	  }
				  
			// 	  DrawStr(x,y,txt, attr == 2 ? A_REVERSE : A_NORMAL );
				  
			// 	} 
			// 	/* else if ( vt100EscCursor > 2) {
			// 		// let chars in the tty matrix
			// 		vt100EscCursor = 0;
			// 	} */
			// }  
			else if (vt100EscCursor > 1) {
				// let chars in the tty matrix
				vt100EscCursor = 0;
			}
		}
		
	}
	
	ttyMatrix[ ttyCursor   ] = 0x00; // to keep clean lines
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~




	char KEYval[8];
	char HANDSHAKEseq[1] = { 0x06 }; 


// Main Function
void _main(void)
{
  // clrscr ();
	// printf("Hello World !");
	// ngetchx (); // WAITs a char
	
	// seems that global delaracted variables
	// doesn't reset @ PRGM re-launch.....
	FontSetSys( F_4x6 );
	cls();
	tty_clear();
	
	char* beginSession = (char*)"X:?begin\n";
  char* endSession   = (char*)"X:?end\n";
  //beginSession[2] = 0xFF; // not applied !!
  //endSession[2] = 0xFF; // not applied !!
	
	LIO_SendData(beginSession,2+6+1);
	
	char inputBuf[byteBuffLen+1];
	int bytesRecvByTi = 0;
	int recvTimeOut = 0;

	
	while( true ) {
		
	    // ,1 -> byte
  	    // ,2 -> short ...
	 	// unsigned short LIO_RecvData (void *dest, unsigned long size, unsigned long WaitDelay);
	 	
	 	/*	
 		if ( LIO_RecvData(inputBuf,1,1) == 0 ) {
			dispOneChar( inputBuf[0] );
			textDirty = true;
 		} else if (textDirty) { // prevent from tty_cursY() division
 			render( tty_cursY() );
 		}
 		*/
 		// new way
 		memset( inputBuf, 0x00, byteBuffLen+1 );
 		LIO_RecvData(inputBuf, byteBuffLen, 1); // don't care about result
 		if ( inputBuf[0] != 0 ) {
			int i; 			
 			for(i = 0; i < byteBuffLen; i++) {
 				if (inputBuf[i] == 0x00) { break; }
 				dispOneChar( inputBuf[i] );

				bytesRecvByTi++;	// new Handshake System
				if ( bytesRecvByTi == byteBuffLen ) {
					char chs[] = { 'h' };
					LIO_SendData(chs,1);
					recvTimeOut = 0;
					while ( LIO_RecvData(chs,1,1) != 0 ) {
						// sleep ...
						recvTimeOut ++;
						if ( recvTimeOut >= 5 ) { break; } // x * 1/20s prevent from infinite loop
					}
					if ( chs[0] != 'H' ) {
						// SIGNAL AN HANDSHAKE ERROR ??
						dispOneChar( '!' );
						dispOneChar( 'H' );
						dispOneChar( '!' );
					}

					bytesRecvByTi = 0;
				}
 			}
			textDirty = true;
 		} else if (textDirty) { // prevent from tty_cursY() division
 			render( tty_cursY() );
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
