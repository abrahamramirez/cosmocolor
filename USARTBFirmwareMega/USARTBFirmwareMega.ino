#include <SoftwareSerial.h>
#include <Wire.h>                 
#include <LiquidCrystal_I2C.h>
#include <IRremote.h>
#include <Keypad.h>
#include <EEPROMex.h>
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

// ------------------------------------------
// ------------- CONEXIONES -----------------
//
// IR Rec Pin: A0
// Teclado 4x4 Filas:  4, 5, 6, 7
// Teclado 4x4 Columnas:  8, 9, 10, 11
// LCD con I2C: SCL = 21, SDA = 20
// Serial RDM6300: Serial1
// MAX485: Serial2

const byte LCD_COLS = 20;
const byte LCD_ROWS = 4;
#define IR_PWR 2704
#define IR_MUTE 656
#define IR_CHUP 144
#define IR_0 2320
#define IR_1 16
#define IR_2 2064
#define IR_3 1040
#define IR_4 3088
#define IR_5 528
#define IR_6 2576
#define IR_7 1552
#define IR_8 3600
#define IR_9 272

#define IR 1
#define KEY 2
#define RS485_PIN 13
#define IS_AUTH 0
#define CMD_LEN 20   

//SoftwareSerial usart(12, 13);   // RX and TX

LiquidCrystal_I2C lcd(0x27, LCD_COLS, 4);
int RECV_PIN = A0;
IRrecv irrecv(RECV_PIN);
decode_results results;

const byte ROWS = 4;         // 4 filas
const byte COLS = 4;         // 4 columnas

// Definir teclas
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {4, 5, 6, 7};      
byte colPins[COLS] = {8, 9, 10, 11};    

Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);  // Crear una instancia de keypad
              // Número de comandos de bajo nivel en el array 

String respFrom485[CMD_LEN];       // Respuestas de la tarjeta maestra a cada comando enviado  
String cmdsTo485[CMD_LEN];         // Comandos de bajo nivel para enviar al maestro 485
String input = "";

int data1 = 0;
boolean isValidated = false;       // Estado de validacion del usuario
int modeSelected = 0;              // Modo de control seleccionado 1. IR, 2. Teclado             
int result = -1;                   // Status de lectura del TAG RF
char c = 0; 
char cc = 0;
char key = 0;
int k = 0;
int j = 0;
int count = 0;                     // Contador para expresiones regulares 


int tag1[] = {2,48,50,48,48,67,68,55,52,52,55,70,67,3,2,48,50,48,48,67,68,55,52,52,55,70,67,3,2,48,50,48,48,67,68,55,52,52,55,70,67,3,2,48,50,48,48,67,68,55,52,52,55,70,67,3};
int dummyTag[60]; 
//int dummyTag[sizeof(tag1)];
char cmdKey[3];
char cmdIr[3];
char cmdArg[32];
String netCmd;
String method = "";         // Método de alto nivel compuesto para extraer comandos de bajo nivel
int intCmdKey = 0;
int intCmdIr = 0;
String str = "";
String temp = "";


