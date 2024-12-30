#pragma once

#include <abstractions/types.h>

#include <chrono>
#include <future>
#include <memory>
#include <type_traits>
#include <vector>

namespace abstractions::threads
{

// Forware declarations
class Job;
class JobContext;

/// @brief Any callable that can execute an abstractions job.
struct IJobFunction
{
    /// @brief Run the job.
    /// @param ctx a context object with information about the specific job
    /// @return the job's success status
    ///
    /// The `ctx` is only valid for the duration of the job.  It should not be
    /// captured or stored.
    virtual Error operator()(JobContext &ctx) const = 0;
    virtual ~IJobFunction() = default;
};

/// @brief Execution information about the currently running job.
class JobContext
{
public:
    /// @brief Create a new job context.
    /// @param job_id job ID
    /// @param worker_id worker ID
    JobContext(int job_id, int worker_id);

    /// @brief ID of the particular job.
    int Id() const { return _job_id; }

    /// @brief ID of the worker that executes the job.
    int Worker() const { return _worker_id; }

private:
    int _job_id;
    int _worker_id;
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
};

/// @brief Runs a job on some concurrent worker, potentially on a separate thread.
class Job
{
public:

    /// @brief Create a new job.
    /// @tparam T function implementation
    /// @param id job ID
    /// @param args arguments to pass into the job function constructor
    template<typename T, typename... Arg>
    static Job New(int id, Arg&&... args)
    {
        static_assert(std::is_base_of<IJobFunction, T>::value, "'T' must inherit from IJobFunction.");
        return Job(id, std::make_shared<T>(std::forward<Arg>(args)...));
    }

    /// @brief Create a new job.
    /// @param id job ID
    /// @param fn function the job executes
    Job(int id, std::shared_ptr<IJobFunction> fn);

    /// @brief Run the job.
    /// @param worker_id ID of the worker executing the job
    JobResult Run(int worker_id);

    /// @brief Wait for the job to complete.
    void Wait() const;

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
