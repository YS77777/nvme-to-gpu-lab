# Implementation roadmap

The project deliberately separates portable development from hardware-specific
validation. A successful API call is never treated as evidence of a particular
storage-to-GPU data path.

## Stage 1 — portable CPU baseline (`v0.1`)

**Environment:** the current Linux VM and native Windows.

**Goal:** establish a correct, reproducible file workload before adding GPU
state and timing.

**Tasks:**

- Generate a real deterministic file whose byte at offset `i` is `i % 251`.
- Read the file in bounded blocks through portable C++ streams.
- Handle missing files, final partial blocks, invalid arguments, and size
  suffixes.
- Produce both human-readable and JSON results.
- Calculate a non-cryptographic byte sum and perform exact pattern validation.
- Build and test without CUDA.

**Exit criteria:** CPU smoke tests pass on Linux; the same target builds on
Windows; a 256 MiB file verifies with several block sizes.

## Stage 2 — Windows CUDA baseline (`v0.2`)

**Environment:** Windows host with RTX 4060, Visual Studio Build Tools, and the
CUDA Toolkit.

**Goal:** compare two paths with the same endpoint: data is resident in GPU
memory and has been consumed by the same checksum kernel.

**Tasks:**

- Add a pageable host-memory backend using synchronous `cudaMemcpy`.
- Add a pinned host-memory backend using `cudaMallocHost`, a CUDA stream, and
  `cudaMemcpyAsync`.
- Implement a GPU byte-sum kernel and compare it with the CPU sum.
- Record storage-read, host-to-device, kernel, and end-to-end time separately.
- Test 64 KiB, 1 MiB, and 4 MiB blocks on one deterministic file.

**Exit criteria:** both GPU backends pass verification on the 4060 and publish
repeatable local results. These results are CUDA transfer baselines, not GDS.

## Stage 3 — Linux storage backends (`v0.3`)

**Environment:** WSL2 or native Linux with a visible NVIDIA GPU.

**Goal:** add Linux-specific storage behavior without changing the correctness
contract or output schema.

**Tasks:**

- Add `pread` and an explicitly aligned `O_DIRECT` backend.
- Capture file-system type and whether data lives in a WSL virtual disk,
  Windows-mounted path, or native block device.
- Separate buffered/page-cache results from direct-I/O results.
- Preserve identical byte ranges, block sizes, GPU kernels, and completion
  points across comparisons.

**Exit criteria:** aligned and buffered runs are correctly labeled and no WSL
result is represented as bare-metal NVMe performance.

## Stage 4 — cuFile compatibility (`v0.4`)

**Environment:** a supported Linux/cuFile development environment.

**Goal:** implement the cuFile lifecycle and diagnose its observed path.

**Tasks:**

- Add synchronous `cuFileRead`, handle registration, optional buffer
  registration, and complete resource cleanup.
- Record `requested_backend` independently from `observed_path`.
- Use environment checks, version-appropriate user-space statistics, and a
  short diagnostic run to classify compatibility, mixed, direct, or unverified.
- Reuse NVIDIA `gdsio` as the official benchmark reference rather than
  rebuilding all of its features.

**Exit criteria:** cuFile data verifies correctly and reports an honest path.
The RTX 4060 is expected to remain a compatibility-mode development device.

## Stage 5 — verified native GDS (`v1.0`)

**Environment:** a machine whose GPU, Linux kernel, file system, storage, and
cuFile versions support GDS.

**Goal:** measure traditional staging and verified GDS on the same machine.

**Tasks:**

- Prepare all build and experiment scripts before starting a paid session.
- Save `gdscheck`, topology, versions, configuration, path evidence, and raw
  runs.
- Compare identical data, endpoints, cache policy, block size, and repetitions.
- Report medians and ranges, CPU utilization, setup cost, and end-to-end time.

**Exit criteria:** direct-path claims are backed by run-specific evidence and
all limitations are documented.
