#include <SoftwareSerial.h>
#include <AltSoftSerial.h>
#include <TinyGPS.h>
#include <Regexp.h>

//
// Para crear validaciones de expresiones regulares
//
//   . --- (a dot) represents all characters. 
//  %a --- all letters. 
//  %c --- all control characters. 
//  %d --- all digits. 
//  %l --- all lowercase letters. 
//  %p --- all punctuation characters. 
//  %s --- all space characters. 
//  %u --- all uppercase letters. 
//  %w --- all alphanumeric characters. 
//  %x --- all hexadecimal digits. 
//  %z --- the character with hex representation 0x00 (null). 
//  %% --- a single '%' character.
//  %1 --- captured pattern 1.
//  %2 --- captured pattern 2 (and so on).
//  %f[s]  transition from not in set 's' to in set 's'.
//  %b()   balanced nested pair ( ... ( ... ) ... ) 
//
// Callback llamado para cada match  
void match_callback  (const char * match,          // matching string (not null-terminated)
                      const unsigned int length,   // length of matching string
                      const MatchState & ms){      // MatchState in use (to get captures)
  char cap [10];   // must be large enough to hold captures
  
  Serial.print ("Matched: ");
//  Serial.write ((byte *) match, length);
  String str485 = String(match);
  Serial.println (str485);
  Serial.println ();
  
  for (byte i = 0; i < ms.level; i++){
    Serial.print ("Capture "); 
    Serial.print (i, DEC);
    Serial.print (" = ");
    ms.GetCapture (cap, i);
    Serial.println (cap); 
  }  
}  

//
// ******************************
// Mapa de Conexiones
// ******************************
// Serial1 -> wifi;    
// Serial2 -> gsm;     
// Serial3 -> bt;    
//
// ******************************
// Mapa de PIC y Puertos
// ******************************
// Micro | Salidas
//   A       0-7
//   B       8-16
//
// PIC | Puerto | Entradas
//  C      D       0-7
//              
//
//

SoftwareSerial port485(10, 11);     // RX = 10 and TX = 11
AltSoftSerial  gpsPort;             // RX = 48 and TX = 46
TinyGPS gps;                        // Instancia de la librería del GPS
#define RS485_PIN 13                // Pin de control de red 485 
#define CMD_LEN 20                  // Número de comandos de bajo nivel en el array 
String cmd485ToSend = "@OK0";       // Comando enviado desde el maestro 485 para si existen, enviar comandos de bajo nivel
String from485 = "";                // Comandos recibidos desde 485
String methodFromUart = "";         // Funcion de alto nivel recibido desde cualquier UART
int total = 0;                      // Total de comandos de bajo nivel generados a partir de funció de alto nivel
unsigned long count;                // Contador coincidencias al validar reg exp
unsigned long count2;
unsigned long count3;
String cmdsTo485[CMD_LEN];          // Comandos de bajo nivel para enviar al maestro 485
String respFrom485[CMD_LEN];        // Respuestas de la tarjeta maestra a cada comando enviado  
String sourceCmd[CMD_LEN];          // Fuente (Wifi, Bt...) de donde provino el comando
String lastUartMethod;              // Último método de alto nivel que arrivó       
int i = 0;
String gpsPos = "";                 
String str = "";
String phone = "";
String ok = "OK";
String ip = "192.168.1.71";
String port = "55056";
String input = "";
boolean isWifi = false;
boolean isGsm = false;
boolean isBt = false;


void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial2.begin(9600);
  Serial3.begin(9600);
  gpsPort.begin(9600);
  port485.begin(9600);

  Serial.println("--------------------------");
  Serial.println(" Inicializando USART A... ");
  Serial.println("--------------------------");
  
  
  Serial3.println(" ");
  Serial3.println(" ");
  Serial3.println("---------------");
  Serial3.println("Bluetooth....  ");
  Serial3.println("---------------");
  Serial3.println(" ");
  Serial3.println(" ");

  configGsm();
  
  delay(3000);
  Serial.println("");
  Serial.print("IP: ");
  String expected[] = {"OK", "ERROR"};
  sendAndWait("AT+IP?", expected, 500, 2, "wifi");
  
  delay(1000);
  Serial.println();
  Serial.println("<<< Escuchando puertos.... >>>");

}



