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
volatile int G_intUserApp1password[11];                /* Correct password */
volatile int G_intUserApp1sound; 											 /* Toggle for sound */ 
volatile int G_intUserApp1failures; 									 /* Counter for failed attempts */


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


/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/

/* Displays the LCD for sound toggle */
void display_sound(void)
{
	LCDCommand(LCD_CLEAR_CMD);
	static u8 au8Message[] = "TOGGLE SOUND?"; 
	static u8 au8Message1[] = "   2 YES       	4 NO"; 
	LCDMessage(LINE1_START_ADDR, au8Message);
	LCDMessage(LINE2_START_ADDR, au8Message1);
} /* end of display_sound */

/* Displays the LCD for setting password */
void display_setting(void)
{
	reset(G_intUserApp1password); /* resets the password */ 
	
	LCDCommand(LCD_CLEAR_CMD);
	static u8 au8Message[] = "SETTING A PASSWORD   "; 
	static u8 au8Message1[] = "1     2	    	3   SET"; 
	LCDMessage(LINE1_START_ADDR, au8Message);
	LCDMessage(LINE2_START_ADDR, au8Message1);
} /* end of display_setting */ 

/* Displays the LCD for entering password */
void display_entering(void)
{
	LCDCommand(LCD_CLEAR_CMD);
	static u8 au8Message[] = "ENTERING A PASSWORD   ";
	static u8 au8Message1[] = "1     2	    	3 ENTER"; 
	LCDMessage(LINE1_START_ADDR, au8Message);
	LCDMessage(LINE2_START_ADDR, au8Message1);
} /* end of display_entering */

/* Displays the LCD for right password */
void display_right(void)
{
	LCDCommand(LCD_CLEAR_CMD);
	static u8 au8Message[] = "CORRECT PASSWORD ";
	static u8 au8Message1[] = "2 CHANGE     4 LOCK"; 
	LCDMessage(LINE1_START_ADDR, au8Message);
	LCDMessage(LINE2_START_ADDR, au8Message1);
} /* end of display_right */

/* Displays the LCD for wrong password */
void display_wrong(void)
{
	LCDCommand(LCD_CLEAR_CMD);
	static u8 au8Message[] = "WRONG PASSWORD ";
	static u8 au8Message1[] = "PRESS ANY BUTTON"; 
	LCDMessage(LINE1_START_ADDR, au8Message);
	LCDMessage(LINE2_START_ADDR, au8Message1);
} /* end of display_wrong */

/* Displays the LCD for the locked state */
void display_locked(void)
{
	LCDCommand(LCD_CLEAR_CMD);
	static u8 au8Message[] = "TRY AGAIN IN "; 
	static u8 au8Message1[] = "LOCKED"; 
	LCDMessage(LINE1_START_ADDR, au8Message1);
	LCDMessage(LINE2_START_ADDR, au8Message);
} /* end of display_locked */

/* Changes the time left in locked state */
void change_time(int wait_time, int clockCounter)
{
	if(clockCounter % 1000 == 0) /* only change once every second to save memory */
	{
		int first_digit = ((wait_time - clockCounter) / 100000) + 48;
		int second_digit = ((wait_time - clockCounter) / 10000) + 48 - ((wait_time - clockCounter) / 100000) * 10;
		int third_digit = ((wait_time - clockCounter) / 1000) + 48 - ((wait_time - clockCounter) / 10000) * 10 ;
		u8 au8Message2[] = {first_digit, second_digit, third_digit, 32, 32, 32 ,32 ,32 ,32}; /* 32 is blank space is ascii */
		LCDMessage(LINE2_START_ADDR + 13, au8Message2);
	}
} /* end of change_time*/

/* Resets array */
void reset(volatile int array[])
{
	for(int i = 0; i < 10; i++)
	{
		array[i] = 0;
	}
} /* end of reset */

/* Checks for button pressed */
int button_pressed(void)
{
	if(WasButtonPressed(BUTTON0) || WasButtonPressed(BUTTON1) || WasButtonPressed(BUTTON2) || WasButtonPressed(BUTTON3) && !IsButtonPressed(BUTTON0) && !IsButtonPressed(BUTTON1) && !IsButtonPressed(BUTTON2) && !IsButtonPressed(BUTTON3) )
  {
		ButtonAcknowledge(BUTTON0);
    ButtonAcknowledge(BUTTON1);
    ButtonAcknowledge(BUTTON2);
    ButtonAcknowledge(BUTTON3);
		return 1;
	}
	return 0;
} /* end of button_pressed */

