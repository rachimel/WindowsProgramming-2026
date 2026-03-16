export module rachimel.AF:Service;

import <bit>;
import <immintrin.h>;
import :Constants;

namespace AF
{
	export class IService
	{
	public:
		IService() = default;
		virtual ~IService() = default; 

		IService(const IService&) = delete;
		IService& operator= (const IService&) = delete;
		IService(IService&&) = delete;
		IService& operator= (IService&&) = delete;
	};
	export class alignas(Constants::CacheLine) ServiceContainer
	{
	public:
		ServiceContainer(IService* service) : data{}
		{
		}
		template <typename _Service>
		_Service* Cast() { return reinterpret_cast<_Service*>(data); }
	private:
		std::byte data[Constants::CacheLine];
	};
}