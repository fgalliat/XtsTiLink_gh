#ifndef _ti_link_h
#define _ti_link_h 1

/**************************
* TI 8x - 9x link hardware routines
* 
* Xtase - fgalliat May2017
*
* thanks to Christopher Mitchell
* for ArTICL Lib. that inspired this work
**************************/

#define serPort Serial

// #define HEADLESS 1

#ifdef HEADLESS
 #define outprint(a) (a)
 #define outprintln(a) (a)
#else
 #define outprint(a) serPort.print(a)
 #define outprintln(a) serPort.println(a)
#endif

#ifdef TI_MODEL_92
  #define PC_2_TI 0x09
  #define TI_2_PC 0x89

  #define TI_SCREEN_WIDTH 240
  #define TI_SCREEN_HEIGHT 128
#else
  #error "No TI model selected";
#endif


// ------ Wiring ------------
 //White 
#define TIring 3
 //Red 
#define TItip 2

// ------ Timeouts ------------
#define TIMEOUT 300
#define GET_ENTER_TIMEOUT 1000
#define ERR_READ_TIMEOUT 300

#define DEFAULT_POST_DELAY 100

// ------------------------------------------------------------

static void ti_resetLines(bool reboot);
static int ti_send(uint8_t *data, uint32_t len);
static int ti_recv(uint8_t *data, uint32_t len, bool wait);

// ------------------------------------------------------------

void DBUG(uint8_t* data, int len) {
  for(int i=0; i < len; i++) {
      serPort.print( data[i]  < 16 ? "0" : "" ); serPort.print( data[i], HEX ); serPort.print( " " );
   }
   serPort.print( "\n" );
}

void __resetTILines(bool reboot=false) {
  pinMode(TIring, INPUT);           // set pin to input
  pinMode(TItip, INPUT);            // set pin to input
  if (reboot) {
    digitalWrite(TIring, LOW);       // for reset purposes
    digitalWrite(TItip, LOW);        // for reset purposes
  }
  digitalWrite(TIring, HIGH);       // turn on pullup resistors
  digitalWrite(TItip, HIGH);        // turn on pullup resistors
}

// ---------------

static int __par_put(uint8_t *data, uint32_t len) {
  int bit;
  uint32_t j;
  long previousMillis = 0;
  uint8_t byte;

  for (j = 0; j < len; j++) {
    byte = data[j];

    for (bit = 0; bit < 8; bit++) {
      previousMillis = 0;

      while ((digitalRead(TIring) << 1 | digitalRead(TItip)) != 0x03) {
        if (previousMillis++ > TIMEOUT)
          return -1;
          //return ERR_WRITE_TIMEOUT + j + 100 * bit;
      };

      if (byte & 1) {
        pinMode(TIring, OUTPUT);
        digitalWrite(TIring, LOW);
        previousMillis = 0;
        while (digitalRead(TItip) == HIGH) {
          if (previousMillis++ > TIMEOUT)
            return -1;
           // return ERR_WRITE_TIMEOUT + 10 + j + 100 * bit;
        };

        __resetTILines();
        previousMillis = 0;
        while (digitalRead(TItip) == LOW) {
          if (previousMillis++ > TIMEOUT)
            return -1;
           // return ERR_WRITE_TIMEOUT + 20 + j + 100 * bit;
        };
      } else {
        pinMode(TItip, OUTPUT);
        digitalWrite(TItip, LOW);     //should already be set because of the pullup resistor register
        previousMillis = 0;
        while (digitalRead(TIring) == HIGH) {
          if (previousMillis++ > TIMEOUT)
            return -1;
            //return ERR_WRITE_TIMEOUT + 30 + j + 100 * bit;
        };

        __resetTILines();
        previousMillis = 0;
        while (digitalRead(TIring) == LOW) {
          if (previousMillis++ > TIMEOUT)
          return -1;
            //return ERR_WRITE_TIMEOUT + 40 + j + 100 * bit;
        };
      }

      byte >>= 1;
    }
    delayMicroseconds(6);
  }
  return 0;
}

static int __par_get(uint8_t *data, uint32_t len) {
  int bit;
  uint32_t j;
  long previousMillis = 0;

  for (j = 0; j < len; j++) {
    uint8_t v, byteout = 0;
    for (bit = 0; bit < 8; bit++) {
      previousMillis = 0;
      while ((v = (digitalRead(TIring) << 1 | digitalRead(TItip))) == 0x03) {
        if (previousMillis++ > GET_ENTER_TIMEOUT)
          return ERR_READ_TIMEOUT + j + 100 * bit;
      }
      if (v == 0x01) {
        byteout = (byteout >> 1) | 0x80;
        pinMode(TItip, OUTPUT);
        digitalWrite(TItip, LOW);     //should already be set because of the pullup resistor register
        previousMillis = 0;
        while (digitalRead(TIring) == LOW) {            //wait for the other one to go low
          if (previousMillis++ > TIMEOUT)
            return ERR_READ_TIMEOUT + 10 + j + 100 * bit;
        }
        //pinMode(TIring,OUTPUT);
        digitalWrite(TIring, HIGH);
      } else {
        byteout = (byteout >> 1) & 0x7F;
        pinMode(TIring, OUTPUT);
        digitalWrite(TIring, LOW);     //should already be set because of the pullup resistor register
        previousMillis = 0;
        while (digitalRead(TItip) == LOW) {
          if (previousMillis++ > TIMEOUT)
            return ERR_READ_TIMEOUT + 20 + j + 100 * bit;
        }
        //pinMode(TItip,OUTPUT);
        digitalWrite(TItip, HIGH);
      }
      pinMode(TIring, INPUT);           // set pin to input
      digitalWrite(TIring, HIGH);       // turn on pullup resistors
      pinMode(TItip, INPUT);            // set pin to input
      digitalWrite(TItip, HIGH);        // turn on pullup resistors
    }
    data[j] = byteout;
    //delayMicroseconds(6);
  }
  return 0;
}

static void ti_resetLines(bool reboot=false) { __resetTILines(reboot); }
static int ti_send(uint8_t *data, uint32_t len) { return __par_put( data, len ); }
static int ti_recv(uint8_t *data, uint32_t len, bool wait=false) { 
  if (!wait) return __par_get( data, len ); 
  else {
    int res = -1;
    while ((res=__par_get( data, len )) !=0) { delay(1); }
    return res;
  }
}

// =================================================================

#endif