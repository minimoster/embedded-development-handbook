#!/usr/bin/env python3
"""Original finite, synthetic Reno/RTO teaching model, not a TCP stack.

Pure cumulative ACKs, full SMSS segments, fixed rwnd, no sequence wrap,
SACK/timestamps/ECN/NewReno/Limited Transmit or real network operations.
Time is exact Fraction seconds; pass integers, decimal strings or Fraction.
Callers schedule events. A timeout is explicit, never a background thread.
"""
from dataclasses import dataclass
from fractions import Fraction
from typing import Optional, Tuple, Union

Time = Union[int, str, Fraction]


def seconds(value: Time) -> Fraction:
    if isinstance(value, Fraction):
        result = value
    elif type(value) is int or type(value) is str:
        try:
            result = Fraction(value)
        except (ValueError, ZeroDivisionError) as error:
            raise ValueError("time must be a finite rational number") from error
    else:
        raise ValueError("use int, decimal string or Fraction, not float/bool")
    if result < 0:
        raise ValueError("negative time")
    return result


def positive_int(value: int, name: str) -> int:
    if type(value) is not int or value <= 0:
        raise ValueError(name + " must be a positive integer")
    return value


@dataclass
class _Segment:
    start: int
    end: int
    last_sent: Fraction
    timeout_count: int = 0
    retransmissions: int = 0


@dataclass(frozen=True)
class State:
    una: int
    nxt: int
    flight: int
    cwnd: int
    ssthresh: int
    dupacks: int
    phase: str
    rto: Fraction
    srtt: Optional[Fraction]
    rttvar: Optional[Fraction]
    deadline: Optional[Fraction]
    now: Fraction
    segments: Tuple[Tuple[int, int, int, int], ...]


@dataclass(frozen=True)
class Effect:
    kind: str
    retransmitted: Optional[int] = None
    sample: Optional[Fraction] = None


