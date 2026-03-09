#include <string>
#include <array>
#include <vector>
#include <iostream>
#include <format>
#include <unordered_map>
#include <cwctype>
#include <sstream>
#include <queue>

// 컴파일시 주의사항
// Visual Studio나 다른 텍스트 편집기로 열 때, 인코딩을 UTF-8 with BOM으로 맞추고 실행해야 한글이 나옵니다.
constexpr int SEATS_COL_SIZE{ 10 };
constexpr int SEATS_ROW_SIZE{ 10 };
constexpr int THEATER_SIZE{ 3 };
constexpr int MAX_RESERVATIONS{ 100 - 10};

using Seats = std::array<std::array<int, SEATS_COL_SIZE>,SEATS_ROW_SIZE>;

struct Theater
{
	Theater() : _title{}, _times{ }, _seats{}
	{

	}
	explicit Theater(const std::wstring& title,std::initializer_list<int> timeList) :
		_title{title}, _times{},_seats{}
	{
		for (const auto& time : timeList)
		{
			// 예약 횟수 초기화
			_times[time] = 0;

			Seats seats{};
			for (int y = 0; y < SEATS_ROW_SIZE; ++y)
			{
				for (int x = 0; x < SEATS_COL_SIZE; ++x)
				{
					seats[y][x] = -1;
				}
			}
			_seats[time] = seats;
		}
	}
	~Theater() 
	{
	}
	Theater(const Theater& other) : _title{other._title}, _times{other._times},_seats{other._seats}
	{

	}
	Theater& operator= (const Theater& other)
	{
		if (this == &other) return *this;
		_title = other._title;
		_times = other._times;
		_seats = other._seats;
		return *this;
	}

	Theater(Theater&& other) noexcept : _title{ std::move(other._title) }, _times{std::move(other._times)}, _seats{std::move(other._seats)}
	{

	}

	Theater& operator=(Theater&& other) noexcept
	{
		if (this == &other) return *this;
		_title = std::move(other._title);
		_times = std::move(other._times);
		_seats = std::move(other._seats);
		return *this;
	}

	friend std::wostream& operator<< (std::wostream& out,const Theater& theater)
	{
		out << L"상영 영화 제목 : " << theater._title << L'\n';
		out << L"[상영 시간]" << L'\n';
		for (const auto& [time,unused] : theater._times)
		{
			out << std::format(L"\t{:02}:{:02}\n",  time / 100, time % 100);
		}
		return out;
	}

	// 영화의 모든 상영시간에 대한 좌석 예약 현황
	void PrintSeats()
	{
		std::wcout << L"상영 영화 제목 : " << _title << L'\n';
		for (const auto& [time, seatMap] : _seats)
		{
			std::wcout << std::format(L"[상영 시간 : {:02}:{:02} | 예약 좌석 수 : {}]\n", time / 100, time % 100,_times[time]);
			for (size_t y = 0; y < SEATS_ROW_SIZE; ++y)
			{
				for (size_t x = 0; x < SEATS_COL_SIZE; ++x)
				{
					int seat{ seatMap[y][x] };
					if (seat < 0)
						std::wcout << L"  _ ";
					else
						std::wcout << std::format(L"{:3} ", seat);
				}
				std::wcout << L'\n';
			}
		}
	}

	// 어떠한 상영시간에 대한 좌석 예약 현황
	void PrintSeat(int Time)
	{
		const auto& seatMap = _seats[Time];
		for (size_t y = 0; y < SEATS_ROW_SIZE; ++y)
		{
			for (size_t x = 0; x < SEATS_COL_SIZE; ++x)
			{
				int seat{ seatMap[y][x] };
				if (seat < 0)
					std::wcout << L"  _ ";
				else
					std::wcout << std::format(L"{:3} ", seat);
			}
			std::wcout << L'\n';
		}
	}
	std::wstring _title;
	// key : time
	// val : reserveCount
	std::unordered_map<int,int> _times;
	std::unordered_map<int,Seats> _seats;
};

