#!/usr/bin/env python3
"""Original finite I2C teaching models. No GPIO, I2C device, or hardware access.

Numeric interval calculations use exact rationals for the rounded UM10204
coefficient 0.8473. Edge traces are ideal digital events, not timed waveforms.
Bus clear uses synthetic integer ticks and fault scripts, not a target driver.
"""
from dataclasses import dataclass
from fractions import Fraction
import json
from typing import Optional, Union

Number = Union[int, float, str, Fraction]
RC_30_70 = Fraction(8473, 10000)


def _q(value: Number) -> Fraction:
    if isinstance(value, bool) or not isinstance(value, (int, float, str, Fraction)):
        raise ValueError("expected a finite numeric quantity")
    try:
        return value if isinstance(value, Fraction) else Fraction(str(value))
    except (ValueError, ZeroDivisionError) as error:
        raise ValueError("expected a finite numeric quantity") from error


@dataclass(frozen=True)
class PullupInterval:
    sink_min_ohm: Fraction
    rise_min_ohm: Fraction
    actual_min_ohm: Fraction
    actual_max_ohm: Fraction
    nominal_min_ohm: Fraction
    nominal_max_ohm: Fraction
    tolerance: Fraction

    @property
    def feasible(self) -> bool:
        return self.nominal_min_ohm <= self.nominal_max_ohm

    def accepts_nominal(self, resistance_ohm: Number) -> bool:
        resistance = _q(resistance_ohm)
        return self.feasible and self.nominal_min_ohm <= resistance <= self.nominal_max_ohm


def pullup_interval(*, vdd_max_v: Number, vol_max_v: Number, iol_ma: Number,
                    cb_min_pf: Number, cb_max_pf: Number,
                    rise_min_ns: Number, rise_max_ns: Number,
                    tolerance: Number = 0) -> PullupInterval:
    """SI-equivalent arithmetic; does not certify leakage, capacitance, or modes.

    iol_ma is the GUARANTEED sink capacity at the selected VOL limit for every
    device that can drive the line, not its absolute maximum or typical value.
    cb_min/max include all fitted pins, wiring and the proposed measurement load.
    Fast-mode's 20 ns rise minimum must be supplied explicitly when applicable.
    """
    vdd, vol, iol, cmin, cmax, tmin, tmax, tol = map(_q, (
        vdd_max_v, vol_max_v, iol_ma, cb_min_pf, cb_max_pf,
        rise_min_ns, rise_max_ns, tolerance))
    if not (vdd > vol >= 0 and iol > 0 and cmax >= cmin > 0
            and tmax >= tmin >= 0 and tmax > 0 and 0 <= tol < 1):
        raise ValueError("invalid voltage/current/capacitance/rise/tolerance bounds")
    sink_min = (vdd - vol) * 1000 / iol
    rise_min = tmin * 1000 / (RC_30_70 * cmin)
    actual_min = max(sink_min, rise_min)
    actual_max = tmax * 1000 / (RC_30_70 * cmax)
    return PullupInterval(sink_min, rise_min, actual_min, actual_max,
                          actual_min / (1 - tol), actual_max / (1 + tol), tol)


def rise_time_ns(resistance_ohm: Number, capacitance_pf: Number) -> Fraction:
    r, c = _q(resistance_ohm), _q(capacitance_pf)
    if r <= 0 or c <= 0:
        raise ValueError("R and C must be positive")
    return RC_30_70 * r * c / 1000


@dataclass(frozen=True)
class Edge:
    sequence: int
    kind: str
    label: str
    bit: Optional[int]
    controller_scl_low: bool
    controller_sda_low: bool
    target_sda_low: bool

    @property
    def scl(self) -> int:
        return int(not self.controller_scl_low)

    @property
    def sda(self) -> int:
        return int(not (self.controller_sda_low or self.target_sda_low))


def _byte(value: int) -> None:
    if isinstance(value, bool) or not isinstance(value, int) or not 0 <= value <= 255:
        raise ValueError("expected a byte")


