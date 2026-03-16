export module rachimel.AF:SharedStorage;

import <memory>;
import :Enums;
import :Memory.Deleter;
namespace AF
{
	export class SharedStorage
	{
	public:
		SharedStorage();
		~SharedStorage();

		SharedStorage(const SharedStorage&) = delete;
		SharedStorage& operator= (const SharedStorage&) = delete;

		SharedStorage(SharedStorage&&) = delete;
		SharedStorage& operator= (SharedStorage&&) = delete;

		// template <typename ViewType>
		// SharedView<ViewType> *const GetView() const {...}
	private:
		std::unique_ptr<class IService,CustomArenaDeleter> _services;
	};

}