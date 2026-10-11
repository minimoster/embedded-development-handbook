"""Bounded host arithmetic tests; no Cortex-M or FreeRTOS execution."""
import importlib.util
from pathlib import Path
import sys
import unittest

MODEL = Path(__file__).resolve().parents[1] / "examples/python/nvic_priority_contract.py"
spec = importlib.util.spec_from_file_location("nvic_priority_contract", MODEL)
assert spec and spec.loader
m = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = m
spec.loader.exec_module(m)


class PriorityContractTests(unittest.TestCase):
    def test_all_supported_group_roundtrips(self):
        for bits in range(1, 8):
            for group in range(8):
                pre_bits, sub_bits = m.priority_partition(bits, group)
                self.assertEqual(pre_bits + sub_bits, bits)
                for logical in range(1 << bits):
                    pre, sub = m.decode_logical(bits, group, logical)
                    self.assertEqual(m.encode_logical(bits, group, pre, sub), logical)

    def test_all_register_roundtrips(self):
        for bits in range(1, 8):
            for logical in range(1 << bits):
                raw = m.to_register_byte(bits, logical)
                self.assertEqual(m.from_register_byte(bits, raw), logical)
                self.assertEqual(m.cmsis_shift_demo(bits, logical), raw)

    def test_all_four_bit_preemption_relations(self):
        for group in range(8):
            pre_bits, sub_bits = m.priority_partition(4, group)
            for candidate in range(16):
                for active in range(16):
                    expected = (candidate >> sub_bits) < (active >> sub_bits)
                    self.assertEqual(m.can_preempt(4, group, candidate, active), expected)

    def test_same_preemption_subpriority_does_not_preempt(self):
        a = m.encode_logical(4, 5, 1, 0)
        b = m.encode_logical(4, 5, 1, 3)
        self.assertFalse(m.can_preempt(4, 5, a, b))
        self.assertFalse(m.can_preempt(4, 5, b, a))

    def test_all_supported_port_threshold_relations(self):
        for bits in range(1, 8):
            for group in range(8 - bits):
                for threshold in range(1, 1 << bits):
                    cfg = m.PortPriorityConfig(bits, group, m.to_register_byte(bits, threshold))
                    self.assertEqual(cfg.max_syscall_logical, threshold)
                    for irq in range(1 << bits):
                        expected = irq >= threshold
                        self.assertEqual(m.basepri_masks(cfg, irq), expected)
                        self.assertEqual(m.fromisr_priority_gate(cfg, irq), expected)

    def test_stm32_hal_group_four_is_raw_three(self):
        cfg = m.PortPriorityConfig(4, 3, 0x50)
        self.assertEqual(cfg.max_syscall_logical, 5)
        with self.assertRaises(ValueError):
            m.PortPriorityConfig(4, 4, 0x50)

    def test_double_shift_is_visible_but_strict_auditor_rejects(self):
        self.assertEqual(m.cmsis_shift_demo(4, 5), 0x50)
        self.assertEqual(m.cmsis_shift_demo(4, 0x50), 0x00)
        with self.assertRaises(ValueError):
            m.to_register_byte(4, 0x50)

    def test_unimplemented_register_bits_and_zero_threshold_rejected(self):
        with self.assertRaises(ValueError):
            m.from_register_byte(4, 0x51)
        with self.assertRaises(ValueError):
            m.PortPriorityConfig(4, 3, 0)

    def test_oversized_fields_are_not_silently_masked(self):
        for args in ((4, 5, 4, 0), (4, 5, 0, 4), (4, 3, 0, 1)):
            with self.assertRaises(ValueError):
                m.encode_logical(*args)

    def test_outside_model_inputs_rejected(self):
        for bits in (0, 8, -1, True, 4.0):
            with self.assertRaises(ValueError):
                m.priority_partition(bits, 3)
        for group in (-1, 8, True):
            with self.assertRaises(ValueError):
                m.priority_partition(4, group)
        for logical in (-1, 16, True):
            with self.assertRaises(ValueError):
                m.to_register_byte(4, logical)
        for raw in (-1, 256, True):
            with self.assertRaises(ValueError):
                m.from_register_byte(4, raw)

    def test_example_priority_gates(self):
        rows = m.example_table()
        self.assertEqual([r["register"] for r in rows], ["0x20", "0x50", "0xa0"])
        self.assertEqual([r["FromISR_priority_gate"] for r in rows], [False, True, True])


if __name__ == "__main__":
    unittest.main()
