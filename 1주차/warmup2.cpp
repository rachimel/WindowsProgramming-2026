#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <array>
#include <iostream>
#include <random>
#include <chrono>
#include <string>
#include <unordered_map>
#include <type_traits>
#include <queue>
#include <stack>

enum class Color
{
	Default = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
	Red = FOREGROUND_RED,
	Blue = FOREGROUND_BLUE
};

enum class RegisteredKeys
{
	Num_0 = '0',
	Num_1,
	Num_2,
	Num_3,
	Num_4,
	Num_5,
	Num_6,
	Num_7,
	Num_8,
	Num_9,

	W = 'W',
	A = 'A',
	S = 'S',
	D = 'D',

	R = 'R',
	Q = 'Q',

	Left = VK_LEFT,
	Up,
	Right,
	Down,

	Enter = VK_RETURN
};

enum class KeyState
{
	Ready,
	Down,
	Hold,
	Up,
};

constexpr size_t ARRAY_SIZE{ 10 };
constexpr size_t MAX_ERROR{ 10 };
using Matrix = std::array<std::array<int, ARRAY_SIZE>, ARRAY_SIZE>;

enum class QueueMessageType
{
	Insert,
	Delete
};
struct QueueMessage
{
	QueueMessage() = delete;
	explicit QueueMessage(QueueMessageType type,int key, int mem = 0) : key{ key }, mem{ mem }, type{ type } {}
	~QueueMessage() {}

	QueueMessage(const QueueMessage&) = default;
	QueueMessage& operator= (const QueueMessage&) = default;
	QueueMessage(QueueMessage&&) = default;
	QueueMessage& operator= (QueueMessage&&) = default;

	int key;
	int mem;
	QueueMessageType type;
};

template <typename T>
concept canModular = requires (T a, T b) { a% b; };
template <canModular T>
T safeMod(T a) {
	return (a % static_cast<T>(ARRAY_SIZE) + ARRAY_SIZE) % ARRAY_SIZE; 
}

class ConsoleManager
{
public:
	ConsoleManager() : _buffer1{ nullptr }, _buffer2{ nullptr }, isBuffer1Active{ true }, _keyboardStatus{}
	{
		_buffer1 = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, NULL, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
		_buffer2 = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, NULL, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	}
	~ConsoleManager()
	{
		CloseHandle(_buffer2);
		CloseHandle(_buffer1);
	}
	ConsoleManager(const ConsoleManager& cm) = delete;
	ConsoleManager& operator= (const ConsoleManager& cm) = delete;

	ConsoleManager(ConsoleManager&& other) = delete;
	ConsoleManager& operator=(ConsoleManager&& other) = delete;

	DWORD InitConsole()
	{
		if (_buffer1 == nullptr || _buffer2 == nullptr)
		{
			return GetLastError();
		}
		if (SetConsoleActiveScreenBuffer(_buffer1) == FALSE)
			return GetLastError();

		return TRUE;
	}

	// Logic : 실제 보여야하는 화면의 커서를 움직이면 안되므로 백 버퍼의 커서를 움직이게 한다.
	DWORD MoveCursor(const COORD& coord)
	{
		COORD cursorPos = {coord.X, coord.Y};
		HANDLE consoleBuffer = (isBuffer1Active) ? _buffer2 : _buffer1;
		if (SetConsoleCursorPosition(consoleBuffer, cursorPos) == FALSE)
			return GetLastError();
		return TRUE;
	}

	DWORD Render(const std::vector<std::vector<CHAR_INFO>>& infos)
	{
		HANDLE consoleBuffer = (isBuffer1Active) ? _buffer2 : _buffer1;
		COORD initPos{ 0,0 };
		size_t infoSize{ infos.size() };
		SMALL_RECT outputRect;
		for (const auto& info : infos)
		{
			// WriteConsoleOutputA를 사용하면 outputRect를 실제 문장을 출력할 때 사용한 직사각형의 크기를 반환하므로
			// 현재 윈도우에서 쓸 수 있는 가장 큰 크기로 다시 바꿔야 한다.
			COORD size{static_cast<SHORT>( info.size()),1};
			outputRect.Left = initPos.X;
			outputRect.Right = static_cast<SHORT>(info.size());
			outputRect.Top = initPos.Y;
			outputRect.Bottom = static_cast<SHORT>(infos.size() * 2);
			if (WriteConsoleOutputA(consoleBuffer, info.data(), size, {0,0}, &outputRect) == FALSE)
			{
				DWORD error = GetLastError();
				return GetLastError();
			}
			initPos.Y += 2;
		}
		return TRUE;
	}