/* Plays click sound for button pressed */
void play_button(void)
{
	if( G_intUserApp1sound && ( IsButtonPressed(BUTTON0) || IsButtonPressed(BUTTON1) || IsButtonPressed(BUTTON2) ) || ( UserApp1_StateMachine == UserApp1SM_wrongPassword && IsButtonPressed(BUTTON3) ) ) 
	{
		PWMAudioSetFrequency(BUZZER1, 400); /* sound of a button */ 
		PWMAudioOn(BUZZER1);
	}
	else
	{
		PWMAudioOff(BUZZER1);
	}
} /* end of play_button */

/* Plays the enter sound */   
void play_enter(void)
{
	if( G_intUserApp1sound )
	{
		PWMAudioSetFrequency(BUZZER1, 500); /* sound of enter*/ 
		PWMAudioOn(BUZZER1);
		for(int i = 0; i < 1000000; i++) { } /* delay loop to allow sound to play but short enough to not notice */
		PWMAudioOff(BUZZER1);
	}
} /* end of play_enter */

/* Play correct sound for password some code refrenced from http://embeddedinembedded.com/?page_id=173 */
void play_correct(int time)
{
	if(G_intUserApp1sound)
	{
		static u16 au16NotesRight[] = {E4, E4, F4, G4, G4, F4, E4, D4, NO};
		static int rightNote = 0; /* tracks the current note being played */
		if(time % 250 == 0)
		{
			PWMAudioSetFrequency(BUZZER1, au16NotesRight[rightNote]);
      PWMAudioOn(BUZZER1);
			rightNote++;
    }
	}		
} /* end of play_correct */

/* Play wrong sound for password */ 
void play_wrong(int time)
{
	
} /* end of play_wrong */

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
		/* set to asking user for toggle sound */
		UserApp1_StateMachine = UserApp1SM_sound;
		display_sound();
		
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

/* Asks user if they want sound */ 
static void UserApp1SM_sound(void)
{
	/* checks for any button 1 pressed to return to toggle sound*/
  if(WasButtonPressed(BUTTON1) && !IsButtonPressed(BUTTON1))
  {
		ButtonAcknowledge(BUTTON1);
		G_intUserApp1sound = 1;
		
		/* changes the state to setting password */
		UserApp1_StateMachine = UserApp1SM_setPassword;
		display_setting();
	}
  
	/* checks for any button 1 pressed to return to toggle no sound*/
  if(WasButtonPressed(BUTTON3) && !IsButtonPressed(BUTTON3))
  {
		ButtonAcknowledge(BUTTON3);
		G_intUserApp1sound = 0;
		
		/* changes the state to setting password */
		UserApp1_StateMachine = UserApp1SM_setPassword;
		display_setting();
	}
} /* end of UserApp1SM_sound */

/* Entering password */
static void UserApp1SM_enterPassword(void)
{
	static int user_input[10]; /* stores the vaules of the buttons passed */
	static int point = 0; /* stores the location of the array */
	
	LedOn(RED);
  LedOff(GREEN);
	LedOff(ORANGE);
	play_button();
   
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
		play_enter();
		point = 0;
		int right = 1; /* tracks if password is correct */
		
		LedOff(GREEN);
		LedOff(RED);
		
    for(int i = 0; i < 10; i++)
    {
			if(user_input[i] != G_intUserApp1password[i]) /* checks that password matches */
      {
				right = 0;
			}
		}
		
		if(right)
		{
			UserApp1_StateMachine = UserApp1SM_rightPassword;
			display_right(); 
		}
		else
		{
			UserApp1_StateMachine = UserApp1SM_wrongPassword;
			display_wrong();
		}
		
		reset(user_input);
  }
} /* end of UserApp1SM_enterPassword */	

