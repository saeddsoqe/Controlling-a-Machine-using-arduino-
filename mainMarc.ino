/*****************************************************************************************/
/*AUTHOR : SAEED DESOUKY                                                                 */
/*PROGRAM: CONTROLLING A MACHINE                                                         */
/*VERSION: V01                                                                           */
/*****************************************************************************************/



/*****************************************************************************************/
/*                            INCLUDES                                                   */
/*****************************************************************************************/

#include <Keypad.h>
#include <SoftwareSerial.h>

/*****************************************************************************************/
/*                            DEFINE PINS                                                */
/*****************************************************************************************/

// Pulse Pin
const int stepPin = 13;
//Direction Pin
const int dirPin  = 12;
//Limit switch 1
const int limit_switch1_pin = A0;
//Push Button  1
const int switch1_pin       = A1;
//Limit switch 2
const int limit_switch2_pin = A2;
//Push Button  2
const int switch2_pin       = A3;

/*****************************************************************************************/
/*                        VARIABLES DEFINETIONS                                          */
/*****************************************************************************************/

// change this to fit the number of steps per revolution to make 360 degree
const int Steps_Per_Revolution = 200;

const byte ROWS = 4;
const byte COLS = 4;
//those variables for keypad
char Key;
char Current_Reading_For_Keypad_Choosed_Branch = 0;
int Get_Number_Of_Rotations;
// counters to calculate the accumlation of the branch
int G_counter_For_Total_Number_Of_Rotations_For_Right_Branch = 0;
int G_counter_For_Total_Number_Of_Rotations_For_Left_Branch  = 0;
int Number_Of_Rotations;
int timer1_counter;

boolean Is_Right_Choosed = false;
boolean Is_Left_Choosed  = false;

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};


byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

/*****************************************************************************************/
/*                          INTERNAL DEFINES                                             */
/*****************************************************************************************/

#define  pressed        0
#define  not_pressed    1
#define  ON             1
#define  OFF            0
// two limit switchs
#define  limit_switch1  1
#define  limit_switch2  2
// two Push Buttons
#define  switch1        1
#define  switch2        2
//please note those two control the pulse width generated smaller value you give high spead you get |----|___
#define  Pulse_ON       100 // |--|
#define  Pulse_OFF      50  // ___

/*****************************************************************************************/
void setup()
{
  //those pins for the stepper
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin , OUTPUT);
  // set the pins for switches
  pinMode(limit_switch1_pin, INPUT);
  pinMode(switch1_pin, INPUT_PULLUP); // using the internal pull up resistor for ON/OFF switch1
  pinMode(limit_switch2_pin, INPUT);
  pinMode(switch2_pin, INPUT_PULLUP); // using the internal pull up resistor for ON/OFF switch2

}


