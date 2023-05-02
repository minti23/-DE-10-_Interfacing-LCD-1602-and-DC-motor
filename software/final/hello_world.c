#include <stdio.h>
#include <altera_avalon_pio_regs.h>
#include <alt_types.h>
#include <sys/alt_alarm.h>
#include <system.h>
#include <string.h>
#include <unistd.h>
/*#######################################################################
							//Mini Project//
- Build a system using Nios II in kit DE10 to connect a LCD 16x2 and an
H-bridge to control a motor. This system can do the following tasks:
  + When SW0 is ON, LCD blinks the sentence “Hello World !!!” in the
    Middle of row 1 with frequency 1Hz. (Using timer)
  + When SW1 is ON, Nios II controls the motor by sending PWM pulses to
    the H-bridge. LCD displays the duty cycle and the frequency of PWM
    pulses.
 + When SW0 and SW1 are OFF, turn off the system.
- EXTENSION:
  + SW1, SW2, SW3 will control the speed of the DC motor based on the PWM
    Pulses be created by DE-10 kit nano.
- FILE: miniProject
#######################################################################*/

/*###################################################
 	 	 	 	 LCD 1602
- Bits: 11
- Order: RS  RW  E  D7  D6  D5  D4  D3  D2  D1  D0
- 3 bit control (RS RW E):
	+ 001: send command
	+ 101: send data
	+ EN (1->0): data was sent to LCD
###################################################*/

/*------------------------------------------------/
 Name:				lcd_write
 Description: support lcd_cmd and lcd_data to
 	 	 	  send data to LCD controller
 ------------------------------------------------*/

void lcd_write(int data)
{
	IOWR_ALTERA_AVALON_PIO_DATA(LCD_BASE, data | 0b00100000000);	// write data and command
	myusleep();
	IOWR_ALTERA_AVALON_PIO_DATA(LCD_BASE, data & 0b11011111111);	// just set bit EN (1->0) to recognize data sent
	myusleep();
}

/*------------------------------------------------/
 Name:				lcd_cmd
 Description: send command to LCD controller
 ------------------------------------------------*/

void lcd_cmd(char cmd)
{
	lcd_write(0b00100000000 + cmd);
}

/*------------------------------------------------/
 Name:				lcd_data
 Description: send data to LCD controller
 ------------------------------------------------*/

void lcd_data(char data)
{
	lcd_write(0b10100000000 + data);
}

/*------------------------------------------------/
 Name:				lcd_init
 Description: Initialize LCD before showing text
 ------------------------------------------------*/

void lcd_init()
{
	lcd_write(0b00100111000);	// Set 2 line on LCD

	lcd_write(0b00100001100);	// Display On/Off control

	lcd_write(0b00100000110);   // Entry mode set

	lcd_write(0b00100000001);   // Clear screen
}

/*------------------------------------------------/
 Name:				lcd_printtext
 Description: print text or string on the screen
 ------------------------------------------------*/

void lcd_printtext(unsigned char string[])
{
	for (int i = 0; i < strlen(string); i++)
		lcd_data(string[i]);
}

/*------------------------------------------------/
 Name:				lcd_setcursor
 Description: set cursor position on display
 ------------------------------------------------*/

void lcd_setcursor(char row, char col)
{
	int row_char = 0;
	if (row == 1) row_char = 64;
	lcd_cmd(0b00010000000 + row_char + col);
}

/*------------------------------------------------/
 Name:				variables
 Description: declare variables for using timer
 ------------------------------------------------*/
unsigned long LCD_state=1, LCD_mark;
unsigned long HIGH, LOW, wait_time, wait;
unsigned long now, PWM_mark, PWM_state;
unsigned long DC;

/*------------------------------------------------/
 Name:				string char
 Description: declare strings printing on LCD
 ------------------------------------------------*/

unsigned char hello[]  = "Hello World !!!";
unsigned char empty[] = "                ";
unsigned char paraPWM[] = "f: 1KHz DC:    %";

/*------------------------------------------------/
 Name:				myusleep
 Description: set delay time without affecting
 			  other operations
 ------------------------------------------------*/

void myusleep()
{
	wait = alt_timestamp();
	while (alt_timestamp() - wait < 5000) create_PWM();
}

/*###################################################
 	 	 	 	 PWM
###################################################*/

/*------------------------------------------------/
 Name:				update_PWM
 Description: Update on time and off time for PWM
 	 	 	  pulse depending on duty cycle change
 ------------------------------------------------*/

void update_PWM()
{
/* Using DE-10 kit with frequency 50MHz --> 1 clock cycle corresponds 20 nanosecond
 * So, when applying required frequency 1kHz --> 1 required clock cycle corresponds 1 millisecond
 * --> 1 millisecond of 1 required clock cycle corresponds 50000 clock cycle on DE-10 kit
 * --> From above, the number of clock cycle using for duty cycle be determined. */
	HIGH = 50000*DC/100;
	LOW = 50000 - HIGH;
}

