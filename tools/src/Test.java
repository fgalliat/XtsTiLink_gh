import java.io.*;
import java.util.ArrayList;
import java.util.List;

import jssc.SerialPort;
import jssc.SerialPortException;
import jssc.SerialPortTimeoutException;

/**************************
* TI 92 link program sender
* 
* Xtase - fgalliat May2017
* 
**************************/

public class Test {
  
  static SerialPort serialPort = null;
  static BufferedReader kbd = null;

  static void halt() {
    try {
      System.out.println("closing");
      serialPort.closePort();// Close serial port
    } catch(Exception ex) {}
    System.exit(0);
  }
  
  public static void main(String[] args) throws Exception {
	 //mainRecv( null );

	//    if ( args != null && args.length >= 1 ) {
	// 	   if ( args[0].equalsIgnoreCase("92b") ) {
	// 			mainSend( args.length >= 2 );
	// 		} else {
	// 			mainSendPrgm(true, "../fs/fargo/"+ args[0] +".92p", "main\\"+ args[0] +"" );
	// 		}
	// 	} else {
	// 		//mainSendPrgm(true, "../fs/bbb.92p", "main\\bbb" );
	// 		mainSendPrgm(true, "../fs/TetrisGB.9xz", "main\\atet" );
	// 	}

	// mainSendPrgm(true, "../fs/fargo/flib.92p", "main\\flib" );
	// mainSendPrgm(true, "../fs/fargo/shell.92p", "main\\shell" );

	if ( args.length > 0 ) {
		sendPrgmToTi(args[0], args.length>1, true);
	} else {
		_("Na args supplied");
	}

  }

  public static void sendPrgmToTi(String filename, boolean autoexec, boolean kbdWait) throws Exception {
	String varName = new File(filename).getName();
	// volountary not lastIndexOf(..)
	varName = varName.substring( 0, varName.indexOf(".") );
	varName = "main\\"+varName;
	mainSendPrgm(autoexec, filename, varName, kbdWait );
  }



public static void mainRecv(String[] args) throws Exception {
    serialPort = new SerialPort(args != null && args.length > 0 ? args[0] : Arduino.commPort /*"/dev/ttyS0"*/ );
    try {
      System.out.println("opening " + serialPort.getPortName());
      serialPort.openPort();// Open serial port

      System.out.println("setting");
      serialPort.setParams(SerialPort.BAUDRATE_115200, SerialPort.DATABITS_8, SerialPort.STOPBITS_1, SerialPort.PARITY_NONE);
      serialPort.setFlowControlMode(SerialPort.FLOWCONTROL_NONE);
      
	  // makes arduino reset when openeing port
	  Thread.sleep(2000);

_("Enter to start");
	  BufferedReader kbd = new BufferedReader( new InputStreamReader( System.in ) );
	  kbd.readLine();

      System.out.println("begin");
	     
serialPort.writeBytes( new byte[] { '\\', 'R', 'B' } );
 _("waits");
      _( readSerialLine(true) );

FileOutputStream out = new FileOutputStream("../fs/test.92b");

	while(true) {
		int ch0 = serialPort.readBytes(1)[0];
		int ch1 = serialPort.readBytes(1)[0];
		if ( ch0 < 0 ) { ch0 += 256; }
		if ( ch1 < 0 ) { ch1 += 256; }
		int len = (ch0*256)+ch1;


		_("receiving "+len+" bytes");
if (len <= 0 || len > 1028) { break; }
		//if (len <= 0 || len > 1028) { break; }

		// only 514 ???
		// byte[] rcv = serialPort.readBytes(len);
		// _("receivd "+rcv.length+" bytes");

		// out.write( rcv, 4, len-4 ); // skips 4 firsts bytes of each blocs

// 		byte[] rcv = serialPort.readBytes( min(len, 514) );
// 		_("a.receivd "+rcv.length+" bytes");
// 		out.write( rcv, 4, min(len, 514)-4 ); // skips 4 firsts bytes of each blocs

// if ( len > 514 ) {
//  		rcv = serialPort.readBytes(514);
// 		_("b.receivd "+rcv.length+" bytes");
// 		out.write( rcv, 0, 514 );
// }

byte[] rcv = new byte[1];
for(int i=0; i < len; i++) {
	rcv = serialPort.readBytes(1);
	int ch = rcv[0];
	if ( ch < 0 ) { ch+= 256; }
	if ( i>=4 ) {
		out.write(ch);
	}
}


// if ( len < 1028 ){
// 	break;
// }

	}	  
_( readSerialLine(true) );
out.flush();
out.close();

      System.out.println("finish");
      
	  _("Enter to exit");
	  
	  kbd.readLine();;

    } catch(Exception ex) {
      ex.printStackTrace();
    } finally {
      System.out.println("closing");
	  serialPort.closePort();// Close serial port
    }
	}


static int min(int a, int b) { return a < b ? a : b; }



