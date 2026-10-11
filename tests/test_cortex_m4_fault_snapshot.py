"""Synthetic snapshot tests; no target memory, emulator or real faults."""
import importlib.util
from pathlib import Path
import sys
import unittest

MODEL = Path(__file__).resolve().parents[1] / "examples/python/cortex_m4_fault_snapshot.py"
spec = importlib.util.spec_from_file_location("cortex_m4_fault_snapshot", MODEL)
assert spec and spec.loader
m = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = m
spec.loader.exec_module(m)
BASE = 0x20001000
CORE = (1, 2, 3, 4, 12, 0x08000101, 0x08000200, 1 << 24)


def sample(exc=0xFFFFFFFD, words=CORE, **kwargs):
    args = dict(exc_return=exc, cfsr=0, hfsr=0, msp=BASE, psp=BASE,
                capture_base=BASE, words=words, readable_start=BASE,
                readable_end=BASE + len(words) * 4, bfar=0x40000000,
                mmfar=0xDEADBEEF)
    args.update(kwargs)
    return m.Snapshot(**args)


class SnapshotTests(unittest.TestCase):
    def test_all_six_exception_return_layouts(self):
        for exc, (stack, mode, extended) in m.EXC_RETURNS.items():
            words = CORE + (0,) * 18 if extended else CORE
            r = m.decode(sample(exc, words))
            self.assertEqual((r["stack"], r["return_mode"], r["extended_fp"]),
                             (stack, mode, extended))
            self.assertEqual(r["core"]["pc"], CORE[6])
            self.assertEqual(r["frame_trust"], "structurally_accepted")
            self.assertEqual(r["pre_exception_sp"], BASE + len(words) * 4)
            self.assertFalse(r["fp_payload_decoded"])

    def test_stack_selection_not_current_handler_assumption(self):
        r = m.decode(sample(msp=BASE + 0x1000))
        self.assertEqual(r["stack"], "PSP")
        self.assertIsNotNone(r["core"])
        r = m.decode(sample(0xFFFFFFF9, psp=BASE + 0x1000))
        self.assertEqual(r["stack"], "MSP")
        self.assertIsNotNone(r["core"])

    def test_padding_after_core_not_before(self):
        padded = CORE[:-1] + (CORE[-1] | (1 << 9), 0xAAAAAAAA)
        r = m.decode(sample(words=padded))
        self.assertEqual(r["core"]["r0"], 1)
        self.assertEqual(r["core"]["pc"], CORE[6])
        self.assertEqual(r["padding_words"], 1)
        self.assertEqual(r["frame_bytes"], 36)
        self.assertEqual(r["pre_exception_sp"], BASE + 36)

    def test_extended_padding_and_capture_prefix(self):
        padded = CORE[:-1] + (CORE[-1] | (1 << 9), 0)
        words = (0xBBBBBBBB,) * 2 + padded[:8] + (0xCCCCCCCC,) * 18 + padded[8:]
        r = m.decode(sample(0xFFFFFFED, words, psp=BASE + 8, fpccr=1))
        self.assertEqual(r["core"]["pc"], CORE[6])
        self.assertEqual(r["frame_bytes"], 108)
        self.assertEqual(r["pre_exception_sp"], BASE + 116)
        self.assertTrue(r["lazy_fp_active"])
        self.assertFalse(r["fp_payload_decoded"])

    def test_missing_padding_rejects_full_frame(self):
        words = CORE[:-1] + (CORE[-1] | (1 << 9),)
        r = m.decode(sample(words=words))
        self.assertEqual(r["frame_trust"], "rejected")
        self.assertIsNone(r["core"])
        self.assertIn("alignment padding", r["reasons"][0])

    def test_short_basic_and_extended_frames_rejected(self):
        for exc, words in ((0xFFFFFFFD, CORE[:7]), (0xFFFFFFED, CORE + (0,) * 17)):
            r = m.decode(sample(exc, words))
            self.assertIsNone(r["core"])
            self.assertEqual(r["frame_trust"], "rejected")

    def test_selected_stack_outside_capture_rejected(self):
        for sp in (BASE - 4, BASE + 32, BASE + 1):
            self.assertEqual(m.decode(sample(psp=sp))["frame_trust"], "rejected")

    def test_address_validity_bits_independent(self):
        for bvalid in (False, True):
            for mvalid in (False, True):
                cfsr = (int(bvalid) << 15) | (int(mvalid) << 7)
                r = m.decode(sample(cfsr=cfsr))
                self.assertEqual(r["bfar"], 0x40000000 if bvalid else None)
                self.assertEqual(r["mmfar"], 0xDEADBEEF if mvalid else None)

    def test_each_stack_risk_bit_rejects_automatic_trust(self):
        for bit in m.FRAME_RISK_BITS:
            r = m.decode(sample(cfsr=1 << bit))
            self.assertEqual(r["frame_trust"], "rejected")
            self.assertIsNone(r["core"])

    def test_precise_and_imprecise_labels(self):
        self.assertTrue(m.decode(sample(cfsr=1 << 9))["precise_pc_candidate"])
        for cfsr in (0, 1 << 10, (1 << 9) | (1 << 10)):
            self.assertFalse(m.decode(sample(cfsr=cfsr))["precise_pc_candidate"])

    def test_forced_is_escalation_not_root_cause(self):
        r = m.decode(sample(hfsr=1 << 30))
        self.assertTrue(r["forced_escalation"])
        self.assertEqual(r["flags"], [])
        self.assertFalse(r["precise_pc_candidate"])

    def test_vector_table_pc_is_interrupted_context(self):
        r = m.decode(sample(cfsr=1 << 9, hfsr=1 << 1))
        self.assertTrue(r["vector_table_fault"])
        self.assertFalse(r["precise_pc_candidate"])

    def test_zero_thumb_bit_keeps_evidence_but_rejects_certification(self):
        r = m.decode(sample(words=CORE[:-1] + (0,)))
        self.assertEqual(r["frame_trust"], "rejected")
        self.assertEqual(r["core"]["pc"], CORE[6])

    def test_invalid_exception_return_rejected(self):
        for value in (0, 0xFFFFFFF5, 0xFFFFFFFC, 0xFFFFFFB9):
            self.assertEqual(m.decode(sample(exc=value))["frame_trust"], "rejected")

    def test_input_capture_contract_rejections(self):
        for changes in ({"cfsr": True}, {"words": list(CORE)}, {"words": (-1,)},
                        {"words": (0,) * 65}, {"capture_base": BASE + 1},
                        {"readable_start": BASE + 4}, {"readable_end": BASE + 4},
                        {"capture_base": 0xFFFFFFFC, "readable_start": 0xFFFFFFFC,
                         "readable_end": 0xFFFFFFFF, "words": (0, 0)}):
            with self.assertRaises(ValueError):
                sample(**changes)

    def test_does_not_mutate_snapshot(self):
        s = sample()
        before = tuple(vars(s).items())
        self.assertEqual(m.decode(s), m.decode(s))
        self.assertEqual(tuple(vars(s).items()), before)


if __name__ == "__main__":
    unittest.main()
