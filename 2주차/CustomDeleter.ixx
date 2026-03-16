export module rachimel.AF:Memory.Deleter;
import :Service;
import :Enums;
import <new>;
namespace AF
{
	struct CustomArenaDeleter
	{
		void operator() (IService* service)
		{
			if (!service) return;
			for (int i = Enums::Device::END - 1; i >= 0; --i)
			{
				(service + i)->~IService();
			}
			operator delete[](service, std::align_val_t(64));
		}
	};
}