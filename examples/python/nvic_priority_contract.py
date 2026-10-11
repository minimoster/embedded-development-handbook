"""Finite priority arithmetic and a narrowly scoped FreeRTOS priority gate.

No registers, hardware, IRQ entry or kernel execution. The grouping arithmetic
is checked against CMSIS-Core 6.1.0; the gate models the pinned V11.1.0
single-core GCC ARM_CM4F port for 1..7 implemented priority bits only.
"""
from dataclasses import dataclass


def _integer(name: str, value: int, low: int, high: int) -> int:
    if type(value) is not int or not low <= value <= high:
        raise ValueError(f"{name} must be an integer in [{low}, {high}]")
    return value


def priority_partition(bits: int, group: int) -> tuple[int, int]:
    """Return implemented preemption/subpriority bit counts, not HAL names."""
    _integer("bits", bits, 1, 7)
    _integer("PRIGROUP", group, 0, 7)
    preemption = min(bits, 7 - group)
    return preemption, bits - preemption


def encode_logical(bits: int, group: int, preemption: int, subpriority: int) -> int:
    """Audit inputs strictly; unlike CMSIS, do not silently mask oversized fields."""
    pre_bits, sub_bits = priority_partition(bits, group)
    _integer("preemption", preemption, 0, (1 << pre_bits) - 1)
    _integer("subpriority", subpriority, 0, (1 << sub_bits) - 1)
    return (preemption << sub_bits) | subpriority


def decode_logical(bits: int, group: int, logical: int) -> tuple[int, int]:
    pre_bits, sub_bits = priority_partition(bits, group)
    _integer("logical", logical, 0, (1 << bits) - 1)
    return logical >> sub_bits, logical & ((1 << sub_bits) - 1)


def to_register_byte(bits: int, logical: int) -> int:
    _integer("bits", bits, 1, 7)
    _integer("logical", logical, 0, (1 << bits) - 1)
    return logical << (8 - bits)


def from_register_byte(bits: int, raw: int) -> int:
    _integer("bits", bits, 1, 7)
    _integer("raw", raw, 0, 255)
    if raw & ((1 << (8 - bits)) - 1):
        raise ValueError("raw byte contains unimplemented low bits")
    return raw >> (8 - bits)


def cmsis_shift_demo(bits: int, supplied_priority: int) -> int:
    """Pure integer illustration of the CMSIS write's shift-and-byte-mask.

    It intentionally accepts an oversized uint32 argument to expose an input
    mistake; it never writes a register and does not replace strict auditing.
    """
    _integer("bits", bits, 1, 7)
    _integer("supplied_priority", supplied_priority, 0, 0xFFFFFFFF)
    return (supplied_priority << (8 - bits)) & 0xFF


def can_preempt(bits: int, group: int, candidate: int, active: int) -> bool:
    """Priority relation only; ignores enable/pending/active and execution state."""
    candidate_pre, _ = decode_logical(bits, group, candidate)
    active_pre, _ = decode_logical(bits, group, active)
    return candidate_pre < active_pre


@dataclass(frozen=True)
class PortPriorityConfig:
    bits: int
    group: int
    max_syscall_register: int

    def __post_init__(self) -> None:
        _, sub_bits = priority_partition(self.bits, self.group)
        if sub_bits:
            raise ValueError("this 1..7-bit ARM_CM4F teaching gate requires no subpriority bits")
        logical = from_register_byte(self.bits, self.max_syscall_register)
        if logical == 0:
            raise ValueError("zero BASEPRI threshold does not protect kernel state")

    @property
    def max_syscall_logical(self) -> int:
        return from_register_byte(self.bits, self.max_syscall_register)


def basepri_masks(config: PortPriorityConfig, irq_logical: int) -> bool:
    """Mask relation for configurable external IRQs in this valid grouping."""
    raw = to_register_byte(config.bits, irq_logical)
    return raw >= config.max_syscall_register


def fromisr_priority_gate(config: PortPriorityConfig, irq_logical: int) -> bool:
    """Pass this priority gate only; NOT a certificate for an entire API call.

    Other requirements include the selected API's ISR contract, object lifetime,
    initialization, arguments and overflow policy. NMI/HardFault, task context,
    M0/M0+, SMP, MPU wrappers and eight-bit priorities are outside this model.
    """
    return basepri_masks(config, irq_logical)


def example_table() -> list[dict]:
    config = PortPriorityConfig(4, 3, 5 << 4)
    rows = []
    for name, logical in (("urgent acquisition", 2), ("UART handoff", 5),
                          ("background completion", 10)):
        rows.append({"name": name, "logical": logical,
                     "register": f"0x{to_register_byte(4, logical):02x}",
                     "BASEPRI_masks": basepri_masks(config, logical),
                     "FromISR_priority_gate": fromisr_priority_gate(config, logical)})
    return rows


if __name__ == "__main__":
    import json
    print(json.dumps({"scope": "finite arithmetic, no hardware/kernel execution",
                      "table": example_table(),
                      "wrong_0x50_as_logical_becomes": f"0x{cmsis_shift_demo(4, 0x50):02x}",
                      "same_preemption_different_subpriority":
                      can_preempt(4, 5, encode_logical(4, 5, 1, 0),
                                  encode_logical(4, 5, 1, 3))}, indent=2))
