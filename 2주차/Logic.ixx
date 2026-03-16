export module rachimel.AF:Logic;

namespace AF
{
	export class ILogic
	{
	public:
		ILogic() = default;
		virtual ~ILogic() = default;

		ILogic(const ILogic&) = delete;
		ILogic& operator= (const ILogic&) = delete;

		ILogic(ILogic&&) = delete;
		ILogic& operator= (ILogic&&) = delete;
	};
}