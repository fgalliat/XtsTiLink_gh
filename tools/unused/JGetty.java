public class JGetty {

  public static void main(String[] args) throws Exception {
      ArduinoMCU.open();
      ArduinoMCU.reset();

      final Process ps = Runtime.getRuntime().exec("/bin/bash -c /bin/bash");

      new Thread() {
        public void run() {
           System.out.println("input start read");
            while(true) {
                try {
                    int av = ps.getInputStream().available();
                    if (av <= 0) { delay(3); continue; }
                    byte[] data = new byte[ av ];
                    ps.getInputStream().read(data);
                    System.out.println( new String(data, 0, data.length) );
                    ArduinoMCU.writeBytes(data);
//                    System.out.println( new String(data, 0, data.length) );
                } catch(Exception ex) {
                    ex.printStackTrace();
                }
                delay(5);
            }
        }
      }.start();

      new Thread() {
        public void run() {
           System.out.println("error start read");
            while(true) {
                try {
                    int av = ps.getErrorStream().available();
                    if (av <= 0) { delay(3); continue; }
                    byte[] data = new byte[ av ];
                    ps.getErrorStream().read(data);

                    System.out.println( new String(data, 0, data.length) );
                    ArduinoMCU.writeBytes(data);
//                    System.out.println( new String(data, 0, data.length) );
                } catch(Exception ex) {
                    ex.printStackTrace();
                }
                delay(5);
            }
        }
      }.start();

      new Thread() {
        public void run() {
            while(true) {
                try {
//                    int ch = ArduinoMCU.readOneByte();
int ch=-1;
if ( ch < 0 ) { delay(3); continue; } 
                    System.out.print( (char)ch );
                    ps.getOutputStream().write(ch);
                    ps.getOutputStream().flush();
  //                  System.out.print( (char)ch );
                } catch(Exception ex) {
                    ex.printStackTrace();
                }
                delay(5);
            }
        }
      }.start();

      ps.waitFor();
      System.out.println( "Bye" );

      ArduinoMCU.close();
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