  // =================================================================
  public static void mainSendPrgm(boolean autoRun, String fileName, String varName, boolean kbdWait) throws Exception {
	serialPort = new SerialPort(Arduino.commPort);
    try {
      System.out.println("opening " + serialPort.getPortName());
      serialPort.openPort();// Open serial port

      System.out.println("setting");
      serialPort.setParams(SerialPort.BAUDRATE_115200, SerialPort.DATABITS_8, SerialPort.STOPBITS_1, SerialPort.PARITY_NONE);
      serialPort.setFlowControlMode(SerialPort.FLOWCONTROL_NONE);
      
	  // proMini doesn't resets @ each serPort.open()
	   Arduino.resetArduino(serialPort);
	//   // makes arduino reset when openeing port
	//   Thread.sleep(2000);

      System.out.println("begin");

	  // flush garbage
	  _( "G:"+readSerialLine(false) );
      
      File b92 = new File(fileName);
	  //File b92 = new File("../fs/backup.92B");
      
      int blen = (int)b92.length();
      
	  InputStream fis = new FileInputStream( b92 );

	  	//byte[] head = new byte[86+2]; //+2 for size...
		byte[] head = new byte[86];
	  	fis.read(head);

	  int d0 = fis.read();
      int d1 = fis.read();
	  blen = (d0*256)+d1;

      _(blen+" bytes");

	  _("Enter to start");
	  kbd = new BufferedReader( new InputStreamReader( System.in ) );
	  if (kbdWait) kbd.readLine();

      serialPort.writeBytes( new byte[] { '\\', 'S', 'P', (byte)d0, (byte)d1 } );
	  serialPort.writeBytes( varName.getBytes() );
	  serialPort.writeByte( (byte)0x00 );

	  if (fileName.toLowerCase().endsWith(".92p")) { serialPort.writeByte( (byte)0x01 ); }
	  else { serialPort.writeByte( (byte)0x00 ); }

	  if (autoRun) { serialPort.writeByte( (byte)0x01 ); }
	  else { serialPort.writeByte( (byte)0x00 ); }
      
      _("waits");
      _( readSerialLine(true) );
      
      //byte[] data = new byte[blen];
      

      	_("Var "+ varName +" to "+ blen +" copy");
		  // wait for '0x01' ready to send
      	int ch = serialPort.readBytes(1)[0];
      	
      	_( ch );
      	
		long t0,t1;
		t0 = System.currentTimeMillis();

      	if ( ch == 1 ) {
      		_("->");
      		// int readed = fis.read( data, 0, blen );
			// System.out.println(readed+" bytes");
      		// serialPort.writeBytes( data );


			serialPort.writeByte((byte)0x00); // ACK

			//final int BLOC_LEN = 128;
			final int BLOC_LEN = 514;
			byte[] data = new byte[BLOC_LEN]; int e;
      		for(int j=0; j < blen; j+=BLOC_LEN) {

				e = BLOC_LEN;
				if ( j+BLOC_LEN > blen ) { e = blen-j; }

				int re = fis.read(data, 0, e);

				// own cts
				ch = serialPort.readBytes(1)[0];
				if ( ch != 2 ) { _("Oups2"); break; }

      			serialPort.writeBytes( data ); // beware of end of file
      		} 
      		_("<-");
			t1 = System.currentTimeMillis();

			_( readSerialLine(!true) ); // chksum line      
			_( readSerialLine(!true) ); // var sent
			_( readSerialLine(!true) );

			_("took "+(t1-t0)+" msec to copy");

			// flush garbage
	  		_( "G:"+readSerialLine(false) );

      	}	else {
      		_("(EE)");
			System.out.print( (char)ch );
      		_( readSerialLine() );
      	}

      
      fis.close();

	  //SendBackup.resetArduino(serialPort);


      System.out.println("finish");
      
	  _("Enter to exit");
	  
	  if (kbdWait) kbd.readLine();;

    } catch(Exception ex) {
      ex.printStackTrace();
    } finally {
      System.out.println("closing");
	  serialPort.closePort();// Close serial port
    }
    
  }
  // =================================================================
  
  public static void _(Object o) {
		System.out.println(o);
	}

	public static void printSerialLine(String str) throws Exception {
		printSerialLine(str, !true);
	}

	public static void printSerialLine(String str, boolean consumeEcho) throws Exception {
		str += "\r\n";
		serialPort.writeBytes(str.getBytes());
		if (consumeEcho) {
			serialPort.readBytes(str.length());
		}
	}

	public static String readSerialLine() throws Exception {
		return readSerialLine(false);
	}

	public static String readSerialLine(boolean wait1stLineInfinitly) throws Exception {
		String ret = "";
		try {
			// char lastCh = 0x00;
			while (true) {
				char ch = 0x00;
				if (wait1stLineInfinitly) {
					ch = (char) serialPort.readBytes(1)[0];
				} else {
					ch = (char) serialPort.readBytes(1, 1000)[0];
				}
				if (ch < 0) {
					ch += 256;
				}
				// may miss the 2nd return char

				// System.out.print(ch);

				// if (ch == 13 || ch == 10) {
				if (ch == 13) {
					// break;
					continue;
				}
				if (ch == 10) {
					break;
				}
				ret += ch;
			}
		} catch (SerialPortTimeoutException ex) {
			return null;
		}
		return ret;
	}

	public static void Zzz(long time) {
		try {
			Thread.sleep(time);
		} catch (Exception ex) {
		}
	}
  
}