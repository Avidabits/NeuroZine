
/*
  Por cada muestra temporal, quiero pintar una chsiporroteo
  Un chisporrteo es un conjunto de chispas, cada chispa es una muestra de canal
  Los todos chisporroteos se guardan en un array
  Cuando el chisporroteo decae, se borra del array
  Cada chispa es un modelo de particula con velocidad, aceleracion y tamaño, aunque sin masa
  el tamaño representa la potencial del canal y va decayendo con el tiempo
  La posicion de partica es central y se aleja con el tiempo, un ruido aleatorio se usa para generar un movimiento pseudo
  browniano
  */

class Chispa
{
  int drawColor;
  int radio;
  long nacimiento;
  PVector pos, vel, acc;//posicion, velocidad y aceleracion
  
  Chispa(int _drawColor, int _x, int _y, int _radio)
  {
    drawColor=_drawColor;   
    pos=new PVector(_x, _y);
    // le asignamos una velocidas modular fija pero con angulo aleatorio 
    // la velocidad de la chispa va a ser proporcional a su radio
    int velocidad_inicial=max(1, _radio/20);
    vel=new PVector(velocidad_inicial, velocidad_inicial); // pixeles por actualizacion en cada direccion
    
    vel.rotate(random(0, 2*PI)); // direccion aleatoria
    acc=new PVector(0, 0); // por el momento velocidad constante
    radio=_radio;
    nacimiento=System.currentTimeMillis();

  }
  long edadMillis() {return System.currentTimeMillis() - nacimiento; }
  
  void draw()
  {
    float alpha=map(edadMillis(), 0, 5000, 100,0);//al llegar a 5 segundos se vuelve trasparente
    stroke(drawColor, alpha); 
    // con esto pintamos bolas
    strokeWeight(radio);
    point(round(pos.x), round(pos.y));  
    //draw_star(round(random(5, 12))); 
    //draw_star(2);  
  }
  void draw_star(int puntas)
  {
    PVector punto_final=new PVector(radio*0.6, 0);
    punto_final.rotate(random(-0.5,0.5)); //para que las puntas no empiecen siempre en el mismo sitio
    float rotacion=2*PI/puntas;
    strokeWeight(1);
    for (int i=0; i<puntas; i++)
    {
      punto_final.rotate(rotacion);
      line(round(pos.x), round(pos.y),round(pos.x)+punto_final.x, round(pos.y)+punto_final.y);
    }
  }
  void update()
  { 

    if (radio>0) radio=radio-ceil(float(radio)/200);//vamos reduciendo el radio
    if (radio<0) radio=0;
    vel.add(acc);
    vel.rotate(random(-0.5,0.5)); // PRUEBA A ver como quedan ruidos aleatorios en la trayectoria
    pos.add(vel);     
  }
  
  void mueveteInercial(PVector velocidad, PVector aceleracion)
  {
    // una veces querremos mover la chispa con su propia velocidad y aceleracion 
    // y otras veces querremos avanzar con la velocidad y aceleracion de un grupo de viaje conjunto
    // este método no cambia el radio, ni la velocidad y aceleracion individual, solo la posicion
    velocidad.add(aceleracion);
    pos.add(velocidad);
    
  } 
  
}//Chispa



/* Un chisporroteo es un conjunto de chispas
Cada muestra cerebral de los 8 canales va a ser un chisporroteo de 8 chispas
que se mueven cojuntamente en cuanto a la posicion central
pero que se separan ligeramente de ella sin salir de su radio
*/

class Chisporroteo extends Chispa
{
  Chispa[] chispas;

  Chisporroteo(Channel [] _channels, color _drawColor, int _x, int _y, int _radio)
  {
    super (_drawColor,  _x,  _y,  _radio);
    int valorMaximoCanales=0;
    for (int i=3; i<_channels.length; i++) valorMaximoCanales=max(valorMaximoCanales, _channels[i].maxValue);
    // el valor maximo historico de todos los canales se debe corresponder con _radio
    // y los valores actuales de la muestra serviran para generar el radio de la chispa
    chispas = new Chispa[8];
    for (int i = 0; i < chispas.length; i++)
    {
      Point punto=_channels[i+3].getLatestPoint(); 
      int nuevoRadio=round(map(punto.value, 0, valorMaximoCanales, 5, _radio)); //el radio minimo no lo pongo a cero, sino a unos pocos pixels
      chispas[i] = new Chispa(_channels[i+3].drawColor,  _x,  _y,  nuevoRadio); 
      
    }

  }
  void update() {
    super.update();
   
    for (int i = 0; i < chispas.length; i++)
    {
      chispas[i].update();
      chispas[i].mueveteInercial(vel, acc);
    }
  }
  void draw ()
  {

    for (int i = 0; i < chispas.length; i++)
    {
      chispas[i].draw();
    } 

  }
} // Chisporroteo

void fadeBackground() // helper
{
  noStroke();
  fill(colorFondo,100); //background 0 para proyector y 255 para 
  rect(0, 0, width, height); // fades background
  noFill();
}
      
class PintaChisporroteos implements PintaOndas {
  ArrayList<Chisporroteo> chisporroteos=null; // conjunto de chisporroteos, cada muestra cerebral incluyendo todos los canales es un chisporroteo
  void inicializa()
  {
    background(0); // si se trata de proyector cero(negro) si se trata de pantalla 255(blanco)
    if (chisporroteos==null) chisporroteos=new ArrayList<Chisporroteo>();
    
  }
  void finaliza()
  {
    chisporroteos=null; 
  }
  void pinta(){
    //background(255, 255, 255); 
    fadeBackground(); //permite que se vea la estela que dejan las chispas
    for (int i = 0; i < chisporroteos.size(); i++) {
      chisporroteos.get(i).draw();
      chisporroteos.get(i).update();
    } 
    //filter(BLUR); //lo quito por quedar muy lento
  } //`pinta

  void actualiza(Channel [] _channels)
  {
    // La posicion Y de nacimiento de chisporroteos que sea relativa al grado de atencion.
    // La posicion X de nacimiento de chisporroteos que sea relativa al grado de meditacion.
    int nacimientoY=round(map(_channels[INDEX_ATENCION].getLatestValue(), _channels[INDEX_ATENCION].minValue, _channels[INDEX_ATENCION].maxValue, height, 0));
    int nacimientoX=round(map(_channels[INDEX_MEDITACION].getLatestValue(), _channels[INDEX_MEDITACION].minValue, _channels[INDEX_MEDITACION].maxValue, 0, width));
    Chisporroteo chisporroteo=new Chisporroteo(_channels, color(255, 255, 255), nacimientoX, nacimientoY, min(width, height)/10);
    chisporroteos.add(chisporroteo);
    // no nos interesa una acumulacion de chisporroteos que ya no se pueden representar.
    // podemos limiparlos cuando el radio es cero, solo borramos los mas viejos, por eso 
    while (chisporroteos.size()>0 && chisporroteos.get(0).radio==0) chisporroteos.remove(0);
  }
}


  