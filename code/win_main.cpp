#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstdint>

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

void WinApp::render()
{
	uint32_t stride = bitmap_width * bytes_per_pixel;

	uint8_t* row = (uint8_t*)(bitmap);

	for(int32_t y = 0; y < bitmap_height; ++y)
	{
		uint32_t* pixel = (uint32_t*)(row);

		for(int32_t x = 0; x < bitmap_width; ++x)
		{
			uint8_t red   = 000;
			uint8_t green = 000;
			uint8_t blue  = 000;

			if(x > 100 && x < bitmap_width - 100 && y > 100 && y < bitmap_height - 100)
			{
				red = 255;
			}

			*pixel++ = ((red << 16) | (green << 8) | (blue));
		}
		row += stride;
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
	}
	return EXIT_SUCCESS;
}

// Program entrypoint
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	return win_app.main(hInstance, hPrevInstance, pCmdLine, nCmdShow);
}
