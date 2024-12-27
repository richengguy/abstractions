#include "abstractions/threads/job.h"

#include <abstractions/errors.h>

namespace abstractions::threads
{

Job::Job(int id, std::shared_ptr<IJobFunction> fn)
    : _fn{fn}, _id{id}, _complete{false} { }

std::shared_future<Error> Job::GetFuture()
{
    return _promise.get_future();
}

void Job::Run()
{
    abstractions_assert(_fn != nullptr);
    _error = (*_fn)(_id);
    _complete = true;
    _promise.set_value(_error);
}

}
