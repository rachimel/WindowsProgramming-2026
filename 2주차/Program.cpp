module rachimel.AF;
import :WinAPI;
import :SharedStorage;
import :Logic;

// TODO : Add Logic Class
namespace AF
{
	Program::Program() 
		: _logic{}, _sharedStorage{std::make_unique<SharedStorage>()}, _isInitialized{false}, _isRunning{false}
	{
	}

	Program::~Program()
	{
	}
	LRESULT Program::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		// Legacy Pointer (this)
		Program* pThis{nullptr};
		if (uMsg == WinAPI::Msg::WM_NCCREATE)
		{
			//lpCreateParams (LPVOID) : additional data that can used to create window
			pThis = static_cast<Program*>(reinterpret_cast<LPCREATESTRUCTW>(lParam)->lpCreateParams);
			SetLastError(0);
			// save this pointer in 
			if (!SetWindowLongPtrW(hWnd, WinAPI::GWLP::USERDATA, reinterpret_cast<LONG_PTR>(pThis)) != 0)
			{
				if (GetLastError() != 0)
				{
					return false;
				}
			}
		}
		else
		{
			pThis = reinterpret_cast<Program*>(GetWindowLongPtrW(hWnd, WinAPI::GWLP::USERDATA));
		}
		if (pThis != nullptr)
		{
			// ...
		}
		return DefWindowProcW(hWnd,uMsg,wParam,lParam);
	}
}