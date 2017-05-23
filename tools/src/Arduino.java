import jssc.SerialPort;

/**************************
* TI 92 link - Arduino settings
* 
* Xtase - fgalliat May2017
* 
**************************/

public class Arduino {

    // COM7 UNO
    // COM11 proMini
    public static String commPort = "COM11";
    public static boolean UNO = false;

    public static void resetArduino(SerialPort serialPort) throws Exception {
        if ( !UNO ) {
            // proMini doesn't resets @ each serPort.open()
            serialPort.writeBytes( "\\SR".getBytes() );
            Thread.sleep(3000);

            // serialPort.closePort();
            // System.out.println("opening " + serialPort.getPortName());
            // serialPort.openPort();// Open serial port

            // System.out.println("setting");
            // serialPort.setParams(SerialPort.BAUDRATE_115200, SerialPort.DATABITS_8, SerialPort.STOPBITS_1, SerialPort.PARITY_NONE);
            // serialPort.setFlowControlMode(SerialPort.FLOWCONTROL_NONE);
            
            try { serialPort.readBytes(256, 500); }
            catch(Exception ex){}
            //_( "garbage : "+readSerialLine(false) );
        } else {
            //just wait after serial port reset...
            Thread.sleep(2000);
        }
    }

}