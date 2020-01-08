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
#include <math.h>

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_UserApp1"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32UserApp1Flags;                       /* Global state flags */
volatile int G_password[11];                           /* correct password */


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
    *reset = 0; /* goes back to zero */
    for(int i = 0; password[i] != 0; i++) /* resets password array */
    {
      password[i] = 0;
    }
  }
} /* end of reset */

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
		/* set to setting password state */
    UserApp1_StateMachine = UserApp1SM_setPassword;
		
		/* change the message on the board */
		LCDCommand(LCD_CLEAR_CMD);
		static u8 au8Message[] = "SETTING A PASSWORD   "; 
		static u8 au8Message1[] = "1     2	    	3   SET"; 
		LCDMessage(LINE1_START_ADDR, au8Message);
		LCDMessage(LINE2_START_ADDR, au8Message1);
		
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

/* when trying to enter a password */
static void UserApp1SM_enterPassword(void)
{
	static int user_input[10]; /* stores the vaules of the buttons passed */
	static int point = 0; /* stores the location of the array */
  static int clockCounter = 0; /*counter for blinking */
	static int failures = 0; /* counts the amounts of failed attempts */
	
  if(point == -1) /* when user enters wrong password */
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
    reset(&point, user_input); /* checks for any button pressed to end cycle */
		
		/* when user has entered password wrong three times */
		if(failures == 3)
		{
			point = 0;
			failures = 2;
			
			LCDCommand(LCD_CLEAR_CMD);
			static u8 au8Message[] = "WRONG PASSWORD"; 
			static u8 au8Message1[] = "TRY AGAIN IN "; 
			LCDMessage(LINE1_START_ADDR, au8Message);
			LCDMessage(LINE2_START_ADDR, au8Message1);
			
			/* changes the state to locked state */
			UserApp1_StateMachine = UserApp1SM_lockedState;
		}
	}
	
  else if(point == -2) /* when user enters correct password */
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
    
		failures = 0; /* resets the fail counter */
    clockCounter++;
    reset(&point, user_input); /* checks for any button pressed to end cycle */
  }
	
  else /* user entering password */
  {
		LedOn(RED);
    LedOff(GREEN); 
   
    if(WasButtonPressed(BUTTON0) && !IsButtonPressed(BUTTON0))
    {
			ButtonAcknowledge(BUTTON0);
      user_input[point] = 1;
      point++;
		}   
   
    if(WasButtonPressed(BUTTON1) && !IsButtonPressed(BUTTON1))
    {
      ButtonAcknowledge(BUTTON1);
      user_input[point] = 2;
      point++;
    }
      
    if(WasButtonPressed(BUTTON2) && !IsButtonPressed(BUTTON2) )
    {
      ButtonAcknowledge(BUTTON2);
      user_input[point] = 3;
      point++;
    }
  
    if(WasButtonPressed(BUTTON3) && !IsButtonPressed(BUTTON3))
    {
			ButtonAcknowledge(BUTTON3);
      for(int i = 0; i  < 10; i++)
      {
				if(user_input[i] != G_password[i]) /* checks that password matches */
        {
					point = -1;
					failures++;					
					break;
				}
				point = -2;
				LedOff(RED);
			}
    }
  }
} /* end of enter */	

