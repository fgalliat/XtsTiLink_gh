/******************
* Ti (92) control Program w/ an Arduino
*
* Xtase - fgalliat @May2017
*
*
* Wiring :
*
* 2   -> red (tip)
* 3   -> white (ring)
* GND -> black (GND)
*
* 4 -> push button -> GND 
* 5 -> push button -> GND 
* 6 -> push button -> GND 
*
* Functions :
* - ScreenShot
* - Kbd reading w/ a local PRGM 
* - getTime (needs CTS managment)
* - Silently send an STR Var 
* - ABLE to SEND a .92P file stored in flashMem (BEWARE 2KB RAM limit for one shot sending)
*    \___ can now call PRGM just after download (by TI Keyboard Controlling)
*    \___ able to send it as ARCHIVE or not
*    \___ able to Stream from FLASH (for long files support)
* - ABLE to Dump Memory (backup) [EXPERIMENTAL]
* - ABLE to send a backup (.92B) to TI
*
* Todo :
*  
*
* thanks to Christopher Mitchell
* for ArTICL Lib. that inspired this work
* thanks to website : http://merthsoft.com/linkguide/ti92/vars.html#program
**************************/

// embed comm PRGM
//#include "PONG.h"
//#include "bbb.h"
// #include "tetrisgb.h"  // Works well on a TI92+ (as ASM Var)
#include "xtsterm.92p.h"

#define TI_MODEL_92
#include "tilink.h"
#include "tisoft.h"


#define BTN 4
#define BTN2 5
#define BTN3 6
#define PRESSED LOW

void sendFlashFileToTi();

// ===================== RAM Settings ==================
// max alowed to store big packets
#define MAX_ARDUINO_MEM_SIZE 870

// 3840 bytes for a TI-92 => to HUDGE to fit in RAM
#define MAX_TI_SCR_SIZE ( TI_SCREEN_HEIGHT*(TI_SCREEN_WIDTH/8) )

  // to have complete scanlines
#define nbScanlines ( MAX_ARDUINO_MEM_SIZE / (TI_SCREEN_WIDTH/8) )

#define SCREEN_SEG_MEM ( nbScanlines * (TI_SCREEN_WIDTH/8) )

// for forwarding ref
//const int __SCREEN_SEG_MEM = SCREEN_SEG_MEM;
// ===================== RAM Settings ==================

uint8_t screen[SCREEN_SEG_MEM];

void dummyMode();

 // ====================================================

  uint8_t data[4] = { PC_2_TI, REQ_SCREENSHOT, 0x00, 0x00 }; // request screen shot (silent mode)
  uint8_t recv[10];
  char intValue[10];

  
  

void setup() {
  __SCREEN_SEG_MEM = SCREEN_SEG_MEM;

  pinMode(BTN, INPUT_PULLUP);
  pinMode(BTN2, INPUT_PULLUP);
  pinMode(BTN3, INPUT_PULLUP);
  
  Serial.begin(115200);
  ti_resetLines();
  
  //while( !Serial ) {delay(300);}

  digitalWrite(13, HIGH);
}

  
  void dispScreenMem(int len) {
    // Dump the screen to the serial console
    for (int i = 0; i < len; i++) {
      for (int j = 7; j >= 0; j--) {
        if (screen[i] & (1 << j)) {
          Serial.write('#');
        } else {
          Serial.write('.');
        }
      }
      if (i % (TI_SCREEN_WIDTH/8) == (TI_SCREEN_WIDTH/8)-1) { // 240/8 => 30 bytes
        Serial.println();
      }
    }
  }
 

void reboot() {
  ti_resetLines();
  asm volatile ("  jmp 0");
}

