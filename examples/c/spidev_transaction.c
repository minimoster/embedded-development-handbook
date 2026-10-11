#include "spidev_transaction.h"

#include <errno.h>
#include <limits.h>
#include <linux/spi/spidev.h>
#include <string.h>

_Static_assert(CHAR_BIT == 8, "This lab requires 8-bit bytes");
_Static_assert(sizeof(uintptr_t) <= sizeof(__u64), "UAPI pointer field too small");
_Static_assert(SPIDEV_LAB_COMMAND_MAX + SPIDEV_LAB_REPLY_MAX <= INT_MAX,
               "Expected ioctl result must fit int");

static enum spidev_lab_status configure(
    const struct spidev_lab_io *io, int fd, unsigned long request,
    void *argument, enum spidev_lab_step step,
    struct spidev_lab_report *report)
{
    report->step = step;
    errno = 0;
    const int result = io->invoke(io->context, fd, request, argument);
    const int saved_errno = errno;
    report->returned = result;
    if (result == -1) {
        report->syscall_errno = saved_errno;
        return SPIDEV_LAB_CONFIG_IOCTL;
    }
    if (result != 0) {
        return SPIDEV_LAB_CONFIG_RETURN;
    }
    return SPIDEV_LAB_OK;
}

enum spidev_lab_status spidev_lab_command_read(
    const struct spidev_lab_io *io, int fd,
    const struct spidev_lab_config *config,
    const uint8_t *command, size_t command_len,
    uint8_t dummy_byte, uint8_t *reply, size_t reply_len,
    struct spidev_lab_report *report)
{
    struct spidev_lab_report local = {0};
    struct spidev_lab_report *r = report != NULL ? report : &local;
    *r = (struct spidev_lab_report){0};
    r->step = SPIDEV_LAB_INPUT;
    if (io == NULL || io->invoke == NULL || fd < 0 || config == NULL ||
        command == NULL || reply == NULL || command_len == 0 || reply_len == 0 ||
        command_len > SPIDEV_LAB_COMMAND_MAX || reply_len > SPIDEV_LAB_REPLY_MAX ||
        config->mode > (uint32_t)SPI_MODE_3 || config->speed_hz == 0 ||
        config->bits_per_word != 8) {
        return SPIDEV_LAB_INVALID;
    }
    r->expected_bytes = (int)(command_len + reply_len);
    uint32_t mode = config->mode;
    uint32_t actual_mode = 0;
    uint8_t word = config->bits_per_word;
    uint8_t actual_word = 0;
    uint32_t speed = config->speed_hz;
    uint32_t actual_speed = 0;
    enum spidev_lab_status status;

    status = configure(io, fd, SPI_IOC_WR_MODE32, &mode, SPIDEV_LAB_WR_MODE, r);
    if (status != SPIDEV_LAB_OK) return status;
    status = configure(io, fd, SPI_IOC_RD_MODE32, &actual_mode, SPIDEV_LAB_RD_MODE, r);
    if (status != SPIDEV_LAB_OK) return status;
    if (actual_mode != config->mode) return SPIDEV_LAB_CONFIG_MISMATCH;

    status = configure(io, fd, SPI_IOC_WR_BITS_PER_WORD, &word, SPIDEV_LAB_WR_WORD, r);
    if (status != SPIDEV_LAB_OK) return status;
    status = configure(io, fd, SPI_IOC_RD_BITS_PER_WORD, &actual_word, SPIDEV_LAB_RD_WORD, r);
    if (status != SPIDEV_LAB_OK) return status;
    if (actual_word != config->bits_per_word) return SPIDEV_LAB_CONFIG_MISMATCH;

    status = configure(io, fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed, SPIDEV_LAB_WR_SPEED, r);
    if (status != SPIDEV_LAB_OK) return status;
    status = configure(io, fd, SPI_IOC_RD_MAX_SPEED_HZ, &actual_speed, SPIDEV_LAB_RD_SPEED, r);
    if (status != SPIDEV_LAB_OK) return status;
    if (actual_speed != config->speed_hz) return SPIDEV_LAB_CONFIG_MISMATCH;

    uint8_t dummy[SPIDEV_LAB_REPLY_MAX] = {0};
    uint8_t received[SPIDEV_LAB_REPLY_MAX] = {0};
    memset(dummy, dummy_byte, reply_len);
    struct spi_ioc_transfer transfers[2];
    /* Includes unused fields/pad. Never send uninitialized UAPI fields. */
    memset(transfers, 0, sizeof transfers);
    transfers[0].tx_buf = (__u64)(uintptr_t)command;
    transfers[0].len = (__u32)command_len;
    transfers[0].speed_hz = config->speed_hz;
    transfers[0].bits_per_word = config->bits_per_word;
    transfers[1].tx_buf = (__u64)(uintptr_t)dummy;
    transfers[1].rx_buf = (__u64)(uintptr_t)received;
    transfers[1].len = (__u32)reply_len;
    transfers[1].speed_hz = config->speed_hz;
    transfers[1].bits_per_word = config->bits_per_word;
    /* Both cs_change fields stay 0: no CS toggle between command and data. */
    r->step = SPIDEV_LAB_MESSAGE;
    errno = 0;
    const int result = io->invoke(io->context, fd, SPI_IOC_MESSAGE(2), transfers);
    const int saved_errno = errno;
    r->returned = result;
    if (result == -1) {
        r->syscall_errno = saved_errno;
        return SPIDEV_LAB_MESSAGE_IOCTL;
    }
    if (result != r->expected_bytes) return SPIDEV_LAB_MESSAGE_LENGTH;
    memcpy(reply, received, reply_len);
    r->step = SPIDEV_LAB_COMPLETE;
    return SPIDEV_LAB_OK;
}
