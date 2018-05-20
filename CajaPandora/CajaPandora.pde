/*
  Software: Caja de Pandora
  
  Software visualizador de ondas cerebrales obtenidas mediante el craqueo del 
  juguete MindFlex de NeuroSky
  Usa el la librería Brain de Eric Mika. 
  En el Arduino ha de estar cargada la librería de Eric Mika BrainSerialTest
  Para craquear el Mindset simplemente basta con abrirlo, acceder a la placa, 
  conectar la tierra de la placa con la tierra del Arduino
  conectar la el TX del MindSet con el RX del Arduino.
  
  Autora: Rosa Bernárdez. Avidabits
  mail: avidabits@gmail.com
  
  Funcionamiento:
   El Arduino ha de estar conectado en el puerto indicado por la variable index_arduino
   Comandos de teclado
   L, l: Activa o Desactiva el almacenamiento de ondas cerebrales en el fichero ondas_cerebrales.txt
   E, e: Imprime en la consola estadísticas de las ondas");
   S, s: Activa o Desactiva la generación de ondas simuladas, que permite trabajar mientras no estamos conectados
   C, c: Pintas las ondas como chisporroteos cerebrales. La metáfora usada es la de un sistema de partículas
   A, a: Pintar personajes abstractos vinculados a las ondas
   F, f: Activa o desactiva el salvado de frames en el directorio frames
       
  
*/

import processing.serial.*;
// En el Arduino ha de estar cargada la liberia BrainSerialTest de Eric Mika y la tierra del mindset a tierra y el Tx del mindset al RX del Arduino

//librerias añadidas para la grabacion de ficheros 
import java.io.File;
import java.io.FileWriter;
import java.util.Date;


Serial serial;

Channel[] channels = new Channel[11]; // el primer canal lleva a calidad de señal
int INDEX_CALIDAD=0;
int INDEX_ATENCION=1;
int INDEX_MEDITACION=2;

boolean bSimulaOn=true;
boolean bSaveFrames=false;
int packetCount = 0; //numero de paquetes, terminados en linefeed, leidos por el puerto serie.


ArtDrawer pintador=null;
color colorFondo=0; //0 negro para proyecto, 255 blanco, para pantallas

