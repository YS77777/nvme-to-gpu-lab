# Codex project instructions

## Project purpose

This repository is a staged, evidence-first lab for learning how file data
reaches GPU memory. Keep every benchmark honest about the path it actually
measures. A CUDA copy benchmark, a cuFile compatibility run, and verified
native GPUDirect Storage are three different results.

Read these files before changing code:

1. `README.md`
2. `docs/ROADMAP.md`
3. `docs/WINDOWS_HANDOFF.md`

## Current state

- Stage 1 / v0.1 is implemented and verified in an Ubuntu VMware guest.
- The CPU baseline reads deterministic files in blocks, calculates `sum64`,
  optionally checks every byte, and prints human or JSON output.
- Linux smoke tests pass. The first Windows task is to confirm the portable
  CPU target builds and verifies the same workload with MSVC.
- The Windows host has an NVIDIA RTX 4060, 16 GiB RAM, and a 1 TB SSD.
- The Ubuntu VMware guest cannot see the GPU. Do not spend time attempting
  VMware GPU passthrough for this project.
- Commit `55efb55` is the original stage 1 baseline.

## Environment responsibilities

- Use native Windows for stage 2 CUDA development and RTX 4060 validation.
- Use the Linux VM for portable CPU work and later Linux I/O code that does
  not need a GPU.
- WSL2 or native Linux may be used for stage 3, but label WSL storage paths
  separately from native Linux storage.
- Native GDS claims belong only to stage 5 on supported Linux, GPU, filesystem,
  storage, driver, and CUDA/cuFile combinations.

## Immediate task: stage 2 / v0.2

Start by recording the output of:

```powershell
nvidia-smi
nvcc --version
cmake --version
git --version
```

Then complete the stage 2 tasks in `docs/WINDOWS_HANDOFF.md`. Preserve the CPU
backend and its output fields while adding CUDA-specific fields.

Required CUDA paths:

1. Pageable host buffer with synchronous `cudaMemcpy`.
2. Page-locked host buffer allocated with `cudaMallocHost`, a CUDA stream, and
   `cudaMemcpyAsync`.
3. One GPU checksum implementation shared by both paths.

## Correctness and measurement rules

- Compare the GPU checksum with the CPU checksum for every measured run.
- Keep exact deterministic-pattern verification available on the CPU path.
- Time storage reads, host-to-device copies, GPU work, and end-to-end execution
  separately. Define every timing boundary in the output documentation.
- Synchronize at the actual completion point before recording asynchronous GPU
  timings.
- Use real, non-sparse files from `scripts/generate_file.py`.
- Run warmups and repeated measured iterations before publishing results.
- Save raw results under `results/`; do not commit large generated data files.
- Record the OS, GPU, driver, CUDA version, build type, test file size, block
  size, and iteration count with benchmark results.
- Treat the RTX 4060 results as file-to-host-to-GPU CUDA baselines. Do not label
  them as GPUDirect Storage.

## Build and validation expectations

- Keep the default CPU-only build working when CUDA is unavailable.
- Gate CUDA sources behind `ENABLE_CUDA=ON` in CMake.
- Make failures actionable: report CUDA API names and error strings.
- Prefer small correctness tests over performance assertions.
- Before committing, build the relevant Release target, run correctness tests,
  inspect `git diff --check`, and summarize commands and outcomes.