class RenoRto:
    """A sender event machine initialized at an established-connection state.

    `cwnd` is a scenario starting value, NOT a recommendation for initial IW.
    State attributes are private; inspect immutable state() results instead.
    Invalid/out-of-model events raise ValueError before changing any state.
    """

    def __init__(self, smss: int = 1000, cwnd: int = 2000,
                 ssthresh: int = 4000, rwnd: int = 64000,
                 initial_rto: Time = 1, max_rto: Time = 60,
                 granularity: Time = "0.001"):
        self.smss = positive_int(smss, "smss")
        self.rwnd = positive_int(rwnd, "rwnd")
        self._cwnd = positive_int(cwnd, "cwnd")
        self._ssthresh = positive_int(ssthresh, "ssthresh")
        self._rto = seconds(initial_rto)
        self.max_rto = seconds(max_rto)
        self.granularity = seconds(granularity)
        if cwnd < smss or ssthresh < 2 * smss or rwnd < smss:
            raise ValueError("cwnd/rwnd >= SMSS and ssthresh >= 2*SMSS required")
        if self._rto < 1 or self.max_rto < 60 or self._rto > self.max_rto:
            raise ValueError("1 <= initial RTO <= maximum; maximum >= 60 s")
        if not 0 < self.granularity <= 1:
            raise ValueError("model clock granularity must be in (0, 1] s")
        self._una = self._nxt = self._dupacks = 0
        self._now = Fraction(0)
        self._deadline: Optional[Fraction] = None
        self._srtt: Optional[Fraction] = None
        self._rttvar: Optional[Fraction] = None
        self._segments = []
        self._fast_recovery = False
        self._probe: Optional[Tuple[int, Fraction]] = None
        self._sample_block_until: Optional[int] = None

    def state(self) -> State:
        phase = "fast-recovery" if self._fast_recovery else (
            "slow-start" if self._cwnd < self._ssthresh else "congestion-avoidance")
        return State(self._una, self._nxt, self._nxt - self._una,
                     self._cwnd, self._ssthresh, self._dupacks, phase,
                     self._rto, self._srtt, self._rttvar, self._deadline,
                     self._now, tuple((s.start, s.end, s.timeout_count,
                                       s.retransmissions) for s in self._segments))

    def _check_time(self, now: Time, expiring: bool = False) -> Fraction:
        time = seconds(now)
        if time < self._now:
            raise ValueError("time moved backwards")
        # At an exact tie the caller chooses ACK-first or timeout-first.
        if not expiring and self._deadline is not None and time > self._deadline:
            raise ValueError("dispatch overdue timeout before another event")
        return time

    def send(self, count: int, now: Time) -> Effect:
        count = positive_int(count, "segment count")
        time = self._check_time(now)
        size = count * self.smss
        if self._nxt - self._una + size > min(self._cwnd, self.rwnd):
            raise ValueError("new data exceeds min(cwnd,rwnd) minus FlightSize")
        if self._nxt + size >= 2**32:
            raise ValueError("sequence wrap is outside this model")
        first_end = self._nxt + self.smss
        for _ in range(count):
            self._segments.append(_Segment(self._nxt, self._nxt + self.smss, time))
            self._nxt += self.smss
        if self._probe is None and self._sample_block_until is None:
            self._probe = (first_end, time)
        if self._deadline is None:
            self._deadline = time + self._rto
        self._now = time
        return Effect("send")

    def _retransmit(self, time: Fraction, timeout: bool) -> int:
        segment = self._segments[0]
        segment.last_sent = time
        segment.retransmissions += 1
        if timeout:
            segment.timeout_count += 1
        # Conservative Karn boundary: discard the probe and inhibit new probes
        # until all data outstanding at this retransmission is acknowledged.
        self._probe = None
        self._sample_block_until = self._nxt
        return segment.start

    def _take_sample(self, sample: Fraction) -> None:
        if self._srtt is None:
            self._srtt = sample
            self._rttvar = sample / 2
        else:
            assert self._rttvar is not None
            old_srtt = self._srtt
            self._rttvar = Fraction(3, 4) * self._rttvar + abs(old_srtt - sample) / 4
            self._srtt = Fraction(7, 8) * old_srtt + sample / 8
        assert self._rttvar is not None
        raw = self._srtt + max(self.granularity, 4 * self._rttvar)
        self._rto = min(self.max_rto, max(Fraction(1), raw))

    def ack(self, number: int, now: Time, *, advertised_window: Optional[int] = None,
            payload_bytes: int = 0, syn: bool = False, fin: bool = False) -> Effect:
        time = self._check_time(now)
        if type(number) is not int or number < 0 or number > self._nxt:
            raise ValueError("ACK must be an integer in [0,SND.NXT]")
        if type(payload_bytes) is not int or payload_bytes != 0 or syn is not False or fin is not False:
            raise ValueError("model accepts established pure ACKs only")
        if advertised_window is not None and (
                type(advertised_window) is not int or advertised_window != self.rwnd):
            raise ValueError("changing rwnd is outside the fixed-window model")
        if number > self._una and number % self.smss != 0:
            raise ValueError("partial-segment ACK is outside this model")

        sample = None
        if number < self._una:
            self._now = time
            return Effect("stale-ack")
        if number == self._una:
            self._now = time
            if not self._segments:
                return Effect("ack-without-flight")
            self._dupacks += 1
            if self._fast_recovery:
                self._cwnd += self.smss
                return Effect("inflate")
            if self._dupacks == 3:
                self._ssthresh = max((self._nxt - self._una) // 2, 2 * self.smss)
                self._cwnd = self._ssthresh + 3 * self.smss
                self._fast_recovery = True
                start = self._retransmit(time, timeout=False)
                # Fast retransmit is allowed before RTO. Protect the following
                # RTO retransmission against firing within one RTO of this send.
                self._deadline = time + self._rto
                return Effect("fast-retransmit", retransmitted=start)
            return Effect("duplicate-ack")

        if self._probe is not None and number >= self._probe[0]:
            sample = time - self._probe[1]
            self._take_sample(sample)
            self._probe = None
        newly_acked = number - self._una
        self._segments = [s for s in self._segments if s.end > number]
        self._una = number
        self._dupacks = 0
        if self._fast_recovery:
            # Classical Reno exits on the first ACK that advances SND.UNA.
            # No NewReno partial-ACK recovery, even if some data remains.
            self._cwnd = self._ssthresh
            self._fast_recovery = False
            kind = "exit-fast-recovery"
        elif self._cwnd < self._ssthresh:
            self._cwnd += min(newly_acked, self.smss)
            kind = "slow-start-ack"
        else:
            self._cwnd += max(1, self.smss * self.smss // self._cwnd)
            kind = "congestion-avoidance-ack"
        if self._sample_block_until is not None and number >= self._sample_block_until:
            self._sample_block_until = None
        self._deadline = time + self._rto if self._segments else None
        self._now = time
        return Effect(kind, sample=sample)

    def expire(self, now: Time) -> Effect:
        time = self._check_time(now, expiring=True)
        if self._deadline is None or not self._segments:
            raise ValueError("no outstanding data timer")
        if time < self._deadline:
            raise ValueError("timer has not reached its deadline")
        if time - self._segments[0].last_sent < self._rto:
            raise ValueError("RTO retransmission would be too early")
        if self._segments[0].timeout_count == 0:
            self._ssthresh = max((self._nxt - self._una) // 2, 2 * self.smss)
        self._cwnd = self.smss
        self._fast_recovery = False
        self._dupacks = 0
        start = self._retransmit(time, timeout=True)
        self._rto = min(self.max_rto, self._rto * 2)
        self._deadline = time + self._rto
        self._now = time
        return Effect("timeout-retransmit", retransmitted=start)


def demo() -> None:
    scenarios = [
        ("slow-start / CA", RenoRto(), [
            ("send", 2, "0"), ("ack", 1000, "0.20"), ("ack", 2000, "0.21"),
            ("send", 4, "0.22"), ("ack", 3000, "0.42"), ("ack", 4000, "0.43"),
            ("ack", 5000, "0.44"), ("ack", 6000, "0.45")]),
        ("single-gap fast recovery", RenoRto(cwnd=6000, ssthresh=8000), [
            ("send", 6, "0"), ("ack", 0, "0.1"), ("ack", 0, "0.2"),
            ("ack", 0, "0.3"), ("ack", 0, "0.4"), ("ack", 0, "0.5"),
            ("ack", 6000, "0.6")]),
        ("timeout / backoff / clean RTT", RenoRto(cwnd=6000, ssthresh=8000), [
            ("send", 6, "0"), ("expire", None, "1"), ("expire", None, "3"),
            ("ack", 6000, "3.2"), ("send", 2, "3.3"),
            ("ack", 7000, "3.5"), ("ack", 8000, "3.51")])]
    print("SYNTHETIC HOST MODEL: no sockets, kernel, packet capture or real network")
    for title, model, events in scenarios:
        print("\n" + title)
        print("time  event(number)                una/nxt  flight  cwnd  ss  dup  RTO  deadline  effect")
        for name, number, time in events:
            if name == "expire":
                effect = model.expire(time)
            else:
                effect = getattr(model, name)(number, time)
            state = model.state()
            deadline = "off" if state.deadline is None else str(state.deadline)
            print(f"{time:>4}  {name}({number})  {state.una}/{state.nxt}  "
                  f"{state.flight}  {state.cwnd}  {state.ssthresh}  {state.dupacks}  "
                  f"{state.rto}  {deadline}  {effect.kind}")


if __name__ == "__main__":
    demo()
