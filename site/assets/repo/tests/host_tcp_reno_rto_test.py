#!/usr/bin/env python3
"""Finite synthetic sender tests; no socket or kernel/packet-capture experiment."""
import copy
import importlib.util
from pathlib import Path
from fractions import Fraction as F
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
MODEL_PATH = ROOT / "examples/python/tcp_reno_rto.py"
spec = importlib.util.spec_from_file_location("tcp_reno_rto", MODEL_PATH)
assert spec is not None and spec.loader is not None
module = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = module
spec.loader.exec_module(module)
RenoRto = module.RenoRto


class HostTcpRenoRtoTest(unittest.TestCase):
    def invariant(self, model):
        state = model.state()
        self.assertEqual(state.flight, state.nxt - state.una)
        self.assertEqual(state.flight, len(state.segments) * model.smss)
        self.assertGreaterEqual(state.cwnd, model.smss)
        self.assertGreaterEqual(state.ssthresh, 2 * model.smss)
        self.assertEqual(state.deadline is None, state.flight == 0)
        self.assertLessEqual(state.rto, model.max_rto)
        self.assertGreaterEqual(state.rto, 1)
        if state.segments:
            self.assertEqual(state.segments[0][0], state.una)
            self.assertEqual(state.segments[-1][1], state.nxt)
            for i, segment in enumerate(state.segments):
                self.assertEqual(segment[1] - segment[0], model.smss)
                if i:
                    self.assertEqual(state.segments[i - 1][1], segment[0])

    def invalid_unchanged(self, model, action):
        previous = copy.deepcopy(vars(model))
        with self.assertRaises(ValueError):
            action()
        self.assertEqual(vars(model), previous)
        self.invariant(model)

    def test_slow_start_ca_hand_trace(self):
        model = RenoRto()
        model.send(2, 0)
        self.assertEqual(model.state().deadline, 1)
        expected = [(1000, "0.20", 3000, "slow-start-ack"),
                    (2000, "0.21", 4000, "slow-start-ack")]
        for number, time, cwnd, kind in expected:
            self.assertEqual(model.ack(number, time).kind, kind)
            self.assertEqual(model.state().cwnd, cwnd)
            self.invariant(model)
        self.assertEqual(model.state().srtt, F(1, 5))
        self.assertEqual(model.state().rttvar, F(1, 10))
        self.assertIsNone(model.state().deadline)
        self.assertEqual(model.state().phase, "congestion-avoidance")
        model.send(4, "0.22")
        for number, time, cwnd in [(3000, "0.42", 4250), (4000, "0.43", 4485),
                                   (5000, "0.44", 4707), (6000, "0.45", 4919)]:
            self.assertEqual(model.ack(number, time).kind, "congestion-avoidance-ack")
            self.assertEqual(model.state().cwnd, cwnd)
            self.invariant(model)
        self.assertEqual(model.state().ssthresh, 4000)
        self.assertEqual(model.state().rttvar, F(3, 40))
        self.assertEqual(model.state().rto, 1)

    def test_cumulative_ack_does_not_grow_once_per_segment(self):
        slow = RenoRto(cwnd=4000, ssthresh=8000)
        slow.send(4, 0)
        slow.ack(4000, "0.2")
        self.assertEqual(slow.state().cwnd, 5000)
        ca = RenoRto(cwnd=4000, ssthresh=4000)
        ca.send(4, 0)
        ca.ack(4000, "0.2")
        self.assertEqual(ca.state().cwnd, 4250)

    def test_single_gap_fast_recovery_hand_trace(self):
        model = RenoRto(cwnd=6000, ssthresh=8000)
        model.send(6, 0)
        for time in ["0.1", "0.2"]:
            self.assertEqual(model.ack(0, time).kind, "duplicate-ack")
            self.assertEqual(model.state().cwnd, 6000)
            self.assertEqual(model.state().deadline, 1)
        effect = model.ack(0, "0.3")
        self.assertEqual((effect.kind, effect.retransmitted), ("fast-retransmit", 0))
        self.assertEqual((model.state().ssthresh, model.state().cwnd), (3000, 6000))
        self.assertEqual(model.state().deadline, F(13, 10))
        for time, cwnd in [("0.4", 7000), ("0.5", 8000)]:
            self.assertEqual(model.ack(0, time).kind, "inflate")
            self.assertEqual(model.state().cwnd, cwnd)
            self.assertEqual(model.state().flight, 6000)
            self.assertEqual(model.state().deadline, F(13, 10))
            self.invariant(model)
        effect = model.ack(6000, "0.6")
        self.assertEqual(effect.kind, "exit-fast-recovery")
        self.assertIsNone(effect.sample)
        self.assertIsNone(model.state().srtt)
        self.assertEqual((model.state().cwnd, model.state().ssthresh), (3000, 3000))
        self.assertEqual(model.state().flight, 0)
        self.assertIsNone(model.state().deadline)

    def test_flight_size_not_cwnd_and_two_smss_floor(self):
        # Scenario starts application-limited: cwnd is much larger than flight.
        for flight, threshold in [(3, 2000), (6, 3000)]:
            model = RenoRto(cwnd=12000, ssthresh=16000)
            model.send(flight, 0)
            for time in ["0.1", "0.2", "0.3"]:
                model.ack(0, time)
            self.assertEqual(model.state().ssthresh, threshold)
            self.assertEqual(model.state().cwnd, threshold + 3000)

    def test_timeout_backoff_and_karn_hand_trace(self):
        model = RenoRto(cwnd=6000, ssthresh=8000)
        model.send(6, 0)
        model.expire(1)
        self.assertEqual((model.state().cwnd, model.state().ssthresh), (1000, 3000))
        self.assertEqual((model.state().rto, model.state().deadline), (2, 3))
        model.expire(3)
        self.assertEqual((model.state().cwnd, model.state().ssthresh), (1000, 3000))
        self.assertEqual((model.state().rto, model.state().deadline), (4, 7))
        self.assertEqual(model.state().segments[0][2:], (2, 2))
        effect = model.ack(6000, "3.2")
        self.assertIsNone(effect.sample)
        self.assertIsNone(model.state().srtt)
        self.assertEqual(model.state().rto, 4)  # ACK alone cannot collapse backoff.
        self.assertEqual(model.state().cwnd, 2000)
        self.assertIsNone(model.state().deadline)
        model.send(2, "3.3")
        self.assertEqual(model.state().deadline, F(73, 10))
        effect = model.ack(7000, "3.5")
        self.assertEqual(effect.sample, F(1, 5))
        self.assertEqual((model.state().srtt, model.state().rttvar), (F(1, 5), F(1, 10)))
        self.assertEqual((model.state().cwnd, model.state().rto, model.state().deadline),
                         (3000, 1, F(9, 2)))
        model.ack(8000, "3.51")
        self.assertEqual(model.state().cwnd, 3333)
        self.invariant(model)

    def test_repeated_timeout_same_segment_holds_ssthresh_after_partial_progress(self):
        model = RenoRto(cwnd=10000, ssthresh=16000)
        model.send(10, 0)
        model.expire(1)
        self.assertEqual(model.state().ssthresh, 5000)
        model.ack(1000, "1.2")
        self.assertEqual(model.state().flight, 9000)
        model.expire("3.2")
        # The new oldest segment has not previously timed out: recompute for it.
        self.assertEqual(model.state().ssthresh, 4500)
        model.expire("7.2")
        self.assertEqual(model.state().ssthresh, 4500)

    def test_rttvar_uses_old_srtt_and_minimum_rto(self):
        model = RenoRto(initial_rto=3)
        model.send(1, 0)
        model.ack(1000, 1)
        self.assertEqual((model.state().srtt, model.state().rttvar, model.state().rto),
                         (1, F(1, 2), 3))
        model.send(1, 1)
        model.ack(2000, 4)  # At exact deadline, this scenario chooses ACK-first.
        self.assertEqual((model.state().srtt, model.state().rttvar, model.state().rto),
                         (F(5, 4), F(7, 8), F(19, 4)))
        minimum = RenoRto()
        minimum.send(1, 0)
        minimum.ack(1000, "0.01")
        self.assertEqual(minimum.state().rto, 1)
        coarse = RenoRto(granularity="0.8")
        coarse.send(1, 0)
        coarse.ack(1000, "0.3")
        self.assertEqual(coarse.state().rto, F(11, 10))

    def test_rto_cap_and_finite_repeated_expiry(self):
        model = RenoRto(cwnd=4000, ssthresh=8000)
        model.send(4, 0)
        expected = [2, 4, 8, 16, 32, 60, 60, 60]
        for rto in expected:
            old = model.state()
            effect = model.expire(old.deadline)
            self.assertEqual(effect.retransmitted, 0)
            self.assertEqual(model.state().rto, rto)
            self.assertEqual(model.state().deadline, old.deadline + rto)
            self.assertEqual(model.state().ssthresh, 2000)
            self.invariant(model)

    def test_timer_no_restart_on_send_or_duplicate_and_restart_on_new_ack(self):
        model = RenoRto(cwnd=4000, ssthresh=8000)
        model.send(2, 0)
        model.send(1, "0.2")
        model.ack(0, "0.3")
        self.assertEqual(model.state().deadline, 1)
        model.ack(1000, "0.4")
        # First sample 0.4 -> raw RTO 1.2, so new deadline is 1.6.
        self.assertEqual(model.state().rto, F(6, 5))
        self.assertEqual(model.state().deadline, F(8, 5))
        self.assertEqual(model.state().dupacks, 0)
        model.ack(3000, "0.5")
        self.assertIsNone(model.state().deadline)

    def test_fast_retransmit_protects_following_rto(self):
        model = RenoRto(cwnd=4000, ssthresh=8000)
        model.send(4, 0)
        for time in ["0.1", "0.2", "0.9"]:
            model.ack(0, time)
        self.assertEqual(model.state().deadline, F(19, 10))
        self.invalid_unchanged(model, lambda: model.expire(1))
        model.expire("1.9")
        self.assertEqual(model.state().phase, "slow-start")
        self.assertEqual(model.state().rto, 2)

    def test_reno_partial_ack_exits_without_newreno(self):
        # Deliberately explores the chosen algorithm's edge, not a claim that
        # this is an efficient multiple-loss recovery implementation.
        model = RenoRto(cwnd=6000, ssthresh=8000)
        model.send(6, 0)
        for time in ["0.1", "0.2", "0.3"]:
            model.ack(0, time)
        effect = model.ack(4000, "0.4")
        self.assertEqual(effect.kind, "exit-fast-recovery")
        self.assertEqual(model.state().flight, 2000)
        self.assertEqual(model.state().phase, "congestion-avoidance")
        self.assertEqual(model.state().cwnd, 3000)
        self.assertIsNone(effect.sample)
        self.assertEqual(model.state().segments[0][0], 4000)

    def test_karn_cumulative_ack_covers_retransmitted_and_clean_data(self):
        model = RenoRto(cwnd=6000, ssthresh=8000)
        model.send(6, 0)
        for time in ["0.1", "0.2", "0.3", "0.4", "0.5"]:
            model.ack(0, time)
        # Inflated window permits new data, but recovery-period probes are off.
        model.send(2, "0.55")
        effect = model.ack(8000, "0.75")
        self.assertIsNone(effect.sample)
        self.assertIsNone(model.state().srtt)
        model.send(1, "0.8")
        self.assertEqual(model.ack(9000, 1).sample, F(1, 5))

    def test_stale_ack_and_ack_without_outstanding_do_not_count_as_duplicate(self):
        model = RenoRto()
        self.assertEqual(model.ack(0, 0).kind, "ack-without-flight")
        self.assertEqual(model.state().dupacks, 0)
        model.send(2, 0)
        model.ack(1000, "0.1")
        model.ack(1000, "0.2")
        before = model.state()
        self.assertEqual(model.ack(0, "0.3").kind, "stale-ack")
        self.assertEqual(model.state().dupacks, before.dupacks)
        self.assertEqual(model.state().deadline, before.deadline)

    def test_illegal_ack_shape_bounds_and_atomic_rejection(self):
        model = RenoRto()
        model.send(2, 0)
        actions = [lambda: model.ack(3000, "0.1"),
                   lambda: model.ack(-1, "0.1"),
                   lambda: model.ack(True, "0.1"),
                   lambda: model.ack(500, "0.1"),
                   lambda: model.ack(0, "0.1", advertised_window=32000),
                   lambda: model.ack(0, "0.1", advertised_window=64000.0),
                   lambda: model.ack(0, "0.1", payload_bytes=1),
                   lambda: model.ack(0, "0.1", payload_bytes=False),
                   lambda: model.ack(0, "0.1", syn=True),
                   lambda: model.ack(0, "0.1", fin=True)]
        for action in actions:
            self.invalid_unchanged(model, action)
        self.assertEqual(model.state().dupacks, 0)
        self.assertEqual(model.ack(0, "0.1", advertised_window=64000).kind, "duplicate-ack")

    def test_illegal_send_timer_and_time_boundaries(self):
        model = RenoRto()
        self.invalid_unchanged(model, lambda: model.expire(0))
        for count in [0, -1, True, 1.5, 3]:
            self.invalid_unchanged(model, lambda count=count: model.send(count, 0))
        model.send(2, 0)
        for time in [-1, "nan", "inf", "1/0", 0.1, True]:
            self.invalid_unchanged(model, lambda time=time: model.ack(0, time))
        self.invalid_unchanged(model, lambda: model.expire("0.999"))
        self.invalid_unchanged(model, lambda: model.ack(1000, "1.001"))
        self.invalid_unchanged(model, lambda: model.send(1, "1.001"))
        model.expire(1)
        self.invalid_unchanged(model, lambda: model.ack(0, "0.9"))
        self.invalid_unchanged(model, lambda: model.send(1, "1.1"))
        self.invalid_unchanged(model, lambda: model.expire(1))

    def test_exact_deadline_tie_is_caller_defined(self):
        ack_first = RenoRto()
        ack_first.send(1, 0)
        ack_first.ack(1000, 1)
        self.assertEqual(ack_first.state().cwnd, 3000)
        self.assertIsNone(ack_first.state().deadline)
        timeout_first = RenoRto()
        timeout_first.send(1, 0)
        timeout_first.expire(1)
        effect = timeout_first.ack(1000, 1)
        self.assertIsNone(effect.sample)
        self.assertEqual(timeout_first.state().cwnd, 2000)
        self.assertEqual(timeout_first.state().rto, 2)

    def test_receive_window_limits_new_send_independently_of_cwnd(self):
        model = RenoRto(cwnd=12000, ssthresh=16000, rwnd=2000)
        self.invalid_unchanged(model, lambda: model.send(3, 0))
        model.send(2, 0)
        self.assertEqual((model.state().cwnd, model.state().flight), (12000, 2000))
        self.invalid_unchanged(model, lambda: model.send(1, "0.1"))
        model.ack(1000, "0.2")
        model.send(1, "0.3")
        self.assertEqual(model.state().flight, 2000)

    def test_sequence_wrap_refused_without_allocating_billions_of_segments(self):
        model = RenoRto(smss=2**31, cwnd=2**32, ssthresh=2**33, rwnd=2**32)
        model.send(1, 0)
        self.invalid_unchanged(model, lambda: model.send(1, 0))

    def test_invalid_configuration(self):
        cases = [dict(smss=0), dict(smss=True), dict(cwnd=999), dict(ssthresh=1999),
                 dict(rwnd=999), dict(initial_rto="0.9"), dict(max_rto=59),
                 dict(initial_rto=61), dict(granularity=0), dict(granularity=2)]
        for options in cases:
            with self.subTest(options=options), self.assertRaises(ValueError):
                RenoRto(**options)


if __name__ == "__main__":
    print("Scope: finite synthetic host sender; no kernel, real network or packet capture.", flush=True)
    unittest.main(verbosity=2)
