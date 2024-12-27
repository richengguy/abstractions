#pragma once

#include <abstractions/types.h>

#include <future>
#include <memory>
#include <type_traits>

namespace abstractions::threads
{

struct IJobFunction
{
    virtual Error operator()(const int job_id) const = 0;
};

/// @brief Runs a job on some concurrent worker, potentially on a separate thread.
class Job
{
public:

    /// @brief Create a new job.
    /// @tparam T function implementation
    /// @param id job ID
    template<typename T>
    Job(int id)
    {
        static_assert(std::is_base_of<IJobFunction, T>::value, "'T' must inherit from IJobFunction.");
        Job(id, std::make_shared<T>());
    }

    /// @brief Create a new job.
    /// @param id job ID
    /// @param fn function the job executes
    Job(int id, std::shared_ptr<IJobFunction> fn);

    /// @brief Get a future that can be used to determine when the job is complete.
    std::shared_future<Error> GetFuture();

    /// @brief Run the job.
    /// @return A future value with the job's error state.
    void Run();

    /// @brief Set to `true` after the job completes.
    ///
    /// Refer to the value of Error() to see if the job completed successfully.
    bool Complete() const { return _complete; }

    /// @brief Set to the error value a job completes with.
    Error Error() const { return _error; }

    Job(const Job &) = default;
    Job &operator=(const Job &) = default;

private:
    std::promise<::abstractions::Error> _promise;
    std::shared_ptr<IJobFunction> _fn;
    int _id;
    bool _complete;
    ::abstractions::Error _error;
};

}