// ---------------------------------------
// Interrupción cuando arriva dato Wifi
// ---------------------------------------
void serialEvent1() {
  methodFromUart = Serial1.readStringUntil('\r');
  Serial.println();
  Serial.print("Wifi Data: ");
  Serial.print(methodFromUart);
  Serial.println();
  
  isWifi = true;
  isGsm = false;
  isBt = false;
  lastUartMethod = methodFromUart;
  
  makeCommands(methodFromUart);
  Serial.println();
  Serial.println("Made from Wifi: ");
  Serial.println(cmdsTo485[0]);
  Serial.println(cmdsTo485[1]);
  Serial.println(cmdsTo485[2]);
}



// ---------------------------------------
// Interrupción cuando arriva dato GSM
// ---------------------------------------
void serialEvent2() {
  methodFromUart = Serial2.readStringUntil('#');
  Serial.println();
  Serial.print("Data from GSM: ");
  Serial.print(methodFromUart);
  Serial.println();

  // Validar encabezado el SMS
  char* cSms = methodFromUart.c_str();
  MatchState ms (cSms);
  count2 = ms.GlobalMatch("%+[A-Z]+: \"[0-9]+\",\"\",\"[0-9/,:-]+\"", match_callback);
  if(count2 == 1){
    // Extrer número del remitente
    phone =  methodFromUart.substring(methodFromUart.indexOf("\"") + 1, methodFromUart.indexOf("\"") + 11);
    Serial.print(phone);
    Serial.println();
    // Extrer método de alto nivel
    String method = methodFromUart.substring(methodFromUart.indexOf("@") + 1, methodFromUart.length());
    Serial.print("method: ");
    Serial.print(method);
    Serial.println();

    isWifi = false;
    isGsm = true;
    isBt = false;
    lastUartMethod = method;
    
    makeCommands(method);
    Serial.println();
    Serial.println("Made from GSM");
    Serial.println(cmdsTo485[0]);
    Serial.println(cmdsTo485[1]);
    Serial.println(cmdsTo485[2]);
  }
}


// ---------------------------------------
// Interrupción cuando arriva dato BT
// ---------------------------------------
void serialEvent3() {
  methodFromUart = Serial3.readStringUntil('\r');
  Serial.println();
  Serial.print("BT Data: ");
  Serial.print(methodFromUart);
  Serial.println();

  isWifi = false;
  isGsm = false;
  isBt = true;
  lastUartMethod = methodFromUart;
  
  makeCommands(methodFromUart);
  Serial.println();
  Serial.println("Made from BT");
  Serial.println(cmdsTo485[0]);
  Serial.println(cmdsTo485[1]);
  Serial.println(cmdsTo485[2]);
}





  
// *********************************************
//                Método principal
// *********************************************
void loop() {
//   if(port485.available() > 0){
//     Serial.print("Incoming: ");
//     Serial.println(port485.read());
//   }
  /**
   * Funcionamiento. Los datos de Wifi, GSM y Bt llegan mediante interrupciones
   * manejadas por hardware. Una vez que llegan métodos de alto nivel p.e.
   * getAllOut(), se procede a almacenar la siguiente información:
   * - Comandos de bajo nivel a generar 
   * - Respuesta hacia la fuente del comando de alto nivel (wifi, Bt, GSM)
   * - Fuente del comando 
   * - Método de alto nivel que arrivó
   * 
   * Una vez con esta información se procede a generar los comandos de bajo
   * nivel para ser enviados a la tarjeta maestra, esperando entre cada uno
   * 1 seg para esperar la respuesta apropiada.
   * 
   * Posteriormente se procede a responder a la fuente del método de alto nivel
   * 
   **/
  for(i = 0; i < CMD_LEN; i++){         
    if(!cmdsTo485[i].equals("")){   // Iterar array de comandos de bajo nivel y enviar si es válido
      port485.print(cmdsTo485[i]);  
      Serial.print("<< cmd bajo nivel enviado: ");
      Serial.print(cmdsTo485[i]);
      Serial.println();
      delay(500);
      for (unsigned long start = millis(); millis() - start < 2000;){   // Verdaderamente procesar en 1 seg
        // Leer respuesta de la tarjeta maestra
        if(port485.available() > 0){
          input = port485.readStringUntil('\r');
          Serial.print("resp ");
          Serial.print(i);
          Serial.print(" : ");
          Serial.println(input);
          if(isStringNum(input) == true){
            respFrom485[i] = input;
          }
        }
      }
    }
  }
  printAllArrays();
  sendResponse(lastUartMethod);
  clearAllArrays();
  
}


