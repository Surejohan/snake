/* mipslabfunc.c
   This file written 2015 by F Lundevall
   Some parts are original code written by Axel Isaksson
	 Edited by Tobias Johannesson

   For copyright and licensing, see file COPYING */

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "mipslab.h"  /* Declatations for these labs */

/* Declare a helper function which is local to this file */
static void num32asc( char * s, int );

#define DISPLAY_CHANGE_TO_COMMAND_MODE (PORTFCLR = 0x10)
#define DISPLAY_CHANGE_TO_DATA_MODE (PORTFSET = 0x10)

#define DISPLAY_ACTIVATE_RESET (PORTGCLR = 0x200)
#define DISPLAY_DO_NOT_RESET (PORTGSET = 0x200)

#define DISPLAY_ACTIVATE_VDD (PORTFCLR = 0x40)
#define DISPLAY_ACTIVATE_VBAT (PORTFCLR = 0x20)

#define DISPLAY_TURN_OFF_VDD (PORTFSET = 0x40)
#define DISPLAY_TURN_OFF_VBAT (PORTFSET = 0x20)

/* quicksleep:
   A simple function to create a small delay.
   Very inefficient use of computing resources,
   but very handy in some special cases. */
void quicksleep(int cyc) {
	int i;
	for(i = cyc; i > 0; i--);
}

/* tick:
   Add 1 to time in memory, at location pointed to by parameter.
   Time is stored as 4 pairs of 2 NBCD-digits.
   1st pair (most significant byte) counts days.
   2nd pair counts hours.
   3rd pair counts minutes.
   4th pair (least significant byte) counts seconds.
   In most labs, only the 3rd and 4th pairs are used. */
void tick( unsigned int * timep )
{
  /* Get current value, store locally */
  register unsigned int t = * timep;
  t += 1; /* Increment local copy */

  /* If result was not a valid BCD-coded time, adjust now */

  if( (t & 0x0000000f) >= 0x0000000a ) t += 0x00000006;
  if( (t & 0x000000f0) >= 0x00000060 ) t += 0x000000a0;
  /* Seconds are now OK */

  if( (t & 0x00000f00) >= 0x00000a00 ) t += 0x00000600;
  if( (t & 0x0000f000) >= 0x00006000 ) t += 0x0000a000;
  /* Minutes are now OK */

  if( (t & 0x000f0000) >= 0x000a0000 ) t += 0x00060000;
  if( (t & 0x00ff0000) >= 0x00240000 ) t += 0x00dc0000;
  /* Hours are now OK */

  if( (t & 0x0f000000) >= 0x0a000000 ) t += 0x06000000;
  if( (t & 0xf0000000) >= 0xa0000000 ) t = 0;
  /* Days are now OK */

  * timep = t; /* Store new value */
}

/* display_debug
   A function to help debugging.

   After calling display_debug,
   the two middle lines of the display show
   an address and its current contents.

   There's one parameter: the address to read and display.

   Note: When you use this function, you should comment out any
   repeated calls to display_image; display_image overwrites
   about half of the digits shown by display_debug.
*/
void display_debug( volatile int * const addr )
{
  display_string( 1, "Addr" );
  display_string( 2, "Data" );
  num32asc( &textbuffer[1][6], (int) addr );
  num32asc( &textbuffer[2][6], *addr );
  display_update();
}

uint8_t spi_send_recv(uint8_t data) {
	while(!(SPI2STAT & 0x08));
	SPI2BUF = data;
	while(!(SPI2STAT & 1));
	return SPI2BUF;
}

void display_init(void) {
        DISPLAY_CHANGE_TO_COMMAND_MODE;
	quicksleep(10);
	DISPLAY_ACTIVATE_VDD;
	quicksleep(1000000);

	spi_send_recv(0xAE);
	DISPLAY_ACTIVATE_RESET;
	quicksleep(10);
	DISPLAY_DO_NOT_RESET;
	quicksleep(10);

	spi_send_recv(0x8D);
	spi_send_recv(0x14);

	spi_send_recv(0xD9);
	spi_send_recv(0xF1);

	DISPLAY_ACTIVATE_VBAT;
	quicksleep(10000000);

	spi_send_recv(0xA1);
	spi_send_recv(0xC8);

	spi_send_recv(0xDA);
	spi_send_recv(0x20);

	spi_send_recv(0xAF);
}

void display_string(int line, char *s) {
	int i;
	if(line < 0 || line >= 4)
		return;
	if(!s)
		return;

	for(i = 0; i < 16; i++)
		if(*s) {
			textbuffer[line][i] = *s;
			s++;
		} else
			textbuffer[line][i] = ' ';
}