	DWORD flip_buffer()
	{
		// 
		HANDLE consoleBuffer = (isBuffer1Active) ? _buffer2 : _buffer1;
		if (SetConsoleActiveScreenBuffer(consoleBuffer) == FALSE)
		{
			return GetLastError();
		}
		isBuffer1Active = (isBuffer1Active) ? false : true;
		return TRUE;
	}

	DWORD flush(const std::vector<std::vector<CHAR_INFO>>& infos)
	{
		HANDLE consoleBuffer = (isBuffer1Active) ? _buffer2 : _buffer1;
		COORD CurrentMaxWinSize = GetLargestConsoleWindowSize(consoleBuffer);
		DWORD charsWritten;
		SHORT AvailableY{static_cast<SHORT>(infos.size())};
		for (SHORT y = 0; y < CurrentMaxWinSize.Y; ++y)
		{
			if (FillConsoleOutputCharacterA(consoleBuffer, ' ', CurrentMaxWinSize.X, {0,y}, &charsWritten) == FALSE)
			{
				return GetLastError();
			}
		}
		return TRUE;
	}

	void UpdateKeyStatus(RegisteredKeys key)
	{
		auto stateIterator = _keyboardStatus.find(key);
		if (stateIterator == _keyboardStatus.end())
		{
			_keyboardStatus[key] = KeyState::Ready;
		}
		KeyState state = _keyboardStatus[key];
		if (GetAsyncKeyState(static_cast<int>(key)) & 0x8000)
		{
			if (stateIterator->second == KeyState::Ready || stateIterator->second == KeyState::Up)
				_keyboardStatus[key] = KeyState::Down;
			else if (state == KeyState::Down)
				_keyboardStatus[key] = KeyState::Hold;
		}
		else
		{
			if (state == KeyState::Hold || state == KeyState::Down)
				_keyboardStatus[key] = KeyState::Up;
			else if (state == KeyState::Up)
				_keyboardStatus[key] = KeyState::Ready;
		}
	}

	bool isKeyPressed(RegisteredKeys key)
	{
		UpdateKeyStatus(key);
		return(_keyboardStatus[key] == KeyState::Up);
	}
private:
	std::unordered_map<RegisteredKeys, KeyState> _keyboardStatus;
	HANDLE _buffer1;
	HANDLE _buffer2;
	bool isBuffer1Active;
};

#pragma region Program_Decl
class Program
{
public:
	Program() : _map{}, _numberSelected{},_abstractCursorPosition{0,0}, _cursorPosition{0,0}, _uid{0,9}, _rd{}, _cm{},
		_qFlag{true},_sumMode{false},_numMemory{}
	{
		for (size_t y = 0; y < ARRAY_SIZE; ++y)
		{
			for (size_t x = 0; x < ARRAY_SIZE; ++x)
				_map[y][x] = _uid(_rd); 
		}

		_abstractCursorPosition.X = _uid(_rd), _abstractCursorPosition.Y = _uid(_rd);
	}
	~Program()
	{
		if (SetConsoleActiveScreenBuffer(GetStdHandle(STD_OUTPUT_HANDLE)) == FALSE)
		{
			// 표준 출력 핸들이 망가질 정도면 컴퓨터 손상이 심각한 수준
			return;
		}
		std::cout <<_errorLog.size() << "Error(s) occured" << '\n';
		std::cout << "[Error code Stack]" << '\n';
		while (!_errorLog.empty())
		{
			LPVOID logMsg;
			auto p = _errorLog.top();
			_errorLog.pop();
			std::cout << "[Failed Function : " << p.first << "]" << '\n';
			if (FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				p.second,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPSTR)(&logMsg),
				0,
				NULL) == FALSE)
			{
				std::cout << "Failed to Create Error Log Message." << '\n';
				continue;
			}
			std::cout << "Error Code : " << p.second << '\n';
			std::cout << "Error Message : " << (LPSTR)logMsg << '\n';

