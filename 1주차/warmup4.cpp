#include <array>
#include <unordered_map>
#include <random>
#include <Windows.h>
#include <iostream>
#include <queue>
#include <memory>
#include <format>

constexpr int g_arraySize{ 20 };

template <typename T>
concept canModular = requires (T a, T b) { a% b; };
template <canModular T>
T safeMod(T a) {
	return (a % static_cast<T>(g_arraySize) + g_arraySize) % g_arraySize;
}

enum class KeyState
{
	Ready,
	Down,
	Hold,
	Up,
};
enum class TileType
{
	None,
	Wall,
	Item,
	Player,
};

enum class Direction
{
	Left,
	Up,
	Right,
	Down
};
enum class RegisteredKeys
{
	None,
	W = 'W', // 0
	A = 'A', 
	S = 'S',
	D = 'D',

	I = 'I',
	J = 'J',
	K = 'K',
	L = 'L',
	O ='O',

	E = 'E',
	R = 'R',
	Q = 'Q',
	F = 'F'
};
struct Vector2I
{
	Vector2I() : x{0}, y{0} {}
	explicit Vector2I(int x, int y) : x{x}, y{y} {}
	~Vector2I() {}

	Vector2I(const Vector2I& other) : x{ other.x }, y{ other.y } {}
	Vector2I& operator= (const Vector2I& other)
	{
		if (this == &other) return *this;
		x = other.x;
		y = other.y;
		return *this;
	}
	Vector2I(Vector2I&& other) noexcept : x{ other.x }, y{ other.y }
	{
		other.x = 0;
		other.y = 0;
	}
	Vector2I& operator= (Vector2I&& other) noexcept
	{
		if (this == &other) return *this;
		x = other.x;
		y = other.y;
		other.x = 0;
		other.y = 0;
		return *this;
	}
	Vector2I operator+ (const Vector2I& other) { return Vector2I{ x + other.x, y + other.y }; }
	Vector2I& operator+= (const Vector2I& other) { 
		if (other == Vector2I{ 0,0 }) 
			return *this; 
		x += other.x;
		y += other.y;
		return *this;
	}
	Vector2I operator* (const Vector2I& other) { return Vector2I{ x * other.x, y * other.y }; }
	bool operator== (const Vector2I& other) { return x == other.x && y == other.y;}

	int x;
	int y;
};

class Player
{
public:
	explicit Player(const Vector2I& pos, char shape, Direction dir) : _pos{ pos }, _shape{ shape }, _dir{ dir }, _score{ 0 } {}
	~Player() {}
	void Move(const Vector2I& dir)  {
		_pos.x = safeMod(_pos.x + dir.x);
		_pos.y = safeMod(_pos.y + dir.y);
	}
	Vector2I MoveTest(const Vector2I& dir)
	{
		return Vector2I{ safeMod(_pos.x + dir.x) , safeMod(_pos.y + dir.y) };
	}
	const Vector2I GetPos() const {
		return _pos;
	}
	void SetPos(const Vector2I& pos) {
		_pos = pos;
	}
	const char GetShape() const
	{
		return _shape;
	}

	const Direction GetDir() const {
		return _dir;
	}
	void SetDir(const Direction& dir) {
		_dir = dir;
	}
	void AddScore() {
		_score++;
	}
	const int GetScore() const {
		return _score;
	}
private:
	Vector2I _pos;
	Direction _dir;
	char _shape;
	int _score;
};

std::array<Vector2I, 4> _refDir{
	Vector2I{-1,0},
	Vector2I{0,-1},
	Vector2I{1,0},
	Vector2I{0,1}
};

std::array<Vector2I, 3> _walls{
	Vector2I{3,4},
	Vector2I{4,4},
	Vector2I{5,2}
};