void loop() {
  //delay(500);
  int recvNb = -1;

  digitalWrite(13, HIGH);

  //ti_resetLines();
  //delay(100);
  recvNb = ti_recv(recv, 2);
  
  if ( recvNb == 0 && recv[0] == 'X' && recv[1] == ':' ) {
    //serPort.println(F("DUMMY ??"));
    recvNb = ti_recv(recv, 6+1);
    //serPort.print( recvNb ); serPort.print( F(" ") ); serPort.print( recv[0] ); serPort.print( F(" ") ); serPort.println( recv[1] ); 
    //if ( recvNb == 0 && recv[0] == 0xFF && recv[1] == 'b' ) {
    if ( recvNb == 0 && recv[0] == '?' && recv[1] == 'b' ) {
      // X:begin\n
      // dummy serial mode : XtsTerm.92p

      serPort.println(F("DUMMY"));

      while(true) {
        if ( (recvNb = serPort.available()) > 0 ) {
          // !! max 814 !!
          memset(screen, 0x00, 128);
          recvNb = serPort.readBytes( screen, recvNb );
          //ti_send( screen, recvNb );
          memset(recv, 0x00, 10);
          for(int i=0; i < recvNb; i++) {
            recv[0] = screen[i];
            ti_send( recv, 1 );
            delay(1);
          }
          delay(DEFAULT_POST_DELAY/2);
        }

        recvNb = ti_recv(recv, 2);
        if ( recvNb == 0 && recv[0] == 'X' && recv[1] == ':' ) {
          recvNb = ti_recv(recv, 4+1);
          serPort.print( recvNb ); serPort.print( F(" ") ); serPort.print( recv[0] ); serPort.print( F(" ") ); serPort.println( recv[1] ); 
          //if ( recvNb == 0 && recv[0] == 0xFF && recv[1] == 'e' ) {
          if ( recvNb == 0 && recv[0] == '?' && recv[1] == 'e' ) {
            // X:end\n -> end of serial session
            // break;
            serPort.println(F("EXIT DUMMY"));
            reboot();
            return;
          } 
        } else if ( recvNb == 0 && recv[0] == 'K' && recv[1] == ':' ) {
            //K:<code>\n

            // found key 97       a
            // found key 56       num
            // found key 338      arrow
            // found key 344      arrow
            // found key 264      Esc
            // found key 4360     2nd + Esc => Quit
            // found key 257      backspace

            ti_recv(recv, 8);
            memset(intValue, 0x00, 10);
            for(int i=0; i < 8; i++) {
              if ( recv[i] == '\n' ) { break; }
              intValue[i] = recv[i];
            }
            int kc = atoi( intValue );
            //Serial.print("found key ["); Serial.print(kc); Serial.print("] ("); Serial.print( (char)kc ); Serial.print(") \n");
            if ( kc == 264 ) { kc = 27; }
            else if ( kc == 13 ) { kc = 10; }
            else if ( kc == 257 ) { 
              serPort.write( 8 );
              kc = 32;
            }
            else if ( kc == 4360 ) {
              // dirty trap
              serPort.println(F("EXIT DUMMY"));
              ti_recv(recv, 2+4+1);
              reboot();
              return;
            }
            Serial.print( (char)kc );
          
        }
        delay(1);
      } // end while
    }
  } else 
  // from my Basic/asmrt program...
  if ( recvNb == 0 && recv[0] == 'K' && recv[1] == ':' ) {
    //K:<code>\n

    // found key 97       a
    // found key 56       num
    // found key 338      arrow
    // found key 344      arrow
    // found key 264      Esc
    // found key 4360     2nd + Esc => Quit

    ti_recv(recv, 8);
    memset(intValue, 0x00, 10);
    for(int i=0; i < 8; i++) {
      if ( recv[i] == '\n' ) { break; }
      intValue[i] = recv[i];
    }
    int kc = atoi( intValue );
    Serial.print("found key ["); Serial.print(kc); Serial.print("] ("); Serial.print( (char)kc ); Serial.print(") \n");
  } else if ( recvNb == 0 && recv[0] == 't' && recv[1] == ':' /*&& recv[2] == '\n'*/ ) {
    // TODO : manage CTS requests
    ti_recv(recv, 1); // '\n'
    // time request
    ti_send((uint8_t*)"23:26:55",8);
  } else if ( recvNb == 0 && recv[0] != 0x00 && recv[0] != 0xFF ) {
    Serial.print("found chars ["); Serial.print( (char)recv[0] );Serial.print( (char)recv[1] ); Serial.print("] \n");
  }

  else if (Serial.available() > 0) {
    int len = Serial.available();
    if ( len > 0 ) {

      if ( Serial.peek() == 'b' ) {
        Serial.read();
        int d0 = Serial.read();
        int d1 = Serial.read();
        // doesn't fit in an int !!!!!!
        // 39350 bytes
        uint16_t blen = (d0*256)+d1;
        sendAbackup(blen);
      }

      else if (Serial.peek() == 'D') {
        Serial.read();
        dummyMode();
      } else
      if (Serial.peek() == '\\') {
        Serial.read();
        if (Serial.read() == 'S') {
          if (Serial.peek() == 'R') {
            Serial.read();
            reboot();
          } else if (Serial.peek() == 'B') {
            Serial.read();
            int d0 = Serial.read();
            int d1 = Serial.read();

            // doesn't fit in an int !!!!!!
            // 39350 bytes
            uint16_t blen = (d0*256)+d1;

            delay(500);

            outprintln("NO MORE SUPPORTED");
            // digitalWrite(13, LOW);
            // ti_sendbackup(blen);
            // digitalWrite(13, HIGH);

            return;
          } else {
            Serial.read();
            // send PRGM
            sendTiFile(false, true);
          }
        } else {
          if (Serial.read() == 'B') {
            ti_receiveBackup();
          }
        }
      }
    }

    uint8_t st[1];
    for(int i=0; i < len; i++) { st[0] = (uint8_t)Serial.read(); ti_send(st,1); }
    ti_resetLines();
  }

  if (PRESSED == digitalRead(BTN2)) {
    sendFlashFileToTi();
    return;
  }

  if (PRESSED == digitalRead(BTN3)) {
    ti_receiveBackup();
    return;
  }


  if (PRESSED != digitalRead(BTN)) {
    return;
  }
  Serial.println("go");
  
  ti_resetLines();
  
  data[1] = REQ_SCREENSHOT;
  ti_send(data, 4);
  delay(50);

  ti_resetLines();
  delay(100);
  recvNb = ti_recv(recv, 4); // => calculator's ACK
  // Serial.print("ACK "); Serial.print(recvNb); Serial.print(" "); 
  // Serial.print( recv[0],HEX ); Serial.print(" ");Serial.print( recv[1],HEX ); Serial.print(" ");Serial.print( recv[2],HEX ); Serial.print(" ");Serial.print( recv[3],HEX );
  // Serial.println("");
  
  ti_resetLines();
  // recvNb is always 0 !!! --> returns the available/remaining bytes ...
  recvNb = ti_recv(recv, 4); // ? 89 15 00 0F ? <TI92> <?> <LL> <HH> => 00 0F => 0F 00 = 3840 screen mem size
  if ( recvNb != 0 ) {
    Serial.println("TI did not ACK'ed, abort");
    return;
  }
  
  // Dumping screen rasetr
  for(int j=0; j < MAX_TI_SCR_SIZE; j+=SCREEN_SEG_MEM) {
    int howMany = (j+SCREEN_SEG_MEM) < MAX_TI_SCR_SIZE ? SCREEN_SEG_MEM : SCREEN_SEG_MEM - ( (j+SCREEN_SEG_MEM) % MAX_TI_SCR_SIZE );

    ti_recv(screen, howMany);
    dispScreenMem(howMany);
  }


  recvNb = ti_recv(recv, 2); // checksum from TI
  //Serial.println(recvNb);
  
  
  data[1] = REP_OK;
  ti_send(data, 4); // Arduino's ACK
  delay(50);
  Serial.println("go 3");
  
}


