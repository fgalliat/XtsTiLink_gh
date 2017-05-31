// C Source File
// Created 20/05/2017; 13:21:29

// Delete or comment out the items you do not need.
#define COMMENT_STRING         "Xtase terminal."
#define COMMENT_PROGRAM_NAME   "XtsTerm"
#define COMMENT_VERSION_STRING "1.0"
#define COMMENT_VERSION_NUMBER 1,0,0,0 /* major, minor, revision, subrevision */
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

//#include <tigcclib.h>
#include "fargodef.h" // replacement for tigcclib.h
#include "utils.h"


#define KEY_QUIT 4360
#define KEY_ESC  264

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 128

#define FONT_WIDTH 4
#define FONT_HEIGHT 6

#define LOCAL_ECHO false
#define LOCAL_DBGKEY false

char chs[2] = {0x00, 0x00};
int x = 0, y = 8;


void cls() {
	moa_cls();
	y=8; x = 0;
	DrawStr(0, 0, "XtsTiTerm 1.1.9", A_NORMAL);
}

void br() {
	y += (FONT_HEIGHT+1); 
	x=0;
	if ( y > SCREEN_HEIGHT - 10 ) {
		cls();
	}
}

void printSeg(char* str) {
	DrawStr(x, y, str, A_NORMAL);
}

void print(char* str, int len) {
  //char str2[ len+1 ];
  //memcpy( str2, str, len );
  //str2[len] = 0x00;	
	//printSeg( str2 );
	
  str[len] = 0x00;	
	printSeg( str );
}


void println(char* str, int len) {
  print(str, len);
	br();	
}


void disp(char* str, int len) {
  if ( len == 0 )	{
    return;
  }
  
  if ( len < 0 ) {
  	len = strlen( str );
  }
	
  char bytes[len+1];
  memset(bytes, 0x00, len+1);
  int cursor = 0;

  //v2 of impl.	
  for(int i=0; i < len; i++) {
    if ( str[i] == 13 ) { continue; }	
  	if ( str[i] == 10 ) {
  		if ( cursor == 0 ) {
  			br();
  		} else {
	  		println( bytes, cursor );
	  		memset(bytes, 0x00, len+1);
	  		cursor = 0;
  		}
  	}
  	else if (str[i] == 27 && len > i+1 && str[i+1] == '[') {
  		// this is a VT100 ESCAPE Cmd
  		if ( len > i+3 && str[i+2] == '2' && str[i+3] == 'J' ) {
  			cls();
  			i+=4;
  		} else if ( len > i+3 && str[i+2] >= '0' && str[i+2] <= '9' ) {
	  		// character attributes
	  		// ex. ^[0m      end of char attribs
	  		// ex. ^[05;01;32;44m 
	  		// blink 05 - bold 01 - green fg 32 ; blue bg 44
	  		while( str[i] != 'm' && i < len ) { i++; }
	  	} else {
	  		i+=2;
	  		bytes[ cursor++ ] = '^';
	  		bytes[ cursor++ ] = '[';
	  		bytes[ cursor++ ] = str[i];
  		}
  	}
  	else {
	  	if ( x + (cursor*FONT_WIDTH) >= SCREEN_WIDTH-10 ) {
	  		println( bytes, cursor );
	  		//memset(bytes, 0x00, cursor);
	  		memset(bytes, 0x00, len+1);
	  		cursor = 0;
	  	}
	  	
	  	bytes[ cursor++ ] = str[i];
  	}
 	}
 	
 	if ( cursor > 0 ) {
 		//println( bytes, cursor );
 		print(bytes, cursor);
 		x += (cursor*FONT_WIDTH);
 		if ( x >= SCREEN_WIDTH-10 ) { br(); }
 	}
}

	char KEYval[8];
	char HANDSHAKEseq[1] = { 0x06 }; 


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
	char inputBuf[512+1];
	
	
	while( true ) {
		
	  // ,1 -> byte
  	// ,2 -> short ...	
	 	//if ( LIO_RecvData(inByte,1,4) ) {
	 		
	 	// read from I/O
	 	while(true) {
			if ( LIO_RecvData(inLengthBuf,2,1) ) {
		 		// failed	
		 		break;
		 	} else {
		 		
		 	  // 2 bytes for length
		 		inLength = (int)inLengthBuf[0] * 256;	
		 		inLength += (int)inLengthBuf[1];
		 		
		 		if ( inLength > 0 ) {
					memset(inputBuf, 0x00, 512+1);
	//			sprintf( KEYval, "%d >", inLength );
	//			println( KEYval, strlen( KEYval ) );
			 		LIO_RecvData(inputBuf,inLength,0); // wait infinitly
			 		disp( inputBuf, inLength );
				}

		 		
		 		LIO_SendData(HANDSHAKEseq,1);
		 	}
	 	}
	
		// read from KBD
		//ngetchx();
		//short key = GetKeyInput();
		if ( kbhit() ) {
			short key = ngetchx();
			
			memset( KEYval, 0x00, 8);
			sprintf( KEYval, "K:%d\n", key );
			LIO_SendData(KEYval,strlen(KEYval));	// send keystroke code
			//LIO_SendData(&key,2);	// send keystroke
			
			if ( key == KEY_QUIT ) {
			  break;				
			} else {
				
			  if ( LOCAL_ECHO ) {
					// local echo
				  chs[0] = (char)key;	
					if ( chs[0] == 13 ) { br(); }
			 		else {
			 			DrawStr(x, y, chs, A_NORMAL);
			 			x += FONT_WIDTH;
			 			if ( x >= SCREEN_WIDTH-10 ) { br(); }
			 		}
		 		}
		 		
		 		if (LOCAL_DBGKEY) {
		 			// local keystroke display code	
			 		DrawStr(x, y, KEYval, A_NORMAL);
			 		x += FONT_WIDTH *5; 
			 		if ( x >= SCREEN_WIDTH-10 ) { br(); }
		 		}
		 		
			}
		} // if kbhit()
		
	} // end while
	
	LIO_SendData(endSession,2+4+1);
}
