#!/usr/bin/env python3
"""Build the Call of Duty: United Offensive (2004) converter plugin DLLs with Visual C++ 7.0/7.1 and compare them with the originals.

Uses Python 3's standard library, the sources in src/ and the manifests in build/sources/<module>.json.
The build compiles and links; it never executes or loads the resulting images.

Examples (run from the repository root):
    python -I build/build.py --module all --fetch-toolchain --strict --identical
    python -I build/build.py --module xanim --vc71 "C:\\Program Files\\Microsoft Visual Studio .NET\\Vc7" --identical

--identical writes <module>_identical.dll with recorded timestamps and debug GUID/age restored.
The printed SHA-256 of the entire file decides identity.
"""
from __future__ import annotations

import argparse
import hashlib
import importlib.util
import json
import os
import string
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

sys.dont_write_bytecode = True

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent
SRC = ROOT / 'src'
DEFAULT_VC7 = HERE / 'toolchain' / 'vc7'
DEFAULT_VC71 = HERE / 'toolchain' / 'vc71'
MODULES = ['defaultconvert', 'configstringconvert', 'xmodelalias', 'mptype', 'aitype', 'character', 'xanim', 'xmodel']

CREATE_NO_WINDOW = 0x08000000 if os.name == 'nt' else 0


def sha256(p):
    h = hashlib.sha256()
    with open(p, 'rb') as f:
        for chunk in iter(lambda: f.read(1 << 20), b''):
            h.update(chunk)
    return h.hexdigest()


def tool_env(vc7):
    env = os.environ.copy()
    for k in ('CL', '_CL_', 'LINK', '_LINK_', 'INCLUDE', 'LIB', 'LIBPATH'):
        env.pop(k, None)
    extra = [vc7 / 'bin']
    for rel in ('../Common7/IDE', '../Common7/Tools', '../Common7/Tools/bin'):
        d = (vc7 / rel).resolve()
        if d.is_dir():
            extra.append(d)
    env['PATH'] = os.pathsep.join([str(d) for d in extra] + [env.get('PATH', '')])
    return env


def run(cmd, cwd, env, timeout=600):
    # A PDB server can inherit pipe handles. File redirection avoids waiting on
    # that server after the tool itself exits, and cleanup is limited to its children.
    with tempfile.TemporaryFile(dir=env.get('TEMP')) as output:
        process = subprocess.Popen(cmd, cwd=str(cwd), env=env, stdout=output,
                                   stderr=subprocess.STDOUT, creationflags=CREATE_NO_WINDOW)
        try:
            process.wait(timeout=timeout)
        finally:
            if process.poll() is None:
                process.kill()
                process.wait()
            source_paths().close_pdb_children(process.pid, Path(cmd[0]).resolve())
        output.seek(0)
        text = output.read().decode('mbcs' if os.name == 'nt' else 'utf-8', 'replace')
    return process.returncode, text


class RootedDrive:
    """A temporary drive letter for sources whose original __FILE__ was a rooted path such as \\trees\\...\\q_shared.c.

    cl.exe records the path it was given. A rooted path (leading backslash, no drive letter) only resolves when the
    current drive holds that directory tree, so the source is staged under build/root and a free drive letter is
    mapped to it with `subst` for the duration of the build. The letter itself never appears in the output.
    """

    def __init__(self, root_dir):
        self.root_dir = root_dir
        self.drive = None

    def ensure(self):
        if self.drive:
            return self.drive
        self.root_dir.mkdir(parents=True, exist_ok=True)
        for letter in reversed(string.ascii_uppercase[3:]):
            drive = letter + ':'
            if os.path.exists(drive + '\\'):
                continue
            r = subprocess.run(['subst', drive, str(self.root_dir)], capture_output=True, text=True,
                               creationflags=CREATE_NO_WINDOW)
            if r.returncode == 0:
                self.drive = drive
                return drive
        raise SystemExit('no free drive letter for subst')

    def release(self):
        if self.drive:
            subprocess.run(['subst', self.drive, '/D'], capture_output=True, creationflags=CREATE_NO_WINDOW)
            self.drive = None


