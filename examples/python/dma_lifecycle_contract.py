#!/usr/bin/env python3
"""Original, bounded, serialized DMAengine-client lifetime teaching model.

NOT a kernel driver, DMA/cache simulator, or proof of concurrent teardown.
Linux v6.6 contracts guide the events; policy is deliberately narrower:
exclusive slave channel, one one-shot SG request, short-lived mapping,
no callback resubmission, no descriptor reuse, CPU accesses only unmapped.
No threads, devices, real addresses, sleeps, or kernel APIs are used.
"""
from dataclasses import dataclass, replace
from enum import Enum


class ContractError(ValueError):
    """An event would violate this teaching design's lifetime contract."""


class Direction(str, Enum):
    TO_DEVICE = "DMA_TO_DEVICE"
    FROM_DEVICE = "DMA_FROM_DEVICE"


class Context(str, Enum):
    PROCESS = "process"
    ATOMIC = "atomic"
    CALLBACK = "callback"


class Phase(str, Enum):
    CPU = "cpu"
    MAPPED = "mapped"
    PREPARED = "prepared"
    QUEUED = "queued"
    ACTIVE = "active"
    HW_DONE = "hardware_done_callback_pending"
    CALLBACK = "callback_running"
    DONE = "callback_returned"
    SUBMIT_FAILED = "submit_failed_needs_cleanup"
    STOPPING = "async_stop_not_synchronized"
    QUIESCED = "dma_and_callbacks_quiesced"
    UNMAPPED = "unmapped_cpu"
    FREED = "freed"


class Outcome(str, Enum):
    NONE = "none"
    SUCCESS = "success"
    IO_ERROR = "io_error"
    TIMEOUT = "timeout"
    CANCELLED = "cancelled"
    SUBMIT_ERROR = "submit_error"


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ContractError(message)


def integer(value: int, low: int, high: int, name: str) -> None:
    require(type(value) is int and low <= value <= high,
            f"{name} must be an integer in [{low}, {high}]")


@dataclass(frozen=True)
class Config:
    dma_device: str = "dma-controller-device"
    original_nents: int = 3
    byte_count: int = 1024
    direction: Direction = Direction.FROM_DEVICE

    def __post_init__(self) -> None:
        require(isinstance(self.dma_device, str) and bool(self.dma_device),
                "DMA mapping device must be explicitly identified")
        integer(self.original_nents, 1, 4, "original_nents")
        integer(self.byte_count, 1, 4096, "byte_count")
        require(isinstance(self.direction, Direction), "unsupported mapping direction")

    @property
    def transfer_direction(self) -> str:
        return "DMA_MEM_TO_DEV" if self.direction == Direction.TO_DEVICE else "DMA_DEV_TO_MEM"


_ENGINE_PHASES = frozenset({Phase.QUEUED, Phase.ACTIVE, Phase.HW_DONE,
                          Phase.CALLBACK, Phase.DONE, Phase.SUBMIT_FAILED,
                          Phase.STOPPING, Phase.QUIESCED})
_MAPPED_PHASES = _ENGINE_PHASES | {Phase.MAPPED, Phase.PREPARED}
_STOPPABLE = _ENGINE_PHASES - {Phase.QUIESCED}