class Board
{
public:
	Board(std::weak_ptr<class Program> program) : _map{}, _program{program}
	{

	}
	~Board()
	{

	}
	Board(const Board& other) = delete;
	Board& operator= (const Board& other) = delete;
	Board(Board&&) = delete;
	Board& operator= (Board&&) = delete;

	void Generate(Player& player1,Player& player2,int& items)
	{
		items = 0;
		std::vector<Vector2I> _availablePos{};

		// 가능한 좌표 초기화
		for (int i = 0; i < g_arraySize * g_arraySize; ++i)
		{
			_availablePos.push_back(Vector2I{ i / g_arraySize, i % g_arraySize });
		}

		for(int i = 0; i < 3; ++i)
		{
			Vector2I hitBox{ _walls[i].x , _walls[i].y };

			for (int y = 0; y < g_arraySize - hitBox.y; ++y)
			{
				for (int hx = g_arraySize - hitBox.x; hx < g_arraySize; ++hx)
				{
					if (std::find(_availablePos.begin(), _availablePos.end(), Vector2I{ hx,y }) != _availablePos.end()) {
						_availablePos.erase(std::remove(_availablePos.begin(), _availablePos.end(), Vector2I{ hx,y }));
					}
				}
			}

			for (int x = 0; x < g_arraySize; ++x)
			{
				for (int hy = g_arraySize - hitBox.y; hy < g_arraySize; ++hy)
				{
					if (std::find(_availablePos.begin(), _availablePos.end(), Vector2I{ x,hy }) != _availablePos.end()) {
						_availablePos.erase(std::remove(_availablePos.begin(), _availablePos.end(), Vector2I{ x,hy }));
					}
				}
			}

			std::shuffle(_availablePos.begin(), _availablePos.end(), rd);
			Vector2I spawnLocation = _availablePos.front();
			int p{ 1 };
			size_t availablePosSize{ _availablePos.size() };
			while ((_map[spawnLocation.y * g_arraySize + spawnLocation.x + _walls[i].x] == TileType::Wall ||
				_map[(spawnLocation.y + _walls[i].y) * g_arraySize + spawnLocation.x] == TileType::Wall  ||
				_map[(spawnLocation.y + _walls[i].y) * g_arraySize + spawnLocation.x + _walls[i].x] == TileType::Wall) && p < availablePosSize)
			{
				spawnLocation = _availablePos[++p];
			}


			Vector2I determinedEnd = spawnLocation + _walls[i];
			for (int wy = spawnLocation.y; wy < determinedEnd.y; ++wy)
			{
				for (int wx = spawnLocation.x; wx < determinedEnd.x; ++wx)
				{
					_map[(wy * g_arraySize) + wx] = TileType::Wall;
				}
			}
			if (i == 2) break;
			
		Vector2I nextHitBox{ _walls[i + 1].x , _walls[i + 1].y };

		
		int min_dy = (determinedEnd.y - 7> 0) ? determinedEnd.y - 7 : 0;
		int min_dx = (determinedEnd.x   -7 > 0) ? determinedEnd.x - 7 : 0;
		for (int dy = determinedEnd.y -1; dy >= min_dy; --dy)
		{
			for (int dx = determinedEnd.x -1; dx >= min_dx; --dx)
			{
				if (std::find(_availablePos.begin(), _availablePos.end(), Vector2I{ dx,dy }) != _availablePos.end()) {
					_availablePos.erase(std::remove(_availablePos.begin(), _availablePos.end(), Vector2I{ dx,dy }));
				}
			}
		}


		}
		std::uniform_int_distribution<int> uid{ 20,36 };
		int itemSpawnCount = uid(rd);
		for (int i = 0; i < g_arraySize * g_arraySize; ++i)
		{
			if (_map[i] != TileType::Wall)
			{
				itemSpawnCount--;
				if (itemSpawnCount == 0)
				{
					_map[i] = TileType::Item;
					itemSpawnCount = uid(rd);
					items++;
				}
			}

		}


		std::shuffle(_availablePos.begin(), _availablePos.end(), rd);
		Vector2I vector{ _availablePos.back() };
		player1.SetPos(vector);
		_availablePos.pop_back();
		std::shuffle(_availablePos.begin(), _availablePos.end(), rd);
		vector = _availablePos.front();
		player2.SetPos(vector);
}


