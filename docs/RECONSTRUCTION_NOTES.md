# Reconstruction notes: evidence and choices

All eight linked DLLs match their original images. Full-image equality
proves compiled bytes, including code, data, imports, relocations, padding and headers; it cannot prove the
original spelling of the source, comments, or implementation of code discarded by the linker.

## Evidence for names and types

Direct evidence takes priority: exports, assertion expressions, diagnostic messages, source-path strings,
published engine-family sources, converter interfaces and symbol information from related retail builds.
An ordinary descriptive name is used where these do not identify the original spelling.

The VC7.1 debug images contain incremental-link jump tables. LINK places their decorated symbol names
according to its name-hash rules. Reproducing that order constrains class, method and structure spellings:
for example `LineNumberAddress`, `FunctionAddress`, `AnimConvertInfo`, `ReadModelMatchList`,
`ComputeVertexNormals`, `ReadBaseBones`, `VerifyBlendedVertexRoot` and `AddQuatBoneList`.
Some names have independent interface or diagnostic evidence; others are plausible candidates selected
by the hash/order constraints and complete body comparisons. Hashes can collide. An exact jump-table
order does not uniquely recover a source identifier, and these inferences are not claimed as original text.

The model helpers `CreateOptimizedTriangleIndices`, `CopySurfaceSkinFiles`, `CopyTextureFile`,
`PrepareSkinNames`, `LoadObjectData` and `LoadSkelGeometryFile` are also role and interface inferences.
Their access, pointer qualifiers and the signed 32-bit `long` override count reproduce the complete
caller graphs and jump-table order. The retained file-copy diagnostics still use the older `CopyFile`
spelling. No unique historical declaration is claimed for these helpers.

With optimization disabled, VC7.1 also assigns local stack homes partly from identifier hashes. Local
names therefore affect generated code. Ordinary names consistent with types, diagnostics and surrounding
code are retained when their stack layouts agree; no padding locals or dummy computation are required.

## Source families and file layout

The five unchanged DLLs use the Call of Duty source revision. The other three have separate United
Offensive source copies. Files are not merged across those revisions with module conditionals.

The original model and animation DLLs name the same `buildmds.cpp`, with shared assertion coordinates.
The animation copy preserves a functional `XMODEL_CONVERTER` configuration containing model operations
from the same United Offensive family. Surface, shader, collision and rigid-model operations occupy the
source regions absent from the animation configuration. That alternate mode has been statically compiled
and linked against the exported model objects. Seventy compiler-computed structure sizes and member
offsets agree with the model interface; no probe or converter was executed. The configuration name and comment wording are reconstruction
choices, not uniquely recovered historical text. No alternate-mode byte identity is implied.

## The #line decision

The animation reconstruction contains no #line directives. Physical source layout, useful format/API
comments, the real shared-family model operations and the portable math branch preserve the measured
assertion coordinates. This decision differs from the older Call of Duty release's documented line
markers; those markers have not been carried into this release. Comments and formatting are not claimed
to be recovered verbatim. Any future line-marker change requires an explicit documented decision and
a fresh complete-image comparison; the current export gate rejects directives in source files.

## Compiler and link details

- The unchanged DLLs use VC7.0. Character uses an optimized VC7.1 build. Animation and model conversion
  use VC7.1 edit-and-continue, unoptimized frames, runtime checks and an incremental debug link.
- Translation-unit and library order, declarations visible to callers, inline visibility and CRT selection
  affect the linked layout. Each manifest records its measured compile and link inputs.
- The little-endian API has C-linkage inline declarations before its scalar identity definitions.
  VC7.1 then emits early weak COMDAT parents; duplicate associative runtime-check contributions are
  discarded by LINK. This restores the animation image's eight runtime-check registration pairs without
  adding state, dummy helpers or altered runtime-check flags.
- Automatic precompiled headers start cold on each compilation. Reusing one can change template emission order.
- The model's NvTriStrip revision uses ordinary header inline definitions and inferred single-threaded
  static debug-library compilation (`/MLd`, `/Od`, `/YX`). It needs neither `hash_map`, `tchar.h`,
  browser-information generation nor cache methods moved into the implementation file. The normal
  linker reports the `LIBCD` default-library conflict; it is not suppressed. Existing multithreaded
  runtime selections resolve the calls and the entire image, including imports and CRT code, matches.
  This mixed library context is a measured reconstruction inference, not a recovered historical option.
  The ownership-copy API and remaining header context are documented source choices; full-image
  equality does not authenticate a unique original implementation.
