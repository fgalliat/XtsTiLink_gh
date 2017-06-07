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
          final byte[] buffer = new byte[1];
          for (int length = 0; (length = istrm_.read(buffer)) != -1; )
          {
              if ( length > 0 ) {
                  synchronized(WRITE_LOCKER) {
                    //ostrm_.write(buffer, 0, length);
                    // byte[] buff2 = new byte[ length ];
                    // System.arraycopy(buffer, 0, buff2, 0, length);
                    // //System.out.print(">"); System.out.write(buff2); System.out.println("<");
                    // ArduinoMCU.writeBytes(buff2);

                    ArduinoMCU.writeBytes(buffer);
                    ArduinoMCU.delay(2);
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
