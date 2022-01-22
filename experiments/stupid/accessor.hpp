#include <functional>

namespace stupid
{
	template<typename T>
	using getter_func = std::function<T const &()>;

	template<typename T>
	using setter_func = std::function<void(T const &)>;

	template<typename T>
	struct Accessor
	{
		void operator = (T const & value) { set(value); }
		operator T() { return get(); }

		const getter_func<T> get = {};
		const setter_func<T> set = {};
	};

	template<typename T>
	struct Getter
	{
		operator T() { return get(); }
		const getter_func<T> get;
	};

	template<typename T>
	struct Setter
	{
		void operator = (T const & value) { set(value); }
		const setter_func<T> set;
	};
}

#define STUPID_IMPORT_ACCESSORS	\
	template<typename T> using Accessor = stupid::Accessor<T>; \
	template<typename T> using Getter = stupid::Getter<T>; \
	template<typename T> using Setter = stupid::Setter<T>