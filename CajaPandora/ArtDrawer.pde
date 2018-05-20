/*
Vamos a generar distintos tipos de visualizaciones para nuestras ondas cerebrales
Desde la clase ArtDrawer proporcionaremos una interfaz comun a todos los 
visualizadores cerebrales, que será la interfaz usada desde el programa principal

*/

// todos los pintadores de ondas deben implementar esta interfaz
public interface PintaOndas {
  void inicializa(); 
  void actualiza(Channel [] _channels);
  void pinta();
  void finaliza();
}

class ArtDrawer implements PintaOndas{
  PintaOndas pintador=null; //PENDIENTE: podemos decidir mas tarde si mantenemos un solo pintador o una lista para que haya varios simultaneos
  void inicializa() // asigna un pintaondas por defecto
  {
    inicializa('a'); //el pintaondas por defectos son los personajes abstractos
  }
  void inicializa(char tipoPintaOndas)
  {
    switch(tipoPintaOndas) {
       case 'C' :
       case 'c': 
          pintador=(PintaOndas)new PintaChisporroteos();         
          break; 
       case 'A' :
       case 'a': 
          pintador=(PintaOndas)new PintaPersonajesAbstractos(); 
          break;        
       default : 
          println("pintaOndas no implemetado");
       break;
    }
    if (pintador!=null) pintador.inicializa();
  }
  void actualiza(Channel [] _channels) {if (pintador!=null) pintador.actualiza(_channels);}
  void pinta() {if (pintador!=null) pintador.pinta();}
  void finaliza(){
    pintador=null;
  };
}