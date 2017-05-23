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

  moa_cls();
	FontSetSys( F_4x6 );
	
	DrawStr(0, 0, "XtsTiTerm 1.0.5", A_NORMAL);

	char chs[2] = {0x00, 0x00};
	int x = 0, y = 8;
	unsigned char inByte = 0;
	char* KEYseq = (char*)"K:";
	
	//char* KEYval = (char*)malloc(6);
	//HANDLE hKEYval = HeapAlloc(6);
	//char* KEYval = HeapDeref( hKEYval );
	char KEYval[6];
	
	char* KEYend = (char*)"\n";
		
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
  beginSession[2] = 0xFF; // not applied !!
  endSession[2] = 0xFF; // not applied !!
	
	LIO_SendData(beginSession,2+6+1);
	
	while( true ) {
		
	  // ,1 -> byte
  	// ,2 -> short ...	
	 	if ( LIO_RecvData(&inByte,1,2) ) {
	 		// failed	
	 	} else {
	 		chs[0] = inByte;
	 		DrawStr(x, y, chs, A_NORMAL);
	 		x += 4; 
	 		if ( x >= 240-10 ) { y += (6+1); x=0; }
	 	}
	
	
	
		//ngetchx();
		// wait a key press
		//short key = GetKeyInput();
		if ( kbhit() ) {
			short key = ngetchx();
			
			LIO_SendData(KEYseq,2);
			memset( KEYval, 0x00, 6);
			//itoa( key, KEYval, 10 );
			sprintf( KEYval, "%d", key );
			LIO_SendData(KEYval,strlen(KEYval));	// send keystroke
			LIO_SendData(KEYend,1);
			
			//LIO_SendData(&key,2);	// send keystroke
			
			if ( key == KEY_QUIT ) {
			  break;				
			}
		}
	}
	
	LIO_SendData(endSession,2+4+1);
}
