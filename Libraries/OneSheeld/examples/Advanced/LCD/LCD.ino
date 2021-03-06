/*

LCD Shield Example

This example shows an application on 1Sheeld's LCD shield.

By using this example, you can print some text on the
LCD shield available in our app.

*/

/* Include 1Sheeld library. */ 
#include <OneSheeld.h>

/* A name for a button on pin 12. */
int buttonPin = 12;

void setup() 
{
  /* Start communication. */
  OneSheeld.begin();
  /* Set the button pin as input. */
  pinMode(buttonPin,INPUT);  
}

void loop()
{
  /* Always check the button pin if pressed. */
  if(digitalRead(buttonPin) == HIGH)
  {
   /* Begin the LCD cursor at the first row and first column. */
   LCD.begin(1,1);
   /* Blink the cursor and for 5 seconds. */
   LCD.blink();
   delay(5000);
   /* Stop blinking. */
   LCD.noBlink();
   /* Write some characters. */
   LCD.write('A');
   LCD.write('B');
   LCD.write('C');
   /* Wait for 2 seconds. */
   delay(2000);
   /* Clear the LCD. */
   LCD.clear();
   /* Print "Hello, World!" */
   LCD.print("Hello, World!");
   /* Set the cursor to begin writing from the second row and first column. */
   LCD.setCursor(2,1);
   /* Print "This is OneSheeld" */ 
   LCD.print("This is OneSheeld");
   /* Wait 10 seconds. */  
   delay(10000);
   /* Print "Closing!" */
   LCD.print("Closing!");
   /* Wait for 2 seconds. */
   delay(10000);
  }
  
  
   
}