/**
 * Analiza el método de alto nivel y construye la respuesta indicada por 
 * el medio por donde arrivó el método.
 * 
 * String uartMethod. Método de alto nivel a analizar.
 * 
 **/
void sendResponse(String uartMethod){
  String response = "OK";
  String bin = "";
  String s = "";
  String ip = "192.168.1.68";   // IP destino de respuesta de Wifi
  String port = "55056";        // Puerto destino de respuesta de Wifi
  
  char* cCmd = uartMethod.c_str();
  MatchState ms (cCmd);
  count = ms.GlobalMatch("setAllOut%([0-1]+%)", match_callback);
  if(count == 1){  
    if(isWifi){
      sendWifiData(ip, port, response);
    }
    else if(isBt){
      Serial3.println(response);
    }
    else if(isGsm){
      sendSms(response.c_str(), phone.c_str());
    }
  }
  count = ms.GlobalMatch("getAllOut%(%)", match_callback);   
  if(count == 1){
    // Componer número binario de N bits
    int ad = respFrom485[0].toInt();
    int bd = respFrom485[1].toInt();
    bin = getBinary(ad, 8);
    bin.concat(getBinary(bd, 8));
    Serial.print("BIN: ");
    Serial.println(bin);
    
    if(isWifi){
      sendWifiData(ip, port, bin);
    }
    else if(isBt){
      Serial3.println(bin);
    }
    else if(isGsm){
      sendSms(bin.c_str(), phone.c_str());
    }
  }
}





/**
 * Genera comandos de bajo nivel en el array "cmds485" dado un método de alto
 * nivel.
 * 
 * String function. Método de alto nivel con argumentos.
 * 
 * Retorna. Int, número de comandos añadidos al array.
 **/
