//-----------------------------------------------------------------
// Game Engine Object
// C++ Header - GameEngine.h - version v8_01
// 2006-2024 Kevin Hoefman - kevin.hoefman@howest.be
//
//-----------------------------------------------------------------

#pragma once

//-----------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------
#define _WIN32_WINNT 0x0A00				// Windows 10 or 11
#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_
#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <tchar.h>
#include <Mmsystem.h>					// winmm.lib header, used for playing sound
#undef MessageBox

#include <objidl.h>						// GDI+ for PNG loading
#include <gdiplus.h>
#include <gdiplusinit.h>
#include <gdiplusheaders.h>
#include <GdiPlus.h>

#include "AbstractGame.h"				// base for all games
#include "GameDefines.h"				// common header files and defines / macros

#include <vector>						// using std::vector for tab control logic
#include <queue>						// using std::queue for event system
#include <algorithm>

//-----------------------------------------------------------------
// Pragma Library includes
//-----------------------------------------------------------------
#pragma comment(lib, "msimg32.lib")		// used for transparency
#pragma comment(lib, "winmm.lib")		// used for sound
#pragma comment(lib, "Gdiplus.lib")		// used for PNG loading

//-----------------------------------------------------------------
// GameEngine Forward Declarations
//-----------------------------------------------------------------
class Bitmap;
class SoundWave;
class Midi;
class HitRegion;
class Font;

//-----------------------------------------------------------------
// GameEngine Class
//-----------------------------------------------------------------
class GameEngine
{
	friend LRESULT CALLBACK WndProc(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam);

public:
	// Constructor and Destructor
	GameEngine();
	virtual ~GameEngine();

	// Disabling copy/move constructors and assignment operators   
	GameEngine(const GameEngine& other)					= delete;
	GameEngine(GameEngine&& other) noexcept				= delete;
	GameEngine& operator=(const GameEngine& other)		= delete;
	GameEngine& operator=(GameEngine&& other) noexcept	= delete;

	// General Member Functions
	void		SetGame				(AbstractGame* gamePtr);
	bool		Run					(HINSTANCE hInstance, int cmdShow);

	void		SetTitle			(const tstring& title);			// SetTitle automatically sets the window class name 
	void		SetWindowPosition	(int left, int top);
	bool		SetWindowRegion		(const HitRegion* regionPtr);
	void		SetKeyList			(const tstring& keyList);
	void		SetFrameRate		(int frameRate);
	void		SetWidth			(int width);
	void		SetHeight			(int height);

	bool		GoFullscreen		();		
	bool		GoWindowedMode		();
	void		ShowMousePointer	(bool value);
	void		Quit				();

	bool		HasWindowRegion		()										const;
	bool		IsFullscreen		()										const;

	bool		IsKeyDown			(int vKey)								const;

	void		MessageBox			(const tstring& message)				const;
	void		MessageBox			(const TCHAR* message)					const;
	template	<typename MyOtherType>
	void		MessageBox			(MyOtherType value)						const	{ MessageBox(to_tstring(value)); }

	bool		MessageContinue		(const tstring& message)				const;

	// Text Dimensions
	SIZE		CalculateTextDimensions(const tstring& text, const Font* fontPtr)							const;
	SIZE		CalculateTextDimensions(const tstring& text, const Font* fontPtr, RECT rect)				const;

	// Draw Functions
	void		SetColor			(COLORREF color);
	void		SetFont				(Font* fontPtr);

	bool		FillWindowRect		(COLORREF color)														const;

	bool		DrawLine			(int x1, int y1, int x2, int y2)										const;

