#include <vector>
#include <string>
#include <queue>
#include <iostream>
#include <cctype>
#include <unordered_map>
#include <algorithm>

// Input 정의
// 영어 대소문자 + 띄어쓰기 (마지막에는 항상 마침표)
// 마침표는 출력이 아닌, 입력할 때 문장의 마지막에만 위치한다고 가정
// 띄어쓰기 only? 이건 Input으로 볼 수 없음 (무조건 대소문자 띄어쓰기)
// 문장의 공백이 처음에 시작하는 것은 띄어쓰기의 정의에 부합하는가?
// ex) " ." 또는 " a  b."
class Word
{
public:
	Word() : _memory{}, _blank{ 0 }
	{

	}
	~Word() {}
	Word(const std::string& s, int blank) : _memory{s}, _blank{blank}
	{
	}
	Word(const Word& other) : _memory{ other._memory }, _blank{ other._blank }
	{
		
	}

	Word& operator= (const Word& other) noexcept
	{
		if (this == &other) return *this;
		_memory = other._memory;
		_blank = other._blank;
		return *this;
	}

	std::string pure_word() const
	{
		return _memory;
	}

	const int blank_length() const
	{
		return _blank;
	}
	const int total_size() const
	{
		return static_cast<int>(_memory.size()) + _blank;
	}
	std::string Out() const
	{
		std::string outBuffer{_memory};
		for (int i = 0; i < _blank; ++i)
			outBuffer += ' ';
		return outBuffer; 
	}

	void changeUpLow(char s)
	{
		for (auto& c : _memory)
		{
			if (c == s || (s - c)== 32)
				c = (islower(c)) ? toupper(c) : tolower(c);
		}
	}
	
	void string_pop_back(int pop_back_amount)
	{
		if   (pop_back_amount <= 0) return;
		for (int i = 0;  i < pop_back_amount; ++i)
		{
			_memory.pop_back();
		}
	}
	int  blank_resize(int resize_amount)
	{
		_blank += resize_amount;
		if (_blank > 5) { _blank = 5;  return 0; }
		else if (_blank < 0) { _blank = 0; return 0; }
		return resize_amount;
	}

	void clear_blank()
	{
		_blank = 0;
	}

	void FindAlpha(std::unordered_map<char, int>& map)
	{
		for (const char& c : _memory)
		{
			if (map.find(c) != map.end())
			{
				map[c]++;
			}
			else
			{
				map[c] = 1;
			}
		}
	}

	int GetAlphaSize() const
	{
		int alphaSize{0};
		for (const char& c : _memory)
		{
			if (isalpha(c) != 0) ++alphaSize;
		}
		return alphaSize;
	}

	bool operator == (const Word& other)
	{
		return _memory == other._memory && _blank == other._blank;
	}
	bool operator != (const Word& other)
	{
		return !(operator==(other));
	}
private:
	std::string _memory;
	int _blank;
};


void GenericOut(const std::vector<Word>& words)
{
	std::string OutputString{};
	for (const auto& w : words)
	{
		OutputString += w.Out();
	}
	if (OutputString.size() > 39)
		OutputString = OutputString.substr(0, 39);
	std::cout << OutputString  << "." << '\n';
}
// 공백이 오는 문자열?
// 
std::vector<Word> Decode(const std::string& s)
{
	std::vector<Word> v{};
	size_t offset_idx{ 0 }, blank_idx{ 0 };
	while ((blank_idx = s.find(" ",offset_idx)) != std::string::npos)
	{
		int blankCount{ 0 };
		while (true)
		{
			if (s[blank_idx] == ' ')
			{
				blankCount++;
				blank_idx++;
			}
			else
				break;
		}
		Word word{ s.substr(offset_idx,blank_idx - blankCount - offset_idx),blankCount };
		offset_idx = blank_idx;
		v.push_back(word);
	}
	size_t last_word_idx;
	last_word_idx = s.find_last_of(" ") + 1;
	v.push_back(Word{ s.substr(last_word_idx, s.size() - last_word_idx - 1),0 });

	return v;
}

