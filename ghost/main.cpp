#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <chrono>
#include <random>
#include <vector>
#include <utility>
#include <Windows.h>
#include "resource.h"

static const double GHOST_POINTS[] =
{
	33.361227, 395.0435,
	29.969887, 388.80882,
	16.148277, 347.38682,
	13.245404, 335.62005,
	4.0145138, 267.38682,
	0.4087038, 202.5779,
	1.6549138, 147.0779,
	4.1079938, 128.88682,
	5.5444034, 122.88682,
	21.716187, 67.165275,
	25.141947, 59.481725,
	34.854747, 43.181585,
	49.160647, 27.045645,
	67.886907, 12.108178,
	97.141949, 0.43130813,
	150.91636, 0.39570813,
	184.27652, 11.760718,
	190.14195, 15.384978,
	194.14195, 17.779178,
	198.88726, 21.272395,
	228.22555, 54.886835,
	232.59165, 62.642205,
	237.63407, 75.142205,
	241.12372, 85.386835,
	244.57935, 100.15665,
	249.6447, 171.88683,
	252.12539, 226.78838,
	257.02345, 253.77039,
	259.58067, 261.88683,
	275.73623, 294.78657,
	284.8409, 307.78029,
	289.10975, 312.88683,
	297.83392, 323.06259,
	303.14196, 330.92862,
	273.84676, 361.17182,
	230.59441, 364.80693,
	205.22784, 363.59077,
	197.4349, 367.37117,
	185.42715, 374.7692,
	167.22695, 387.55002,
	159.89654, 391.3791,
	145.26925, 391.69488,
	101.64195, 382.91813,
	79.641947, 378.29875,
	43.236907, 391.72744,
	33.361227, 395.04351,
	33.361227, 395.0435,
};

static LONG d2l(double d)
{
	return static_cast<LONG>(d);
}

class Window
{

public:

	Window(HINSTANCE hinst);
	~Window();

	void go();

private:

	static HWND make_window(HINSTANCE hinst);
	static RECT get_display_rect();
	static BOOL CALLBACK monitor_enum(HMONITOR hmon, HDC hdc, LPRECT monitor_rect, LPARAM lparam);
	static LRESULT WINAPI wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	static const LONG MAX_VELOCITY;
	static const POINT DEFAULT_VELOCITY;
	static const POINT DEFAULT_POSITION;
	static const TCHAR * NAME;
	static const UINT_PTR TIMER_ID = 1;
	static const int MARGIN        = 20000;

	HINSTANCE hinst_;
	HWND      hwnd_;
	POINT     velocity_;
	RECT      display_rect_;

};

const LONG Window::MAX_VELOCITY = 5;
const POINT Window::DEFAULT_VELOCITY = { 0, 0 };
const POINT Window::DEFAULT_POSITION = { -MARGIN, -MARGIN };
const TCHAR * Window::NAME = TEXT("ghost");

HWND Window::make_window(HINSTANCE hinst)
{
	const WNDCLASS wc =
	{
		0,
		wndproc,
		0,
		0,
		hinst,
		0, 0, 0, 0,
		NAME
	};

	RegisterClass(&wc);

	return
		CreateWindowEx(
		WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
		NAME,
		NAME,
		WS_POPUP | WS_VISIBLE,
		DEFAULT_POSITION.x, DEFAULT_POSITION.y,
		305, 401,
		0,
		0,
		hinst,
		0);
}