def source_paths():
    spec = importlib.util.spec_from_file_location('converter_source_paths', HERE / 'source_paths.py')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def compile_all(man, vc7, sdk, out, env, rooted):
    (out / 'obj').mkdir(parents=True, exist_ok=True)
    for tu in man['translation_units']:
        src = ROOT / tu['source']
        work = out / 'work' / tu['obj'][:-4]
        work.mkdir(parents=True, exist_ok=True)
        obj = out / 'obj' / tu['obj']
        # __FILE__ is the name cl is given. The original project passed each file relative to its directory, so it is
        # compiled from its directory by bare name. "compile_path" names the original path for the few that differ.
        cwd, name = src.parent, src.name
        cp = tu.get('compile_path')
        c_root = None
        paths = None
        if cp and cp.lower().startswith('c:\\'):
            paths = source_paths()
            c_root = paths.stage(src, cp, out / 'source_root', [ROOT, vc7, sdk, out])
            name = cp
        elif cp and cp.startswith('\\'):
            staged = rooted.root_dir / cp.lstrip('\\')
            staged.parent.mkdir(parents=True, exist_ok=True)
            staged.write_bytes(src.read_bytes())
            cwd, name = Path(rooted.ensure() + os.path.dirname(cp)), cp
        elif cp:
            staged = work / 'stage' / cp
            staged.parent.mkdir(parents=True, exist_ok=True)
            staged.write_bytes(src.read_bytes())
            cwd, name = work / 'stage', cp
        cmd = [str(vc7 / 'bin' / 'cl.exe'), *man['compile_flag_sets'][tu['flags']], '/I' + str(src.parent),
               '/I' + str(SRC), '/I' + str(vc7 / 'include'), '/I' + str(sdk / 'include'),
               '/Fo' + str(obj), '/Fd' + str(work / 'compiler.pdb'), name]
        if '/FR' in cmd:
            cmd[cmd.index('/FR')] = '/FR' + str(work / 'browser.sbr')
        # VC7's automatic precompiled header hangs when TMP/TEMP are overridden; give each object its own PCH path.
        if '/YX' in man['compile_flag_sets'][tu['flags']]:
            # Always start without a precompiled header: reusing one from an earlier build changes the order in which
            # templates are emitted, so a second build would differ from the first.
            (work / 'auto.pch').unlink(missing_ok=True)
            cmd.insert(-1, '/Fp' + str(work / 'auto.pch'))
            e = dict(env)
        else:
            e = dict(env, TMP=str(work), TEMP=str(work))
        if c_root is not None:
            result = paths.run(cmd, c_root=c_root, cwd=cwd, env=e)
            code = result.returncode
            text = (result.stdout + result.stderr).decode('mbcs', 'replace')
        else:
            code, text = run(cmd, cwd, e)
        (work / 'compile.log').write_text(text, encoding='utf-8')
        if code or not obj.exists():
            raise SystemExit('compile failed: %s\n%s' % (tu['source'], text))
        print('compiled', tu['source'])


def link_all(man, vc7, sdk, out, env):
    dll = out / (man['module'] + '.dll')
    # A fresh incremental link is required; an old ILK can change image layout.
    for extension in ('.dll', '.ilk', '.pdb', '.exp', '.lib'):
        stale = out / (man['module'] + extension)
        if stale.exists():
            stale.unlink()
    archived = set()
    archives = {}
    for name, members in man.get('static_libraries', {}).items():
        library = out / (name + '.lib')
        cmd = [str(vc7 / 'bin' / 'lib.exe'), '/nologo', '/OUT:' + str(library)]
        cmd += [str(out / 'obj' / member) for member in members]
        code, text = run(cmd, out, env)
        if code:
            raise SystemExit('archive failed: %s\n%s' % (name, text))
        archived.update(members)
        archives['@' + name] = str(library)
    flags = list(man['link_flags']) + ['/OUT:' + str(dll), '/MAP:' + str(out / (man['module'] + '.map')),
                                       '/LIBPATH:' + str(out), '/LIBPATH:' + str(vc7 / 'lib'), '/LIBPATH:' + str(sdk / 'lib')]
    if man.get('pdb_path'):
        flags += ['/PDB:' + str(out / (man['module'] + '.pdb')), '/PDBALTPATH:' + man['pdb_path']]
    inputs = [str(out / 'obj' / t['obj']) for t in man['translation_units'] if t['obj'] not in archived]
    libraries = [archives.get(name, name) for name in man.get('link_libraries', [])]
    rsp = out / (man['module'] + '.rsp')
    rsp.write_text('\n'.join('"' + a + '"' for a in flags + inputs + libraries) + '\n', encoding='ascii')
    code, text = run([str(vc7 / 'bin' / 'link.exe'), '@' + str(rsp)], out, env)
    (out / 'link.log').write_text(text, encoding='utf-8')
    if code:
        print(text[-4000:])
        raise SystemExit('link.exe failed (exit %d)' % code)
    return dll