			LocalFree(logMsg);
		}
	}
	 
	// 프로그램은 유일할 예정이고, std::unique_ptr도 사용할 일이 없으므로 move_assignment & move_ctor
	Program(const Program&) = delete;
	Program& operator= (const Program&) = delete;

	Program(Program&&) = delete;
	Program& operator= (Program&&) = delete;
	// 1차원 + 개행문자 : 개행문자를 WriteConsoleOutputA 하면 개행되지 않고 ASCII 코드 10번에 해당 되는 문자가 출력됨.
	// 따라서 CHAR_INFO를 2차원으로 두고 한 번에 한 줄만 출력하게 구현
	// 한국어는 ◙에 해당됨.
	std::vector<std::vector<CHAR_INFO>> MakeCharInfo()
	{
		std::vector<std::vector<CHAR_INFO>> output{};
		COORD pos{ 0,0 };
		for (size_t y = 0; y < ARRAY_SIZE; ++y)
		{
			std::vector<CHAR_INFO> infos{};
			for (size_t x = 0; x < ARRAY_SIZE; ++x)
			{
				int currentElement{ _map[y][x] };
				std::string elementConv{ std::to_string(currentElement) };
				Color color{ Color::Default };
				if ((x == _abstractCursorPosition.X) && (y == _abstractCursorPosition.Y))
				{
					_cursorPosition.X = pos.X + elementConv.size();
					_cursorPosition.Y = pos.Y;
					color = Color::Red;
				}
				else if (currentElement < 10 && _numberSelected[currentElement])
					color = Color::Blue;
				for (const char& c : elementConv)
				{
					CHAR_INFO info;
					info.Char.AsciiChar = c;
					info.Attributes = static_cast<WORD>(color);
					infos.push_back(info);
					pos.X += 1;
				}
				CHAR_INFO info;
				info.Char.AsciiChar = ' ';
				info.Attributes = static_cast<WORD>(color);
				infos.push_back(info);
				pos.X += 1;
			}
			output.push_back(infos);
			pos.X = 0;
			pos.Y += 2;
		}
		return output;
	}

	void Execute()
	{
		DWORD error = _cm.InitConsole();
		if (error != TRUE)
		{
			_errorLog.push(std::make_pair(typeid(&ConsoleManager::InitConsole).name(),error));
		}
		while (_qFlag && _errorLog.size() < MAX_ERROR)
		{
			KeyboardEvent();
			Render();
		}
	}
	void Render()
	{
		DWORD error{};
		// buffer1 , back : buffer2
		const std::vector < std::vector<CHAR_INFO>>& infoRef{ MakeCharInfo() };
		error = _cm.Render(infoRef);
		if (error != TRUE)
		{
			_errorLog.push(std::make_pair(typeid(&ConsoleManager::Render).name(), error));
		}
		error = _cm.MoveCursor(_cursorPosition);
		if (error != TRUE)
		{
			_errorLog.push(std::make_pair(typeid(&ConsoleManager::MoveCursor).name(), error));
		}
		// buffer2, back : buffer1
		error = _cm.flip_buffer();
		if (error != TRUE)
		{
			_errorLog.push(std::make_pair(typeid(&ConsoleManager::flip_buffer).name(),error));
		}
		// buffer1 플러시
		error = _cm.flush(infoRef);
		if (error != TRUE)
		{
			_errorLog.push(std::make_pair(typeid(&ConsoleManager::flush).name(),error));
		}
	}

	void Reset()
	{
		_numberSelected = std::array<bool, ARRAY_SIZE>{};
		for (size_t y = 0; y < ARRAY_SIZE; ++y)
		{
			for (size_t x = 0; x < ARRAY_SIZE; ++x)
				_map[y][x] = _uid(_rd);
		}

		_abstractCursorPosition.X = _uid(_rd), _abstractCursorPosition.Y = _uid(_rd);
	}
	void MoveCol(int amount)
	{
		if (safeMod<int>(amount) == 0) return;
		int targetCol {safeMod<int>(_abstractCursorPosition.X + amount) };
		for (size_t i = 0; i < ARRAY_SIZE; ++i)
		{ 
			std::swap(_map[i][_abstractCursorPosition.X], _map[i][targetCol]);
		}

		std::queue<QueueMessage> q{};
		int lastIdxSwapped{ 0 };
		for (auto it = _numMemory.begin(); it != _numMemory.end(); it++)
		{
			int targetCoord{ ((it->first) / static_cast<int>(ARRAY_SIZE)) * static_cast<int>(ARRAY_SIZE) + targetCol };
			if (it->first % static_cast<int>(ARRAY_SIZE) == _abstractCursorPosition.X)
			{
				if (!q.empty() && q.front().key == it->first) continue;
				auto nearIt = _numMemory.find(targetCoord);
				q.push(QueueMessage{ QueueMessageType::Delete,it->first });
				if (nearIt != _numMemory.end())
				{
					q.push(QueueMessage{ QueueMessageType::Delete,targetCoord });
					q.push(QueueMessage{ QueueMessageType::Insert,it->first,nearIt->second });
					lastIdxSwapped = targetCoord;
				}
				q.push(QueueMessage{ QueueMessageType::Insert,targetCoord,it->second });
			}
			// 현재 커서랑 맞바꾸는 원소가 합으로 표시된 원소라면
			else if (it->first == targetCoord &&  lastIdxSwapped != targetCoord)
			{
				q.push(QueueMessage{ QueueMessageType::Delete,targetCoord });
				int currentCol{ safeMod<int>(targetCol - amount) };
				int currentCoord{(targetCoord / static_cast<int>(ARRAY_SIZE)) * static_cast<int>(ARRAY_SIZE) + currentCol};
				q.push(QueueMessage{ QueueMessageType::Insert,currentCoord,it->second });
			}
		}

		while (!q.empty())
		{
			QueueMessage msg = q.front();
			q.pop();

			switch (msg.type)
			{
			case QueueMessageType::Insert:
			{
				_numMemory[msg.key] = msg.mem;
				break;
			}
			case QueueMessageType::Delete:
			{
				_numMemory.erase(msg.key);
				break;
			}
			}
		}
		_abstractCursorPosition.X = targetCol;
	}

	// amount : amount 만큼 열을 이동
	void MoveRow(int amount)
	{
		amount = safeMod<int>(amount);
		if (amount == 0) return;
		// 이동할 때 현재 위치에서 amount만큼 떨어져 있는 열 번호
		int targetRow{ safeMod<int>(_abstractCursorPosition.Y + amount) };
		std::swap(_map[_abstractCursorPosition.Y], _map[targetRow]);


		std::queue<QueueMessage> q{};
		int lastIdxSwaped{ 0 };
		for (auto it = _numMemory.begin(); it != _numMemory.end(); it++)
		{
			// 이동할 때 현재 위치에서 amount만큼 떨어져 있는 원소의 인덱스
			int targetCoord{ static_cast<int>(ARRAY_SIZE) * targetRow + (it->first % static_cast<int>(ARRAY_SIZE)) };
			if (it->first / static_cast<int>(ARRAY_SIZE) == _abstractCursorPosition.Y)
			{
				if (!q.empty() && q.front().key == it->first) continue;
				auto nearIt = _numMemory.find(targetCoord);
				q.push(QueueMessage{ QueueMessageType::Delete,it->first });
				if (nearIt != _numMemory.end())
				{
					q.push(QueueMessage{ QueueMessageType::Delete,targetCoord });
					q.push(QueueMessage{ QueueMessageType::Insert,it->first,nearIt->second });
					lastIdxSwaped = targetCoord; 
				}
				q.push(QueueMessage{ QueueMessageType::Insert,targetCoord,it->second });
			}
			else if (it->first == targetCoord && lastIdxSwaped != targetCoord)
			{
				q.push(QueueMessage{ QueueMessageType::Delete,targetCoord});
				int currentRow{ safeMod<int>(targetRow - amount) };
				int currentCoord{ static_cast<int>(ARRAY_SIZE) * currentRow + (targetCoord % (static_cast<int>(ARRAY_SIZE))) };
				q.push(QueueMessage{QueueMessageType::Insert,currentCoord,it->second});
			}
		}

		while (!q.empty())
		{
			QueueMessage msg = q.front();
			q.pop();

			switch (msg.type)
			{
			case QueueMessageType::Insert:
			{
				_numMemory[msg.key] = msg.mem;
				break;
			}
			case QueueMessageType::Delete:
			{
				_numMemory.erase(msg.key);
				break;
			}
			}
		}

		_abstractCursorPosition.Y = targetRow;
	}

	void MoveCursor(COORD moveAmount)
	{
		// early return
		if (moveAmount.X == 0 && moveAmount.Y == 0) return;
		if (moveAmount.X != 0)
			_abstractCursorPosition.X = safeMod<SHORT>(_abstractCursorPosition.X + moveAmount.X);
		else if (moveAmount.Y != 0)
			_abstractCursorPosition.Y = safeMod<SHORT>(_abstractCursorPosition.Y + moveAmount.Y);
	}

	int GetSum(int data)
	{
		int s{ 0 };
		for (size_t y = 0; y < ARRAY_SIZE; ++y)
		{
			for (size_t x = 0; x < ARRAY_SIZE; ++x)
			{
				int currentElement{ _map[y][x] };
				if (currentElement < 10 && _numberSelected[currentElement])
				{
					s += currentElement;
				}
			}
		}
		return (s == 0) ? data : s;
	}
	void KeyboardEvent()
	{
		if (_cm.isKeyPressed(RegisteredKeys::Enter))
		{
			int coordIdx{ _abstractCursorPosition.Y * ARRAY_SIZE + _abstractCursorPosition.X };
			if (_numMemory.find(coordIdx) != _numMemory.end())
			{
				_map[_abstractCursorPosition.Y][_abstractCursorPosition.X] = _numMemory[coordIdx];
				_numMemory.erase(coordIdx);
			}
			else
			{
				int originalNumber{ _map[_abstractCursorPosition.Y][_abstractCursorPosition.X] };
				_map[_abstractCursorPosition.Y][_abstractCursorPosition.X] = GetSum(_map[_abstractCursorPosition.Y][_abstractCursorPosition.X]);
				if (_map[_abstractCursorPosition.Y][_abstractCursorPosition.X] != originalNumber)
				{
					_numMemory[coordIdx] = originalNumber;
				}
			}
		}

		if (_cm.isKeyPressed(RegisteredKeys::Left))
		{
			MoveCol(-1);
		}
		if (_cm.isKeyPressed(RegisteredKeys::Up))
		{
			MoveRow(-1);
		}
		if (_cm.isKeyPressed(RegisteredKeys::Right))
		{
			MoveCol(1);
		}
		if (_cm.isKeyPressed(RegisteredKeys::Down))
		{
			MoveRow(1);
		}

		if (_cm.isKeyPressed(RegisteredKeys::Q))
		{
			_qFlag = false;
		}
		if (_cm.isKeyPressed(RegisteredKeys::R))
		{
			Reset();
		}

		int enumStart{ static_cast<int>(RegisteredKeys::Num_0) };
		int enumEnd{ static_cast<int>(RegisteredKeys::Num_9) };
		for (int i = enumStart; i <= enumEnd; ++i)
		{
			bool numKeyPressed{ _cm.isKeyPressed(static_cast<RegisteredKeys>(i)) }, numPadKeyPressed{ _cm.isKeyPressed(static_cast<RegisteredKeys>(i + '0')) };
			if (numKeyPressed || numPadKeyPressed)
			{
				_numberSelected[i - '0'] = (_numberSelected[i - '0']) ? false : true;
			}
		}
		if (_cm.isKeyPressed(RegisteredKeys::W))
		{
			MoveCursor(COORD{ 0,-1 });
		}
		if (_cm.isKeyPressed(RegisteredKeys::A))
		{
			MoveCursor(COORD{ -1,0 });
		}
		if (_cm.isKeyPressed(RegisteredKeys::S))
		{
			MoveCursor(COORD{ 0,1 });
		}
		if (_cm.isKeyPressed(RegisteredKeys::D))
		{
			MoveCursor(COORD{ 1,0 });
		}
	}
private:
	Matrix _map;
	bool _qFlag;
	bool _sumMode;
	std::array<bool, ARRAY_SIZE> _numberSelected;
	COORD _abstractCursorPosition;
	COORD _cursorPosition;
	std::uniform_int_distribution<int> _uid;
	// 첫번째 int : COORD의 int 변환값
	// 두번째 int : _map[COORD]가 가리키는 값
	std::unordered_map<int, int> _numMemory;
	std::random_device _rd;
	std::stack<std::pair<const char*,DWORD>> _errorLog; 
	std::vector<std::vector<CHAR_INFO>> _charInfos;
	ConsoleManager _cm;
};
#pragma endregion Program_Decl
int main()
{
	Program p{};
	p.Execute();
}