- The supplied build reproduces the source spellings recorded by the compiler using isolated source-path
  staging. Original debug paths are historical metadata, rather than locations on the build machine.
- Existing x86 inline assembly comes from the original engine math/assertion idioms. No embedded image
  arrays, instruction emission directives or post-link code/data repairs are used.

## Metadata normalization and limits

The normal link produces code, data, imports, relocations, sections and the Rich header. A separate
`_identical.dll` copy receives only recorded COFF/export/debug timestamps and, for debug images, RSDS
GUID and age. The embedded PDB path is produced by the linker from its manifest option. The complete
file hash remains the gate; reproducing metadata alone establishes no code identity.

The standalone build needs only the exported sources, manifests, Python and separately supplied
Microsoft tools. It never executes or loads a reconstructed converter. The source inventory records
exact file hashes so the tested snapshot can be identified.

## Credits

- Reconstruction by Jane.
- Published by Rose.
- Thanks to Prawy (https://github.com/PrawyCoD1) for the idea for this project.

## Conservative identifier catalogue

All active converter and library sources have been reviewed by function,
including their alternate configurations and header context. The accompanying
[identifier catalogue](RECONSTRUCTED_NAMES.json) records declarations, naming
reasons and exact source hashes for that scope. It includes inherited donor names
as well as descriptive inferences, so an unverified spelling is not silently
presented as authenticated original source. The catalogue covers the complete active source closure. NVIDIA and Quake donor
names are distinguished from inferred converter interfaces, include guards and
reconstruction choices. Reuse of the finished CoD1 reconstruction does not by
itself authenticate a historical IW spelling.

`Vector2Copy` and `Vector4Copy` describe the two quaternion copy widths, following
the existing vector macro family. The lean-position update uses `VectorMA`.
These ordinary utility spellings are inferred; their complete compiled results
retain the original bytes. Duplicate math declaration blocks and an unused
generic-text header have been removed without changing either DLL.

The animation conversion wrapper retains a nullable optional model owner that
is never loaded. Its zero initialization and two guarded cleanup paths occur in
the original unoptimized code. Direct code inspection and a compiler-only
minimal example establish this as a retained source leftover. Removing it,
using ordinary unconditional delete, or replacing it with a standard ownership
wrapper changes the original code or exception handling. The `matchModel` name
describes its inferred role in the model-reference conversion family.

## Stripifier copy APIs and source attribution

The cache-copy operation retains NVIDIA's `VertexCache::Copy` API. Its read-only
`std::copy` input range and self-copy guard are reconstruction choices. The
algorithm dependency is included with that implementation in the internal cache
header. Moving it into the public stripifier header changes deletion declarations
visible to callers and fails the complete-image comparison.

`PrimitiveGroup` owns an index buffer. Its explicit deep-copy constructor was
already present in the reconstructed header, but is absent from the retained
primary NVIDIA header. It remains an inferred ownership API, not authenticated
original source. Removal, destructor order, ordinary copy implementations and
compiler/header-context controls have been tested. The current source preserves
the complete original image; that equality does not uniquely establish why the
historical header differed. This attribution limitation remains explicit.

The shared stack-trace formatter keeps a mutable cursor for trimming decorated
symbols and read-only views for source/object path components. Owned character
buffers use matching array allocation and deletion. These cleanups preserve
the complete images. Some older assertion interfaces retain C-style qualifier
casts; exact historical pointer qualifiers remain unproven.

The existing interned-name release and string-table cleanup are preserved.
The assertion-source cleanup does not claim to repair original error paths.

## Stripifier utility-header context

The early standard `utility` include is retained from the finished CoD1 source
family as inferred SDK/STL declaration context. It is absent from the retained
primary NVIDIA public implementation, and its historical presence is not
authenticated. Removing both NV includes changes 1,229 image bytes. Eight real
dependency-order alternatives fail to restore the complete target order. A
28-line compiler-only example shows why context matters: parsing utility before
the Windows header shifts later inline-method header IL by 706 bytes while
leaving the common native instruction bodies unchanged. The same include has no
offset effect without the Windows header. This establishes a measured context
interaction rather than a uniquely recovered historical header or PCH.