enum class InputCheckCondition
{
	NoWord = 0,
	Overflow,
	Blank_Front,
	Blank_Back,
	Period_Only,
	Blank_Over_5,
	NotAlphabet,
	NotEndsWithPeriod,
	End
};
bool input_sentence_check(const std::string& sentence)
{
	// 프로시저 콜 줄이기 위해 변수 선언
	size_t sentence_length{ sentence.size() };
	std::unordered_map<InputCheckCondition, bool> condition{};
	// edge-case 검사
	// 1) 만들 수 있는 단어가 없거나, 문장이 비어있다면 퇴장
	if (sentence_length < 2) {
		if (sentence_length == 0) {
			std::cout << "[Error] The Sentence is Empty!" << '\n';
			return false;
		}
		std::cout << "[Error] The Sentence must to have at least two character" << '\n';
		condition[InputCheckCondition::NoWord] = true;
	}
	// 2) 문장이 마침표 포함 40개 초과
	if (sentence_length > 40) {
		std::cout << "[Error] The Sentence must be lesser than 40 length." << '\n';
		condition[InputCheckCondition::Overflow] = true;
	}
	// 3) 공백이 시작 되는 문장은 문장을 아니라고 보고 퇴장 (제안 문자 제시 가능)
	if (sentence.front() == ' ')
	{ 
		std::cout << "[Error] First word must be started without blank." << '\n';
		condition[InputCheckCondition::Blank_Front] = true;
	}
	// 4) 공백 뒤 마침표는 문장이라고 볼 수 없으므로 퇴장 (제안 문자 제시 가능)
	if (sentence.size() > 1 && *(sentence.end() - 2) == ' ')
	{
		std::cout << "[Error] Front of Period cannot be blank." << '\n';
		condition[InputCheckCondition::Blank_Back] = true;
	}
	// 5) 문장이 알파벳으로 구성되어 있는지 체크 (마침표 제외)
	// 공백이 5개 넘어가는지도 체크 (제안 문자 가능)
	// + 문장에 공백만 있다면 이것도 edge이므로 따로 체크
	size_t blankCount{ 0 };
	for (size_t i = 0; i < sentence_length - 1; ++i)
	{
		if ((sentence[i] < 'A' || sentence[i] > 'z') && sentence[i] != ' ')
		{
			std::cout << "[Error] Word must only contain alphabet." << '\n'; 
			condition[InputCheckCondition::NotAlphabet] = true;
			break;
		}
		else if (sentence[i] == ' ')
			blankCount++;
		else if (isalnum(sentence[i])) { blankCount = 0; }
		if (blankCount > 5)
		{
			std::cout << "[Error] Blank must not exceed 5." << '\n';
			condition[InputCheckCondition::Blank_Over_5] = true;
			break;
		}
	}
	//
	if (std::count(sentence.begin(), sentence.end(),' ') == sentence.size() - 1)
	{
		std::cout << "[Error] Single Period Cannot be a Word." << '\n';
		condition[InputCheckCondition::Period_Only] = true;
	}
	if (std::count(sentence.begin(), sentence.end(), ' ') == sentence.size())
	{
		std::cout << "[Error] The Sentence is Empty!" << '\n';
		return false;
	}
	// 6) 문장의 끝이 마침표인지 체크 (제안 문자 제시 가능)
	if (sentence.back() != '.')
	{
		std::cout << "[Error] Sentence must be ended with period character." << '\n';
		condition[InputCheckCondition::NotEndsWithPeriod] = true;
	}

	if (condition.empty()) return true;


	// 제안 문자를 제시 할 수 있는 문장이라면
	if((condition.find(InputCheckCondition::NoWord) == condition.end()) && 
		(condition.find(InputCheckCondition::Overflow) == condition.end()) &&
		(condition.find(InputCheckCondition::NotAlphabet) == condition.end()) &&
		(condition.find(InputCheckCondition::Period_Only) == condition.end()))
	{
		// 제안문자를 제시하기
		std::string intended_string{sentence};

		// 문장 앞에 공백이 있었다면
		if (condition.find(InputCheckCondition::Blank_Front) != condition.end())
		{
			size_t blank_erase_length{ 0 };
			while (intended_string[blank_erase_length] == ' ')
			{
			blank_erase_length++;
			}
			intended_string = intended_string.substr(blank_erase_length, sentence_length - blank_erase_length);
		}

		if (intended_string.size() > 1 && condition.find(InputCheckCondition::Blank_Back) != condition.end())
		{
			// period도 지우기
			intended_string.pop_back();
			while (intended_string[intended_string.size() - 1] == ' ')
			{
				intended_string.pop_back();
			}
			intended_string.push_back('.');
		}

		if (condition.find(InputCheckCondition::Blank_Over_5) != condition.end())
		{
			std::vector<Word> words = Decode(intended_string);
			std::string buffer{};
			for (auto& word : words)
			{
				int blank_length{ word.blank_length() };
				if (blank_length > 5)
				{
					word.blank_resize((-1) * (blank_length - 5));
				}
				buffer += word.Out();
			}
			intended_string = buffer;
		}
		if (condition.find(InputCheckCondition::NotEndsWithPeriod) != condition.end())
		{
			intended_string.back() = '.';
		}
		std::cout << "[Suggestion] Did you intend to type : \"" << intended_string << "\"?\n";
	}
	return false;
}
int main(void)
{	
	std::string sentence{};
	while (true)
	{
		std::cout << " = Enter The Sentence = " << '\n'
			<< "[Sentence Rules]" << '\n'
			<< "1. The Sentence must to have at least two character (i.e. \"s.\",\"hello world.\")" << '\n'
			<< "2. The Sentence must be lesser than 40 length. (including period)" << '\n'
			<< "3. First word cannot have blank. (\" s.\" is rejected since there's blank character after \'s\'.)" << '\n'
			<< "4. character which at in front of Period cannot be blank. (\"s .\" is rejected becuase of the space between \'s\' and \'.\'.)" << '\n'
			<< "5. Word must only contain alphabet. (except for period located at the last word.)" << '\n'
			<< "6. All Blanks' length must be less than or equal 5." << '\n'
			<< "7.  Single Period Cannot be a Word. (\"    .\" cannot be a word.) " << '\n'
			<< "8. Sentence must be ended with period character." << '\n';
		std::cout << "Enter the Sentence : ";
		std::getline(std::cin, sentence);
		if (input_sentence_check(sentence)) { break; }

		system("timeout -1");
		system("cls");
	}

	// 공백 5개 초과 문장 : edge
	// 공백이 먼저 오는 문장 : edge

	std::vector<Word> v = Decode(sentence);

	char command[2];
	bool alpha_count_mode{ false }, word_sort_mode{ false };
	std::cout << "[Available Commands]" << '\n';
	std::cout << "a-z : converts matching Letter cases into opposite cases." << '\n';
	std::cout << "1 :  removes blanks between words by 1." << '\n';
	std::cout << "2 :  adds blanks between words by 1." << '\n';
	std::cout << "3 :  counts the character in alphabetical order from lower to upper case." << '\n';
	std::cout << "4 :  orders the sentence in ascending order according to each word's alphabet count." << '\n';
	std::cout << "0 :  exits the program." << '\n';
	while (true)
	{
		std::cout << "Command : ";
		// command < - a'\n'
		std::cin.getline(command,2);
		if (std::cin.fail())
		{
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			std::cout << "Invalid Command!" << '\n';
			std::cout << "[Available Commands]" << '\n';
			std::cout << "a-z : converts matching Letter cases into opposite cases." << '\n';
			std::cout << "1 :  removes blanks between words by 1." << '\n';
			std::cout << "2 :  adds blanks between words by 1." << '\n';
			std::cout << "3 :  counts the character in alphabetical order from lower to upper case." << '\n';
			std::cout << "4 :  orders the sentence in ascending order according to each word's alphabet count." << '\n';
			std::cout << "0 :  exits the program." << '\n';
			continue;
		}
		// a a a a a bb  bb   ccc dd eee fffgg.

		// 3이 켜지면 3을 다시 입력, 4는 4를 다시 입력해야만 다른 커맨드 사용 가능하게 고정

		if ((command[0] >= 'a' && command[0] <= 'z') && ((alpha_count_mode || word_sort_mode) == false))
		{
			for (auto& w : v)
				w.changeUpLow(command[0]);
		}
		else if ((command[0] == '0'))
		{
			break;
		}
		else if ((command[0] == '1') && ((alpha_count_mode || word_sort_mode) == false))
		{
			for (auto& w : v)
			{
				w.blank_resize(-1);
			}
		}
		else if ((command[0] == '2') && ((alpha_count_mode || word_sort_mode) == false))
		{
			int blank_increase{ 0 };
			int total_size{ 0 };
			size_t word_size{ v.size() };
			for (size_t i = 0; i < word_size; ++i)
			{
				if(i !=word_size  - 1)
					blank_increase += v[i].blank_resize(1);
				total_size += v[i].total_size();
			}
		}
		// 3번 커맨드 실행
		else if (command[0] == '3' && (word_sort_mode == false))
		{
			if (alpha_count_mode)
			{
				alpha_count_mode = false;
			}
			else
			{
				std::unordered_map<char, int> charMap{};
				for (auto& w : v)
				{
					w.FindAlpha(charMap);
				}
				// unordered_map -> vector -> sort -> vector-> unordered_map 보다 큰 스트링 할당 후 navie compare가 효율적
				std::string s{ "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ" };
				for (const char& c : s)
				{
					if (charMap.find(c) == charMap.end())
						continue;
					else
						std::cout << c << charMap[c];
				}
				std::cout << ".\n";
				alpha_count_mode = true;
				continue;
			}
		}
		// 4번 커맨드 (알파벳 갯수에 따라 정렬)
		else if (command[0] == '4' && (alpha_count_mode == false))
		{
			if (word_sort_mode)
			{
				word_sort_mode = false;
			}
			else
			{
				std::vector<Word> orderedWords{v};
				// move-assignment 처리시 오류
				std::sort(orderedWords.begin(), orderedWords.end(), [](const Word& w1, const Word& w2) {return w1.GetAlphaSize() < w2.GetAlphaSize(); });

				std::string OutputString{};
				size_t printSize{ orderedWords.size() - 1 };
				for (size_t i = 0; i < printSize; ++i)
				{
					std::cout << orderedWords[i].pure_word() << " ";
				}
				std::cout << orderedWords.back().pure_word() << ".\n";
				word_sort_mode = true;
				continue;
			}
		}
		// 비정상 입력 값 처리
		else
		{
			// 모드 고정 처리
			if ((alpha_count_mode || word_sort_mode) == true)
			{
				std::string modeString = (alpha_count_mode) ? "Command '3'" : "Command '4' ";
				std::cout << modeString << " mode is currently activated. Use " << modeString << " to disable the mode or input '0' to exit program." << '\n';
				continue;
			}
			std::cout << "Invalid Command!" << '\n';
			std::cout << "[Available Commands]" << '\n';
			std::cout << "a-z : converts matching Letter cases into opposite cases." << '\n';
			std::cout << "1 :  removes blanks between words by 1." << '\n';
			std::cout << "2 :  adds blanks between words by 1." << '\n';
			std::cout << "3 :  counts the character in alphabetical order from lower to upper case." << '\n';
			std::cout << "4 :  orders the sentence in ascending order according to each word's alphabet count." << '\n';
			std::cout << "0 :  exits the program." << '\n';
			continue;
		}


		GenericOut(v);
		// command = 0;
	}
}