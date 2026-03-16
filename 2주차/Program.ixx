export module rachimel.AF;

import <Windows.h>;
import <memory>;
import <array>;
import <utility>;
import <vector>;

import :Enums;

namespace AF
{
	export class Program
	{
	public:
		Program();
		~Program();

		Program(const Program&) = delete;
		Program& operator= (const Program&) = delete;

		Program(Program&&) = delete;
		Program& operator= (Program&&) = delete;

		//void Initialize(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR szCmdLine, int nCmdShow);

		static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	private:
		std::unique_ptr<class ILogic> _logic;
		std::unique_ptr<class SharedStorage> _sharedStorage;
		// 프로그램 플래그 
		bool _isInitialized;
		bool _isRunning;
	};
}