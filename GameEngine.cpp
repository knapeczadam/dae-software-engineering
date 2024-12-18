//-----------------------------------------------------------------
// Game Engine Object
// C++ Source - GameEngine.cpp
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------
#include "GameEngine.h"

#define _USE_MATH_DEFINES	// necessary for including (among other values) PI  - see math.h
#include <math.h>			// used in various draw member functions

#include <stdio.h>
#include <tchar.h>			// used for unicode strings

#include <vector>			// using std::vector for tab control logic

using namespace std;

//-----------------------------------------------------------------
// Windows Functions
//-----------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Route all Windows messages to the game engine
	return GAME_ENGINE->HandleEvent(hWindow, msg, wParam, lParam);
}

//-----------------------------------------------------------------
// GameEngine Constructor(s)/Destructor
//-----------------------------------------------------------------
GameEngine::GameEngine()
{
	// start GDI+ 
	Gdiplus::GdiplusStartupInput gpStartupInput{};
	Gdiplus::GdiplusStartup(&m_GDIPlusToken, &gpStartupInput, NULL);
}

GameEngine::~GameEngine()
{
	// clean up keyboard monitor buffer 
	delete m_KeyListPtr;

	// clean up the font
	if (m_FontDraw != 0)
	{
		DeleteObject(m_FontDraw);
	}

	// shut down GDI+
	Gdiplus::GdiplusShutdown(m_GDIPlusToken);

	// delete the game object
	delete m_GamePtr;
}

//-----------------------------------------------------------------
// Game Engine Member Functions
//-----------------------------------------------------------------
void GameEngine::SetGame(AbstractGame* gamePtr)
{
	m_GamePtr = gamePtr;
}

void GameEngine::MonitorKeyboard()
{
	if (m_KeyListPtr != nullptr && GetForegroundWindow() == m_Window)
	{
		int count{};
		int key{ m_KeyListPtr[0] };

		while (key != '\0' && count < (8 * sizeof(unsigned int)))
		{	
			if ( !(GetAsyncKeyState(key)<0) ) // key is not pressed
			{	    
				if (m_KeybMonitor & (0x1 << count)) {
					m_GamePtr->KeyPressed(key); // if the bit was 1, this fires a keypress
				}
				m_KeybMonitor &= ~(0x1 << count);   // the bit is set to 0: key is not pressed
			}
			else m_KeybMonitor |= (0x1 << count);	// the bit is set to 1: key is pressed

			key = m_KeyListPtr[++count]; // increase count and get next key
		}
	}	
}

void GameEngine::SetTitle(const tstring& title)
{
	m_Title = title;
}

bool GameEngine::Run(HINSTANCE hInstance, int cmdShow)
{
	// set the instance member variable of the game engine
	SetInstance(hInstance);

	// Game initialization
	m_GamePtr->Initialize();

	// Create the game window
	if (!CreateGameWindow(cmdShow)) return false;

	// Double buffering code
	HDC hDC = GetDC(m_Window);
	HDC hBufferDC = CreateCompatibleDC(hDC);

	// Create the buffer
	HBITMAP hBufferBmp = CreateCompatibleBitmap(hDC, m_Width, m_Height);
	HBITMAP hOldBmp = (HBITMAP)SelectObject(hBufferDC, hBufferBmp);

	m_HdcDraw = hBufferDC;
	GetClientRect(m_Window, &m_RectDraw);

	// Framerate control
	LARGE_INTEGER tickFrequency, tickTrigger, currentTick;
	QueryPerformanceFrequency(&tickFrequency);
	int countsPerMillisecond{ int(tickFrequency.LowPart) / 1000};
	QueryPerformanceCounter(&currentTick);
	tickTrigger = currentTick;

	// Enter the main message loop
	MSG msg;
	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			// Process the message
			if (msg.message == WM_QUIT) break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// Get current time stamp
			QueryPerformanceCounter(&currentTick);
			if (currentTick.QuadPart >= tickTrigger.QuadPart)
			{
				// Paint the window
				HDC hDC = GetDC(m_Window);
				PaintDoubleBuffered(hDC);
				ReleaseDC(m_Window, hDC);

				// Call the game tick
				m_GamePtr->Tick();

				// Process user input
				m_GamePtr->CheckKeyboard();
				MonitorKeyboard();

				// update the tick trigger
				tickTrigger.QuadPart = currentTick.QuadPart + m_FrameDelay * countsPerMillisecond;
			}
		}
	}

	// Reset the old bmp of the buffer
	SelectObject(hBufferDC, hOldBmp);

	// Kill the buffer
	DeleteObject(hBufferBmp);
	DeleteDC(hBufferDC);

	// Exit
	return msg.wParam?true:false;
}

void GameEngine::PaintDoubleBuffered(HDC hDC)
{
	m_IsPainting = true;
	m_GamePtr->Paint(m_RectDraw);
	m_IsPainting = false;

	// As a last step copy the buffer DC to the window DC
	BitBlt(hDC, 0, 0, m_Width, m_Height, m_HdcDraw, 0, 0, SRCCOPY);
}

void GameEngine::ShowMousePointer(bool value)
{
	// set the value
	ShowCursor(value);	
	
	// redraw the screen
	InvalidateRect(m_Window, nullptr, true);
}

bool GameEngine::SetWindowRegion(const HitRegion* regionPtr)
{
	if (m_Fullscreen) return false;

	if (regionPtr == nullptr) 
	{	
		// turn off window region
		SetWindowRgn(m_Window, NULL, true);

		// delete the buffered window region (if it exists)
		delete m_WindowRegionPtr;
		m_WindowRegionPtr = nullptr;
	}
	else 
	{
		// if there is already a window region set, release the buffered region object
		if (m_WindowRegionPtr != nullptr)
		{
			// turn off window region for safety
			SetWindowRgn(m_Window, NULL, true);
				
			// delete the buffered window region 
			delete m_WindowRegionPtr;
		}

		// create a copy of the submitted region (windows will lock the region handle that it receives)
		m_WindowRegionPtr = new HitRegion(*regionPtr); 

		// translate region coordinates in the client field to window coordinates, taking title bar and frame into account
		m_WindowRegionPtr->Move(GetSystemMetrics(SM_CXFIXEDFRAME), GetSystemMetrics(SM_CYFIXEDFRAME) + GetSystemMetrics(SM_CYCAPTION));

		// set the window region
		SetWindowRgn(m_Window, m_WindowRegionPtr->GetHandle(), true);
	}

	return true;
}

bool GameEngine::HasWindowRegion() const
{
	return (m_WindowRegionPtr?true:false);
}

