#pragma once

#include <abstractions/types.h>

#include <any>
#include <chrono>
#include <future>
#include <memory>
#include <type_traits>
#include <vector>

namespace abstractions::threads {

// Forware declarations
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
    /// @param job_id job ID
    /// @param worker_id worker ID
    JobContext(int job_id, int worker_id, std::any &data);

    /// @brief Gets a reference to the data stored in the job context.
    /// @tparam T expected data type
    /// @return contained data
    template<typename T>
    T Data() const {
        AssertHasData();
        return std::any_cast<T>(_data);
    }

    /// @brief Check if the context contains data of the given type.
    /// @tparam T type being checked
    /// @return `true` if there is data and the type matches
    template<typename T>
    bool HasValueOfType() const
    {
        auto &type = typeid(T);
        return HasData() && _data.type() == type;
    }

    /// @brief Gets a reference to the data stored in the job context.
    std::any &Data();

    /// @brief Determine if context contains any data.
    bool HasData() const;

    /// @brief ID of the particular job.
    int Id() const {
        return _job_id;
    }

    /// @brief ID of the worker that executes the job.
    int Worker() const {
        return _worker_id;
    }

private:
    void AssertHasData() const;
    int _job_id;
    int _worker_id;
    std::any &_data;
};

/// @brief The status of a job once it completes.
struct JobStatus {
    /// @brief The ID of the finished job.
    int job_id;

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
    /// @param id user-specified job ID
    /// @param args constructor arguments
    /// @return a new Job instance
    template <typename T, typename... Arg>
    static Job New(int id, Arg &&...args) {
        static_assert(std::is_base_of<IJobFunction, T>::value,
                      "'T' must inherit from IJobFunction.");
        return Job(id, std::make_unique<T>(std::forward<Arg>(args)...));
    }

    /// @brief Create a new job.
    /// @tparam T IJobFunction class type
    /// @tparam S payload type
    /// @tparam Arg IJobFunction constructor argument types
    /// @param id user-specified job ID
    /// @param payload data the job can access when it executes
    /// @param args constructor arguments
    /// @return a new Job instance
    template<typename T, typename S, typename... Arg>
    static Job NewWithPayload(int id, S &&payload, Arg &&...args)
    {
        static_assert(std::is_base_of<IJobFunction, T>::value, "'T' must inherit from IJobFunction.");
        return Job(id, std::make_any<S>(payload), std::make_unique<T>(std::forward<Arg>(args)...));
    }

    /// @brief Create a new job.
    /// @param id job ID
    /// @param fn function the job executes
    Job(int id, std::unique_ptr<IJobFunction> fn);

    /// @brief Create a new job.
    /// @param id job ID
    /// @param payload data that goes to the job when it runs
    /// @param fn function the job executes
    Job(int id, std::any payload, std::unique_ptr<IJobFunction> fn);

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
    int Id() const;

    Job(const Job &) = delete;
    Job &operator=(const Job &) = delete;
    Job(Job &&) = default;
    Job &operator=(Job &&) = default;

private:
    int _id;
    std::unique_ptr<IJobFunction> _fn;
    std::unique_ptr<std::any> _payload;
    std::promise<JobStatus> _job_status;
};

}  // namespace abstractions::threads
