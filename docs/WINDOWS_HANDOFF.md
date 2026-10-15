# Windows Codex handoff

This document carries the decisions and next actions from the original Linux
Codex chat into a Windows checkout. It is intentionally stored in Git so a new
Codex chat can recover the project context without access to the old local chat.

## What has already been decided

The project will be developed in stages instead of trying to begin with native
GPUDirect Storage:

1. Portable file-to-host CPU baseline.
2. Native Windows file-to-host-to-GPU CUDA baseline.
3. Linux buffered and direct-I/O backends.
4. cuFile API and compatibility-path experiments.
5. Verified native GDS on suitable Linux server hardware.

The first stage is implemented. It was tested in an Ubuntu VMware guest with a
real 256 MiB deterministic file using 64 KiB, 1 MiB, and 4 MiB blocks. Every
run read the full file, produced the expected checksum, and reported zero
pattern mismatches. The smoke-test suite has four passing tests.

Those concurrent functional runs establish correctness only. They are not
representative storage-performance measurements.

## Why stage 2 runs on Windows

The NVIDIA RTX 4060 belongs to the Windows host. The Ubuntu development system
is a VMware guest with no visible NVIDIA device, `nvidia-smi`, or CUDA compiler.
Native Windows avoids a difficult passthrough setup and directly exercises the
machine that owns the GPU.

The RTX 4060 is appropriate for learning CUDA staging, memory allocation,
streams, events, kernels, and benchmark design. It must not be assumed to
provide a verified native GDS path.

## Prepare the Windows checkout

Install or confirm:

- NVIDIA display driver
- CUDA Toolkit
- Visual Studio 2022 or Visual Studio Build Tools with Desktop development
  with C++
- CMake
- Git
- Python 3

Clone the shared repository onto the Windows filesystem:

```powershell
mkdir C:\code -ErrorAction SilentlyContinue
cd C:\code
git clone https://github.com/YS77777/nvme-to-gpu-lab.git
cd nvme-to-gpu-lab
```

In the Windows ChatGPT/Codex desktop app, add `C:\code\nvme-to-gpu-lab` as a
local project and keep the agent environment on **Windows native** for stage 2.
The repository root must be the primary folder so Codex discovers this
`AGENTS.md` file automatically.

## Preflight

Run and preserve the output:

```powershell
nvidia-smi
nvcc --version
cmake --version
git --version
```

First verify that the portable stage 1 target builds with MSVC:

```powershell
cmake -S . -B build-cpu -DENABLE_CUDA=OFF
cmake --build build-cpu --config Release
python scripts/generate_file.py --output data\windows-smoke.bin --size 64MiB
.\build-cpu\Release\nvme_gpu_lab.exe `
  --file data\windows-smoke.bin `
  --block-size 1MiB `
  --verify-pattern `
  --json
```

Expected correctness conditions:

- `bytes_read` is `67108864`.
- `pattern_mismatches` is `0`.
- `verification_ok` is `true`.

If the generator creates the target executable in a different CMake output
directory, use the actual Release executable path.

## Stage 2 implementation slices

### Slice 1: CUDA build and device inventory

- Change CMake so `ENABLE_CUDA=ON` enables CUDA instead of stopping with the
  stage 1 placeholder error.
- Add a small CUDA runtime wrapper that reports the selected device name,
  compute capability, and relevant memory properties.
- Keep `ENABLE_CUDA=OFF` fully functional on systems without CUDA.

**Done when:** CPU-only and CUDA-enabled Release builds both succeed, and a
device-info or benchmark invocation identifies the RTX 4060.

### Slice 2: pageable transfer backend

- Reuse the existing block-reading and checksum contract.
- Read into ordinary host memory.
- Copy each valid byte range with synchronous `cudaMemcpy`.
- Run the GPU checksum and compare its result with the CPU checksum.
- Report storage, copy, kernel, and end-to-end time separately.

**Done when:** deterministic files, including a final partial block, verify on
the CPU and GPU for 64 KiB, 1 MiB, and 4 MiB block sizes.

### Slice 3: pinned asynchronous backend

- Allocate host buffers with `cudaMallocHost`.
- Use at least one CUDA stream and `cudaMemcpyAsync`.
- Use CUDA events for GPU copy and kernel timing.
- Synchronize at a documented completion point before consuming results.
- Keep output comparable with the pageable backend.

Start with one pinned buffer for correctness. Add double buffering only after
the single-buffer path passes all checks and timing boundaries are clear.

**Done when:** pageable and pinned paths share the same checksum kernel and
produce identical verified results.

### Slice 4: repeatable experiment output

- Add warmup and iteration options.
- Record individual runs in JSON; compute medians from measured runs.
- Include system and build metadata.
- Document the effect of the Windows filesystem cache. Do not present one warm
  cached run as SSD bandwidth.
- Add a results template instead of committing machine-generated large files.

**Done when:** another developer can reproduce the experiment from the README
and understand exactly what each number measures.

## Required command-line behavior

Keep `--backend cpu`. Add explicit names such as:

```text
cuda-pageable
cuda-pinned
```

The final names may differ if they remain unambiguous. Invalid combinations and
unavailable CUDA builds must fail with a useful error. JSON should identify at
least:

- requested backend
- observed path (`file-to-host-to-gpu` for stage 2)
- bytes and block size
- storage-read time
- host-to-device time
- kernel time
- end-to-end time
- CPU and GPU checksums
- verification result
- warmup and measured iteration counts
- GPU, driver, CUDA, OS, and build type

## Stage 2 acceptance checklist

- [ ] Existing CPU behavior still passes on Linux.
- [ ] CPU target builds and verifies on Windows.
- [ ] CUDA Release target builds with MSVC and `nvcc`.
- [ ] Pageable backend verifies all required block sizes.
- [ ] Pinned backend verifies all required block sizes.
- [ ] Final partial blocks verify on both GPU paths.
- [ ] CUDA failures include the failed API and error string.
- [ ] JSON output contains sufficient reproduction metadata.
- [ ] Raw repeated runs are saved without performance exaggeration.
- [ ] README clearly calls the results CUDA staging baselines, not GDS.

## Prompt for the first Windows Codex chat

Paste this into the first chat created from the Windows local project:

```text
Read AGENTS.md, README.md, docs/ROADMAP.md, and docs/WINDOWS_HANDOFF.md.
Continue the repository from stage 1 into stage 2 on native Windows. Begin by
running the documented preflight and the CPU Windows smoke build. Fix any
portable-build issue found. Then implement Slice 1 only: CUDA-enabled CMake and
device inventory, while preserving the CPU-only build. Run the relevant Release
builds and correctness checks, summarize the evidence, and stop before Slice 2
if the environment itself is not verified.
```

After Slice 1 is reviewed, continue with one implementation slice per focused
chat. All slices belong to the same local project and share these repository
instructions.

