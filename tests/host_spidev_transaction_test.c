#include "../examples/c/spidev_transaction.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <linux/spi/spidev.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static const unsigned long requests[7] = {
    SPI_IOC_WR_MODE32, SPI_IOC_RD_MODE32,
    SPI_IOC_WR_BITS_PER_WORD, SPI_IOC_RD_BITS_PER_WORD,
    SPI_IOC_WR_MAX_SPEED_HZ, SPI_IOC_RD_MAX_SPEED_HZ,
    SPI_IOC_MESSAGE(2)
};
static const enum spidev_lab_step steps[7] = {
    SPIDEV_LAB_WR_MODE, SPIDEV_LAB_RD_MODE,
    SPIDEV_LAB_WR_WORD, SPIDEV_LAB_RD_WORD,
    SPIDEV_LAB_WR_SPEED, SPIDEV_LAB_RD_SPEED, SPIDEV_LAB_MESSAGE
};

struct mock {
    int calls;
    int fail_at;
    int fail_errno;
    int odd_at;
    int odd_return;
    int mismatch_at;
    bool override_message;
    int message_return;
    uint32_t mode;
    uint8_t word;
    uint32_t speed;
    struct spidev_lab_config expected_config;
    const uint8_t *command;
    size_t command_len;
    size_t reply_len;
    uint8_t dummy;
};

static uint8_t response_byte(size_t i, uint32_t mode)
{
    return (uint8_t)(i ^ (size_t)(UINT32_C(0xa0) + mode));
}

static void inspect_message(struct mock *m, void *argument)
{
    const struct spi_ioc_transfer *t = argument;
    assert(t[0].len == m->command_len && t[1].len == m->reply_len);
    assert(t[0].tx_buf != 0 && t[0].rx_buf == 0);
    assert(t[1].tx_buf != 0 && t[1].rx_buf != 0);
    const uint8_t *command = (const uint8_t *)(uintptr_t)t[0].tx_buf;
    const uint8_t *dummy = (const uint8_t *)(uintptr_t)t[1].tx_buf;
    uint8_t *received = (uint8_t *)(uintptr_t)t[1].rx_buf;
    assert(memcmp(command, m->command, m->command_len) == 0);
    for (size_t i = 0; i < 2; ++i) {
        assert(t[i].speed_hz == m->expected_config.speed_hz);
        assert(t[i].bits_per_word == 8);
        assert(t[i].cs_change == 0 && t[i].delay_usecs == 0);
        assert(t[i].tx_nbits == 0 && t[i].rx_nbits == 0);
        assert(t[i].word_delay_usecs == 0 && t[i].pad == 0);
    }
    for (size_t i = 0; i < m->reply_len; ++i) {
        assert(dummy[i] == m->dummy);
        received[i] = response_byte(i, m->expected_config.mode);
    }
}

static int mock_ioctl(void *context, int fd, unsigned long request, void *argument)
{
    struct mock *m = context;
    assert(fd == 37 && m->calls >= 0 && m->calls < 7);
    const int index = m->calls++;
    assert(request == requests[index] && argument != NULL);
    /* Dirty the private RX staging buffer even when a message fails. */
    if (index == 6) inspect_message(m, argument);
    if (index == m->fail_at) {
        errno = m->fail_errno;
        return -1;
    }
    if (index == m->odd_at) return m->odd_return;
    switch (index) {
    case 0:
        m->mode = *(uint32_t *)argument;
        assert(m->mode == m->expected_config.mode);
        break;
    case 1:
        *(uint32_t *)argument = m->mismatch_at == index ? m->mode ^ 1u : m->mode;
        break;
    case 2:
        m->word = *(uint8_t *)argument;
        assert(m->word == 8);
        break;
    case 3:
        *(uint8_t *)argument = m->mismatch_at == index ? 16 : m->word;
        break;
    case 4:
        m->speed = *(uint32_t *)argument;
        assert(m->speed == m->expected_config.speed_hz);
        break;
    case 5:
        *(uint32_t *)argument = m->mismatch_at == index ? m->speed - 1u : m->speed;
        break;
    case 6:
        if (m->override_message) return m->message_return;
        return (int)(m->command_len + m->reply_len);
    default:
        assert(false);
    }
    /* Successful ioctl does not make errno meaningful. */
    errno = EAGAIN;
    return 0;
}

static struct mock fixture(const uint8_t *command, size_t command_len,
                           size_t reply_len, uint32_t mode, uint8_t dummy)
{
    struct mock m = {0};
    m.fail_at = -1;
    m.odd_at = -1;
    m.mismatch_at = -1;
    m.fail_errno = EIO;
    m.expected_config = (struct spidev_lab_config){mode, UINT32_C(1000000), 8};
    m.command = command;
    m.command_len = command_len;
    m.reply_len = reply_len;
    m.dummy = dummy;
    return m;
}

