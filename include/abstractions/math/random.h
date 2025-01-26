#pragma once

#include <array>
#include <mutex>
#include <optional>
#include <random>
#include <thread>
#include <type_traits>

namespace abstractions {

/// @brief The default random number generator type.
using DefaultRngType = std::minstd_rand0;

/// @brief A lightweight adapter to a random number engine to make it easier to
///     keep track of what seed it used.  This complies with the
///     *UniformRandomBitGenerator* named requirement.
/// @tparam G an object that meets the *RandomNumberEngine* requirements
template <typename G = DefaultRngType>
class Prng {
public:
    /// @brief The wrapped bit generator's result type.
    typedef G::result_type result_type;

    /// @brief Create a new PRNG with the given seed.
    /// @param seed initial seed
    Prng(result_type seed) :
        _generator{seed},
        _seed{seed} {}

    Prng(const Prng<G> &other) :
        Prng(other.seed()) {}

    /// @brief The smallest value that the generator will return.
    static constexpr result_type min() {
        return G::min();
    }

    /// @brief The largest value that the generator will return.
    static constexpr result_type max() {
        return G::max();
    }

    /// @brief The seed the generator was initialized with.
    constexpr result_type seed() const {
        return _seed;
    }

    /// @brief Generate a pseudo-random number.
    /// @return A pseudo-random value between min() and max().
    result_type operator()() {
        return _generator();
    }

private:
    G _generator;
    G::result_type _seed;
};

/// @brief A thread-safe PRNG generator.
/// @tparam G the PRNG engine to use; defaults to the Mersenne Twister
///
/// This implementation is loosely based on https://stackoverflow.com/a/72101361.
/// A base seed is picked only once, and a sequence number is incremented every
/// time someone requests a PRNG.  The use case is for each thread to have
/// access to its own PRNG rather than attempting to sample from a single one.
///
/// The generator is explicitly designed to be "immovable" so that it's created
/// only in one spot and then can distribute PRNGs as needed.
template <typename G = DefaultRngType>
class PrngGenerator {
public:
    /// @brief Draw a random see from some random source.
    /// @return randomly-generated seed for a PRNG
    /// @note This uses std::random_device so shouldn't be called all that
    ///     often.  Use this when a PRNG seed only needs to be drawn once.
    ///     Otherwise, use the full PrngGenerator class to quick spawn PRNGs.
    static G::result_type DrawRandomSeed() {
        std::random_device device;
        return device();
    }

    /// @brief Create a new PRNG generator.
    /// @param seed base seed value; a random one will be generated if not provided
    PrngGenerator(std::optional<typename G::result_type> seed = {}) :
        _sequence_number{0} {
        if (seed) {
            _base_seed = *seed;
        } else {
            std::random_device device;
            _base_seed = device();
        }
    }

    /// @brief Create a new PRNG with a unique seed.
    /// @note This method is thread-safe and may be called by multiple threads.
    /// @return The initialized PRNG.
    Prng<G> CreatePrng() {
        std::lock_guard<std::mutex> lock{_mutex};
        _sequence_number++;
        typename G::result_type seed = _base_seed + _sequence_number;
        return Prng<G>(seed);
    }

    /// @brief Get the base seed used when generating PRNGs.
    G::result_type BaseSeed() const {
        return _base_seed;
    }

    /// @brief Get the sequence number used when generating PRNGs.
    ///
    /// This value is updated everytime a PRNG is created.  It will correspond
    /// to the number of PRNGs this generator has created so far.
    G::result_type SequenceNumber() const {
        return _sequence_number;
    }

    PrngGenerator(const PrngGenerator &) = delete;
    PrngGenerator(PrngGenerator &&) = delete;
    void operator=(const PrngGenerator &) = delete;
    void operator=(PrngGenerator &&) = delete;

private:
    G::result_type _base_seed;
    G::result_type _sequence_number;

    std::mutex _mutex;
};

/// @brief Combine an object that can take the output of a PRNG and turn it into
///     a proper statistical distribution.
/// @tparam G an object that meets the *RandomNumberEngine* named requirement
/// @tparam D an object that means the *RandomNumberDistribution* name requirement
template <typename G, typename D>
class Distribution {
public:
    /// @brief Create a new Distribution instance.
    /// @param generator the PRNG instance
    /// @param distribution the statistical distribution
    Distribution(Prng<G> generator, D distribution) :
        _generator{generator},
        _distribution{distribution} {}

    /// @brief Draw a sample from the given distribution.
    D::result_type Sample() {
        return _distribution(_generator);
    }

    /// @brief Seed used by the internal PRNG.
    G::result_type Seed() const {
        return _generator.seed();
    }

private:
    Prng<G> _generator;
    D _distribution;
};

/// @brief Generate a sequence of normally distributed random numbers.
/// @tparam G an object that meets the *RandomNumberEngine* named requirement
template <typename G = DefaultRngType>
class NormalDistribution : public Distribution<G, std::normal_distribution<double>> {
public:
    /// @brief Create a new normal distribution instance.
    /// @param generator a PRNG instance
    /// @param mean the distribution mean
    /// @param sigma the distribution standard deviation
    NormalDistribution(Prng<G> generator, double mean, double sigma) :
        Distribution<G, std::normal_distribution<double>>(generator,
                                                          std::normal_distribution(mean, sigma)) {}
};

/// @brief Generate a sequence of uniformally distributed random numbers on [0,1).
/// @tparam G an object that meets the *RandomNumberEngine* named requirement
template <typename G = DefaultRngType>
class UniformDistribution : public Distribution<G, std::uniform_real_distribution<double>> {
public:
    /// @brief Create a new uniform distribution instance.
    /// @param generator a PRNG instance
    UniformDistribution(Prng<G> generator) :
        Distribution<G, std::uniform_real_distribution<double>>(
            generator, std::uniform_real_distribution(0.0, 1.0)) {}
};

}  // namespace abstractions
