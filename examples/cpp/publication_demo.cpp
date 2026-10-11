#include "publication_packet.hpp"
#include "spsc_publication_slot.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>

int main() {
    using namespace publication_lab;
    SpscPublicationSlot<Packet> slot;
    constexpr std::uint64_t count = 32;
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    std::atomic<bool> abort{false};
    std::uint64_t received = 0;  // Consumer-only until join.
    bool valid = true;
    std::thread consumer([&] {
        Packet output{};
        while (received < count && !abort.load(std::memory_order_relaxed)) {
            if (std::chrono::steady_clock::now() >= deadline) {
                valid = false;
                abort.store(true, std::memory_order_relaxed);
                break;
            }
            if (!slot.try_consume(output)) {
                std::this_thread::yield();  // Scheduling hint; not synchronization.
                continue;
            }
            if (!packet_valid(output) || output.sequence != received) {
                valid = false;
                abort.store(true, std::memory_order_relaxed);
                break;
            }
            ++received;
        }
    });
    std::uint64_t sent = 0;
    bool producer_valid = true;
    while (sent < count && !abort.load(std::memory_order_relaxed)) {
        auto input = make_packet(sent, static_cast<std::uint32_t>(sent % 33U));
        if (slot.try_publish(input)) {
            ++sent;
        } else {
            std::this_thread::yield();
        }
        if (std::chrono::steady_clock::now() >= deadline) {
            abort.store(true, std::memory_order_relaxed);
            producer_valid = false;
            break;
        }
    }
    consumer.join();
    const bool passed = producer_valid && valid && sent == count && received == count;
    std::cout << "publication demo: " << (passed ? "PASS" : "FAIL")
              << " sent=" << sent << " received=" << received
              << " flag_lock_free=" << slot.flag_is_lock_free() << '\n';
    return passed ? 0 : 1;
}