void makeCommands(String cmd){
  String method = "";     // Método de alto nivel procedente de cualquier medio UART
  String smsNum = "";
  String smsText = "";
  String output = "";
  String str = "";
  String sTemp = "";
  String sTemp2 = ""; 
  String endChar = "#";
  String inputA = "@VA#";
  String inputB = "@VB#";
  int temp = 0;  
  int index = 0;          // Índice del array de comandos de bajo nivel
  int val = 0;
  int val2 = 0;
  String s = "";
  String mIndex = "";      
  String mVal = "";
  String sBit = "";
  String sVal = "";
  int iIndex = 0;
  int iVal = 0;

  if(!cmd.equals("")){
    char* cCmd = cmd.c_str();
    MatchState ms (cCmd);
    // -----------------------------------------
    // Validar función: setAllOut(16 bits)
    // -----------------------------------------
    count = ms.GlobalMatch("setAllOut%([0-1]+%)", match_callback); 
    // Traducir método a comandos de bajo nivel
    if(count == 1){    
      Serial.println("*********** setAllOut(16 bits)");
      // Extraer argumentos del método <<< caso inicial 16 bits >>>
      method = String(cCmd);
      method.trim();                    
      method.replace("\r", "");
      method.replace("\n", "");
      method.replace("setAllOut(", "");
      method.replace(")", "");
      
      sTemp = method.substring(0, 8);
      sTemp2 = method.substring(8, 16);

      // Convertir binario a decimal
      val = strtol(sTemp.c_str(), NULL, 2);  
      val2 = strtol(sTemp2.c_str(), NULL, 2); 

      // Convertir decimal a String para componer comandos de bajo nivel
      s = String(val);
      sTemp = completeZeros(s, 3);
      s = String(val2);
      sTemp2 = completeZeros(s, 3);
           
      // Añadir comandos de bajo nivel a array
      output = "@A" + sTemp + endChar;
      cmdsTo485[index] = output;
      setSourceCmd(index);
      index++;

      output = "@B" + sTemp2 + endChar;
      cmdsTo485[index] = output;
      setSourceCmd(index);
      index++;
    }

    // -----------------------------------------
    // Validar función: getOut(int bit)
    // -----------------------------------------
    count = ms.GlobalMatch("getOut%([0-9]+%)", match_callback); 
    // Traducir método a comandos de bajo nivel
    if(count == 1){    
      // Extraer argumentos del método 
      method = String(cCmd);
      method.trim();                    
      method.replace("\r", "");
      method.replace("\n", "");
      method.replace("getOut(", "");
      method.replace(")", "");

      // Extraer argumentos del método y completar a 2 cifras
      sTemp = completeZeros(method, 2);
           
      // Añadir comandos de bajo nivel a array
      cmdsTo485[index] = "@G" + sTemp + "#";
      setSourceCmd(index);
    }
    

    // ------------------------------------------------
    // Validar función: setOut(int salida, int valor)
    // ------------------------------------------------
    count = ms.GlobalMatch("setOut%([0-9]+,[0-1]+%)", match_callback); 
    // Traducir método a comandos de bajo nivel
    if(count == 1){    
      Serial.println("*********** setOut(int salida, int valor)");
      // Extraer argumentos del método 
      method = String(cCmd);
      method.trim();                    
      method.replace("\r", "");
      method.replace("\n", "");
      method.replace("setOut(", "");
      method.replace(")", "");
      
      for(int i = 0; i <= method.length(); i++){
          char ch = method.charAt(i);
          if(ch != ','){
            sBit.concat(ch);
          }
          else  
            break;
      }
      sVal = method.substring(method.length() - 1, method.length());
      
      // Añadir comandos de bajo nivel a array
      int bit1 = sBit.toInt();
      int val = sVal.toInt();

      sTemp = completeZeros(sBit, 2);

      str = "@S" + sTemp + val + endChar;
      cmdsTo485[index] = str;
    }

    // ----------------------------------------
    // Validar función: getAllOut()
    // ----------------------------------------
    count = ms.GlobalMatch("getAllOut%(%)", match_callback);   
    if(count == 1){
      Serial.println("********* getAllOut()");
      index = 0;
      cmdsTo485[index] = inputA;
      setSourceCmd(index);
      index++;
      
      cmdsTo485[index] = inputB;
      setSourceCmd(index);
      index++;
    }

    // ----------------------------------------
    // Validar función: getAllIn()
    // ----------------------------------------
    count = ms.GlobalMatch("getAllIn%(%)", match_callback);   
    if(count == 1){
      Serial.println("********* getAllIn()");
      index = 0;
      cmdsTo485[index] = "@VPCD#";
      setSourceCmd(index);
      index++;
    }

    // --------------------------------------------------
    // Validar función: sendMsg(String msg, String num)
    // --------------------------------------------------
    count = ms.GlobalMatch("sendMsg%(%\"[%a%d%p%s]+%\",\"[0-9]+\"%)", match_callback); 
    if(count == 1){
      Serial.println("******** sendMsg()");
      // Extraer argumentos del método 
      method = String(cCmd);
      method.trim();                    
      method.replace("\r", "");
      method.replace("\n", "");
      method.replace("sendMsg(", "");
      method.replace(")", "");
      method.replace("\"", "");
      
      for(int i = 0; i <= method.length(); i++){
          char ch = method.charAt(i);
          if(ch != ','){
            smsText.concat(ch);
          }
          else  
            break;
      }
      smsNum = method.substring(method.length() - 10, method.length());
      Serial.println(smsNum);
      Serial.println(smsText);
      sendSms(smsText.c_str(), smsNum.c_str());

      if(isWifi){
        sendWifiData(ip, port, ok);
      }
      else if(isBt){
        Serial3.println(ok);
      }
    }

    // ----------------------------------------
    // Validar función: getGpsPos()
    // ----------------------------------------
    count = ms.GlobalMatch("getGpsPos%(%)", match_callback);   
    if(count == 1){
      Serial.println("********* getGpsPos()");
//      getGpsInfo();
      index = 0;
      cmdsTo485[index] = "@ADC00#";
      index++;
    }
  } 
}


/**
 * Completa tantos ceros a la izquierda de un string que contiene
 * un número como se indique.
 * 
 * String sNum. String que contiene un número
 * int digits. Número de dígitos de la cifra final.
 **/
String completeZeros(String sNum, int digits){
  int num = sNum.toInt();
  String s = "";
  String t = "";

  if(digits == 2){
    if(num < 10){
      t = "0" + sNum;
    }
    else 
      t = sNum;
  }
  else if(digits == 3){
    if(num < 10){
      t = "00" + sNum;
    }
    else if(num < 100){
      t = "0" + sNum;
    }
    else 
      t = sNum;
  }
  return t;
}