	bool		DrawRect			(int left, int top, int right, int bottom)								const;
	bool		FillRect			(int left, int top, int right, int bottom)								const;
	bool		FillRect			(int left, int top, int right, int bottom, int opacity)					const;
	bool		DrawRoundRect		(int left, int top, int right, int bottom, int radius)					const;
	bool		FillRoundRect		(int left, int top, int right, int bottom, int radius)					const;
	bool		DrawOval			(int left, int top, int right, int bottom)								const;
	bool		FillOval			(int left, int top, int right, int bottom)								const;
	bool		FillOval			(int left, int top, int right, int bottom, int opacity)					const;
	bool		DrawArc				(int left, int top, int right, int bottom, int startDegree, int angle)	const;
	bool		FillArc				(int left, int top, int right, int bottom, int startDegree, int angle)	const;

	int			DrawString			(const tstring& text, int left, int top)								const;
	int			DrawString			(const tstring& text, int left, int top, int right, int bottom)			const;

	bool		DrawBitmap			(const Bitmap* bitmapPtr, int left, int top)							const;
	bool		DrawBitmap			(const Bitmap* bitmapPtr, int left, int top, RECT sourceRect)			const;

	bool		DrawPolygon			(const POINT ptsArr[], int count)										const;
	bool		DrawPolygon			(const POINT ptsArr[], int count, bool close)							const;
	bool		FillPolygon			(const POINT ptsArr[], int count)										const;
	bool		FillPolygon			(const POINT ptsArr[], int count, bool close)							const;

	COLORREF	GetDrawColor		()						const;
	bool		Repaint				()						const;

	// Accessor Member Functions	
	tstring		GetTitle			()						const; 
	HINSTANCE	GetInstance			()						const	{ return m_Instance; }
	HWND		GetWindow			()						const	{ return m_Window; }
	int			GetWidth			()						const	{ return m_Width; }
	int			GetHeight			()						const	{ return m_Height; }
	int			GetFrameRate		()						const	{ return m_FrameRate; }
	int			GetFrameDelay		()						const	{ return m_FrameDelay; }
	POINT		GetWindowPosition	()						const;

	// Tab control
	void		TabNext				(HWND ChildWindow)		const;
	void		TabPrevious			(HWND ChildWindow)		const;
		
private:
	// Private Member Functions	
	LRESULT     HandleEvent			(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam);
	bool        CreateGameWindow	(int cmdShow);
	void		MonitorKeyboard		();

	void		SetInstance			(HINSTANCE hInstance);
	void		SetWindow			(HWND hWindow);

	void		PaintDoubleBuffered	(HDC hDC);
	void		FormPolygon			(const POINT ptsArr[], int count, bool close)			const;
	POINT		AngleToPoint		(int left, int top, int right, int bottom, int angle)	const;

	// Member Variables
	HINSTANCE           m_Instance			{};
	HWND                m_Window			{};
	tstring				m_Title				{};
	int                 m_Width				{};
	int					m_Height			{};
	int                 m_FrameRate			{ 50 };
	int					m_FrameDelay		{ 1000 / m_FrameRate };
	bool				m_RunGameLoop		{ true };
	TCHAR*				m_KeyListPtr		{};
	unsigned int		m_KeybMonitor		{};
	AbstractGame*		m_GamePtr			{};
	bool				m_Fullscreen		{};

	// GDI+ for PNG loading
	ULONG_PTR			m_GDIPlusToken;

	// Draw assistance variables
	HDC					m_HdcDraw			{};
	RECT				m_RectDraw			{};
	bool				m_IsPainting		{};
	COLORREF			m_ColDraw			{};
	HFONT				m_FontDraw			{};

	// Fullscreen assistance variable
	POINT				m_OldPosition		{};

	// Window Region assistance variable
	HitRegion*			m_WindowRegionPtr	{};
};

//------------------------------------------------------------------------------------------------
// Callable Interface
//
// Interface implementation for classes that can be called by "caller" objects
//------------------------------------------------------------------------------------------------
class Caller;	// forward declaration

class Callable
{
public:
	virtual ~Callable() = default;								// virtual destructor for polymorphism

	virtual void CallAction(Caller* callerPtr) = 0;
};

