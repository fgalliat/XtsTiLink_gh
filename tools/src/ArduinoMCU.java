import java.io.*;

import jssc.SerialPort;
import jssc.SerialPortException;
import jssc.SerialPortTimeoutException;

/**************************
* TI 92 link - Arduino settings & routines
* 
* Xtase - fgalliat May2017
* 
**************************/

public class ArduinoMCU {

    // COM7 UNO
    // COM11 proMini
    public static String commPort = "COM11";
    public static boolean UNO = false;

    protected static SerialPort serialPort = null;

    public static void open() throws Exception {
      serialPort = new SerialPort(commPort);
      System.out.println("opening " + serialPort.getPortName());
      serialPort.openPort();// Open serial port

      System.out.println("setting");
      serialPort.setParams(SerialPort.BAUDRATE_115200, SerialPort.DATABITS_8, SerialPort.STOPBITS_1, SerialPort.PARITY_NONE);
      serialPort.setFlowControlMode(SerialPort.FLOWCONTROL_NONE);
    }

    public static void close() {
        System.out.println("closing");
        try {
            serialPort.closePort();// Close serial port
        } catch(Exception ex) {}
    }

    public static void reset() throws Exception {
        if ( serialPort == null || !serialPort.isOpened() ) {
            throw new Exception("Port not initialised");
        }
        if ( !UNO ) {
            // proMini doesn't resets @ each serPort.open()
            serialPort.writeBytes( "\\SR".getBytes() );
            Thread.sleep(3000);
    
            try { serialPort.readBytes(256, 500); }
            catch(Exception ex){}
            //_( "garbage : "+readSerialLine(false) );
        } else {
            //just wait after serial port reset...
            Thread.sleep(2000);
        }
    }

    // ===============================================

    public static void writeBytes(byte[] data) throws Exception {
        serialPort.writeBytes(data);
    }

    public static void writeByte(int b0) throws Exception {
        serialPort.writeByte((byte)b0);
    }

    public static int readOneByte() throws Exception {
        int ch = serialPort.readBytes(1)[0];
        if ( ch < 0 ) { ch += 256; }
        return ch;
    }

    // ===============================================

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

}