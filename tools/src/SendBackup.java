import java.io.*;
import java.util.ArrayList;
import java.util.List;

import jssc.SerialPort;
import jssc.SerialPortException;
import jssc.SerialPortTimeoutException;

/**************************
* TI 92 link backup sender
* 
* Xtase - fgalliat May2017
* 
**************************/


public class SendBackup {
  
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

	mainSendBackup();
	// _("ENTER"); kbd.readLine();
	Test.sendPrgmToTi( "../fs/fargo/flib.92p", !true, false );
	// _("ENTER"); kbd.readLine();
	Test.sendPrgmToTi("../fs/fargo/shell.92p", true, false );

  }


static int min(int a, int b) { return a < b ? a : b; }


  public static void mainSendBackup() throws Exception {
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

//headlessBackup = !true;

      System.out.println("begin");
	  _( readSerialLine(false) );
	  
      
      File p92 = new File("../fs/MOAFI.92B");
	  p92 = new File("../fs/MOAF.92B");
	  //p92 = new File("C:/vm_mnt/devl/TI_92/Fargo/backupsWfargo/back-fargo1/1-12/fargo.92b");

      int blen = (int)p92.length();
	  InputStream fis = new FileInputStream( p92 );

	  	//byte[] head = new byte[86 + 2]; //+2 for size...
		byte[] head = new byte[82]; 

	  	fis.read(head, 0, head.length); // skip firsts ?? bytes
	  	blen -= head.length;
		blen -= 2; // - final CHKSUM

    int d0 = blen / 256;
    int d1 = blen % 256;

	boolean archived = false;
	boolean silent = true;
	int sendingMode = -1;
	int fileType = -1; // ready for PRGM & ASM
      
    _(blen+" bytes");

	_("Enter to start");
	kbd = new BufferedReader( new InputStreamReader( System.in ) );
	kbd.readLine();

	serialPort.writeByte( (byte)'b');
	byte[] data = new byte[ 2 ];
	data[0] = i2b( blen/256 );
	data[1] = i2b( blen%256 );
	serialPort.writeBytes(data);

	int blocCpt = 0;
	long t0,t1;
	t0 = System.currentTimeMillis();

	while( true ) {
		int i = serialPort.readBytes(1)[0];
		if ( i == 1 ) {
			// send 512 bytes
			int ll = fis.available();
			if ( ll > 512 ) { ll = 512; }
			else { ll -= 2; }  
			byte[] dd = new byte[ll];
			outprintln("Sending "+ll+" bytes ("+blocCpt+")");
			int l = fis.read(dd, 0, dd.length);
			serialPort.writeBytes(dd);
			//_( readSerialLine(true) );

			blocCpt++;
			if ( ll < 512 ) { break; }

		} else {
			_( readSerialLine(false) );
		}

	}

	int i = serialPort.readBytes(1)[0];
	t1 = System.currentTimeMillis();

	outprintln("Backup sent "+((t1-t0)/1000)+"sec");



// halt();


      fis.close();
      
      System.out.println("finish");
	  _( readSerialLine(false) );
      
	  _("Enter to exit");
	  
	  kbd.readLine();

    } catch(Exception ex) {
      ex.printStackTrace();
    } finally {
      System.out.println("closing");
	  serialPort.closePort();// Close serial port
    }
    
  }

  // ================================================================

	static void DBUG(byte[] data, int len) {
		for(int i=0; i < len; i++) {
			System.out.print( data[i]  < 16 ? "0" : "" ); System.out.print( Integer.toHexString(b2i(data[i])) ); System.out.print( " " );
		}
		System.out.print( "\n" );
	}

  // =================================================================


  // =================================================================
  static void outprint(Object o) { System.out.print(o); }
  static void outprintln(Object o) { System.out.println(o); }
  static byte i2b(int i) { return (byte)i; }

  static int b2i(byte b) { int r = (int)b; return r<0?r+256:r; }

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
  
    static void delay(long time) {
		Zzz(time);
	}

}