void sendFlashFileToTi() {
  char fqfn[32];
  memset(fqfn, 0x00, 32);

  int len = strlen_P( (const char*)FILE_NAME);
  for (int k = 0; k < len; k++) {
    char myChar = pgm_read_byte_near(FILE_NAME + k);
    fqfn[k] = myChar;
  }

  uint16_t flen = pgm_read_word_near( FILE_SIZE+0 );

  Serial.print(F("-= Sending : ")); Serial.print( (char*)fqfn ); Serial.println(F(" =-"));
  Serial.print(F("-= Bytes   : ")); Serial.print( flen ); Serial.println(F(" =-"));

  sendTiFile(true);

}

uint8_t tmp[32];
uint8_t b[1];
int tmpOffset = 0;
int tmpLength = 0;

void dummyMode() {
  serPort.write(0x06);
  int ch, l;//, l0,l1;
  while( true ) {

    while(serPort.available() == 0) { 
      //delay(1); 
      // @ this time : for Dump (max return is 16)
      
      int r = ti_recv(b, 1); // direct reading
      if ( r == 0 ) { tmp[ tmpOffset++ ] = b[0]; tmpLength++; }
    }

    ch = serPort.read();
    if ( ch == (int)'X' ) {
      serPort.write( 0x06 );
      break;
    } else if ( ch == (int)'S' ) {
      l = (serPort.read()*256) + serPort.read();
      // BEWARE if more than 514
      serPort.readBytes( screen, l );
      // ti_resetLines();
      
      ti_send(screen, l);
// //delay(5);
// ti_resetLines();
//       ti_recv(tmp, 8); // direct reading
      serPort.write( 0x06 ); //handshake - internal
    } else if ( ch == (int)'R' ) {
      l = (serPort.read()*256) + serPort.read();
      // BEWARE if more than 514
      // ti_resetLines();
      //ti_recv(screen, l);
      
      //ti_recv(tmp, l);

      for(int i=0; i < l; i++) {
        serPort.write(tmpOffset-tmpLength+i);  
      }
      tmpOffset -= l; // NOT CERTIFIIED
      tmpLength -= l;

    }
  }
}

uint16_t chksum(uint8_t* seg, int len) {
  uint16_t result = 0x0000;
  for(int i=4; i < len-2; i++) {
    result += seg[i];
  }
  result &= 0xFFFF;
  return result;
}

void sendAbackup(uint16_t dataLen) {
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




