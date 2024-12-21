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
#include <io.h>
#include <fcntl.h>
#include <filesystem>
#include <vector>

//-----------------------------------------------------------------
// Create GAME_ENGINE global (singleton) object and pointer
//-----------------------------------------------------------------
GameEngine myGameEngine;
GameEngine* GAME_ENGINE{ &myGameEngine };

void InitLua(std::string const &scriptName)
{
    auto &lua = GetLua();
    lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math, sol::lib::table, sol::lib::string);

    lua.new_usertype<POINT>(
	    "POINT",
	    sol::constructors<POINT(int, int)>()
    );

	lua.new_usertype<Bitmap>(
		"Bitmap",
		sol::constructors<Bitmap(const tstring&, bool)>(),
		"set_transparency_color", &Bitmap::SetTransparencyColor,
		"set_opacity", &Bitmap::SetOpacity,
		"get_width", &Bitmap::GetWidth,
		"get_height", &Bitmap::GetHeight
	);


	lua.new_usertype<GameEngine>(
		"GameEngine",
		"set_title", &GameEngine::SetTitle,
		"set_frame", &GameEngine::SetFrameRate,
		"set_width", &GameEngine::SetWidth,
		"set_height", &GameEngine::SetHeight,
		"show_mouse_pointer", &GameEngine::ShowMousePointer,
		"is_key_down", &GameEngine::IsKeyDown,
		"message_box", sol::overload(
			static_cast<void (GameEngine::*)(const tstring&) const>(&GameEngine::MessageBox),
			static_cast<void (GameEngine::*)(const TCHAR*) const>(&GameEngine::MessageBox)
		),
		"message_continue", &GameEngine::MessageContinue,
		"calculate_text_dimensions", sol::overload(
			static_cast<SIZE (GameEngine::*)(const tstring&, const Font*) const>(&GameEngine::CalculateTextDimensions),
			static_cast<SIZE (GameEngine::*)(const tstring&, const Font*, RECT) const>(&GameEngine::CalculateTextDimensions)
		),
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
		"draw_polygon", sol::overload(
			sol::resolve<bool (std::vector<POINT>, int) const>(&GameEngine::DrawPolygon),
			sol::resolve<bool (std::vector<POINT>, int, bool) const>(&GameEngine::DrawPolygon)
		),
		"fill_polygon", sol::overload(
			sol::resolve<bool (std::vector<POINT>, int) const>(&GameEngine::FillPolygon),
			sol::resolve<bool (std::vector<POINT>, int, bool) const>(&GameEngine::FillPolygon)
		),
		"get_draw_color", &GameEngine::GetDrawColor,
		"debug", &GameEngine::Debug
		);

	lua["GAME_ENGINE"] = GAME_ENGINE;

	auto const scriptResult = lua.safe_script_file(scriptName);
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

auto ParseCommandLine(LPWSTR lpCmdLine) -> std::vector<tstring>
{
    std::vector<tstring> args;
    tstring const cmdLine(lpCmdLine);
    tstring currentArg;
    bool insideQuotes = false;

    for (size_t i = 0; i < cmdLine.size(); ++i)
    {
        if (cmdLine[i] == L'"') // Toggle the quote flag
        {
            insideQuotes = !insideQuotes;
        }
        else if (cmdLine[i] == L' ' && !insideQuotes)
        {
            // Argument separator
            if (!currentArg.empty())
            {
                args.push_back(currentArg);
                currentArg.clear();
            }
        }
        else
        {
            currentArg += cmdLine[i];
        }
    }

    // Add the last argument if any
    if (!currentArg.empty())
    {
        args.push_back(currentArg);
    }

    return args;
}

auto LoadLuaScript(LPWSTR lpCmdLine) -> std::string
{
	std::vector<tstring> const args = ParseCommandLine(lpCmdLine);

	std::string scriptName = "game_breakout.lua";
	if (args.size() == 1)
	{
		const std::filesystem::path scriptPath(args[0]);

		scriptName = scriptPath.filename().string();
	}
	std::cout << "[INFO] Using Script: " << scriptName << std::endl << std::endl;

	return scriptName;
}

//-----------------------------------------------------------------
// Main Function
//-----------------------------------------------------------------
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    AllocateConsole();

	InitLua(LoadLuaScript(lpCmdLine));
	// HookLuaDebug();

	GAME_ENGINE->SetGame(new Game());					// any class that implements AbstractGame

	return GAME_ENGINE->Run(hInstance, nCmdShow);		// here we go
}