//displays the whole screens pixel values
void display_screen( int x, const uint8_t *data) {
	int i, j;//i changes row, j steps to the right

	for(i = 0; i < 4; i++) {
		DISPLAY_CHANGE_TO_COMMAND_MODE;

		spi_send_recv(0x22);
		spi_send_recv(i);

		spi_send_recv(x & 0xF);
		spi_send_recv(0x10 | ((x >> 4) & 0xF));

		DISPLAY_CHANGE_TO_DATA_MODE;

		//how big picture is
		int size = 128;
		for(j = 0; j < size; j++)
			spi_send_recv(~data[i*size + j]);
	}
}

//fixa fel vid mod 8 = 0
int powerOf(int val){
		/*//för rätt y t.ex. y = 1 ska ha värde 1 = 2 power 0
		//int exp = val - 1;
		exp = exp % 8;

		//detta för att det ska bli rätt y
		//fel//exp = exp - 1;
		int base = 2;

		int result = 1;
		while (exp)
		{
				if (exp & 1)
						result *= base;
				exp >>= 1;
				base *= base;
		}*/

		int exp = val;
		int result = 1;
		exp = exp % 8;

		for (; exp > 0; exp--) {
		    result *= 2;
		}

		return result;
}

int slotToChange(int x, int y){
		int id;

		//testar vilken rad den bör vara på
		if(y < 8)
			id = x;
		else if(y < 16)
			id = 128 * 1 + x;
		else if(y < 24)
			id = 128 * 2 + x;
		else
			id = 128 * 3 + x;

		//sub one since x is 1-128 and address is 0-127
		id--;
		return id;
}

void display_update(void) {
	int i, j, k;
	int c;
	for(i = 0; i < 4; i++) {
		DISPLAY_CHANGE_TO_COMMAND_MODE;
		spi_send_recv(0x22);
		spi_send_recv(i);

		spi_send_recv(0x0);
		spi_send_recv(0x10);

		DISPLAY_CHANGE_TO_DATA_MODE;

		for(j = 0; j < 16; j++) {
			c = textbuffer[i][j];
			if(c & 0x80)
				continue;

			for(k = 0; k < 8; k++)
				spi_send_recv(font[c*8 + k]);
		}
	}
}

/* Helper function, local to this file.
   Converts a number to hexadecimal ASCII digits. */
static void num32asc( char * s, int n )
{
  int i;
  for( i = 28; i >= 0; i -= 4 )
    *s++ = "0123456789ABCDEF"[ (n >> i) & 15 ];
}

#define ITOA_BUFSIZ ( 24 )
char * itoaconv( int num )
{
  register int i, sign;
  static char itoa_buffer[ ITOA_BUFSIZ ];
  static const char maxneg[] = "-2147483648";

  itoa_buffer[ ITOA_BUFSIZ - 1 ] = 0;   /* Insert the end-of-string marker. */
  sign = num;                           /* Save sign. */
  if( num < 0 && num - 1 > 0 )          /* Check for most negative integer */
  {
    for( i = 0; i < sizeof( maxneg ); i += 1 )
    itoa_buffer[ i + 1 ] = maxneg[ i ];
    i = 0;
  }
  else
  {
    if( num < 0 ) num = -num;           /* Make number positive. */
    i = ITOA_BUFSIZ - 2;                /* Location for first ASCII digit. */
    do {
      itoa_buffer[ i ] = num % 10 + '0';/* Insert next digit. */
      num = num / 10;                   /* Remove digit from number. */
      i -= 1;                           /* Move index to next empty position. */
    } while( num > 0 );
    if( sign < 0 )
    {
      itoa_buffer[ i ] = '-';
      i -= 1;
    }
  }
  /* Since the loop always sets the index i to the next empty position,
   * we must add 1 in order to return a pointer to the first occupied position. */
  return( &itoa_buffer[ i + 1 ] );
}

//io for btns
int getbtns(void)
{
		return ((PORTD & 0x00E0) >> 4); //0x00F0 >> 4???
}

int getbtn1(void)
{
		return ((PORTF & 0x02) >> 1);
}

//Copied from github change later
void labinit( void )
{
    volatile int *trise = (volatile int*)0xBF886100;  // Skapar en volatile som pekar på samma som TRISE
    *trise &= ~0xFF;                                  // Maskar och har kvar alla andra utom bitarna 0 -> 7 bitarna, samt inverterar allt i slutet. Lämnar dem andra som dem var sedan innan.

		TRISF |= 0x02;  //sets bit one to input
    TRISD |= 0x0E0;     // Sätter bitarna 4 -> 7 som input. Lämnar dem andra som dem var sedan innan.
		//kanske 0x0F0

    T2CON = 0x070;      // 1:256 == 80 000 000Hz / 256 = 312 500
                        // 312 500  .... 1 gånger per sekund
                        // 312500 = 4C4B4 i HEX
    TMR2 = 0x0;         // Clearar
    PR2 = 0x4C4B4;       // Så länge kommer den räkna, tills den uppnår det värdet, värdet vi räknat ut ovan
    // http://ww1.microchip.com/downloads/en/DeviceDoc/61143H.pdf Sida 90.
    IPCSET(2) = 0x0000001C;       // Sätter priority level enligt manual.
    IPCSET(2) = 0x00000003;       // Sätter priority level enligt manual.
    IFSCLR(0) = 0x00000100;       // Clearar timerns interupt flagga enligt manual.
    IECSET(0) = 0x00000100;       // Enablar timer interut enligt manual.

    T2CONSET = 0x8000;            // Startar timern
    return;
}

