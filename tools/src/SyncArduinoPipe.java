import java.io.*;

class SyncArduinoPipe implements Runnable {

  //private final OutputStream ostrm_;
  private final InputStream istrm_;

public SyncArduinoPipe(InputStream istrm) {
      istrm_ = istrm;
      //ostrm_ = ostrm;
  }

  public static Object WRITE_LOCKER = "coucou";

  public void run() {
      try {
          //final byte[] buffer = new byte[1024];
          //final byte[] buffer = new byte[32];
          //final byte[] buffer = new byte[1];
          //final byte[] buffer = new byte[16];

          // less than Arduino side...
          //final byte[] buffer = new byte[32];
          final byte[] buffer = new byte[1];

String subBuffer = "";
          for (int length = 0; (length = istrm_.read(buffer)) != -1; )
          {
              if ( length > 0 ) {
                  synchronized(WRITE_LOCKER) {
                    //ostrm_.write(buffer, 0, length);

                    //byte[] buff2 = new byte[ length ];
                    //System.arraycopy(buffer, 0, buff2, 0, length);
                    //// //System.out.print(">"); System.out.write(buff2); System.out.println("<");
                    //ArduinoMCU.writeBytes(buff2);
                    //ArduinoMCU.delay(buff2.length*5);
                    //ArduinoMCU.delay(8);

                    ArduinoMCU.writeByte(buffer[0]);              
                    ArduinoMCU.delay(10); // optimum value
                    //ArduinoMCU.delay(8);

                    
//  for(int i=0; i < length; i++) {
//    ArduinoMCU.writeByte(buffer[i]);
//    ArduinoMCU.delay(15);
//  }

                    //ArduinoMCU.writeBytes(buffer);
                    //ArduinoMCU.delay(buffer.length*10);

                    // for(int i=0; i < length; i++) {
                    //     //ArduinoMCU.writeByte(buffer[i]);
                    //     subBuffer += (char)buffer[i];

                    //     if ( buffer[i] == (byte)'\r') {
                    //         // ....
                    //     }
                    //     else if ( buffer[i] == (byte)'\n') {
                    //         //ArduinoMCU.delay(3);
                    //         ArduinoMCU.writeBytes(subBuffer.getBytes());
                    //         ArduinoMCU.delay(10);
                    //         subBuffer = "";
                    //     } else {
                    //         //ArduinoMCU.delay(1);
                    //     }
                    // }

                    // if ( subBuffer.length() > 0 ) {
                    //     ArduinoMCU.writeBytes(subBuffer.getBytes());
                    //     ArduinoMCU.delay(10);
                    // }

                  }
              }
          }
      }
      catch (Exception e)
      {
          e.printStackTrace();
      }
  }

}
