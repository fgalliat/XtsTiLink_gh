/******************
* Ti-92 / V200 control Program w/ an Arduino
*
* Xtase - fgalliat @May2017 @Aug2020 @May2021
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
* - Kbd reading w/ a local PRGM (asm version & basic version)
* - getTime (needs CTS managment)
* - Silently send an STR Var / Get as CBL
* - ABLE to SEND a .92P file stored in flashMem (BEWARE 2KB RAM limit for one shot sending)
*    \___ can now call PRGM just after download (by TI Keyboard Controlling)
*    \___ able to send it as ARCHIVE or not
*    \___ able to Stream from FLASH (for long files support)
*    \___ able to Stream from SERIAL (for very-long files support)
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

#include "globals.h"

#if MODE_92P_ASM
 #include "xtsterm.92p.h"  // Works well on TI92-1 w/ Fargo
#else
 #include "keyb.92p.h"  // TiBasic only keyb version (non tokenized WLINK92.EXE)
#endif

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

// 3840 bytes for a TI-92 => too HUDGE to fit in RAM
#define MAX_TI_SCR_SIZE ( TI_SCREEN_HEIGHT*(TI_SCREEN_WIDTH/8) )

// to have complete scanlines
#define nbScanlines ( MAX_ARDUINO_MEM_SIZE / (TI_SCREEN_WIDTH/8) )

#define SCREEN_SEG_MEM ( nbScanlines * (TI_SCREEN_WIDTH/8) )

// ===================== RAM Settings ==================

uint8_t screen[SCREEN_SEG_MEM];

void dummyMode();

 // ====================================================

  uint8_t data[4] = { PC_2_TI, REQ_SCREENSHOT, 0x00, 0x00 }; // request screen shot (silent mode)
  uint8_t recv[10];
  char intValue[10];

void setup() {
  __SCREEN_SEG_MEM = SCREEN_SEG_MEM;

  ti_resetLines();

  pinMode(BTN, INPUT_PULLUP);
  pinMode(BTN2, INPUT_PULLUP);
  pinMode(BTN3, INPUT_PULLUP);
  digitalWrite(13, LOW);
  
  // init & clean serial port =======
  Serial.begin(115200);
  //serPort.begin(9600);
  serPort.setTimeout(1000); // default value
  serPort.flush();
  while(serPort.available() > 0) { serPort.read(); }
  // ================================
}

  
  void dispScreenMem(int len, bool ascii) {

    if ( !ascii ) {
      serPort.write( screen, len );
      return;
    }

    // Dump the screen to the serial console
    for (int i = 0; i < len; i++) {
      for (int j = 7; j >= 0; j--) {
        if (screen[i] & (1 << j)) {
          serPort.write('#');
        } else {
          serPort.write('.');
        }
      }
      if (i % (TI_SCREEN_WIDTH/8) == (TI_SCREEN_WIDTH/8)-1) { // 240/8 => 30 bytes
        serPort.println();
      }
    }
  }
 

void reboot() {
  ti_resetLines(true);
  asm volatile ("  jmp 0");
}

#define DBUG_DUMMY true

void dummyMode() {
  int recvNb;
  if (DBUG_DUMMY) serPort.println(F("DUMMY"));

  serPort.setTimeout(400);

  ti_resetLines();

  // see : https://internetofhomethings.com/homethings/?p=927
      const int MAX_READ_LEN = 32;

      memset(screen, 0x00, MAX_READ_LEN+1);
      int fullPacketLen = 0;

      int toRead;
      int cpt=0;
      int kc;

      char chs[2] = { 0x00, 0x00 };

      char msg[30];

      while(true) {

        while ( Serial.available() > 0 ) {
          int t = min( MAX_READ_LEN, Serial.available() ); // read only what's available 
          int ch = serPort.readBytes( screen, t );
          if ( ch <= 0 ) { break; }

          ti_send( screen, ch );
        }

        recvNb = ti_recv(recv, 2);
        if ( recvNb == 0 ) {
          if ( recv[0] == 'X' && recv[1] == ':' ) {
            recvNb = ti_recv(recv, 4+1);
            //serPort.print( recvNb ); serPort.print( F(" ") ); serPort.print( recv[0] ); serPort.print( F(" ") ); serPort.println( recv[1] ); 
            //if ( recvNb == 0 && recv[0] == 0xFF && recv[1] == 'e' ) {
            if ( recvNb == 0 && recv[0] == '?' && recv[1] == 'e' ) {
              // X:end\n -> end of serial session
              // break;
              if (DBUG_DUMMY) serPort.println(F("EXIT DUMMY"));
              reboot();
              return;
            } 
          } else if ( recv[0] == 'K' && recv[1] == ':' ) {
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
                if ( recv[i] == '\n' ) { intValue[i] = 0x00; break; }
                intValue[i] = recv[i];
              }
              kc = atoi( intValue );
              //Serial.print("found key ["); Serial.print(kc); Serial.print("] ("); Serial.print( (char)kc ); Serial.print(") \n");
              if ( kc == 264 ) { 
                // Esc
                kc = 27; 
                serPort.write( kc );
              }
              else if ( kc == 13 ) { 
                // Enter
                kc = 10; 
                serPort.write( kc );
              }
              else if ( kc == 257 ) { 
                // BackSpace
                serPort.write( 8 );
                serPort.write( 32 );
                serPort.write( 8 );
              }
              else if ( kc >= 337 && kc <= 348 ) {
                // Arrows key
                if ( kc == 338 ) { // UP
                  serPort.write( 27 );
                  serPort.print(F("[A"));
                } else if ( kc == 344 ) { // DOWN
                  serPort.write( 27 );
                  serPort.print(F("[B"));
                } else if ( kc == 337 ) { // LEFT
                  serPort.write( 27 );
                  serPort.print(F("[D"));
                } else if ( kc == 340 ) { // RIGHT
                  serPort.write( 27 );
                  serPort.print(F("[C"));
                } 
              }
              else if ( kc == 4360 ) {
                // 2nd + Quit
                // - dirty trap -
                if (DBUG_DUMMY) serPort.println(F("EXIT DUMMY"));
                ti_recv(recv, 2+4+1);
                reboot();
                return;
              }
              else if (kc > 0 && kc < 256) {
                serPort.write( kc );
              } else {
                serPort.print(">K:");
                serPort.println( intValue );
              }
          } // end of key read

        } // end if recvNb == 0
        
        //delay(5);
      } // end while dummy

      if (DBUG_DUMMY) serPort.println(F("LEAVING DUMMY MODE"));
}

// to Serial ASCII for now
void dumpScreen(bool ascii=true) {
  int recvNb = -1;
  ti_resetLines();
  
  data[1] = REQ_SCREENSHOT;
  ti_send(data, 4);
  delay(50);

  ti_resetLines();
  delay(100);
  recvNb = ti_recv(recv, 4); // => calculator's ACK
  ti_resetLines();
  // recvNb is always 0 !!! --> returns the available/remaining bytes ...
  recvNb = ti_recv(recv, 4); // ? 89 15 00 0F ? <TI92> <?> <LL> <HH> => 00 0F => 0F 00 = 3840 screen mem size
  if ( recvNb != 0 ) {
    serPort.println(F("E:TI did not ACK'ed, abort"));
    return;
  }
  
  if ( !ascii ) {
    serPort.write( (uint8_t)(MAX_TI_SCR_SIZE >> 8) );
    serPort.write( (uint8_t)(MAX_TI_SCR_SIZE % 256) );
  }

  // Dumping screen raster
  for(int j=0; j < MAX_TI_SCR_SIZE; j+=SCREEN_SEG_MEM) {
    int howMany = (j+SCREEN_SEG_MEM) < MAX_TI_SCR_SIZE ? SCREEN_SEG_MEM : SCREEN_SEG_MEM - ( (j+SCREEN_SEG_MEM) % MAX_TI_SCR_SIZE );

    ti_recv(screen, howMany);
    dispScreenMem(howMany, ascii);
  }

  recvNb = ti_recv(recv, 2); // checksum from TI
  //Serial.println(recvNb);
  
  data[1] = REP_OK;
  ti_send(data, 4); // Arduino's ACK
  delay(50);
  // serPort.println("go 3");
}

void sendText(char* txt, bool CR=false) {
  ti_sendKeyStrokes(txt);
  if ( CR ) { ti_sendKeyStroke(0x0D); }
}

void wakeUpCalc() {
  ti_sendKeyStroke(267); // ON Key
}

// doesn't work
// void sleepCalc(bool closePrgm) {
//   if ( closePrgm ) {
//     ti_sendKeyStroke(4360); // 2nd+QUIT Keys
//   }
//   //ti_sendKeyStroke(8459); // Diamond+ON Key (not really OFF key...)
// }


#define CBL_92 0x19
#define CALC_92 0x89
#define ACK REP_OK
#define CTS 0x09

void CBL_ACK() {
  static uint8_t E[4] = { CBL_92, ACK, 0x00, 0x00 };
  int ok = ti_send(E, 4);
  if ( ok != 0 ) { Serial.print(F("(!!) fail sending CBL-ACK : ")); Serial.println(ok); }
  delay(3);
}

void CBL_CTS() {
  static uint8_t E[4] = { CBL_92, CTS, 0x00, 0x00 };
  int ok = ti_send(E, 4);
  if ( ok != 0 ) { Serial.println(F("(!!) fail sending CBL-CTS")); }
  delay(3);
}

void relaunchKeybPrgm() {
  Serial.println(F("I:RELAUNCH keyb"));
  ti_sendKeyStroke(264); // Esc
  delay(150);
  ti_sendKeyStroke(263); // Clear
  delay(150);

  ti_sendKeyStrokes("main\\keyb()");
  ti_sendKeyStroke(0x0D);
}

void debugDatas(uint8_t* data, int len) {
  for (int i=0; i < len; i++) {
    Serial.print( data[i], HEX ); Serial.print( F(" ") );
  }
  Serial.println();
}


void loop() {
  int recvNb = -1;

  digitalWrite(13, HIGH);
  recvNb = ti_recv(recv, 2);
  
  // ASM version PRGM starts
  if ( recvNb == 0 && recv[0] == 'X' && recv[1] == ':' ) {
    recvNb = ti_recv(recv, 6+1);
    //serPort.print( recvNb ); serPort.print( F(" ") ); serPort.print( recv[0] ); serPort.print( F(" ") ); serPort.println( recv[1] ); 
    //if ( recvNb == 0 && recv[0] == 0xFF && recv[1] == 'b' ) {
    if ( recvNb == 0 && recv[0] == '?' && recv[1] == 'b' ) {
      // X:begin\n
      // dummy serial mode : XtsTerm.92p

      dummyMode();
    }
  }

  // dummy from Ti
  else if ( recvNb == 0 ) {
    // SendCalc c (c=102)
    // 89 6 7 0 5 0 0 0 0 1 63 69 0 

    // CBL : Send {1}
    // 89 6 7 0 3 0 0 0 4 1 FF 7 1 
    // fixe pour une liste de 1 arg

    // Ti Voyage 200 - Send {c} c = 53
    // 89 6 8 0 4 0 0 0 4 1 FF 0 8 
    // Ti Voyage 200 - Send {1}
    // 89 6 8 0 3 0 0 0 4 1 FF 0 7 

    #define DBG_CBL 1

    // #if DBG_CBL
    //   debugDatas(recv, 2);
    // #endif

    // HERE : the Ti sends something by itself (CBL, Var)
    bool cblSend = false;
    bool varSend = false;
    bool varSend2 = false;

    if ( recv[0] == 0x89 && recv[1] == 0x06 ) {
      cblSend = true;
    } else if ( recv[0] == 0x89 && recv[1] == 0x68 ) {
      varSend = true;
    } else if ( recv[0] == 0x88 && recv[1] == 0x06 ) {
      varSend2 = true;
    } else {
      Serial.print(F("E:Not a KNOWN Header : "));
      debugDatas( recv, 2 );
    }

    // should it be a data type ?
    // 07 & 08 for CBL data - 0 whwn Ti Sends a Var
    uint8_t headLength[1];
    ti_recv( headLength, 1 );
    #if DBG_CBL
     Serial.print(F("i:Len to read : "));Serial.println(headLength[0], HEX);
    #endif

    // 7 is 11 for ti92
    // 8 is 12 for ti voyage 200
    /*const*/ int head = ( headLength[0] + 4 ) -1; // -1 for head len

