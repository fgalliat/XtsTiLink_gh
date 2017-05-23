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

//tios::globals		equ	tios@001C
//tios::main_lcd		equ	tios::globals+$0000	; $F00 bytes
//LCD_MEM			equ	tios::main_lcd

#define LCD_MEM tios__001C
#define LCD_MEM_SIZE 0x0F00

void moa_cls() {
  memset( &LCD_MEM, 0x00, LCD_MEM_SIZE);	
}

// The LIO functions in the TI-92 ROM functions are not exported by Fargo.
unsigned short (*LIO_SendData) (const void *src, unsigned long size); 
unsigned short (*LIO_RecvData) (void *dest, unsigned long size, unsigned long WaitDelay); 

