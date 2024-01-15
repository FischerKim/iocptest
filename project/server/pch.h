#pragma once

#ifndef		OUT
#define		OUT
#endif

#ifndef		IN
#define		IN
#endif

typedef unsigned char byte;

#include <memory.h>
#include <algorithm>
#include <array>
#include <cassert>
#include <cctype>
#include <concepts>
#include <cstdint>
#include <cwctype>
#include <deque>
#include <exception>
#include <iterator>
#include <list>
#include <memory.h>
#include <queue>
#include <ranges>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <iostream>
#include <fstream>
#include <vector>
#include <mutex>

#define BOOST_BIND_GLOBAL_PLACEHOLDERS

#include <boost/program_options.hpp>
#include <boost/chrono.hpp>
#include <boost/asio.hpp>
#include <boost/process/environment.hpp>
#include <boost/noncopyable.hpp>
#include <boost/format.hpp>
#include <boost/move/unique_ptr.hpp>
#include <boost/make_unique.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/smart_ptr/bad_weak_ptr.hpp>
#include <boost/smart_ptr/atomic_shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/chrono.hpp>
#include <boost/random.hpp>
#include <boost/tokenizer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/make_shared.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext.hpp>
#include <boost/thread.hpp>
#include <boost/atomic.hpp>
#include <boost/pool/pool_alloc.hpp>
#include <boost/pool/object_pool.hpp>
#include <boost/pool/singleton_pool.hpp>
#include <boost/filesystem.hpp>
#include <boost/locale.hpp>
#include <boost/variant.hpp>
#include <boost/any.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/functional/hash.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/json.hpp>
#include <boost/log/trivial.hpp>
#include <boost/optional.hpp>

#include <server.h>