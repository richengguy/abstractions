#include "abstractions/threads/job.h"

#include <abstractions/errors.h>
#include <abstractions/profile.h>

namespace abstractions::threads
{

JobContext::JobContext(int job_id, int worker_id)
    : _job_id{job_id},
      _worker_id{worker_id}
{

}

void JobContext::EnqueueWork(const Job &job)
{
    _to_queue.push_back(job);
}

Job::Job(int id, std::shared_ptr<IJobFunction> fn)
    : _fn{fn}, _id{id}, _complete{false} { }

JobResult Job::Run(int worker_id)
{
    abstractions_assert(_fn != nullptr);
    JobContext ctx(_id, worker_id);
    OperationTiming timer;
    {
        Profile op_profiler{timer};
        _error = (*_fn)(ctx);
    }
    _complete = true;
    _time = timer.GetTiming().total;
    return JobResult{
        .job_id = _id,
        .error = _error,
        .time = _time,
        .dependencies = std::vector<Job>(ctx.GetEnqueuedWork()),
    };
}

}
