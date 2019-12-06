//	----==== WIN32_MAIN.H ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		2
//	Date:			6/04
//	Description:	Sets up the Win32 environment and including the WinMain function and
//					Windows message handling
//	-------------------------------------------------------------------------------------

#ifndef WIN32_MAIN_H
#define WIN32_MAIN_H

/*-----------------
---- FUNCTIONS ----
-----------------*/

bool	handleMsg(void);
void	swapBuffers(void);
void	sendCloseMsg(void);
bool	initWindow(void);
void	killWindow(void);

#endif