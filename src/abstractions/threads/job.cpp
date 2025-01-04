#include "abstractions/threads/job.h"

#include <abstractions/errors.h>
#include <abstractions/profile.h>

#include <iterator>
#include <thread>
#include <type_traits>

namespace abstractions::threads {

namespace
{

template<typename T>
class Awaiter
{
public:
    Awaiter(const T *future)
        : _future{future}
    { }

    Awaiter(const T &future)
        : _future{&future}
    { }

    bool Ready() const
    {
        // Don't particularly like doing this but don't know of a better way
        // without writing a custom future/promise class given C++'s current
        // level of support.  See https://stackoverflow.com/a/52515669
        std::this_thread::yield();
        return _future->wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
    }

private:
    const T *_future;
};


template<typename I>
void WaitForJobs(I begin, I end)
{
    static_assert(std::forward_iterator<I>, "Expected a container that supports forward iterator.");
    static_assert(std::is_same_v<typename std::iterator_traits<I>::value_type, const Job::Future *> ||
                  std::is_same_v<typename std::iterator_traits<I>::value_type, Job::Future>,
                  "Iterator value type must refer to a Job::Future.");

    auto num_jobs = std::distance(begin, end);
    while (true)
    {
        auto num_finished = static_cast<std::iterator_traits<I>::difference_type>(0);
        for (auto it = begin; it != end; it++)
        {
            Awaiter await_future(*it);
            num_finished += await_future.Ready() ? 1 : 0;
        }

        if (num_finished == num_jobs)
        {
            break;
        }
    }
}

}


JobContext::JobContext(int index, int worker_id, std::any &data) :
    _index{index},
    _worker_id{worker_id},
    _data{data} {}

std::any &JobContext::Data() {
    return _data;
}

Job::Job(int index, std::unique_ptr<IJobFunction> fn) :
    _index{index},
    _fn{std::move(fn)},
    _payload{std::make_unique<std::any>()} {}

Job::Job(int index, std::any payload, std::unique_ptr<IJobFunction> fn) :
    _index{index},
    _fn{std::move(fn)},
    _payload{std::make_unique<std::any>(payload)} { }

JobStatus Job::Run(int worker_id) {
    abstractions_assert(_fn != nullptr);
    JobContext ctx(_index, worker_id, *_payload);
    Error error;

    OperationTiming timer;
    {
        Profile op_profiler{timer};
        error = (*_fn)(ctx);
    }

    JobStatus status{
        .index = _index,
        .error = error,
        .time = timer.GetTiming().total,
    };

    _job_status.set_value(status);
    return status;
}

void Job::SetPromise(Promise& promise) {
    _job_status = std::move(promise);
}

int Job::Index() const {
    return _index;
}

void WaitForJobs(std::initializer_list<const Job::Future *> futures)
{
    WaitForJobs(std::begin(futures), std::end(futures));
}

void WaitForJobs(const std::vector<Job::Future> &futures)
{
    WaitForJobs(std::begin(futures), std::end(futures));
}

}  // namespace abstractions::threads