/**
 * Decodifica la latitud y longitud del GPS
 * 
 * String portName. Puerto serial por donde se debe enviar las coordenadas
 * 
 * Retorna. 19.415798@-99.2389486
 **/
void getGpsInfo(){
  bool newData = false;
  unsigned long chars;
  unsigned short sentences, failed;

  // For one second we parse GPS data and report some key values
  for (unsigned long start = millis(); millis() - start < 1000;){
    while (gpsPort.available()){
      char c = gpsPort.read();
      if (gps.encode(c)) // Did a new valid sentence come in?
        newData = true;
    }
  }
  if (newData){
    float flat, flon;
    unsigned long age;
    gps.f_get_position(&flat, &flon, &age);
    flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6;
    flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6;
    
    // Componer string para devolver
    gpsPos = "";
    str = String(flat, 6);
    gpsPos.concat(str);
    str = String(flon, 6);
    gpsPos.concat("@");
    gpsPos.concat(str);
    Serial.print(gpsPos);
  }
}


/**
 * Envía un texto a la IP y puerto especificado por Wifi
 * 
 * String function. 
 * 
 * Retorna. 
 **/
 void sendWifiData(String ip, String port, String text){
   String expected[] = {"OK", "ERROR"};
   String cmd = "AT+SEND=";
   cmd.concat(ip);
   cmd.concat(",");
   cmd.concat(port);
   cmd.concat(",");
   cmd.concat(text);
   sendAndWait(cmd.c_str(), expected, 500, 2, "wifi");
 }


/**
 * Inicializa el módulo GSM para escuchar los SMS entrantes, además
 * de eliminar todos los SMS existentes del chip.
 * 
 **/
void configGsm(){
  String expected[] = {"OK", "ERROR"};
  sendAndWait("AT+CNMI=2,2,0,0,0", expected, 10, 2, "gsm");
  deleteAllSms();
}


/**
 * Envía un mensaje SMS al destinatario indicado. Corroborar
 * que el chip insertado en el módulo GSM sea Telcel y que cuente 
 * con tiempo aire.
 * 
 * text. Texto a enviar al destino
 * passoword. Número al cual enviar el mensaje p.e. +527711725611 (se puede omitir el código +52)
 **/
void sendSms(char* text, char* number){
  String expected[] = {"OK", "ERROR"};
  if(sendAndWait("AT+CMGF=1", expected, 500, 2, "gsm") == 0){
      delay(500);
      Serial2.print("AT+CMGS=\"");
      Serial2.print(number);
      Serial2.println("\"");
      delay(1500);
      Serial2.write(text);
      Serial2.write((char)26);
  }
}


/**
 * Envía los comandos AT para eliminar todos los mensajes
 * de texto del chip.
 **/
void deleteAllSms(){
  char cmd[11];
  for(int i = 1; i <= 30; i++){
    sprintf(cmd, "AT+CMGD=%u", i);
    Serial.println(cmd);
    String expected[] = {"ERROR", "OK"};
    sendAndWait(cmd, expected, 10, 2, "gsm");
  }
}


/**
 * Envía un comando o AT al puerto elegido y espera un tiempo máximo 
 * indicado por la respuesta
 * 
 * char* atCommand. Comando AT a enviar 
 * String expected[]. Array de respuestas esperadas por el comando AT
 * int expectedSize. Tamaño del array expected[]
 * int latency. Tiempo de espera máximo para esperar
 * String portName. Nombre del puerto a enviar y recibir datos
 * 
 * Return. Índice del array expected[] el cual corresponde con la respuesta
 * del comando AT.
 * 
 */
