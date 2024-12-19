//-----------------------------------------------------------------
// Game Engine WinMain Function
// C++ Source - GameWinMain.cpp - version v8_01
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------
#include "GameWinMain.h"
#include "GameEngine.h"
#include "Game.h"
#include "MyLua.h"

#include <windows.h>
#include <iostream>
#include <io.h>
#include <fcntl.h>

//-----------------------------------------------------------------
// Create GAME_ENGINE global (singleton) object and pointer
//-----------------------------------------------------------------
GameEngine myGameEngine;
GameEngine* GAME_ENGINE{ &myGameEngine };

void InitLua()
{
    auto &lua = GetLua();
    lua.open_libraries(sol::lib::base);

	lua.new_usertype<GameEngine>(
		"GameEngine",
		"set_title", &GameEngine::SetTitle,
		"set_window_position", &GameEngine::SetWindowPosition,
		"set_window_region", &GameEngine::SetWindowRegion,
		"set_key_list", &GameEngine::SetKeyList,
		"set_frame", &GameEngine::SetFrameRate,
		"set_width", &GameEngine::SetWidth,
		"set_height", &GameEngine::SetHeight,
		"go_fullscreen", &GameEngine::GoFullscreen,
		"go_windowed_mode", &GameEngine::GoWindowedMode,
		"show_mouse_pointer", &GameEngine::ShowMousePointer,
		"quit", &GameEngine::Quit,
		"has_window_region", &GameEngine::HasWindowRegion,
		"is_fullscreen", &GameEngine::IsFullscreen,
		"is_key_down", &GameEngine::IsKeyDown,
		"message_box", sol::overload(
			static_cast<void (GameEngine::*)(const tstring&) const>(&GameEngine::MessageBox),
			static_cast<void (GameEngine::*)(const TCHAR*) const>(&GameEngine::MessageBox)
		),
		"message_continue", &GameEngine::MessageContinue,
		"set_color", &GameEngine::SetColor,
		"set_font", &GameEngine::SetFont,
		"fill_window_rect", &GameEngine::FillWindowRect,
		"draw_line", &GameEngine::DrawLine,
		"draw_rect", &GameEngine::DrawRect,
		"fill_rect", sol::overload(
			static_cast<bool (GameEngine::*)(int, int, int, int) const>(&GameEngine::FillRect),
			static_cast<bool (GameEngine::*)(int, int, int, int, int) const>(&GameEngine::FillRect)
		),
		"draw_round_rect", &GameEngine::DrawRoundRect,
		"fill_round_rect", &GameEngine::FillRoundRect,
		"draw_oval", &GameEngine::DrawOval,
		"fill_oval", sol::overload(
			static_cast<bool (GameEngine::*)(int, int, int, int) const>(&GameEngine::FillOval),
			static_cast<bool (GameEngine::*)(int, int, int, int, int) const>(&GameEngine::FillOval)
		),
		"draw_arc", &GameEngine::DrawArc,
		"fill_arc", &GameEngine::FillArc,
		"draw_string", sol::overload(
			static_cast<int (GameEngine::*)(const tstring&, int, int) const>(&GameEngine::DrawString),
			static_cast<int (GameEngine::*)(const tstring&, int, int, int, int) const>(&GameEngine::DrawString)
		),
		"draw_bitmap", sol::overload(
			static_cast<bool (GameEngine::*)(const Bitmap*, int, int) const>(&GameEngine::DrawBitmap),
			static_cast<bool (GameEngine::*)(const Bitmap*, int, int, RECT) const>(&GameEngine::DrawBitmap)
		),
		// "draw_polygon", sol::overload(
			// static_cast<bool (GameEngine::*)(const POINT[], int) const>(&GameEngine::DrawPolygon),
			// static_cast<bool (GameEngine::*)(const POINT[], int, bool) const>(&GameEngine::DrawPolygon)
		// ),
		// "fill_polygon", sol::overload(
			// static_cast<bool (GameEngine::*)(const POINT[], int) const>(&GameEngine::FillPolygon),
			// static_cast<bool (GameEngine::*)(const POINT[], int, bool) const>(&GameEngine::FillPolygon)
		// ),
		"get_draw_color", &GameEngine::GetDrawColor,
		"repaint", &GameEngine::Repaint,
		"get_title", &GameEngine::GetTitle,
		"get_instance", &GameEngine::GetInstance,
		"get_window", &GameEngine::GetWindow,
		"get_width", &GameEngine::GetWidth,
		"get_height", &GameEngine::GetHeight,
		"get_frame_rate", &GameEngine::GetFrameRate,
		"get_frame_delay", &GameEngine::GetFrameDelay,
		"get_window_position", &GameEngine::GetWindowPosition,
		"tab_next", &GameEngine::TabNext,
		"tab_previous", &GameEngine::TabPrevious,
		"debug", &GameEngine::Debug
		);

	lua["GAME_ENGINE"] = GAME_ENGINE;

	auto const scriptResult = lua.safe_script_file("game_breakout.lua");
	if (!scriptResult.valid())
	{
		const sol::error err = scriptResult;
		throw std::exception(err.what());
	}
}

void AllocateConsole()
{
	// Allocate a new console for the application
	if (AllocConsole())
	{
		// Redirect STDOUT to the console
		FILE *fp;
		freopen_s(&fp, "CONOUT$", "w", stdout);
		setvbuf(stdout, NULL, _IONBF, 0); // Disable buffering for stdout

		// Redirect STDERR to the console
		freopen_s(&fp, "CONOUT$", "w", stderr);
		setvbuf(stderr, NULL, _IONBF, 0); // Disable buffering for stderr

		// Redirect STDIN to the console
		freopen_s(&fp, "CONIN$", "r", stdin);
		setvbuf(stdin, NULL, _IONBF, 0); // Disable buffering for stdin

		// Sync C++ streams with the console
		std::ios::sync_with_stdio(true);
	}
}

//-----------------------------------------------------------------
// Main Function
//-----------------------------------------------------------------
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    AllocateConsole();

	InitLua();

	GAME_ENGINE->SetGame(new Game());					// any class that implements AbstractGame

	return GAME_ENGINE->Run(hInstance, nCmdShow);		// here we go
}
