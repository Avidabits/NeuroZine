
import papaya.*; //uso de estadísticas

class Channel {
  // Value object class to store EEG power information for each channel.
  // One instance per EEG channel.

  String name;
  int drawColor;
  String description;
  boolean graphMe;
  boolean relative; //no se usa nunca?
  int maxValue=0;
  int minValue=400000;
  int meanValue=0;
  ArrayList puntos;
  
  boolean allowGlobal;
  

  Channel(String _name, int _drawColor, String _description) {
    name = _name;
    drawColor = _drawColor;
    description = _description;
    allowGlobal = true;
    puntos = new ArrayList();
  }

  void addDataPoint(int value, long time) {
   
    if (value > maxValue) maxValue = value;
    if (value < minValue) minValue = value;
    meanValue=(meanValue+value)/2; // en realidad esto es válido sólo asumiendo tiempos de lectura constantes.
    
    puntos.add(new Point(time, value));
  }
  
  void removeFirstPoint()
  {
    if (size()>0) puntos.remove(0);
  } //para evitar que la lista sobrepase un deterinado tamano
  
  Point getLatestPoint() {
    if (size() > 0) {
      return (Point)puntos.get(size() - 1);
    }
    else {
      return new Point(0, 0);
    }   
  } // getLastestPoint
  
  int getLatestValue()
  {
    if (size() > 0)  return ((Point)puntos.get(size() - 1)).value;
    else return 0;
    
  }

  int size()
  {
    return puntos.size();
  }
  Point get(int i)
  { 
    return (Point)puntos.get(i);
  }
  
  void imprimeEstadisticas()
  {
    if (size()<=0) return;
    println(name+": valor mínimo="+minValue+" valor máximo="+maxValue + " valor medio="+meanValue);
    float[] valores=new float[size()];
    for (int i=0; i<size(); i++) valores[i]=(float)(get(i).value);
    float media_aritmetica=Descriptive.Mean.arithmetic(valores);
    float suma=Descriptive.Sum.sum(valores);
    float media_armonica=Descriptive.Mean.harmonic(valores);
    float maximo=Descriptive.max(valores);
    float minimo=Descriptive.min(valores);
    float desviacion_estandar=Descriptive.std(valores, false); //unbiased
    println("minimo="+minimo+ " maximo="+maximo+" media aritmetica=" + media_aritmetica + " suma="+suma+ " media_armonica="+media_armonica+ " desviacion estandar="+desviacion_estandar);
    

  }
    
} // class Channel