int sendAndWait(char* atCommand, String expected[], int expectedSize, int latency, String portName){
  int retVal = -1;
  int timeout = 0;
  int maxTimeout = 30;   
  char c = 0;
  int j = 0;
  String comp = "";
  if(portName.equals("wifi"))  Serial1.println(atCommand);
  if(portName.equals("gsm"))  Serial2.println(atCommand);
  if(portName.equals("bt"))  Serial3.println(atCommand);
  while(true){
    if(portName.equals("wifi")){
      if(Serial1.available() > 0){
        if(timeout == maxTimeout){ 
          break;  // Timeout reached
        }
        timeout++;
        c = Serial1.read();
        Serial.print(c);
        comp.concat(c);
      } 
    }
    if(portName.equals("gsm")){
      if(Serial2.available() > 0){
        if(timeout == maxTimeout){ 
          break;  // Timeout reached
        }
        timeout++;
        c = Serial2.read();
        Serial.print(c);
        comp.concat(c);
      } 
    }
    if(portName.equals("bt")){
      if(Serial3.available() > 0){
        if(timeout == maxTimeout){ 
          break;  // Timeout reached
        }
        timeout++;
        c = Serial3.read();
        Serial.print(c);
        comp.concat(c);
      } 
    }
    
    for(int i = 0; i < comp.length(); i++){ 
      if (comp.substring(i) == expected[0]){
        return 0;
      } 
      if (comp.substring(i) == expected[1]){
        return 1;
      } 
    }
    delay(latency);
  }
  return -1;
}


/**
 * Prueba la cadena y retorna true si sólo se trata de dígitos,
 * false de cualquier otra forma.
 * 
 * String s. String a analizar.
 **/
boolean isStringNum(String s){
  char* cCmd = s.c_str();
  MatchState ms (cCmd);
  count = ms.GlobalMatch("[0-9]+", match_callback); 
  if(count == 1){      
    return true;
  }
  else
    return false;
}


/**
 * Limpia los arrays de control de comando, ver comentario al inicio
 **/
void clearAllArrays(){
  for(i = 0; i < CMD_LEN; i++){
    respFrom485[i] = "";
    cmdsTo485[i] = "";
    sourceCmd[i] = "";
    lastUartMethod = "";
  }
}


/**
 * Iteración para imprimir: comando de bajo nivel, respuesta, fuente y método de alto nivel
 **/
void printAllArrays(){
  for(i = 0; i < CMD_LEN; i++){
    if(!cmdsTo485[i].equals("")){
//      Serial.print(cmdsTo485[i]);
//      Serial.print(" , ");
      Serial.print(respFrom485[i]);
//      Serial.print(" , ");
//      Serial.print(sourceCmd[i]);
//      Serial.print(" , ");
//      Serial.print(lastUartMethod);
      Serial.println();
    }
  }
}


/**
 * Establece la fuente de donde provino el método de alto nivel
 **/
void setSourceCmd(int index){
  if(isBt)
    sourceCmd[index] = "Bt";
  else if(isWifi)
    sourceCmd[index] = "Wifi";
  else if(isGsm)
    sourceCmd[index] = "Gsm";
}


/**
 * Convierte un entero en su equivalente binario con el tamaño
 * de bits indicado.
 * 
 * int number. Número entero a convertir
 * int bits. Tamaño del número binario.
 * 
 * Retorna. String, representación binaria del número entero.
 **/
String getBinary(int number, int bits){
  String temp = String(number, BIN);;
  String s = "";
  if(temp.length() <= bits){
    for(int i = 0; i < bits - temp.length(); i++){
      s.concat("0");  
    }
    s.concat(temp);
  }
  else{
    s = String(number, BIN);
  }
  return s;
}

























