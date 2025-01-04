#include "abstractions/threads/job.h"

#include <abstractions/errors.h>
#include <abstractions/profile.h>

namespace abstractions::threads {

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

}  // namespace abstractions::threads
