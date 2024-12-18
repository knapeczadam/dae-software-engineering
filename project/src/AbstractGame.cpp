//-----------------------------------------------------------------
// AbstractGame Object
// C++ Source - AbstractGame.cpp - version v8_01
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------
#include "GameEngine.h"		
#include "AbstractGame.h"

//-----------------------------------------------------------------
// AbstractGame Member Functions
//-----------------------------------------------------------------
void AbstractGame::Initialize()
{
	// Set required values
	GAME_ENGINE->SetTitle(_T("Game Engine version 8_01"));

	// Set optional values
	GAME_ENGINE->SetWidth(640);
	GAME_ENGINE->SetHeight(480);
	GAME_ENGINE->SetFrameRate(50);
}