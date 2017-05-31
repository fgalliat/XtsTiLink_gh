import java.io.*;
import java.util.ArrayList;
import java.util.List;

/**************************
* TI 92 link backup sender
* 
* Xtase - fgalliat May2017
* 
**************************/


public class SendBackup {
  
  static BufferedReader kbd = null;

  static void halt() {
    ArduinoMCU.close();// Close serial port
    System.exit(0);
  }
  
  public static void main(String[] args) throws Exception {

	mainSendBackup();
	Test.sendPrgmToTi( "../fs/fargo/flib.92p", !true, false, false );
	Test.sendPrgmToTi("../fs/fargo/shell.92p", true, false, false );

  }


static int min(int a, int b) { return a < b ? a : b; }


  public static void mainSendBackup() throws Exception {
    try {
      ArduinoMCU.open();
      
	  // proMini doesn't resets @ each serPort.open()
	  ArduinoMCU.reset();

		System.out.println("begin");
		_( ArduinoMCU.readSerialLine(false) );
		
		
		File p92 = new File("../fs/MOAFI.92B");
		p92 = new File("../fs/MOAF.92B");

		int blen = (int)p92.length();
		InputStream fis = new FileInputStream( p92 );

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

		ArduinoMCU.writeByte( (byte)'b');
		byte[] data = new byte[ 2 ];
		data[0] = i2b( blen/256 );
		data[1] = i2b( blen%256 );
		ArduinoMCU.writeBytes(data);

		int blocCpt = 0;
		long t0,t1;
		t0 = System.currentTimeMillis();

		while( true ) {
			int i = ArduinoMCU.readOneByte();
			if ( i == 1 ) {
				// send 512 bytes
				int ll = fis.available();
				if ( ll > 512 ) { ll = 512; }
				else { ll -= 2; }  
				byte[] dd = new byte[ll];
				outprintln("Sending "+ll+" bytes ("+blocCpt+")");
				int l = fis.read(dd, 0, dd.length);
				ArduinoMCU.writeBytes(dd);
				//_( readSerialLine(true) );

				blocCpt++;
				if ( ll < 512 ) { break; }

			} else {
				_( ArduinoMCU.readSerialLine(false) );
			}

		}

		int i = ArduinoMCU.readOneByte();
		t1 = System.currentTimeMillis();

		outprintln("Backup sent "+((t1-t0)/1000)+"sec");

		fis.close();
		
		System.out.println("finish");
		_( ArduinoMCU.readSerialLine(false) );
		
		//   _("Enter to exit");
		//   kbd.readLine();
		Zzz(2000);

    } catch(Exception ex) {
      ex.printStackTrace();
    } finally {
	  ArduinoMCU.close();// Close serial port
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