//Copied from github change later
int labwork(int* fL, int* sH, int* dir, int* sBX, int* sBY, int sBLength)
{
		//give it x and y value
				/* random int between 1 and 125 */
				fL[0] == /*rand()*/ 25 % 124 + 2;
				/* random int between 1 and 127 */
				fL[1] == /*rand()*/ 36 % 13 + 2;

					//remember dir[0]
		//tests if a btn is pressed and changes dir[0]
		if (getbtns)
		{
						//can't be left because dir[0] is always set to left at start
						if ((getbtns() & 0x08) == 8)                  // btn 4
						{
								if(dir[0] != 1)
										dir[0] = 0; //left
						}
						else if ((getbtns() & 0x04) == 4)             // btn 3
						{
								if(dir[0] != 0)
										dir[0] = 1; //right
						}
						else if ((getbtns() & 0x02) == 2)             // btn 2
						{
								if(dir[0] != 3)
						    		dir[0] = 2; //up
						}
						/*else{
								dir[0] = 4; //nothing
						}*/
						//problem, always here when no button pressed?!?!?!?
		}
		if(getbtn1){
				if ((getbtn1() & 0x01) == 0x1)   // btn 1
				{
						if(dir[0] != 2)
								dir[0] = 3; //down
				}
		}

		//init to be used in here
		int i;
		int id;
		uint8_t screenUpd[512];
		int hit = 0;
		int length = sBLength;

    volatile int* porte = (volatile int*)0xBF886110;      // Skapar en volatile som pekar på samma som TRISE
    static int count = 2;                                 // Skapar en count
    if (IFS(0) & 0x100)                                   // Om IFS(0)
    {
	      IFS(0) = 0;
				//change count to change update speed          // Så vi återställer den till 0
	      if (count == 2)
	      {
		        count = 0;                                 // Återställer timeoutcount
		        //display_string( 3, "Snake" );
		        //display_update();
						//Rensar planen o ritar ramen
					  for(i = 0; i < 512; i++){
								screenUpd[i] = screen[i];
						}

						//updateBody(sBX, sBY); Flytta nedan till funktionen
						for(i = sBLength - 1; i >= 0; i--){
								//uppdaterar alla delar utom den precis vid huvudet
								if(i > 0){
										sBX[i] = sBX[i-1];
										sBY[i] = sBY[i-1];

										//Lagrar nya snakeBody value
										id = slotToChange(sBX[i], sBY[i]);
										screenUpd[id] = screenUpd[id] - powerOf(sBY[i]);
								}
								//uppdaterar biten precis vid huvudet
								else{
										sBX[0] = sH[0];
										sBY[0] = sH[1]; //dunno why 1

										//Lagrar nya snakeBody value //kanske ett y värde ner fel
										id = slotToChange(sBX[0], sBY[0]);
										screenUpd[id] = screenUpd[id] - powerOf(sBY[0]);
								}
						}

						//updaterar snakeHead cords
						if(dir[0] == 0)
								sH[0] -= 1; //left
						else if(dir[0] == 1)
								sH[0] += 1; //right
						else if(dir[0] == 2)
								sH[1] -= 1;  //up
						else if(dir[0] == 3)
								sH[1] += 1;  //down

						//test head body col
						//if
						//new random value for food if hit
						if(sH[0] == fL[0] && sH[1] == fL[1]){
								hit = 1;
								length++;
						}
						//end of food hit check


						//Test head wall collision
						if(sH[0] < 2 | sH[0] > 127 |sH[1] < 1 | sH[1] > 31){
								display_string(2, "GameOver WallCol"); //Max length of string
								display_update();

						}
						else{
								//Lagrar nya snakeHead value
								id = slotToChange(sH[0], sH[1]);
								screenUpd[id] = screenUpd[id] - powerOf(sH[1]);

								//if(hit = 1)
								if(!hit){
										fL[0] = 60;
										fL[1] = 5;
										//Lagrar foodValue
										id = slotToChange(fL[0], fL[1]);
										screenUpd[id] = screenUpd[id] - powerOf(fL[1]);
								}

								display_update();
								display_screen(0, screenUpd);
				        // Sätter LEDsen som det värde counten motsvarar
								// Ökar LED counten med 1
						}
      }
		   count++;
			 *porte = count;                                      // Ökar timeoutcount med 1
   }

	 //return dir[0];
	 return length;
}