struct ReservationInfo
{
	ReservationInfo() : _title{}, _time{ }, _seatX{}, _seatY{}, _reserveId{}
	{

	}
	explicit ReservationInfo(const std::wstring& title, int time, int seatX, int seatY, int reserveId) :
		_title{ title }, _time{ time}, _seatX{seatX}, _seatY{seatY}, _reserveId{reserveId}
	{

	}
	~ReservationInfo()
	{
	}
	ReservationInfo(const ReservationInfo& other) : _title{ other._title }, _time{other._time}
		,_seatX{other._seatX},_seatY{other._seatY},_reserveId{other._reserveId}
	{

	}
	ReservationInfo& operator= (const ReservationInfo& other)
	{
		if (this == &other) return *this;
		_title = other._title;
		_time = other._time;
		_seatX = other._seatX;
		_seatY = other._seatY;
		_reserveId = other._reserveId;
		return *this;
	}

	ReservationInfo(ReservationInfo&& other) noexcept : _title{ std::move(other._title) },_time{other._time},
		_seatX{other._seatX}, _seatY{other._seatY}, _reserveId{other._reserveId}
	{
		other._time = 0;
		other._seatX = 0;
		other._seatY = 0;
		other._reserveId = 0;
	}

	ReservationInfo& operator=(ReservationInfo&& other) noexcept
	{
		if (this == &other) return *this;
		_title = std::move(other._title);
		_time = other._time;
		_seatX = other._seatX;
		_seatY = other._seatY;
		_reserveId = other._reserveId;
		other._time = 0;
		other._seatX = 0;
		other._seatY = 0;
		other._reserveId = 0;
		return *this;
	}

	friend std::wostream& operator<< (std::wostream& out, const ReservationInfo& theater)
	{
		out << L"상영 영화 제목 : " << theater._title << L'\n';
		out << std::format(L"상영 시간 : {:02}:{:02}\n", theater._time / 100, theater._time % 100);
		out << std::format(L"좌석 번호 : ({},{})\n",theater._seatX,theater._seatY) << L'\n';
		return out;
	}

	std::wstring _title;
	int _reserveId;
	int _time;
	int _seatX;
	int _seatY;
};

std::array<Theater, THEATER_SIZE> theaters{
	Theater{L"4",{1100, 1300}},
	Theater{L"샤크네이도",{900,2300}},
	Theater{L"콜 오브 듀티 4 : 모던 워페어",{1000,1800}},
};
std::queue<int> availableReservationNumbers{};

