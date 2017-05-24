#ifndef _ti_soft_h
#define _ti_soft_h 1

/**************************
* TI 8x - 9x link software routines
* 
* Xtase - fgalliat May2017
*
* 
**************************/


// ====================
// content sent in one-shot mode from RAM
#define SEND_MODE_RAM    0x01
// content is streamed from PROGMEM
#define SEND_MODE_FLASH  0x02
// content is streamed from Serial Port
#define SEND_MODE_SERIAL 0x03
// ====================

#define VAR_FILE_SIZE_OFFSET 86
#define VAR_FILE_DATA_OFFSET (VAR_FILE_SIZE_OFFSET+2)

#define REQ_SCREENSHOT 0x6D
#define REQ_BACKUP     0xA2
#define REP_OK         0x56
#define CMD_REMOTE     0x87

// File types
#define FTYPE_EXP 0x00
#define FTYPE_STR 0X0C
#define FTYPE_TXT 0X0B

#define FTYPE_ASM 0x21
#define FTYPE_BAS 0x12

// @ dataLen - 3
#define VAR_ARCHIVED_NO 0x00
#define VAR_ARCHIVED_YES 0x01

// ------------------------------------------------------------

static int ti_sendKeyStroke(int data);
static int ti_sendKeyStrokes(char* data, int len=-1);

// ------------------------------------------------------------

// =================================================================
// =================================================================
// =================================================================
// =================================================================

uint8_t* TI_chk(uint8_t b[], int len) {
		int sum = 0;
		for (int i = 4; i < len - 2; i++) {
			sum += b[i];
		}
		int checksum0 = sum % 256;
		int checksum1 = (sum / 256) & 0xFF;

    // static -> the ONLY way to be sure to return correctly the given array (when converting to pointer)
    static uint8_t result[2] = { (uint8_t) checksum0, (uint8_t) checksum1 };
		return result;
}

void TI_header(const char* constCharFileName, int fileType, int dataLen, boolean silent, int& dtLen, bool send) {
		int i;

		int nameLength = strlen(constCharFileName);
		// packLen = 4 bytes for dataLen
		// + 1 for dataType
		// + 1 for fileName.length
		// + n for filename
		// + 1 for 0 terminating
		int packLen = 4 + 1 + 1 + nameLength + 1;
		// finalLen is : 1 byte for machine ID
		// + 1 for cmd
		// + 2 for packLen
		// + packLen
		// + 2 for CHK
		const int finalLen = 1 + 1 + 2 + packLen + 2;

    // static : the ONLY way to be sure to return correctly the given array (when converting to pointer)
    // but consume all RAM
    uint8_t result[ finalLen ];
		//static char result[ 256 ];

		// MACHINE ID
		result[0] = (char) (silent ? 0x09 : 0x08);
		// CMD
		result[1] = (char) (silent ? 0xC9 : 0x06);
    //result[1] = (char) (silent ? 0x06 : 0xC9);

		// packet length
		result[2] = (char) (packLen % 256);
		result[3] = (char) (packLen / 256);

		// dataLen :: TODO : manager more than 64KB files
		result[4] = (char) (dataLen % 256);
		result[5] = (char) (dataLen / 256);
		result[6] = 0;
		result[7] = 0;

		// file type
		result[8] = (char) fileType;

		// file name length
		result[9] = (char) nameLength;

		// file name
		for (i = 0; i < nameLength; i++) {
			result[10 + i] = constCharFileName[i];
		}
		// zero-terminated
		result[10 + nameLength] = (char) 0;

		// 11+nameLength => 0
		// 12+nameLength => 0

		// CHK
		uint8_t* chk = TI_chk(result, finalLen);
    result[11 + nameLength] = *(chk+0);
		result[12 + nameLength] = *(chk+1);
                
    dtLen = finalLen;

    if (send) {
       ti_send(result, finalLen);
    }

}

// ======= reuse allocated screen mem-segment ===
extern uint8_t screen[];
// beware w/ that (filled by ain.ino)
int __SCREEN_SEG_MEM = -1;
// ==============================================