//------------------------------------------------------------------------------------------------
// Caller Base Class
//
// Base Clase implementation for up- and downcasting of "caller" objects: TextBox, Button, Timer, Audio and Video
//------------------------------------------------------------------------------------------------
class Caller
{
public:
	// -------------------------
	// Enums
	// -------------------------
	enum class Type
	{
		TextBox, Button, Timer, Audio, Video
	};

	// -------------------------
	// Destructor   
	// -------------------------
	virtual ~Caller() = default;						// do not delete the targets!

	// -------------------------
	// Disabling copy/move constructors and assignment operators   
	// -------------------------
	Caller(const Caller& other)					= delete;
	Caller(Caller&& other) noexcept				= delete;
	Caller& operator=(const Caller& other)		= delete;
	Caller& operator=(Caller&& other) noexcept	= delete;

	// -------------------------
	// General Member Functions
	// -------------------------
	virtual Type GetType() const = 0;									// pure virtual function

	virtual bool AddActionListener		(Callable* targetPtr);			// public interface method, call is passed on to private method
	virtual bool RemoveActionListener	(const Callable* targetPtr);	// public interface method, call is passed on to private method

protected:
	Caller() {}								// constructor only for derived classes
	std::vector<Callable*> m_TargetList;

	virtual bool CallListeners();			// placing the event code in a separate method instead of directly in the windows messaging
											// function allows inheriting classes to override the event code. 

private:
	bool		AddListenerObject		(Callable* targetPtr);
	bool		RemoveListenerObject	(const Callable* targetPtr);
};

//--------------------------------------------------------------------------
// Timer Class
//--------------------------------------------------------------------------

class Timer : public Caller
{
public:
	// -------------------------
	// Constructor(s) and Destructor
	// -------------------------
	Timer(int msec, Callable* targetPtr, bool repeat = false); 

	virtual ~Timer() override;

	// -------------------------
	// Disabling copy/move constructors and assignment operators   
	// -------------------------
	Timer(const Timer& other)					= delete;
	Timer(Timer&& other) noexcept				= delete;
	Timer& operator=(const Timer& other)		= delete;
	Timer& operator=(Timer&& other) noexcept	= delete;

	// -------------------------
	// General Member Functions
	// -------------------------
	void	Start			();
	void	Stop			();
	void	SetDelay		(int msec);
	void	SetRepeat		(bool repeat);

	bool	IsRunning		()					const;
	int		GetDelay		()					const;
	Type	GetType			()					const;

private:	
	// -------------------------
	// Datamembers
	// -------------------------
	HANDLE	m_TimerHandle	{};
	bool	m_IsRunning		{};
	bool	m_MustRepeat;
	int		m_Delay;

	// -------------------------
	// Handler functions
	// -------------------------	
	static void CALLBACK TimerProcStatic(void* lpParameter, BOOLEAN TimerOrWaitFired); // proc will call CallListeners()
};

//-----------------------------------------------------------------
// TextBox Class
//-----------------------------------------------------------------

class TextBox : public Caller
{
public:
	// -------------------------
	// Constructor(s) and Destructor
	// -------------------------
	TextBox(const tstring& text);
	TextBox();

	virtual ~TextBox() override;

	// -------------------------
	// Disabling copy/move constructors and assignment operators   
	// -------------------------
	TextBox(const TextBox& other)					= delete;
	TextBox(TextBox&& other) noexcept				= delete;
	TextBox& operator=(const TextBox& other)		= delete;
	TextBox& operator=(TextBox&& other) noexcept	= delete;

	// -------------------------
	// General Member Functions
	// -------------------------
	void		SetBounds			(int left, int top, int right, int bottom);
	void		SetText				(const tstring& text);
	void		SetFont				(const tstring& fontName, bool bold, bool italic, bool underline, int size);
	void		SetBackcolor		(COLORREF color);
	void		SetForecolor		(COLORREF color);
	void		SetEnabled			(bool enable);
	void		Show				();
	void		Hide				();

