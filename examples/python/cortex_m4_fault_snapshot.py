"""Decode caller-supplied synthetic Cortex-M4 snapshots without memory access.

A conservative teaching policy rejects automatic frame trust on stacking,
unstacking or lazy-FP preservation faults. It is not an on-target handler.
"""
from dataclasses import dataclass

EXC_RETURNS = {
    0xFFFFFFF1: ("MSP", "Handler", False),
    0xFFFFFFF9: ("MSP", "Thread", False),
    0xFFFFFFFD: ("PSP", "Thread", False),
    0xFFFFFFE1: ("MSP", "Handler", True),
    0xFFFFFFE9: ("MSP", "Thread", True),
    0xFFFFFFED: ("PSP", "Thread", True),
}
CFSR_FLAGS = {
    0: "IACCVIOL", 1: "DACCVIOL", 3: "MUNSTKERR", 4: "MSTKERR",
    5: "MLSPERR", 7: "MMARVALID", 8: "IBUSERR", 9: "PRECISERR",
    10: "IMPRECISERR", 11: "UNSTKERR", 12: "STKERR", 13: "LSPERR",
    15: "BFARVALID", 16: "UNDEFINSTR", 17: "INVSTATE", 18: "INVPC",
    19: "NOCP", 24: "UNALIGNED", 25: "DIVBYZERO",
}
FRAME_RISK_BITS = (3, 4, 5, 11, 12, 13)
CORE_NAMES = ("r0", "r1", "r2", "r3", "r12", "lr", "pc", "xpsr")


def u32(name: str, value: int) -> int:
    if type(value) is not int or not 0 <= value <= 0xFFFFFFFF:
        raise ValueError(f"{name} must be uint32")
    return value


@dataclass(frozen=True)
class Snapshot:
    exc_return: int
    cfsr: int
    hfsr: int
    msp: int
    psp: int
    capture_base: int
    words: tuple[int, ...]
    readable_start: int
    readable_end: int  # exclusive; a caller-declared capture contract, not probed RAM
    bfar: int = 0
    mmfar: int = 0
    fpccr: int = 0

    def __post_init__(self) -> None:
        for name in ("exc_return", "cfsr", "hfsr", "msp", "psp", "capture_base",
                     "readable_start", "readable_end", "bfar", "mmfar", "fpccr"):
            u32(name, getattr(self, name))
        if not isinstance(self.words, tuple) or len(self.words) > 64:
            raise ValueError("capture must be an immutable tuple of at most64 words")
        for word in self.words:
            u32("word", word)
        if self.capture_base % 4 or self.readable_start >= self.readable_end:
            raise ValueError("invalid capture alignment or readable interval")
        capture_end = self.capture_base + 4 * len(self.words)
        if capture_end > 0xFFFFFFFF or not (
                self.readable_start <= self.capture_base <= capture_end <= self.readable_end):
            raise ValueError("capture outside declared readable interval or address overflow")


def decode(snapshot: Snapshot) -> dict:
    """Interpret copied integers only. Never dereference MSP/PSP/BFAR/MMFAR."""
    flags = [name for bit, name in CFSR_FLAGS.items() if snapshot.cfsr & (1 << bit)]
    result = {
        "scope": "synthetic copied Cortex-M4 snapshot; no target access",
        "flags": flags,
        "forced_escalation": bool(snapshot.hfsr & (1 << 30)),
        "vector_table_fault": bool(snapshot.hfsr & (1 << 1)),
        "bfar": snapshot.bfar if snapshot.cfsr & (1 << 15) else None,
        "mmfar": snapshot.mmfar if snapshot.cfsr & (1 << 7) else None,
        "frame_trust": "rejected", "reasons": [], "core": None,
        "stack": None, "return_mode": None, "extended_fp": None,
        "precise_pc_candidate": False,
    }
    layout = EXC_RETURNS.get(snapshot.exc_return)
    if layout is None:
        result["reasons"].append("EXC_RETURN outside this Cortex-M4 whitelist")
        return result
    stack, mode, extended = layout
    result.update(stack=stack, return_mode=mode, extended_fp=extended)
    sp = snapshot.msp if stack == "MSP" else snapshot.psp
    if sp % 4:
        result["reasons"].append("selected SP not word aligned")
        return result
    if any(snapshot.cfsr & (1 << bit) for bit in FRAME_RISK_BITS):
        result["reasons"].append("conservative policy: stack/return/lazy-FP fault requires manual recovery")
        return result
    offset_bytes = sp - snapshot.capture_base
    offset_words = offset_bytes // 4
    core_offset = offset_words  # Cortex-M4: core registers remain at SP+0..0x1c
    minimum_end = core_offset + 8
    if offset_bytes < 0 or minimum_end > len(snapshot.words):
        result["reasons"].append("selected complete frame not in copied capture")
        return result
    core = dict(zip(CORE_NAMES, snapshot.words[core_offset:minimum_end]))
    padding = 1 if core["xpsr"] & (1 << 9) else 0
    full_end = offset_words + (26 if extended else 8) + padding
    if full_end > len(snapshot.words):
        result["reasons"].append("alignment padding missing from copied capture")
        return result
    result.update(core=core, padding_words=padding,
                  frame_bytes=(26 if extended else 8) * 4 + padding * 4,
                  pre_exception_sp=sp + ((26 if extended else 8) + padding) * 4,
                  fp_payload_decoded=False,
                  lazy_fp_active=bool(snapshot.fpccr & 1))
    if not core["xpsr"] & (1 << 24):
        result["reasons"].append("stacked xPSR.T is zero; do not certify return-state consistency")
        return result
    result["frame_trust"] = "structurally_accepted"
    # A deliberately narrow evidence label, not a general root-cause detector.
    precise_only = bool(snapshot.cfsr & (1 << 9)) and not bool(snapshot.cfsr & (1 << 10))
    result["precise_pc_candidate"] = precise_only and not result["vector_table_fault"]
    if snapshot.cfsr & (1 << 10):
        result["reasons"].append("IMPRECISERR: stacked PC is not certified as faulting instruction")
    if result["vector_table_fault"]:
        result["reasons"].append("VECTTBL: stacked PC is interrupted context, not failed vector read")
    return result


def example_snapshots() -> dict[str, dict]:
    base = 0x20001000
    core = (1, 2, 3, 4, 12, 0x08000101, 0x08000200, 1 << 24)
    def sample(cfsr: int, words: tuple[int, ...], exc: int = 0xFFFFFFFD) -> dict:
        return decode(Snapshot(exc, cfsr, 1 << 30, 0x20002000, base, base,
                               words, base, base + 4 * len(words), bfar=0x40000000))
    return {"precise": sample((1 << 9) | (1 << 15), core),
            "imprecise": sample(1 << 10, core),
            "stacking_fault": sample(1 << 12, core),
            "extended": sample(0, core + (0,) * 18, 0xFFFFFFED)}


if __name__ == "__main__":
    import json
    print(json.dumps(example_snapshots(), indent=2))