void setup(){
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial2.begin(9600);
  Serial.println("--------------------- ");
  Serial.println("   Iniciando USARTB   ");
  Serial.println("--------------------- ");
  initLcd();
  
  // Inicializar variables
  data1 = 0;
  isValidated = false;
  modeSelected = 0;
  result = -1;
  c = 0;
  j = 0;
  k = 0;
  key = 0;
  intCmdKey = 0;
  intCmdIr = 0;
  
  irrecv.enableIRIn();                // Inicializar receptor IR
  
//  EEPROM.writeBlock(1, tag1);
  isValidated = isAuth();
}

 
void loop(){
//  if(Serial2.available() > 0){
//    char c = Serial2.read();
//    Serial.print(c);
//  }
  
  if(isValidated){                  // Si el usuario ha sido validado mediante TAG RF    
    Serial1.end();
    setAuth(true); 
    c = kpd.getKey();
    delay(30);                      // Para rebote del teclado
    if(c){ 
      if(c == 'B'){                 // Habilitar modo de control mediante teclado
        modeSelected = 2;
      }
      else if(c == 'C'){            // Cerrar sesion
        resetArduino(true);
      }
    }
    if (irrecv.decode(&results)) {
      k = results.value;
      Serial.print("xxxxxxxxxxxxx");
      Serial.println(k);
      if(isValidIr(k)){
        if(k == IR_PWR){                 // Habilitar modo de control mediante IR
          modeSelected = 1;
        }
        else if(k == IR_MUTE){
          resetArduino(true);            // Cerrar sesion
        }
      }
      irrecv.resume();                   // Receive the next value
      delay(100);
    }
    
    if(modeSelected == 0){          // Validado, pero aun no se seleccionado modo: A) IR,  B) Teclado  
      printToLcd("Seleccione Modo     ", 
                 "PWR. Control IR     ", 
                 "B. Teclado          ", 
                 "C|MUTE. Salir       ");
    }
    // ------------------------------------------
    // Validado y seleccionado modo 2 (Teclado) 
    //
    else if(modeSelected == KEY){      
      printToLcd(" --Control Teclado--", 
                 " Esperando comando..", 
                 "                    ", 
                 " #. Enviar  C. Salir");
      
      clearAllArrays();
      readKeyStringUntil(cmdKey, 3, '#', 1, 2);         // Véase metodo 
      intCmdKey = atoi(cmdKey);                         // Convertir array a entero
      // -------------------------------------------
      // Procesar comando: setAllOut(16 bits) - 10
      // -------------------------------------------
      if(intCmdKey == 10){ 
          setAllOutWrapper(KEY);
      }
      // --------------------------------------
      // Procesar comando: getAllOut() - 11
      // --------------------------------------
      else if(intCmdKey == 11){ 
        getAllOutWrapper(KEY);
      }
      // ------------------------------------
      // Procesar comando: setOut() - 12
      // ------------------------------------
      else if(intCmdKey == 12){ 
        setOutWrapper(KEY);
      }
      // Si el comando capturado por teclado no es valido
      else{
        lcd.setCursor(0,1);
        lcd.print(" Comando invalido!!!");
        delay(3000);
        clearAllArrays();
      }
    }
    // ------------------------------------
    // Validado y seleccionado modo 1 (IR) 
    //
    else if(modeSelected == IR){ 
      lcd.setCursor(0,0);
      printToLcd(" ----Control IR---- ",
                 " Esperando comando  ",
                 "                    ",
                 "MUT.Salir PWR.Enviar");
                 
      clearAllArrays();
      readIrStringUntil(cmdIr, 3, IR_PWR, 1, 2);
      intCmdIr = atoi(cmdIr);
      // -------------------------------------------
      // Procesar comando: setAllOut(16 bits) - 10
      // -------------------------------------------
      if(intCmdIr == 10){ 
        setAllOutWrapper(IR);
      }
      // -------------------------------------
      // Procesar comando: getAllOut() - 11
      // -------------------------------------
      else if(intCmdIr == 11){ 
        getAllOutWrapper(IR);
      }
      // -----------------------------------
      // Procesar comando: setOut() - 12
      // -----------------------------------
      else if(intCmdIr == 12){ 
        setOutWrapper(IR);
      }
      // Si el comando capturado por teclado no es valido
      else{
        lcd.setCursor(0,1);
        lcd.print(" Comando invalido!!!");
        delay(3000);
        clearAllArrays();
      }
    }
  }
  // Si el usuario no ha sido validado mediante TAG RF
  else{  
    Serial1.begin(9600);  
    readTags();
    
  }
}



/**
 * Métodos de convenienvia para encapsular el procesamiento 
 * de comandos de alto nivel, sólo eligiendo el modo por 
 * donde fue ordenado.
 * 
 * int mode. IR o KEY.
 **/