	RECT		GetBounds			()			const;
	tstring		GetText				()			const;
	COLORREF	GetForecolor		()			const;
	COLORREF	GetBackcolor		()			const;
	HBRUSH		GetBackcolorBrush	()			const;
	Type		GetType				()			const;

private:
	// -------------------------
	// Member Functions
	// -------------------------
	void		Repaint				();
	void		Update				();	

	// -------------------------
	// Datamembers
	// -------------------------
	RECT		m_Bounds			{ 0, 0, 100, 25 };
	HWND		m_WndEdit			{};
	WNDPROC		m_ProcOldEdit		{};
	COLORREF	m_BgColor			{ RGB(255, 255, 255) };
	COLORREF	m_ForeColor			{};
	HBRUSH		m_BgColorBrush		{};
	HFONT		m_Font				{};
	HFONT		m_OldFont			{};

	// -------------------------
	// Handler functions
	// -------------------------	
	static LRESULT CALLBACK EditProcStatic(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT EditProc(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam);
};

//-----------------------------------------------------------------
// Button Class
//-----------------------------------------------------------------
class Button : public Caller
{
public:
	// -------------------------
	// Constructor(s) and Destructor
	// -------------------------
	Button(const tstring& label);
	Button();

	virtual ~Button() override;

	// -------------------------
	// Disabling copy/move constructors and assignment operators 
	// -------------------------  
	Button(const Button& other)					= delete;
	Button(Button&& other) noexcept				= delete;
	Button& operator=(const Button& other)		= delete;
	Button& operator=(Button&& other) noexcept	= delete;

	// -------------------------
	// General Member Functions
	// -------------------------
	void		SetBounds		(int left, int top, int right, int bottom);
	void		SetText			(const tstring& text);
	void		SetFont			(const tstring& fontName, bool bold, bool italic, bool underline, int size);
	void		SetEnabled		(bool enable);
	void		Show			();
	void		Hide			();

	RECT		GetBounds		()			const;
	tstring		GetText			()			const;
	Type		GetType			()			const;

private:
	// -------------------------
	// Member Functions
	// -------------------------
	void		Update();

	// -------------------------
	// Datamembers
	// -------------------------
	RECT		m_Bounds		{ 0, 0, 100, 25 };
	HWND		m_WndButton		{};
	WNDPROC		m_ProcOldButton	{};
	bool		m_Armed			{};
	HFONT		m_Font			{};
	HFONT		m_OldFont		{};

	// -------------------------
	// Handler functions
	// -------------------------	
	static LRESULT CALLBACK ButtonProcStatic(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT ButtonProc(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam);
};

//-----------------------------------------------------------------
// Audio Class
//-----------------------------------------------------------------
class Audio : public Caller
{
public:
	// -------------------------
	// Constructor(s) and Destructor
	// -------------------------
	Audio(const tstring& filename);
	//Audio(int IDAudio, const tstring& type);

	virtual ~Audio() override;

	// -------------------------
	// Disabling copy/move constructors and assignment operators   
	// -------------------------
	Audio(const Audio& other)					= delete;
	Audio(Audio&& other) noexcept				= delete;
	Audio& operator=(const Audio& other)		= delete;
	Audio& operator=(Audio&& other) noexcept	= delete;

	// -------------------------
	// General member functions   
	// -------------------------
	// Commands are queued, and only sent when Tick() is called. Calling Tick() needs to be done on the main thread (mcisendstring isn't thread safe) 
	void			Tick				();

	void			Play				(int msecStart = 0, int msecStop = -1);
	void			Pause				();
	void			Stop				();
	void			SetVolume			(int volume);
	void			SetRepeat			(bool repeat);

	const tstring&	GetName				()					const;
	const tstring&	GetAlias			()					const;
	int				GetDuration			()					const;
	bool			IsPlaying			()					const;
	bool			IsPaused			()					const;
	bool			GetRepeat			()					const;
	bool			Exists				()					const;
	int				GetVolume			()					const;
	Type			GetType				()					const;

private:	
	// -------------------------
	// Datamembers
	// -------------------------
	static int	m_Nr;

