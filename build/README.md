# Building the United Offensive converter DLLs

Windows and Python 3.9 or later (standard library only) are required. The build runs compiler, librarian
and linker tools; it never executes or loads the resulting DLLs. No Microsoft files are distributed.

## Required tools

| Modules | Compiler/linker | Runtime | SDK |
|---|---|---|---|
| defaultconvert, configstringconvert, xmodelalias, mptype, aitype | VC7.0, cl 13.00.9466 / link 7.00.9466 | static release CRT | August 2001 Platform SDK |
| character | VC7.1, cl 13.10.3077 / link 7.10.3077 | static release CRT | UO mix below |
| xanim, xmodel | VC7.1, cl 13.10.3077 / link 7.10.3077 | static CRTs, including debug C++ library | UO mix below |

The UO SDK uses August 2001 headers with the VC7.1 compiler headers; `kernel32` comes from VS .NET 2003.
The `user32` import library is the July 2002 SDK revision (Rich build 2067), and `gdi32` is the February
2003 revision (build 2179). The manifest pins all supplied compiler, CRT, header and SDK files, including
the historical library mix. Similar SDKs can link successfully while producing different bytes.
The VC7.0 compiler host runtime may be the media version or its pinned 7.00.9981 update; this DLL is
not linked into a converter. Compiler and linked CRT library pins remain exact.
VC7.1 media provenance is Visual Studio .NET 2003 Professional, build 3077. Supply these files under
their own licences. Automatic download of VC7.1 and the SDK mix is unavailable.

## Commands

With tools in `build/toolchain/vc7`, `build/toolchain/vc71` and `build/toolchain/uo_sdk`:

```bat
python -I build\fetch_toolchain.py --verify-only
python -I build\build.py --module all --strict --identical
```

With supplied installations:

```bat
python -I build\build.py --module all --vc70 <VC70-folder> --vc71 <VC71-folder> --sdk <UO-SDK-folder> --strict --identical
```

| Option | Meaning |
|---|---|
| `--module NAME` | One DLL name or `all` (default). |
| `--vc70 DIR`, `--vc71 DIR` | Compiler folders containing `bin`, `include` and `lib`. |
| `--sdk DIR` | UO SDK folder containing `include` and `lib`; VC7.0 uses its own `PlatformSDK`. |
| `--fetch-toolchain` | Verify supplied files and repair missing/differing VC7.0 files; VC7.1 requires supplied files. |
| `--identical` | Keep the ordinary link and write a copy with recorded metadata restored. |
| `--strict` | Fail if any pinned toolchain/SDK file is missing or differs. |
| `--out DIR` | Output parent, with one subdirectory per DLL. Default: `build/out`. |
| `--report FILE` | Write complete per-DLL hashes, sizes and identity booleans as JSON. |
| `--original-dir DIR` | Compare differing images with your own original DLLs. |

The build prints every resulting SHA-256 beside the recorded original hash. All eight must match for a
complete release. Current validation: 8 of eight original hashes match; the complete build requires all eight.

## Optional VC7.0 download

`fetch_toolchain.py --toolchain vc70` delegates to the CoD1 fetcher, using pinned Microsoft retail disc
images and August 2001 SDK media. It requires 7-Zip. Archive and extracted-file hashes are verified;
installers are never run. The download manifest lists exact media names, archive members, sizes and hashes.

```bat
python -I build\fetch_toolchain.py --toolchain vc70 --7z "C:\Program Files\7-Zip\7z.exe"
```

## Original source paths and output

Assertion strings and edit-and-continue records retain historical source names. Bare or relative names
are compiled with their original spelling. Rooted names use a temporary `subst` drive which is removed
after the build. Absolute historical C: paths use a compiler-process-only device map backed by a source
copy under the output directory. Directory junctions provide read access to Windows and supplied tools;
the actual system drive and machine-wide mappings are unchanged. The helper uses Windows native process
device-map APIs, so restricted environments may reject that operation. PDB-server children started by
the compiler are closed when that compilation finishes.

Objects, logs, PCH/PDB/ILK files and reconstructed DLLs stay in the selected output directory. Each
incremental link begins without an old DLL/PDB/ILK, and automatic PCHs are removed before compilation.
The metadata stamp restores timestamps and RSDS GUID/age only; it performs no code or data repair.