void setup() {
  // Set up window
  size(displayWidth, displayHeight);// M
  
  frameRate(25);
  smooth();
  surface.setTitle("Caja de pandora");  
  

  println("Pulsa L o l para activar o desactivar el log ondas_cerebrales.txt");
  println("Pulsa E o e para activar imprimir estadísticas");
  println("Pulsa S o s para generar muestras simuladas");
  println("Pulsa C o C para pintar chisporroteos");  
  println("Pulsa A o a para pintar personajes abstractos vinculados a las ondas");
  println("Pulsa F o f para activar o desactiva el salvado de frames ");
  
  // Set up serial connection
  println("Find your Arduino in the list below, note its [index]:");
  
  for (int i = 0; i < Serial.list().length; i++) {
    println("[" + i + "] " + Serial.list()[i]);
  }
  
  
  int index_arduino=1;
  // Put the index found above here:
  if (index_arduino<Serial.list().length)
  {
    serial = new Serial(this, Serial.list()[index_arduino], 9600);    
    serial.bufferUntil(10); // lee del puerto serie hasta encontrar un caracter de linefeed
    println("Leyendo puerto ", index_arduino, "a 9600 baudios");
    bSimulaOn=false;
  }
  else 
   {
     println("No hay puerto de lectura, modo simulacion");
     bSimulaOn=true;
   }
  
   // Create the channel objects // RMBR CAMBIO EN COLORES
  channels[INDEX_CALIDAD] = new Channel("Calidad de señal", color(0), "");
  channels[INDEX_ATENCION] = new Channel("Atención", color(100), "");
  channels[INDEX_MEDITACION] = new Channel("Meditación", color(50), "");
  // de 0.5 a 3.5 hz. Se generan mientras se duerme o debido a un transtorno fisico en el cerebrio
  channels[3] = new Channel("Delta (0.5-3.5 Hz)", color(#0A7AF0), "Duermes o tienes un trastorno físico en el cerebro"); 
  // theta: 3.5 a 7.5 hz, ineficiencia, somnolencia, la frontera entre estar dormido o despierto
  // Acceso al material del subconsciente, inspiracion creativa y meditacion profunda
  // pueden surgir del estres emocional, la decepcion y la frustracion
  // Altos niveles de theta son anormales en adultos y estan muy relacionados TDAH
  channels[4] = new Channel("Theta (3.5-7.5 Hz)", color(92, 9, 132), "Somnolencia, meditación o TDAH");
  // alfa: de 7.5 a 12 Hz. relacionadas con la relajacion y desvinculacion
  channels[5] = new Channel("Low Alpha (>7.5)", color(132, 9, 132), "Mucha relajación");
  channels[6] = new Channel("High Alpha (<12-Hz)", color(171, 9, 114), "Relajación");
  // beta de 12 a 30 hz. Concentracion enfocada, resistirse a un movimiento o suprimirlo, resolver tareas complejas, matematicas
  channels[7] = new Channel("Low Beta (>12Hz)", color(194, 9, 92), "Alerta");                  
  channels[8] = new Channel("High Beta (<30z)", color(222, 9, 66), "Muy Alerta");             
  // gamma, desde 3  a 100 Hz. relacionadas con la consciencia, cognicion y la atencion
  channels[9] = new Channel("Low Gamma (>30)", color(231, 99, 40), "Proceso multisensorial"); 
  channels[10] = new Channel("High Gamma", color(237, 161, 23), "???");
  
  // Manual override for a couple of limits.
  channels[0].minValue = 0;
  channels[0].maxValue = 200;
  channels[1].minValue = 0;
  channels[1].maxValue = 100;
  channels[2].minValue = 0;
  channels[2].maxValue = 100;
  channels[0].allowGlobal = false;
  channels[1].allowGlobal = false;
  channels[2].allowGlobal = false;

   pintador = new ArtDrawer();
   if (pintador!=null) pintador.inicializa(); // tiene por defecto chisporroteos

}


synchronized void draw(){
  
   if (bSimulaOn) {
      generaMuestraAleatoria();//simulacion:quitar esta generacion artificial de muestras
      pintador.actualiza(channels);
   }
   pintador.pinta();
   if (bSaveFrames) saveFrame("frames\\pandora-######.tif");
 
}


void generaMuestra(String incomingString)
{
  // See https://github.com/kitschpatrol/Arduino-Brain-Library/blob/master/README for information on the CSV packet format
  String[] incomingValues = split(incomingString, ',');
  // Verify that the packet looks legit
  if (incomingValues.length <=1)  return;
  LogString(incomingString+"\n"); /*RMBR*/
  long time = System.currentTimeMillis(); // RMBR: El mismo tiempo para todos los canales, tomado inmediatamente al recibir el evento

  for (int i = 0; i < incomingValues.length; i++) {
      String stringValue = incomingValues[i].trim();
      int newValue = Integer.parseInt(stringValue);
      
      // Zero the EEG power values if we don't have a signal.
      // Can be useful to leave them in for development.
      if ((Integer.parseInt(incomingValues[0]) == 200) && (i > 2)) {
          newValue = 0;
      }
      
      channels[i].addDataPoint(newValue, time);
      //si queremos que dure mucho tiempo, no hay que dejar que se acumulen datos. 
      //cuando la longitud exceda determinado valor quitar el primer punto
      if (channels[i].size()>500) channels[i].removeFirstPoint(); 
    
    }//para cada valor, es decir para cada canal de datos
} 

void generaMuestraAleatoria()
{
  //generaMuestra("26,30,48,539208,1092968,1759531,442149,352094,363780,149525,104737");
  String muestra=ceil(random(25, 51))+","+ceil(random(14, 83))+","+ceil(random(20, 90))+","; //calidad, atencion, meditacion
         muestra+=ceil(random(3548, 2248426))+","+ceil(random(6992, 1677761))+","; //delta y theta (somnolencia)
         muestra+=ceil(random(981, 1183148))+","+ceil(random(283, 411430))+","; // low. high alpha
         muestra+=ceil(random(502, 127813))+","+ceil(random(251, 240618))+",";// low high beta
         muestra+=ceil(random(127, 71638))+","+ceil(random(2952, 1446308))+"\n"; // low high gamma
         //print("muestra aleatoria: "+muestra);
  generaMuestra(muestra); 
}  
synchronized void serialEvent(Serial p) {
  
  // Split incoming packet on commas
  String incomingString = p.readString().trim();
  packetCount++;
  print("Received string over serial: ");
  println(incomingString);   
  // Wait till the third packet or so to start recording to avoid initialization garbage.
  if (packetCount <= 3) return;


  generaMuestra(incomingString);
     //<>//
  pintador.actualiza(channels);
  
}// serial Event


// Utilities

// Extend Processing's built-in map() function to support the Long datatype
long mapLong(long x, long in_min, long in_max, long out_min, long out_max) { 
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Extend Processing's built-in constrain() function to support the Long datatype
long constrainLong(long value, long min_value, long max_value) {
  if (value > max_value) return max_value;
  if (value < min_value) return min_value;
  return value;
}


/*RMBR - Aqui empiezan nuevas funciones que he añadido*/
boolean bLogOn=false; // inicialmente no hacemos log. Se conmutará la funcionalidad al pulsar la tecla l o L
// esta funcion tiene como objetivo grabar en un fichero las lecturas de MindFlex

String fichero_log="brain_log.txt";

void GeneraNuevoLog()
{
   fichero_log = "brain_log_" + String.valueOf(year()) + String.valueOf(month()) + String.valueOf(day()) + String.valueOf(hour()) + String.valueOf(minute()) + String.valueOf(second()) + ".txt";
   LogString("Calidad de Señal,Atención,Meditación,Delta,Teta,Low Alfa, Hight Alfa, Low Beta, Hight Beta,Low Gamma, Hight Gamma\n");//TO DO: poner el dia y la hora 

}


void   LogString(String string) { /*RMBR*/
   
    if(!bLogOn) return;
    try{
        File file=new File(fichero_log);
        FileWriter filewriter=new FileWriter(file, true); // creamos el fichero para añadir al final
        filewriter.append(string);
        filewriter.flush();
        filewriter.close();
    } catch (IOException e){
        System.out.println("error de fichero..... ");
    }
      
}


// Habilitaremos o deshabilitaremos ciertas funciones , como el logging, pulsando teclas
void keyPressed() {
   
  switch(key)
  {
    case 'l':
    case 'L':
       bLogOn=!bLogOn; 
       if (bLogOn) {
         println("Log activado");
         GeneraNuevoLog();
       }
       else println("Log desactivado");

     case 'C':
     case 'c': //chisporroteo
     case 'a':
     case 'A': // personajes abstractos
       pintador.inicializa(key);
       redraw();
     break;
     case 's':
     case 'S': //simulacion
      bSimulaOn=!bSimulaOn;
      if (bSimulaOn) println("Simulacion Activada");
      else println("Simulacion desactivada");
      redraw();    
      break;
     case 'e':
     case 'E': //estadisticas
       // Para cada canal que imprima las estadisticas de ese canal
       for (int i=0; i< channels.length; i++)
       {
         channels[i].imprimeEstadisticas();
       }
      break;
    case 'F' :
    case 'f':
       bSaveFrames=!bSaveFrames;
       if (bSaveFrames) println("Guardado de frames activo");
       else println("Guardado de frames desactivado");
       break;
      default: 
      break;
     
  }// switch

}