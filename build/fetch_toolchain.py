"""Fetch VC7.0 or verify separately supplied VC7.1 and the UO SDK.

Microsoft files are not part of this source release. VC7.0 media downloads and
individual files are pinned by SHA-256. VC7.1/SDK downloads are not automated.
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys

HERE = Path(__file__).resolve().parent


def missing_files(groups):
    bad = []
    for base, files in groups:
        for rel, expected in files.items():
            path = base / rel
            if not path.is_file() or hashlib.sha256(path.read_bytes()).hexdigest() not in (expected if isinstance(expected, list) else [expected]):
                bad.append(str(path))
    return bad


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--toolchain', choices=('all', 'vc70', 'vc71'), default='all')
    parser.add_argument('--dest', type=Path, default=HERE / 'toolchain')
    parser.add_argument('--vc70', type=Path)
    parser.add_argument('--vc71', type=Path)
    parser.add_argument('--sdk', type=Path)
    parser.add_argument('--verify-only', action='store_true')
    parser.add_argument('--7z', dest='sevenzip')
    parser.add_argument('--keep', action='store_true')
    args = parser.parse_args()
    pins = json.loads((HERE / 'toolchain_manifest.json').read_text(encoding='utf-8'))
    names = ['vc70', 'vc71'] if args.toolchain == 'all' else [args.toolchain]
    bad_count = 0
    for name in names:
        vc = (args.vc70 or args.dest / 'vc7') if name == 'vc70' else (args.vc71 or args.dest / 'vc71')
        groups = [(vc, pins[name]['tools'])]
        if name == 'vc71':
            groups.append((args.sdk or args.dest / 'uo_sdk', pins[name]['sdk']))
        bad = missing_files(groups)
        if name == 'vc70' and not args.verify_only and bad:
            if vc.name.lower() != 'vc7':
                raise SystemExit('VC7.0 fetch destination must end with vc7; supplied installations can be verified anywhere')
            command = [sys.executable, '-I', str(HERE / 'fetch_vc70.py'), '--dest', str(vc.parent)]
            if args.sevenzip:
                command += ['--7z', args.sevenzip]
            if args.keep:
                command += ['--keep']
            if subprocess.run(command).returncode:
                return 1
            bad = missing_files(groups)
        print('%s: %d files checked; %d missing or differing' % (name, sum(len(p) for _, p in groups), len(bad)))
        for path in bad[:12]:
            print('  ' + path)
        if bad and name == 'vc71':
            print('Supply VC7.1 13.10.3077 and the documented UO SDK mix; automatic download is unavailable.')
        bad_count += len(bad)
    return int(bool(bad_count))


if __name__ == '__main__':
    sys.exit(main())