def combined_read_trace(address: int = 0x50, register: int = 0x12,
                        read_bytes: tuple[int, ...] = (0xA5, 0x3C)) -> tuple[Edge, ...]:
    """Ideal single-controller, all-address/write-ACK combined read.

    Restricts address to ordinary 7-bit target addresses and payload to 1..16
    bytes for finite host exploration. It neither implements addressing on a
    device nor models arbitration, stretching, propagation or analog thresholds.
    """
    if isinstance(address, bool) or not isinstance(address, int) or not 0x08 <= address <= 0x77:
        raise ValueError("expected an ordinary 7-bit target address")
    _byte(register)
    if not isinstance(read_bytes, tuple) or not 1 <= len(read_bytes) <= 16:
        raise ValueError("read_bytes must be a 1..16-byte tuple")
    for value in read_bytes:
        _byte(value)
    result: list[Edge] = []
    cl = cd = td = False

    def emit(kind: str, label: str = "", bit: Optional[int] = None) -> None:
        result.append(Edge(len(result), kind, label, bit, cl, cd, td))

    def start(repeated: bool = False) -> None:
        nonlocal cl, cd, td
        cd = td = False
        if repeated:
            emit("SR_SETUP")  # SCL is low before either transmitter changes SDA.
            cl = False
            emit("SR_SCL_RISE")
        cd = True
        emit("REPEATED_START" if repeated else "START")
        cl = True
        emit("START_SCL_FALL")

    def send(value: int, label: str, transmitter: str, ack: bool) -> None:
        nonlocal cl, cd, td
        for bit in range(7, -1, -1):
            low = not bool(value & (1 << bit))
            cd, td = (low, False) if transmitter == "controller" else (False, low)
            emit("DATA_SETUP", label, bit)
            cl = False
            emit("DATA_RISE", label, bit)
            cl = True
            emit("DATA_FALL", label, bit)
        # Transmitter releases; receiver is the only possible ACK driver.
        cd, td = (False, ack) if transmitter == "controller" else (ack, False)
        emit("ACK_SETUP", label, 8)
        cl = False
        emit("ACK_RISE", label, 8)
        cl = True
        emit("ACK_FALL", label, 8)
        cd = td = False
        emit("ACK_RELEASE", label)

    emit("IDLE")
    start()
    send(address << 1, "address-write", "controller", True)
    send(register, "register", "controller", True)
    start(repeated=True)
    send((address << 1) | 1, "address-read", "controller", True)
    for index, value in enumerate(read_bytes):
        send(value, f"read-{index}", "target", index != len(read_bytes) - 1)
    cd = True  # Form STOP from low SCL, without introducing another START.
    emit("STOP_SETUP")
    cl = False
    emit("STOP_SCL_RISE")
    cd = False
    emit("STOP")
    return tuple(result)


def check_edge_invariants(trace: tuple[Edge, ...]) -> None:
    """Checks local digital edge rules; not a complete I2C protocol validator."""
    if not trace or trace[0].kind != "IDLE" or (trace[0].scl, trace[0].sda) != (1, 1):
        raise ValueError("trace must start idle")
    for index, event in enumerate(trace):
        if event.sequence != index:
            raise ValueError("nonsequential trace")
        if index == 0:
            continue
        before = trace[index - 1]
        if before.scl == event.scl == 1 and before.sda != event.sda:
            required = "STOP" if event.sda else ("START", "REPEATED_START")
            if event.kind != required and not (isinstance(required, tuple) and event.kind in required):
                raise ValueError("unexpected SDA transition with SCL high")
        if event.kind in ("DATA_RISE", "ACK_RISE"):
            if (before.scl, event.scl) != (0, 1) or before.sda != event.sda:
                raise ValueError("sampling edge must preserve prepared SDA")
        if event.kind in ("DATA_FALL", "ACK_FALL"):
            if (before.scl, event.scl) != (1, 0) or before.sda != event.sda:
                raise ValueError("falling edge must precede next SDA change")


@dataclass(frozen=True)
class RecoveryConfig:
    low_ticks: int = 5
    high_ticks: int = 5
    stretch_ticks: int = 10
    stop_setup_ticks: int = 5
    bus_free_ticks: int = 5
    total_ticks: int = 200

    def validate(self) -> None:
        for value in (self.low_ticks, self.high_ticks, self.stretch_ticks,
                      self.stop_setup_ticks, self.bus_free_ticks, self.total_ticks):
            if isinstance(value, bool) or not isinstance(value, int) or value <= 0:
                raise ValueError("all tick budgets must be positive integers")


