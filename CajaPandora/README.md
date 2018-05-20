
##CAJA DE PANDORA

### Descripci�n
Software: Caja de Pandora
Software visualizador de ondas cerebrales obtenidas mediante el craqueo del   juguete MindFlex de NeuroSky

-Usa el la librer�a Brain de Eric Mika (https://github.com/kitschpatrol/Brain) 
-En el Arduino ha de estar cargada la librer�a de Eric Mika BrainSerialTest
-Para craquear el Mindset simplemente basta con abrirlo, acceder a la placa, conectar la tierra de la placa con la tierra del Arduino   conectar la el TX del MindSet con el RX del Arduino.
  
  Autora: Rosa Bern�rdez. Avidabits
  mail: avidabits@gmail.com
 
### Funcionamiento
   El Arduino ha de estar conectado en el puerto indicado por la variable index_arduino 
   Comandos de teclado
   L, l: Activa o Desactiva el almacenamiento de ondas cerebrales en el fichero ondas_cerebrales.txt
   E, e: Imprime en la consola estad�sticas de las ondas");
   S, s: Activa o Desactiva la generaci�n de ondas simuladas, que permite trabajar mientras no estamos conectados
   C, c: Pintas las ondas como chisporroteos cerebrales. La met�fora usada es la de un sistema de part�culas
   A, a: Pintar personajes abstractos vinculados a las ondas
   F, f: Activa o desactiva el salvado de frames en el directorio frames
Este programa ha sido creado partiendo del  BrainGrapher como inspiraci�n, 

Para escribir este software he partido del BrainGrapher de Eric Mika (https://github.com/kitschpatrol/BrainGrapher). 
Tras las modificaciones para visualizaci�n artistica, queda el fichero channel.pde, para conservar los datos recogidos con la libreria de Eric Mika brain.cpp