	void Render(const Player& player,const Player& otherPlayer, Direction playerDir)
	{
		int refVecIdx{ static_cast<int>(playerDir) };
		Vector2I povStartDirection{ _renderRefVec2[refVecIdx] };
		Vector2I povEndDirection{ _renderRefVec2[(refVecIdx+ 1) % 4] };
		Vector2I playerDirVec{ _refDir[static_cast<int>(playerDir)] };
		std::queue<Vector2I> renderQueue;
		renderQueue.push(player.GetPos());
		Vector2I renderRay{povStartDirection + player.GetPos()};
		

		while (0 <= renderRay.x && renderRay.x < g_arraySize &&
			0 <= renderRay.y && renderRay.y < g_arraySize)
		{
			if (_map[renderRay.y * g_arraySize + renderRay.x] == TileType::Wall)
				break;
			else if (_map[renderRay.y * g_arraySize + renderRay.x] == TileType::Player)
				continue;
			renderQueue.push({ renderRay });
			renderRay += povStartDirection;
		}
		renderRay = povEndDirection + player.GetPos();
		while (0 <= renderRay.x && renderRay.x < g_arraySize &&
			0 <= renderRay.y && renderRay.y < g_arraySize)
		{
			if (_map[renderRay.y * g_arraySize + renderRay.x] == TileType::Wall)
				break;
			else if (_map[renderRay.y * g_arraySize + renderRay.x] == TileType::Player)
				continue;
			renderQueue.push({ renderRay });
			renderRay += povEndDirection;
		}

		while (!renderQueue.empty())
		{
			Vector2I directionRay{ renderQueue.front() };
			renderQueue.pop();
			while (0 <= directionRay.x && directionRay.x < g_arraySize &&
				0 <= directionRay.y && directionRay.y < g_arraySize)
			{
				std::cout << std::format("\x1b[{};{}H", directionRay.y + 1, 2 * directionRay.x + 1);
				if (directionRay == player.GetPos())
				{
					std::cout << player.GetShape();
				}
				else if (directionRay == otherPlayer.GetPos())
				{
					directionRay += playerDirVec;
					continue;
				}
				else if (_map[directionRay.y * g_arraySize + directionRay.x] == TileType::Wall)
				{
					std::cout << "■";
					break;
				}
				else if (_map[directionRay.y * g_arraySize + directionRay.x] == TileType::None)
				{
					std::cout << "□";
				}
				else if (_map[directionRay.y * g_arraySize + directionRay.x] == TileType::Item)
				{
					std::cout << "*";
				}
				directionRay += playerDirVec;
			}
		}
	}

	// Player1 고정
	void Render(const Player& player1, const Player& player2)
	{
		for (int i = 0; i < g_arraySize * g_arraySize; ++i)
		{
			if (player1.GetPos().y * g_arraySize + player1.GetPos().x == i)
			{
				std::cout << "#";
			}
			else if (player2.GetPos().y * g_arraySize + player2.GetPos().x == i)
			{
				std::cout << "@";
			}
			else if (_map[i] == TileType::Wall)
			{
				std::cout << "■";
			}
			else if (_map[i] == TileType::None)
			{
				std::cout << "□";
			}
			else if (_map[i] == TileType::Item)
			{
				std::cout << "*";
			}

			std::cout << " ";
			if ( i % g_arraySize == g_arraySize - 1)
			{
				std::cout << "\n";
			}
		}
	}

	TileType GetCollidingType(const Vector2I& pos)
	{
		return _map[pos.y * g_arraySize + pos.x];
	}