void TI_xdp(char data[], int dataLen, int sendingMode, boolean silent, int& dtLen, bool archived) {
		// packLen = 4 bytes for dataLen
		// + n for filename
		int packLen = 4 + dataLen;

		// finalLen is : 1 byte for machine ID
		// + 1 for cmd
		// + 2 for packLen
		// + packLen
		// + 2 for CHK
		int finalLen = 1 + 1 + 2 + packLen + 2;

    uint8_t result[8];

		// MACHINE ID
		result[0] = (char) (silent ? 0x09 : 0x08);
		// CMD
		result[1] = (char) 0x15;

		// packet length
		result[2] = (char) (packLen % 256);
		result[3] = (char) (packLen / 256);

		// dataLen :: TODO : ???
		result[4] = 0;
		result[5] = 0;
    result[6] = 0;
		result[7] = 0;

    int sum = 0;

    // head
    ti_send(result, 8);
    delay( DEFAULT_POST_DELAY/2 );


    // data
    if ( sendingMode == SEND_MODE_RAM ) {
      sum = 0;
      for (int i = 0; i < dataLen; i++) { sum += (uint8_t)data[i];	}
      ti_send( (uint8_t*)data, dataLen);
      delay( DEFAULT_POST_DELAY/2 );
    } else if ( sendingMode == SEND_MODE_FLASH ) {
      // Var len
      sum = 0;
      sum += (uint8_t)data[0];	
      sum += (uint8_t)data[1];	
      ti_send( (uint8_t*)data, 2);

      uint8_t D[1];
      for (int i = 0; i < dataLen-2; i++) { 
        D[0] = pgm_read_byte_near(FILE_CONTENT + VAR_FILE_DATA_OFFSET + i);
        if ( i == dataLen - 2 - 3 ) {
           D[0] = (uint8_t)( archived ? VAR_ARCHIVED_YES : VAR_ARCHIVED_NO );
        }
        sum += (uint8_t)D[0];	
        ti_send( (uint8_t*)D, 1);
      }
      delay( DEFAULT_POST_DELAY/2 );
    } else if ( sendingMode == SEND_MODE_SERIAL ) {
      // Var len
      sum = 0;
      sum += (uint8_t)data[0];	
      sum += (uint8_t)data[1];	
      ti_send( (uint8_t*)data, 2);

      //const int BLOC_LEN =128;
      //uint8_t D[BLOC_LEN]; 
      const int BLOC_LEN =514;
      uint8_t* D = screen;
      int e;
      for (int i = 0; i < dataLen-2; i+=BLOC_LEN) { 

       // request to send - handshake
       serPort.write( 0x02 );
       while( serPort.available() == 0 ) {}

        e = BLOC_LEN;
        if ( i+BLOC_LEN > dataLen-2 ) { e = (dataLen-2)-i; }
        
        // even if ends w/ garbages
        serPort.readBytes( D, BLOC_LEN );

        for(int j=0; j < e; j++) { 
          if ( i+j == dataLen - 2 - 3 ) {
            D[j] = (uint8_t)( archived ? VAR_ARCHIVED_YES : VAR_ARCHIVED_NO );
          }
          sum += D[j];	
        } 
        ti_send( D, e);
      }
      //delay( DEFAULT_POST_DELAY/2 );
    } else {
      outprint(F("OUPS !!!"));
    } 

		// 8+nameLength => 0
		// 9+nameLength => 0
    sum &= 0xFFFF;
		int checksum0 = sum % 256;
		int checksum1 = (sum / 256);// & 0xFF;

    result[0] = checksum0;
    result[1] = checksum1;

    dtLen = finalLen;

    ti_send(result, 2);
    delay( DEFAULT_POST_DELAY/2 );

    DBUG(result, 2); // just to verify

	}


