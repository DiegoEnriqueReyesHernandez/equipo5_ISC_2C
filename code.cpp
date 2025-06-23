/*# equipo5_ISC_2C
Final proyect from programation 1
Members:
Diego Enrique Reyes Hernandez
Jose Angel Carmona Gonzales
Gustavo de Luna Dorantes
Mauricio Ramirez de la Rosa
*/
#include <graphics.h>
#include <windows.h>
#include <iostream>
#include <stdlib.h>
#include <conio.h>
#include <time.h>

#define TAM 3
#define TAM_CELDAS 100
#define X_INI 200
#define Y_INI 100
void printMenu();
void jugar();
void invalid();
void dibujarCeldas();
void elegirDificultad();
void sonido(int);

const int matFrec[3][3]={ //Matriz que corresponde a la frecuencia del sonido que emitirá cada cuadro
	{261.63, 293.66, 329.63},
	{349.23, 392, 440},
	{493.88, 523.26, 587.32},
};
int main(){
	int gd = DETECT, gm;
	char opcion;
    initgraph(&gd, &gm, (char*)"");
    do{
	    printMenu();
	    opcion = getch();
	 	switch(opcion){
	 		case '1': 
	 			sonido(1);
			 	elegirDificultad();
	 				break;
	 		case '2':
			 		sonido(1); 
			 		break;
	 		default: invalid();
			 		break;
	 			
		 }
		 
	}while(opcion!='2');
}
//Funcion que imprime el menu
void printMenu(){
	cleardevice();
	settextstyle(3, 0, 4);
    outtextxy(150, 100, (char*)"FABULOSO FRED");
    settextstyle(10, 0, 2);
    outtextxy(250, 200, (char*)"1. Jugar");
    outtextxy(250, 225, (char*)"2. Salir");
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

void elegirDificultad(){ //Función que permite elegir la dificultad, esto determinará cuanto tiempo de espera hay entre cada paso de la muestra de pasos
	//TODO: incluir un switch para retornar el tiempo de espera a la función del juego, falta retornar un valor.
	cleardevice();
	int sel;
	settextstyle(10, 0, 2);
    outtextxy(250, 120, (char*)"1. Facil");
    outtextxy(250, 145, (char*)"2. Medio");
    outtextxy(250, 170, (char*)"3. Dificil");
    sel = getch();
    sonido(1);
}

void sonido(int opc){//Función para emitir sonido, el numero entero pertenece al sonido que se desea emitir dependiendo de la intención del beep
	/*
	1.- Opción seleccionada
	2.- Selección incorrecta
	3.- Música de victoria
	*/
	switch(opc){
		case 1:
			Beep(550,400);
			break;
		case 2:
			Beep(300,400);
			Beep(250,600);
			break;
		case 3:
			//TODO: Hacer una cancion de victoria
			break;
		default:
			break;
	}
}