/*------------------------------------------------/
 Name:				pwm_init
 Description: Initialize the default values and set
 	 	 	  the time start & time control for PWM
 ------------------------------------------------*/

void pwm_init()
{
	DC = 50;
	update_PWM();
	PWM_state = 0;
	wait_time = LOW;
	alt_timestamp_start();
	PWM_mark = alt_timestamp();
	LCD_mark = alt_timestamp();
}

/*------------------------------------------------/
 Name:				create_PWM
 Description: create PWM pulse to L298_H_bridge
 ------------------------------------------------*/

void create_PWM()
{
	  now = alt_timestamp();
	  if (now - PWM_mark  >= wait_time)
	  {
		  PWM_state = !PWM_state;

	  if (PWM_state == 0) wait_time = LOW;
	  else                wait_time = HIGH;

		  PWM_mark = alt_timestamp();
	  }
}

/*------------------------------------------------/
 Name:				displacy_PWM
 Description: display frequency and duty cycle
 	 	 	  controlling the motor on LCD
 ------------------------------------------------*/

void display_PWM()
{
	lcd_setcursor(1,0);
	lcd_printtext(paraPWM);
	lcd_setcursor(1,12);

	unsigned long num = DC;

	unsigned long a = num/100;			// Split to find and print hundreds
		if (a==0) lcd_printtext(" ");
		else lcd_data(a + 0x30);
		lcd_setcursor(1,13);
		num = num - a*100;				    // Update number to find and print next
		a = num/10;							// Split to find and print tens
		lcd_data(a+0x30);
		num = num - a*10;					// Update number to find and print next
		a = num/1;							// Split to find and print units
		lcd_data(a+0x30);
	}

	/*------------------------------------------------/
	 Name:				MAIN PROGRAM
	 ------------------------------------------------*/

	int main()
	{
		  pwm_init();
		  lcd_init();

		  while(1){

	/*----------------------------------------------------------------------------------------------/
	Operation: 						  			SWITCH 0 IS ON
	Description: LCD blinks the sentence “Hello World !!!” in the middle of row 1 with frequency 1Hz
	----------------------------------------------------------------------------------------------*/
		  now = alt_timestamp();
		  if ((IORD_ALTERA_AVALON_PIO_DATA(SWITCH_BASE)&1)== 0X01)
		  {
			  lcd_setcursor(0,1);
			  lcd_printtext(hello);		// Print "Hello World!!!"

			  /*-------------------------------------------------*/
				if (now - LCD_mark >= 25000000) {
			    	if (LCD_state == 0) {
			    		lcd_setcursor(0,1);
			    		lcd_printtext(empty);
			    	} else {
			    		lcd_setcursor(0,1);
			    		lcd_printtext(hello);
			    	}
					LCD_state = !LCD_state;
					LCD_mark = alt_timestamp(); // Save current timestamp right after toggling the state.
		  }
		  else
		  {
			  LCD_state = 1;
			  lcd_setcursor(0,1);
			  lcd_printtext(empty);	    // Clear 1st line if SW0 is OFF
		  }
		  }

	/*----------------------------------------------------------------------------------------------/
	Operation: 						SWITCH 1 OR SWITCH 2 OR SWITCH 3 IS ON
	Description: Create PWM pulses to control DC motor based on duty cycle with  100%, 50% and 25%
	----------------------------------------------------------------------------------------------*/

		  if (((IORD_ALTERA_AVALON_PIO_DATA(SWITCH_BASE) >> 1) & 1) == 1)
		  {
			  DC = 50;
			  update_PWM();
			  create_PWM();
			  IOWR_ALTERA_AVALON_PIO_DATA(MOTOR_BASE, PWM_state);
			  display_PWM();
		  }
		  else if (((IORD_ALTERA_AVALON_PIO_DATA(SWITCH_BASE) >> 2) & 1) == 1)
		  {
			  DC = 100;
			  update_PWM();
			  create_PWM();
			  IOWR_ALTERA_AVALON_PIO_DATA(MOTOR_BASE, PWM_state);
			  display_PWM();
		  }
		  else if (((IORD_ALTERA_AVALON_PIO_DATA(SWITCH_BASE) >> 3) & 1) == 1)
		  {
			  DC = 25;
			  update_PWM();
			  create_PWM();
			  IOWR_ALTERA_AVALON_PIO_DATA(MOTOR_BASE, PWM_state);
			  display_PWM();
		  }
		  else
		  {
			  IOWR_ALTERA_AVALON_PIO_DATA(MOTOR_BASE, 0);
			  lcd_setcursor(1,0);
			  lcd_printtext(empty);		// Clear 1st line if SW(1||2||3) is OFF
		  }
	  }
	}


