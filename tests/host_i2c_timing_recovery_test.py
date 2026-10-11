#!/usr/bin/env python3
"""Finite host model tests only. No hardware/board/electrical verification."""
from dataclasses import replace
from fractions import Fraction
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "examples/python"))
from i2c_timing_recovery import (  # noqa: E402
    RecoveryConfig, RecoveryFaults, bus_clear_model, check_edge_invariants,
    combined_read_trace, pullup_interval, rise_time_ns,
)


def interval(**changes):
    parameters = dict(vdd_max_v="3.465", vol_max_v="0.4", iol_ma=3,
                      cb_min_pf=80, cb_max_pf=120, rise_min_ns=20,
                      rise_max_ns=300, tolerance="0.05")
    return pullup_interval(**(parameters | changes))


class PullupTests(unittest.TestCase):
    def test_legal_interval_and_nominal_corners(self):
        bounds = interval()
        self.assertTrue(bounds.feasible)
        self.assertAlmostEqual(float(bounds.nominal_min_ohm), 1075.438596491228)
        self.assertAlmostEqual(float(bounds.nominal_max_ohm), 2810.046478168749)
        self.assertTrue(bounds.accepts_nominal(2200))
        self.assertFalse(bounds.accepts_nominal(1000))
        self.assertFalse(bounds.accepts_nominal(3300))
        # Independently evaluate BOTH tolerance corners of the selected part.
        self.assertLessEqual((Fraction("3.465") - Fraction("0.4")) * 1000
                             / (2200 * Fraction("0.95")), 3)
        self.assertGreaterEqual(rise_time_ns(2200 * Fraction("0.95"), 80), 20)
        self.assertLessEqual(rise_time_ns(2200 * Fraction("1.05"), 120), 300)

    def test_empty_interval_has_no_legal_resistance(self):
        bounds = interval(vdd_max_v="5.5", cb_min_pf=400, cb_max_pf=400, tolerance=0)
        self.assertEqual(bounds.sink_min_ohm, 1700)
        self.assertAlmostEqual(float(bounds.actual_max_ohm), 885.164640623156)
        self.assertFalse(bounds.feasible)
        for candidate in (330, 680, 885, 1000, 1700, 2200, 10000):
            self.assertFalse(bounds.accepts_nominal(candidate))

    def test_single_point_boundary_and_one_decimal_step_outside(self):
        # 0.8473 * 1000 ohm * 100 pF = exactly 84.73 ns in this model.
        bounds = interval(vdd_max_v="3.4", cb_min_pf=100, cb_max_pf=100,
                          rise_min_ns=0, rise_max_ns="84.73", tolerance=0)
        self.assertEqual(bounds.actual_min_ohm, 1000)
        self.assertEqual(bounds.actual_max_ohm, 1000)
        self.assertTrue(bounds.accepts_nominal(1000))
        self.assertFalse(bounds.accepts_nominal("999.999"))
        self.assertFalse(bounds.accepts_nominal("1000.001"))
        self.assertFalse(interval(vdd_max_v="3.4", cb_min_pf=100, cb_max_pf=100,
            rise_min_ns=0, rise_max_ns="84.729", tolerance=0).feasible)
        self.assertFalse(interval(vdd_max_v="3.4", cb_min_pf=100, cb_max_pf=100,
            rise_min_ns=0, rise_max_ns="84.73", tolerance="0.01").feasible)

    def test_rise_minimum_can_be_active_lower_bound(self):
        bounds = interval(vdd_max_v="3.3", iol_ma=20, cb_min_pf=10, cb_max_pf=100,
                          rise_min_ns=20, rise_max_ns=300, tolerance=0)
        self.assertGreater(bounds.rise_min_ohm, bounds.sink_min_ohm)
        self.assertEqual(bounds.actual_min_ohm, bounds.rise_min_ohm)
        self.assertFalse(bounds.accepts_nominal(1000))
        self.assertTrue(bounds.accepts_nominal(3000))

    def test_named_rc_numbers_and_parallel_parts(self):
        self.assertEqual(rise_time_ns(2200, 120), Fraction("223.6872"))
        self.assertEqual(rise_time_ns(4700, 120), Fraction("477.8772"))
        # Three existing 2.2k pull-ups in parallel: 733.33 ohm, below sink floor.
        equivalent = 1 / sum(Fraction(1, 2200) for _ in range(3))
        self.assertFalse(interval(tolerance=0).accepts_nominal(equivalent))

    def test_invalid_quantities_are_rejected(self):
        invalid = [dict(vdd_max_v=0), dict(vol_max_v=-1), dict(vol_max_v=4),
                   dict(iol_ma=0), dict(iol_ma=-1), dict(cb_min_pf=0),
                   dict(cb_min_pf=121), dict(cb_max_pf=-1), dict(rise_min_ns=-1),
                   dict(rise_min_ns=301), dict(rise_max_ns=0), dict(tolerance=-1),
                   dict(tolerance=1), dict(vdd_max_v=float("inf")),
                   dict(cb_max_pf=float("nan")), dict(iol_ma=True), dict(iol_ma=None)]
        for change in invalid:
            with self.subTest(change=change), self.assertRaises(ValueError):
                interval(**change)
        for r, c in ((0, 100), (1000, -1), (float("inf"), 1)):
            with self.assertRaises(ValueError):
                rise_time_ns(r, c)


