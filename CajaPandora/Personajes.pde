

// todos los pintadores de ondas deben implementar esta interfaz
// En este archivo, vamos haciendo nuestros experimentos

class PintaPersonajesAbstractos implements PintaOndas {
  // en esta clase voy a poner conceptos asociados a 
  // las lecturas de ondas
  int sopor=0, maxSopor=1, prevSopor=0;
  int densidad=0, prevDensidad=0; //densidad de la cortina de sopor
  ///////
  // una raya  loca recorre el horizontalmente el cuadro
  // cuanto mayor es la multiatención mayores numero de quiebros
  int multiAtencion=0, maxMultiAtencion=1, prevMultiAtencion=0;
  PVector[] vectores=null;
  PVector centroAtraccion;
  
  int relajacion=0, maxRelajacion=1, prevRelajacion=0, radioRelajacion=0, prevRadioRelajacion=0;
  int alerta=0, maxAlerta=1, prevAlerta=0, radioAlerta=0, prevRadioAlerta=0;  
  int meditacion=0, maxMeditacion=1, prevMeditacion=0;  
  int atencion=0, maxAtencion=1, prevAtencion=0;  
  

  void inicializa()
  {
    // estos vectores representan los punto de la raya loca del estado multiatencion 
    vectores=new PVector[15];
    int h=height/2;
    int dx=width/vectores.length;
    for (int i=0; i<vectores.length; i++) vectores[i]=new PVector(i*dx, h);
    
    centroAtraccion=new PVector(width/2, height/2);
    
  }//inicializa
  
  void actualiza(Channel [] _channels)
  {
    //aqui recojo de los canales la información que necesito para pintarme.
    prevSopor=sopor;
    sopor=_channels[3].getLatestValue()+_channels[4].getLatestValue();
    maxSopor=max(maxSopor, sopor);
    //maxSopor=3926187;
    prevMultiAtencion=multiAtencion;
    multiAtencion=channels[9].getLatestValue()+_channels[10].getLatestValue();
    maxMultiAtencion=max(prevMultiAtencion, multiAtencion);
    
    //relajacion
    prevRelajacion=relajacion;
    relajacion=channels[5].getLatestValue()+_channels[6].getLatestValue();
    maxRelajacion=max(prevRelajacion, relajacion);
    radioRelajacion=ceil(map(relajacion, 0, maxRelajacion, 0, width/3));
    // para decidir de forma precisa como actualizamos el radio, deberíamos tener en 
    //cuenta el tiempo entre una actualizacion y otra, que no coincide con el framerate
    // porque los eventos de actualizacio vendrán del puerto serie y el frame rate es fijo
    if (radioRelajacion > prevRadioRelajacion) radioRelajacion=prevRadioRelajacion+1;
    else if (radioRelajacion < prevRadioRelajacion) radioRelajacion=prevRadioRelajacion-1;  
    prevRadioRelajacion=radioRelajacion;

    //alerta
    prevAlerta=alerta;
    alerta=channels[7].getLatestValue()+_channels[8].getLatestValue();
    maxAlerta=max(prevAlerta, alerta);
    radioAlerta=ceil(map(alerta, 0, maxAlerta, width/40, 0));
    // para decidir de forma precisa como actualizamos el radio, deberíamos tener en 
    //cuenta el tiempo entre una actualizacion y otra, que no coincide con el framerate
    // porque los eventos de actualizacio vendrán del puerto serie y el frame rate es fijo
    if (radioAlerta > prevRadioAlerta) radioAlerta=prevRadioAlerta+1;
    else if (radioAlerta < prevRadioAlerta) radioAlerta=prevRadioAlerta-1;  
     prevRadioAlerta=radioAlerta;
    
    
    //meditacion
    prevMeditacion=meditacion;
    meditacion=channels[INDEX_MEDITACION].getLatestValue();
    maxMeditacion=max(prevMeditacion, meditacion);   

    //atencion
    prevAtencion=atencion;
    atencion=channels[INDEX_ATENCION].getLatestValue();
    maxAtencion=max(prevAtencion, atencion);   
    
    // con la meditacion y la atencion lo que hacermos es calcular un
    // punto central 
    // haremos que una atencion elevada se relacion con una posicion alta en Y
    // y que una meditacion alta se relacion con posicion central en X 
    // como estos parametros los usaremos en varios elementos conceptuales del grafico
    // los calculamos ahora. 
    // El punto del centro Y se refiere a la media entre atencion minima y maxima 
    PVector newCentro=new PVector();
    int y=ceil(map(atencion, 0, maxAtencion, height, 0)); //pendiente: hay que recoger los minimos
    int x=ceil(map(meditacion, 0, maxMeditacion, 0, width));
    newCentro.set(x, y);
    centroAtraccion.lerp(newCentro, 0.001); // el movimiento hacia el centro debe ser suave
    // calcular la velocidad de movimiento:
    // si quiero que tarde 1 segundo en moverse de un lado al otro ¿cuantos pixeles por frame?
    // puesto que actualiza no tiene porque llamarse desde draw, no podemos contar usando framerate
    //frameRate  
    //esto no me convence del todo...
    // ademas de no tener claro que la meditacion represente un movimiento hacia la derecha
    // por ahora ambos movimientos son lineales, 
    // PENDIENTE: probar con un mapeo no lineal. 
    // se puede hacer mapeando primero entre 0 y uno, elevando el resultado a una constante
    // despues mapeando de cero - uno a los valores finales.
    
    
  }//actualiza
  void desvaneceFondo()
  {
    noStroke();
    fill(colorFondo,50); //background 0 para proyector y 255 para 
    rect(0, 0, width, height); // fades background    fill(0, 100); 
    // el fondo se desvanece con el tiempo, no se borra lo anterior se deja un rastro
  }
  
