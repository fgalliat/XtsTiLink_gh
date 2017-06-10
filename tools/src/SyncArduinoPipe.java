import java.io.*;

class SyncArduinoPipe implements Runnable {

    private final InputStream istrm_;

    public SyncArduinoPipe(InputStream istrm) {
      istrm_ = istrm;
    }

    public static Object WRITE_LOCKER = "coucou";
    int cpt = 0;

  public void run() {
      try {
          // less than Arduino side...
          //final byte[] buffer = new byte[32];
          final byte[] buffer = new byte[1];

          String subBuffer = "";
          for (int length = 0; (length = istrm_.read(buffer)) != -1; ) {
              if ( length > 0 ) {
                  synchronized(WRITE_LOCKER) {

                    ArduinoMCU.writeByte(buffer[0]); 
                    //ArduinoMCU.delay(10); // optimum value

                    if ( cpt++ % 4 < 2 ) {
                      ArduinoMCU.delay(10); // optimum value
                    } else {
                      ArduinoMCU.delay(6);
                    }

                  }
              }
          }
      }
      catch (Exception e) {
          e.printStackTrace();
      }
  }

}
