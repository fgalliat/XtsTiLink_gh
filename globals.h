/**
 * XtsTiLink_gh
 * 
 * Xtase - fgalliat @Aug2020 @Jun2021
 * 
 * Arduino Vs Ti92 / Voyage 200
 * 
 */


// set your mode to 1 (only one)
#define MODE_92P_ASM 0
#define MODE_92P_BAS 1

// 1 - for Human readable display (when use a SerialTerm)
// 0 - for Binary readable packets (when communicate w/ another MCU)
#define ASCII_OUTPUT 0

#if not ASCII_OUTPUT

 // DummyMode
 #define OUT_BIN_ENTER_DUMMY "\x89\xFE"
 #define OUT_BIN_EXIT_DUMMY  "\x89\xFF"

 // Ti Send Var
 #define OUT_BIN_SENDVAR_NAME "\x89\x06\x01"
 #define OUT_BIN_SENDVAR_TYPE "\x89\x06\x02"
 #define OUT_BIN_SENDVAR_SIZE "\x89\x06\x03"
 #define OUT_BIN_SENDVAR_DATA "\x89\x06\x04"
 #define OUT_BIN_SENDVAR_EOF  "\x89\x06\x05"

 // Ti Send CBL Value(s)
 #define OUT_BIN_SENDCBL "\x89\xCB"

#endif