static enum spidev_lab_status run(struct mock *m, uint8_t *reply,
                                struct spidev_lab_report *report)
{
    const struct spidev_lab_io io = {mock_ioctl, m};
    return spidev_lab_command_read(&io, 37, &m->expected_config,
                                   m->command, m->command_len, m->dummy,
                                   reply, m->reply_len, report);
}

static unsigned success_matrix(const uint8_t *command)
{
    const size_t commands[] = {1, 2, SPIDEV_LAB_COMMAND_MAX};
    const size_t replies[] = {1, 4, SPIDEV_LAB_REPLY_MAX};
    const uint8_t dummies[] = {0x00, 0xff, 0xa5};
    unsigned cases = 0;
    for (uint32_t mode = 0; mode < 4; ++mode)
        for (size_t c = 0; c < 3; ++c)
            for (size_t n = 0; n < 3; ++n)
                for (size_t d = 0; d < 3; ++d) {
                    struct mock m = fixture(command, commands[c], replies[n], mode, dummies[d]);
                    uint8_t reply[SPIDEV_LAB_REPLY_MAX];
                    memset(reply, 0x5a, sizeof reply);
                    struct spidev_lab_report r;
                    assert(run(&m, reply, &r) == SPIDEV_LAB_OK);
                    assert(m.calls == 7 && r.step == SPIDEV_LAB_COMPLETE);
                    assert(r.syscall_errno == 0);
                    assert(r.expected_bytes == (int)(commands[c] + replies[n]));
                    assert(r.returned == r.expected_bytes);
                    for (size_t i = 0; i < replies[n]; ++i)
                        assert(reply[i] == response_byte(i, mode));
                    for (size_t i = replies[n]; i < sizeof reply; ++i) assert(reply[i] == 0x5a);
                    ++cases;
                }
    struct mock m = fixture(command, 2, 4, 0, 0xff);
    uint8_t reply[4] = {0};
    assert(run(&m, reply, NULL) == SPIDEV_LAB_OK);
    return cases + 1u;
}

static void unchanged(const uint8_t *reply, size_t n)
{
    for (size_t i = 0; i < n; ++i) assert(reply[i] == 0x5a);
}

static unsigned failures(const uint8_t *command)
{
    unsigned cases = 0;
    const int errors[] = {EIO, EINTR, ETIMEDOUT};
    for (int index = 0; index < 7; ++index)
        for (size_t e = 0; e < 3; ++e) {
            struct mock m = fixture(command, 2, 4, 0, 0xff);
            m.fail_at = index;
            m.fail_errno = errors[e];
            uint8_t reply[4];
            memset(reply, 0x5a, sizeof reply);
            struct spidev_lab_report r;
            assert(run(&m, reply, &r) ==
                   (index == 6 ? SPIDEV_LAB_MESSAGE_IOCTL : SPIDEV_LAB_CONFIG_IOCTL));
            assert(r.step == steps[index] && r.syscall_errno == errors[e]);
            assert(r.returned == -1 && m.calls == index + 1);
            unchanged(reply, sizeof reply);
            ++cases;
        }
    for (int index = 0; index < 6; ++index) {
        struct mock m = fixture(command, 2, 4, 0, 0xff);
        m.odd_at = index;
        m.odd_return = 1;
        uint8_t reply[4];
        memset(reply, 0x5a, sizeof reply);
        struct spidev_lab_report r;
        assert(run(&m, reply, &r) == SPIDEV_LAB_CONFIG_RETURN);
        assert(r.step == steps[index] && r.syscall_errno == 0 && r.returned == 1);
        assert(m.calls == index + 1);
        unchanged(reply, sizeof reply);
        ++cases;
    }
    for (int index = 1; index < 6; index += 2) {
        struct mock m = fixture(command, 2, 4, 0, 0xff);
        m.mismatch_at = index;
        uint8_t reply[4];
        memset(reply, 0x5a, sizeof reply);
        struct spidev_lab_report r;
        assert(run(&m, reply, &r) == SPIDEV_LAB_CONFIG_MISMATCH);
        assert(r.step == steps[index] && r.syscall_errno == 0);
        assert(m.calls == index + 1);
        unchanged(reply, sizeof reply);
        ++cases;
    }
    const int anomalous_lengths[] = {0, 1, 4, 5, 7, INT_MAX, -2};
    for (size_t i = 0; i < sizeof anomalous_lengths / sizeof anomalous_lengths[0]; ++i) {
        struct mock m = fixture(command, 2, 4, 0, 0xff);
        m.override_message = true;
        m.message_return = anomalous_lengths[i];
        uint8_t reply[4];
        memset(reply, 0x5a, sizeof reply);
        struct spidev_lab_report r;
        assert(run(&m, reply, &r) == SPIDEV_LAB_MESSAGE_LENGTH);
        assert(r.step == SPIDEV_LAB_MESSAGE && r.returned == anomalous_lengths[i]);
        assert(r.expected_bytes == 6 && r.syscall_errno == 0 && m.calls == 7);
        unchanged(reply, sizeof reply);
        ++cases;
    }
    return cases;
}

