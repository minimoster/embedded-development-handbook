#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace publication_lab {

struct Packet {
    std::uint64_t sequence = 0;
    std::uint32_t length = 0;
    std::array<std::uint8_t, 32> bytes{};
    std::uint32_t checksum = 0;
};

// Deterministic test pattern/checksum, NOT a wire protocol, CRC or authentication.
inline std::uint32_t packet_checksum(const Packet& packet) noexcept {
    std::uint32_t result = static_cast<std::uint32_t>(packet.sequence) ^
                           static_cast<std::uint32_t>(packet.sequence >> 32U) ^
                           packet.length;
    for (const auto byte : packet.bytes) {
        result = (result * 33U) ^ byte;  // Unsigned modulo arithmetic is intentional.
    }
    return result;
}

inline Packet make_packet(std::uint64_t sequence, std::uint32_t length) noexcept {
    Packet packet{};
    packet.sequence = sequence;
    packet.length = length;
    for (std::size_t i = 0; i < packet.bytes.size(); ++i) {
        packet.bytes[i] = static_cast<std::uint8_t>(sequence + i * 17U + length);
    }
    packet.checksum = packet_checksum(packet);
    return packet;
}

inline bool packet_equal(const Packet& left, const Packet& right) noexcept {
    return left.sequence == right.sequence && left.length == right.length &&
           left.bytes == right.bytes && left.checksum == right.checksum;
}

inline bool packet_valid(const Packet& packet) noexcept {
    return packet.length <= packet.bytes.size() &&
           packet_equal(packet, make_packet(packet.sequence, packet.length));
}

}  // namespace publication_lab