bool GameEngine::GoFullscreen()
{
	// exit if already in fullscreen mode
	if (m_Fullscreen) return false;

	// turn off window region without redraw
	SetWindowRgn(m_Window, NULL, false);

	DEVMODE newSettings{};

	// request current screen settings
	EnumDisplaySettings(nullptr, 0, &newSettings);

	//  set desired screen size/res	
 	newSettings.dmPelsWidth  = GetWidth();		
	newSettings.dmPelsHeight = GetHeight();		
	newSettings.dmBitsPerPel = 32;		

	//specify which aspects of the screen settings we wish to change 
 	newSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

	// attempt to apply the new settings, exit if failure, else set datamember to fullscreen and return true
	if (ChangeDisplaySettings(&newSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL )	return false;
	else 
	{
		// store the location of the window
		m_OldPosition = GetWindowPosition();

		// switch off the title bar
	    DWORD dwStyle = (DWORD) GetWindowLongPtr(m_Window, GWL_STYLE);
	    dwStyle &= ~WS_CAPTION;
	    SetWindowLongPtr(m_Window, GWL_STYLE, dwStyle);

		// move the window to (0,0)
		SetWindowPos(m_Window, 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		InvalidateRect(m_Window, nullptr, true);		

		m_Fullscreen = true;

		return true;
	}
}

bool GameEngine::GoWindowedMode()
{
	// exit if already in windowed mode
	if (!m_Fullscreen) return false;

	// this resets the screen to the registry-stored values
  	ChangeDisplaySettings(0, 0);

	// replace the title bar
	DWORD dwStyle = (DWORD) GetWindowLongPtr(m_Window, GWL_STYLE);
    dwStyle = dwStyle | WS_CAPTION;
    SetWindowLongPtr(m_Window, GWL_STYLE, dwStyle);

	// move the window back to its old position
	SetWindowPos(m_Window, 0, m_OldPosition.x, m_OldPosition.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	InvalidateRect(m_Window, nullptr, true);

	m_Fullscreen = false;

	return true;
}

bool GameEngine::IsFullscreen() const
{
	return m_Fullscreen;
}

bool GameEngine::CreateGameWindow(int cmdShow)
{
	// Create the window class for the main window
	WNDCLASSEX wndclass{};
	wndclass.cbSize         = sizeof(wndclass);
	wndclass.style          = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc    = WndProc;
	wndclass.hInstance      = m_Instance;
	wndclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
	wndclass.lpszClassName  = m_Title.c_str();
	
	// Register the window class
	if (!RegisterClassEx(&wndclass)) return false;
	
	// Calculate window dimensions based on client rect
	RECT windowRect{0, 0, m_Width, m_Height};
	AdjustWindowRect(&windowRect, WS_POPUPWINDOW | WS_CAPTION | WS_MINIMIZEBOX | WS_CLIPCHILDREN, false);

	// Calculate the window size and position based upon the size
	int iWindowWidth = windowRect.right - windowRect.left,
		iWindowHeight = windowRect.bottom - windowRect.top;

	if (wndclass.lpszMenuName != NULL)
		iWindowHeight += GetSystemMetrics(SM_CYMENU);

	int iXWindowPos = (GetSystemMetrics(SM_CXSCREEN) - iWindowWidth) / 2,
		iYWindowPos = (GetSystemMetrics(SM_CYSCREEN) - iWindowHeight) / 2;
	
	// Create the window, exit if fail
	m_Window = CreateWindow(m_Title.c_str(), 
							m_Title.c_str(), 
							WS_POPUPWINDOW | WS_CAPTION | WS_MINIMIZEBOX | WS_CLIPCHILDREN, 
							iXWindowPos, 
							iYWindowPos, 
							iWindowWidth,
							iWindowHeight, 
							NULL, 
							NULL, 
							m_Instance, 
							NULL);

	if (!m_Window) return false;
	
	// Show and update the window
	ShowWindow(m_Window, cmdShow);
	UpdateWindow(m_Window);
	
	return true;
}

bool GameEngine::IsKeyDown(int vKey) const
{
	if (GetAsyncKeyState(vKey) < 0) return true;
	else return false;
}

void GameEngine::SetKeyList(const tstring& keyList)
{
	delete m_KeyListPtr; // clear list if one already exists

	// make keylist if needed
	if (keyList.size() > 0)
	{
		m_KeyListPtr = (TCHAR*)malloc((keyList.size() + 1) * sizeof(TCHAR)); // make place for this amount of keys + 1

		for (int count{}; count < (int)keyList.size() + 1; ++count)
		{
			TCHAR key = keyList.c_str()[count];
			m_KeyListPtr[count] = (key > 96 && key < 123) ? key - 32 : key; // insert the key, coverted to uppercase if supplied character is lowercase
		}
	}
}

void GameEngine::SetFrameRate(int frameRate)
{
	m_FrameRate  = frameRate;
	m_FrameDelay = 1000 / frameRate;
}

void GameEngine::SetWidth(int width) 
{
	m_Width = width; 
}

void GameEngine::SetHeight(int height) 
{ 
	m_Height = height; 
}

void GameEngine::Quit()
{
	PostMessage(GameEngine::GetWindow(), WM_DESTROY, NULL, NULL);
}

bool GameEngine::MessageContinue(const tstring& message) const
{
	// MessageBox define is undef'd at begin of GameEngine.h
	#ifdef UNICODE						
		return MessageBoxW(GetWindow(), message.c_str(), m_Title.c_str(), MB_ICONWARNING | MB_OKCANCEL) == IDOK;
	#else
		return MessageBoxA(GetWindow(), text.c_str(), m_Title.c_str(), MB_ICONWARNING | MB_OKCANCEL) == IDOK;
	#endif 
}

void GameEngine::MessageBox(const tstring& message) const
{
	// MessageBox define is undef'd at begin of GameEngine.h
	#ifdef UNICODE						
		MessageBoxW(GetWindow(), message.c_str(), m_Title.c_str(), MB_ICONEXCLAMATION | MB_OK);
	#else
		MessageBoxA(GetWindow(), text.c_str(), m_Title.c_str(), MB_ICONEXCLAMATION | MB_OK);
	#endif 
}

void GameEngine::MessageBox(const TCHAR* message) const
{
	MessageBox(tstring(message));
}

static void CALLBACK EnumInsertChildrenProc(HWND hwnd, LPARAM lParam)
{
	std::vector<HWND>* rowPtr{ reinterpret_cast<std::vector<HWND>*>(lParam) };

	rowPtr->push_back(hwnd); // fill in every element in the vector
}

void GameEngine::TabNext(HWND ChildWindow) const
{
	std::vector<HWND> childWindows; 

	EnumChildWindows(m_Window, (WNDENUMPROC) EnumInsertChildrenProc, (LPARAM) &childWindows);

	int position{};
	HWND temp{ childWindows[position] };
	while(temp != ChildWindow) temp = childWindows[++position]; // find the childWindow in the vector

	if (position == childWindows.size() - 1) SetFocus(childWindows[0]);
	else SetFocus(childWindows[position + 1]);
}

void GameEngine::TabPrevious(HWND ChildWindow) const
{	
	std::vector<HWND> childWindows; 

	EnumChildWindows(m_Window, (WNDENUMPROC) EnumInsertChildrenProc, (LPARAM) &childWindows);

	int position{ (int)childWindows.size() - 1 };
	HWND temp{ childWindows[position] };
	while(temp != ChildWindow) temp = childWindows[--position]; // find the childWindow in the vector

	if (position == 0) SetFocus(childWindows[childWindows.size() - 1]);
	else SetFocus(childWindows[position - 1]);
}

void GameEngine::SetInstance(HINSTANCE hInstance) 
{ 
	m_Instance = hInstance; 
}

void GameEngine::SetWindow(HWND hWindow) 
{ 
	m_Window = hWindow; 
}

SIZE GameEngine::CalculateTextDimensions(const tstring& text, const Font* fontPtr) const
{
	HDC hdc = GetDC(NULL);
	SelectObject(hdc, fontPtr->GetHandle());    //attach font to hdc

	SIZE size;
	GetTextExtentPoint32(hdc, text.c_str(), (int) text.size(), &size);

	ReleaseDC(NULL, hdc);

	return size;
}

SIZE GameEngine::CalculateTextDimensions(const tstring& text, const Font* fontPtr, RECT rect) const
{
	HDC hdc = GetDC(NULL);
	SelectObject(hdc, fontPtr->GetHandle());    //attach font to hdc

	SIZE size;
	GetTextExtentPoint32(hdc, text.c_str(), (int) text.size(), &size);

	int height = DrawText(hdc, text.c_str(), (int) text.size(), &rect, DT_CALCRECT);

	if (size.cx > rect.right - rect.left)
	{
		size.cx = rect.right - rect.left;
		size.cy = height;
	}

	ReleaseDC(NULL, hdc);

	return size;
}

bool GameEngine::DrawLine(int x1, int y1, int x2, int y2) const
{
	if (m_IsPainting)
	{
		HPEN hOldPen, hNewPen = CreatePen(PS_SOLID, 1, m_ColDraw);
		hOldPen = (HPEN)SelectObject(m_HdcDraw, hNewPen);
		MoveToEx(m_HdcDraw, x1, y1, nullptr);
		LineTo(m_HdcDraw, x2, y2);
		MoveToEx(m_HdcDraw, 0, 0, nullptr); // reset the position - sees to it that eg. AngleArc draws from 0,0 instead of the last position of DrawLine
		SelectObject(m_HdcDraw, hOldPen);
		DeleteObject(hNewPen);

		return true;
	}
	else return false;
}

bool GameEngine::DrawPolygon(const POINT ptsArr[], int count) const
{
	return DrawPolygon(ptsArr, count, false);
}

bool GameEngine::DrawPolygon(const POINT ptsArr[], int count, bool close) const
{
	if (m_IsPainting) 
	{	
		HPEN hOldPen, hNewPen = CreatePen(PS_SOLID, 1, m_ColDraw);
		hOldPen = (HPEN)SelectObject(m_HdcDraw, hNewPen);

		FormPolygon(ptsArr, count, close);

		SelectObject(m_HdcDraw, hOldPen);
		DeleteObject(hNewPen);

		return true;
	}
	else return false;
}

bool GameEngine::FillPolygon(const POINT ptsArr[], int count) const
{
	return FillPolygon(ptsArr, count, false);
}

bool GameEngine::FillPolygon(const POINT ptsArr[], int count, bool close) const
{
	if (m_IsPainting)
	{
		HPEN hOldPen, hNewPen = CreatePen(PS_SOLID, 1, m_ColDraw);
		HBRUSH hOldBrush, hNewBrush = CreateSolidBrush(m_ColDraw);
		hOldPen = (HPEN)SelectObject(m_HdcDraw, hNewPen);
		hOldBrush = (HBRUSH)SelectObject(m_HdcDraw, hNewBrush);

		BeginPath(m_HdcDraw);

		FormPolygon(ptsArr, count, close);

		EndPath(m_HdcDraw);
		StrokeAndFillPath(m_HdcDraw);

		SelectObject(m_HdcDraw, hOldPen);
		SelectObject(m_HdcDraw, hOldBrush);

		DeleteObject(hNewPen);
		DeleteObject(hNewBrush);

		return true;
	}
	else return false;
}

void GameEngine::FormPolygon(const POINT ptsArr[], int count, bool close) const
{
	if (!close) Polyline(m_HdcDraw, ptsArr, count);
	else
	{
		POINT* newPtsArr= new POINT[count+1]; // interesting case: this code does not work with memory allocation at compile time => demo case for dynamic memory use
		for (int index{}; index < count; ++index) newPtsArr[index] = ptsArr[index];
		newPtsArr[count] = ptsArr[0];

		Polyline(m_HdcDraw, newPtsArr, count+1);

		delete[] newPtsArr;
	}
}

bool GameEngine::DrawRect(int left, int top, int right, int bottom) const
{
	if (m_IsPainting)
	{
		HPEN hOldPen, hNewPen = CreatePen(PS_SOLID, 1, m_ColDraw);
		hOldPen = (HPEN)SelectObject(m_HdcDraw, hNewPen);

		POINT pts[4] = { left, top, right - 1, top, right - 1, bottom - 1, left, bottom - 1 };
		DrawPolygon(pts, 4, true);

		SelectObject(m_HdcDraw, hOldPen);
		DeleteObject(hNewPen);

		return true;
	}
	else return false;
}

bool GameEngine::FillRect(int left, int top, int right, int bottom) const
{
	if (m_IsPainting)
	{
		HBRUSH hOldBrush, hNewBrush = CreateSolidBrush(m_ColDraw);
		HPEN hOldPen, hNewPen = CreatePen(PS_SOLID, 1, m_ColDraw);

		hOldBrush = (HBRUSH)SelectObject(m_HdcDraw, hNewBrush);
		hOldPen = (HPEN)SelectObject(m_HdcDraw, hNewPen);

		Rectangle(m_HdcDraw, left, top, right, bottom);

		SelectObject(m_HdcDraw, hOldPen);
		SelectObject(m_HdcDraw, hOldBrush);

		DeleteObject(hNewPen);
		DeleteObject(hNewBrush);

		return true;
	}
	else return false;
}

bool GameEngine::FillRect(int left, int top, int right, int bottom, int opacity) const
{
	if (m_IsPainting)
	{
		HDC tempHdc = CreateCompatibleDC(m_HdcDraw);
		BLENDFUNCTION blend = { AC_SRC_OVER, 0, (BYTE)opacity, 0 };

		RECT dim{};
		dim.right = right - left;
		dim.bottom = bottom - top;

		// setup bitmap info   
		BITMAPINFO bmi{};
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = dim.right;
		bmi.bmiHeader.biHeight = dim.bottom;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;         // four 8-bit components 
		bmi.bmiHeader.biCompression = BI_RGB;
		bmi.bmiHeader.biSizeImage = dim.right * dim.bottom * 4;

		// create our DIB section and select the bitmap into the dc 
		HBITMAP hbitmap = CreateDIBSection(tempHdc, &bmi, DIB_RGB_COLORS, nullptr, NULL, 0x0);
		SelectObject(tempHdc, hbitmap);

		HBRUSH fillBrush = CreateSolidBrush(m_ColDraw);
		::FillRect(tempHdc, &dim, fillBrush);

		AlphaBlend(m_HdcDraw, left, top, dim.right, dim.bottom, tempHdc, dim.left, dim.top, dim.right, dim.bottom, blend);

		DeleteObject(fillBrush);
		DeleteObject(hbitmap);
		DeleteObject(tempHdc);

		return true;
	}
	else return false;
}

bool GameEngine::DrawRoundRect(int left, int top, int right, int bottom, int radius) const
{
	if (m_IsPainting)
	{
		HPEN hOldPen, hNewPen = CreatePen(PS_SOLID, 1, m_ColDraw);
		hOldPen = (HPEN)SelectObject(m_HdcDraw, hNewPen);

		BeginPath(m_HdcDraw);

		RoundRect(m_HdcDraw, left, top, right, bottom, radius, radius);

		EndPath(m_HdcDraw);
		StrokePath(m_HdcDraw);

		SelectObject(m_HdcDraw, hOldPen);
		DeleteObject(hNewPen);

		return true;
	}
	else return false;
}

bool GameEngine::FillRoundRect(int left, int top, int right, int bottom, int radius) const
{
	if (m_IsPainting) 
	{
		HBRUSH hOldBrush, hNewBrush = CreateSolidBrush(m_ColDraw);
		HPEN hOldPen, hNewPen = CreatePen(PS_SOLID, 1, m_ColDraw);

		hOldBrush = (HBRUSH)SelectObject(m_HdcDraw, hNewBrush);
		hOldPen = (HPEN)SelectObject(m_HdcDraw, hNewPen);

		RoundRect(m_HdcDraw, left, top, right, bottom, radius, radius);

		SelectObject(m_HdcDraw, hOldPen);
		SelectObject(m_HdcDraw, hOldBrush);

		DeleteObject(hNewPen);
		DeleteObject(hNewBrush);

		return true;
	}
	else return false;
}

bool GameEngine::DrawOval(int left, int top, int right, int bottom) const
{
	if (m_IsPainting)
	{
		HPEN hOldPen, hNewPen = CreatePen(PS_SOLID, 1, m_ColDraw);
		hOldPen = (HPEN)SelectObject(m_HdcDraw, hNewPen);

		Arc(m_HdcDraw, left, top, right, bottom, left, top + (bottom - top) / 2, left, top + (bottom - top) / 2);

		SelectObject(m_HdcDraw, hOldPen);
		DeleteObject(hNewPen);

		return true;
	}
	else return false;
}

bool GameEngine::FillOval(int left, int top, int right, int bottom) const
{
	if (m_IsPainting)
	{
		HBRUSH hOldBrush, hNewBrush = CreateSolidBrush(m_ColDraw);
		HPEN hOldPen, hNewPen = CreatePen(PS_SOLID, 1, m_ColDraw);

		hOldBrush = (HBRUSH)SelectObject(m_HdcDraw, hNewBrush);
		hOldPen = (HPEN)SelectObject(m_HdcDraw, hNewPen);

		Ellipse(m_HdcDraw, left, top, right, bottom);

		SelectObject(m_HdcDraw, hOldPen);
		SelectObject(m_HdcDraw, hOldBrush);

		DeleteObject(hNewPen);
		DeleteObject(hNewBrush);

		return true;
	}
	else return false;
}

bool GameEngine::FillOval(int left, int top, int right, int bottom, int opacity) const
{
	if (m_IsPainting)
	{
		COLORREF color = m_ColDraw;
		if (color == RGB(0, 0, 0)) color = RGB(0, 0, 1);

		HDC tempHdc = CreateCompatibleDC(m_HdcDraw);

		RECT dim{};
		dim.right = right - left;
		dim.bottom = bottom - top;

		// setup bitmap info   
		BITMAPINFO bmi{};
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = dim.right;
		bmi.bmiHeader.biHeight = dim.bottom;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;         // four 8-bit components 
		bmi.bmiHeader.biCompression = BI_RGB;
		bmi.bmiHeader.biSizeImage = dim.right * dim.bottom * 4;

		// create our DIB section and select the bitmap into the dc 
		int* dataPtr = nullptr;
		HBITMAP hbitmap = CreateDIBSection(tempHdc, &bmi, DIB_RGB_COLORS, (void**)&dataPtr, NULL, 0x0);
		SelectObject(tempHdc, hbitmap);

		memset(dataPtr, 0, dim.right * dim.bottom);

		HBRUSH hOldBrush, hNewBrush = CreateSolidBrush(color);
		HPEN hOldPen, hNewPen = CreatePen(PS_SOLID, 1, color);

		hOldBrush = (HBRUSH)SelectObject(tempHdc, hNewBrush);
		hOldPen = (HPEN)SelectObject(tempHdc, hNewPen);

		Ellipse(tempHdc, 0, 0, dim.right, dim.bottom);

		for (int count{}; count < dim.right * dim.bottom; ++count)
		{
			if (dataPtr[count] != 0)
			{
				// set alpha channel and premultiply
				unsigned char* pos = (unsigned char*)&(dataPtr[count]);
				pos[0] = (int)pos[0] * opacity / 255;
				pos[1] = (int)pos[1] * opacity / 255;
				pos[2] = (int)pos[2] * opacity / 255;
				pos[3] = opacity;
			}
		}

		SelectObject(tempHdc, hOldPen);
		SelectObject(tempHdc, hOldBrush);

		BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
		AlphaBlend(m_HdcDraw, left, top, dim.right, dim.bottom, tempHdc, dim.left, dim.top, dim.right, dim.bottom, blend);

		DeleteObject(hNewPen);
		DeleteObject(hNewBrush);
		DeleteObject(hbitmap);
		DeleteObject(tempHdc);

		return true;
	}
	else return false;
}

bool GameEngine::DrawArc(int left, int top, int right, int bottom, int startDegree, int angle) const
{
	if (m_IsPainting)
	{
		if (angle == 0) return false;
		if (angle > 360) { DrawOval(left, top, right, bottom); }
		else
		{
			HPEN hOldPen, hNewPen = CreatePen(PS_SOLID, 1, m_ColDraw);
			hOldPen = (HPEN)SelectObject(m_HdcDraw, hNewPen);

			POINT ptStart = AngleToPoint(left, top, right, bottom, startDegree);
			POINT ptEnd = AngleToPoint(left, top, right, bottom, startDegree + angle);

			if (angle > 0) Arc(m_HdcDraw, left, top, right, bottom, ptStart.x, ptStart.y, ptEnd.x, ptEnd.y);
			else Arc(m_HdcDraw, left, top, right, bottom, ptEnd.x, ptEnd.y, ptStart.x, ptStart.y);

			SelectObject(m_HdcDraw, hOldPen);
			DeleteObject(hNewPen);
		}

		return true;
	}
	else return false;
}

bool GameEngine::FillArc(int left, int top, int right, int bottom, int startDegree, int angle) const
{
	if (m_IsPainting)
	{
		if (angle == 0) return false;
		if (angle > 360) { FillOval(left, top, right, bottom); }
		else
		{
			HBRUSH hOldBrush, hNewBrush = CreateSolidBrush(m_ColDraw);
			HPEN hOldPen, hNewPen = CreatePen(PS_SOLID, 1, m_ColDraw);

			hOldBrush = (HBRUSH)SelectObject(m_HdcDraw, hNewBrush);
			hOldPen = (HPEN)SelectObject(m_HdcDraw, hNewPen);

			POINT ptStart = AngleToPoint(left, top, right, bottom, startDegree);
			POINT ptEnd = AngleToPoint(left, top, right, bottom, startDegree + angle);

			if (angle > 0) Pie(m_HdcDraw, left, top, right, bottom, ptStart.x, ptStart.y, ptEnd.x, ptEnd.y);
			else Pie(m_HdcDraw, left, top, right, bottom, ptEnd.x, ptEnd.y, ptStart.x, ptStart.y);

			SelectObject(m_HdcDraw, hOldPen);
			SelectObject(m_HdcDraw, hOldBrush);

			DeleteObject(hNewPen);
			DeleteObject(hNewBrush);
		}

		return true;
	}
	else return false;
}

POINT GameEngine::AngleToPoint(int left, int top, int right, int bottom, int angle) const
{
	POINT pt{};

	const int width { right  - left };
	const int height{ bottom - top  };

	// if necessary adjust angle so that it has a value between 0 and 360 degrees
	if (angle > 360 || angle < -360) angle = angle % 360;
	if (angle < 0) angle += 360;

	// default values for standard angles
	if		(angle == 0)	{ pt.x = right;					pt.y = top + (height / 2);	}
	else if (angle == 90)	{ pt.x = left + (width / 2);	pt.y = top;					}
	else if (angle == 180)	{ pt.x = left;					pt.y = top + (height / 2);	}
	else if (angle == 270)	{ pt.x = left + (width / 2);	pt.y = top + height;		}
	// else calculate non-default values
	else
	{
		// point on the ellipse = "stelsel" of the cartesian equation of the ellipse combined with y = tg(alpha) * x
		// using the equation for ellipse with 0,0 in the center of the ellipse
		double aSquare		= pow(width  / 2.0, 2);
		double bSquare		= pow(height / 2.0, 2);
		double tangens		= tan(angle * M_PI / 180);
		double tanSquare	= pow(tangens, 2);

		// calculate x
		pt.x = (long) sqrt( aSquare * bSquare / (bSquare + tanSquare * aSquare));
		if (angle > 90 && angle < 270) pt.x *= -1; // sqrt returns the positive value of the square, take the negative value if necessary

		// calculate y
		pt.y = (long) (tangens * pt.x);
		pt.y *= -1;	// reverse the sign because of inverted y-axis

		// offset the ellipse into the screen
		pt.x += left + (width  / 2);
		pt.y += top  + (height / 2);
	}

	return pt;
}

int GameEngine::DrawString(const tstring& text, int left, int top, int right, int bottom) const
{
	if (m_IsPainting)
	{
		if (m_FontDraw != NULL)
		{
			HFONT hOldFont = (HFONT)SelectObject(m_HdcDraw, m_FontDraw);

			COLORREF oldColor = SetTextColor(m_HdcDraw, m_ColDraw);
			SetBkMode(m_HdcDraw, TRANSPARENT);

			RECT rc = { left, top, right - 1, bottom - 1 };
			int result = DrawText(m_HdcDraw, text.c_str(), -1, &rc, DT_WORDBREAK);

			SetBkMode(m_HdcDraw, OPAQUE);
			SetTextColor(m_HdcDraw, oldColor);

			SelectObject(m_HdcDraw, hOldFont);

			return result;
		}
		else
		{
			COLORREF oldColor = SetTextColor(m_HdcDraw, m_ColDraw);
			SetBkMode(m_HdcDraw, TRANSPARENT);

			RECT rc = { left, top, right - 1, bottom - 1 };
			int result = DrawText(m_HdcDraw, text.c_str(), -1, &rc, DT_WORDBREAK);

			SetBkMode(m_HdcDraw, OPAQUE);
			SetTextColor(m_HdcDraw, oldColor);

			return result;
		}
	}
	else return -1;
}

int GameEngine::DrawString(const tstring& text, int left, int top) const
{
	if (m_IsPainting)
	{
		if (m_FontDraw != 0)
		{
			HFONT hOldFont = (HFONT)SelectObject(m_HdcDraw, m_FontDraw);

			COLORREF oldColor = SetTextColor(m_HdcDraw, m_ColDraw);
			SetBkMode(m_HdcDraw, TRANSPARENT);

			int result = TextOut(m_HdcDraw, left, top, text.c_str(), (int)text.size());

			SetBkMode(m_HdcDraw, OPAQUE);
			SetTextColor(m_HdcDraw, oldColor);

			SelectObject(m_HdcDraw, hOldFont);

			return result;
		}
		else
		{
			COLORREF oldColor = SetTextColor(m_HdcDraw, m_ColDraw);
			SetBkMode(m_HdcDraw, TRANSPARENT);

			int result = TextOut(m_HdcDraw, left, top, text.c_str(), (int)text.size());

			SetBkMode(m_HdcDraw, OPAQUE);
			SetTextColor(m_HdcDraw, oldColor);

			return result;
		}
	}
	else return -1;
}


bool GameEngine::DrawBitmap(const Bitmap* bitmapPtr, int left, int top, RECT rect) const
{
	if (m_IsPainting)
	{
		if (!bitmapPtr->Exists()) return false;

		const int opacity = bitmapPtr->GetOpacity();

		if (opacity == 0 && bitmapPtr->HasAlphaChannel()) return true; // don't draw if opacity == 0 and opacity is used

		HDC hdcMem = CreateCompatibleDC(m_HdcDraw);
		HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, bitmapPtr->GetHandle());

		if (bitmapPtr->HasAlphaChannel())
		{
			BLENDFUNCTION blender = { AC_SRC_OVER, 0, (BYTE)(2.55 * opacity), AC_SRC_ALPHA }; // blend function combines opacity and pixel based transparency
			AlphaBlend(m_HdcDraw, left, top, rect.right - rect.left, rect.bottom - rect.top, hdcMem, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, blender);
		}
		else TransparentBlt(m_HdcDraw, left, top, rect.right - rect.left, rect.bottom - rect.top, hdcMem, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, bitmapPtr->GetTransparencyColor());

		SelectObject(hdcMem, hbmOld);
		DeleteDC(hdcMem);

		return true;
	}
	else return false;
}

bool GameEngine::DrawBitmap(const Bitmap* bitmapPtr, int left, int top) const
{
	if (m_IsPainting)
	{
		if (!bitmapPtr->Exists()) return false;

		BITMAP bm;
		GetObject(bitmapPtr->GetHandle(), sizeof(bm), &bm);
		RECT rect { 0, 0, bm.bmWidth, bm.bmHeight };

		return DrawBitmap(bitmapPtr, left, top, rect);
	}
	else return false;
}

bool GameEngine::FillWindowRect(COLORREF color) const
{	
	if (m_IsPainting)
	{
		COLORREF oldColor = GetDrawColor();
		const_cast<GameEngine*>(this)->SetColor(color);
		FillRect(0, 0, m_RectDraw.right, m_RectDraw.bottom);
		const_cast<GameEngine*>(this)->SetColor(oldColor);

		return true;
	}
	else return false;
}

COLORREF GameEngine::GetDrawColor() const
{ 
	return m_ColDraw; 
}

bool GameEngine::Repaint() const
{
	return InvalidateRect(m_Window, nullptr, true)?true:false;
}

tstring	GameEngine::GetTitle() const
{
	#pragma warning(disable:4244)
	return m_Title;
	#pragma warning(default:4244)
}

POINT GameEngine::GetWindowPosition() const
{
	RECT info;
	GetWindowRect(m_Window, &info);

	return { info.left, info.top };
}

void GameEngine::SetWindowPosition(int left, int top)
{
	SetWindowPos(m_Window, NULL, left, top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	InvalidateRect(m_Window, nullptr, TRUE);
}

void GameEngine::SetColor(COLORREF color) 
{ 
	m_ColDraw = color; 
}

void GameEngine::SetFont(Font* fontPtr)
{
	m_FontDraw = fontPtr->GetHandle();
}

LRESULT GameEngine::HandleEvent(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Route Windows messages to game engine member functions
	switch (msg)
	{
		case WM_CREATE:
			// Seed the random number generator
			srand((unsigned int) GetTickCount64());

			// Set the game window 
			SetWindow(hWindow);

			// Run user defined functions for start of the game
			m_GamePtr->Start();  

			return 0;

		case WM_PAINT:
		{
			// Get window, rectangle and HDC
			PAINTSTRUCT ps;
			HDC hDC = BeginPaint(hWindow, &ps);

			PaintDoubleBuffered(hDC);

			// end paint
			EndPaint(hWindow, &ps);

			return 0;
		}
		case WM_CTLCOLOREDIT:
			return SendMessage((HWND) lParam, WM_CTLCOLOREDIT, wParam, lParam);	// delegate this message to the child window

		case WM_CTLCOLORBTN:
			return SendMessage((HWND) lParam, WM_CTLCOLOREDIT, wParam, lParam);	// delegate this message to the child window

		case WM_LBUTTONDOWN:
			m_GamePtr->MouseButtonAction(true, true, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
			return 0;

		case WM_LBUTTONUP:
			m_GamePtr->MouseButtonAction(true, false, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
			return 0;

		case WM_RBUTTONDOWN:
			m_GamePtr->MouseButtonAction(false, true, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
			return 0;

		case WM_RBUTTONUP:
			m_GamePtr->MouseButtonAction(false, false, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
			return 0;
			
		case WM_MOUSEWHEEL:
			m_GamePtr->MouseWheelAction(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (short) HIWORD(wParam), wParam);
			return 0;

		case WM_MOUSEMOVE:
			m_GamePtr->MouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
			return 0;
			
		case WM_SYSCOMMAND:	// trapping this message prevents a freeze after the ALT key is released
			if (wParam == SC_KEYMENU) return 0;			// see win32 API : WM_KEYDOWN
			else break;    

		case WM_DESTROY:
			// User defined code for ending the game
			m_GamePtr->End();
			
			// End and exit the application
			PostQuitMessage(0);

			return 0;

	}
	return DefWindowProc(hWindow, msg, wParam, lParam);
}

//-----------------------------------------------------------------
// Caller Member Functions
//-----------------------------------------------------------------
bool Caller::AddActionListener(Callable* targetPtr)
{
	return AddListenerObject(targetPtr);
}	

bool Caller::RemoveActionListener(const Callable* targetPtr) 
{
	return RemoveListenerObject(targetPtr);
}

class CallAllActions
{
public:
	CallAllActions(Caller* callerPtr) : m_CallerPtr(callerPtr)
	{}

	void operator()(Callable* callablePtr)
	{
		callablePtr->CallAction(m_CallerPtr);
	}

private:
	Caller* m_CallerPtr;
};

bool Caller::CallListeners()   
{	
	for_each(m_TargetList.begin(), m_TargetList.end(), CallAllActions(this));

	return (m_TargetList.size() > 0);
}

bool Caller::AddListenerObject(Callable* targetPtr) 
{
	vector<Callable*>::iterator pos = find(m_TargetList.begin(), m_TargetList.end(), targetPtr);

	if (pos != m_TargetList.end()) return false;
	
	m_TargetList.push_back(targetPtr);

	return true;
}
	
bool Caller::RemoveListenerObject(const Callable* targetPtr) 
{
	vector<Callable*>::iterator pos = find(m_TargetList.begin(), m_TargetList.end(), targetPtr); 

	if (pos == m_TargetList.end()) return false;

	m_TargetList.erase(pos);

	return true;
}

//-----------------------------------------------------------------
// Bitmap Member Functions
//-----------------------------------------------------------------
// set static datamember to zero
int Bitmap::m_Nr = 0;
//
//Bitmap::Bitmap(HBITMAP hBitmap) : m_TransparencyKey(-1), m_Opacity(100), m_Exists(true), m_IsTarga(false), m_HasAlphaChannel(false), m_hBitmap(hBitmap)
//{
//	// nothing to create
//}

Bitmap::Bitmap(const tstring& filename, bool createAlphaChannel) : m_HasAlphaChannel(createAlphaChannel)
{
	{	// separate block => the file stream will close 
		tifstream testExists(filename);
		if (!testExists.good()) throw FileNotFoundException{filename};
	}

	size_t len{ filename.length() };
	if (len < 5) throw BadFilenameException{ filename };

	tstring suffix{ filename.substr(len - 4) };

	// check if the file to load is a png
	if (suffix == _T(".png"))
	{
		m_hBitmap = LoadPNG((TCHAR*) filename.c_str());

		if (!m_hBitmap) throw CouldNotLoadFileException{ filename };
	}
	// else load as bitmap
	else if (suffix == _T(".bmp"))
	{
		m_hBitmap = (HBITMAP) LoadImage(GAME_ENGINE->GetInstance(), filename.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		
		if (!m_hBitmap) throw CouldNotLoadFileException{ filename };

		if (createAlphaChannel) CreateAlphaChannel();
	}
	else throw UnsupportedFormatException{ filename };
}

/*
Bitmap::Bitmap(int IDBitmap, const tstring& type, bool createAlphaChannel): m_TransparencyKey(-1), m_Opacity(100), m_Exists(false)
{
	if (type == _T("BITMAP"))
	{	
		m_IsTarga = false;
		m_HasAlphaChannel = createAlphaChannel;

		m_hBitmap = LoadBitmap(GAME_ENGINE->GetInstance(), MAKEINTRESOURCE(IDBitmap));
		
		if (m_hBitmap != 0) m_Exists = true;
		
		if (createAlphaChannel) CreateAlphaChannel();
	}
	//else if (type == _T("TGA"))
	//{
	//	m_IsTarga = true;
	//	m_HasAlphaChannel = true;

	//	tstringstream buffer;
	//	buffer << "temp\\targa";
	//	buffer << m_Nr++;
	//	buffer << ".tga";

	//	tstring fileName = buffer.str();

	//	Extract(IDBitmap, _T("TGA"), fileName);

	//	TargaLoader* targa = new TargaLoader();

	//	if (targa->Load((TCHAR*) fileName.c_str()) == 1)
	//	{
	//		m_hBitmap = CreateBitmap(targa->GetWidth(), targa->GetHeight(), 1, targa->GetBPP(), (void*)targa->GetImg());
	//		if (m_hBitmap != 0) m_Exists = true;
	//	}
	//	
	//	delete targa;
	//	
	//	CreateAlphaChannel();
	//}
}
*/

HBITMAP Bitmap::LoadPNG(TCHAR* pFilePath)
{
	Gdiplus::Bitmap* bitmapPtr = Gdiplus::Bitmap::FromFile(pFilePath, false);

	HBITMAP hBitmap{};

	if (bitmapPtr)
	{
		bitmapPtr->GetHBITMAP(Gdiplus::Color(0, 0, 0), &hBitmap);
		delete bitmapPtr;
	}

	return hBitmap;
}

void Bitmap::CreateAlphaChannel()
{
	BITMAPINFOHEADER bminfoheader{};
	bminfoheader.biSize        = sizeof(BITMAPINFOHEADER);
	bminfoheader.biWidth       = GetWidth();
	bminfoheader.biHeight      = GetHeight();
	bminfoheader.biPlanes      = 1;
	bminfoheader.biBitCount    = 32;
	bminfoheader.biCompression = BI_RGB;
	
	HDC windowDC = GetWindowDC(GAME_ENGINE->GetWindow());
	m_PixelsPtr = new unsigned char[this->GetWidth() * this->GetHeight() * 4];
	
	GetDIBits(windowDC, m_hBitmap, 0, GetHeight(), m_PixelsPtr, (BITMAPINFO*)&bminfoheader, DIB_RGB_COLORS); // load pixel info

	// add alpha channel values of 255 for every pixel if bmp
	for (int count{}; count < GetWidth() * GetHeight(); ++count)
	{
		m_PixelsPtr[count * 4 + 3] = 255;
	}
	
	SetDIBits(windowDC, m_hBitmap, 0, GetHeight(), m_PixelsPtr, (BITMAPINFO*)&bminfoheader, DIB_RGB_COLORS); 
}

/*
void Bitmap::Premultiply() // Multiply R, G and B with Alpha
{
    //Note that the APIs use premultiplied alpha, which means that the red,
    //green and blue channel values in the bitmap must be premultiplied with
    //the alpha channel value. For example, if the alpha channel value is x,
    //the red, green and blue channels must be multiplied by x and divided by
    //0xff prior to the call.

    unsigned long Index,nPixels;
    unsigned char *bCur;
    short iPixelSize;

	// Set ptr to start of image
    bCur=pImage;

    // Calc number of pixels
    nPixels=width*height;

	// Get pixel size in bytes
    iPixelSize=iBPP/8;

    for(index{};index!=nPixels;++index)  // For each pixel
    {

        *bCur=(unsigned char)((int)*bCur* (int)*(bCur+3)/0xff);
        *(bCur+1)=(unsigned char)((int)*(bCur+1)* (int)*(bCur+3)/0xff);
        *(bCur+2)=(unsigned char)((int)*(bCur+2)* (int)*(bCur+3)/0xff);

        bCur+=iPixelSize; // Jump to next pixel
    }
}
*/

Bitmap::~Bitmap()
{
	if (HasAlphaChannel())
	{
		delete[] m_PixelsPtr;
	}

	DeleteObject(m_hBitmap);
}

bool Bitmap::Exists() const
{
	return m_hBitmap ? true : false;
}

void Bitmap::Extract(WORD id, const tstring& type, const tstring& fileName) const
{
	CreateDirectory(_T("temp\\"), nullptr);

    HRSRC hrsrc = FindResource(NULL, MAKEINTRESOURCE(id), type.c_str());
    HGLOBAL hLoaded = LoadResource(NULL, hrsrc);
    LPVOID lpLock =  LockResource(hLoaded);
    DWORD dwSize = SizeofResource(NULL, hrsrc);
    HANDLE hFile = CreateFile(fileName.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    DWORD dwByteWritten;
    WriteFile(hFile, lpLock , dwSize , &dwByteWritten , NULL);
    CloseHandle(hFile);
    FreeResource(hLoaded);
} 

HBITMAP Bitmap::GetHandle() const
{
	return m_hBitmap;
}

int Bitmap::GetWidth() const
{
	if (!Exists()) return 0;

    BITMAP bm;
	GetObject(m_hBitmap, sizeof(bm), &bm);

    return bm.bmWidth;
}

int Bitmap::GetHeight() const
{
	if (!Exists()) return 0;

	BITMAP bm;
	GetObject(m_hBitmap, sizeof(bm), &bm);

    return bm.bmHeight;
}

void Bitmap::SetTransparencyColor(COLORREF color) // converts transparency value to pixel-based alpha
{
	m_TransparencyKey = color;

	if (HasAlphaChannel())
	{
		const int width	{ GetWidth()  };
		const int height{ GetHeight() };

		BITMAPINFOHEADER bminfoheader{};
		bminfoheader.biSize        = sizeof(BITMAPINFOHEADER);
		bminfoheader.biWidth       = width;
		bminfoheader.biHeight      = height;
		bminfoheader.biPlanes      = 1;
		bminfoheader.biBitCount    = 32;
		bminfoheader.biCompression = BI_RGB;
		
		HDC windowDC = GetWindowDC(GAME_ENGINE->GetWindow());

		unsigned char* newPixelsPtr = new unsigned char[width * height * 4]; // create 32 bit buffer

		for (int count{}; count < width * height; ++count)
		{
			if (RGB(m_PixelsPtr[count * 4 + 2], m_PixelsPtr[count * 4 + 1], m_PixelsPtr[count * 4]) == color) // if the color of this pixel == transparency color
			{
				((int*) newPixelsPtr)[count] = 0;	// set all four values to zero, this assumes sizeof(int) == 4 on this system
													// setting values to zero means premultiplying the RGB values to an alpha of 0
			}
			else ((int*) newPixelsPtr)[count] = ((int*) m_PixelsPtr)[count]; // copy all four values from m_PixelsPtr to NewPixels
		}

		SetDIBits(windowDC, m_hBitmap, 0, height, newPixelsPtr, (BITMAPINFO*) &bminfoheader, DIB_RGB_COLORS); // insert pixels into bitmap

		delete[] newPixelsPtr; //destroy buffer

		ReleaseDC(GAME_ENGINE->GetWindow(), windowDC); // release DC
	}
}

COLORREF Bitmap::GetTransparencyColor() const
{
	return m_TransparencyKey;
}

void Bitmap::SetOpacity(int opacity)
{
	if (HasAlphaChannel())
	{
		if (opacity > 100) m_Opacity = 100;
		else if (opacity < 0) m_Opacity = 0;
		else m_Opacity = opacity;
	}
}

int Bitmap::GetOpacity() const
{
	return m_Opacity;
}

//bool Bitmap::IsTarga() const
//{
//	return m_IsTarga;
//}

bool Bitmap::HasAlphaChannel() const
{
	return m_HasAlphaChannel;
}

bool Bitmap::SaveToFile(const tstring& filename) const
{
	bool result{ true };

	HDC hScreenDC = CreateDC(TEXT("DISPLAY"), nullptr, nullptr, nullptr);

	const int width { GetWidth()  };
	const int height{ GetHeight() };

	BYTE* dataPtr = new BYTE[width * height * 4];

	BITMAPINFO bmi{};
	bmi.bmiHeader.biSize		= sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth		= width;
	bmi.bmiHeader.biHeight		= height;
	bmi.bmiHeader.biPlanes		= 1;
	bmi.bmiHeader.biBitCount	= 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage	= width * height * 4;

	GetDIBits(hScreenDC, m_hBitmap, 0, width, dataPtr, (BITMAPINFO*) &bmi, DIB_RGB_COLORS); // load pixel info

	const DWORD size{ bmi.bmiHeader.biSizeImage };
	
	BITMAPFILEHEADER bfh{};
	bfh.bfType = ('M' << 8) + 'B';
	bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bfh.bfSize = size + bfh.bfOffBits;

	HANDLE hFile = ::CreateFile(filename.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) result = false;
	else
	{
		DWORD dw;
		WriteFile(hFile, &bfh, sizeof(bfh), &dw, 0);
		WriteFile(hFile, &bmi.bmiHeader, sizeof(BITMAPINFOHEADER), &dw, 0);
		WriteFile(hFile, dataPtr, size, &dw, 0);
		CloseHandle(hFile);
	}

	delete[] dataPtr;

	DeleteDC(hScreenDC);

	return result;
}

//-----------------------------------------------------------------
// Audio Member Functions
//-----------------------------------------------------------------

// set static datamember to zero
int Audio::m_Nr = 0;

#pragma warning(disable:4311)
#pragma warning(disable:4312)
Audio::Audio(const tstring& filename)
{	
	{	// separate block => the file stream will close 
		tifstream testExists(filename);
		if (!testExists.good()) throw FileNotFoundException{ filename };
	}

	size_t len{ filename.length() };
	if (len < 5) throw BadFilenameException{ filename };

	tstring suffix{ filename.substr(len - 4) };
	if (suffix == _T(".mp3") || suffix == _T(".wav") || suffix == _T(".mid"))
	{
		tstringstream buffer;
		buffer << _T("audio");
		buffer << m_Nr++;

		m_Alias = buffer.str();
		m_Filename = filename;

		Create(filename);
	}
	else throw UnsupportedFormatException{ filename };
}

//Audio::Audio(int IDAudio, const tstring& type)
//{
//	if (type == _T("MP3") || type == _T("WAV") || type == _T("MID"))
//	{
//		tstringstream buffer;
//		buffer << _T("audio");
//		buffer << m_Nr++;
//
//		m_Alias = buffer.str();
//		m_Filename = tstring(_T("temp\\")) + m_Alias;
//
//		if (type == _T("MP3")) m_Filename += _T(".mp3");
//		else if (type == _T("WAV")) m_Filename += _T(".wav");
//		else m_Filename += _T(".mid");
//			
//		Extract(IDAudio, tstringType, m_Filename);
//
//		Create(m_Filename);
//	}
//}

void Audio::Create(const tstring& filename)
{
	size_t len{ filename.length() };
	ASSERT(len > 4, _T("Audio name length must be longer than 4 characters!"));

	TCHAR response[100];
	tstringstream buffer;

	tstring suffix{ filename.substr(len - 4) };
	if (suffix == _T(".mp3"))
	{
		buffer << _T("open \"") + m_Filename + _T("\" type mpegvideo alias ");
		buffer << m_Alias;
	}
	else if (suffix == _T(".wav"))
	{
		buffer << _T("open \"") + m_Filename + _T("\" type waveaudio alias ");
		buffer << m_Alias;
	}
	else if (suffix == _T(".mid"))
	{
		buffer << _T("open \"") + m_Filename + _T("\" type sequencer alias ");
		buffer << m_Alias;
	}

	int result = mciSendString(buffer.str().c_str(), nullptr, 0, NULL);	
	if (result != 0) return;
	
	buffer.str(_T(""));
	buffer << _T("set ") + m_Alias + _T(" time format milliseconds");
	mciSendString(buffer.str().c_str(), nullptr, 0, NULL);

	buffer.str(_T(""));
	buffer << _T("status ") + m_Alias + _T(" length");
	mciSendString(buffer.str().c_str(), response, 100, NULL);

	buffer.str(_T(""));
	buffer << response;
	buffer >> m_Duration;
	
	// Create a window to catch the MM_MCINOTIFY message with
	m_hWnd = CreateWindow(TEXT("STATIC"), TEXT(""), NULL, 0, 0, 0, 0, NULL, NULL, GAME_ENGINE->GetInstance(), NULL);
	SetWindowLongPtr(m_hWnd, GWLA_WNDPROC, (LONG_PTR) AudioProcStatic);	// set the custom message loop (subclassing)
	SetWindowLongPtr(m_hWnd, GWLA_USERDATA, (LONG_PTR) this);			// set this object as the parameter for the Proc
}

void Audio::Extract(WORD id , const tstring& type, const tstring& filename) const
{
	CreateDirectory(TEXT("temp\\"), nullptr);

    HRSRC hrsrc = FindResource(NULL, MAKEINTRESOURCE(id), type.c_str());
    HGLOBAL hLoaded = LoadResource( NULL, hrsrc);
    LPVOID lpLock =  LockResource(hLoaded);
    DWORD dwSize = SizeofResource(NULL, hrsrc);
    HANDLE hFile = CreateFile(filename.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    DWORD dwByteWritten;
    WriteFile(hFile, lpLock, dwSize, &dwByteWritten, nullptr);
    CloseHandle(hFile);
    FreeResource(hLoaded);
} 

#pragma warning(default:4311)
#pragma warning(default:4312)

Audio::~Audio()
{
	Stop();

	tstring sendString = tstring(_T("close")) + m_Alias;
	mciSendString(sendString.c_str(), nullptr, 0, NULL);

	// release the window resources if necessary
	if (m_hWnd)
	{
		DestroyWindow(m_hWnd);
	}
}

void Audio::Play(int msecStart, int msecStop)
{
	if (!m_Playing)
	{
		m_Playing = true;
		m_Paused = false;

		if (msecStop == -1) QueuePlayCommand(msecStart);
		else QueuePlayCommand(msecStart, msecStop);
	}	
	else if (m_Paused)
	{
		m_Paused = false;

		QueueResumeCommand();
	}
}

void Audio::Pause()
{
	if (m_Playing && !m_Paused) 
	{
		m_Paused = true;

		QueuePauseCommand();
	}
}

void Audio::Stop()
{
	if (m_Playing)
	{
		m_Playing = false;
		m_Paused = false;

		QueueStopCommand();
	}
}

void Audio::QueuePlayCommand(int msecStart)
{
	tstringstream buffer;
	buffer << _T("play ") + m_Alias + _T(" from ");
	buffer << msecStart;
	buffer << _T(" notify");
	
	QueueCommand(buffer.str());
}

void Audio::QueuePlayCommand(int msecStart, int msecStop)
{
	tstringstream buffer;
	buffer << _T("play ") + m_Alias + _T(" from ");
	buffer << msecStart;
	buffer << _T(" to ");
	buffer << msecStop;
	buffer << _T(" notify");
	
	QueueCommand(buffer.str());
}

void Audio::QueuePauseCommand()
{
	QueueCommand(_T("pause ") + m_Alias);
}

void Audio::QueueResumeCommand()
{
	QueueCommand(_T("resume ") + m_Alias);
}

void Audio::QueueStopCommand()
{
	QueueCommand(_T("stop ") + m_Alias);
}

void Audio::QueueVolumeCommand(int volume)
{
	tstringstream buffer;
	buffer << _T("setaudio ") + m_Alias + _T(" volume to ");
	buffer << volume * 10;
	
	QueueCommand(buffer.str());
}

void Audio::QueueCommand(const tstring& command)
{
	m_CommandQueue.push(command);
}

void Audio::Tick()
{
	if (!m_CommandQueue.empty())
	{
		SendMCICommand(m_CommandQueue.front());
		m_CommandQueue.pop();
	}
}

void Audio::SendMCICommand(const tstring& command)
{
	mciSendString(command.c_str(), nullptr, 0, m_hWnd);
}

const tstring& Audio::GetName() const
{
	return m_Filename;
}
	
const tstring& Audio::GetAlias() const
{
	return m_Alias;
}

bool Audio::IsPlaying() const
{
	return m_Playing;
}

bool Audio::IsPaused() const
{
	return m_Paused;
}

void Audio::SwitchPlayingOff()
{
	m_Playing = false;
	m_Paused  = false;
}

void Audio::SetRepeat(bool repeat)
{
	m_MustRepeat = repeat;
}

bool Audio::GetRepeat() const
{
	return m_MustRepeat;
}

int Audio::GetDuration() const
{
	return m_Duration;
}

void Audio::SetVolume(int volume)
{
	m_Volume = min(100, max(0, volume));	// values below 0 and above 100 are trimmed to 0 and 100, respectively

	QueueVolumeCommand(volume);
}

int Audio::GetVolume() const
{
	return m_Volume;
}

bool Audio::Exists() const
{
	return m_hWnd?true:false;
}

Caller::Type Audio::GetType() const
{
	return Caller::Type::Audio;
}

LRESULT Audio::AudioProcStatic(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{	
	#pragma warning(disable: 4312)
	Audio* audioPtr = reinterpret_cast<Audio*>(GetWindowLongPtr(hWnd, GWLA_USERDATA));
	#pragma warning(default: 4312)

	switch (msg)
	{		
	case MM_MCINOTIFY: // message received when an audio file has finished playing - used for repeat function

		if (wParam == MCI_NOTIFY_SUCCESSFUL && audioPtr->IsPlaying())
		{
			audioPtr->SwitchPlayingOff();

			if (audioPtr->GetRepeat()) audioPtr->Play();	// repeat the audio
			else audioPtr->CallListeners();					// notify listeners that the audio file has stopped 
		}
	}
	return 0;	
}

//-----------------------------------------------------------------
// TextBox Member Functions
//-----------------------------------------------------------------

#pragma warning(disable:4311)	
#pragma warning(disable:4312)
TextBox::TextBox(const tstring& text)
{
	// Create the edit box
	m_WndEdit = CreateWindow(_T("EDIT"), text.c_str(), WS_BORDER | WS_CHILD | WS_CLIPSIBLINGS | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL, 0, 0, 0, 0, GAME_ENGINE->GetWindow(), NULL, GAME_ENGINE->GetInstance(), nullptr);

	// Set the new WNDPROC for the edit box, and store old one
	m_ProcOldEdit = (WNDPROC) SetWindowLongPtr(m_WndEdit, GWLA_WNDPROC, (LONG_PTR) EditProcStatic);

	// Set this object as userdata for the static wndproc function of the edit box so that it can call members
	SetWindowLongPtr(m_WndEdit, GWLA_USERDATA, (LONG_PTR) this);

	// Set to a default position
	SetBounds(m_Bounds.left, m_Bounds.top, m_Bounds.right, m_Bounds.bottom);

	// Show by default
	Show();
}

TextBox::TextBox() : TextBox(_T(""))
{}
#pragma warning(default:4311)
#pragma warning(default:4312)

TextBox::~TextBox()
{
	// release the background brush if necessary
	if (m_BgColorBrush != NULL) 
	{
		DeleteObject(m_BgColorBrush);
	}

	// release the font if necessary
	if (m_Font != NULL)
	{
		SelectObject(GetDC(m_WndEdit), m_OldFont);
		DeleteObject(m_Font);
	}
		
	// release the window resources
	DestroyWindow(m_WndEdit);
}

void TextBox::SetBounds(int left, int top, int right, int bottom)
{
	m_Bounds = { left, top, right, bottom };

	MoveWindow(m_WndEdit, left, top, right - left, bottom - top, true);
}

RECT TextBox::GetBounds() const
{
	return m_Bounds;
}

Caller::Type TextBox::GetType() const
{ 
	return Caller::Type::TextBox;
}

void TextBox::SetEnabled(bool enable)
{
	EnableWindow(m_WndEdit, enable);
}

void TextBox::Update()
{
	UpdateWindow(m_WndEdit);
}

void TextBox::Show()
{
	// Show and update the edit box
	ShowWindow(m_WndEdit, SW_SHOW);
	
	Update();
}

void TextBox::Hide()
{
	// Show and update the edit box
	ShowWindow(m_WndEdit, SW_HIDE);
	
	Update();
}

tstring TextBox::GetText() const
{
	int textLength = (int) SendMessage(m_WndEdit, WM_GETTEXTLENGTH, NULL, NULL);
		
	TCHAR* bufferPtr = new TCHAR[textLength + 1];

	SendMessage(m_WndEdit, (UINT) WM_GETTEXT, (WPARAM) (textLength + 1), (LPARAM) bufferPtr);

	tstring newString(bufferPtr);

	delete[] bufferPtr;

	return newString;
}

void TextBox::SetText(const tstring& text)
{
	SendMessage(m_WndEdit, WM_SETTEXT, NULL, (LPARAM) text.c_str());
}

void TextBox::SetFont(const tstring& fontName, bool bold, bool italic, bool underline, int size)
{
	LOGFONT ft{};

	for (int counter{}; counter < (int)fontName.size() && counter < LF_FACESIZE; ++counter)
	{
		ft.lfFaceName[counter] = fontName[counter];
	}

	ft.lfUnderline	= underline ? 1 : 0;
	ft.lfHeight		= size;
	ft.lfWeight		= bold ? FW_BOLD : 0;
	ft.lfItalic		= italic ? 1 : 0;

	// clean up if another custom font was already in place
	if (m_Font != NULL) { DeleteObject(m_Font); }

	// create the font
	m_Font = CreateFontIndirect(&ft);

	// set the font
	SendMessage(m_WndEdit, WM_SETFONT, (WPARAM) m_Font, NULL);

	// redraw the textbox
	Repaint();
}

void TextBox::SetForecolor( COLORREF color )
{
	m_ForeColor = color;

	Repaint();
}

void TextBox::SetBackcolor( COLORREF color )
{
	m_BgColor = color;
	
	if (m_BgColorBrush != 0) DeleteObject(m_BgColorBrush);
	m_BgColorBrush = CreateSolidBrush( color );
	
	Repaint();
}

void TextBox::Repaint()
{
	InvalidateRect(m_WndEdit, nullptr, true);
}

COLORREF TextBox::GetForecolor() const
{
	return m_ForeColor;
}

COLORREF TextBox::GetBackcolor() const
{
	return m_BgColor;
}

HBRUSH TextBox::GetBackcolorBrush() const
{
	return m_BgColorBrush;
}

LRESULT TextBox::EditProcStatic(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	#pragma warning(disable: 4312)
	return reinterpret_cast<TextBox*>(GetWindowLongPtr(hWnd, GWLA_USERDATA))->EditProc(hWnd, msg, wParam, lParam);
	#pragma warning(default: 4312)
}

LRESULT TextBox::EditProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{		
	case WM_CTLCOLOREDIT:
		SetBkColor((HDC) wParam, GetBackcolor() );
		SetTextColor((HDC) wParam, GetForecolor() );

		return (LRESULT) GetBackcolorBrush();

	case WM_CHAR: 
		if (wParam == VK_TAB || wParam == VK_RETURN) return 0;
		break;

	case WM_KEYDOWN :
		switch (wParam)
		{
		case VK_TAB:
			if (GAME_ENGINE->IsKeyDown(VK_SHIFT)) GAME_ENGINE->TabPrevious(hWnd);
			else GAME_ENGINE->TabNext(hWnd);
			return 0;
		case VK_ESCAPE:
			SetFocus(GetParent(hWnd));
			return 0;
		case VK_RETURN:
			CallListeners();
			break;
		}
	}
	return CallWindowProc(m_ProcOldEdit, hWnd, msg, wParam, lParam);
}

//-----------------------------------------------------------------
// Button Member Functions
//-----------------------------------------------------------------

#pragma warning(disable:4311)
#pragma warning(disable:4312)
Button::Button(const tstring& label) 
{
	// Create the button object
	m_WndButton = CreateWindow(_T("BUTTON"), label.c_str(), WS_BORDER | WS_CHILD | WS_CLIPSIBLINGS | WS_TABSTOP | BS_PUSHBUTTON, 0, 0, 0, 0, GAME_ENGINE->GetWindow(), NULL, GAME_ENGINE->GetInstance(), nullptr);

	// Set de new WNDPROC for the button, and store the old one
	m_ProcOldButton = (WNDPROC) SetWindowLongPtr(m_WndButton, GWLA_WNDPROC, (LONG_PTR) ButtonProcStatic);

	// Store 'this' as data for the Button object so that the static PROC can call the member proc
	SetWindowLongPtr(m_WndButton, GWLA_USERDATA, (LONG_PTR) this);

	// Set to a default position
	SetBounds(m_Bounds.left, m_Bounds.top, m_Bounds.right, m_Bounds.bottom);

	// Show by default
	Show();
}

Button::Button() : Button(_T(""))
{}
#pragma warning(default:4311)
#pragma warning(default:4312)

Button::~Button()
{
	// release the font if necessary
	if (m_Font != 0)
	{
		SelectObject(GetDC(m_WndButton), m_OldFont);
		DeleteObject(m_Font);
	}
		
	// release the window resource
	DestroyWindow(m_WndButton);
}

void Button::SetBounds(int left, int top, int right, int bottom)
{
	m_Bounds = { left, top, right, bottom };

	MoveWindow(m_WndButton, left, top, right - left, bottom - top, true);
}

RECT Button::GetBounds() const
{
	return m_Bounds;
}

void Button::SetEnabled(bool enable)
{
	EnableWindow(m_WndButton, enable);
}

void Button::Update()
{
	UpdateWindow(m_WndButton);
}

void Button::Show()
{
	// Show and update the button
	ShowWindow(m_WndButton, SW_SHOW); 
	
	Update();
}

void Button::Hide()
{
	// Show and update the button
	ShowWindow(m_WndButton, SW_HIDE);
	
	Update();
}

tstring Button::GetText() const
{
	int textLength = (int) SendMessage(m_WndButton, WM_GETTEXTLENGTH, NULL, NULL);
	
	TCHAR* bufferPtr = new TCHAR[textLength + 1];

	SendMessage(m_WndButton, WM_GETTEXT, (WPARAM) (textLength + 1), (LPARAM) bufferPtr);

	tstring newString(bufferPtr);

	delete[] bufferPtr;

	return newString;
}

Caller::Type Button::GetType() const
{ 
	return Caller::Type::Button;
}

void Button::SetText(const tstring& text)
{
	SendMessage(m_WndButton, WM_SETTEXT, NULL, (LPARAM) text.c_str());
}

void Button::SetFont(const tstring& fontName, bool bold, bool italic, bool underline, int size)
{
	LOGFONT ft{};

	for (int counter{}; counter < (int)fontName.size() && counter < LF_FACESIZE; ++counter)
	{
		ft.lfFaceName[counter] = fontName[counter];
	}

	ft.lfUnderline	= underline ? 1 : 0;
	ft.lfHeight		= size;
	ft.lfWeight		= bold ? FW_BOLD : 0;
	ft.lfItalic		= italic ? 1 : 0;

	// clean up if another custom font was already in place
	if (m_Font != NULL) { DeleteObject(m_Font); }

	// create the new font. The WM_CTLCOLOREDIT message will set the font when the button is about to redraw
    m_Font = CreateFontIndirect(&ft);

	// redraw the button
	InvalidateRect(m_WndButton, nullptr, true);
}

LRESULT Button::ButtonProcStatic(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	#pragma warning(disable: 4312)
	return reinterpret_cast<Button*>(GetWindowLongPtr(hWnd, GWLA_USERDATA))->ButtonProc(hWnd, msg, wParam, lParam);
	#pragma warning(default: 4312)
}

LRESULT Button::ButtonProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CTLCOLOREDIT:
		if (m_Font != NULL) 
		{
			if (m_OldFont == NULL) m_OldFont = (HFONT) SelectObject((HDC) wParam, m_Font);
			else SelectObject((HDC) wParam, m_Font);
		}
		return 0;

	case WM_CHAR: 
		if (wParam == VK_TAB || wParam == VK_RETURN) return 0;
		break;

	case WM_KEYDOWN :
		switch (wParam)
		{
		case VK_TAB:			
			if (GAME_ENGINE->IsKeyDown(VK_SHIFT)) GAME_ENGINE->TabPrevious(hWnd);
			else GAME_ENGINE->TabNext(hWnd);
			return 0;
		case VK_ESCAPE:
			SetFocus(GetParent(hWnd));
			return 0;
		case VK_SPACE:
			CallListeners();
			break;
		}
		break;
	case WM_LBUTTONDOWN :
	case WM_LBUTTONDBLCLK:					// clicking fast will throw LBUTTONDBLCLK's as well as LBUTTONDOWN's, you need to capture both to catch all button clicks
		m_Armed = true;
		break;
	case WM_LBUTTONUP :
		if (m_Armed)
		{
			RECT rc;
			GetWindowRect(hWnd, &rc);

			POINT pt;
			GetCursorPos(&pt);

			if (PtInRect(&rc, pt)) CallListeners();

			m_Armed = false;
		}
	}
	return CallWindowProc(m_ProcOldButton, hWnd, msg, wParam, lParam);
}

//-----------------------------------------------------------------
// Timer Member Functions
//-----------------------------------------------------------------

Timer::Timer(int msec, Callable* targetPtr, bool repeat) : m_Delay{ msec }, m_MustRepeat{repeat}
{
	AddActionListener(targetPtr);
}

Timer::~Timer()
{
	if (m_IsRunning) Stop(); // stop closes the handle
}

void Timer::Start()
{
	if (m_IsRunning == false)
	{
		CreateTimerQueueTimer(&m_TimerHandle, NULL, TimerProcStatic, (void*) this, m_Delay, m_Delay, WT_EXECUTEINTIMERTHREAD);	
		m_IsRunning = true;
	}
}

void Timer::Stop()
{	
	if (m_IsRunning == true)
	{
		DeleteTimerQueueTimer(NULL, m_TimerHandle, NULL);  
		//CloseHandle (m_TimerHandle);		DeleteTimerQueueTimer automatically closes the handle? MSDN Documentation seems to suggest this
		
		m_IsRunning = false;
	}
}

bool Timer::IsRunning() const
{
	return m_IsRunning;
}

void Timer::SetDelay(int msec)
{
	m_Delay = max(msec, 1); // timer will not accept values less than 1 msec

	if (m_IsRunning)
	{
		Stop();
		Start();
	}
}

void Timer::SetRepeat(bool repeat)
{
	m_MustRepeat = repeat;
}

int Timer::GetDelay() const
{
	return m_Delay;
}

Caller::Type Timer::GetType() const 
{ 
	return Caller::Type::Timer;
}

void CALLBACK Timer::TimerProcStatic(void* lpParameter, BOOLEAN TimerOrWaitFired)
{
	Timer* timerPtr = reinterpret_cast<Timer*>(lpParameter);

	if (timerPtr->m_IsRunning)		timerPtr->CallListeners();

	if (!timerPtr->m_MustRepeat)	timerPtr->Stop();
}

//---------------------------
// HitRegion Member Functions
//---------------------------
HitRegion::HitRegion(Shape shape, int left, int top, int right, int bottom)
{
	if (shape == HitRegion::Shape::Ellipse)
		m_HitRegion = CreateEllipticRgn(left, top, right, bottom);
	else
		m_HitRegion = CreateRectRgn(left, top, right, bottom);
}

HitRegion::HitRegion(const POINT* pointsArr, int numberOfPoints)
{
	m_HitRegion = CreatePolygonRgn(pointsArr, numberOfPoints, WINDING);
}	

HitRegion::HitRegion(const Bitmap* bmpPtr, COLORREF cTransparent, COLORREF cTolerance)
{
	HBITMAP hBitmap = bmpPtr->GetHandle();

	if (!hBitmap) throw BitmapNotLoadedException{};
	else
	{
		// for some reason, the BitmapToRegion function has R and B switched. Flipping the colors to get the right result.
		COLORREF flippedTransparent = RGB(GetBValue(cTransparent), GetGValue(cTransparent), GetRValue(cTransparent));
		COLORREF flippedTolerance = RGB(GetBValue(cTolerance), GetGValue(cTolerance), GetRValue(cTolerance));

		m_HitRegion = BitmapToRegion(hBitmap, flippedTransparent, flippedTolerance);

		if (!m_HitRegion) throw CouldNotCreateHitregionFromBitmapException{};
	}
}	

HitRegion::~HitRegion()
{
	if (m_HitRegion)
		DeleteObject(m_HitRegion);
}

bool HitRegion::Exists() const
{
	return m_HitRegion;
}

//	BitmapToRegion :	Create a region from the "non-transparent" pixels of a bitmap
//	Author :			Jean-Edouard Lachand-Robert (http://www.geocities.com/Paris/LeftBank/1160/resume.htm), June 1998
//  Some modifications: Kevin Hoefman, Febr 2007
HRGN HitRegion::BitmapToRegion(HBITMAP hBmp, COLORREF cTransparentColor, COLORREF cTolerance) const
{
	HRGN hRgn{};

	if (hBmp)
	{
		// Create a memory DC inside which we will scan the bitmap content
		HDC hMemDC = CreateCompatibleDC(NULL);

		if (hMemDC)
		{
			// Get bitmap siz
			BITMAP bm;
			GetObject(hBmp, sizeof(bm), &bm);

			// Create a 32 bits depth bitmap and select it into the memory DC
			BITMAPINFOHEADER RGB32BitsBitmapInfo{};
			RGB32BitsBitmapInfo.biSize			= sizeof(BITMAPINFOHEADER);
			RGB32BitsBitmapInfo.biWidth			= bm.bmWidth;
			RGB32BitsBitmapInfo.biHeight		= bm.bmHeight;
			RGB32BitsBitmapInfo.biPlanes		= 1;
			RGB32BitsBitmapInfo.biBitCount		= 32;
			RGB32BitsBitmapInfo.biCompression	= BI_RGB;

			VOID* pbits32;
			HBITMAP hbm32 = CreateDIBSection(hMemDC, (BITMAPINFO*) &RGB32BitsBitmapInfo, DIB_RGB_COLORS, &pbits32, NULL, NULL);

			if (hbm32)
			{
				HBITMAP holdBmp = (HBITMAP)SelectObject(hMemDC, hbm32);

				// Create a DC just to copy the bitmap into the memory D
				HDC hDC = CreateCompatibleDC(hMemDC);

				if (hDC)
				{
					// Get how many bytes per row we have for the bitmap bits (rounded up to 32 bits
					BITMAP bm32;
					GetObject(hbm32, sizeof(bm32), &bm32);

					while (bm32.bmWidthBytes % 4) bm32.bmWidthBytes++;

					// Copy the bitmap into the memory D
					HBITMAP holdBmp = (HBITMAP)SelectObject(hDC, hBmp);
					BitBlt(hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, hDC, 0, 0, SRCCOPY);

					// For better performances, we will use the ExtCreateRegion() function to create the
					// region. This function take a RGNDATA structure on entry. We will add rectangles b
					// amount of ALLOC_UNIT number in this structure
					#define ALLOC_UNIT	100
					DWORD maxRects = ALLOC_UNIT;
					HANDLE hData = GlobalAlloc(GMEM_MOVEABLE, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects));
					RGNDATA *pData = (RGNDATA *)GlobalLock(hData);
					pData->rdh.dwSize = sizeof(RGNDATAHEADER);
					pData->rdh.iType = RDH_RECTANGLES;
					pData->rdh.nCount = pData->rdh.nRgnSize = 0;
					SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);

					// Keep on hand highest and lowest values for the "transparent" pixel
					BYTE lr = GetRValue(cTransparentColor);
					BYTE lg = GetGValue(cTransparentColor);
					BYTE lb = GetBValue(cTransparentColor);
					BYTE hr = min(0xff, lr + GetRValue(cTolerance));
					BYTE hg = min(0xff, lg + GetGValue(cTolerance));
					BYTE hb = min(0xff, lb + GetBValue(cTolerance));

					// Scan each bitmap row from bottom to top (the bitmap is inverted vertically)
					BYTE *p32 = (BYTE *)bm32.bmBits + (bm32.bmHeight - 1) * bm32.bmWidthBytes;
					for (int y = 0; y < bm.bmHeight; y++)
					{
						// Scan each bitmap pixel from left to righ
						for (int x = 0; x < bm.bmWidth; x++)
						{
							// Search for a continuous range of "non transparent pixels"
							int x0 = x;
							LONG *p = (LONG *)p32 + x;
							while (x < bm.bmWidth)
							{
								BYTE b = GetRValue(*p);
								if (b >= lr && b <= hr)
								{
									b = GetGValue(*p);
									if (b >= lg && b <= hg)
									{
										b = GetBValue(*p);
										if (b >= lb && b <= hb)
											// This pixel is "transparent"
											break;
									}
								}
								p++;
								x++;
							}

							if (x > x0)
							{
								// Add the pixels (x0, y) to (x, y+1) as a new rectangle in the regio
								if (pData->rdh.nCount >= maxRects)
								{
									GlobalUnlock(hData);
									maxRects += ALLOC_UNIT;
									hData = GlobalReAlloc(hData, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), GMEM_MOVEABLE);
									pData = (RGNDATA *)GlobalLock(hData);
								
								}
								RECT *pr = (RECT *)&pData->Buffer;
								SetRect(&pr[pData->rdh.nCount], x0, y, x, y+1);
								if (x0 < pData->rdh.rcBound.left)
									pData->rdh.rcBound.left = x0;
								if (y < pData->rdh.rcBound.top)
									pData->rdh.rcBound.top = y;
								if (x > pData->rdh.rcBound.right)
									pData->rdh.rcBound.right = x;
								if (y+1 > pData->rdh.rcBound.bottom)
									pData->rdh.rcBound.bottom = y+1;
								pData->rdh.nCount++;
							}
						}

						// Go to next row (remember, the bitmap is inverted vertically)
						p32 -= bm32.bmWidthBytes;
					}

					// Create or extend the region with the remaining rectangle
					HRGN h = ExtCreateRegion(nullptr, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), pData);

					if (hRgn)
					{
						CombineRgn(hRgn, hRgn, h, RGN_OR);
						DeleteObject(h);
					}
					else
						hRgn = h;

					// Clean up
					SelectObject(hDC, holdBmp);
					DeleteDC(hDC);
				}

				DeleteObject(SelectObject(hMemDC, holdBmp));
			}

			DeleteDC(hMemDC);
		}
	}

	return hRgn;
}

HitRegion::HitRegion(const HitRegion& other)
{
	m_HitRegion = CreateRectRgn(0, 0, 10, 10); // create dummy region
	CombineRgn(m_HitRegion, other.m_HitRegion, 0, RGN_COPY);
}

HitRegion::HitRegion(HitRegion&& other) noexcept
{
	m_HitRegion = other.m_HitRegion;
	other.m_HitRegion = NULL;
}
	
void HitRegion::Move(int deltaX, int deltaY)
{
	OffsetRgn(m_HitRegion, deltaX, deltaY);
}
	
RECT HitRegion::GetBounds() const
{
	RECT boundingbox;
	GetRgnBox(m_HitRegion, &boundingbox);

	return boundingbox;
}

HRGN HitRegion::GetHandle() const
{
	return m_HitRegion;
}

bool HitRegion::HitTest(int x, int y) const
{
	return PtInRegion(m_HitRegion, x, y) ? true : false;
}

bool HitRegion::HitTest(const HitRegion* regionPtr) const
{
	HRGN temp = CreateRectRgn(0, 0, 10, 10);			// dummy region
	bool result = (CombineRgn(temp, m_HitRegion, regionPtr->m_HitRegion, RGN_AND) != NULLREGION);

	DeleteObject(temp);
	return result;
}
	
POINT HitRegion::CollisionTest(const HitRegion* regionPtr) const
{
	POINT result;

	HRGN temp = CreateRectRgn(0, 0, 10, 10);			// dummy region
	int overlap = CombineRgn(temp, m_HitRegion, regionPtr->m_HitRegion, RGN_AND);

	if (overlap == NULLREGION)
	{
		result.x = -1000000;
		result.y = -1000000;
	}
	else
	{
		RECT boundingbox;
		GetRgnBox(temp, &boundingbox);
		result.x = boundingbox.left + (boundingbox.right - boundingbox.left)/2;
		result.y = boundingbox.top + (boundingbox.bottom - boundingbox.top)/2;
	}

	DeleteObject(temp);
	
	return result;
}


//-----------------------------------------------------------------
// Font Member Functions 
//-----------------------------------------------------------------

Font::Font(const tstring& fontName, bool bold, bool italic, bool underline, int size)
{
	LOGFONT ft{};

	for (int counter{}; counter < (int)fontName.size() && counter < LF_FACESIZE; ++counter)
	{
		ft.lfFaceName[counter] = fontName[counter];
	}

	ft.lfUnderline	= underline ? 1 : 0;
	ft.lfHeight		= size;
	ft.lfWeight		= bold ? FW_BOLD : 0;
	ft.lfItalic		= italic ? 1 : 0;

    m_Font = CreateFontIndirect(&ft);
}

Font::~Font()
{
	DeleteObject(m_Font);
}

HFONT Font::GetHandle() const 
{ 
	return m_Font;
}

//-----------------------------------------------------------------
// Exception Classes Member Functions 
//-----------------------------------------------------------------

BadFilenameException::BadFilenameException(const tstring& filename) : m_Filename{filename}
{}

tstring BadFilenameException::GetMessage()
{
	return tstring(_T("Bad filename: ")) + m_Filename;
}

FileNotFoundException::FileNotFoundException(const tstring& filename) : m_Filename{ filename }
{}

tstring FileNotFoundException::GetMessage()
{
	return tstring(_T("File not found: ")) + m_Filename;
}

UnsupportedFormatException::UnsupportedFormatException(const tstring& filename) : m_Filename{ filename }
{}

tstring UnsupportedFormatException::GetMessage()
{
	return tstring(_T("Unsupported format: ")) + m_Filename;
}

CouldNotLoadFileException::CouldNotLoadFileException(const tstring& filename) : m_Filename{ filename }
{}

tstring CouldNotLoadFileException::GetMessage()
{
	return tstring(_T("Could not load file: ")) + m_Filename;
}
	
//-----------------------------------------------------------------
// OutputDebugString functions
//-----------------------------------------------------------------

void OutputDebugString(const tstring& text)
{
	OutputDebugString(text.c_str());
}
