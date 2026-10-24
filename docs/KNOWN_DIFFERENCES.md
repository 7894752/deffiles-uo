# Known differences and original hashes

Measured by compiling and linking all eight DLLs from this exported source snapshot.
No converter DLL was executed or loaded. See BUILD_RESULTS.json for the machine-readable receipt.

| DLL | Bytes | Original SHA-256 | Differing bytes | Result |
|---|---:|---|---:|---|
| defaultconvert.dll | 53,248 | `036ef3220d833f68e2007e7c49cf56a82538963e7314c0282743f7ea4bf9eaa9` | 0 | IDENTICAL |
| configstringconvert.dll | 49,152 | `ea35e7cad99a5c0c33ffa4e1ab0ca7929ea51a06e12702738aa61921f030e830` | 0 | IDENTICAL |
| xmodelalias.dll | 73,728 | `f49ada71d86c53f900beaa16660759151cec2506e07bd888947f727ad122d690` | 0 | IDENTICAL |
| mptype.dll | 81,920 | `cd7aa4bfa6bcad2bfdec7f2025cc3fc80521890878af8b8446f714140adc7265` | 0 | IDENTICAL |
| aitype.dll | 81,920 | `74e932664a828850fe675c070f5075cf847c86c79c41dcc6fb4722a16eaa0b60` | 0 | IDENTICAL |
| character.dll | 81,920 | `9642b67eaf14b05ca10348274f7ca2c7d569e5fa2dfeeca5e48802b96043def0` | 0 | IDENTICAL |
| xanim.dll | 397,312 | `e116dfbeeea7cb18825837a9202493a6b95b22f383bb058b3a07da0c4e419dee` | 0 | IDENTICAL |
| xmodel.dll | 720,896 | `bc710eb763a2d9547344ddeace5ca978657f9a283dd6d50f7aa5a3c94da5f953` | 0 | IDENTICAL |

Identity covers the entire image after recorded timestamp and debug GUID/age normalization.
Code, data, sections, imports, relocations and Rich entries come from the compiler/linker.
The ordinary link output is kept separately; no code/data repairs are applied.

The complete release requires all eight original hashes. See [reconstruction notes](RECONSTRUCTION_NOTES.md).
