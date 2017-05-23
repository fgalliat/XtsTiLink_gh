import java.io.*;

/**************************
* TI 92 BinToH - made to embed .92P in 
* Arduino Serial "cable" Flash memory
*
* Xtase - fgalliat May2017
* 
**************************/

public class BinToH {

  public static void main(String[] args) throws Exception {
      process(args[0]);
  }

  static void process(String filename) throws Exception {
      String file = new File(filename).getName();
      String fileWOExt = file.substring(0, file.lastIndexOf("."));

      PrintStream out = System.out;
      InputStream is = new FileInputStream(filename);

      int len = (int)new File(filename).length();

      out.println("// "+len+" bytes");

      out.println("const PROGMEM uint8_t  FILE_NAME[] = \""+file+"\";");
      out.println("const PROGMEM uint16_t FILE_SIZE[] = { "+len+" };");

      out.println("const PROGMEM uint8_t FILE_CONTENT[] = {");

      int i=0,ch;
      while( (ch = is.read()) != -1 ) {
          out.print("0x"+( ch < 16 ? "0" : "" )+Integer.toHexString(ch).toUpperCase()+", ");
          if ( i >= 20 ) {
              out.println("");
              i=0;
          }
          i++;
      }
      out.println("};");
      

      is.close();
  }


}