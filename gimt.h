/* Copying and distribution of this file, with or without modification,
 * are permitted in any medium without royalty provided the copyright
 * notice and this notice are preserved.  This file is offered as-is,
 * without any warranty. */

#pragma once
#include <tuple>
#include <functional>

/* Glib-In-Main-Thread
 * This helper allow to run code in the thread of main event loop via g_idle_add(...).
 * This is useful for ex. to call GTK functions from other threads.
 *
 * Sample of usage:
 * 1. Call static functions:
 * void foo1(int x, int& y, int * z) {...}
 * void foo2()
 * {
 * 	...
 * 	int x_var, y_var, z_var;
 * ...
 * 	in_main_thread(foo, x_var, std::ref(y_var), &z_vaz);
 * 	...
 * }
 *
 * 2: Call member functions^
 * void Some::func(int x) {...}
 * void foo3()
 * {
 * 	Some some;
 * 	int x_var;
 * 	in_main_thread(&some::func, this, x_var);
 * }
 *
 * 3: Call lambdas:
 * void foo4()
 * {
 * 	...
 * 	int x_var, y_var;
 * 	...
 *	in_main_thread([x_var](int y) {...}, y_var);
 *	...
 * }
 * */

template<typename F, typename T, size_t ...S >
void apply_tuple_impl(F&& fn, T&& t, std::index_sequence<S...>)
{
	std::invoke(std::forward<F>(fn), std::get<S>(std::forward<T>(t))...);
}

template<typename F, typename ... U>
guint in_main_thread_caller(gpointer user_data)
{
	std::pair<F, std::tuple<U...>> * data = (std::pair<F, std::tuple<U...>>*) user_data;
	std::size_t constexpr size = std::tuple_size<std::tuple<U...>>::value;
	apply_tuple_impl(std::forward<F>(data->first), std::forward<std::tuple<U...>>(data->second), std::make_index_sequence<size>());
	delete data;
	return 0;
}

template<typename F, typename ... U>
void in_main_thread(F&& f, U ... args)
{
	auto data = new std::pair<F, std::tuple<U...>> (std::forward<F>(f), std::tuple<U...>(std::forward<U>(args)...));
	g_idle_add((GSourceFunc)in_main_thread_caller<F, U...>, data);
}
