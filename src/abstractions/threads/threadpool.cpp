#include "abstractions/threads/threadpool.h"

#include <abstractions/errors.h>

namespace abstractions
{




// ThreadPool::ThreadPool(std::optional<int> num_threads)
// {
//     const int available_threads = std::thread::hardware_concurrency();
//     const int default_max_threads = std::max(1, static_cast<int>(0.75 * available_threads));
//     const int requested_threads = num_threads.value_or(default_max_threads);
//     abstractions_assert(requested_threads > 1);
// }

}
