/*# equipo5_ISC_2C
Final proyect from programation 1
Members:
Diego Enrique Reyes Hernandez
Jose Angel Carmona Gonzales
Gustavo de Luna Dorantes
Mauricio Ramirez de la Rosa
*/
#include <graphics.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <conio.h>

#define TAM 3
#define TAM_CELDAS 100
#define X_INI 200
#define Y_INI 100
void printMenu();
void jugar();
void invalid();
void dibujarCeldas();
int main(){
	int gd = DETECT, gm;
	char opcion;
    initgraph(&gd, &gm, (char*)"");
    do{
	    printMenu();
	    opcion = getch();
	 	switch(opcion){
	 		case '1': jugar();
	 				break;
	 		case '2': 
			 		break;
	 		default: invalid();
			 		break;
	 			
		 }
		 
	}while(opcion!='2');
}
//Funcion que imprime el menu
void printMenu(){
	settextstyle(3, 0, 4);
    outtextxy(150, 100, (char*)"FABULOSO FRED");
    settextstyle(3, 0, 2);
    outtextxy(250, 200, (char*)"1. Jugar");
    outtextxy(250, 250, (char*)"2. Salir");
}
void jugar(){
	dibujarCeldas();
	char e=getch();
}
void invalid(){
	cleardevice();
    settextstyle(3, 0, 4);
    outtextxy(150, 100, (char*)"Opcion invalida");
    delay(1000);
}
void dibujarCeldas(){
	cleardevice();
	setcolor(WHITE);
    for (int i = 0; i <= TAM; i++) {
        line(X_INI, Y_INI + i * TAM_CELDAS, X_INI + TAM * TAM_CELDAS, Y_INI + i * TAM_CELDAS); // H
        line(X_INI + i * TAM_CELDAS, Y_INI, X_INI + i * TAM_CELDAS, Y_INI + TAM * TAM_CELDAS); // V
    }
}