int sendTiFile(bool autolaunch=false, bool fromSerial=false) {
  int i, postDelay=DEFAULT_POST_DELAY;

  // ==== preparing datas ====
  int fileType = FTYPE_TXT;
  fileType = FTYPE_EXP;
  fileType = FTYPE_STR;
  fileType = FTYPE_TXT; // Nt Yet functional
  fileType = FTYPE_BAS;
  //fileType = FTYPE_ASM; // Works well on a TI92+

  char* initDatas = NULL;
  int initDatasLen = 1;
  char* fileName = (char*)"main\\atxt2";

  int sendingMode = SEND_MODE_RAM;
  bool archived = false;

if ( fromSerial ) {
  // just after '\SP' ...
  initDatasLen = (serPort.read()*256)+serPort.read();

  fileName = (char*)malloc(32);
  memset(fileName, 0x00, 32);
  char ch;
  int nl = 0;
  while( (ch = serPort.read()) != 0 ) {
    fileName[nl++] = ch;
  }
  fileName[nl] = 0x00;

  bool mode92p = serPort.read() == 1; // else ASM
  autolaunch = serPort.read() == 1;

  if ( mode92p ) { fileType = FTYPE_BAS; }
  else { fileType = FTYPE_ASM; }

  sendingMode = SEND_MODE_SERIAL;
  initDatas = NULL;

}
else if ( fileType == FTYPE_EXP || fileType == FTYPE_STR ) {
  initDatas = (char*)"ABC";
  initDatasLen = strlen( initDatas );
} else if ( fileType == FTYPE_TXT ) { // Not Yet finisihed
  initDatas = (char*)"?ABC?";
  initDatasLen = strlen( initDatas );
  // seee : http://merthsoft.com/linkguide/ti92/vars.html
  // for text lines format
  initDatas[0] = 0x20;
  initDatas[initDatasLen-1] = 0x00;
} else if ( fileType == FTYPE_BAS || fileType == FTYPE_ASM ) {
  // initDatas = (char*)"_txt2()\nPrgm\nDisp \"123\"\nEndprgm?";
  // prgm name cant start by '_'
  fileName = (char*)"main\\bbb";
  if ( fileType == FTYPE_ASM ) {
    fileName = (char*)"main\\tetris";
  }

  int i=0;
  for (int k = 0; k < VAR_FILE_SIZE_OFFSET; k++) { // head
    pgm_read_byte_near(FILE_CONTENT + i);
    i++;
  }

  int d0=0,d1=0;
  d0 = pgm_read_byte_near(FILE_CONTENT + i++);
  d1 = pgm_read_byte_near(FILE_CONTENT + i++);

  int rlen = (d0*256)+d1;
  outprint("Var len = "); outprint(rlen); outprint("\n");
  initDatasLen = rlen; // up to 'DC'

  if ( true ) {
    sendingMode = SEND_MODE_FLASH;
    initDatas = NULL;
  } else {
    sendingMode = SEND_MODE_RAM;

    initDatas = (char*)malloc( initDatasLen );
    memset(initDatas, 0x00, initDatasLen );

    for (int k = 0; k < initDatasLen; k++) { // var content
      char myChar = pgm_read_byte_near(FILE_CONTENT + i);
      initDatas[k] = myChar;
      i++;
    }

    // if 0x01 => PRGM is archived 
    initDatas[ initDatasLen-3 ] = archived ? VAR_ARCHIVED_YES : VAR_ARCHIVED_NO;

    // 2 bytes remaining for CHKSUM
  }
}

  outprint(F("Var sending : ")); outprint(fileName); 
    outprint(" "); outprint(initDatasLen);
    outprint(F(" bytes long ")); outprint(fileType);
    outprint(" "); outprintln(sendingMode);

  int len = 0;
  // http://merthsoft.com/linkguide/ti92/vars.html

  // max can occurs
  char data[ 20 + (sendingMode == SEND_MODE_RAM ? initDatasLen : 0 ) ];
  int dataLen = 0;
  if (fileType == FTYPE_EXP) {
			data[0] = (char) (initDatasLen / 256);
			data[1] = (char) (initDatasLen % 256);
			for (i = 0; i < initDatasLen; i++) {
				data[2 + i] = initDatas[i];
			}
      dataLen = 2 + initDatasLen;

  } else if (fileType == FTYPE_STR) {
			data[0] = (char) ((initDatasLen + 3) / 256); // -2 instead of +3 ???? => http://merthsoft.com/linkguide/ti92/vars.html
			data[1] = (char) ((initDatasLen + 3) % 256);
			data[2] = (char) 0;
			for (i = 0; i < initDatasLen; i++) {
				data[3 + i] = initDatas[i];
			}
			data[3 + initDatasLen] = 0;
			data[4 + initDatasLen] = (char) 0x2D;
      
      dataLen = 2 + 1 + initDatasLen + 1 + 1;
  } else if (fileType == FTYPE_TXT) {
			data[0] = (char) ((initDatasLen + 3) / 256);
			data[1] = (char) ((initDatasLen + 3) % 256);
			data[2] = (char) 0; // CH \__ last opening caret position
      data[3] = (char) 1; // CL /

      // ===================== each lines ================
      // 1st byte : Line type: 0Ch=page break, 20h=normal, 43h=Command, 50h=PrintObj
      // terminated by a 0x0D as '\n'
      // whole file by a 0x00
			for (i = 0; i < initDatasLen; i++) {
				data[4 + i] = initDatas[i];
			}
      // ====================================

			data[4 + initDatasLen] = (char) 0xE0; // end of Text Var
      dataLen = 2 + 2 + initDatasLen + 1;

  } else if (fileType == FTYPE_BAS || fileType == FTYPE_ASM) {
      // FROM a .92P content
			data[0] = (char) ((initDatasLen + 0) / 256); // -2 instead of +3 ???? => http://merthsoft.com/linkguide/ti92/vars.html
			data[1] = (char) ((initDatasLen + 0) % 256);
      if ( initDatas != NULL ) {
        for (i = 0; i < initDatasLen; i++) {
          data[2 + i] = initDatas[i];
        }
      }
      dataLen = 2 + initDatasLen;
  } else {
      outprint(F("Oups ftype"));
  }

   
  bool silent = true;
 
  // ==== preparing datas ====  
  if (sendingMode == SEND_MODE_SERIAL) {
     // request to send
     serPort.write( 0x01 );
     while( Serial.available() == 0 ) { delay(2); }
     serPort.read(); //0x00
  }
  
  // == RTS ==
  TI_header(fileName, fileType, dataLen, silent, len, true);
  delay(postDelay);

  uint8_t recv[4];
  
  // == ACK ==
  ti_recv(recv, 4); if ( recv[1] == 0x5a ) { outprintln(F("failed to read ACK")); return -1; }
  
  // == CTS ==
  ti_recv(recv, 4); if ( recv[1] == 0x5a ) { outprintln(F("failed to read CTS")); return -1; }
  
  // == ACK ==
  uint8_t B[4] = { 0x09, 0x56, 0x00, 0x00 };
  ti_send(B, 4);
  delay(postDelay);
  
  // == XDP == 
  TI_xdp(data, dataLen, sendingMode, silent, len, archived);
  delay(postDelay);
  
  // == ACK ==
  ti_recv(recv, 4); if ( recv[1] == 0x5a ) { outprintln(F("failed to read ACK (2)")); return -1; }
  
  // == EOT ==
  uint8_t D[4] = { 0x09, 0x92, 0x00, 0x00 };
  ti_send(D, 4);
  delay(postDelay);  
  
  // ACK ==
  ti_recv(recv, 4); if ( recv[1] == 0x5a ) { outprintln(F("failed to read ACK (3)")); return -1; }

  outprintln(F("Var sent"));
  
  if ( autolaunch ) {
    outprintln(F("Launch Var"));

    ti_sendKeyStroke( 0x0107 ); // CLEAR from TI92 manual p.484
    ti_sendKeyStrokes(fileName);
    ti_sendKeyStrokes((char*)"()");
    ti_sendKeyStroke(0x0D); // ENTER
  }

  return 0;
}

