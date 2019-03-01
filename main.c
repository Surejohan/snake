#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "mipslab.h"  /* Declatations for these labs */

int main(void) {
  /*
	  This will set the peripheral bus clock to the same frequency
	  as the sysclock. That means 80 MHz, when the microcontroller
	  is running at 80 MHz. Changed 2017, as recommended by Axel.
	*/
	SYSKEY = 0xAA996655;  /* Unlock OSCCON, step 1 */
	SYSKEY = 0x556699AA;  /* Unlock OSCCON, step 2 */
	while(OSCCON & (1 << 21)); /* Wait until PBDIV ready */
	OSCCONCLR = 0x180000; /* clear PBDIV bit <0,1> */
	while(OSCCON & (1 << 21));  /* Wait until PBDIV ready */
	SYSKEY = 0x0;  /* Lock OSCCON */

	/* Set up output pins */
	AD1PCFG = 0xFFFF;
	ODCE = 0x0;
	TRISECLR = 0xFF;
	PORTE = 0x0;

	/* Output pins for display signals */
	PORTF = 0xFFFF;
	PORTG = (1 << 9);
	ODCF = 0x0;
	ODCG = 0x0;
	TRISFCLR = 0x70;
	TRISGCLR = 0x200;

	/* Set up input pins */
	TRISDSET = (1 << 8);
	TRISFSET = (1 << 1);

	/* Set up SPI as master */
	SPI2CON = 0;
	SPI2BRG = 4;
	/* SPI2STAT bit SPIROV = 0; */
	SPI2STATCLR = 0x40;
	/* SPI2CON bit CKP = 1; */
  SPI2CONSET = 0x40;
	/* SPI2CON bit MSTEN = 1; */
	SPI2CONSET = 0x20;
	/* SPI2CON bit ON = 1; */
	SPI2CONSET = 0x8000;

	display_init();
	//display_string(0, "KTH/ICT lab");
	//display_string(1, "in Computer");
	//display_string(2, "Engineering");
	//display_string(3, "Welcome!");
	display_update();

	//initializing stuff
	uint8_t screenUpd[512];
	int i;
	int snakeHead[2];
	int foodLoc[2];
	//x cord start in middle-ish
	snakeHead[0] = 50;
	//y cord start in middle-ish
	snakeHead[1] = 12;

	int snakeBodyLength = 0;
	int snakeBodyX[100];
	int snakeBodyY[100];

	//Rensar planen o ritar ramen
  for(i = 0; i < 512; i++){
			screenUpd[i] = screen[i];
	}

	/*
	snakeBodyLength = 1;
	snakeBodyX[0] = 49;
	snakeBodyY[0] = 12;
	*/


	//int snakeDirX = 1;
	//int snakeDirY = 0;
	int dir[1]; //start direction
	dir[0] = 1;

	//Ändrar vilken pixel som ska lysa
	/*
	int xVal = 60;
	int yVal = 5;
	//uppdaterar pixeln
	int id = slotToChange(xVal, yVal);
	screenUpd[id] = screenUpd[id] - powerOf(yVal);
	*/

	//draws snakesHead
	int id = slotToChange(snakeHead[0], snakeHead[1]);
	screenUpd[id] = screenUpd[id] - powerOf(snakeHead[1]);

	//id = slotToChange(49, 15);
	//screenUpd[id] = screenUpd[id] - powerOf(15);

	display_screen(0, screenUpd);

  //updatesPixels to contain body of snake and updates body cords
	/*for(i = snakeBodyLength; i >= 0; i--){
			id = slotToChange(snakeBodyX[i], snakeBodyY[i]);
			screenUpd[id] = screenUpd[id] - powerOf(snakeBodyY[i]);

			if(i > 0){
					//moves bodyVal back one step until part before head
					snakeBodyX[i] = snakeBodyX[i-1];
					snakeBodyY[i] = snakeBodyY[i-1];
			}
			else{
					//saves heads loc
					snakeBodyX[i] = snakeHead[0];
					snakeBodyY[i] = snakeHead[1];
			}
	}*/

	//ritar ut ny skärm

	//labinit(); /* Do any lab-specific initialization */


	/*//Rensar planen o ritar ramen
  for(i = 0; i < 512; i++){
			screenUpd[i] = screen[i];
	}
	//updaterar snakeHead cords 20 times
	for(i = 20; i > 0; i--){
			snakeHead[0] += snakeDirX;
			snakeHead[1] += snakeDirY;
	}
	//Lagrar nya snakeHead value
	id = slotToChange(snakeHead[0], snakeHead[1]);
	screenUpd[id] = screenUpd[id] - powerOf(snakeHead[1]);

	display_update();
	display_screen(0, screenUpd);
	*/

	labinit(); /* Do any lab-specific initialization */

	while( 1 )
	{
			snakeBodyLength = labwork(foodLoc, snakeHead, dir, snakeBodyX, snakeBodyY, snakeBodyLength);
			//dir = labwork(foodLoc, snakeHead, dir, snakeBodyX, snakeBodyY, snakeBodyLength);
	}

	return 0;
}