  void pinta()
  {
    //desvaneceFondo();
    background(colorFondo);
    pintaCortinaBezier();
    pintaMultiAtencion(); // ondas gamma
    pintaRelajacion(); //onfas alfa, canales 5 y 6
    pintaAlerta(); //ondas beta, canales 7 y 8
    pintaSopor(); // Las ondas delta y theta ... muy altas si estamos atontolinaos...
    //filter(BLUR); //lo quito por quedar muy lento
  }//pinta
  
  void finaliza()
  {
    vectores=null;
  }//finaliza
  
  void pintaSopor()
  {
    //sobrepongo rectangulos transparentes
    // generan un veladura que representa el sopor
    //sopor=maxSopor;
    float alfa=map(sopor, 0, maxSopor, 0, 1); //cero transparente
    float prevAlfa=map(prevSopor, 0, maxSopor, 0, 1); //cero transparente
    //hago que vaya de 0 a 100 pero de forma exponencial X elevado a 4
    alfa=pow(lerp(prevAlfa, alfa, 0.5), 4)*100; 
    //uso una interpolacion entre en Alfa anterior y el nuevo para que el movimiento
    // mas suave y no de saltos bruscos
    noStroke();
    fill(255, alfa);
    rect(0, 0, width, height);
    prevAlfa=alfa; 
    
  }//pinta sopor
  
  void pintaAlerta()
  {
    // un circulo blanco palpita segun los niveles de alerta (canales 5 y 6) 
    
    stroke(255, 100); //100->opaco
    noFill();
    // con esto pintamos bolas
    
    strokeWeight(radioAlerta);
    //pos depende de atencion , meditacion
    ellipse(centroAtraccion.x, centroAtraccion.y, radioAlerta*8, radioAlerta*8);  //<>//
  }
  
  int  mapExp(float valor, float rangoValorMin, float rangoValorMax, float rangoObjetivoMin, float rangoObjetivoMax, float exp)
  {
    float valor_exponencial=pow(map(valor, rangoValorMin, rangoValorMax, 0, 1), exp); 
    return ceil(map(valor_exponencial, 0, 1, rangoObjetivoMin, rangoObjetivoMax));
    
  }
    
  void pintaRelajacion()
  {
    // una bola azul palpita segun los niveles de relajacion (canales 5 y 6) 
   

    stroke(#6010E3, 100); //100->opaco
    strokeWeight(radioRelajacion);
    //pos depende de atencion , meditacion
    point(centroAtraccion.x, centroAtraccion.y); 
    
  }
  void pintaMultiAtencion()
  {
    // una raya blanca loca recorre el horizontalmente el cuadro
    // cuanto mayor es la multiatención mayores numero de quiebros
    // vamos a usar el maximo historico 1446308, PENDIENTE, analizar esto para todo
    
    int maxStroke=round(map(multiAtencion, 0, 1446308, 0, 12));
    //println (maxStroke, " maxStroke");
    int maxMag=ceil(map(multiAtencion, 0, 1446308, 0, 50));
    PVector ruido=PVector.random2D();
    for (int i=0; i<vectores.length; i++) {
      
      ruido.rotate(random(-PI,PI));
      ruido.setMag(random(0, maxMag));
      vectores[i].add(ruido);
      // no dejar que se salga de la pantalla
      if (vectores[i].x>width-2*maxStroke) vectores[i].x=width-2*maxStroke;
      if (vectores[i].x<2*maxStroke) vectores[i].x=2*maxStroke;
      if (vectores[i].y>height-2*maxStroke) vectores[i].y=height-2*maxStroke;
      if (vectores[i].y<2*maxStroke) vectores[i].y=2*maxStroke;
    
    }
    stroke(#E39D10);
    noFill();
    beginShape();
    for(int i=0; i<vectores.length; i++)
    {
      strokeWeight(ceil(random(0,maxStroke)));
      curveVertex(vectores[i].x, vectores[i].y);
    }
    curveVertex(vectores[vectores.length/2].x, vectores[vectores.length/2].y);
    endShape();
  }//pintaMultiAtencion
  
    
void pintaCortinaBezier()
{
 //curtain
     stroke(#F4ADFC);
     noFill();
     strokeWeight(0.01);
     beginShape();
     //la densidad de la cortina queremos que sea proporcional al sopor
     int incrementoDensidad=50;
     int densidad=ceil(map(sopor, 0, maxSopor, 0, 1600));
     if (prevDensidad==0) prevDensidad=densidad; //inicializamos la densidad previa
     if (densidad > prevDensidad) densidad=prevDensidad+incrementoDensidad; // para que la densidad aumente 
     else if (densidad<prevDensidad) densidad=prevDensidad-incrementoDensidad;// o disminuya en progresion
     int despliegue=ceil(map(densidad, 0, 1600, 1, 8));
     for(int x = 100; x < densidad; x += despliegue) {
       float n = map(x, 0, 1600, 0, 5);
       float p = pow(n, 4); // rango de 0 a 625
       vertex(-x, p);
       bezierVertex(n,p,-x,-n,x*4,height);
  
     }    
       
     endShape();
     prevDensidad=densidad;
   }// otroPinta
   
}// class PersonajesAbstractos