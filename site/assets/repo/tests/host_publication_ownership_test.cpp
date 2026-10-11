#include "../examples/cpp/publication_packet.hpp"
#include "../examples/cpp/spsc_publication_slot.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <type_traits>

namespace {
using publication_lab::Packet;
using publication_lab::SpscPublicationSlot;
using publication_lab::make_packet;
using publication_lab::packet_equal;
using publication_lab::packet_valid;
using Clock = std::chrono::steady_clock;

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void test_empty_and_full() {
    SpscPublicationSlot<Packet> slot;
    static_assert(!std::is_copy_constructible_v<decltype(slot)>);
    static_assert(!std::is_move_constructible_v<decltype(slot)>);
    auto output = make_packet(99, 7);
    const auto sentinel = output;
    require(!slot.try_consume(output), "empty must reject consume");
    require(packet_equal(output, sentinel), "failed consume changed output");
    unsigned visits = 0;
    require(!slot.try_visit([&](const Packet&) noexcept { ++visits; }),
            "empty must reject visit");
    require(visits == 0, "empty visit invoked callback");
    auto first = make_packet(0, 0);
    auto second = make_packet(1, 32);
    const auto second_before = second;
    require(slot.try_publish(first), "initial publish failed");
    require(!slot.try_publish(second), "full slot accepted another message");
    require(packet_equal(second, second_before), "failed publish changed source");
    require(slot.try_consume(output), "full slot rejected consume");
    require(packet_equal(output, make_packet(0, 0)), "wrong first message");
    require(slot.try_publish(second), "slot did not become reusable");
    require(slot.try_consume(output), "second message missing");
    require(packet_equal(output, second_before), "wrong second message");
    require(!slot.try_consume(output), "message consumed twice");
    require(packet_equal(output, second_before), "empty retry changed output");
}

void test_payload_boundaries() {
    SpscPublicationSlot<Packet> slot;
    for (std::uint32_t length = 0; length <= 32; ++length) {
        auto input = make_packet(std::numeric_limits<std::uint64_t>::max() - length,
                                 length);
        const auto expected = input;
        Packet output{};
        require(slot.try_publish(input), "boundary publish failed");
        require(slot.try_consume(output), "boundary consume failed");
        require(packet_valid(output) && packet_equal(output, expected),
                "boundary payload mismatch");
    }
    auto invalid = make_packet(1, 33);
    require(!packet_valid(invalid), "test oracle accepted oversized logical length");
    // The slot is a transport, not a validator: never index by this length.
}

void test_sequential_reuse() {
    SpscPublicationSlot<Packet> slot;
    Packet output{};
    for (std::uint64_t sequence = 0; sequence < 1000; ++sequence) {
        auto input = make_packet(sequence, static_cast<std::uint32_t>(sequence % 33U));
        const auto expected = input;
        require(slot.try_publish(input), "sequential publish failed");
        auto blocked = make_packet(sequence + 1U, 0);
        const auto blocked_before = blocked;
        require(!slot.try_publish(blocked), "sequential overwrite accepted");
        require(packet_equal(blocked, blocked_before), "rejected source moved");
        require(slot.try_consume(output), "sequential consume failed");
        require(packet_equal(output, expected), "sequential payload mismatch");
        require(!slot.try_consume(output), "sequential duplicate accepted");
        require(packet_equal(output, expected), "duplicate retry changed output");
    }
}

struct Counted {
    int id;
    int* destroyed;
    ~Counted() noexcept { ++*destroyed; }
};

void test_move_only_ownership() {
    int destroyed = 0;
    std::unique_ptr<Counted> output;
    {
        SpscPublicationSlot<std::unique_ptr<Counted>> slot;
        auto first = std::make_unique<Counted>();
        first->id = 1;
        first->destroyed = &destroyed;
        const auto* first_address = first.get();
        require(slot.try_publish(first), "unique_ptr publish failed");
        require(!first, "successful publish did not transfer unique_ptr");
        auto second = std::make_unique<Counted>();
        second->id = 2;
        second->destroyed = &destroyed;
        const auto* second_address = second.get();
        require(!slot.try_publish(second), "full unique_ptr slot accepted item");
        require(second.get() == second_address && destroyed == 0,
                "failed unique_ptr publish lost ownership");
        require(slot.try_consume(output), "unique_ptr consume failed");
        require(output.get() == first_address && output->id == 1,
                "consumer failed to acquire exact owned object");
        require(slot.try_publish(second), "unique_ptr slot reuse failed");
        require(!second && output->id == 1 && destroyed == 0,
                "slot reuse invalidated moved-out object");
        require(slot.try_consume(output), "second unique_ptr consume failed");
        require(output.get() == second_address && destroyed == 1,
                "replacing consumer output did not destroy prior resource once");
        auto pending = std::make_unique<Counted>();
        pending->id = 3;
        pending->destroyed = &destroyed;
        require(slot.try_publish(pending), "pending unique_ptr publish failed");
    }
    require(destroyed == 2 && output->id == 2,
            "slot destruction should release only its pending owned resource");
    output.reset();
    require(destroyed == 3, "resource count must be exactly three");
}

void test_borrow_then_copy() {
    SpscPublicationSlot<Packet> slot;
    auto input = make_packet(47, 32);
    Packet independent{};
    unsigned visits = 0;
    require(slot.try_publish(input), "borrow publish failed");
    require(slot.try_visit([&](const Packet& borrowed) noexcept {
        independent = borrowed;
        ++visits;
    }), "borrow visit failed");
    require(visits == 1 && packet_equal(independent, make_packet(47, 32)),
            "borrow snapshot wrong");
    auto next = make_packet(48, 0);
    require(slot.try_publish(next), "borrow did not grant reuse after callback");
    require(packet_equal(independent, make_packet(47, 32)),
            "owned snapshot changed after reuse");
    Packet output{};
    require(slot.try_consume(output) && output.sequence == 48,
            "next item wrong after borrowed visit");
}

void test_controlled_reuse() {
    SpscPublicationSlot<Packet> slot;
    auto first = make_packet(701, 32);
    const auto expected = first;
    require(slot.try_publish(first), "controlled first publish failed");
    std::atomic<bool> entered{false};
    std::atomic<bool> leave{false};
    const auto deadline = Clock::now() + std::chrono::seconds(5);
    bool visit_result = false;  // Child-only until join.
    bool borrowed_valid = false;
    bool callback_timeout = false;
    std::thread consumer([&] {
        visit_result = slot.try_visit([&](const Packet& borrowed) noexcept {
            entered.store(true, std::memory_order_release);
            while (!leave.load(std::memory_order_acquire)) {
                if (Clock::now() >= deadline) {
                    callback_timeout = true;
                    break;
                }
                std::this_thread::yield();
            }
            // Read at the END of the controlled borrow, before returning the grant.
            borrowed_valid = packet_equal(borrowed, expected);
        });
    });
    bool entered_in_time = false;
    while (Clock::now() < deadline) {
        if (entered.load(std::memory_order_acquire)) {
            entered_in_time = true;
            break;
        }
        std::this_thread::yield();
    }
    bool all_rejected_unchanged = entered_in_time;
    if (entered_in_time) {
        for (unsigned i = 0; i < 32; ++i) {
            auto next = make_packet(702, 1);
            const auto before = next;
            if (slot.try_publish(next) || !packet_equal(next, before)) {
                all_rejected_unchanged = false;
            }
        }
    }
    leave.store(true, std::memory_order_release);
    consumer.join();  // Always release/join before require can throw.
    require(entered_in_time && !callback_timeout, "controlled borrow timed out");
    require(all_rejected_unchanged, "producer reused payload while callback held grant");
    require(visit_result && borrowed_valid, "borrowed payload changed before return");
    auto next = make_packet(702, 1);
    require(slot.try_publish(next), "post-borrow reuse failed");
    Packet output{};
    require(slot.try_consume(output) && output.sequence == 702,
            "post-borrow payload wrong");
}

void run_fifo_case(std::uint64_t count, unsigned producer_yield, unsigned consumer_yield) {
    SpscPublicationSlot<Packet> slot;
    std::atomic<bool> abort{false};
    const auto deadline = Clock::now() + std::chrono::seconds(8);
    std::uint64_t received = 0;
    bool consumer_valid = true;
    std::thread consumer([&] {
        Packet output{};
        while (received < count && !abort.load(std::memory_order_relaxed)) {
            if (Clock::now() >= deadline) {
                consumer_valid = false;
                abort.store(true, std::memory_order_relaxed);
                break;
            }
            if (!slot.try_consume(output)) {
                std::this_thread::yield();
                continue;
            }
            if (output.sequence != received || !packet_valid(output)) {
                consumer_valid = false;
                abort.store(true, std::memory_order_relaxed);
                break;
            }
            ++received;
            if (consumer_yield != 0 && received % consumer_yield == 0) {
                std::this_thread::yield();
            }
        }
    });
    std::uint64_t sent = 0;
    bool producer_valid = true;
    while (sent < count && !abort.load(std::memory_order_relaxed)) {
        auto input = make_packet(sent, static_cast<std::uint32_t>(sent % 33U));
        const auto before = input;
        if (slot.try_publish(input)) {
            ++sent;
            if (producer_yield != 0 && sent % producer_yield == 0) {
                std::this_thread::yield();
            }
        } else {
            if (!packet_equal(input, before)) {
                producer_valid = false;
                abort.store(true, std::memory_order_relaxed);
            }
            std::this_thread::yield();
        }
        if (Clock::now() >= deadline) {
            producer_valid = false;
            abort.store(true, std::memory_order_relaxed);
        }
    }
    consumer.join();
    require(producer_valid && consumer_valid && sent == count && received == count,
            "concurrent FIFO failed or exceeded bounded deadline");
    Packet output = make_packet(900000, 2);
    const auto sentinel = output;
    require(!slot.try_consume(output) && packet_equal(output, sentinel),
            "FIFO did not finish empty or changed failed output");
}

void test_concurrent_fifo() {
    run_fifo_case(30000, 0, 0);
    run_fifo_case(30000, 1, 13);
    run_fifo_case(30000, 11, 1);
}

}  // namespace

int main() {
    struct Case { const char* name; void (*run)(); };
    const Case cases[] = {
        {"empty/full and unchanged failure", test_empty_and_full},
        {"33 lengths and uint64 boundary patterns", test_payload_boundaries},
        {"1000 sequential ownership round trips", test_sequential_reuse},
        {"move-only exact-once resource ownership", test_move_only_ownership},
        {"borrow then independent copy", test_borrow_then_copy},
        {"controlled borrow forbids early reuse", test_controlled_reuse},
        {"90000 FIFO messages in three yield patterns", test_concurrent_fifo},
    };
    try {
        for (const auto& test : cases) {
            test.run();
            std::cout << "PASS: " << test.name << '\n';
        }
        SpscPublicationSlot<Packet> slot;
        std::cout << "HOST OBSERVATION: atomic<bool>::is_always_lock_free="
                  << std::atomic<bool>::is_always_lock_free
                  << " flag_is_lock_free=" << slot.flag_is_lock_free() << '\n';
        std::cout << "publication ownership: PASS (7 groups; host only)\n";
    } catch (const std::exception& error) {
        std::cerr << "publication ownership: FAIL: " << error.what() << '\n';
        return 1;
    }
}