@dataclass(frozen=True)
class RecoveryFaults:
    # 0: SDA initially free. 1..9: free after that completed clear pulse.
    # None: never releases. This is a synthetic script, not a target state machine.
    sda_release_after: Optional[int] = 3
    # One delay per release/observation attempt: initial, each pulse, then STOP.
    # None holds SCL forever; unspecified trailing attempts have zero delay.
    scl_release_delays: tuple[Optional[int], ...] = ()
    sda_stuck_at_stop: bool = False
    # Relative tick in the tBUF observation window; None means no reassertion.
    idle_sda_low_at: Optional[int] = None

    def validate(self) -> None:
        if self.sda_release_after is not None and (
                isinstance(self.sda_release_after, bool)
                or not isinstance(self.sda_release_after, int)
                or not 0 <= self.sda_release_after <= 9):
            raise ValueError("release_after must be None or an integer in 0..9")
        if not isinstance(self.scl_release_delays, tuple) or len(self.scl_release_delays) > 11:
            raise ValueError("delay script must be a tuple of at most 11 attempts")
        for delay in (*self.scl_release_delays, self.idle_sda_low_at):
            if delay is not None and (isinstance(delay, bool) or not isinstance(delay, int) or delay < 0):
                raise ValueError("delays must be None or nonnegative integers")
        if not isinstance(self.sda_stuck_at_stop, bool):
            raise ValueError("stop fault must be boolean")


@dataclass(frozen=True)
class RecoveryEvent:
    ticks: int
    phase: str
    pulses: int
    controller_scl_low: bool
    controller_sda_low: bool
    external_scl_low: bool
    external_sda_low: bool

    @property
    def scl(self) -> int:
        return int(not (self.controller_scl_low or self.external_scl_low))

    @property
    def sda(self) -> int:
        return int(not (self.controller_sda_low or self.external_sda_low))


@dataclass(frozen=True)
class RecoveryResult:
    status: str
    pulses: int
    ticks: int
    trace: tuple[RecoveryEvent, ...]


