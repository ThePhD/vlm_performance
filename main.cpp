#define NONIUS_RUNNER
#include <nonius.h++>

#include "ordered.hpp"
#include <random>
#include <limits>
#include <vector>
#include <set>
#include <map>
#include <forward_list>

template <typename Cont, std::size_t Iterations>
struct ordered_tests {
	static std::size_t rdseed;

	typedef ordered_container<Cont> list_t;
	typedef typename list_t::value_type value_type;
	typedef std::set<std::size_t> source_list_t;
	typedef std::vector<std::size_t> delete_list_t;
	typedef typename list_t::iterator iterator;
#if PTRDIFF_MAX == INT64_MAX
	typedef std::mt19937_64 default_random_engine;
#else
	typedef std::mt19937 default_random_engine;
#endif // cheap 64 bit vs 32 bit
	typedef std::uniform_int_distribution<std::size_t> distribution;
	typedef typename distribution::param_type distribution_range;

	struct insertion {
		source_list_t values;

		insertion(default_random_engine& re) {
			distribution dist(0, std::numeric_limits<std::size_t>::max());
			// Ensure perfect distribution
			for (std::size_t n = 0; n < Iterations; ++n) {
				std::size_t value = dist(re);
				while (!values.insert(value).second)
					value = dist(re);
			}
		}

		auto operator ()(list_t& list) const {
			iterator x;
			for (const auto& value : values) {
				x = list.insert(value);
			}
			return x;
		}
	};

	struct deletion {
		delete_list_t values;
		default_random_engine& re;
		
		deletion(default_random_engine& re) : re(re) {
			distribution dist(0, std::numeric_limits<std::size_t>::max());
			for (std::size_t n = Iterations; n-- > 0;) {
				values.push_back(dist(re, distribution_range{ 0, n }));
			}
		}

		auto operator ()( list_t& list ) const {
			iterator x;
			for (const auto& idx : values) {
				x = list.erase(idx);
			}
			return x;
		}
	};

	struct insertion_measure {
		void operator ()(nonius::chronometer& meter) const {
			list_t list;
			default_random_engine re(rdseed);
			insertion ifx(re);
			meter.measure([&](int run) {
				return ifx(list);
			});
		}
	};

	struct deletion_measure {
		void operator () ( nonius::chronometer& meter ) const {
			default_random_engine re(rdseed);
			insertion ifx(re);
			// Generate an insertion list that's identical using the given random engine seed
			list_t list;
			ifx(list);
			// Duplicate it for the number of runs we're going to do
			std::vector<list_t> runlists(meter.runs(), list);
			std::vector<deletion> deletions;
			for (int i = 0; i < meter.runs(); ++i) {
				deletions.emplace_back(re);
			}
			meter.measure([&](int run) {
				auto& dfx = deletions[run];
				return dfx(runlists[run]);
			});
		}
	};

};

#ifdef VLM_FIXED_SEED
template <typename T, std::size_t N>
std::size_t ordered_tests<T, N>::rdseed = VLM_FIXED_SEED;
#else
template <typename T, std::size_t N>
std::size_t ordered_tests<T, N>::rdseed = std::random_device{}();
#endif // Fixed Seed

#define VLM_xstr(a) VLM_str(a)
#define VLM_str(a) #a

#define INSERTION_BENCHMARK(T, N) NONIUS_BENCHMARK( #T ", insertion - size " VLM_str(N), ordered_tests<T, N>::insertion_measure())

#define DELETION_BENCHMARK(T, N) NONIUS_BENCHMARK( #T ", deletion - size " VLM_str(N), ordered_tests<T, N>::deletion_measure())

#if !defined(VLM_INSERTION) && !defined(VLM_DELETION)
#define VLM_DELETION
#endif // default insertion

#if !defined(VLM_SIZE)
#define VLM_SIZE 1000000
#endif // default size

#ifdef VLM_INSERTION
INSERTION_BENCHMARK(std::vector<std::size_t>, VLM_SIZE);
INSERTION_BENCHMARK(std::list<std::size_t>, VLM_SIZE);
INSERTION_BENCHMARK(std::set<std::size_t>, VLM_SIZE);
#else
DELETION_BENCHMARK(std::vector<std::size_t>, VLM_SIZE);
DELETION_BENCHMARK(std::list<std::size_t>, VLM_SIZE);
DELETION_BENCHMARK(std::set<std::size_t>, VLM_SIZE);
#endif // Insertion or Deletion