if ( varSend ) {
  // 1st step only
  // just a try ==> don"t know why -- TiVoyage200
  // have to read : 0 55 1 55 1 53 2 65 0 1 1 80 7F 0 0 0 8 CB 7 48 8 CB F B1 
  head = 24;
}

    uint8_t sendHead[head]; // 2 frst already read ...
    recvNb = ti_recv( sendHead, head );
    if ( recvNb != 0 || sendHead[ 6-1 ] != 0x04 ) { // -1 for head len
      Serial.println(F("E:Not CBL packet"));
      debugDatas( sendHead, head );
      cblSend = false;
    }
    // #if DBG_CBL
    //   debugDatas(sendHead, head);
    // #endif

    if ( varSend ) {
      if (!false) Serial.println(F("Send for TiVarSend"));
      // just ignore this time wait for next packet reading loop
    } else if ( varSend2 ) {
      if (!false) Serial.println(F("Send for TiVarSend 2nd step"));
      recvNb = ti_recv( sendHead, head );
      // -> 8 0 0 0 = could be the var size LSB -> MSB (A + (B * 256) + (C * 65536) + ... )
      // C = var Type : String
      // 1 = name len => 
      // 64 => 'd' : name
      // 79 0 => can be CHK
      // 0 8 0 0 0 C 1 64 0 79 0
      debugDatas( sendHead, head );

      int vnLen = sendHead[6];
      char varName[8+1]; memset(varName, 0x00, 8+1);
      for(int i=0; i < vnLen; i++) { varName[i]= sendHead[6+1+i]; }
      if (!false) { Serial.print(F("TiVarSend >>")); Serial.println(varName); }

      // 0x89 -> 0x09 - Ti92
      // 0x88 -> 0x08 - Ti89 & tiVoyage200
      const static uint8_t cACK[] = { 0x08, REP_OK, 0x00, 0x00 };
      const static uint8_t cCTS[] = { 0x08, CTS, 0x00, 0x00 };
      ti_send( cACK, 4 ); // ACK
      ti_send( cCTS, 4 ); // CTS

      ti_recv( TMP_RAM, 4 );
      if (!false) Serial.println(F("TiVarSend Waits data"));

      // FIXME !!! = have to get len to read = see var size upper
      //       v                  a  b  c      ch ch 
      // 88 15 C 0 0 0 0 0 0 6 0 61 62 63 0 2D 59 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0
      memset( TMP_RAM, 0x00, 32 );
      recvNb = ti_recv( TMP_RAM, 32 );
      debugDatas( TMP_RAM, 32 );

      ti_send( cACK, 4 ); // ACK datas
      recvNb = ti_recv( TMP_RAM, 4 ); // read EOT
      ti_send( cACK, 4 ); // ACK EOT

    } else

    if ( cblSend ) {
      if (false) Serial.println(F("Send for CBL"));
      CBL_ACK();
      CBL_CTS();

      // 89 56 0 0 - Ti :ACK
      recvNb = ti_recv( recv, 4 );
      if ( recvNb != 0 ) {
        Serial.println(F("E:CBL/ACK"));

        #if DBG_CBL
          debugDatas(recv, 4);
        #endif

        if (!true) { relaunchKeybPrgm(); return; }
      }

// 89 6 7 0 3 0 0 0 4 1 FF 7 1
// 89 15 7 0 1 0 0 0 20 31 00 52 0 // Send {1}
// 89 15 7 0 1 0 0 0 20 32 00 53 0 // Send {2}
// 89 15 7 0 1 0 0 0 20 33 00 54 0 // Send {3}

// 89 6 7 0 5 0 0 0 4 1 FF 9 1
// 89 15 9 0 1 0 0 0 20 32 35 35 0 BD 0 // Send {255}
//                       2  5  5 as chars

// Send {102,103} -> 20 -> ',' (aka ' ')
// 89 6 7 0 9 0 0 0 4 1 FF D 1 
// 89 15 D 0 2 0 0 0 20 31 30 32 20 31 30 33 0 69 1

// 32(h) -> 50(10) -> '2'
// 35(h) -> 53(10) -> '5'
// from byte #9 sent as string (Cf compat Ti82 legacy ...)
// from 0x20 to 0x00 or read len in headers

//       L M 1 2 3 4 5  6  7  CHK CHK
// 89 15 7 0 1 0 0 0 20 33 00 54 0 // Send {3}

#if 1
    recvNb = ti_recv( recv, 4 );
    if ( recvNb != 0 ) {
      Serial.println(F("E:CBL/DT-HEAD"));
      #if DBG_CBL
        debugDatas(recv, 4);
      #endif
    }

    int d0 = recv[2];
    int d1 = recv[3];
    int dataLen = ( d1 << 8 ) + d0;

    uint8_t cbldata[dataLen+2]; // +2 for CHK

    recvNb = ti_recv( cbldata, dataLen+2 );
    if ( recvNb != 0 ) {
      Serial.println(F("E:CBL/DT-VAL"));
    }

    cbldata[dataLen] = 0x00; // remove CHK
    cbldata[dataLen+1] = 0x00; // remove CHK

    char* value = &cbldata[5]; // just after 0x20, ends w/ 0x00
#if 0
    Serial.print(F("CBL:"));
    Serial.println(value);
#else
    int kc = atoi( value ); // assumes that there is only one value in { list }

    char msg[4];
    msg[0] = 'K';
    msg[1] = kc < 256 ? (char)kc : 0xFF;
    msg[2] = kc >> 8; // beware often be 0x00
    msg[3] = kc % 256;
    Serial.write(msg, 4);

#endif

#else
    bool first20 = false;
    bool last0 = false;
    do {
      recvNb = ti_recv( recv, 1 );
      if ( recvNb != 0 ) { break; }

      if ( !first20 && recv[0] == 0x20 ) {
        first20 = true;
      } else if ( first20 && recv[0] == 0x00 ) {
        first20 = false;
        last0 = true;
      } else if ( cblSend && first20 && !last0) {
        // Serial.print( recv[0], HEX ); Serial.print( F(" ") );
        Serial.print( (char)recv[0] );
      }
 
    } while(true);
Serial.println("");
#endif

      CBL_ACK();

      // 89 92 0 0 - Ti : EOT
      recvNb = ti_recv( recv, 4 );
      if ( recvNb != 0 ) {
        Serial.println(F("E:CBL/EOT"));
      }
      CBL_ACK();
    }
  }


  // // from my Basic/asmrt program...
  // else if ( recvNb == 0 && recv[0] == 'K' && recv[1] == ':' ) {
  //   //K:<code>\n

  //   // found key 97       a
  //   // found key 56       num
  //   // found key 338      arrow
  //   // found key 344      arrow
  //   // found key 264      Esc
  //   // found key 4360     2nd + Esc => Quit

  //   ti_recv(recv, 8);
  //   memset(intValue, 0x00, 10);
  //   for(int i=0; i < 8; i++) {
  //     if ( recv[i] == '\n' ) { break; }
  //     intValue[i] = recv[i];
  //   }
  //   int kc = atoi( intValue );
  //   Serial.print("found key ["); Serial.print(kc); Serial.print("] ("); Serial.print( (char)kc ); Serial.print(") \n");
  // } else if ( recvNb == 0 && recv[0] == 't' && recv[1] == ':' /*&& recv[2] == '\n'*/ ) {
  //   // TODO : manage CTS requests
  //   ti_recv(recv, 1); // '\n'
  //   // time request
  //   ti_send((uint8_t*)"23:26:55",8);
  // } else if ( recvNb == 0 && recv[0] != 0x00 && recv[0] != 0xFF ) {
  //   Serial.print("found chars ["); Serial.print( (char)recv[0] );Serial.print( (char)recv[1] ); Serial.print("] \n");
  // }

  //else 
  
  if (serPort.available() > 0) {

      if ( serPort.peek() == 'b' ) {
        serPort.read();
        int d0 = serPort.read();
        int d1 = serPort.read();
        // doesn't fit in an int !!!!!!
        // 39350 bytes
        uint16_t blen = (d0*256)+d1;
        sendAbackup(blen);

        reboot();
        return;
      }

      else if (serPort.peek() == '\\') {
        serPort.read();
        if (serPort.peek() == 'S') {
          serPort.read();
          if (serPort.peek() == 'R') {
            serPort.read();
            reboot();         // Reboot Arduino
          } else if (serPort.peek() == 'S') {
            serPort.read();
            dumpScreen();     // DumpScreen in ASCII mode
            reboot();
            return;
          } else if (serPort.peek() == 's') {
            serPort.read();
            dumpScreen(false); // DumpScreen in BINARY mode
            reboot();
            return;
          } else if (serPort.peek() == 'T') {
            serPort.read();
            sendText("Hello World !", false);
          } else if (serPort.peek() == 'P') {
            serPort.read();
            sendTiFile(false, true);  // send PRGM from Serial (silent mode)
            reboot();
            return;
          } else if (serPort.peek() == 'p') {
            serPort.read();
            int err = receiveTiVar("main\\d"); // receive TiVar to Serial (silent mode)
            if ( err != 0 ) {
              Serial.print("Err : "); Serial.println(err);
            }
            delay(1000);
            reboot();
            return;
          } 
          else if (serPort.peek() == 'K') {
            serPort.read();
            sendFlashFileToTi(); // send PRGM from flashMem (silent mode)
            reboot();
            return;
          } else if (serPort.peek() == 'L') {
            serPort.read();
            // launch PRGM
            relaunchKeybPrgm();
            reboot();
            return;
          } else if ( serPort.peek() == 'W' ) {
            serPort.read();
            wakeUpCalc();
            reboot();
            return;
          } 
          // else if ( serPort.peek() == 'w' ) {
          //   serPort.read();
          //   sleepCalc(true);
          //   reboot();
          //   return;
          // }

          else if (serPort.peek() == 'D') {
            serPort.read();
            // DUMMY commanded mode
            dummyMode();
            reboot();
            return;
          }
        } else {
          if (serPort.peek() == 'B') {
            serPort.read();
            ti_receiveBackup(); // NOT fully CRETIFIED
          }
        }
      }

    ti_resetLines();
  }

  // if (PRESSED == digitalRead(BTN2)) {
  //   sendFlashFileToTi();
  //   return;
  // }
  // if (PRESSED == digitalRead(BTN3)) {
  //   ti_receiveBackup();
  //   return;
  // }
  // if (PRESSED != digitalRead(BTN)) {
  //   return;
  // }
  // serPort.println("go");
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

  Serial.print(F("I:-= Sending : ")); Serial.print( (char*)fqfn ); Serial.println(F(" =-"));
  Serial.print(F("I:-= Bytes   : ")); Serial.print( flen ); Serial.println(F(" =-"));

  sendTiFile(true);
}

