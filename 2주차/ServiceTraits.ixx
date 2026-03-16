export module rachimel.AF:Traits.Service;

import <concepts>;
namespace AF::Traits
{
	namespace Manager
	{
		struct ManagerTrait {};
		struct EventTrait : public ManagerTrait  {};
		struct ObjectTrait : public ManagerTrait{};
		struct SceneTrait : public ManagerTrait {};
		struct InputTrait :public ManagerTrait {};
		struct FileTrait :public ManagerTrait {};

		template <typename _Trait>
		concept is_manager_trait = std::derived_from<_Trait, ManagerTrait>;

	}
	namespace System
	{
		struct SystemTrait {};
		struct PhysicsTrait : public SystemTrait {};
		struct RenderTrait {};

		template <typename _Trait>
		concept is_system_trait = std::derived_from<_Trait, SystemTrait>;
	}
	namespace Device
	{
		struct DeviceTrait {};
		struct GraphicTrait : public DeviceTrait {};

		template <typename _Trait>
		concept is_device_trait = std::derived_from<_Trait, DeviceTrait>;
	}
}