def pe_sections(path):
    d = Path(path).read_bytes()
    pe = struct.unpack_from('<I', d, 0x3C)[0]
    nsec, = struct.unpack_from('<H', d, pe + 6)
    optsz, = struct.unpack_from('<H', d, pe + 20)
    out = {}
    for i in range(nsec):
        o = pe + 24 + optsz + 40 * i
        name = d[o:o + 8].rstrip(b'\0').decode('latin1')
        vs, va, rs, rp = struct.unpack_from('<IIII', d, o + 8)
        out[name] = (vs, va, rs, d[rp:rp + rs])
    return d, out


def report(ours, original):
    a, b = Path(ours).read_bytes(), Path(original).read_bytes()
    n = min(len(a), len(b))
    diff = [i for i in range(n) if a[i] != b[i]]
    lines = ['file size : ours %d, original %d' % (len(a), len(b)),
             'file bytes: %d differing in common part%s' % (len(diff), (', first at 0x%X, last at 0x%X' % (diff[0], diff[-1])) if diff else '')]
    _, sa = pe_sections(ours)
    _, sb = pe_sections(original)
    for name in sorted(set(sa) | set(sb)):
        if name not in sa or name not in sb:
            lines.append('%-7s only in %s' % (name, 'ours' if name in sa else 'original'))
            continue
        x, y = sa[name], sb[name]
        m = min(len(x[3]), len(y[3]))
        dd = [i for i in range(m) if x[3][i] != y[3][i]]
        lines.append('%-7s vsize ours 0x%X / orig 0x%X  raw 0x%X / 0x%X  differing %d%s' % (
            name, x[0], y[0], x[2], y[2], len(dd), (' first +0x%X' % dd[0]) if dd else ''))
    return '\n'.join(lines)


def stamp(data, policy):
    """Normalize only recorded timestamps and, for debug images, RSDS GUID/age."""
    spec = importlib.util.spec_from_file_location('converter_stamp', HERE / 'stamp_meta.py')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module.apply_policy(data, policy)


