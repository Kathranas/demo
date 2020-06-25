#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cmath>

// :(
#include <chrono>

#ifdef M_PI
#undef M_PI
#endif
#define M_PI 3.14159265f

// Windows params from main
struct WinParams
{
	HINSTANCE hInstance;
	HINSTANCE hPrevInstance; 
	PWSTR     pCmdLine; 
	int       nCmdShow;
};

// Windows Application Class
class WinApp
{
public:
	int main(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow);
private:
	WinParams  win_params;
	WNDCLASS   main_window_class;
	HWND       main_window;
	BITMAPINFO bitmap_info;
	void*      bitmap;
	int32_t    bitmap_width;
	int32_t    bitmap_height;
	int64_t    frame_time;
	int64_t    total_time;

	static LRESULT CALLBACK main_window_proc(HWND window, UINT message, WPARAM WParam, LPARAM LParam);
	void render();
	void finish_render(HDC device_context);
	void resize_bitmap(int32_t width, int32_t height);
};

// Constants
static const int bytes_per_pixel = 4;

// Globals
static WinApp win_app;

void quit()
{
	std::exit(0);
}

void WinApp::finish_render(HDC device_context)
{
	int32_t window_width  = bitmap_width;
	int32_t window_height = bitmap_height;

	// Copies the colour information from the device indepentdent bitmap
	StretchDIBits(
			device_context,
			0, 0, bitmap_width, bitmap_height,
			0, 0, window_width, window_height,
			bitmap,
			&bitmap_info,
			DIB_RGB_COLORS,
			SRCCOPY);
}

void WinApp::resize_bitmap(int32_t width, int32_t height)
{
	bitmap_width  = width;
	bitmap_height = height;

	if(bitmap)
	{
		VirtualFree(bitmap, 0, MEM_RELEASE);
	}

	bitmap_info.bmiHeader.biSize        = sizeof(bitmap_info.bmiHeader);
	bitmap_info.bmiHeader.biWidth       =  width;
	bitmap_info.bmiHeader.biHeight      = -height;
	bitmap_info.bmiHeader.biPlanes      = 1;
	bitmap_info.bmiHeader.biBitCount    = 32;
	bitmap_info.bmiHeader.biCompression = BI_RGB;

	size_t bitmap_memory_size = (width * height) * bytes_per_pixel;
	bitmap = VirtualAlloc(0, bitmap_memory_size, MEM_COMMIT, PAGE_READWRITE);
}

LRESULT CALLBACK WinApp::main_window_proc(HWND window, UINT message, WPARAM WParam, LPARAM LParam)
{
	switch(message)
	{
		// Resize the window
		case WM_SIZE:
		{
			RECT client_rect;
			GetClientRect(window, &client_rect);
			int32_t width  = client_rect.right - client_rect.left;
			int32_t height = client_rect.bottom - client_rect.top;
			win_app.resize_bitmap(width, height);
			return 0;
		}
		break;

		// Close the window
		case WM_CLOSE:
		{
			quit();
			return 0;
		}
		break;

		case WM_DESTROY:
		{
			quit();
			return 0;
		}
		break;

		case WM_PAINT:
		{
			PAINTSTRUCT paint;
			HDC device_context = BeginPaint(window, &paint);
			win_app.finish_render(device_context);
			EndPaint(window, &paint);
			return 0;
		}
		break;

		default: return DefWindowProc(window, message, WParam, LParam);
	}
}

inline float radians(float x)
{
    return x * (180.0f / M_PI);
}

void WinApp::render()
{
	uint32_t* pixel = nullptr;
	uint8_t*  ptr   = (uint8_t*)(bitmap);
	uint8_t   red   = 000;
	uint8_t   green = 000;
	uint8_t   blue  = 000;

	float time = total_time * 1e-3f;

	for(int32_t y = 0; y < bitmap_height; ++y)
	{
		for(int32_t x = 0; x < bitmap_width; ++x)
		{
			pixel = (uint32_t*)(ptr);

			red   = 000;
			green = 000;
			blue  = 000;

			// Shader code
			
			// Required because we want floating point maths
			float fx = (float)(x);
			float fy = (float)(y);

			// Divide the screen up into squares
			float num_squares = 12.0f;
			float px          = floor(fx / bitmap_width  * num_squares) / num_squares;
			float py          = floor(fy / bitmap_height * num_squares) / num_squares;

			// Traveling waves through each colour channel
			float r = cos(3.0f * px + 2.0f * py - 2.0f * time +  0.0f);
			float g = cos(3.0f * px + 2.0f * py - 2.0f * time + 23.0f);
			float b = cos(3.0f * px + 2.0f * py - 2.0f * time + 21.0f);

			// Normalise result between 0 -> 255
			red   = (uint8_t) (255.0f * (0.5f + 0.5f * r));
			green = (uint8_t) (255.0f * (0.5f + 0.5f * g));
			blue  = (uint8_t) (255.0f * (0.5f + 0.5f * b));

			// Set pixel colour
			*pixel = ((red << 16) | (green << 8) | (blue));

			// Advance
			ptr += bytes_per_pixel;
		}
	}
}

int WinApp::main(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	win_params.hInstance     = hInstance;
	win_params.hPrevInstance = hPrevInstance;
	win_params.pCmdLine      = pCmdLine;
	win_params.nCmdShow      = nCmdShow;

	main_window_class               = WNDCLASS{};
	main_window_class.lpfnWndProc   = main_window_proc;
	main_window_class.lpszClassName = "MainWindow";

	HRESULT result = RegisterClassA(&main_window_class);

	if(!result)
	{
		return EXIT_FAILURE;
	}

	main_window = CreateWindowExA(
			0,
			main_window_class.lpszClassName,
			"Demo",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			0,
			0,
			win_params.hInstance,
			0);

	if(!main_window)
	{
		return EXIT_FAILURE;
	}

	MSG message;

	using Clock = std::chrono::steady_clock;
	Clock::time_point p1 = Clock::now();
	Clock::time_point p2 = p1;

	// Main Loop
	for(;;)
	{
		// Handle window message queue
		while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		HDC device_context = GetDC(main_window);
		render();
		finish_render(device_context);
		ReleaseDC(main_window, device_context);

		p2 = Clock::now();
		frame_time = std::chrono::duration_cast<std::chrono::milliseconds>(p2 - p1).count();
		total_time += frame_time;
		p1 = p2;
	}
	return EXIT_SUCCESS;
}

// Program entrypoint
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	return win_app.main(hInstance, hPrevInstance, pCmdLine, nCmdShow);
}