static unsigned invalid_inputs(const uint8_t *command)
{
    struct mock m = fixture(command, 2, 4, 0, 0xff);
    const struct spidev_lab_io io = {mock_ioctl, &m};
    const struct spidev_lab_io missing_callback = {NULL, &m};
    const struct spidev_lab_config config = m.expected_config;
    struct spidev_lab_config invalid = config;
    uint8_t reply[4];
    memset(reply, 0x5a, sizeof reply);
    struct spidev_lab_report r;
    unsigned cases = 0;
#define INVALID_CALL(bus, fd, cfg, cmd, cmd_len, out, out_len) do { \
    assert(spidev_lab_command_read((bus), (fd), (cfg), (cmd), (cmd_len), \
                                   0xff, (out), (out_len), &r) == SPIDEV_LAB_INVALID); \
    assert(m.calls == 0 && r.step == SPIDEV_LAB_INPUT && r.syscall_errno == 0); \
    unchanged(reply, sizeof reply); \
    ++cases; \
} while (0)
    INVALID_CALL(NULL, 37, &config, command, 2, reply, 4);
    INVALID_CALL(&missing_callback, 37, &config, command, 2, reply, 4);
    INVALID_CALL(&io, -1, &config, command, 2, reply, 4);
    INVALID_CALL(&io, 37, NULL, command, 2, reply, 4);
    INVALID_CALL(&io, 37, &config, NULL, 2, reply, 4);
    INVALID_CALL(&io, 37, &config, command, 0, reply, 4);
    INVALID_CALL(&io, 37, &config, command, SPIDEV_LAB_COMMAND_MAX + 1u, reply, 4);
    INVALID_CALL(&io, 37, &config, command, SIZE_MAX, reply, 4);
    INVALID_CALL(&io, 37, &config, command, 2, NULL, 4);
    INVALID_CALL(&io, 37, &config, command, 2, reply, 0);
    INVALID_CALL(&io, 37, &config, command, 2, reply, SPIDEV_LAB_REPLY_MAX + 1u);
    INVALID_CALL(&io, 37, &config, command, 2, reply, SIZE_MAX);
    invalid.mode = 4;
    INVALID_CALL(&io, 37, &invalid, command, 2, reply, 4);
    invalid = config;
    invalid.speed_hz = 0;
    INVALID_CALL(&io, 37, &invalid, command, 2, reply, 4);
    invalid = config;
    invalid.bits_per_word = 16;
    INVALID_CALL(&io, 37, &invalid, command, 2, reply, 4);
    invalid.bits_per_word = 0;
    INVALID_CALL(&io, 37, &invalid, command, 2, reply, 4);
#undef INVALID_CALL
    return cases;
}

/* Original ideal-edge model, not a simulator of a real controller. Mode table
 * is the independent expected physical sample edge for this finite oracle. */
static unsigned edge_models(void)
{
    const bool rising_sample[4] = {true, false, false, true};
    unsigned cases = 0;
    for (unsigned mode = 0; mode < 4; ++mode)
        for (unsigned value = 0; value < 256; ++value) {
            const bool cpol = (mode & 2u) != 0;
            const bool cpha = (mode & 1u) != 0;
            bool clock = cpol;
            unsigned current_bit = 7;
            unsigned data = cpha ? 0u : ((value >> 7) & 1u);
            unsigned sampled = 0;
            unsigned count = 0;
            for (unsigned e = 1; e <= 16; ++e) {
                const bool leading = (e & 1u) != 0;
                const bool prior_clock = clock;
                clock = !clock;
                const bool rising = !prior_clock && clock;
                const bool sample = cpha ? !leading : leading;
                if (sample) {
                    assert(rising == rising_sample[mode]);
                    sampled = (sampled << 1) | data;
                    ++count;
                } else if (cpha) {
                    current_bit = 7u - (e - 1u) / 2u;
                    data = (value >> current_bit) & 1u;
                } else if (e < 16) {
                    --current_bit;
                    data = (value >> current_bit) & 1u;
                }
            }
            assert(count == 8 && sampled == value && clock == cpol);
            ++cases;
        }
    return cases;
}

int main(void)
{
    uint8_t command[SPIDEV_LAB_COMMAND_MAX];
    for (size_t i = 0; i < sizeof command; ++i) command[i] = (uint8_t)(0x80u + i);
    const unsigned success = success_matrix(command);
    const unsigned failure = failures(command);
    const unsigned invalid = invalid_inputs(command);
    const unsigned edges = edge_models();
    printf("spidev transaction: PASS (%u success, %u injected failure/length/mismatch, "
           "%u invalid-input, %u ideal-edge byte models)\n", success, failure, invalid, edges);
    puts("Scope: mock ioctl and ideal edges only; no /dev, kernel SPI, electrical or hardware experiment.");
    return 0;
}