class CombinedReadTests(unittest.TestCase):
    def test_decode_bytes_ack_owners_and_exact_last_nack(self):
        trace = combined_read_trace()
        check_edge_invariants(trace)
        labels = ["address-write", "register", "address-read", "read-0", "read-1"]
        expected = [0xA0, 0x12, 0xA1, 0xA5, 0x3C]
        for label, byte in zip(labels, expected):
            samples = [e for e in trace if e.kind == "DATA_RISE" and e.label == label]
            self.assertEqual([e.bit for e in samples], list(range(7, -1, -1)))
            decoded = 0
            for event in samples:
                decoded = (decoded << 1) | event.sda
                if label.startswith("read-"):
                    self.assertFalse(event.controller_sda_low)
                else:
                    self.assertFalse(event.target_sda_low)
            self.assertEqual(decoded, byte)
        acks = [e for e in trace if e.kind == "ACK_RISE"]
        self.assertEqual([e.sda for e in acks], [0, 0, 0, 0, 1])
        for event in acks[:3]:
            self.assertFalse(event.controller_sda_low)
            self.assertTrue(event.target_sda_low)
        self.assertTrue(acks[3].controller_sda_low)
        self.assertFalse(acks[3].target_sda_low)
        self.assertFalse(acks[4].controller_sda_low)
        self.assertFalse(acks[4].target_sda_low)
        # NACK is still a ninth CLOCK; no substitution by STOP.
        self.assertEqual(len([e for e in trace if e.kind in ("DATA_RISE", "ACK_RISE")]), 45)
        self.assertEqual([e.kind for e in trace if e.kind in ("START", "REPEATED_START", "STOP")],
                         ["START", "REPEATED_START", "STOP"])

    def test_ack_handover_precedes_rise_and_release_follows_fall(self):
        trace = combined_read_trace()
        for index, event in enumerate(trace):
            if event.kind == "ACK_RISE":
                setup, falling, released = trace[index - 1], trace[index + 1], trace[index + 2]
                self.assertEqual((setup.kind, setup.scl, setup.sda), ("ACK_SETUP", 0, event.sda))
                self.assertEqual((falling.kind, falling.scl, falling.sda), ("ACK_FALL", 0, event.sda))
                self.assertEqual((released.kind, released.scl), ("ACK_RELEASE", 0))
                self.assertFalse(released.controller_sda_low or released.target_sda_low)

    def test_single_byte_read_nacks_and_never_acks_payload(self):
        trace = combined_read_trace(read_bytes=(0,))
        check_edge_invariants(trace)
        self.assertEqual([e.sda for e in trace if e.kind == "ACK_RISE"], [0, 0, 0, 1])
        self.assertEqual(len([e for e in trace if e.kind == "DATA_RISE"]), 32)

    def test_local_invariant_checker_detects_high_data_change(self):
        trace = list(combined_read_trace())
        index = next(i for i, e in enumerate(trace) if e.kind == "DATA_RISE" and e.sda == 1)
        trace[index] = replace(trace[index], controller_sda_low=True)
        with self.assertRaises(ValueError):
            check_edge_invariants(tuple(trace))

    def test_invalid_inputs_are_rejected(self):
        for address in (-1, 0, 7, 0x78, 128, True):
            with self.assertRaises(ValueError):
                combined_read_trace(address=address)
        for data in ((), (256,), (-1,), (True,), (0,) * 17, [1]):
            with self.assertRaises(ValueError):
                combined_read_trace(read_bytes=data)
        with self.assertRaises(ValueError):
            combined_read_trace(register=256)


