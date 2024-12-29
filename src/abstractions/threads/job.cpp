#include "abstractions/threads/job.h"

#include <abstractions/errors.h>
#include <abstractions/profile.h>

namespace abstractions::threads
{

Job::Job(int id, std::shared_ptr<IJobFunction> fn)
    : _fn{fn}, _id{id}, _complete{false} { }

JobResult Job::Run()
{
    abstractions_assert(_fn != nullptr);
    std::vector<Job> dependencies;
    OperationTiming timer;
    {
        Profile op_profiler{timer};
        _error = (*_fn)(_id, dependencies);
    }
    _complete = true;
    _time = timer.GetTiming().total;
    return JobResult{
        .job_id = _id,
        .error = _error,
        .time = _time,
        .dependencies = std::move(dependencies),
    };
}

}
