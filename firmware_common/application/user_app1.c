/**********************************************************************************************************************
File: user_app1.c                                                                

----------------------------------------------------------------------------------------------

Description:
Code to enter and set a password. 

------------------------------------------------------------------------------------------------------------------------
API:

Public functions:


Protected System functions:
void UserApp1Initialize(void)
Runs required initialzation for the task.  Should only be called once in main init section.

void UserApp1RunActiveState(void)
Runs current task state.  Should only be called once in main loop.


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_UserApp1"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32UserApp1Flags;                       /* Global state flags */
volatile int G_UserApp1password[11];       /* Password to match where 0 is the end */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32ApplicationFlags;             /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "UserApp1_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type UserApp1_StateMachine;            /* The state machine function pointer */
//static u32 UserApp1_u32Timeout;                      /* Timeout counter used across states */


/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/

/* resets the password array for next attempt*/
void reset(int* reset, int password[])
{	
  if(WasButtonPressed(BUTTON0) || WasButtonPressed(BUTTON1) || WasButtonPressed(BUTTON2) || WasButtonPressed(BUTTON3) && !IsButtonPressed(BUTTON0) && !IsButtonPressed(BUTTON1) && !IsButtonPressed(BUTTON2) && !IsButtonPressed(BUTTON3) )
  {
    ButtonAcknowledge(BUTTON0);
    ButtonAcknowledge(BUTTON1);
    ButtonAcknowledge(BUTTON2);
    ButtonAcknowledge(BUTTON3);
    *reset = 0;
    for(int i = 0; password[i] != 0; i++)
    {
      password[i] = 0;
    }
  }
}

/* when trying to enter a password */
void enter(int *state)
{
	static int password[10]; //stores the vaules of the buttons passed 
	static int point = 0; //stores the location of the array 
  static int clockCounter = 0; //counter for blinking 
	
  LCDCommand(LCD_CLEAR_CMD);
	static u8 au8Message[] = "ENTERING A PASSWORD   ";
  LCDMessage(LINE1_START_ADDR, au8Message);
	
  if(point == -1)
  {
    if(clockCounter == 1000)
    {
      LedOn(RED);
      clockCounter = 0;
    }
    if(clockCounter == 500)
    {
      LedOff(RED);
    }
    clockCounter++;
    reset(&point, password);
  }
  else if(point == -2)
  {
    if(clockCounter == 1000)
    {
      LedOn(GREEN);
      clockCounter = 0;
    }
    if(clockCounter == 500)
    {
      LedOff(GREEN);
    }
    
    clockCounter++;
    LedOff(RED);
    reset(&point, password);
  }
  else
  {
		LedOn(RED);
    LedOff(GREEN); 
   
    if(WasButtonPressed(BUTTON0) && !IsButtonPressed(BUTTON0))
    {
			ButtonAcknowledge(BUTTON0);
      password[point] = 1;
      point++;
		}   
   
    if(WasButtonPressed(BUTTON1) && !IsButtonPressed(BUTTON1))
    {
      ButtonAcknowledge(BUTTON1);
      password[point] = 2;
      point++;
    }
      
    if(WasButtonPressed(BUTTON2) && !IsButtonPressed(BUTTON2) )
    {
      ButtonAcknowledge(BUTTON2);
      password[point] = 3;
      point++;
    }
  
    if(WasButtonPressed(BUTTON3) && !IsButtonPressed(BUTTON3))
    {
			ButtonAcknowledge(BUTTON3);
      for(int i = 0; i  < 10; i++)
      {
				if(password[i] != G_UserApp1password[i])
        {
					point = -1;  
					break;
				}
				point = -2;
			}
    }
  }
} /* end of enter */	

/* when trying to set a password */
void set(int* state)
{
  static u8 au8Message[] = "SETTING A PASSWORD     ";
	static int point = 0;
  
  LCDMessage(LINE1_START_ADDR, au8Message);
	
	if(WasButtonPressed(BUTTON0) && !IsButtonPressed(BUTTON0))
  {
		ButtonAcknowledge(BUTTON0);
		G_UserApp1password[point] = 1;
    point++;
	}   
   
  if(WasButtonPressed(BUTTON1) && !IsButtonPressed(BUTTON1))
  {
    ButtonAcknowledge(BUTTON1);
    G_UserApp1password[point] = 2;
    point++;
  }
      
  if(WasButtonPressed(BUTTON2) && !IsButtonPressed(BUTTON2) )
  {
    ButtonAcknowledge(BUTTON2);
    G_UserApp1password[point] = 3;
    point++;
  }
  
  if(WasButtonPressed(BUTTON3) && !IsButtonPressed(BUTTON3))
  {
	  ButtonAcknowledge(BUTTON3);
	  *state = 1;
	  point = 0;
  }
}
	
	
/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
Function: UserApp1Initialize

Description:
Initializes the State Machine and its variables.

Requires:
  -

Promises:
  - 
*/
void UserApp1Initialize(void)
{
 
  /* If good initialization, set state to Idle */
  if( 1 )
  {
    UserApp1_StateMachine = UserApp1SM_Idle;
    LedOff(WHITE);
    LedOff(PURPLE);
    LedOff(BLUE);
    LedOff(CYAN);
    LedOff(GREEN);
    LedOff(YELLOW);
    LedOff(ORANGE);
    LedOff(RED);
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    UserApp1_StateMachine = UserApp1SM_Error;
  }

} /* end UserApp1Initialize() */

  
/*----------------------------------------------------------------------------------------------------------------------
Function UserApp1RunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.
All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void UserApp1RunActiveState(void)
{
  UserApp1_StateMachine();

} /* end UserApp1RunActiveState */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/


/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------------------------*/
static void UserApp1SM_Idle(void)
{
	static int state = 0;  //keeps track of which state the board is in 
	
	if( state == 0 )
	{
		set(&state);
	}
	else
	{
		enter(&state);
	}

} /* end UserApp1SM_Idle() */
    

/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserApp1SM_Error(void)          
{
  
} /* end UserApp1SM_Error() */



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
