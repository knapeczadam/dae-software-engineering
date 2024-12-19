//-----------------------------------------------------------------
// Main Game File
// C++ Source - Game.cpp - version v8_01
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------
#include "Game.h"
#include "MyLua.h"

//-----------------------------------------------------------------
// Game Member Functions																				
//-----------------------------------------------------------------



Game::Game()
{
}

Game::~Game()																						
{
	// nothing to destroy
}

void Game::Initialize()			
{
	// Code that needs to execute (once) at the start of the game, before the game window is created

	AbstractGame::Initialize();

	auto luaInitialize = GetLua()["initialize"];
	if (luaInitialize.valid())
	{
		luaInitialize();
	}
	else
	{
		GAME_ENGINE->MessageBox(_T("Initialize function not found."));
	}

	// Set the keys that the game needs to listen to
	//tstringstream buffer;
	//buffer << _T("KLMO");
	//buffer << (char) VK_LEFT;
	//buffer << (char) VK_RIGHT;
	//GAME_ENGINE->SetKeyList(buffer.str());
}

void Game::Start()
{
	// Insert code that needs to execute (once) at the start of the game, after the game window is created
	auto luaStart = GetLua()["start"];
	if (luaStart.valid())
	{
		luaStart();
	}
	else
	{
		GAME_ENGINE->MessageBox(_T("Start function not found."));
	}
}

void Game::End()
{
	// Insert code that needs to execute when the game ends
	auto luaEnd = GetLua()["end_"];
	if (luaEnd.valid())
	{
		luaEnd();
	}
	else
	{
		GAME_ENGINE->MessageBox(_T("End function not found."));
	}
}

void Game::Paint(RECT rect) const
{
	// Insert paint code
	auto luaPaint = GetLua()["paint"];
	if (luaPaint.valid())
	{
		luaPaint(rect);
	}
	else
	{
		GAME_ENGINE->MessageBox(_T("Paint function not found."));
	}
}

void Game::Tick()
{
	// Insert non-paint code that needs to execute each tick
	auto luaTick = GetLua()["tick"];
	if (luaTick.valid())
	{
		luaTick();
	}
	else
	{
		GAME_ENGINE->MessageBox(_T("Tick function not found."));
	}
}

void Game::MouseButtonAction(bool isLeft, bool isDown, int x, int y, WPARAM wParam)
{	
	// Insert code for a mouse button action

	/* Example:
	if (isLeft == true && isDown == true) // is it a left mouse click?
	{
		if ( x > 261 && x < 261 + 117 ) // check if click lies within x coordinates of choice
		{
			if ( y > 182 && y < 182 + 33 ) // check if click also lies within y coordinates of choice
			{
				GAME_ENGINE->MessageBox(_T("Clicked."));
			}
		}
	}
	*/
	auto luaMouseButtonAction = GetLua()["mouse_button_action"];
	if (luaMouseButtonAction.valid())
	{
		luaMouseButtonAction(isLeft, isDown, x, y, wParam);
	}
	else
	{
		GAME_ENGINE->MessageBox(_T("Mouse button action not found."));
	}
}

void Game::MouseWheelAction(int x, int y, int distance, WPARAM wParam)
{	
	// Insert code for a mouse wheel action
	auto luaMouseWheelAction = GetLua()["mouse_wheel_action"];
	if (luaMouseWheelAction.valid())
	{
		luaMouseWheelAction(x, y, distance, wParam);
	}
	else
	{
		GAME_ENGINE->MessageBox(_T("Mouse wheel action not found."));
	}
}

void Game::MouseMove(int x, int y, WPARAM wParam)
{	
	// Insert code that needs to execute when the mouse pointer moves across the game window

	/* Example:
	if ( x > 261 && x < 261 + 117 ) // check if mouse position is within x coordinates of choice
	{
		if ( y > 182 && y < 182 + 33 ) // check if mouse position also is within y coordinates of choice
		{
			GAME_ENGINE->MessageBox("Mouse move.");
		}
	}
	*/
	auto luaMouseMove = GetLua()["mouse_move"];
	if (luaMouseMove.valid())
	{
		luaMouseMove(x, y, wParam);
	}
	else
	{
		GAME_ENGINE->MessageBox(_T("Mouse move function not found."));
	}
}

void Game::CheckKeyboard()
{	
	// Here you can check if a key is pressed down
	// Is executed once per frame 

	/* Example:
	if (GAME_ENGINE->IsKeyDown(_T('K'))) xIcon -= xSpeed;
	if (GAME_ENGINE->IsKeyDown(_T('L'))) yIcon += xSpeed;
	if (GAME_ENGINE->IsKeyDown(_T('M'))) xIcon += xSpeed;
	if (GAME_ENGINE->IsKeyDown(_T('O'))) yIcon -= ySpeed;
	*/
	auto luaCheckKeyboard = GetLua()["check_keyboard"];
	if (luaCheckKeyboard.valid())
	{
		luaCheckKeyboard();
	}
	else
	{
		GAME_ENGINE->MessageBox(_T("Check keyboard function not found."));
	}
}

void Game::KeyPressed(TCHAR key)
{	
	// DO NOT FORGET to use SetKeyList() !!

	// Insert code that needs to execute when a key is pressed
	// The function is executed when the key is *released*
	// You need to specify the list of keys with the SetKeyList() function

	/* Example:
	switch (key)
	{
	case _T('K'): case VK_LEFT:
		GAME_ENGINE->MessageBox("Moving left.");
		break;
	case _T('L'): case VK_DOWN:
		GAME_ENGINE->MessageBox("Moving down.");
		break;
	case _T('M'): case VK_RIGHT:
		GAME_ENGINE->MessageBox("Moving right.");
		break;
	case _T('O'): case VK_UP:
		GAME_ENGINE->MessageBox("Moving up.");
		break;
	case VK_ESCAPE:
		GAME_ENGINE->MessageBox("Escape menu.");
	}
	*/
	auto luaKeyPressed = GetLua()["key_pressed"];
	if (luaKeyPressed.valid())
	{
		luaKeyPressed(key);
	}
	else
	{
		GAME_ENGINE->MessageBox(_T("Key pressed function not found."));
	}
}

void Game::CallAction(Caller* callerPtr)
{
	// Insert the code that needs to execute when a Caller (= Button, TextBox, Timer, Audio) executes an action
	auto luaCallAction = GetLua()["call_action"];
	if (luaCallAction.valid())
	{
		luaCallAction(callerPtr);
	}
	else
	{
		GAME_ENGINE->MessageBox(_T("Call action function not found."));
	}
}