LRESULT WINAPI Window::wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	const auto window = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWL_USERDATA));

	switch(msg)
	{
		case WM_TIMER:
		{
			RECT r;

			GetWindowRect(hwnd, &r);

			POINT pos = { r.left, r.top };

			pos.x += window->velocity_.x;
			pos.y += window->velocity_.y;

			if(pos.x < -MARGIN)
			{
				window->velocity_.x = 0;
				pos.x = -MARGIN + 100;
			}
				
			if(pos.x > window->display_rect_.right + MARGIN)
			{
				window->velocity_.x = 0;
				pos.x = window->display_rect_.right + MARGIN - 100;
			}

			if(pos.y < -MARGIN)
			{
				window->velocity_.y = 0;
				pos.y = -MARGIN + 100;
			}

			if(pos.y > window->display_rect_.bottom + MARGIN)
			{
				window->velocity_.y = 0;
				pos.y = window->display_rect_.bottom + MARGIN - 100;
			}

			static auto seed = unsigned(std::chrono::system_clock::now().time_since_epoch().count());
			
			std::mt19937 rand(seed++);

			POINT target;

			target.x = d2l(-MARGIN + (double(rand()) / rand.max()) * (window->display_rect_.right + (MARGIN * 2)));

			rand.seed(seed++);
			target.y = d2l(-MARGIN + (double(rand()) / rand.max()) * (window->display_rect_.bottom + (MARGIN * 2)));

			if(pos.x < target.x)
			{
				rand.seed(seed++);
				window->velocity_.x += d2l((double(rand()) / rand.max()) * 2);
			}

			if(pos.x > target.x)
			{
				rand.seed(seed++);
				window->velocity_.x -= d2l((double(rand()) / rand.max()) * 2);
			}

			if(pos.y < target.y)
			{
				rand.seed(seed++);
				window->velocity_.y += d2l((double(rand()) / rand.max()) * 2);
			}

			if(pos.y > target.y)
			{
				rand.seed(seed++);
				window->velocity_.y -= d2l((double(rand()) / rand.max()) * 2);
			}

			if(window->velocity_.x > MAX_VELOCITY)
			{
				window->velocity_.x = MAX_VELOCITY;
			}

			if(window->velocity_.y > MAX_VELOCITY)
			{
				window->velocity_.y = MAX_VELOCITY;
			}

			SetWindowPos(hwnd, HWND_TOPMOST, pos.x, pos.y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);

			return 0;
		}
		case WM_PAINT:
		{
			BITMAP bmp;
			PAINTSTRUCT ps;

			const auto hdc = BeginPaint(hwnd, &ps);

			const auto hbmp = LoadBitmap(window->hinst_, MAKEINTRESOURCE(IDB_GHOST));
			const auto mdc = CreateCompatibleDC(hdc);

			const auto old_hbmp = SelectObject(mdc, hbmp);

			GetObject(hbmp, sizeof(bmp), &bmp);

			BitBlt(hdc, 1, 0, bmp.bmWidth, bmp.bmHeight, mdc, 0, 0, SRCCOPY);

			SelectObject(mdc, old_hbmp);
			
			DeleteDC(mdc);
			DeleteObject(hbmp);

			EndPaint(hwnd, &ps);

			return 0;
		}
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}
		default:
		{
			return DefWindowProc(hwnd, msg, wparam, lparam);
		}
	}
}

BOOL CALLBACK Window::monitor_enum(HMONITOR hmon, HDC hdc, LPRECT monitor_rect, LPARAM lparam)
{
	const auto rects = reinterpret_cast<std::vector<RECT>*>(lparam);

	rects->push_back(*monitor_rect);

	return TRUE;
}

RECT Window::get_display_rect()
{
	RECT result;

	SetRectEmpty(&result);

	std::vector<RECT> monitor_rects;

	EnumDisplayMonitors(0, 0, monitor_enum, LPARAM(&monitor_rects));

	for(const auto & r : monitor_rects)
	{
		UnionRect(&result, &result, &r);
	}

	return result;
}

Window::Window(HINSTANCE hinst) :
	hinst_(hinst),
	hwnd_(make_window(hinst)),
	velocity_(DEFAULT_VELOCITY),
	display_rect_(get_display_rect())
{
	std::vector<POINT> points;

	for(size_t i = 0; i < sizeof(GHOST_POINTS) / sizeof(GHOST_POINTS[0]); i += 2)
	{
		points.push_back({ d2l(GHOST_POINTS[i]), d2l(GHOST_POINTS[i + 1]) });
	}

	const auto region = CreatePolygonRgn(points.data(), points.size(), WINDING);

	SetWindowRgn(hwnd_, region, TRUE);
	SetWindowLongPtr(hwnd_, GWL_USERDATA, LONG(this));
	ShowWindow(hwnd_, SW_SHOW);
	UpdateWindow(hwnd_);
}

Window::~Window()
{
	UnregisterClass(NAME, hinst_);
}

void Window::go()
{
	SetTimer(hwnd_, TIMER_ID, 10, 0);

	MSG msg;

	while(GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Window(hInstance).go();

	return EXIT_SUCCESS;
}