void PrintError()
{
	std::wcout << L"[오류] : 명령어가 올바르지 않습니다. \n"
		<< L"[사용 가능한 명령어 목록] \n"
		<< L"\td: 3개 영화관의 정보를 출력합니다. (영화관 번호, 영화 제목, 상영시간)\n"
		<< L"\tp: 모든 영화관의 모든 상영 시간에 대한 좌석 상태를 출력합니다.\n"
		<< L"\tp x: x번 영화관의 모든 상영 시간에 대한 좌석 상태를 출력합니다.\n"
		<< L"\t\tp \"영화 제목\": 영화가 상영되는 영화관의 모든 상영 시간에 대한 좌석 상태를 출력합니다.\n"
		<< L"\t\tp \"숫자\": 0 ~9까지의 숫자를 제목으로 간주하여 p 커맨드를 실행합니다.\n"
		<< L"\tr: 영화를 예약합니다.\n"
		<< L"\tc: 예약을 취소합니다.\n"
		<< L"\th: 모든 영화관의 모든 상영 시간의 예약된 좌석의 비율을 출력합니다.\n"
		<< L"\tq: 프로그램을 종료합니다.\n" << std::endl;
	std::wcin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int GetIndexOfMovie()
{
	std::wstring argument{};
	// 영화관의 좌석 정보 (예약 번호 포함) 출력
	std::getline(std::wcin, argument);

	// 제목 시작의 빈칸과 제목 끝 뒤의 빈칸 제거
	size_t frontBlanks{ 0 };
	while (argument[frontBlanks] == L' ')
	{
		frontBlanks++;
	}
	if (frontBlanks > 0) { argument = argument.substr(frontBlanks, argument.size() - frontBlanks); }
	while (argument.size() > 0 && argument.back() == L' ')
		argument.pop_back();

	if (argument.empty()) {
		return -1;
	}

	bool isNum{ true };
	int Idx{ 0 };
	// 숫자 변환 가능?
	try
	{
		Idx = std::stoi(argument);
	}
	catch (std::exception e)
	{
		isNum = false;
	}

	// 매개변수가 제목이라면 (길이가 1인 문자도 들어가지만, 숫자가 아닌 문자는 밑에서 예외처리 되므로 여기서 하진 않는다.)
	if (!isNum)
	{
		// 3이라는 제목을 가진 영화를 찾으려면
		// p "3"
		// p 3
		if (argument.front() == L'\"' && argument.back() == L'\"') { argument = argument.substr(1, argument.size() - 2); }
		bool find{ false };
		for (size_t i = 0; i < THEATER_SIZE; ++i)
		{
			if (theaters[i]._title == argument)
				return static_cast<int>(i);
		}
		// 없으면 오류 출력
		std::wcout << std::format(L"[오류] : \'{}\'에 해당하는 제목의 영화가 멀티플렉스에서 상영되고 있지 않습니다.\n", argument);
	}

	else
	{
		// 영화관 인덱스 찾기
		if (Idx  >  0  && Idx <= THEATER_SIZE)
			return Idx - 1;
		std::wcout << std::format(L"[오류] : {}은(는) 적절한 숫자가 아닙니다. 1에서 {} 사이의 숫자를 입력해주십시오.\n", Idx, THEATER_SIZE);
	}
	return -2;
}
int main()
{
	std::wcout.imbue(std::locale("korean"));
	std::wcin.imbue(std::locale("korean"));

	wchar_t command{'\0'};
	std::wstring argument{};
	std::vector<ReservationInfo> reservations{};
	while (true)
	{
		std::system("cls");
		std::wcout << L"커맨드를 입력해주세요 : ";
		std::wcin >> command;
		//std::wcin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		if (std::wcin.fail())
		{
			std::wcin.clear();
			PrintError();
		}

		if (command == L'p')
		{
			int movieIdx{ GetIndexOfMovie() };
			if (movieIdx == -1)
			{
				// 만약 비었다면 모든 영화관에 대해 출력
				for (size_t i = 0; i < THEATER_SIZE; ++i)
				{
					std::system("cls");
					std::wcout << std::format(L"=영화관 #{}=\n", i + 1);
					theaters[i].PrintSeats();
					std::system("timeout -1");
				}
				continue; 
			}
			else if (movieIdx >= 0)
				theaters[movieIdx].PrintSeats();
			std::system("timeout -1");
			continue;
		}
		else if (command == L'd')
		{
			// 영화관 정보 출력
			for (size_t i = 0; i < THEATER_SIZE; ++i)
			{
				std::wcout << std::format(L"=영화관 #{}=\n",i + 1) << theaters[i] << L'\n';
			}
		}
		else if (command == L'r')
		{
			// 영화 좌석 예약
			for (size_t i = 0; i < THEATER_SIZE; ++i)
			{
				std::wcout  << std::format(L"=영화관 #{}=\n",i + 1) << theaters[i];
			}
			int movieIdx{-1};
			std::wcin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			while (movieIdx < 0)
			{
				std::wcout << L"영화 제목 또는 영화관 번호를 입력하십시오  : ";
				movieIdx = GetIndexOfMovie();
			}
			std::wstring inputTime{};
			int inputTimeHigh{ 0 }, inputTimeLow{ 0 }, totalInputTime{ 0 };
			auto& movie = theaters[movieIdx];
			while (true)
			{
				std::wcout << L"원하시는 상영 시간대를 입력하십시오 (q 입력으로 취소): (예시 : 09:00)";
				std::wcin >> inputTime;
				if (inputTime.size() == 1 && inputTime == L"q") { break; }
				if (inputTime.size() != 5) { std::wcout << L"올바른 시간 포맷이 아닙니다. \n"; continue; }
				try
				{
					inputTimeHigh = std::stoi(inputTime.substr(0, 2));
					inputTimeLow = std::stoi(inputTime.substr(3, 2));
					totalInputTime = inputTimeHigh * 100 + inputTimeLow;
				}
				catch (std::exception)
				{
					std::wcout << L"올바른 시간 포맷이 아닙니다. \n";
					continue;
				}

				if (movie._times.find(totalInputTime) != movie._times.end())
					break;
				else
					std::wcout << std::format(L"영화가 {}에 시작하지 않습니다.\n",inputTime);
			}
			if (inputTime.size() == 1 && inputTime == L"q") { continue; }
			movie.PrintSeat(totalInputTime);
			std::wstring inputCoord{};
			int SeatX{ 0 }, SeatY{ 0 };
			wchar_t delimiter{ 0 };
			while (true)
			{
				std::wcout << L"원하시는 좌석의 번호를 입력하십시오 (q 입력으로 취소): (예시 | 3,5) \n";
				std::wcin >> inputCoord;
				std::wistringstream stream{ inputCoord };
				stream >> SeatX >> delimiter >> SeatY;
				std::wcin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				if (inputCoord.size() == 1 && inputCoord == L"q")
					break;
				if (SeatX < 1 || SeatX > SEATS_COL_SIZE || SeatY < 1 || SeatY > SEATS_ROW_SIZE)
				{
					std::wcout << std::format(L"좌석 번호의 첫 번째는 1 ~ {}, 두 번째는 1 ~ {}이어야 합니다.\n", SEATS_COL_SIZE, SEATS_ROW_SIZE);
					continue;
				}
				if (delimiter == L',' && (movie._seats[totalInputTime][SeatY - 1][SeatX -1]) == -1)
				{
					int reservationNum{static_cast<int>(reservations.size() + 10)};
					if (!availableReservationNumbers.empty())
					{
						reservationNum = availableReservationNumbers.front();
						availableReservationNumbers.pop();
					}
					else
					{
						if (reservations.size() == MAX_RESERVATIONS)
						{
							std::wcout << std::format(L"예약이 전부 찼습니다.\n", SEATS_COL_SIZE, SEATS_ROW_SIZE);
							break;
						}
						movie._times[totalInputTime]++;
					}
					movie._seats[totalInputTime][SeatY - 1][SeatX - 1] = reservationNum;
					ReservationInfo reserveInfo{ movie._title,totalInputTime,SeatX,SeatY,reservationNum };
					std::wcout << reserveInfo << L'\n';
					std::wcout << std::format(L"예약 번호는 {} 입니다.",reservationNum) << L'\n';
					reservations.push_back(reserveInfo);
					break;
				}
				else if ((movie._seats[totalInputTime][SeatY - 1][SeatX - 1]) != -1)
				{
					std::wcout << L"이미 예약된 좌석입니다. \n";
				}
			}
		}
		else if (command == L'c')
		{
			// 예약 취소
			int reserveId{ 0 };
			std::wcout << L"예약 번호를 입력해주십시오. \n";
			std::wcin >> reserveId;
			if (std::wcin.fail())
			{
				std::wcin.clear();
				std::wcin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				std::wcout << std::format(L"올바른 예약번호가 아닙니다. \n",MAX_RESERVATIONS - 1);
				std::system("timeout -1");
				continue;
			}
			else if (reserveId < 10 || reserveId > MAX_RESERVATIONS - 1)
			{
				std::wcout << std::format(L"예약 번호가 10-{} 사이의 수여야 합니다. \n", MAX_RESERVATIONS - 1);
				std::system("timeout -1");
				continue;
			}
			auto reserveIterator{ std::find_if(reservations.begin(), reservations.end(), [&](const ReservationInfo& info) {return info._reserveId == reserveId; }) };
			if (reserveIterator != reservations.end())
			{
				const auto& reserveInfo = *reserveIterator;
				availableReservationNumbers.push(reserveId);
				size_t i = 0;
				for (i ; i < THEATER_SIZE; ++i)
				{
					if (theaters[i]._title == reserveInfo._title)
						break;
				}
				theaters[i]._seats[reserveInfo._time][reserveInfo._seatY -1][reserveInfo._seatX -1] = -1;
				reservations.erase(reserveIterator);
				std::wcout << std::format(L"예약이 성공적으로 취소되었습니다.\n", MAX_RESERVATIONS - 1);
				std::system("timeout -1");
				continue;
			}
			std::wcout << std::format(L"예약 번호 {}로 예약된 정보가 존재하지 않습니다 \n", reserveId);
		}
		else if (command == L'h')
		{
			// 비율 출력
			for (size_t i = 0; i < THEATER_SIZE; ++i)
			{
				std::wcout << std::format(L"=영화관 #{}=\n", i + 1);
				std::wcout << std::format(L"영화 제목 : {}\n", theaters[i]._title);
				for (const auto& [time, reservationCount] : theaters[i]._times)
				{
					double percent{ reservationCount / static_cast<double>(SEATS_ROW_SIZE * SEATS_COL_SIZE) * 100.0 };
					std::wcout << std::format(L"\t 상영 시간 : {:02}:{:02} | 예약률 : {:.2}%\n", time / 100, time % 100, percent);
				}
			}
		}
		else if (command == L'q')
		{
			std::wcout << L"프로그램을 종료합니다.\n";
			break;
		}
		else
		{
			PrintError();
		}
		std::system("timeout -1");
	}
}