	tstring		m_Filename;
	tstring		m_Alias;
	bool		m_Playing			{};
	bool		m_Paused			{};
	bool		m_MustRepeat		{};
	HWND		m_hWnd				{};
	int			m_Duration			{ -1 };
	int			m_Volume			{ 100 };

	// -------------------------
	// General Member Functions
	// -------------------------		
	void Create(const tstring& filename);
	void Extract(WORD id, const tstring& type, const tstring& filename) const;
	void SwitchPlayingOff();		

	// -------------------------
	// Private mcisendstring command member functions and command queue datamember
	// -------------------------
	std::queue<tstring> m_CommandQueue;

	void QueuePlayCommand			(int msecStart);
	void QueuePlayCommand			(int msecStart, int msecStop);
	void QueuePauseCommand			();
	void QueueResumeCommand			();
	void QueueStopCommand			();
	void QueueVolumeCommand			(int volume);		
	void QueueCommand				(const tstring& command);

	void SendMCICommand				(const tstring& command);

	//void QueuePositionCommand(int x, int y);
			
	// -------------------------
	// Handler functions
	// -------------------------	
	static LRESULT CALLBACK AudioProcStatic(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam);
};

//-----------------------------------------------------------------
// Bitmap Class
//-----------------------------------------------------------------
class Bitmap final
{
public:
	// -------------------------
	// Constructor(s) and Destructor
	// -------------------------
	Bitmap(const tstring& filename, bool createAlphaChannel = true);
	//Bitmap(HBITMAP hBitmap);						
	//Bitmap(int IDBitmap, const tstring& type, bool createAlphaChannel = true);

	~Bitmap();

	// -------------------------
	// Disabling copy/move constructors and assignment operators   
	// -------------------------
	Bitmap(const Bitmap& other)					= delete;
	Bitmap(Bitmap&& other) noexcept				= delete;
	Bitmap& operator=(const Bitmap& other)		= delete;
	Bitmap& operator=(Bitmap&& other) noexcept	= delete;

	// -------------------------
	// General Member Functions
	// -------------------------
	void		SetTransparencyColor	(COLORREF color);
	void		SetOpacity				(int);

	bool		Exists					()									const;
	int			GetWidth				()									const;
	int			GetHeight				()									const;
	COLORREF	GetTransparencyColor	()									const;
	int			GetOpacity				()									const;
	bool		HasAlphaChannel			()									const;
	bool		SaveToFile				(const tstring& filename)			const;

	HBITMAP		GetHandle				()									const;
	
private:	
	// -------------------------
	// Datamembers
	// -------------------------
	static int		m_Nr;

	HBITMAP			m_hBitmap			{};
	COLORREF		m_TransparencyKey	{};
	int				m_Opacity			{ 100 };
	unsigned char*	m_PixelsPtr			{};
	bool			m_HasAlphaChannel;
	
	// -------------------------
	// Member Functions
	// -------------------------
	HBITMAP LoadPNG(TCHAR* pFilePath);
	void	CreateAlphaChannel(); 
	void	Extract(WORD id, const tstring& type, const tstring& fileName) const;
};

//-----------------------------------------------------------------
// HitRegion Class								
//-----------------------------------------------------------------
class HitRegion final
{
public:
	// -------------------------
	// Enums
	// -------------------------
	enum class Shape
	{
		Ellipse, Rectangle
	};

	//---------------------------
	// Constructor(s) and Destructor
	//---------------------------
	HitRegion(Shape shape, int left, int top, int right, int bottom);			// Use this create to form a rectangular or elliptic hitregion
	HitRegion(const POINT* pointsArr, int numberOfPoints);					// Use this create to form a polygonal hitregion
	HitRegion(const Bitmap* bmpPtr, COLORREF cTransparent = RGB(255, 0, 255), COLORREF cTolerance = 0); // Use this create to create a hitregion from a bitmap