void getAllOutWrapper(int mode){
  lcd.clear();
  printToLcd("     getAllOut()    ",
             "--------------------", 
             "  Espere...         ",
             "                    ");
  lcd.setCursor(0,1);

  method = method + "getAllOut()" + "\r";
  cmdsTo485[0] = method;
  sendCmdsAndWait(true);
  
  // Imprimir respuesta por pantalla LCD
  lcd.clear();
  Serial.print("respFrom485: "); Serial.print(getBinary(respFrom485[0].toInt(), 16).c_str());
  if(mode == KEY){
    printToLcd( "     Respuesta:     ",
                getBinary(respFrom485[0].toInt(), 16).c_str(),
                "",
                " C. Salir            ");
  }
  else if(mode == IR){
    printToLcd( "     Respuesta:     ",
                getBinary(respFrom485[0].toInt(), 8).c_str(), 
                getBinary(respFrom485[1].toInt(), 8).c_str(), 
                " MUTE. Salir        ");
  }
  clearAllArrays485();
  method = "";
  if(mode == KEY){
    readKeyStringUntil(cmdArg, 2, 'C', 1, 1);   // Ciclar hasta esperar 'C'
  }
  else if(mode == IR){
    readIrStringUntil(cmdArg, 2, IR_MUTE, 1, 1);   // Ciclar hasta esperar MUTE
  }
}

// ************************************
//
//
void setAllOutWrapper(int mode){
  lcd.clear();
  if(mode == KEY){
    printToLcd(" setAllOut(16 bits) ", 
               "                     ", 
               "                     ", 
               " #. Enviar  C. Salir ");
  }
  else if(mode == IR){
    printToLcd(" setAllOut(16 bits) ", 
               "                     ", 
               "                     ", 
               "MUT.Salir PWR.Enviar ");
  }
  lcd.setCursor(0,1);
  
  if(mode == KEY)
    readKeyStringUntil(cmdArg, 17, '#',1, 1);    // Esperar argumentos del comando  
  else if(mode == IR)
    readIrStringUntil(cmdArg, 17, IR_PWR, 1, 1);
        
  Serial.println(cmdArg);
  delay(100);
  
  method = method + "setAllOut(" + String(cmdArg) + ")" + "\r";
  cmdsTo485[0] = method;
  sendCmdsAndWait(false);
  
  printToLcd(" Procesando comando ",
             "--------------------", 
             "  Espere...         ",
             "                    ");
  clearAllArrays485();
  method = "";
  delay(2000);
}


/**
 * Método de convenienvia para encapsular el procesamiento del 
 * comando de alto nivel, sólo eligiendo el modo por donde fue 
 * ordenado.
 * 
 * int mode. IR o KEY.
 **/
void setOutWrapper(int mode){
  lcd.clear();
  if(mode == KEY){
    printToLcd(" setOut(bit,value)   ", 
               "                     ", 
               "                     ", 
               " #. Enviar  C. Salir ");
  }
  else if(mode == IR){
    printToLcd(" setOut(bit,value)   ", 
               "                     ", 
               "                     ", 
               " MUT.Salir PWR.Enviar");
  }
  
  lcd.setCursor(0,1);
  
  if(mode == KEY)
    readKeyStringUntil(cmdArg, 4, '#',1, 1);    // Esperar argumentos del comando  
  else if(mode == IR)
    readIrStringUntil(cmdArg, 4, IR_PWR, 1, 1);
  delay(100);
  
  method.concat("setOut(");
  str = String(cmdArg);
  Serial.print("str: "); Serial.println(str.length()); 
  method = method + str.substring(0,2) + "," + str.substring(2,4) + ")";
  
  Serial.println(method);
  
//  makeCommands(method);
  sendCmdsAndWait(false);
  printToLcd(" Procesando comando ",
             "--------------------", 
             "  Espere...         ",
             "                    ");
  
//  for(int i = 0; i < CMD_LEN; i++){         
//    if(!cmdsTo485[i].equals("")){   // Iterar array de comandos de bajo nivel y enviar si es válido
//      Serial2.print(cmdsTo485[i]);  
//      Serial.print("<< cmd bajo nivel enviado: ");
//      Serial.print(cmdsTo485[i]);
//      Serial.println();
//      delay(500);
//      for (unsigned long start = millis(); millis() - start < 2000;){   // Verdaderamente procesar en 1 seg  
//      }
//    }
//  }
  clearAllArrays485();
  method = "";
  delay(2000);
}


/**
 * Itera el array cmdsTo485 para enviar los comandos de bajo nivel a la
 * tarjeta maestra, espera durante un par se segundos para obtener respuesta,
 * almacenando éstas para colocarlas en el array respFrom485.
 **/
