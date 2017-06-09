import java.io.*;

import jssc.SerialPort;
import jssc.SerialPortEvent;
import jssc.SerialPortEventListener;
import jssc.SerialPortException;

public class Shell {

static PrintWriter stdin = null;
static Process p = null;
static boolean dummyActivated = false;

  public static void main(String[] args) throws Exception {
ArduinoMCU.open();
//ArduinoMCU.reset();

 int mask = SerialPort.MASK_RXCHAR;

SerialPort serialPort = ArduinoMCU.getSerialPort();
serialPort.setEventsMask(mask);
//Add an interface through which we will receive information about events
serialPort.addEventListener(new SerialPortReader());


      boolean bashMode = new File("/bin/bash").exists();
    String[] command =
    {
        bashMode ? "/bin/bash" : "cmd",
    };
    p = Runtime.getRuntime().exec(command);
    //new Thread(new SyncPipe(p.getErrorStream(), System.err)).start();
    //new Thread(new SyncPipe(p.getInputStream(), System.out)).start();

    System.out.println("Waiting prompt or enter");
    new Thread() { public void run() { 
        try {
            new BufferedReader(new InputStreamReader(System.in)).readLine();
            ArduinoMCU.activateDUMMY();
            dummyActivated = true;
        } catch(Exception ex) {}
    } }.start();

    while(!dummyActivated) {
        ArduinoMCU.delay(500);
    }

    new Thread(new SyncArduinoPipe(p.getErrorStream())).start();
    new Thread(new SyncArduinoPipe(p.getInputStream())).start();
    stdin = new PrintWriter(p.getOutputStream());

    // if ( bashMode ) {
    //   stdin.println("ls -lh ~");    
    // } else {
    //   stdin.println("dir c:\\ /A /Q");
    // }

    ArduinoMCU.delay(500);


    int returnCode = p.waitFor();
    System.out.println("Return code = " + returnCode);

ArduinoMCU.close();
  }

  static void halt() {
    p.destroy();
    ArduinoMCU.close();
    System.exit(0);
  }
  
  static void printToProcess(byte[] b) {
    stdin.write(new String(b));
    stdin.flush();
  }

static class SerialPortReader implements SerialPortEventListener {
   String dummy = "";
   String cmd = "";
        public void serialEvent(SerialPortEvent event) {
            SerialPort serialPort = ArduinoMCU.getSerialPort();

            //Object type SerialPortEvent carries information about which event occurred and a value.
            //For example, if the data came a method event.getEventValue() returns us the number of bytes in the input buffer.
            if(event.isRXCHAR()){
                if(event.getEventValue() > 0){
                    try {
                        byte buffer[] = serialPort.readBytes(event.getEventValue());

                        //if ( new String( buffer ).trim().equals("logout") ) {
                        if ( new String( buffer ).trim().startsWith("EXIT DU") ) {
                            // never happens .... (keystroke are sent one by one)
                            Shell.halt();
                            return;
                        } else {
                            // // if (buffer[0] == 10) { Shell.printToProcess( "\n".getBytes() ); }
                            // if ( Shell.dummyActivated ) {
                            //   Shell.printToProcess( buffer );
                            // }

                            cmd += (char)buffer[0];
                            if (buffer[0] == '\n') { 
                                Shell.printToProcess( cmd.getBytes() ); 
                                System.out.println("SHELL> "+cmd);
                                cmd = "";
                            }

                        }
                        if ( dummy != null ) {
                          dummy +=  new String( buffer );
                          if ( dummy.trim().endsWith("DUMMY") ) {
                              Shell.dummyActivated = true;
                              dummy = null;
                              cmd = "";
                          }
                        }
                        System.out.println(">"+ new String( buffer ));

                    }
                    catch (SerialPortException ex) {
                        ex.printStackTrace();
                    }
                }
            }
            // //If the CTS line status has changed, then the method event.getEventValue() returns 1 if the line is ON and 0 if it is OFF.
            // else if(event.isCTS()){
            //     if(event.getEventValue() == 1){
            //         System.out.println("CTS - ON");
            //     }
            //     else {
            //         System.out.println("CTS - OFF");
            //     }
            // }
            // else if(event.isDSR()){
            //     if(event.getEventValue() == 1){
            //         System.out.println("DSR - ON");
            //     }
            //     else {
            //         System.out.println("DSR - OFF");
            //     }
            // }
        }
    }



}