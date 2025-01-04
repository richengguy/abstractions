#pragma once

#include <abstractions/errors.h>
#include <abstractions/types.h>

#include <fmt/format.h>
#include <fmt/std.h>

#include <initializer_list>
#include <any>
#include <chrono>
#include <future>
#include <memory>
#include <type_traits>
#include <vector>

namespace abstractions::threads {

// Forward declarations
class Job;
class JobContext;

/// @brief Any callable that can execute an abstractions job.
struct IJobFunction {
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
class JobContext {
public:
    /// @brief Create a new job context.
    /// @param index job ID
    /// @param worker_id worker ID
    JobContext(int index, int worker_id, std::any &data);

    /// @brief Check if the context contains data of the given type.
    /// @tparam T type being checked
    /// @return `true` if there is data and the type matches
    template<typename T>
    bool HasValueOfType() const
    {
        auto &type = typeid(T);
        return _data.has_value() && _data.type() == type;
    }

    /// @brief Gets a reference to the data stored in the job context.
    /// @tparam T expected data type
    /// @return contained data or an error if it could not be extracted
    template<typename T>
    Expected<T> Data() const {
        if (!_data.has_value()) {
            return errors::report<T>("Context contains no data.");
        }

        if (!HasValueOfType<T>()) {
            return errors::report<T>(fmt::format("Context contains data of type '{}'; expected '{}'.", _data.type(), typeid(T)));
        }

        return std::any_cast<T>(_data);
    }

    /// @brief Gets a reference to the data stored in the job context.
    std::any &Data();

    /// @brief ID of the particular job.
    int Index() const {
        return _index;
    }

    /// @brief ID of the worker that executes the job.
    int Worker() const {
        return _worker_id;
    }

private:
    int _index;
    int _worker_id;
    std::any &_data;
};

/// @brief The status of a job once it completes.
struct JobStatus {
    /// @brief The ID of the finished job.
    int index;

    /// @brief The job's error status.
    Error error;

    /// @brief The length of time the job took.
    std::chrono::microseconds time;
};

/// @brief Runs a job on some concurrent worker, potentially on a separate thread.
class Job {
public:
    typedef std::future<JobStatus> Future;    ///< Future type a job uses to report on its status.
    typedef std::promise<JobStatus> Promise;  ///< Promise type a job expects for reporting status.

    /// @brief Create a new job.
    /// @tparam T IJobFunction class type
    /// @tparam Arg IJobFunction constructor argument types
    /// @param index user-specified job ID
    /// @param args constructor arguments
    /// @return a new Job instance
    template <typename T, typename... Arg>
    static Job New(int index, Arg &&...args) {
        static_assert(std::is_base_of<IJobFunction, T>::value,
                      "'T' must inherit from IJobFunction.");
        return Job(index, std::make_unique<T>(std::forward<Arg>(args)...));
    }

    /// @brief Create a new job.
    /// @tparam T IJobFunction class type
    /// @tparam S payload type
    /// @tparam Arg IJobFunction constructor argument types
    /// @param index user-specified job ID
    /// @param payload data the job can access when it executes
    /// @param args constructor arguments
    /// @return a new Job instance
    template<typename T, typename S, typename... Arg>
    static Job NewWithPayload(int index, S &&payload, Arg &&...args)
    {
        static_assert(std::is_base_of<IJobFunction, T>::value, "'T' must inherit from IJobFunction.");
        return Job(index, std::make_any<S>(payload), std::make_unique<T>(std::forward<Arg>(args)...));
    }

    /// @brief Create a new job.
    /// @param index job ID
    /// @param fn function the job executes
    Job(int index, std::unique_ptr<IJobFunction> fn);

    /// @brief Create a new job.
    /// @param index job ID
    /// @param payload data that goes to the job when it runs
    /// @param fn function the job executes
    Job(int index, std::any payload, std::unique_ptr<IJobFunction> fn);

    /// @brief Run the job.
    /// @param worker_id ID of the worker executing the job
    JobStatus Run(int worker_id);

    /// @brief Provide a promise to the job for it to use to signal when job is
    ///     complete and report its final status.
    /// @param promise promise object used for reporting status
    ///
    /// The job will take ownership of the promise.  The caller can then create
    /// a future from the promise to get information on the job's status after
    /// it finishes.
    void SetPromise(Promise &promise);

    /// @brief The user-specified job ID.
    int Index() const;

    Job(const Job &) = delete;
    Job &operator=(const Job &) = delete;
    Job(Job &&) = default;
    Job &operator=(Job &&) = default;

private:
    int _index;
    std::unique_ptr<IJobFunction> _fn;
    std::unique_ptr<std::any> _payload;
    std::promise<JobStatus> _job_status;
};

/// @brief Have the current thread wait for a set of jobs to complete.
/// @param futures pointers to a set of job futures
void WaitForJobs(std::initializer_list<const Job::Future *> futures);

/// @brief Have the current thread wait for a set of jobs to complete.
/// @param futures set of job futures
void WaitForJobs(const std::vector<Job::Future> &futures);

}  // namespace abstractions::threads
