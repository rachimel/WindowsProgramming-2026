module rachimel.AF:SharedStorage;

import <memory>;
import <Windows.h>;
import :Enums;
import :Constants;
import :Traits.Service;
import :Service;
import :Service.Manager;
namespace AF
{
	SharedStorage::SharedStorage()
	{
		void* temporaryBuffer = operator new[](Enums::Total* Constants::CacheLine,
			std::align_val_t{ Constants::CacheLine });
		_services.reset(static_cast<IService*>(temporaryBuffer));
		auto add = reinterpret_cast<uintptr_t>(temporaryBuffer);
		if (add % 64 != 0)
			OutputDebugStringW(L"I hate MSVC\n");
	}
	SharedStorage::~SharedStorage()
	{
	}
}