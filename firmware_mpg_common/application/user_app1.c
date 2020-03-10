/**********************************************************************************************************************
File: user_app1.c                                                                

--------------------------------------------------------------------------------------------------------------------

Description:

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
volatile int G_intTotalMoney = 3000;	 								 /* Total money of the play */


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

/* Displays the LCD for the start screen */
void display_start(void)
{
	LCDCommand(LCD_CLEAR_CMD);
	static u8 au8Message[] = "    !!BLACKJACK!!  "; 
	static u8 au8Message1[] = "  PRESS ANY BUTTON "; 
	LCDMessage(LINE1_START_ADDR, au8Message);
	LCDMessage(LINE2_START_ADDR, au8Message1);
} /* end of display_setting */

/* Displays the LCD for the game selection screen */
void display_SelectionScreen(void)
{
	LedOff(WHITE);
  LedOff(PURPLE);
	LedOff(BLUE);
  LedOff(CYAN);
  LedOff(GREEN);
  LedOff(YELLOW);
  LedOff(ORANGE);
  LedOff(RED);
	
	LCDCommand(LCD_CLEAR_CMD);
	static u8 au8Message[] = "  WHICH GAME MODE?"; 
	static u8 au8Message1[] = "1 PLAYER 	3 COMPUTER"; 
	LCDMessage(LINE1_START_ADDR, au8Message);
	LCDMessage(LINE2_START_ADDR, au8Message1);
}/* end of displaySelectionScreen() */

/* Displays the LCD for the Player VS Dealer screen */
void display_PlayerVSDealer_PlaceBets(void)
{
	LCDCommand(LCD_CLEAR_CMD);
	int firstDigit = G_intTotalMoney / 10000 + 48;
	int secondDigit = G_intTotalMoney /1000 + 48 - (firstDigit - 48) * 10;
	int thirdDigit = G_intTotalMoney /100 + 48 - (secondDigit - 48) * 10 - (firstDigit - 48) * 100;
	if(firstDigit == 48) // sets to blank space if empty 
	{
		firstDigit = 32;
	}
	if(secondDigit == 48) // sets to blank space if empty 
	{
		secondDigit = 32;
	}
	
  u8 au8Message[] = {'B','E','T',32, 32, 32, 32, 32, 32, 32, 'L','E','F','T',32, firstDigit, secondDigit, thirdDigit, 48, 48}; 
	static u8 au8Message1[] = "1 UP  2 DOWN 3 DEAL"; 
	LCDMessage(LINE1_START_ADDR, au8Message);
	LCDMessage(LINE2_START_ADDR, au8Message1);
}/* end of display_PlayerVSDealer_PlaceBets() */

/* Displays the LCD for playing BlackJack */
void display_PlayScreen(void){
	
}

/* Displays the current amount bet and how much the player has left */
void display_ChangeBet(int bet)
{
	int firstDigit = bet / 10000 + 48;
	int secondDigit = bet /1000 + 48 - (firstDigit - 48) * 10;
	int thirdDigit = bet /100 + 48 - (secondDigit - 48) * 10 - (firstDigit - 48) * 100;
	if(firstDigit == 48) // sets to blank space if empty 
	{
		firstDigit = 32;
	}
	if(secondDigit == 48) // sets to blank space if empty 
	{
		secondDigit = 32;
	}
	
	u8 au8Message1[] = {firstDigit, secondDigit, thirdDigit, 48, 48};
	LCDMessage(LINE1_START_ADDR + 4, au8Message1);
} /* end of display_ChangeBetAndMoney() */
		
/* Displays the LCD for the Computer Vs Dealer screen */
void display_ComputerVSDealer(void)
{
	
}/* end of display_ComputerVSDealer */

/* Flashes the LCD across the screen */
void coolLCD(void)
{
	static u32 lcdPoint	= WHITE;
	static int clockCounter = 0; /*counter for blinking */
	
	if(clockCounter % 250 == 0) 
	{
		LedOff(lcdPoint % 8);
		LedOff( (lcdPoint + 4) % 8);
		lcdPoint++;
		LedOn(lcdPoint % 8);
		LedOn( (lcdPoint + 4) % 8);
	}
	
	clockCounter++;
}/* end of coolLCD */

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
 
  /* If good initialization, set state to StartScreen  */
  if( 1 )
  {
    UserApp1_StateMachine = UserApp1SM_StartScreen;   
		display_start();
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
/* Displays the start screen and waits for user input to start */
static void UserApp1SM_StartScreen(void)   
{
	coolLCD();
	if(button_pressed())
	{
		UserApp1_StateMachine = UserApp1_SelectionScreen;
		display_SelectionScreen();
	}
} /* end UserApp1SM_Idle() */

/*-------------------------------------------------------------------------------------------------------------------*/
/* Displays the selction screen for which mode the user wishes to play */
static void UserApp1_SelectionScreen(void)
{
	/* checks for any button 0 pressed to play human vs dealer*/
  if(WasButtonPressed(BUTTON0) && !IsButtonPressed(BUTTON0))
  {
		ButtonAcknowledge(BUTTON0);
		
		/* changes the state to PlayerVSDealer */
		UserApp1_StateMachine = UserApp1SM_PlayerVSDealer_PlaceBets;
		display_PlayerVSDealer_PlaceBets();
	}
  
	/* checks for any button 2 pressed to play computer vs dealer */ 
  if(WasButtonPressed(BUTTON2) && !IsButtonPressed(BUTTON2))
  {
		ButtonAcknowledge(BUTTON2);
		
		/* changes the state to ComputerVSDealer */
		UserApp1_StateMachine = UserApp1SM_ComputerVSDealer;
		display_ComputerVSDealer();
	}
} /* end UserApp1_SelectionScreen() */

/*-------------------------------------------------------------------------------------------------------------------*/
/* Allows the player to bet */
static void UserApp1SM_PlayerVSDealer_PlaceBets(void)
{
	static int bet = 0;
	
	// Increase the bet by 100
	if(WasButtonPressed(BUTTON0) && !IsButtonPressed(BUTTON0))
	{
		ButtonAcknowledge(BUTTON0);
		if( bet != G_intTotalMoney )
		{
			bet += 100;
			display_ChangeBet(bet);
		}
	}
	
	// Decrease the bet by 100
	if(WasButtonPressed(BUTTON1) && !IsButtonPressed(BUTTON1))
	{
		ButtonAcknowledge(BUTTON1);
		if( bet != 0 )
		{
			bet -= 100;
			display_ChangeBet(bet);
		}
	}
	
	if(WasButtonPressed(BUTTON2) && !IsButtonPressed(BUTTON2))
	{
		ButtonAcknowledge(BUTTON2);
		UserApp1_StateMachine = UserApp1SM_PlayerVSDealer_PlayGame(bet);
		display_PlayScreen();
	}
} /* end UserApp1SM_PlayerVSDealer_PlaceBets() */

/*-------------------------------------------------------------------------------------------------------------------*/
/* Plays the PlayerVSDealer */
static void UserApp1SM_PlayerVSDealer_PlayGame(void)
{
	
} /* end UserApp1SM_PlayerVSDealer_PlayGame() */

/*-------------------------------------------------------------------------------------------------------------------*/
/* Plays the Computer vs dealer*/
static void UserApp1SM_ComputerVSDealer(void){
	
} /* end UserApp1SM_ComputerVSDealer() */

/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserApp1SM_Error(void)          
{
  
} /* end UserApp1SM_Error() */



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