// ==========================================

static int ti_sendKeyStrokes(char* data, int len) {
  if ( len < 0 ) { len = strlen(data); } 
  for(int i=0; i < len; i++) { ti_sendKeyStroke( (int)data[i] ); }
  return 0;
}

static int ti_sendKeyStroke(int data) {
  // http://merthsoft.com/linkguide/ti92/remote.html
  uint8_t D[4] = { PC_2_TI, CMD_REMOTE, (uint8_t)(data%256), (uint8_t)(data/256) };
  ti_send(D, 4);
  delay(DEFAULT_POST_DELAY/2);
  ti_recv(D, 4); if ( D[1] != REP_OK ) { outprint("failed to read ACK"); outprintln(D[1]); return -1; }
  delay(DEFAULT_POST_DELAY/2);
  return 0;
}

 // ==============================================================
 // ==============================================================

uint16_t chksum(uint8_t* seg, int len) {
  uint16_t result = 0x0000;
  for(int i=4; i < len-2; i++) {
    result += seg[i];
  }
  result &= 0xFFFF;
  return result;
}

void sendAbackup(uint16_t dataLen) {
  // BEWWARE : limited to ROM 1.12 support
  uint8_t romVer[] = { '1', '.', '1', '2' };
  
  // 9A 08 -> 39432 -> dataLen
  //                                               vvvv  vvvv
  uint8_t pc2tiMsg[16] = { 0x09, 0x06, 0x0A, 0x00, 0x08, 0x9A, 0x00, 0x00, 0x1D, 0x04, romVer[0], romVer[1], romVer[2], romVer[3], 0x00, 0x00 };
  uint8_t ti2pcMsg[4];
  uint8_t intermediate[4] = { 0x09, 0x56, 0x00, 0x00 };
  uint8_t chkSUM[2] = { 0x00, 0x00 };
  
  pc2tiMsg[4] = ( dataLen % 256 );
  pc2tiMsg[5] = ( dataLen / 256 ) & 0xFF;
  
  uint16_t chk = chksum(pc2tiMsg, 16);
  pc2tiMsg[14] = ( chk % 256 );
  pc2tiMsg[15] = ( chk / 256 ) & 0xFF;
  
  ti_send( pc2tiMsg, 16 );
  delay(DEFAULT_POST_DELAY);
  
  ti_recv(ti2pcMsg, 4);
  if ( ti2pcMsg[1] != 0x56 ) { outprint("ERR HEAD 1"); DBUG(ti2pcMsg, 4); return; }
  
  for(uint16_t i=0; i < dataLen; i += 1024) {
    int remLen = 1024;
    if ( i+1024 > dataLen ) { remLen = dataLen-i; }
  
  	pc2tiMsg[4] = ( remLen / 256 ) & 0xFF; 
  	pc2tiMsg[5] = ( remLen % 256 );
  	chk = chksum(pc2tiMsg, 16);
    pc2tiMsg[14] = ( chk % 256 );
    pc2tiMsg[15] = ( chk / 256 ) & 0xFF; 
    ti_send( pc2tiMsg, 16);
    delay(DEFAULT_POST_DELAY/2);
    
  	ti_recv(ti2pcMsg, 4);
  	if ( ti2pcMsg[1] != 0x56 ) { outprint(F("ERR BB a ")); outprint( i/1024 ); DBUG(ti2pcMsg, 4); return; }  
    delay(DEFAULT_POST_DELAY/2);

  	ti_recv(ti2pcMsg, 4);
    if ( ti2pcMsg[1] == 0x56 ) {
      ti_recv(ti2pcMsg, 4);
      delay(DEFAULT_POST_DELAY/2);
    }
  	if ( ti2pcMsg[1] != 0x09 ) { outprint(F("ERR BB b ")); outprint( i/1024 ); DBUG(ti2pcMsg, 4); return; }  
    delay(DEFAULT_POST_DELAY/2);

    intermediate[1] = 0x56;
    ti_send(intermediate, 4);
    delay(DEFAULT_POST_DELAY/2);
    
    intermediate[1] = 0x15;
    
    intermediate[2] = remLen%256;
    intermediate[3] = remLen/256;
    ti_send(intermediate, 4);
    
    
    // =======================
    // SEND 1024 bytes
    // get chk
    chk = 0;int e=0;
    if ( remLen > 512 ) {
      serPort.write(0x01);

      serPort.readBytes( screen, 512 );
      for(int i=0; i < 512; i++) { chk += screen[i]; }
      ti_send( screen, 512 );
      e+=512;
      // outprint( 512 ); outprintln(F(" bytes"));
    }
    
    int ll = min( remLen-e, 512 );
    serPort.write(0x01);

    serPort.readBytes( screen, ll );
    for(int i=0; i < ll; i++) { chk += screen[i]; }
    ti_send( screen, ll );
    
    // outprint( ll+e ); outprintln(F(" bytes"));
    // =======================
    
    chk &= 0xFFFF;
    
    chkSUM[0] = chk % 256;
    chkSUM[1] = chk / 256;
    ti_send(chkSUM, 2);
    delay(DEFAULT_POST_DELAY/2);
    
    
    ti_recv(ti2pcMsg, 4);
  	if ( ti2pcMsg[1] != 0x56 ) { outprint(F("ERR EB a ")); outprint( i/1024 ); DBUG(ti2pcMsg, 4); return; }  
    delay(DEFAULT_POST_DELAY/2);
  }
  
  intermediate[1] = 0x92;
  ti_send(intermediate, 4);
  delay(DEFAULT_POST_DELAY/2);
  
  serPort.write(0x01);
}

 // ==============================================================
 // ==============================================================

 // goes to Serial
 int ti_receiveBackup() {
   outprint("-BOF-\n");

   static uint8_t D[23] = { PC_2_TI, REQ_BACKUP, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1D, 0x0B, 0x6D, 0x61, 0x69, 0x6E, 0x5C, 0x62, 0x61, 0x63, 0x6B, 0x75, 0x70, 0x9F, 0x04 };
   ti_send(D, 23);
   delay(DEFAULT_POST_DELAY);

   // to prevent TIMEOUT waiting
   int realSegLen = __SCREEN_SEG_MEM;
   
   // ... a bit dirty
   realSegLen = 1028 / 2; // 514
   if (realSegLen > __SCREEN_SEG_MEM) { realSegLen = __SCREEN_SEG_MEM; }
   // ... a bit dirty
   //outprint("seglen : ");outprint(realSegLen);outprint("\n");

   static uint8_t E[4] = { PC_2_TI, REP_OK, 0x00, 0x00 };
   static uint8_t F[4] = { PC_2_TI, 0x09, 0x00, 0x00 };
   

   int blocCpt = 0;

   ti_recv(D, 4);  if ( D[1] != REP_OK ) { outprint("failed to read ACK : "); outprintln(D[1]); return -1; }
   ti_recv(D, 16); if ( D[1] != (blocCpt == 0 ? 0x06 : 0x15) ) { outprint("failed to read BCK bloc (1)"); outprintln(D[1]); return -1; }

    // serPort.print( "HED : " );
    // for(int i=0; i < 16; i++) {
    //   serPort.print( D[i]  < 16 ? "0" : "" ); serPort.print( D[i], HEX ); serPort.print( " " );
    // }
    // serPort.print( "\n" );

   do {
      //outprint("Bloc : "); outprintln(blocCpt);

      ti_send(E, 4); 
      ti_send(F, 4);

      ti_recv(D, 4); if ( D[1] != REP_OK ) { outprint("failed to read ACK (2)"); outprintln(D[1]); return -1; }
      ti_recv(D, 4); if ( D[1] != 0x15 ) { outprint("failed to read BCK bloc (2)"); outprintln(D[1]); return -1; }

      int len = (D[3]*256) + D[2];
      //len -= 4;

      serPort.write( D[3] );
      serPort.write( D[2] );

      // beware : not a multiple of 1028....
      // for(int i=0; i < len; i+=realSegLen) {
      //   ti_recv(screen, min(realSegLen, len) );
      //   //serPort.write( D[0] ); // A REMETTTRE !!!!!!
      //   // serPort.print( D[0]  < 16 ? "0" : "" ); serPort.print( D[0], HEX ); serPort.print( " " );
      //   serPort.write( screen, min(realSegLen, len)  );
      // }

      for(int i=0; i < len; i++) {
        ti_recv(D, 1 );
        serPort.write( D[0] );
      }

      ti_recv(D, 2); // read chk sum

      ti_send(E, 4);

      // //if ( len < 1024 ) {
      // if ( len <= 1024 ) {
      //   // outprint("last bloc : "); outprintln(len);
      //   ti_recv(D, 4); if ( D[1] != 0x92 ) { outprint("failed to read -EOT-"); outprintln(D[1]); return -1; }
      //   ti_send(E, 4); delay(DEFAULT_POST_DELAY/2);
      //   break;
      // } else {
        // ti_recv(D, 16); 
        ti_recv(D, 4); 

        if ( D[1] == 0x92 ) {
          ti_send(E, 4); delay(DEFAULT_POST_DELAY/2);
          break;
        }

        if ( D[1] != 0x06 ) { outprint("failed to read BCK bloc (3)"); outprintln(D[1]); return -1; }
        ti_recv(D, 12); 
        
      // }

      blocCpt++;

   } while( true );

      serPort.write( 0x00 );
      serPort.write( 0x00 );

   outprint("-EOF-\n");

   return 0;
 }


#endif