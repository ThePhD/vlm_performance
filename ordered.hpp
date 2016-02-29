#pragma once

#include <functional>
#include <vector>
#include <set>
#include <unordered_set>

namespace detail {
	template<typename T, template<typename...> class Templ>
	struct is_specialization_of : std::false_type { };
	template<typename... T, template<typename...> class Templ>
	struct is_specialization_of<Templ<T...>, Templ> : std::true_type { };

	template <typename T, typename, bool = detail::is_specialization_of<T, std::map>::value || detail::is_specialization_of<T, std::set>::value>
	struct key_compare {
		typedef typename T::key_compare type;
	};

	template <typename T, typename Comp>
	struct key_compare<T, Comp, false> {
		typedef Comp type;
	};

	template <typename T, typename Comp>
	using key_compare_t = typename key_compare<T, Comp>::type;

	template <typename C>
	auto adl_begin(C&& c) {
		using std::begin;
		return begin(c);
	}

	template <typename C>
	auto adl_end(C&& c) {
		using std::end;
		return end(c);
	}

	template <typename C>
	auto adl_cbegin(C&& c) {
		using std::cbegin;
		return cbegin(c);
	}

	template <typename C>
	auto adl_cend(C&& c) {
		using std::cend;
		return cend(c);
	}

	template <typename It, typename Diff>
	auto incr_tagged(std::forward_iterator_tag, It it, Diff n) {
		if (n < 0) throw std::range_error("negative increment to forward iterator");
		for (;n--;) ++it;
		return it;
	}

	template <typename It, typename Diff>
	auto incr_tagged(std::bidirectional_iterator_tag, It it, Diff n) {
		if (n < 0) for (;n++;) --it;
		else for (;n--;) ++it;
		return it;
	}

	template <typename It, typename Diff>
	auto incr_tagged(std::random_access_iterator_tag, It it, Diff n) {
		it += n;
		return it;
	}

	template <typename It, typename Diff>
	auto incr(It it, Diff n) {
		return incr_tagged(std::iterator_traits<It>::iterator_category(), it, n);
	}
}

template <typename Val, typename Predicate = std::less<>, typename Cont = std::vector<Val>>
struct ordered;

template <typename Cont>
using ordered_container = ordered<typename Cont::value_type, detail::key_compare_t<Cont, std::less<>>, Cont>;

template <typename Val, typename Predicate, typename Cont>
struct ordered {
private:
	Cont container;
	Predicate predicate; // TODO: EBCO this member

public:
	typedef typename Cont::value_type key_type;
	typedef typename Cont::value_type value_type;
	typedef typename Cont::reference reference;
	typedef typename Cont::const_reference const_reference;
	typedef typename Cont::pointer pointer;
	typedef typename Cont::const_pointer const_pointer;
	typedef Predicate key_compare;
	typedef Predicate value_compare;
	typedef typename Cont::allocator_type allocator_type;
	typedef typename Cont::size_type size_type;
	typedef typename Cont::difference_type difference_type;
	typedef size_type index_type;
	typedef typename Cont::iterator begin_iterator;
	typedef typename Cont::iterator end_iterator;
	typedef typename Cont::iterator iterator;
	typedef typename Cont::const_iterator const_iterator;

	iterator insert(const value_type& val) {
		if (container.empty()) {
			return container.insert(detail::adl_cend(container), val);
		}
		auto elementgreaterthanorequalto = std::lower_bound(detail::adl_cbegin(container), detail::adl_cend(container), val, std::ref(predicate));
		return container.insert(elementgreaterthanorequalto, val);
	}
	
	iterator insert(value_type&& val) {
		if (container.empty()) {
			return container.insert(detail::adl_cend(container), val);
		}
		auto elementgreaterthanorequalto = std::lower_bound(detail::adl_cbegin(container), detail::adl_cend(container), val, std::ref(predicate));
		return container.insert(elementgreaterthanorequalto, std::move(val));
	}

	template <typename... Tn>
	iterator emplace(Tn&&... argn) {
		if (container.empty()) {
			return container.insert(detail::adl_cend(container), std::forward<Tn>(argn)...);
		}
		auto elementgreaterthanorequalto = std::lower_bound(detail::adl_cbegin(container), detail::adl_cend(container), val, std::ref(predicate));
		return container.emplace(elementgreaterthanorequalto, std::forward<Tn>(argn)...);
	}

	iterator erase(index_type i) {
		auto it = detail::incr(detail::adl_begin(container), i);
		return erase(it);
	}

	iterator erase(const_iterator i) {
		return container.erase(i);
	}

	void clear() {
		container.clear();
	}

	size_type size() const { return container.size(); }

	auto begin() { return detail::adl_begin(container); }
	auto begin() const { return detail::adl_begin(container); }
	auto cbegin() const { return detail::adl_cbegin(container); }
	auto end() { return detail::adl_end(container); }
	auto end() const { return detail::adl_end(container); }
	auto cend() const { return detail::adl_cend(container); }
};

template <typename Val, typename Predicate, typename Allocator>
struct ordered<Val, Predicate, std::set<Val, Predicate, Allocator>> {
private:
	typedef std::set<Val, Predicate, Allocator> Cont;
	Cont container;

public:
	typedef typename Cont::value_type key_type;
	typedef typename Cont::value_type value_type;
	typedef typename Cont::reference reference;
	typedef typename Cont::const_reference const_reference;
	typedef typename Cont::pointer pointer;
	typedef typename Cont::const_pointer const_pointer;
	typedef Predicate key_compare;
	typedef Predicate value_compare;
	typedef typename Cont::allocator_type allocator_type;
	typedef typename Cont::size_type size_type;
	typedef typename Cont::difference_type difference_type;
	typedef size_type index_type;
	typedef typename Cont::iterator begin_iterator;
	typedef typename Cont::iterator end_iterator;
	typedef typename Cont::iterator iterator;
	typedef typename Cont::const_iterator const_iterator;
	typedef typename Cont::reverse_iterator reverse_iterator;
	typedef typename Cont::const_reverse_iterator const_reverse_iterator;

	iterator insert(const value_type& val) {
		auto x = container.insert(val);
		return x.first;
	}

	iterator insert(value_type&& val) {
		auto x = container.insert(std::move(val));
		return x.first;
	}

	template <typename... Tn>
	iterator emplace(Tn&&... argn) {
		auto x = container.emplace(std::forward<Tn>(argn)...);
		return x.first;
	}

	iterator erase(index_type i) {
		auto it = detail::incr(detail::adl_begin(container), i);
		return erase(it);
	}

	iterator erase(const_iterator i) {
		return container.erase(i);
	}

	void clear() {
		container.clear();
	}

	auto begin() { return detail::adl_begin(container); }
	auto begin() const { return detail::adl_begin(container); }
	auto cbegin() const { return detail::adl_cbegin(container); }
	auto end() { return detail::adl_end(container); }
	auto end() const { return detail::adl_end(container); }
	auto cend() const { return detail::adl_cend(container); }
};

