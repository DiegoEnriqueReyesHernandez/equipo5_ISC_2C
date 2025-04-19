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
#include <stdio.h>
#include <conio.h>

#define TAM 3
#define CELL_SIZE 100
void printMenu();
int main(){
	int gd = DETECT, gm;
	char opcion;
    initgraph(&gd, &gm, (char*)"");
    printMenu();
    opcion = getch();
    
}
//Funcion que imprime el menu
void printMenu(){
	settextstyle(3, 0, 4);
    outtextxy(150, 100, (char*)"JUEGO DE MEMORIA");
    settextstyle(3, 0, 2);
    outtextxy(250, 200, (char*)"1. Jugar");
    outtextxy(250, 250, (char*)"2. Salir");
}
