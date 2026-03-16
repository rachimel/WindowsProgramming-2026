export module rachimel.AF:Service.Manager;

import <concepts>;

import :Constants;
import :Service;
import :Traits.Service;
namespace AF
{
	export template <Traits::Manager::is_manager_trait _Trait>
	class Manager : public IService
	{
	public:
		Manager() = delete;
		static_assert(false, "Trait is not specialized!");
	};
	export template<>
		class Manager<Traits::Manager::EventTrait > : public IService
	{
		void Update()
		{

		}
	};

	export template<>
	class Manager<Traits::Manager::ObjectTrait> : public IService
	{
		void Update()
		{

		}
	};
}