	void SwapTile(const Vector2I& pos1, const Vector2I& pos2)
	{
		std::swap(_map[pos1.y * g_arraySize + pos1.x], _map[pos2.y * g_arraySize + pos2.x]);
	}
	void SetTile(const Vector2I& pos, TileType type)
	{
		_map[pos.y * g_arraySize + pos.x] = type;
	}
private:
	// Dependency Injection
	std::weak_ptr<class Program> _program;
	std::array<TileType, g_arraySize* g_arraySize> _map;
	std::array<Vector2I, 4> _renderRefVec2{
		Vector2I{-1,1},
		Vector2I{-1,-1},
		Vector2I{1,-1},
		Vector2I{1,1}
	};
	std::random_device rd{};
};
class Program : public std::enable_shared_from_this<Program>
{
public:
	Program() : _player1{ Player{Vector2I{0,0}, '#',Direction::Down} }, _player2{ Player{Vector2I{0,0}, '@',Direction::Down} }, _items{ 0 } {
		
	}
	~Program() {}

	Program(const Program&) = delete;
	Program& operator= (const Program&) = delete;

	Program(Program&&) = delete;
	Program& operator= (Program&&) = delete;

	void Init()
	{
		_board = std::make_shared<Board>(weak_from_this());
		_board->Generate(_player1, _player2, _items);
	}
	void Reset()
	{
		_board->Generate(_player1, _player2,_items);
	}
	void Execute()
	{
		Init();
		bool delta{ true };
		bool player1Turn{ true };
		while (!(GetAsyncKeyState('Q') & 0x8000) && _items > 0)
		{
			if(delta)
			{
				system("cls");
				if(isFogged)
				{
					_board->Render(_player1,_player2, _player1.GetDir());
					_board->Render(_player2,_player1,_player2.GetDir());
				}
				else
				{
					_board->Render(_player1,_player2);
				}
				std::cout << std::format("\x1b[{};{}H", 21, 0);
				std::cout << std::format("플레이어 1 점수 : {} | 플레이어 2 점수 : {}\n", _player1.GetScore(), _player2.GetScore());
				delta = false;
			}
			delta = KeyEventCheck(player1Turn);
		}
		std::cout << std::format("플레이어 1 취종 점수 : {} | 플레이어 2 최종 점수 : {}", _player1.GetScore(), _player2.GetScore());
		if(_items == 0){
			std::cout << "게임 결과" << '\n';
			if (_player1.GetScore() > _player2.GetScore())
				std::cout << "플레이어 1의 승리!" << '\n';
			else if (_player2.GetScore() > _player1.GetScore())
				std::cout << "플레이어 2의 승리!" << '\n';
			else
				std::cout << "무승부" << '\n';
		}

			std::cout << "프로그램을 종료합니다 : " << '\n';
		std::system("timeout -1");
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
	bool isKeyHold(RegisteredKeys key)
	{
		UpdateKeyStatus(key);
		return(_keyboardStatus[key] == KeyState::Hold);
	}
	bool KeyEventCheck(bool& player1Turn)
	{
		Player& playerRef = (player1Turn) ? _player1 : _player2;
		std::unordered_map<char, Vector2I> keyMap{
			{'W',Vector2I{0,-1}},
			{'A',Vector2I{-1,0}},
			{'S',Vector2I{0,1}},
			{'D',Vector2I{1,0}},
			{'I',Vector2I{0,-1}},
			{'J',Vector2I{-1,0}},
			{'K',Vector2I{0,1}},
			{'L',Vector2I{1,0}},
		};
		if (isKeyHold(RegisteredKeys::E) && player1Turn)
		{
			if (isKeyPressed(RegisteredKeys::W))
			{
				playerRef.SetDir(Direction::Up);
				return true;
			}
			else if (isKeyPressed(RegisteredKeys::A))
			{
				playerRef.SetDir(Direction::Left);
				return true;
			}
			else if (isKeyPressed(RegisteredKeys::S))
			{
				playerRef.SetDir(Direction::Down);
				return true;
			}
			else if (isKeyPressed(RegisteredKeys::D))
			{
				playerRef.SetDir(Direction::Right);
				return true;
			}
		}
		if (!isKeyHold(RegisteredKeys::E) && player1Turn)
		{
			RegisteredKeys inputKey = RegisteredKeys::None;
			if (isKeyPressed(RegisteredKeys::W))
				inputKey = RegisteredKeys::W;
			else if (isKeyPressed(RegisteredKeys::A))
				inputKey = RegisteredKeys::A;
			else if (isKeyPressed(RegisteredKeys::S))
				inputKey = RegisteredKeys::S;
			else if (isKeyPressed(RegisteredKeys::D))
				inputKey = RegisteredKeys::D;

			if (inputKey != RegisteredKeys::None)
			{
				Vector2I nextPos{ playerRef.MoveTest(keyMap[static_cast<char>(inputKey)] )};
				if (_board->GetCollidingType(nextPos) == TileType::Wall)
					return false;
				else if (_board->GetCollidingType(nextPos) == TileType::Item)
				{
					playerRef.SetPos(nextPos);
					_board->SetTile(playerRef.GetPos(), TileType::Wall);
					playerRef.AddScore();
					player1Turn = false;
					--_items;
					return true;
				}
				playerRef.SetPos(nextPos);
				player1Turn = false;
				return true;
			}
		}
		if (isKeyHold(RegisteredKeys::O) && !player1Turn)
		{
			if (isKeyPressed(RegisteredKeys::I))
			{
				playerRef.SetDir(Direction::Up);
				return true;
			}
			else if (isKeyPressed(RegisteredKeys::J))
			{
				playerRef.SetDir(Direction::Left);
				return true;
			}
			else if (isKeyPressed(RegisteredKeys::K))
			{
				playerRef.SetDir(Direction::Down);
				return true;
			}
			else if (isKeyPressed(RegisteredKeys::L))
			{
				playerRef.SetDir(Direction::Right);
				return true;
			}
		}
		if (!isKeyHold(RegisteredKeys::O) && !player1Turn)
		{
			RegisteredKeys inputKey = RegisteredKeys::None;
			if (isKeyPressed(RegisteredKeys::I))
				inputKey = RegisteredKeys::I;
			else if (isKeyPressed(RegisteredKeys::J))
				inputKey = RegisteredKeys::J;
			else if (isKeyPressed(RegisteredKeys::K))
				inputKey = RegisteredKeys::K;
			else if (isKeyPressed(RegisteredKeys::L))
				inputKey = RegisteredKeys::L;

			if (inputKey != RegisteredKeys::None)
			{
				Vector2I nextPos{ playerRef.MoveTest(keyMap[static_cast<char>(inputKey)]) };
				if (_board->GetCollidingType(nextPos) == TileType::Wall)
					return false;
				else if (_board->GetCollidingType(nextPos) == TileType::Item)
				{
					playerRef.SetPos(nextPos);
					_board->SetTile(playerRef.GetPos(), TileType::Wall);
					playerRef.AddScore();
					--_items;
					player1Turn = true;
					return true;
				}
				playerRef.SetPos(nextPos);
				player1Turn = true;
				return true;
			}
		}
		if (isKeyPressed(RegisteredKeys::R))
		{
			Init();
			return true;
		}
		if (isKeyPressed(RegisteredKeys::F))
		{
			isFogged = (isFogged) ? false : true;
			return true;
		}
		return false;
	}
private:
	std::shared_ptr<Board> _board;
	std::unordered_map<RegisteredKeys, KeyState> _keyboardStatus;
	Player _player1;
	Player _player2;
	bool isFogged{ false };
	int _items;
};

int main()
{
	std::shared_ptr<Program> p{std::make_shared<Program>()};
	p->Execute();
}