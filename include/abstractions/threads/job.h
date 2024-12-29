#pragma once

#include <abstractions/types.h>

#include <chrono>
#include <future>
#include <memory>
#include <type_traits>
#include <vector>

namespace abstractions::threads
{

class Job;

/// @brief Defines what a Job actually does.
struct IJobFunction
{
    virtual Error operator()(const int job_id, std::vector<Job> &dependencies) const = 0;
    virtual ~IJobFunction() = default;
};

/// @brief The results of a job.
struct JobResult
{
    /// @brief The ID of the finished job.
    int job_id;

    /// @brief The job's error status.
    Error error;

    /// @brief The length of time the job took.
    std::chrono::microseconds time;

    /// @brief A set of dependent jobs (if any) that should be queued up after
    ///     this job completes successfully.
    std::vector<Job> dependencies;
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
    JobResult Run();

    /// @brief Job ID.
    [[nodiscard]] int Id() const { return _id; }

    /// @brief Set to `true` after the job completes.
    ///
    /// Refer to the value of Error() to see if the job completed successfully.
    [[nodiscard]] bool Finished() const { return _complete; }

    /// @brief Set to the error value a job completes with.
    [[nodiscard]] Error Error() const { return _error; }

    /// @brief The length of time the job took to complete, in microseconds.
    [[nodiscard]] std::chrono::microseconds ExecutionTime() const { return _time; }

    Job(const Job &) = default;
    Job &operator=(const Job &) = default;

private:
    std::shared_ptr<IJobFunction> _fn;
    int _id;
    bool _complete;
    ::abstractions::Error _error;
    std::chrono::microseconds _time;
};

}
