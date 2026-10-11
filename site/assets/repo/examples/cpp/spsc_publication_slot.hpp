#pragma once

#include <atomic>
#include <type_traits>
#include <utility>

namespace publication_lab {

// HOST C++17 contract, not an ISR/DMA port.
// One non-reentrant producer, one non-reentrant consumer. The slot outlives both.
// Source/output must not alias the slot or concurrently accessed external objects.
// T's reachable state must obey the same ownership discipline as T itself.
// A visit borrows const T only for the duration of its noexcept callback.
// No reset/close/third endpoint. Join users before destruction.
template <class T>
class SpscPublicationSlot final {
    static_assert(std::is_nothrow_default_constructible_v<T>,
                  "T must have nonthrowing default construction");
    static_assert(std::is_nothrow_move_assignable_v<T>,
                  "T must have nonthrowing move assignment");
    static_assert(std::is_nothrow_destructible_v<T>,
                  "T must have a nonthrowing destructor");

    T payload_{};                       // Always alive; no placement new.
    std::atomic<bool> ready_{false};     // false: producer grant; true: consumer grant.

public:
    SpscPublicationSlot() noexcept = default;
    ~SpscPublicationSlot() = default;
    SpscPublicationSlot(const SpscPublicationSlot&) = delete;
    SpscPublicationSlot& operator=(const SpscPublicationSlot&) = delete;
    SpscPublicationSlot(SpscPublicationSlot&&) = delete;
    SpscPublicationSlot& operator=(SpscPublicationSlot&&) = delete;

    // Failure does not move from source. Success uses T's move semantics.
    bool try_publish(T& source) noexcept {
        if (ready_.load(std::memory_order_acquire)) {
            return false;
        }
        payload_ = std::move(source);
        ready_.store(true, std::memory_order_release);
        return true;
    }

    // Output changes only on success. Reuse is granted AFTER move assignment.
    bool try_consume(T& output) noexcept {
        if (!ready_.load(std::memory_order_acquire)) {
            return false;
        }
        output = std::move(payload_);
        ready_.store(false, std::memory_order_release);
        return true;
    }

    // Alternative consumer operation: inspect then drop the message.
    // Callback must not retain references/pointers, reenter this slot, or give
    // other threads access to its borrowed payload/reachable mutable state.
    // Dropping returns the slot grant, not necessarily T's resource immediately.
    template <class F>
    bool try_visit(F&& visitor) noexcept {
        static_assert(std::is_nothrow_invocable_v<F&&, const T&>,
                      "visitor(const T&) must be noexcept");
        if (!ready_.load(std::memory_order_acquire)) {
            return false;
        }
        std::forward<F>(visitor)(std::as_const(payload_));
        ready_.store(false, std::memory_order_release);
        return true;
    }

    // Observation of this host object only; does not certify ISR/WCET suitability.
    bool flag_is_lock_free() const noexcept { return ready_.is_lock_free(); }
};

}  // namespace publication_lab