def bus_clear_model(faults: RecoveryFaults = RecoveryFaults(),
                    config: RecoveryConfig = RecoveryConfig(), *,
                    exclusive_owner: bool = True,
                    open_drain_pins: bool = True) -> RecoveryResult:
    """At most nine clear pulses, one STOP attempt, and a finite idle check.

    The separate STOP preparation can create a further SCL rising edge. Cleanup
    releases pins even on failure; this is not a push-pull HIGH command. Tick
    deadlines apply to this model only. Real callbacks, scheduling and pin modes
    require separate board/adapter validation.
    """
    config.validate()
    faults.validate()
    if not isinstance(exclusive_owner, bool) or not isinstance(open_drain_pins, bool):
        raise ValueError("preconditions must be boolean")
    if not exclusive_owner or not open_drain_pins:
        return RecoveryResult("REFUSED", 0, 0, ())
    now = pulses = attempt = 0
    cl = cd = False
    first_delay = faults.scl_release_delays[0] if faults.scl_release_delays else 0
    el = first_delay is None or first_delay > 0
    ed = faults.sda_release_after != 0
    trace: list[RecoveryEvent] = []

    def emit(phase: str) -> None:
        trace.append(RecoveryEvent(now, phase, pulses, cl, cd, el, ed))

    def finish(status: str) -> RecoveryResult:
        nonlocal cl, cd
        cl = cd = False
        emit("cleanup-release")
        return RecoveryResult(status, pulses, now, tuple(trace))

    def advance(ticks: int, phase: str) -> bool:
        nonlocal now
        if now + ticks >= config.total_ticks:
            now = config.total_ticks
            emit("deadline")
            return False
        now += ticks
        emit(phase)
        return True

    def high(phase: str) -> Optional[str]:
        nonlocal cl, el, attempt, now
        delay = faults.scl_release_delays[attempt] if attempt < len(faults.scl_release_delays) else 0
        attempt += 1
        cl = False
        el = delay is None or delay > 0
        begin = now
        emit(phase + "-release-scl")
        # Valid observations are strictly before both deadlines. A level first
        # visible at exactly a deadline is too late, even if it could be sampled.
        while now < begin + config.stretch_ticks and now < config.total_ticks:
            if delay is not None and now - begin >= delay:
                el = False
                emit(phase + "-scl-high")
                return None
            now += 1
            emit(phase + "-wait-scl")
        return "DEADLINE" if now >= config.total_ticks else "SCL_TIMEOUT"

    def idle() -> Optional[str]:
        nonlocal ed
        for tick in range(config.bus_free_ticks + 1):
            if faults.idle_sda_low_at is not None and tick >= faults.idle_sda_low_at:
                ed = True
            emit("idle-observe")
            if cl or cd or el or ed:
                return "BUS_NOT_IDLE"
            if tick < config.bus_free_ticks and not advance(1, "idle-tick"):
                return "DEADLINE"
        return None

    emit("initial-release")
    error = high("initial")
    if error:
        return finish(error)
    if not ed:
        error = idle()
        return finish(error or "ALREADY_IDLE")
    for pulse in range(1, 10):
        cl = True
        emit("clear-low")
        if not advance(config.low_ticks, "clear-low-held"):
            return finish("DEADLINE")
        error = high("clear")
        if error:
            return finish(error)
        emit("clear-rise")
        if not advance(config.high_ticks, "clear-high-held"):
            return finish("DEADLINE")
        pulses = pulse
        # Target-release script is applied only in LOW, never as a data
        # transition in HIGH. Leave SCL HIGH after an unreleased ninth pulse,
        # so failure cleanup does not manufacture a tenth clear clock.
        releases_now = faults.sda_release_after is not None and pulse >= faults.sda_release_after
        if pulse < 9 or releases_now:
            cl = True
            if releases_now:
                ed = False
            emit("clear-fall")
        else:
            emit("clear-budget-end")
        if not ed:
            break
    if ed:
        return finish("SDA_STUCK")
    # Successful SDA release is observed with SCL LOW. Prepare exactly one STOP.
    cd = True
    emit("stop-sda-low")
    if not advance(config.low_ticks, "stop-low-held"):
        return finish("DEADLINE")
    error = high("stop")
    if error:
        return finish(error)
    if not advance(config.stop_setup_ticks, "stop-setup-held"):
        return finish("DEADLINE")
    if faults.sda_stuck_at_stop:
        ed = True
    cd = False
    emit("stop-release-sda")
    if ed:
        return finish("STOP_FAILED")
    error = idle()
    return finish(error or "RECOVERED")


def main() -> None:
    interval = pullup_interval(vdd_max_v="3.465", vol_max_v="0.4", iol_ma=3,
        cb_min_pf=80, cb_max_pf=120, rise_min_ns=20, rise_max_ns=300, tolerance="0.05")
    trace = combined_read_trace()
    check_edge_invariants(trace)
    samples = [(e.label, e.sda) for e in trace if e.kind == "ACK_RISE"]
    scenarios = {
        "already_idle": RecoveryFaults(sda_release_after=0),
        "release_after_3": RecoveryFaults(sda_release_after=3),
        "scl_timeout": RecoveryFaults(scl_release_delays=(None,)),
        "sda_stuck": RecoveryFaults(sda_release_after=None),
        "stop_failed": RecoveryFaults(sda_stuck_at_stop=True),
    }
    print(json.dumps({
        "scope": "finite host model; no electrical or board tests",
        "nominal_pullup_ohm": [float(interval.nominal_min_ohm), float(interval.nominal_max_ohm)],
        "accepts_2200_ohm": interval.accepts_nominal(2200),
        "combined_read_ack_samples": samples,
        "recovery": {name: {"status": result.status, "pulses": result.pulses,
                             "ticks": result.ticks}
                     for name, faults in scenarios.items()
                     for result in [bus_clear_model(faults)]},
    }, ensure_ascii=False, indent=2))


if __name__ == "__main__":
    main()