void loop()
{
  // this variable to calculate number of rotations
  //int Number_Of_Rotations;

  // select desired branch and geting number of rotatioons from the keypad
  Key = customKeypad.getKey();
  if (Key != NO_KEY)
  {
    Detect_Pressed_Buttons();
  }

  //to know which branch is choosed
  if (Current_Reading_For_Keypad_Choosed_Branch == '#') //the selected branch is the right branch
  {
    //set the value of the right branch to true
    Is_Right_Choosed = true;
    Is_Left_Choosed  = false;
  }
  else if (Current_Reading_For_Keypad_Choosed_Branch == '*') // the selected branch is the left branch
  {

    Is_Right_Choosed = false;
    //set the value of the Left branch to true
    Is_Left_Choosed  = true;
  }
  
  /*****************************************************************************************/
  /*      For Right Branch (MicroSwitch1 and its related ON/OFF Switch1                    */
  /*****************************************************************************************/

  //check if the related ON/OFF switch1 is ON(PRESSED) it will execute the rotations after the limit switch1 is PRESSED
  //if ON/OFF switch1 is OFF (NOT_PRESSED). if there is already number of rotations it will back to position zero if there is no number rotations it will hold on
  if (Is_Switch1_Pressed() == true)
  {
    //check if alread the right branch is choosed
    if (Is_Right_Choosed == true)
    {
      // check is there already selected number of rotations
      if (Get_Number_Of_Rotations != 0)
      {
        //check if the microswitch1 is pressed
        //if it is ON(PRESSED) it will trigger the motor to start rotations
        // if it is OFF (NOT_PRESSED) do nothing
        if (Is_Limit_Switch1_Pressed() == true)
        {
          // calculate the total number of the steps that the motor will take
          Number_Of_Rotations = Steps_Per_Revolution * Get_Number_Of_Rotations ;
          // start rotating
          for (int i = 0 ; i < Number_Of_Rotations ; i++)
          {
            // To rotate clock wise to move forwared from position Zero
            digitalWrite(dirPin , HIGH);
            digitalWrite(stepPin, HIGH);
            delayMicroseconds(Pulse_ON);
            digitalWrite(stepPin, LOW);
            delayMicroseconds(Pulse_OFF);
          }
          // collect the accumlated pressed
          G_counter_For_Total_Number_Of_Rotations_For_Right_Branch += Get_Number_Of_Rotations;
        }
        else // if the microswitch1  is OFF (NOT_PRESSED) do nothing
        {
          //do nothing
        }
      }
      else
      {
        //do nothing
      }
    }
  }
  else //if ON/OFF switch1 is OFF (NOT_PRESSED). if there is already number of rotations it will back to position zero if there is no number rotations it will hold on
  {
    // this for loop get the motor back as the acumlated pressed
    for (int i = 0 ; i < G_counter_For_Total_Number_Of_Rotations_For_Right_Branch ; i++)
    {
      // back to position zero
      for (int j = 0 ; j < Steps_Per_Revolution ; j++)
      {
        // To rotate counter clock wise to back to position zero
        digitalWrite(dirPin , LOW);
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(Pulse_ON);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(Pulse_OFF);
      }
    }
    //reset the accumlated rotations after exeuting all of it
    G_counter_For_Total_Number_Of_Rotations_For_Right_Branch = 0 ;
  }

  /*****************************************************************************************/
  /*      For Left Branch (MicroSwitch2 and its related ON/OFF Switch2                     */
  /*****************************************************************************************/

  //check if the related ON/OFF switch1 is ON(PRESSED) it will execute the rotations after the limit switch1 is PRESSED
  //if ON/OFF switch1 is OFF (NOT_PRESSED). if there is already number of rotations it will back to position zero if there is no number rotations it will hold on
  if (Is_Switch2_Pressed() == true)
  {
    //check if alread the Left branch is choosed
    if (Is_Left_Choosed == true)
    {
      // check is there already selected number of rotations
      if (Get_Number_Of_Rotations != 0)
      {
        //check if the microswitch2 is pressed
        //if it is ON(PRESSED) it will trigger the motor to start rotations
        // if it is OFF (NOT_PRESSED) do nothing
        if (Is_Limit_Switch2_Pressed() == true)
        {
          // calculate the total number of the steps that the motor will take
          Number_Of_Rotations = Steps_Per_Revolution * Get_Number_Of_Rotations * 10;
          // start rotating
          for (int i = 0 ; i < Number_Of_Rotations ; i++)
          {
            // To rotate clock wise to move forwared from position Zero
            digitalWrite(dirPin , HIGH);
            digitalWrite(stepPin, HIGH);
            delayMicroseconds(Pulse_ON);
            digitalWrite(stepPin, LOW);
            delayMicroseconds(Pulse_OFF);
          }
          // collect the accumlated pressed
          G_counter_For_Total_Number_Of_Rotations_For_Left_Branch += Get_Number_Of_Rotations;
        }
        else // if the microswitch2  is OFF (NOT_PRESSED) do nothing
        {
          //do nothing
        }
      }
      else
      {
        //do nothing
      }
    }
  }
  else //if ON/OFF switch2 is OFF (NOT_PRESSED). if there is already number of rotations it will back to position zero if there is no number rotations it will hold on
  {
    // this for loop get the motor back as the acumlated pressed
    for (int i = 0 ; i < G_counter_For_Total_Number_Of_Rotations_For_Left_Branch ; i++)
    {
      // back to position zero
      for (int j = 0 ; j < Steps_Per_Revolution ; j++)
      {
        // To rotate counter clock wise to back to position zero
        digitalWrite(dirPin , LOW);
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(Pulse_ON);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(Pulse_OFF);
      }
    }
    //reset the accumlated rotations after exeuting all of it
    G_counter_For_Total_Number_Of_Rotations_For_Left_Branch = 0 ;
  }
}