/* when trying to set a password */
static void UserApp1SM_setPassword(void)
{
	static int point = 0;
	
	if(WasButtonPressed(BUTTON0) && !IsButtonPressed(BUTTON0))
  {
		ButtonAcknowledge(BUTTON0);
		G_password[point] = 1;
    point++;
	}   
   
  if(WasButtonPressed(BUTTON1) && !IsButtonPressed(BUTTON1))
  {
    ButtonAcknowledge(BUTTON1);
    G_password[point] = 2;
    point++;
  }
      
  if(WasButtonPressed(BUTTON2) && !IsButtonPressed(BUTTON2) )
  {
    ButtonAcknowledge(BUTTON2);
    G_password[point] = 3;
    point++;
  }
  
  if(WasButtonPressed(BUTTON3) && !IsButtonPressed(BUTTON3))
  {
	  ButtonAcknowledge(BUTTON3);
	  point = 0;
		
		/* change the message on the board */
		LCDCommand(LCD_CLEAR_CMD);
		for(int i = 0; i < 1000; i++) {} //delay loop 
		static u8 au8Message[] = "ENTERING A PASSWORD   ";
		static u8 au8Message1[] = "1     2	    	3 ENTER"; 
		LCDMessage(LINE1_START_ADDR, au8Message);
		LCDMessage(LINE2_START_ADDR, au8Message1);
		
		/* change the state to entering password */
		UserApp1_StateMachine = UserApp1SM_enterPassword;	
  }
}  /* end of set */ 

/* stops the user from trying any more passwords */
static void UserApp1SM_lockedState(void)
{
	static int locked_state_attempts = 1;  /* tracks how many times locked state has been entered */
	static int clockCounter = 0; /*counter for blinking */
	int wait_time = (int) (-7.0833 * pow(locked_state_attempts,4) + 84.167 * pow(locked_state_attempts,3) - 322.92 * pow(locked_state_attempts,2) + 505.83 * locked_state_attempts - 250) * 1000;
  /* counts how long the system will remained in locked state */

	/* Switch the time left in locked state */
	if(clockCounter % 1000 == 0)
	{
		static u8 au8Message[] = "TRY AGAIN IN "; 
		LCDMessage(LINE2_START_ADDR, au8Message);
		
		int first_digit = ((wait_time - clockCounter) / 100000) + 48; //first digit in ascii
		int second_digit = ((wait_time - clockCounter) / 10000) + 48 - ((wait_time - clockCounter) / 100000) * 10;
		int third_digit = ((wait_time - clockCounter) / 1000) + 48 - ((wait_time - clockCounter) / 10000) * 10 - ((wait_time - clockCounter) / 100000) * 100;
		u8 au8Message1[] = {first_digit, second_digit, third_digit, 32, 32, 32 ,32 ,32 ,32}; /* 32 is blank space is ascii */
		LCDMessage(LINE2_START_ADDR + 13, au8Message1);
	}
	
	/* Switching from locked state to entering password */
	if(clockCounter == wait_time)
	{ 
		locked_state_attempts++; 
		clockCounter = 0;
		
		/* change the message on the board */
		LCDCommand(LCD_CLEAR_CMD);
		for(int i = 0; i < 1000; i++) {} //delay loop 
		static u8 au8Message[] = "ENTERING A PASSWORD   ";
		static u8 au8Message1[] = "1     2	    	3 ENTER"; 
		LCDMessage(LINE1_START_ADDR, au8Message);
		LCDMessage(LINE2_START_ADDR, au8Message1);
		
		/* change the state to entering password */
		UserApp1_StateMachine = UserApp1SM_enterPassword;	
	}
		
	if(clockCounter % 1000 == 0 & clockCounter <= 5000)
  {		
    LedOff(WHITE);
    LedOff(PURPLE);
    LedOff(BLUE);
    LedOff(CYAN);
    LedOff(GREEN);
    LedOff(YELLOW);
    LedOff(ORANGE);
    LedOff(RED);
		
		if(clockCounter == 5000)
		{
			LedOn(ORANGE);
		}
  }
	
  if(clockCounter % 500 == 0 & !(clockCounter % 1000 == 0) & clockCounter <= 5000)
  {
    LedOn(WHITE);
    LedOn(PURPLE);
    LedOn(BLUE);
    LedOn(CYAN);
    LedOn(GREEN);
    LedOn(YELLOW);
    LedOn(ORANGE);
    LedOn(RED);
  }
	
	clockCounter++;
}
	
	
/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserApp1SM_Error(void)          
{
  
} /* end UserApp1SM_Error() */



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
