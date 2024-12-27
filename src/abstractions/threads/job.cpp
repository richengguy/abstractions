#include "abstractions/threads/job.h"

#include <abstractions/errors.h>

namespace abstractions::threads
{

Job::Job(int id, std::shared_ptr<IJobFunction> fn)
    : _fn{fn}, _id{id}, _complete{false} { }

void Job::Run(std::promise<::abstractions::Error> promise)
{
    abstractions_assert(_fn != nullptr);
    _error = (*_fn)(_id);
    _complete = true;
    promise.set_value(_error);
}

}
