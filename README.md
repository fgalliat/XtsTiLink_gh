## XtsTiLink project

- project is as an early stage so : Ugly in a lot of parts
- I'll try to clean it as soon as possible

Project to transform TI92 (1st gen) to a Serial TTL Temrinal
using an Arduino as multipurposes "Link Cable"

![minimal wiring](/pictures/d_uno_wiring.jpg)

(Note that the backup sending code is limited to "1.12" version of the TI-ROM @ this time)

* /devl4ti/ -> the TI92 Fargo workspace for onTI Terminal software
* /tools/   -> Java tools & utilities to send backup & PRGMs to TI via the Arduino (Shell.java to prompt a cmdline to TI)
* /fs/      -> some content that can be sent to TI (it does not contains the used .92B because it's forbiden by law)
* /picures/ -> diagrams & photographs about the process of my TI reborn
* /docs/    -> guess ...
* ./        -> Arduino program for Serial Inteface

BEWARE w/ 3.3v Rapsberry logic
 - don't forget to protect RX line from Arduino 5v logic
![Full Wiring](/pictures/f_arduinoWiring.jpg) 



STILL HAVE TODO :
 - VT100 minimal command parsing/executing

@Xtase - fgalliat May 2017

![ScreenShot Dump](/pictures/c_screenshot.jpg)

![cmd Terminal](/pictures/g_terminal.jpg)

You're allowed to fork / modify / redistribute these contents since you mentionate me ....
