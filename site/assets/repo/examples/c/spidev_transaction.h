#ifndef SPIDEV_TRANSACTION_H
#define SPIDEV_TRANSACTION_H

#include <stddef.h>
#include <stdint.h>

#define SPIDEV_LAB_COMMAND_MAX 16u
#define SPIDEV_LAB_REPLY_MAX 64u

/* libc ioctl convention: 0/config or byte count/message; -1 with errno on error.
 * The callback is synchronous, does not retain pointers, and does not re-enter.
 * No real syscall adapter, open(), device path, or hardware main is supplied. */
struct spidev_lab_io {
    int (*invoke)(void *context, int fd, unsigned long request, void *argument);
    void *context;
};

struct spidev_lab_config {
    uint32_t mode;        /* Only SPI_MODE_0..3; MSB first, ordinary 4-wire SPI. */
    uint32_t speed_hz;    /* Nonzero requested maximum, not measured wire speed. */
    uint8_t bits_per_word; /* Exactly 8 in this deliberately narrow example. */
};

enum spidev_lab_status {
    SPIDEV_LAB_OK,
    SPIDEV_LAB_INVALID,
    SPIDEV_LAB_CONFIG_IOCTL,
    SPIDEV_LAB_CONFIG_RETURN,
    SPIDEV_LAB_CONFIG_MISMATCH,
    SPIDEV_LAB_MESSAGE_IOCTL,
    SPIDEV_LAB_MESSAGE_LENGTH
};

enum spidev_lab_step {
    SPIDEV_LAB_INPUT,
    SPIDEV_LAB_WR_MODE,
    SPIDEV_LAB_RD_MODE,
    SPIDEV_LAB_WR_WORD,
    SPIDEV_LAB_RD_WORD,
    SPIDEV_LAB_WR_SPEED,
    SPIDEV_LAB_RD_SPEED,
    SPIDEV_LAB_MESSAGE,
    SPIDEV_LAB_COMPLETE
};

struct spidev_lab_report {
    enum spidev_lab_step step;
    int syscall_errno; /* Only nonzero for a callback returning -1. */
    int returned;      /* Last callback result, or 0 before any callback. */
    int expected_bytes; /* Command length + reply length; full duplex counts once. */
};

/* A complete synthetic command+read transaction: checked configuration, then
 * one SPI_IOC_MESSAGE(2), both cs_change=0. The second transfer sends dummy_byte
 * while receiving. Caller owns the device/configuration across the whole call.
 * cmd and reply spans must be readable/writable for their given lengths and
 * stay alive; io/config/command must not be changed concurrently.
 * Failure leaves reply unchanged, but cannot undo device/configuration effects.
 * No retry, acknowledgement, timeout, or target-specific register is assumed.
 * report may be NULL; otherwise it must be an independent writable object,
 * not overlapping command, reply, config, or io.
 * Success is byte-count completion, not device acceptance. */
enum spidev_lab_status spidev_lab_command_read(
    const struct spidev_lab_io *io, int fd,
    const struct spidev_lab_config *config,
    const uint8_t *command, size_t command_len,
    uint8_t dummy_byte, uint8_t *reply, size_t reply_len,
    struct spidev_lab_report *report);

#endif
