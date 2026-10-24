# deffiles — Call of Duty: United Offensive Converter Plugins (Byte-Identical Source)

Compiles with Visual C++ 7.0 and 7.1 to the exact bytes of the eight converter DLLs that shipped with the 2004 Call of Duty: United
Offensive mod tools.

The converter plugins turn the asset manager's entries (`.gdf` data) into game files. Five plugins are unchanged from the 2003 Call
of Duty mod tools and share the [Call of Duty deffiles](https://github.com/7894752/deffiles) reconstruction; the character, animation
and model converters are the United Offensive revisions, built with Visual C++ 7.1. The source is written so that the original
compilers reproduce the original machine code.

This project is not affiliated with or endorsed by Activision, Infinity Ward, Gray Matter, id Software, ZeniMax, NVIDIA or Microsoft.
"Call of Duty", "Quake" and "Wolfenstein" are trademarks of their respective owners and are used here only to describe what the code
is. No game data, no original binary and no Microsoft software is part of this repository.

## The plugins

| DLL | What it does |
|---|---|
| `xmodel.dll` | Builds models from `XMODEL_EXPORT` files: the xmodel with its LODs, the bone parts and the render surfaces (triangle strips), collision surfaces and skin overrides. |
| `xanim.dll` | Builds animations from `XANIM_EXPORT` files: bone parts, compressed frames, notetracks, multiplayer bone stabilizers. |
| `character.dll` | Writes a character script (`.gsc`) that sets the body and head models (directly or from a model alias) and adds the hat, view model and attachments. |
| `aitype.dll` | Writes an AI type script (`.gsc`) with its level-editor spawner definition, weapons and character choices. |
| `mptype.dll` | Writes a multiplayer type script (`.gsc`) that picks one of its characters at random and precaches them all. |
| `xmodelalias.dll` | Writes a model alias script (`.gsc`) that returns the list of models to choose from. |
| `configstringconvert.dll` | Writes an entry as a configstring file (`\key\value` pairs), as used for weapons, info files, player models and vehicles. |
| `defaultconvert.dll` | The converter for entries without their own plugin: writes the entry's keys and values to a text file. |

## Status

| DLL | Compiler | Project functions | Result |
|---|---|---:|---|
| defaultconvert.dll | VC7.0 | 3 | byte-identical |
| configstringconvert.dll | VC7.0 | 3 | byte-identical |
| xmodelalias.dll | VC7.0 | 29 | byte-identical |
| mptype.dll | VC7.0 | 30 | byte-identical |
| aitype.dll | VC7.0 | 30 | byte-identical |
| character.dll | VC7.1 | 15 | byte-identical |
| xanim.dll | VC7.1 | 359 | byte-identical |
| xmodel.dll | VC7.1 | 403 | byte-identical |

- Every project function compiles to the same bytes and relocations as the original, checked one by one (872 in total).
- Each DLL links normally. `--identical` writes a separate copy with the recorded link timestamp and debug GUID/age restored. Those
  copies are byte-identical to the originals; the SHA-256 of each is printed next to the original's. Code, data, imports,
  relocations, layout and the Rich header come from compilation and linking.
- Byte identity proves the compiled image, not the source text: where several spellings compile to the same bytes, the choice among
  them is not evidence about the original. See [docs/RECONSTRUCTION_NOTES.md](docs/RECONSTRUCTION_NOTES.md).

Details: [docs/KNOWN_DIFFERENCES.md](docs/KNOWN_DIFFERENCES.md).

## Requirements

- Windows and Python 3 (standard library only).
- Visual C++ 7.0, `cl.exe` 13.00.9466, with its Platform SDK and static CRT: either your own copy, or let the build download it
  (`--fetch-toolchain`, needs 7-Zip).
- Visual C++ 7.1, `cl.exe` 13.10.3077, and the Platform SDK mix used for United Offensive: your own copies (see
  [build/README.md](build/README.md)). Other compilers do not give the same code.

## Build

```bat
python -I build\build.py --module all --strict --identical
```

or, with installed compilers in other folders:

```bat
python -I build\build.py --module all --vc70 <VC70-folder> --vc71 <VC71-folder> --sdk <UO-SDK-folder> --strict --identical
```

The build compiles each DLL's translation units, links it, and prints the SHA-256 of the result next to the recorded release hash.
It never runs or loads what it builds. Add `--original-dir C:\path\to\deffiles` to get a byte and section comparison with your own
copies if a result differs. All options are in [build/README.md](build/README.md).

## Binaries

This repository contains no DLLs. If you just need the converters, use the official Call of Duty: United Offensive mod tools.

## How the match is established

Each function is compiled with the original compiler and measured flags. Its instruction bytes, relocation targets, jump tables and
C++ exception-handling contributions must agree with the original. The complete linked image is then compared byte for byte,
including headers, data, imports, relocations and padding. The per-function checks used private tooling; the standalone build needs
only the source in `src/` and the manifests in `build/sources/`.

## Repository layout

```
src/        the reconstruction (deffiles/<plugin>/, deffiles/shared/, common/, universal/, q3map/, libs/nvtristrip/)
build/      build.py, build.bat, sources/<plugin>.json, fetch_toolchain.py, toolchain_manifest.json, README.md
docs/       known differences, notes on what is proven and what is a choice, reconstructed names, source hashes
COPYING     GPLv3 with id Software's additional terms
THIRD_PARTY_NOTICES.md
```

## License

GPL-3.0-or-later with the additional terms id Software applied to its Wolfenstein source releases: see [COPYING](COPYING). Files that
descend from the Quake III Arena source keep their original headers (GPL version 2 or, at your option, any later version), which is used
here under version 3. NVIDIA's NvTriStrip and Stan Melax's vector library are included with their attribution; see
[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

## Credits

- Reconstruction by Jane.
- Published by Rose.
- Thanks to Prawy (https://github.com/PrawyCoD1) for the idea for this project.

Evidence and sources:

- id Software: Quake III Arena and the Return to Castle Wolfenstein and Wolfenstein: Enemy Territory source releases, the base of
  the shared math, string, parsing and command libraries.
- NVIDIA: NvTriStrip (Curtis Beeson, Joe Demers, Jason Regier, Jon Stone), here in the early revision preserved in the Torque Game
  Engine source.
- Stan Melax: the 1998 BunnyLOD vector library, here as preserved in the OpenFOAM source mirror.
- The Call of Duty (2003) releases of the same converter plugins, and the later releases in the Call of Duty 2, World at War and
  Black Ops mod tools, which show the earlier and later state of the same files.
- Function and variable names come from strings in the binaries, the published sources above and the symbol tables and link maps of
  other retail builds of the same game family, wherever the compiled image agrees; the rest are descriptive.

## Ports

This repository is the byte-identical reference build and stays that way. Ports to x64, to modern compilers or to other
platforms are welcome as forks; the hope is that someone takes this on.