/* User entered wrong password */
static void UserApp1SM_wrongPassword(void)
{
	static int clockCounter = 0; /* counter for blinking */ 
	
	play_wrong(clockCounter);
	play_button(); 
	
	if(clockCounter % 1000 == 0)
  {
    LedOn(RED);
  }
  if(clockCounter % 500 == 0 && !(clockCounter % 1000 == 0) )
  {
    LedOff(RED);
  }
    
	clockCounter++;
	
	/* when user has entered password wrong three times */
	if(G_intUserApp1failures == 3)
	{
		G_intUserApp1failures = 2;
		
		/* changes the state to locked state */
		UserApp1_StateMachine = UserApp1SM_lockedState;
		display_locked();
	}
	
	/* checks for any button pressed to end cycle */
	if(button_pressed())
	{
		G_intUserApp1failures++;
		
		/* changes the state to entering password */
		UserApp1_StateMachine = UserApp1SM_enterPassword;
		display_entering();
	}
} /* end of UserApp1SM_wrongPassword */

/* User entered right password */
static void UserApp1SM_rightPassword(void)
{
	static int clockCounter = 0;
	G_intUserApp1failures = 0; /* resets failure counter */
   
	play_correct(clockCounter);
	
	if(clockCounter % 1000 == 0)
  {
    LedOn(GREEN);
  }
    
	if(clockCounter % 500 == 0 && !(clockCounter % 1000 == 0) )
  {
    LedOff(GREEN);
  }
    
  clockCounter++;
	
	/* checks for any button 1 pressed to return to set password*/
  if(WasButtonPressed(BUTTON1) && !IsButtonPressed(BUTTON1))
  {
		ButtonAcknowledge(BUTTON1);
		play_enter();
		
		/* changes the state to entering password */
		UserApp1_StateMachine = UserApp1SM_setPassword;
		display_setting();
	}
  
	/* checks for any button 3 pressed to return to enter password*/
  if(WasButtonPressed(BUTTON3) && !IsButtonPressed(BUTTON3))
  {
		ButtonAcknowledge(BUTTON3);
		play_enter();
		
		/* changes the state to entering password */
		UserApp1_StateMachine = UserApp1SM_enterPassword;
		display_entering();
	}
	
} /* end of UserApp1SM_rightPassword */

/* Setting password */
static void UserApp1SM_setPassword(void)
{
	static int point = 0;
	
	play_button();

	if(WasButtonPressed(BUTTON0) && !IsButtonPressed(BUTTON0))
  {
		ButtonAcknowledge(BUTTON0);
		G_intUserApp1password[point] = 1;
    point++;
	}   
   
  if(WasButtonPressed(BUTTON1) && !IsButtonPressed(BUTTON1))
  {
    ButtonAcknowledge(BUTTON1);
    G_intUserApp1password[point] = 2;
    point++;
  }
      
  if(WasButtonPressed(BUTTON2) && !IsButtonPressed(BUTTON2) )
  {
    ButtonAcknowledge(BUTTON2);
    G_intUserApp1password[point] = 3;
    point++;
  }
  
  if(WasButtonPressed(BUTTON3) && !IsButtonPressed(BUTTON3))
  {
	  ButtonAcknowledge(BUTTON3);
	  point = 0;
		play_enter();
		
		/* change the state to entering password */
		UserApp1_StateMachine = UserApp1SM_enterPassword;
		display_entering();
  }
}  /* end of UserApp1SM_setPassword */ 

/* Locked state */
static void UserApp1SM_lockedState(void)
{
	static int locked_state_attempts = 1;  /* tracks how many times locked state has been entered */
	static int clockCounter = 0; /*counter for blinking */
	int wait_time = (int) (-7.0833 * pow(locked_state_attempts,4) + 84.167 * pow(locked_state_attempts,3) - 322.92 * pow(locked_state_attempts,2) + 505.83 * locked_state_attempts - 250) * 1000;
  /* counts how long the system will remained in locked state */

	change_time(wait_time, clockCounter); /* Switch the time left in locked state */
	
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
	
	/*Disregard any button pressed in this state */
  ButtonAcknowledge(BUTTON0);
  ButtonAcknowledge(BUTTON1);
  ButtonAcknowledge(BUTTON2);
  ButtonAcknowledge(BUTTON3);
	
	/* Switching from locked state to entering password */
	if(clockCounter == wait_time)
	{ 
		locked_state_attempts++; 
		clockCounter = 0;
		
		/* change the state to entering password */
		UserApp1_StateMachine = UserApp1SM_enterPassword;	
		display_entering();
	}
} /* end of UserApp1SM_lockedState */
	
	
/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserApp1SM_Error(void)          
{
  
} /* end UserApp1SM_Error() */



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
