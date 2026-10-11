#!/usr/bin/env python3
"""Finite serialized host-model checks, NOT DMA/kernel/concurrency tests."""
from collections import deque
from dataclasses import replace
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "examples/python"))
from dma_lifecycle_contract import (  # noqa: E402
    Config, Context, ContractError, Direction, Lifecycle, Outcome, Phase,
)


def active(config=Config(), mapped_nents=1):
    return Lifecycle(config=config).map_sg(mapped_nents).prepare().submit().issue_pending()


def completed(state=None, *, ok=True):
    state = active() if state is None else state
    return state.hardware_complete(ok=ok).callback_enter().callback_publish().callback_return()


def removed(state):
    return state.free_resources().release_channel().finish_remove()


class DmaLifecycleTests(unittest.TestCase):
    def reject_unchanged(self, state, operation, **arguments):
        snapshot = replace(state)
        with self.assertRaises(ContractError):
            getattr(state, operation)(**arguments)
        self.assertEqual(state, snapshot, "rejected events must not partially mutate state")
        state.check()

    def test_normal_rx_all_small_sg_merges_and_both_directions(self):
        paths = 0
        for direction in Direction:
            for original in range(1, 5):
                for count in range(1, original + 1):
                    config = Config(original_nents=original, direction=direction)
                    state = completed(active(config, count))
                    self.assertEqual(state.outcome, Outcome.SUCCESS)
                    self.reject_unchanged(state, "cpu_access")
                    state = state.unmap_sg(original_nents=original)
                    state = state.consume_rx() if direction == Direction.FROM_DEVICE else state.cpu_access()
                    state = removed(state.begin_remove())
                    self.assertTrue(state.removed)
                    self.assertEqual((state.map_count, state.unmap_count), (1, 1))
                    paths += 1
        self.assertEqual(paths, 20)

    def test_mapping_failure_has_no_unmap_and_can_retry(self):
        failed = Lifecycle().map_sg(0)
        self.assertEqual(failed.phase, Phase.CPU)
        self.assertEqual((failed.map_count, failed.unmap_count), (0, 0))
        for operation in ("prepare", "submit", "issue_pending", "unmap_sg"):
            self.reject_unchanged(failed, operation)
        self.assertTrue(removed(failed.begin_remove()).removed)
        self.assertEqual(failed.map_sg(2).map_count, 1)

    def test_prepare_failure_only_unmaps_existing_mapping(self):
        state = Lifecycle().map_sg(2).prepare(ok=False)
        self.assertFalse(state.descriptor_client_owned)
        self.reject_unchanged(state, "submit")
        self.assertTrue(removed(state.begin_remove().unmap_sg()).removed)

    def test_controller_device_is_required_for_mapping_and_unmapping(self):
        state = Lifecycle()
        self.reject_unchanged(state, "map_sg", mapped_nents=1, device="peripheral-device")
        state = state.map_sg(2)
        self.reject_unchanged(state, "unmap_sg", device="peripheral-device")
        self.assertEqual(state.unmap_sg().phase, Phase.UNMAPPED)

    def test_original_and_mapped_sg_counts_have_different_jobs(self):
        state = Lifecycle(Config(original_nents=4)).map_sg(2)
        self.reject_unchanged(state, "prepare", sg_len=4)
        state = completed(state.prepare(sg_len=2).submit().issue_pending())
        self.reject_unchanged(state, "unmap_sg", original_nents=2)
        self.assertEqual(state.unmap_sg(original_nents=4).unmap_count, 1)

    def test_direction_enums_are_not_interchangeable(self):
        state = Lifecycle(Config(direction=Direction.TO_DEVICE))
        self.reject_unchanged(state, "map_sg", mapped_nents=1, direction=Direction.FROM_DEVICE)
        state = state.map_sg(1)
        self.reject_unchanged(state, "prepare", transfer_direction="DMA_TO_DEVICE")
        self.reject_unchanged(state, "prepare", transfer_direction="DMA_DEV_TO_MEM")
        self.reject_unchanged(state, "unmap_sg", direction=Direction.FROM_DEVICE)
        self.assertEqual(state.prepare(transfer_direction="DMA_MEM_TO_DEV").phase, Phase.PREPARED)

    def test_cpu_cannot_access_successful_mapping_or_inflight_data(self):
        state = Lifecycle().cpu_access().map_sg(1)
        for stage in (state, state.prepare(), state.prepare().submit(), active()):
            self.reject_unchanged(stage, "cpu_access")
            self.reject_unchanged(stage, "free_resources")

    def test_descriptor_access_ends_at_submit(self):
        state = Lifecycle().map_sg(1).prepare().descriptor_access().submit()
        self.assertFalse(state.descriptor_client_owned)
        self.reject_unchanged(state, "descriptor_access")
        self.reject_unchanged(state, "hardware_complete")
        self.assertEqual(state.issue_pending().phase, Phase.ACTIVE)

    def test_submit_error_is_conservatively_stopped_not_reused(self):
        state = Lifecycle().map_sg(1).prepare().submit(cookie=-5)
        self.assertEqual(state.outcome, Outcome.SUBMIT_ERROR)
        for operation in ("descriptor_access", "issue_pending", "unmap_sg", "free_resources"):
            self.reject_unchanged(state, operation)
        state = state.terminate_sync().unmap_sg()
        self.assertEqual(state.outcome, Outcome.SUBMIT_ERROR)
        self.assertTrue(removed(state.begin_remove()).removed)

    def test_timeout_is_a_waiter_decision_not_dma_completion(self):
        state = active().timeout()
        self.assertTrue(state.dma_may_access)
        self.assertEqual(state.phase, Phase.ACTIVE)
        self.assertEqual(state.outcome, Outcome.TIMEOUT)
        for operation in ("unmap_sg", "free_resources", "consume_rx"):
            self.reject_unchanged(state, operation)
        state = state.terminate_sync().unmap_sg()
        self.reject_unchanged(state, "consume_rx")
        self.assertEqual(state.cpu_access().phase, Phase.UNMAPPED)

    def test_late_completion_does_not_replace_timeout_with_success(self):
        state = completed(active().timeout()).unmap_sg()
        self.assertTrue(state.completion_ok)
        self.assertEqual(state.outcome, Outcome.TIMEOUT)
        self.reject_unchanged(state, "consume_rx")

    def test_published_completion_does_not_mean_callback_has_returned(self):
        state = active().hardware_complete().callback_enter().callback_publish()
        self.assertEqual(state.outcome, Outcome.SUCCESS)
        self.assertTrue(state.callback_running)
        for operation in ("unmap_sg", "free_resources", "consume_rx"):
            self.reject_unchanged(state, operation)
        self.assertEqual(state.callback_return().unmap_sg().consume_rx().outcome, Outcome.SUCCESS)

    def test_hardware_done_alone_keeps_pending_callback_resources(self):
        state = active().hardware_complete()
        self.assertFalse(state.dma_may_access)
        self.assertTrue(state.callback_pending)
        self.reject_unchanged(state, "unmap_sg")
        self.reject_unchanged(state, "free_resources")

    def test_failed_data_is_reclaimable_but_not_successful_rx(self):
        state = completed(ok=False).unmap_sg()
        self.assertEqual(state.outcome, Outcome.IO_ERROR)
        self.reject_unchanged(state, "consume_rx")
        self.assertTrue(removed(state.begin_remove()).removed)

    def test_async_stop_is_not_synchronized_stop(self):
        state = active().timeout().terminate_async()
        self.assertEqual(state.phase, Phase.STOPPING)
        self.assertTrue(state.dma_may_access)
        for operation in ("unmap_sg", "free_resources", "issue_pending", "callback_enter"):
            self.reject_unchanged(state, operation)
        state = state.synchronize().unmap_sg()
        self.assertEqual(state.outcome, Outcome.TIMEOUT)
        self.assertTrue(removed(state.begin_remove()).removed)

    def test_queued_not_issued_descriptor_also_needs_termination(self):
        state = Lifecycle().map_sg(1).prepare().submit().begin_remove()
        self.reject_unchanged(state, "unmap_sg")
        self.assertTrue(removed(state.terminate_sync().unmap_sg()).removed)

    def test_incomplete_cancel_has_no_completion_callback_to_wait_for(self):
        state = active().terminate_async().synchronize()
        self.assertEqual(state.outcome, Outcome.CANCELLED)
        self.reject_unchanged(state, "callback_enter")
        self.reject_unchanged(state, "callback_publish")
        self.assertEqual(state.unmap_sg().free_resources().phase, Phase.FREED)

    def test_async_stop_can_race_already_pending_callback(self):
        state = active().hardware_complete().terminate_async()
        state = state.callback_enter().callback_publish().callback_return()
        self.assertEqual(state.phase, Phase.STOPPING)
        self.reject_unchanged(state, "unmap_sg")
        self.assertEqual(state.synchronize().unmap_sg().phase, Phase.UNMAPPED)

    def test_async_stop_from_callback_then_process_synchronize(self):
        state = active().hardware_complete().callback_enter()
        state = state.terminate_async(context=Context.CALLBACK)
        for operation in ("terminate_sync", "synchronize"):
            self.reject_unchanged(state, operation, context=Context.CALLBACK)
        state = state.callback_publish().callback_return().synchronize()
        self.assertFalse(state.callback_running)
        self.assertEqual(state.unmap_sg().free_resources().phase, Phase.FREED)

    def test_sync_waited_return_drains_already_running_callback(self):
        state = active().hardware_complete().callback_enter().callback_publish()
        state = state.begin_remove().terminate_sync()
        self.assertFalse(state.callback_running)
        self.reject_unchanged(state, "callback_return")
        self.assertTrue(removed(state.unmap_sg()).removed)

    def test_sync_operations_reject_atomic_and_callback_contexts(self):
        for context in (Context.ATOMIC, Context.CALLBACK):
            self.reject_unchanged(active(), "terminate_sync", context=context)
            self.reject_unchanged(active().terminate_async(), "synchronize", context=context)
        self.reject_unchanged(active(), "synchronize")

    def test_remove_gate_blocks_mapping_preparation_and_issuing(self):
        for state, operation, args in (
            (Lifecycle(), "map_sg", {"mapped_nents": 1}),
            (Lifecycle().map_sg(1), "prepare", {}),
            (Lifecycle().map_sg(1).prepare().submit(), "issue_pending", {}),
        ):
            self.reject_unchanged(state.begin_remove(), operation, **args)
        self.reject_unchanged(active().begin_remove(), "finish_remove")

    def test_prepare_submit_pair_is_serialized_against_remove(self):
        state = Lifecycle().map_sg(1).prepare()
        self.reject_unchanged(state, "begin_remove")
        self.reject_unchanged(state, "terminate_sync")
        state = state.submit().begin_remove().terminate_sync().unmap_sg()
        self.assertTrue(removed(state).removed)

    def test_stop_failure_quarantines_instead_of_fabricating_safe_free(self):
        for operation in ("terminate_sync", "terminate_async"):
            start = active().timeout().begin_remove()
            state = getattr(start, operation)(ok=False)
            self.assertTrue(state.quarantined)
            self.assertFalse(state.admission_open)
            self.assertTrue(state.resources_live and state.mapped and state.dma_may_access)
            self.assertEqual(state.unmap_count, 0)
            for forbidden in ("unmap_sg", "free_resources", "release_channel", "finish_remove", "synchronize"):
                self.reject_unchanged(state, forbidden)
            # A later verified successful stop can permit cleanup. No reset is invented.
            state = state.terminate_sync(ok=True).unmap_sg()
            self.assertFalse(state.quarantined)
            self.assertTrue(removed(state).removed)

    def test_quarantine_also_retains_running_callback(self):
        state = active().hardware_complete().callback_enter().terminate_sync(ok=False)
        self.assertTrue(state.callback_running and state.resources_live)
        self.reject_unchanged(state, "free_resources")
        state = state.callback_publish().callback_return()
        self.reject_unchanged(state, "unmap_sg")
        self.assertEqual(state.terminate_async().synchronize().unmap_sg().phase, Phase.UNMAPPED)

    def test_mapping_unmapping_free_release_and_remove_are_not_double_actions(self):
        state = Lifecycle().map_sg(1)
        self.reject_unchanged(state, "map_sg", mapped_nents=1)
        state = state.unmap_sg()
        self.reject_unchanged(state, "unmap_sg")
        state = state.begin_remove().free_resources()
        self.reject_unchanged(state, "free_resources")
        self.reject_unchanged(state, "cpu_access")
        state = state.release_channel()
        self.reject_unchanged(state, "release_channel")
        state = state.finish_remove()
        self.reject_unchanged(state, "finish_remove")
        self.reject_unchanged(state, "begin_remove")

    def test_invalid_bounded_configuration_and_results(self):
        for values in ({"original_nents": 0}, {"original_nents": 5}, {"original_nents": True},
                       {"byte_count": 0}, {"byte_count": 4097}, {"dma_device": ""},
                       {"direction": "DMA_BIDIRECTIONAL"}):
            with self.subTest(values=values), self.assertRaises(ContractError):
                Config(**values)
        for value in (-1, 4, True, 1.5):
            self.reject_unchanged(Lifecycle(), "map_sg", mapped_nents=value)
        for value in (0, True, 2**31, -(2**31) - 1):
            self.reject_unchanged(Lifecycle().map_sg(1).prepare(), "submit", cookie=value)
        self.reject_unchanged(Lifecycle().map_sg(1), "prepare", ok=1)
        self.reject_unchanged(active(), "hardware_complete", ok=1)
        self.reject_unchanged(active(), "terminate_sync", context="process")
        self.reject_unchanged(active().terminate_async(), "synchronize", context="process")
        self.reject_unchanged(Lifecycle(), "map_sg", mapped_nents=1, direction="DMA_FROM_DEVICE")
        self.reject_unchanged(Lifecycle().map_sg(1), "unmap_sg", direction="DMA_FROM_DEVICE")

    def test_bounded_event_graph_invariants_and_rejected_edges(self):
        # Explore unique abstract states through at most 14 events. Trace is
        # excluded from keys; this is NOT exhaustive kernel/interleaving proof.
        actions = (
            lambda s: s.map_sg(0), lambda s: s.map_sg(1), lambda s: s.prepare(),
            lambda s: s.prepare(ok=False), lambda s: s.submit(), lambda s: s.submit(-5),
            lambda s: s.issue_pending(), lambda s: s.timeout(), lambda s: s.hardware_complete(),
            lambda s: s.hardware_complete(ok=False), lambda s: s.callback_enter(),
            lambda s: s.callback_publish(), lambda s: s.callback_return(), lambda s: s.begin_remove(),
            lambda s: s.terminate_sync(), lambda s: s.terminate_sync(ok=False),
            lambda s: s.terminate_async(), lambda s: s.terminate_async(ok=False),
            lambda s: s.synchronize(), lambda s: s.unmap_sg(), lambda s: s.free_resources(),
            lambda s: s.release_channel(), lambda s: s.finish_remove(),
        )
        queue = deque([(Lifecycle(), 0)])
        seen = {Lifecycle()}
        rejected = 0
        terminal = 0
        while queue:
            state, depth = queue.popleft()
            state.check()
            terminal += state.removed
            if depth == 14:
                continue
            for action in actions:
                before = replace(state)
                try:
                    nxt = action(state)
                except ContractError:
                    rejected += 1
                    self.assertEqual(state, before)
                    continue
                nxt.check()
                key = replace(nxt, trace=())
                if key not in seen:
                    seen.add(key)
                    queue.append((key, depth + 1))
        self.assertGreater(len(seen), 100)
        self.assertGreater(rejected, 1000)
        self.assertGreater(terminal, 0)
        print(f"Bounded event graph: {len(seen)} unique states, {rejected} rejected edges, {terminal} removed states (depth <=14).")


if __name__ == "__main__":
    unittest.main(verbosity=2)
