#pragma once

#include <abstractions/types.h>

#include <future>
#include <memory>
#include <type_traits>

namespace abstractions::threads
{

/// @brief Defines what a Job actually does.
struct IJobFunction
{
    virtual Error operator()(const int job_id) const = 0;
    virtual ~IJobFunction() = default;
};

/// @brief Runs a job on some concurrent worker, potentially on a separate thread.
class Job
{
public:

    /// @brief Create a new job.
    /// @tparam T function implementation
    /// @param id job ID
    template<typename T>
    static Job New(int id)
    {
        static_assert(std::is_base_of<IJobFunction, T>::value, "'T' must inherit from IJobFunction.");
        return Job(id, std::make_shared<T>());
    }

    /// @brief Create a new job.
    /// @param id job ID
    /// @param fn function the job executes
    Job(int id, std::shared_ptr<IJobFunction> fn);

    /// @brief Run the job.
    /// @param promise a promise the job will use to signal when its done, along
    ///     with any errors that may have occurred
    void Run(std::promise<Error> promise);

    /// @brief Job ID.
    int Id() const { return _id; }

    /// @brief Set to `true` after the job completes.
    ///
    /// Refer to the value of Error() to see if the job completed successfully.
    bool Complete() const { return _complete; }

    /// @brief Set to the error value a job completes with.
    Error Error() const { return _error; }

    Job(const Job &) = default;
    Job &operator=(const Job &) = default;

private:
    std::shared_ptr<IJobFunction> _fn;
    int _id;
    bool _complete;
    ::abstractions::Error _error;
};

}