/**
 * Old functions
 *
 **/

 /*
  * 
  * 
  * 
  * 
  * 
  * 

  void serialEvent2() {
  methodFromUart = Serial2.readStringUntil('\r');
  Serial.println();
  Serial.print("Data from GSM: ");
  Serial.print(methodFromUart);
  Serial.println();
  char* cSms = methodFromUart.c_str();
  MatchState ms (cSms);
  // Validar encabezado el SMS
  count2 = ms.GlobalMatch("%+[A-Z]+: \"[0-9]+\",\"\",\"[0-9/,:-]+\"", match_callback);
  
  // Validar datos del SMS, o sea método de alto nivel
  count3 = ms.GlobalMatch("setAllOut%([0-1]+%)", match_callback); 
  if(count3 == 1){
    makeCommands(methodFromUart);
    Serial.println();
    Serial.println("Made from GSM");
    Serial.println(cmdsTo485[0]);
    Serial.println(cmdsTo485[1]);
    Serial.println(cmdsTo485[2]);
  }
}
  
  
  // **********************************************************
  // Impresiones útiles para traducir métodos del alto nivel
  //
  Serial.println();
  Serial.print("method = ");
  Serial.print(method);
  Serial.print(", length = ");
  Serial.print(method.length());
  Serial.print(", lastIndex = ");
  Serial.print(method.lastIndexOf('0'));
  Serial.print(", charAt 0 = ");
  Serial.print(method.charAt(0));
  Serial.println();
  
  for(int i = 0; i < method.length(); i++){
    Serial.println();
    Serial.print("i = ");
    Serial.print(i);
    Serial.print(", c = ");
    Serial.print(method.charAt(i));
  } 


 // Wifi
  while(Serial1.available() > 0){
    str = "";
    cmdFromUart = Serial1.readStringUntil('\r');

    // Descomponer comando para obtener parámetro....
    
    str.concat("@AD");
    str.concat(cmdFromUart);
    str.concat("#");

    // Validar comando
    cStr = str.c_str();
    Serial.print("comando completo: ");
    Serial.print(cStr);
    Serial.println();
    
    // match state object
    MatchState ms (cStr);
    count = ms.GlobalMatch ("@[A-Z][A-Z][0-9][0-9][0-9]#", match_callback);
    Serial.print ("Found ");
    Serial.print (count);            // 8 in this case
    Serial.println (" matches.");
    
    Serial.println(to485);

    if(count == 1){
        isCommand = true;
    }
    
  }
 
 String connectToWifi(String ssid, String password){
  char c = 0;
  String str = "";
  String myIp = "";
  int maxTimeout = 60;
  wifi.listen();
  wifi.println("AT+SSID=" + ssid);
  delay(1000);
  wifi.println("AT+PASS=" + password);
  delay(1000);
  Serial.println("conectando...");
  delay(1000);
  wifi.println("AT+CON");  
  delay(1000);
  
  // Ciclar hasta obtener respuesta o agotar tiempo de espera
  while(true){
    if(wifi.available() > 0){
      if(timeout == maxTimeout){  // set this to your timeout value in milliseconds
         // Timeout reached
        break;
      }
      timeout++;
      c = wifi.read();
      str.concat(c);
      Serial.print(c);
    }
    if(str.endsWith("OK")){
      break;
    }
    if(str.endsWith("ERROR")){
      break;
    }
  }
  delay(1000);
  for(int i = 0; i <= str.length(); i++){
    if(str.charAt(i) == '0' || 
       str.charAt(i) == '1' ||
       str.charAt(i) == '2' ||
       str.charAt(i) == '3' ||
       str.charAt(i) == '4' ||
       str.charAt(i) == '5' ||
       str.charAt(i) == '6' ||
       str.charAt(i) == '7' ||
       str.charAt(i) == '8' ||
       str.charAt(i) == '9' ||
       str.charAt(i) == '.'){
          
      myIp.concat(str.charAt(i));  
    } 
  }
//  Serial.print("ip: ");
//  Serial.println(myIp);
  return myIp;
}




int sendAndWait(char* atCommand, String expected[], int expectedSize, int latency){
//  Serial.println("inciando...");
//  String expected[] = {"EROOR", "OK"};
//  int expectedSize = ((unsigned int) (sizeof (expected) / sizeof (expected[0])));
  boolean matches = false;
  int retVal = -1;
  int timeout = 0;
  int maxTimeout = 30;   // No modificar a más de 9 por que se cicla :(
  char c = 0;
  int j = 0;
  String comp = "";
//  port.listen();
  Serial1.println(atCommand);
  while(true){
    if(Serial1.available() > 0){
//      Serial.print(timeout);
      if(timeout == maxTimeout){ 
        break;  // Timeout reached
      }
      timeout++;
      c = Serial1.read();
      Serial.print(c);
      comp.concat(c);
    } 
    for(int i = 0; i < comp.length(); i++){ 
//      Serial.println(" cadena a comparar ");
//      Serial.print(comp);
      if (comp.substring(i) == expected[0]){
        return 0;
//        retVal = 0;
//        matches = true;
//        break;  
      } 
      if (comp.substring(i) == expected[1]){
        return 1;
//        Serial.println("siiii en 1");
//        retVal = 1;
//        matches = true;
//        break;  
      } 
    }
    delay(latency);
  }
  return -1;
}
 
 
 */



