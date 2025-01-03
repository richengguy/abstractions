#include "abstractions/threads/job.h"

#include <abstractions/errors.h>
#include <abstractions/profile.h>

namespace abstractions::threads {

JobContext::JobContext(int job_id, int worker_id, std::any &data) :
    _job_id{job_id},
    _worker_id{worker_id},
    _data{data} {}

std::any &JobContext::Data() {
    AssertHasData();
    return _data;
}

void JobContext::AssertHasData() const
{
    abstractions_assert(_data.has_value());
}

bool JobContext::HasData() const {
    return _data.has_value();
}

Job::Job(int id, std::unique_ptr<IJobFunction> fn) :
    _id{id},
    _fn{std::move(fn)},
    _payload{std::make_unique<std::any>()} {}

Job::Job(int id, std::any payload, std::unique_ptr<IJobFunction> fn) :
    _id{id},
    _fn{std::move(fn)},
    _payload{std::make_unique<std::any>(payload)} { }

JobStatus Job::Run(int worker_id) {
    abstractions_assert(_fn != nullptr);
    JobContext ctx(_id, worker_id, *_payload);
    Error error;

    OperationTiming timer;
    {
        Profile op_profiler{timer};
        error = (*_fn)(ctx);
    }

    JobStatus status{
        .job_id = _id,
        .error = error,
        .time = timer.GetTiming().total,
    };

    _job_status.set_value(status);
    return status;
}

void Job::SetPromise(Promise& promise) {
    _job_status = std::move(promise);
}

int Job::Id() const {
    return _id;
}

}  // namespace abstractions::threads