@dataclass(frozen=True)
class Lifecycle:
    config: Config = Config()
    phase: Phase = Phase.CPU
    mapped_nents: int = 0
    map_count: int = 0
    unmap_count: int = 0
    descriptor_client_owned: bool = False
    dma_may_access: bool = False
    hardware_completed: bool = False
    callback_pending: bool = False
    callback_running: bool = False
    callback_published: bool = False
    completion_ok: bool = False
    outcome: Outcome = Outcome.NONE
    removal_requested: bool = False
    quarantined: bool = False
    channel_released: bool = False
    removed: bool = False
    trace: tuple[str, ...] = ()

    @property
    def mapped(self) -> bool:
        return self.map_count == 1 and self.unmap_count == 0

    @property
    def admission_open(self) -> bool:
        return not (self.removal_requested or self.quarantined or self.channel_released)

    @property
    def resources_live(self) -> bool:
        return self.phase != Phase.FREED

    def check(self) -> None:
        """Check invariants after legal events, not arbitrary kernel executions."""
        require(isinstance(self.config, Config) and isinstance(self.phase, Phase), "invalid state type")
        require(0 <= self.unmap_count <= self.map_count <= 1, "map/unmap must pair exactly once")
        require(self.mapped == (self.phase in _MAPPED_PHASES), "mapping/phase mismatch")
        require(self.map_count == 0 or 1 <= self.mapped_nents <= self.config.original_nents,
                "successful map needs a valid returned segment count")
        require(self.descriptor_client_owned == (self.phase == Phase.PREPARED),
                "client descriptor access ends at submit")
        require(not self.callback_running or self.resources_live, "callback resources freed early")
        require(not self.dma_may_access or (self.mapped and self.resources_live), "DMA needs live mapped storage")
        require(not self.callback_pending or self.resources_live, "pending callback needs live resources")
        require(not self.callback_running or self.hardware_completed, "this one-shot callback follows HW completion")
        require(self.phase not in {Phase.DONE, Phase.QUIESCED, Phase.UNMAPPED, Phase.FREED}
                or not (self.dma_may_access or self.callback_pending or self.callback_running),
                "reclaimable phases require DMA and callbacks quiescent")
        require(not self.channel_released or (self.phase == Phase.FREED and self.removal_requested),
                "release follows removal gate and resource cleanup")
        require(not self.removed or (self.channel_released and not self.quarantined),
                "removed cannot leave channel or quarantine outstanding")
        require(not self.quarantined or self.resources_live, "quarantine retains resources")

    def _event(self, label: str, **changes) -> "Lifecycle":
        self.check()
        candidate = replace(self, trace=self.trace + (label,), **changes)
        candidate.check()
        return candidate

    def _admit(self) -> None:
        self.check()
        require(self.admission_open, "new work blocked by removal/quarantine/release gate")

    def map_sg(self, mapped_nents: int, *, device: str | None = None,
               original_nents: int | None = None,
               direction: Direction | None = None) -> "Lifecycle":
        self._admit()
        require(self.phase == Phase.CPU, "one successful map per request")
        device = self.config.dma_device if device is None else device
        nents = self.config.original_nents if original_nents is None else original_nents
        direction = self.config.direction if direction is None else direction
        require(device == self.config.dma_device, "use dmaengine_get_dma_device(chan), not peripheral dev")
        require(type(nents) is int and nents == self.config.original_nents, "wrong original SG count")
        require(isinstance(direction, Direction) and direction == self.config.direction, "wrong mapping direction")
        integer(mapped_nents, 0, nents, "mapped_nents")
        if mapped_nents == 0:
            return self._event("map_failed_no_mapping")
        return self._event(f"map:{nents}->{mapped_nents}", phase=Phase.MAPPED,
                           mapped_nents=mapped_nents, map_count=1)

    def prepare(self, *, sg_len: int | None = None,
                transfer_direction: str | None = None, ok: bool = True) -> "Lifecycle":
        self._admit()
        require(self.phase == Phase.MAPPED, "prepare requires a successful mapping")
        sg_len = self.mapped_nents if sg_len is None else sg_len
        transfer_direction = self.config.transfer_direction if transfer_direction is None else transfer_direction
        require(type(sg_len) is int and sg_len == self.mapped_nents, "prepare uses returned DMA segment count")
        require(transfer_direction == self.config.transfer_direction, "mapping and transfer directions disagree")
        require(type(ok) is bool, "prepare result must be boolean")
        if not ok:
            return self._event("prepare_failed_no_descriptor")
        return self._event("prepare", phase=Phase.PREPARED, descriptor_client_owned=True)

    def descriptor_access(self) -> "Lifecycle":
        require(self.phase == Phase.PREPARED and self.descriptor_client_owned,
                "submitted descriptor pointer is invalid for this client")
        return self._event("set_callback_before_submit")

    def submit(self, cookie: int = 1) -> "Lifecycle":
        self._admit()
        require(self.phase == Phase.PREPARED, "submit must closely follow successful prepare")
        require(type(cookie) is int and -(2**31) <= cookie < 2**31 and cookie != 0,
                "cookie must be a positive request cookie or a negative error")
        # On error this policy conservatively requires channel stop/synchronize;
        # it does NOT claim every provider started or retains a failed submit.
        failed = cookie < 0
        return self._event("submit_error" if failed else "submit",
                           phase=Phase.SUBMIT_FAILED if failed else Phase.QUEUED,
                           descriptor_client_owned=False, dma_may_access=True,
                           outcome=Outcome.SUBMIT_ERROR if failed else Outcome.NONE)

    def issue_pending(self) -> "Lifecycle":
        self._admit()
        require(self.phase == Phase.QUEUED, "issue_pending requires queued work, never STOPPING")
        return self._event("issue_pending", phase=Phase.ACTIVE)

    def timeout(self) -> "Lifecycle":
        require(self.phase in {Phase.QUEUED, Phase.ACTIVE, Phase.HW_DONE, Phase.CALLBACK},
                "timeout only models an outstanding waiter")
        require(self.outcome == Outcome.NONE, "terminal outcome already chosen")
        # A timeout changes only the request decision, never stop or ownership.
        return self._event("waiter_timeout_not_completion", outcome=Outcome.TIMEOUT)

    def hardware_complete(self, *, ok: bool = True) -> "Lifecycle":
        require(self.phase == Phase.ACTIVE, "normal HW completion requires active transfer")
        require(type(ok) is bool, "completion result must be boolean")
        return self._event("hardware_complete", phase=Phase.HW_DONE,
                           dma_may_access=False, hardware_completed=True,
                           callback_pending=True, completion_ok=ok)

    def callback_enter(self) -> "Lifecycle":
        require(self.phase in {Phase.HW_DONE, Phase.STOPPING}, "no completion callback available")
        require(self.callback_pending and not self.callback_running, "callback is not pending")
        return self._event("callback_enter", phase=Phase.STOPPING if self.phase == Phase.STOPPING else Phase.CALLBACK,
                           callback_pending=False, callback_running=True)

    def callback_publish(self) -> "Lifecycle":
        require(self.callback_running and not self.callback_published, "publish once inside callback")
        outcome = self.outcome
        if outcome == Outcome.NONE:
            outcome = Outcome.SUCCESS if self.completion_ok else Outcome.IO_ERROR
        return self._event("callback_publish_waiter_can_wake", callback_published=True, outcome=outcome)

    def callback_return(self) -> "Lifecycle":
        require(self.callback_running and self.callback_published, "callback must publish then return")
        return self._event("callback_return", phase=Phase.STOPPING if self.phase == Phase.STOPPING else Phase.DONE,
                           callback_running=False)

    def begin_remove(self) -> "Lifecycle":
        require(not self.removal_requested and not self.removed, "removal gate closes once")
        require(self.phase != Phase.PREPARED,
                "this design serializes prepare+submit against removal; don't abandon an unsubmitted descriptor")
        return self._event("remove_close_admission", removal_requested=True)

    def _stop(self, *, synchronous: bool, context: Context, ok: bool) -> "Lifecycle":
        self.check()
        require(isinstance(context, Context) and type(ok) is bool, "invalid termination arguments")
        if synchronous:
            require(context == Context.PROCESS, "terminate_sync must not run in atomic/callback context")
        require(self.phase in _STOPPABLE, "termination requires a submitted request")
        require(synchronous or self.phase != Phase.STOPPING, "synchronize the accepted async termination first")
        if not ok:
            # No success evidence: preserve mapping, callback storage and channel.
            return self._event("stop_failed_quarantine", quarantined=True)
        outcome = Outcome.CANCELLED if self.outcome == Outcome.NONE else self.outcome
        if synchronous:
            return self._event("terminate_sync_success", phase=Phase.QUIESCED,
                               dma_may_access=False, callback_pending=False,
                               callback_running=False, quarantined=False, outcome=outcome)
        return self._event("terminate_async_accepted_not_quiescent", phase=Phase.STOPPING, outcome=outcome)

    def terminate_sync(self, *, context: Context = Context.PROCESS, ok: bool = True) -> "Lifecycle":
        return self._stop(synchronous=True, context=context, ok=ok)

    def terminate_async(self, *, context: Context = Context.ATOMIC, ok: bool = True) -> "Lifecycle":
        return self._stop(synchronous=False, context=context, ok=ok)

    def synchronize(self, *, context: Context = Context.PROCESS) -> "Lifecycle":
        require(isinstance(context, Context) and context == Context.PROCESS,
                "synchronize must not run in atomic/callback context")
        require(self.phase == Phase.STOPPING, "synchronize follows an accepted terminate_async")
        # This event denotes the WAITED RETURN, not entry into the wait.
        return self._event("synchronize_return", phase=Phase.QUIESCED, dma_may_access=False,
                           callback_pending=False, callback_running=False, quarantined=False)

    def unmap_sg(self, *, device: str | None = None, original_nents: int | None = None,
                 direction: Direction | None = None) -> "Lifecycle":
        self.check()
        require(not self.quarantined, "failed stop cannot justify unmap")
        require(self.phase in {Phase.MAPPED, Phase.DONE, Phase.QUIESCED},
                "unmap waits for DMA AND callback quiescence in this design")
        device = self.config.dma_device if device is None else device
        nents = self.config.original_nents if original_nents is None else original_nents
        direction = self.config.direction if direction is None else direction
        require(device == self.config.dma_device and type(nents) is int
                and nents == self.config.original_nents and isinstance(direction, Direction)
                and direction == self.config.direction,
                "unmap must match device, original nents, and mapping direction")
        return self._event("unmap_to_cpu", phase=Phase.UNMAPPED, unmap_count=1)

    def cpu_access(self) -> "Lifecycle":
        require(self.phase in {Phase.CPU, Phase.UNMAPPED}, "CPU accesses only unmapped live storage in this policy")
        return self._event("cpu_access")

    def consume_rx(self) -> "Lifecycle":
        require(self.phase == Phase.UNMAPPED and self.config.direction == Direction.FROM_DEVICE,
                "RX result requires unmapped CPU-owned storage")
        require(self.outcome == Outcome.SUCCESS and self.completion_ok,
                "safe reclamation does not turn timeout/cancel/error into valid RX data")
        return self._event("consume_valid_rx")

    def free_resources(self) -> "Lifecycle":
        self.check()
        require(not self.quarantined and self.phase in {Phase.CPU, Phase.UNMAPPED},
                "free only CPU-owned, unmapped, callback-quiescent resources")
        return self._event("free_buffer_and_callback_storage", phase=Phase.FREED)

    def release_channel(self) -> "Lifecycle":
        require(self.removal_requested and self.phase == Phase.FREED and not self.channel_released,
                "release once after gate closure and cleanup")
        return self._event("release_channel", channel_released=True)

    def finish_remove(self) -> "Lifecycle":
        require(self.removal_requested and self.channel_released and not self.removed,
                "remove cannot finish with outstanding resources/channel")
        return self._event("remove_finished", removed=True)


def demo() -> None:
    state = Lifecycle().map_sg(2).prepare().descriptor_access().submit().issue_pending()
    state = state.timeout().begin_remove().terminate_async()
    for attempted in (state.unmap_sg, state.free_resources, state.issue_pending):
        try:
            attempted()
        except ContractError as error:
            print(f"REJECTED: {error}")
    state = state.synchronize().unmap_sg().free_resources().release_channel().finish_remove()
    for index, event in enumerate(state.trace, 1):
        print(f"{index:02d} {event}")
    print(f"END phase={state.phase.value}, outcome={state.outcome.value}, removed={state.removed}")
    print("Host event model only; no kernel, cache, DMA, hardware, or concurrency verification.")


if __name__ == "__main__":
    demo()
