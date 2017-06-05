public class Reboot {
  public static void main(String[] args) throws Exception {
    ArduinoMCU.open();
    ArduinoMCU.reset();
    ArduinoMCU.close();
  }
}