	~HitRegion();	

	//---------------------------
	// Implementing copy/move constructors, disabling assignment operators   
	//---------------------------
	HitRegion(const HitRegion& other);
	HitRegion(HitRegion&& other) noexcept;
	HitRegion& operator=(const HitRegion& other)		= delete;
	HitRegion& operator=(HitRegion&& other) noexcept	= delete;

	//---------------------------
	// General Member Functions
	//---------------------------
	void		Move			(int deltaX, int deltaY);				// Moves the region over the given displacement 

	bool		HitTest			(int x, int y)					const;	// Returns true if the point that corresponds to the given x- and y-values is in the region, false if not
	bool		HitTest			(const HitRegion* regionPtr)	const;	// Returns true if the regions overlap, false if they don't

	POINT		CollisionTest	(const HitRegion* regionPtr)	const;	// Returns {-1000000, -1000000} if the regions don't overlap, and the center point of the overlapping region if they do overlap
																		//		CollisionTest is useful to determine the hitting point of two forms that barely touch

	RECT		GetBounds		()								const;	// Returns the position + width and height of the smallest rectangle that encloses this region (in case of a rectangular region: the region itself) 			

	bool		Exists			()								const;  // Returns true if the hitregion was successfully created, false if not

	HRGN		GetHandle		()								const;	// Returns the handle of the region (Win32 stuff)

private:
	//---------------------------
	// Datamembers
	//---------------------------
	HRGN m_HitRegion			{};			// The region data is stored by means of a Win32 "region" resources (not for 1st semester)

	//---------------------------
	// Private Member Functions
	//---------------------------
	HRGN BitmapToRegion(HBITMAP hBmp, COLORREF cTransparentColor, COLORREF cTolerance) const;	
};

//-----------------------------------------------------------------
// Font Class
//-----------------------------------------------------------------
class Font final
{
public:
	//---------------------------
	// Constructor(s) and Destructor
	//---------------------------
	Font(const tstring& fontName, bool bold, bool italic, bool underline, int size);

	~Font();

	//---------------------------
	// Disabling copy/move constructors and assignment operators   
	//---------------------------
	Font(const Font& other)					= delete;
	Font(Font&& other) noexcept				= delete;
	Font& operator=(const Font& other)		= delete;
	Font& operator=(Font&& other) noexcept	= delete;

	//---------------------------
	// General Member Functions
	//---------------------------
	HFONT GetHandle() const;

private:
	//---------------------------
	// Datamembers
	//---------------------------
	HFONT m_Font;
};

//-----------------------------------------------------------------
// Exception Classes
//-----------------------------------------------------------------
class BadFilenameException final 
{
public:
	BadFilenameException(const tstring& filename);
	tstring GetMessage();

private:
	tstring m_Filename;
};

class FileNotFoundException final
{
public:
	FileNotFoundException(const tstring& filename);
	tstring GetMessage();

private:
	tstring m_Filename;
};

class UnsupportedFormatException final
{
public:
	UnsupportedFormatException(const tstring& filename);
	tstring GetMessage();

private:
	tstring m_Filename;
};

class CouldNotLoadFileException final
{
public:
	CouldNotLoadFileException(const tstring& filename);
	tstring GetMessage();

private:
	tstring m_Filename;
};

class BitmapNotLoadedException final {};
class CouldNotCreateHitregionFromBitmapException final {};

//-----------------------------------------------------------------
// Extra OutputDebugString functions
//-----------------------------------------------------------------
void OutputDebugString(const tstring& text);

//-----------------------------------------------------------------
// Windows Procedure Declarations
//-----------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//-----------------------------------------------------------------
// Extern declaration for GAME_ENGINE global (singleton) object and pointer 
//-----------------------------------------------------------------
extern GameEngine myGameEngine;
extern GameEngine* GAME_ENGINE;