class RecoveryTests(unittest.TestCase):
    def assert_bounded(self, result, config=RecoveryConfig()):
        self.assertLessEqual(result.pulses, 9)
        self.assertLessEqual(result.ticks, config.total_ticks)
        self.assertEqual([e.ticks for e in result.trace], sorted(e.ticks for e in result.trace))
        self.assertFalse(result.trace[-1].controller_scl_low)
        self.assertFalse(result.trace[-1].controller_sda_low)
        rises = [e for e in result.trace if e.phase == "clear-rise"]
        self.assertLessEqual(len(rises), 9)
        self.assertLessEqual(result.pulses, len(rises))
        self.assertLessEqual(len(rises), result.pulses + 1)  # At most one incomplete pulse.
        for event in rises:
            self.assertEqual(event.scl, 1)  # Released-but-stuck SCL never counts.
            self.assertFalse(event.controller_sda_low)
        # Maximum deterministic trace budget for this implementation, not WCET.
        self.assertLessEqual(len(result.trace), 100 + 11 * (config.stretch_ticks + 2)
                             + 2 * config.bus_free_ticks)

    def test_already_idle_and_all_successful_release_counts(self):
        result = bus_clear_model(RecoveryFaults(sda_release_after=0))
        self.assertEqual((result.status, result.pulses, result.ticks), ("ALREADY_IDLE", 0, 5))
        self.assertFalse(any(e.phase.startswith("stop-") for e in result.trace))
        for count in range(1, 10):
            result = bus_clear_model(RecoveryFaults(sda_release_after=count))
            self.assertEqual((result.status, result.pulses, result.ticks),
                             ("RECOVERED", count, 10 * count + 15))
            self.assert_bounded(result)
            stop_index = next(i for i, e in enumerate(result.trace) if e.phase == "stop-release-sda")
            before, stop = result.trace[stop_index - 1], result.trace[stop_index]
            self.assertEqual((before.scl, before.sda, stop.scl, stop.sda), (1, 0, 1, 1))
            # Extra edge belongs to STOP preparation, explicitly not a clear pulse.
            self.assertEqual(len([e for e in result.trace if e.phase == "stop-scl-high"]), 1)

    def test_initial_and_mid_recovery_scl_timeouts(self):
        initial = bus_clear_model(RecoveryFaults(scl_release_delays=(None,)))
        self.assertEqual((initial.status, initial.pulses, initial.ticks), ("SCL_TIMEOUT", 0, 10))
        self.assertFalse(any(e.phase == "clear-rise" for e in initial.trace))
        mid = bus_clear_model(RecoveryFaults(sda_release_after=9, scl_release_delays=(0, 0, None)))
        self.assertEqual((mid.status, mid.pulses, mid.ticks), ("SCL_TIMEOUT", 1, 25))
        self.assertEqual(len([e for e in mid.trace if e.phase == "clear-rise"]), 1)
        self.assert_bounded(initial)
        self.assert_bounded(mid)

    def test_deadline_boundary_is_exclusive(self):
        before = bus_clear_model(RecoveryFaults(scl_release_delays=(9,)))
        at = bus_clear_model(RecoveryFaults(scl_release_delays=(10,)))
        self.assertEqual(before.status, "RECOVERED")
        self.assertEqual((at.status, at.pulses, at.ticks), ("SCL_TIMEOUT", 0, 10))
        at_total = bus_clear_model(config=RecoveryConfig(total_ticks=45))
        after_total = bus_clear_model(config=RecoveryConfig(total_ticks=46))
        self.assertEqual((at_total.status, at_total.ticks), ("DEADLINE", 45))
        self.assertEqual((after_total.status, after_total.ticks), ("RECOVERED", 45))
        incomplete = bus_clear_model(config=RecoveryConfig(total_ticks=7))
        self.assertEqual((incomplete.status, incomplete.pulses, incomplete.ticks), ("DEADLINE", 0, 7))
        self.assertEqual(len([e for e in incomplete.trace if e.phase == "clear-rise"]), 1)
        self.assert_bounded(incomplete, RecoveryConfig(total_ticks=7))

    def test_stuck_sda_stops_after_nine_without_an_extra_clear_edge(self):
        result = bus_clear_model(RecoveryFaults(sda_release_after=None))
        self.assertEqual((result.status, result.pulses, result.ticks), ("SDA_STUCK", 9, 90))
        self.assertFalse(any(e.phase.startswith("stop-") for e in result.trace))
        self.assertEqual(result.trace[-2].phase, "clear-budget-end")
        self.assertEqual(result.trace[-2].scl, 1)
        self.assert_bounded(result)

    def test_stop_failure_and_scl_stuck_during_stop(self):
        result = bus_clear_model(RecoveryFaults(sda_stuck_at_stop=True))
        self.assertEqual((result.status, result.pulses, result.ticks), ("STOP_FAILED", 3, 40))
        self.assertEqual(result.trace[-1].sda, 0)
        stop_timeout = bus_clear_model(RecoveryFaults(sda_release_after=1,
                                                     scl_release_delays=(0, 0, None)))
        self.assertEqual((stop_timeout.status, stop_timeout.pulses, stop_timeout.ticks),
                         ("SCL_TIMEOUT", 1, 25))
        self.assertFalse(any(e.phase == "stop-release-sda" for e in stop_timeout.trace))
        self.assert_bounded(result)
        self.assert_bounded(stop_timeout)

    def test_idle_must_stay_free_for_entire_observation_window(self):
        for low_at in (0, 2, 5):
            result = bus_clear_model(RecoveryFaults(idle_sda_low_at=low_at))
            self.assertEqual(result.status, "BUS_NOT_IDLE")
            self.assertEqual(result.ticks, 40 + low_at)
            self.assert_bounded(result)
        self.assertEqual(bus_clear_model(RecoveryFaults(idle_sda_low_at=6)).status, "RECOVERED")
        idle = bus_clear_model(RecoveryFaults(sda_release_after=0, idle_sda_low_at=2))
        self.assertEqual((idle.status, idle.ticks), ("BUS_NOT_IDLE", 2))

    def test_unknown_owner_or_push_pull_configuration_refuses_without_edges(self):
        for flags in (dict(exclusive_owner=False), dict(open_drain_pins=False)):
            result = bus_clear_model(**flags)
            self.assertEqual((result.status, result.pulses, result.ticks, result.trace),
                             ("REFUSED", 0, 0, ()))

    def test_finite_fault_matrix_and_global_budget_sweep(self):
        scenarios = 0
        # 11 SDA scripts * 12 delays * 11 SCL-attempt positions = 1452 traces.
        for release in (*range(10), None):
            for delay in (*range(11), None):
                for position in range(11):
                    result = bus_clear_model(RecoveryFaults(sda_release_after=release,
                        scl_release_delays=(0,) * position + (delay,)))
                    self.assert_bounded(result)
                    if result.status == "RECOVERED":
                        self.assertEqual(result.pulses, release)
                        self.assertEqual((result.trace[-1].scl, result.trace[-1].sda), (1, 1))
                    scenarios += 1
        # Budget can expire at any tick, including LOW, HIGH, STOP and tBUF.
        for budget in range(1, 121):
            config = RecoveryConfig(total_ticks=budget)
            for release in range(1, 10):
                result = bus_clear_model(RecoveryFaults(sda_release_after=release), config)
                self.assert_bounded(result, config)
                self.assertEqual(result.status, "RECOVERED" if budget > 10 * release + 15 else "DEADLINE")
                scenarios += 1
        self.assertEqual(scenarios, 2532)
        print("Finite recovery matrix: PASS (2532 bounded synthetic traces; not exhaustive proof)")

    def test_invalid_config_or_faults_are_rejected(self):
        for config in (RecoveryConfig(low_ticks=0), RecoveryConfig(total_ticks=True),
                       RecoveryConfig(stretch_ticks=-1), RecoveryConfig(high_ticks=1.5)):
            with self.assertRaises(ValueError):
                bus_clear_model(config=config)
        for faults in (RecoveryFaults(sda_release_after=10), RecoveryFaults(sda_release_after=True),
                       RecoveryFaults(scl_release_delays=(-1,)), RecoveryFaults(scl_release_delays=(0,) * 12),
                       RecoveryFaults(idle_sda_low_at=-1), RecoveryFaults(sda_stuck_at_stop=1)):
            with self.assertRaises(ValueError):
                bus_clear_model(faults)
        with self.assertRaises(ValueError):
            bus_clear_model(exclusive_owner=1)


if __name__ == "__main__":
    unittest.main(verbosity=2)