/*****************************************************************************************/
/*NOTES    : this function gets the pressed key from the Keypad                          */
/*INPUTS   : No INPUTS                                                                   */
/*OUTPUTS  : Update the global variable Get_Number_Of_Rotations                          */
/*RETURNES : No RETUENS                                                                  */
/*****************************************************************************************/

void Detect_Pressed_Buttons(void)
{
  switch (Key)
  {
    //If Button 0 is pressed
    case '0':
      Get_Number_Of_Rotations = 0;
      break;
    //If Button 1 is pressed
    case '1':
      Get_Number_Of_Rotations = 1;
      break;
    //If Button 2 is pressed
    case '2':
      Get_Number_Of_Rotations = 2;
      break;
    //If Button 3 is pressed
    case '3':
      Get_Number_Of_Rotations = 3;
      break;
    //If Button 4 is pressed
    case '4':
      Get_Number_Of_Rotations = 4;
      break;
    //If Button 5 is pressed
    case '5':
      Get_Number_Of_Rotations = 5;
      break;
    case '*':
      Current_Reading_For_Keypad_Choosed_Branch = '*';
      break;
    case '#':
      Current_Reading_For_Keypad_Choosed_Branch = '#';
      break;

  }
}

/*****************************************************************************************/
/*                            FUNCTIONS                                                  */
/*****************************************************************************************/

/*****************************************************************************************/
/*NOTES    : this function get the states of the microswitche1 in the Right branch       */
/*INPUTS   : No INPUTS                                                                   */
/*RETURNES : True or False                                                               */
/*****************************************************************************************/

boolean Is_Limit_Switch1_Pressed (void)
{
  if (digitalRead(limit_switch1_pin) == pressed)
  {
    return true ;
  }
  else
  {
    return false ;
  }

}

/*****************************************************************************************/
/*NOTES    : this function get the states of the microswitche1 in the Left branch        */
/*INPUTS   : No INPUTS                                                                   */
/*RETURNES : True or False                                                               */
/*****************************************************************************************/

boolean Is_Limit_Switch2_Pressed (void)
{
  if (digitalRead(limit_switch2_pin) == pressed)
  {
    return true ;
  }
  else
  {
    return false ;
  }

}

/*****************************************************************************************/
/*NOTES    : this function get the states of the ON/OFF switch1 in the Right branch      */
/*INPUTS   : No INPUTS                                                                   */
/*RETURNES : True or False                                                               */
/*****************************************************************************************/

boolean Is_Switch1_Pressed (void)
{
  if (digitalRead(switch1_pin) == pressed)
  {
    return true ;
  }
  else
  {
    return false ;
  }

}

/*****************************************************************************************/
/*NOTES    : this function get the states of the ON/OFF switch2 in the Left branch       */
/*INPUTS   : No INPUTS                                                                   */
/*RETURNES : True or False                                                               */
/*****************************************************************************************/

boolean Is_Switch2_Pressed (void)
{
  if (digitalRead(switch2_pin) == pressed)
  {
    return true ;
  }
  else
  {
    return false ;
  }

}
