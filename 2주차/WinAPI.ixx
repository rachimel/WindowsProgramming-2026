export module rachimel.AF:WinAPI;

namespace AF::WinAPI
{
	export enum Msg
	{
		WM_NCCREATE = 0x0081
	};
	namespace GWLP{
		export constexpr int USERDATA{ -21 };
	}
}