def build_module(module, args, vc7, sdk, rooted):
    man = json.loads((HERE / 'sources' / (module + '.json')).read_text(encoding='utf-8'))
    out = (Path(args.out).resolve() / module) if args.out else HERE / 'out' / module
    out.mkdir(parents=True, exist_ok=True)
    env = tool_env(vc7)
    temp = out / 'temp'
    temp.mkdir(exist_ok=True)
    env['TMP'] = env['TEMP'] = str(temp)
    compile_all(man, vc7, sdk, out, env, rooted)
    dll = link_all(man, vc7, sdk, out, env)
    result = dll
    if args.identical:
        result = out / (module + '_identical.dll')
        result.write_bytes(stamp(dll.read_bytes(), man['stamp']))
    digest = sha256(result)
    print('%s sha256   %s' % (module, digest))
    print('%s original %s' % (module, man['original_sha256']))
    if digest == man['original_sha256']:
        print('RESULT: %s.dll IDENTICAL to the original' % module)
        return {'identical': True, 'sha256': digest, 'original_sha256': man['original_sha256'], 'size': result.stat().st_size}
    print('RESULT: %s.dll differs from the original' % module)
    orig = Path(args.original_dir) / (module + '.dll') if args.original_dir else None
    if orig and orig.is_file():
        rep = report(result, orig)
        (out / 'diff_report.txt').write_text(rep + '\n', encoding='utf-8')
        print(rep)
    return {'identical': False, 'sha256': digest, 'original_sha256': man['original_sha256'], 'size': result.stat().st_size}


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--module', default='all', help='one of %s, or all (default)' % ', '.join(MODULES))
    ap.add_argument('--vc70', '--vc7', default=str(DEFAULT_VC7), help='Visual C++ 7.0 Vc7 directory')
    ap.add_argument('--vc71', default=str(DEFAULT_VC71), help='Visual C++ 7.1 Vc7 directory')
    ap.add_argument('--sdk', default=str(HERE / 'toolchain' / 'uo_sdk'), help='UO SDK headers/import libraries for VC7.1')
    ap.add_argument('--fetch-toolchain', action='store_true', help='fetch VC7.0 if needed; VC7.1 must be supplied')
    ap.add_argument('--report', help='write per-DLL hashes and identity results as JSON')
    ap.add_argument('--out', help='output folder (default: build/out/<module>)')
    ap.add_argument('--identical', action='store_true', help='also write <module>_identical.dll with recorded link metadata')
    ap.add_argument('--original-dir', help='folder with your original DLLs, for a difference report when a build differs')
    ap.add_argument('--strict', action='store_true', help='treat a toolchain hash mismatch as an error')
    args = ap.parse_args()

    modules = MODULES if args.module == 'all' else [args.module]
    for m in modules:
        if m not in MODULES:
            raise SystemExit('unknown module: ' + m)
    pins = json.loads((HERE / 'toolchain_manifest.json').read_text(encoding='utf-8'))
    toolchains = {'vc70': Path(args.vc70).resolve(), 'vc71': Path(args.vc71).resolve()}
    needed = {json.loads((HERE / 'sources' / (m + '.json')).read_text(encoding='utf-8'))['toolchain'] for m in modules}
    for name in sorted(needed):
        vc = toolchains[name]
        if args.fetch_toolchain:
            fetch = [sys.executable, '-I', str(HERE / 'fetch_toolchain.py'), '--toolchain', name,
                     '--vc70', str(toolchains['vc70']), '--vc71', str(toolchains['vc71']), '--sdk', args.sdk]
            if subprocess.run(fetch).returncode:
                raise SystemExit('toolchain fetch failed; see build/README.md')
        bases = [(vc, pins[name]['tools'])]
        if name == 'vc71':
            bases.append((Path(args.sdk).resolve(), pins[name]['sdk']))
        bad = [str(base / rel) for base, hashes in bases for rel, digest in hashes.items()
               if not (base / rel).is_file() or sha256(base / rel) not in (digest if isinstance(digest, list) else [digest])]
        if bad:
            print('WARNING: %s toolchain files missing or differing: %s' % (name, ', '.join(bad[:8])))
            if args.strict:
                raise SystemExit('--strict: toolchain mismatch')
    rooted = RootedDrive(HERE / 'root')
    results = {}
    try:
        for m in modules:
            man = json.loads((HERE / 'sources' / (m + '.json')).read_text(encoding='utf-8'))
            vc7 = toolchains[man['toolchain']]
            sdk = Path(args.sdk).resolve() if man['toolchain'] == 'vc71' else vc7 / 'PlatformSDK'
            results[m] = build_module(m, args, vc7, sdk, rooted)
    finally:
        rooted.release()
    if len(modules) > 1:
        for m in modules:
            print('%-20s %s' % (m, 'IDENTICAL' if results[m]['identical'] else 'differs'))
    if args.report:
        Path(args.report).resolve().write_text(json.dumps(results, indent=2) + '\n', encoding='utf-8')
    return 0 if all(r['identical'] for r in results.values()) else 1


if __name__ == '__main__':
    sys.exit(main())