void sendCmdsAndWait(boolean receive){
  for(int i = 0; i < CMD_LEN; i++){         
    if(!cmdsTo485[i].equals("")){   // Iterar array de comandos de bajo nivel y enviar si es válido
      Serial2.print(cmdsTo485[i]);  
      Serial.print("<< cmd bajo nivel enviado: ");
      Serial.print(cmdsTo485[i]);
      Serial.println();
      delay(500);
      for (unsigned long start = millis(); millis() - start < 2000;){   // Verdaderamente procesar en 1 seg
        // Leer respuesta de la tarjeta maestra
        if(receive){
          if(Serial2.available() > 0){
            input = Serial2.readStringUntil('\r');
            Serial.print("resp: ");
            Serial.println(input);
            if(isStringNum(input) == true){
              respFrom485[i] = input;
            }
          }
        }
      }
    }
  }  
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
 * Resetea el arduino desde el inicio, no modifica perifericos y registros
 * boolean endSession: true. finaliza sesion, false. la sesion persiste 
 */ 
void resetArduino(boolean endSession) {
  if(endSession){  
    lcd.clear();
    printToLcd("  Control Cajero V1 ", 
               "--------------------", 
               "  Hasta luego...    ", 
               "--------------------");
    delay(3000);
    setAuth(false);
  }
  else{  
    setAuth(true);
  }
  asm volatile ("  jmp 0"); 
} 


/**
 * Crea un ciclo hasta presionar el boton PWR, los caracteres presionados
 * se almacenan en el array provisto.
 *   *pStr. Array de tipo char donde almacenar los caracteres.
 *   len. Longitud del array *pStr.
 *   endVal. Tecla del control IR que indica terminacion de captura.
 *   col. Columna donde comenzar a mostrar los caracteres.
 *   row. Fila donde comenzar a mostrar los caracteres
 */
void readIrStringUntil(char *pStr, int len, int endVal, int col, int row){
  int k = 0;
  int j = 0;
  while (k != endVal) { 
    if (irrecv.decode(&results)) {
      
      k = results.value;
      delay(80);                        // Estabiliza la captura
      if(isValidIr){
        if(k == IR_MUTE){
          resetArduino(false);
        }
        if(j < len - 1){                // Para eviar simbolo raro en LCD
          pStr[j] = getcFromIr(k);
          lcd.setCursor(col, row);
          lcd.print(pStr);
//          Serial.print("array: ");
//          Serial.println(pStr);
          j++;
        }
      }
      irrecv.resume();                 // Receive the next value
      delay(100);
    }  
  }
}


/**
 * Crea un ciclo hasta presionar la tecla #, los caracteres presionados
 * se almacenan en el array provisto.
 *   *pStr. Array de tipo char donde almacenar los caracteres.
 *   len. Longitud del array *pStr.
 *   endChar. Caracter que indica terminacion de captura.
 *   col. Columna donde comenzar a mostrar los caracteres.
 *   row. Fila donde comenzar a mostrar los caracteres
 */
void readKeyStringUntil(char *pStr, int len, char endChar, int col, int row){
  char k = 0;
  int j = 0;
  int colCount = 0;
  while (k != endChar) {          
    k = kpd.getKey();
    delay(30);                // Para evitar rebote
    // Almacenar comando en array    
    if(k){
      if(k == 'C'){
        resetArduino(false);  
      }
      if(j < len - 1){        // Para eviar simbolo raro en LCD
        pStr[j] = k;
//        lcd.setCursor(1,2);
        lcd.setCursor(col, row);
        lcd.print(pStr);
        colCount++;
        j++;
      }
     }
   }
}


boolean isValidIr(int irVal){
  if(irVal == IR_0)  return true;
  else if(irVal == IR_1)  return true;
  else if(irVal == IR_2)  return true;
  else if(irVal == IR_3)  return true;
  else if(irVal == IR_4)  return true;
  else if(irVal == IR_5)  return true;
  else if(irVal == IR_6)  return true;
  else if(irVal == IR_7)  return true;
  else if(irVal == IR_8)  return true;
  else if(irVal == IR_9)  return true; 
  else if(irVal == IR_PWR)  return true; 
  else if(irVal == IR_MUTE)  return true; 
  else return false;
}

char getcFromIr(int irVal){
  if(irVal == IR_0)  return '0';
  if(irVal == IR_1)  return '1';
  if(irVal == IR_2)  return '2';
  if(irVal == IR_3)  return '3';
  if(irVal == IR_4)  return '4';
  if(irVal == IR_5)  return '5';
  if(irVal == IR_6)  return '6';
  if(irVal == IR_7)  return '7';
  if(irVal == IR_8)  return '8';
  if(irVal == IR_9)  return '9'; 
}


/**
 * Compara el TAG presentado al lector RF, con el almacenado en el array "tag1"
 */ 
boolean comparetag(int aa[14], int bb[14]){
  boolean ff = false;
  int fg = 0;
  for (int cc = 0 ; cc < 14 ; cc++){
    if (aa[cc] == bb[cc]){
      fg++;
    }
  }
  if (fg == 14){
    ff = true;
  }
  return ff;
}
 

/**
 * Inicializa pantalla LCD y presenta el inicial
 */ 
void initLcd(){
  // Mensaje inicial en LCD
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  
  printToLcd("  Control Cajero V1", 
             "--------------------", 
             "  Esperando TAG RF ", 
             "--------------------");
}


/**
 * Limpia todos los arrays de control
 */ 
void clearAllArrays(){
  for(int i = 0; i < sizeof(cmdKey);  ++i){
    cmdKey[i] = (char)0;
  }
  for(int i = 0; i < sizeof(cmdIr);  ++i){
    cmdIr[i] = (char)0;
  }
  for(int i = 0; i < sizeof(cmdArg);  ++i){
    cmdArg[i] = (char)0;
  }
}

/**
 * Comienza la lectura de los TAG RF y valida el TAG presentado.
 * Retorna. int result.  1) El TAG ha sido validado. 
 *                       0) Se leyo un TAG, pero es invalido
 *                      -1) No hubi lectura de TAG
 */  
int readTags(){
//  Serial1.begin(9600);
  int result = -1;
  if (Serial1.available() > 0) {
    delay(100);                         // Needed to allow time for the data to come in from the serial buffer.
    for (int z = 0 ; z < 14 ; z++) {    // Read the rest of the tag
      data1 = Serial1.read();
      dummyTag[z] = data1;
    }
    Serial1.flush();                       // Stops multiple reads
    // This variable helps decision-making, if it is 1 we have a match, 
    // zero is a read but no match, -1 is no read attempt made
    result = 0;                        
    
    if (comparetag(dummyTag, tag1) == true){
      result++;
    }
  }
  
  if (result > 0) {                    // TAG validado
    lcd.setCursor(0,2);
    lcd.print("  TAG RF Validado   ");
    lcd.setCursor(0,3);
    lcd.print("  Espere...         ");
    isValidated = true;
    result = -1;
    delay(3000);
    lcd.clear();
  }
  else if (result == 0) {              // TAG invalido
    lcd.setCursor(0,2);
    lcd.print("   TAG RF Invalido  ");
    lcd.setCursor(0,3);
    lcd.print("Intentelo nuevamente");
    isValidated = false;
    result = -1;
    initLcd();
    asm volatile ("  jmp 0"); 
  }
//  Serial1.end();
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
 * Imprime los strings en cada fila de la pantalla LCD de 20x4
 */
void printToLcd(char row0[20], char row1[20], char row2[20], char row3[20]){
  lcd.setCursor(0,0);
  lcd.print(row0);
  
  lcd.setCursor(0,1);
  lcd.print(row1);
  
  lcd.setCursor(0,2);
  lcd.print(row2);
  
  lcd.setCursor(0,3);
  lcd.print(row3);
}


boolean isAuth(){
  return EEPROM.read(IS_AUTH);
}

void setAuth(boolean isAuth){
  EEPROM.write(IS_AUTH, isAuth); 
}


/**
 * Limpia los arrays de control de comando, ver comentario al inicio
 **/
void clearAllArrays485(){
  for(int i = 0; i < CMD_LEN; i++){
    respFrom485[i] = "";
    cmdsTo485[i] = "";
  }
}
