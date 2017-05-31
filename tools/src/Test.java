import java.io.*;
import java.util.ArrayList;
import java.util.List;

/**************************
* TI 92 link program sender
* 
* Xtase - fgalliat May2017
* 
**************************/

public class Test {
  
  static BufferedReader kbd = null;

  static void halt() {
    try {
      System.out.println("closing");
      ArduinoMCU.close();// Close serial port
    } catch(Exception ex) {}
    System.exit(0);
  }
  
  public static void main(String[] args) throws Exception {
	// mainSendPrgm(true, "../fs/fargo/flib.92p", "main\\flib" );
	// mainSendPrgm(true, "../fs/fargo/shell.92p", "main\\shell" );

	if ( args.length > 0 ) {
		sendPrgmToTi(args[0], args.length>1, true, args.length > 2);
	} else {
		_("Na args supplied");
	}

  }

  // autoExecLockEnter -> types varname but not final Enter key
  public static void sendPrgmToTi(String filename, boolean autoexec, boolean kbdWait, boolean autoExecLockEnter) throws Exception {
	String varName = new File(filename).getName();
	// volountary not lastIndexOf(..)
	varName = varName.substring( 0, varName.indexOf(".") );
	varName = "main\\"+varName;
	mainSendPrgm(autoexec, filename, varName, kbdWait, autoExecLockEnter );
  }



public static void mainRecv(String[] args) throws Exception {
	try {
    ArduinoMCU.open();
      
	// makes arduino reset when openeing port
	ArduinoMCU.reset();

_("Enter to start");
	  BufferedReader kbd = new BufferedReader( new InputStreamReader( System.in ) );
	  kbd.readLine();

      System.out.println("begin");
	     
		ArduinoMCU.writeBytes( new byte[] { '\\', 'R', 'B' } );
 _("waits");
      _( ArduinoMCU.readSerialLine(true) );

	FileOutputStream out = new FileOutputStream("../fs/test.92b");

	while(true) {
		int ch0 = ArduinoMCU.readOneByte();
		int ch1 = ArduinoMCU.readOneByte();
		int len = (ch0*256)+ch1;


		_("receiving "+len+" bytes");
		if (len <= 0 || len > 1028) { break; }

		for(int i=0; i < len; i++) {
			int ch = ArduinoMCU.readOneByte();
			if ( i>=4 ) {
				out.write(ch);
			}
		}


	}	  
_( ArduinoMCU.readSerialLine(true) );
out.flush();
out.close();

      System.out.println("finish");
      
	  _("Enter to exit");
	  
	  kbd.readLine();

    } catch(Exception ex) {
      ex.printStackTrace();
    } finally {
      System.out.println("closing");
	  ArduinoMCU.close();// Close serial port
    }
}


static int min(int a, int b) { return a < b ? a : b; }



  // =================================================================

  public static void mainSendPrgm(boolean autoRun, String fileName, String varName, boolean kbdWait, boolean autoExecLockEnter) throws Exception {
    try {
      ArduinoMCU.open();
      
	  // proMini doesn't resets @ each serPort.open()
	   ArduinoMCU.reset();

      System.out.println("begin");

	  // flush garbage
	  _( "G:"+ArduinoMCU.readSerialLine(false) );
      
      File b92 = new File(fileName);
      int blen = (int)b92.length();
      
	  InputStream fis = new FileInputStream( b92 );

	  byte[] head = new byte[86];
	  fis.read(head);

	  int d0 = fis.read();
      int d1 = fis.read();
	  blen = (d0*256)+d1;

      _(blen+" bytes");

	  _("Enter to start");
	  kbd = new BufferedReader( new InputStreamReader( System.in ) );
	  if (kbdWait) kbd.readLine();

      ArduinoMCU.writeBytes( new byte[] { '\\', 'S', 'P', (byte)d0, (byte)d1 } );
	  ArduinoMCU.writeBytes( varName.getBytes() );
	  ArduinoMCU.writeByte( (byte)0x00 );

	  if (fileName.toLowerCase().endsWith(".92p")) { ArduinoMCU.writeByte( (byte)0x01 ); }
	  else { ArduinoMCU.writeByte( (byte)0x00 ); }

	  if (autoRun) { ArduinoMCU.writeByte( (byte) (autoExecLockEnter ? 0x02 : 0x01) ); }
	  else { ArduinoMCU.writeByte( (byte)0x00 ); }
      
      _("waits");
      _( ArduinoMCU.readSerialLine(true) );
      
      	_("Var "+ varName +" to "+ blen +" copy");
		  // wait for '0x01' ready to send
      	int ch = ArduinoMCU.readOneByte();
      	
      	_( ch );
      	
		long t0,t1;
		t0 = System.currentTimeMillis();

      	if ( ch == 1 ) {
      		_("->");
			ArduinoMCU.writeByte((byte)0x00); // ACK

			//final int BLOC_LEN = 128;
			final int BLOC_LEN = 514;
			byte[] data = new byte[BLOC_LEN]; int e;
      		for(int j=0; j < blen; j+=BLOC_LEN) {

				e = BLOC_LEN;
				if ( j+BLOC_LEN > blen ) { e = blen-j; }

				int re = fis.read(data, 0, e);

				// own cts
				ch = ArduinoMCU.readOneByte();
				if ( ch != 2 ) { _("Oups2"); break; }

      			ArduinoMCU.writeBytes( data ); // beware of end of file
      		} 
      		_("<-");
			t1 = System.currentTimeMillis();

			_( ArduinoMCU.readSerialLine(!true) ); // chksum line      
			_( ArduinoMCU.readSerialLine(!true) ); // var sent
			_( ArduinoMCU.readSerialLine(!true) );

			_("took "+(t1-t0)+" msec to copy");

			// flush garbage
	  		_( "G:"+ArduinoMCU.readSerialLine(false) );

      	}	else {
      		_("(EE)");
			System.out.print( (char)ch );
      		_( ArduinoMCU.readSerialLine() );
      	}

      
      fis.close();

      System.out.println("finish");
      
	  _("Enter to exit");
	  
	  if (kbdWait) kbd.readLine();

    } catch(Exception ex) {
      ex.printStackTrace();
    } finally {
      System.out.println("closing");
	  ArduinoMCU.close();// Close serial port
    }
    
  }
  // =================================================================
  
  public static void _(Object o) {
		System.out.println(o);
	}

	public static void Zzz(long time) {
		try {
			Thread.sleep(time);
		} catch (Exception ex) {
		}
	}
  
}