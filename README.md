# nvme-to-gpu-lab

A staged, evidence-first experiment for understanding how file data reaches GPU
memory. The repository starts with a portable CPU baseline, then adds Windows
CUDA transfers, Linux I/O backends, cuFile compatibility, and finally a verified
native GPUDirect Storage run on suitable hardware.

The current release is **stage 1 / v0.1**. It does not contain a GPU backend and
does not make GDS performance claims.

## Why the stages are separated

Development currently happens in an Ubuntu VMware guest that cannot see the
Windows host's RTX 4060. Portable code and CPU tests run in the VM. Stage 2 will
be built and tested on native Windows with the 4060. Linux-specific I/O and
cuFile are later stages because native Windows does not provide those APIs.

See [docs/ROADMAP.md](docs/ROADMAP.md) for goals, tasks, and exit criteria for
every stage.

When continuing from a Windows Codex checkout, start with
[docs/WINDOWS_HANDOFF.md](docs/WINDOWS_HANDOFF.md). The repository-level
[`AGENTS.md`](AGENTS.md) gives a new Codex chat the decisions and validation
rules carried over from the original Linux development conversation.

## Stage 1 build

The VM has GNU Make and `g++`:

```bash
make
make test
```

CMake is also supported when available:

```bash
cmake -S . -B build -DENABLE_CUDA=OFF
cmake --build build --config Release
```

## Generate deterministic data

The generator writes real bytes; it does not create a sparse placeholder. Byte
`i` has value `i % 251`, which makes exact streaming validation possible.

```bash
python3 scripts/generate_file.py \
  --output data/test.bin \
  --size 256MiB \
  --fsync
```

## Run the CPU baseline

```bash
./bin/nvme_gpu_lab \
  --file data/test.bin \
  --backend cpu \
  --block-size 1MiB \
  --verify-pattern
```

Machine-readable output:

```bash
./bin/nvme_gpu_lab \
  --file data/test.bin \
  --block-size 64KiB \
  --verify-pattern \
  --json
```

The `checksum_sum64` field is a small cross-platform checksum intended for the
later GPU kernel. Exact correctness in stage 1 comes from `--verify-pattern`,
not from treating the sum as a cryptographic hash.

## Result interpretation

- `storage_read_ms` measures time spent inside stream reads.
- `processing_ms` measures checksum and optional pattern validation.
- `end_to_end_ms` covers the complete read loop.
- `observed_path=file-to-host` means this release stops in host memory.
- Performance inside VMware is only a development baseline and must not be
  presented as physical NVMe performance.

## Project status

- [x] Deterministic, non-sparse input generator
- [x] Portable block reader
- [x] Final partial-block handling
- [x] CPU checksum and exact pattern verification
- [x] Human and JSON results
- [x] Linux smoke tests
- [ ] Windows pageable-to-GPU backend
- [ ] Windows pinned-to-GPU backend
- [ ] GPU checksum
- [ ] Linux `pread` and `O_DIRECT`
- [ ] cuFile compatibility backend
- [ ] Verified native GDS experiment

## References

The later cuFile implementation will follow NVIDIA's public samples and API
lifecycle without copying their benchmark design:

- NVIDIA MagnumIO GDS samples:
  <https://github.com/NVIDIA/MagnumIO/tree/main/gds/samples>
- NVIDIA GPUDirect Storage documentation:
  <https://docs.nvidia.com/gpudirect-storage/>
- NVIDIA KvikIO:
  <https://github.com/rapidsai/kvikio>

## License

MIT
