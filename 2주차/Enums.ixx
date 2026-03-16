export module rachimel.AF:Enums;


namespace AF::Enums
{
	namespace Manager
	{
		enum : int
		{
			Event,
			Object,
			Scene,
			Input,
			FIle,
			End,
		};
		constexpr int BEG{ Event };
		constexpr int END{ End };
	}
	namespace System
	{
		constexpr int BEG{ Manager::END + 1 };
		enum : int
		{
			Physics = BEG,
			Render,
			End
		};
		constexpr int END{ End };
	}
	namespace Device
	{
		constexpr int BEG{ System::END + 1 };
		enum : int
		{
			Graphic = BEG,
			End
		};
		constexpr int END{ End };
	}
	constexpr int Total{ Device::End };
}