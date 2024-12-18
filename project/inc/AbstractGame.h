//-----------------------------------------------------------------
// AbstractGame Object
// C++ Header - AbstractGame.h - version v8_01
//
// AbstractGame is the abstract class which declares the functions that a 
// game class needs to implement to work with the game engine
//-----------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>

//-----------------------------------------------------------------
// AbstractGame Class
//-----------------------------------------------------------------
class AbstractGame
{
public :
	// Constructor(s) and destructor
	AbstractGame()			= default;
	virtual ~AbstractGame() = default;

	// Disabling copy/move constructors and assignment operators   
	AbstractGame(const AbstractGame& other)					= delete;
	AbstractGame(AbstractGame&& other) noexcept				= delete;
	AbstractGame& operator=(const AbstractGame& other)		= delete;
	AbstractGame& operator=(AbstractGame&& other) noexcept	= delete;

	// Game functions
	virtual void Initialize			();																// implemented in the .cpp file

	virtual void Start				()														= 0;	// pure virtual function
	virtual void End				()														= 0;	// pure virtual function
	virtual void MouseButtonAction	(bool isLeft, bool isDown, int x, int y, WPARAM wParam) = 0;	// pure virtual function
	virtual void MouseWheelAction	(int x, int y, int distance, WPARAM wParam)				= 0;	// pure virtual function
	virtual void MouseMove			(int x, int y, WPARAM wParam)							= 0;	// pure virtual function
	virtual void CheckKeyboard		()														= 0;	// pure virtual function
	virtual void KeyPressed			(TCHAR key)												= 0;	// pure virtual function
	virtual void Paint				(RECT rect) const										= 0;	// pure virtual function
	virtual void Tick				()														= 0;	// pure virtual function
};