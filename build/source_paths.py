"""Run a compiler with a private C: device map backed by the build output directory.

No machine/logon drive mappings are changed. Source copies and junctions live in the build output directory.
The suspended compiler alone receives the map; handles are closed after it exits.
NtSetInformationProcess(ProcessDeviceMap) is documented by the System Informer
phnt headers and ntdoc. This is source-path staging, never binary modification.
"""
import ctypes as ct
from ctypes import wintypes as wt
import os
from pathlib import Path, PureWindowsPath
import subprocess
import struct
import uuid
import tempfile

def stage(source, compile_path, c_root, access_roots):
    """Stage an original source spelling under the build output directory."""
    path = PureWindowsPath(compile_path)
    if not str(path).lower().startswith('c:\\projects\\cod\\src\\') or '..' in path.parts:
        raise ValueError('unsupported historical source path')
    c_root = Path(c_root).resolve()
    prepare(c_root)
    for access in sorted({Path(p).resolve() for p in access_roots}, key=lambda p: len(p.parts)):
        if access.drive.lower() != 'c:':
            continue
        target = c_root.joinpath(*access.parts[1:])
        if target.exists():
            continue
        target.parent.mkdir(parents=True, exist_ok=True)
        junction(target, access)
    target = c_root.joinpath(*path.parts[1:])
    # A staged source must never resolve through a junction into a supplied tree.
    if not target.resolve().is_relative_to(c_root):
        raise ValueError('historical path overlaps a supplied source/toolchain directory')
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_bytes(Path(source).read_bytes())
    return c_root


def prepare(c_root):
    """Provide the child's read access to Windows through a directory junction."""
    c_root = Path(c_root).resolve()
    c_root.mkdir(parents=True, exist_ok=True)
    windows = c_root / 'Windows'
    if windows.exists():
        assert os.lstat(windows).st_reparse_tag == 0xa0000003, windows
        assert os.readlink(windows).startswith('\\Device\\') and os.readlink(windows).endswith('\\Windows')
        return
    junction(windows, Path(os.environ.get('SystemRoot', 'C:/Windows')))


def junction(target, destination):
    kernel = ct.WinDLL('kernel32', use_last_error=True)
    kernel.QueryDosDeviceW.argtypes = [wt.LPCWSTR, wt.LPWSTR, wt.DWORD]
    kernel.CreateFileW.argtypes = [wt.LPCWSTR, wt.DWORD, wt.DWORD, wt.LPVOID,
                                  wt.DWORD, wt.DWORD, wt.HANDLE]
    kernel.CreateFileW.restype = wt.HANDLE
    kernel.DeviceIoControl.argtypes = [wt.HANDLE, wt.DWORD, wt.LPVOID, wt.DWORD,
                                      wt.LPVOID, wt.DWORD, ct.POINTER(wt.DWORD), wt.LPVOID]
    kernel.CloseHandle.argtypes = [wt.HANDLE]
    device = ct.create_unicode_buffer(32768)
    if not kernel.QueryDosDeviceW(str(destination.drive), device, len(device)):
        raise ct.WinError(ct.get_last_error())
    assert device.value.startswith('\\Device\\')
    substitute = (device.value + str(destination)[2:]).encode('utf-16-le')
    display = str(destination).encode('utf-16-le')
    body = struct.pack('<4H', 0, len(substitute), len(substitute) + 2, len(display))
    body += substitute + b'\0\0' + display + b'\0\0'
    data = struct.pack('<IHH', 0xa0000003, len(body), 0) + body
    target.mkdir()
    handle = kernel.CreateFileW(str(target), 0x40000000, 7, None, 3, 0x02200000, None)
    if handle == wt.HANDLE(-1).value:
        raise ct.WinError(ct.get_last_error())
    try:
        returned = wt.DWORD()
        buffer = ct.create_string_buffer(data)
        if not kernel.DeviceIoControl(handle, 0x900a4, buffer, len(data), None, 0,
                                      ct.byref(returned), None):
            raise ct.WinError(ct.get_last_error())
    finally:
        kernel.CloseHandle(handle)


class UNICODE_STRING(ct.Structure):
    _fields_ = [('Length', wt.USHORT), ('MaximumLength', wt.USHORT), ('Buffer', wt.LPWSTR)]


class OBJECT_ATTRIBUTES(ct.Structure):
    _fields_ = [('Length', wt.ULONG), ('RootDirectory', wt.HANDLE),
                ('ObjectName', ct.POINTER(UNICODE_STRING)), ('Attributes', wt.ULONG),
                ('SecurityDescriptor', wt.LPVOID), ('SecurityQualityOfService', wt.LPVOID)]


class PROCESSENTRY32(ct.Structure):
    _fields_ = [('dwSize', wt.DWORD), ('cntUsage', wt.DWORD), ('th32ProcessID', wt.DWORD),
                ('th32DefaultHeapID', ct.c_size_t), ('th32ModuleID', wt.DWORD),
                ('cntThreads', wt.DWORD), ('th32ParentProcessID', wt.DWORD),
                ('pcPriClassBase', wt.LONG), ('dwFlags', wt.DWORD), ('szExeFile', wt.WCHAR * 260)]


def close_pdb_children(parent_pid, compiler):
    """Stop only compiler-spawned local PDB servers after the compiler exits."""
    k = ct.WinDLL('kernel32', use_last_error=True)
    k.CreateToolhelp32Snapshot.argtypes = [wt.DWORD, wt.DWORD]
    k.CreateToolhelp32Snapshot.restype = wt.HANDLE
    k.Process32FirstW.argtypes = [wt.HANDLE, ct.POINTER(PROCESSENTRY32)]
    k.Process32NextW.argtypes = [wt.HANDLE, ct.POINTER(PROCESSENTRY32)]
    k.OpenProcess.argtypes = [wt.DWORD, wt.BOOL, wt.DWORD]
    k.OpenProcess.restype = wt.HANDLE
    k.QueryFullProcessImageNameW.argtypes = [wt.HANDLE, wt.DWORD, wt.LPWSTR, ct.POINTER(wt.DWORD)]
    k.TerminateProcess.argtypes = [wt.HANDLE, wt.UINT]
    k.WaitForSingleObject.argtypes = [wt.HANDLE, wt.DWORD]
    k.CloseHandle.argtypes = [wt.HANDLE]
    snapshot = k.CreateToolhelp32Snapshot(2, 0)
    if snapshot == wt.HANDLE(-1).value:
        raise ct.WinError(ct.get_last_error())
    try:
        entry = PROCESSENTRY32()
        entry.dwSize = ct.sizeof(entry)
        more = k.Process32FirstW(snapshot, ct.byref(entry))
        while more:
            if entry.th32ParentProcessID == parent_pid and entry.szExeFile.lower() == 'mspdbsrv.exe':
                handle = k.OpenProcess(0x101001, False, entry.th32ProcessID)
                if handle:
                    try:
                        name = ct.create_unicode_buffer(32768)
                        length = wt.DWORD(len(name))
                        if k.QueryFullProcessImageNameW(handle, 0, name, ct.byref(length)):
                            expected = compiler.parent / 'mspdbsrv.exe'
                            if Path(name.value).resolve() == expected:
                                k.TerminateProcess(handle, 0)
                                k.WaitForSingleObject(handle, 5000)
                    finally:
                        k.CloseHandle(handle)
            more = k.Process32NextW(snapshot, ct.byref(entry))
    finally:
        k.CloseHandle(snapshot)


def run(cmd, *, c_root, cwd, env, timeout=300):
    c_root = Path(c_root).resolve()
    assert c_root.is_dir(), c_root
    # Only a compiler process receives this mapping.
    executable = Path(cmd[0]).resolve()
    assert executable.name.lower() == 'cl.exe', executable
    nt = ct.WinDLL('ntdll')
    kernel = ct.WinDLL('kernel32', use_last_error=True)
    nt.NtCreateDirectoryObject.argtypes = [ct.POINTER(wt.HANDLE), wt.ULONG, ct.POINTER(OBJECT_ATTRIBUTES)]
    nt.NtCreateSymbolicLinkObject.argtypes = [ct.POINTER(wt.HANDLE), wt.ULONG,
                                            ct.POINTER(OBJECT_ATTRIBUTES), ct.POINTER(UNICODE_STRING)]
    nt.NtSetInformationProcess.argtypes = [wt.HANDLE, wt.ULONG, wt.LPVOID, wt.ULONG]
    nt.NtResumeProcess.argtypes = [wt.HANDLE]
    nt.NtClose.argtypes = [wt.HANDLE]
    kernel.QueryDosDeviceW.argtypes = [wt.LPCWSTR, wt.LPWSTR, wt.DWORD]
    handles = []
    child = None
    stdout_file = tempfile.TemporaryFile(dir=c_root.parent)
    stderr_file = tempfile.TemporaryFile(dir=c_root.parent)

    def checked(status, operation):
        if status < 0:
            raise OSError('%s: NTSTATUS 0x%08X' % (operation, status & 0xffffffff))

    def string(value):
        buf = ct.create_unicode_buffer(value)
        return UNICODE_STRING(len(value) * 2, (len(value) + 1) * 2, ct.cast(buf, wt.LPWSTR)), buf

    def attributes(name, parent=None):
        value, buf = string(name)
        return OBJECT_ATTRIBUTES(ct.sizeof(OBJECT_ATTRIBUTES), parent, ct.pointer(value), 0x40,
                                 None, None), (value, buf)

    try:
        attrs, keep = attributes('\\BaseNamedObjects\\converter_sources_' + uuid.uuid4().hex)
        directory = wt.HANDLE()
        checked(nt.NtCreateDirectoryObject(ct.byref(directory), 0xf, ct.byref(attrs)), 'create private directory')
        handles.append(directory)
        mappings = []
        backing = None
        for letter in 'ABCDEFGHIJKLMNOPQRSTUVWXYZ':
            target = ct.create_unicode_buffer(32768)
            drive = letter + ':'
            if kernel.QueryDosDeviceW(drive, target, len(target)):
                mappings.append((drive, target.value))
                if drive == c_root.drive.upper():
                    backing = target.value
        if not c_root.drive or backing is None:
            raise ValueError('source staging needs a local drive')
        mappings = [(drive, dest) for drive, dest in mappings if drive != 'C:']
        mappings.append(('C:', backing + str(c_root)[2:]))
        for drive, dest in mappings:
            attrs, keep = attributes(drive, directory)
            value, buf = string(dest)
            link = wt.HANDLE()
            checked(nt.NtCreateSymbolicLinkObject(ct.byref(link), 0xf0001, ct.byref(attrs), ct.byref(value)),
                    'create private ' + drive)
            handles.append(link)
        child = subprocess.Popen(cmd, cwd=str(cwd), env=env, stdout=stdout_file,
                                 stderr=stderr_file, creationflags=0x08000004)
        # PROCESS_DEVICEMAP_INFORMATION.Set is a directory handle.
        checked(nt.NtSetInformationProcess(wt.HANDLE(int(child._handle)), 23,
                                          ct.byref(directory), ct.sizeof(directory)), 'set compiler device map')
        checked(nt.NtResumeProcess(wt.HANDLE(int(child._handle))), 'resume compiler')
        child.wait(timeout=timeout)
        close_pdb_children(child.pid, executable)
        stdout_file.seek(0)
        stderr_file.seek(0)
        stdout, stderr = stdout_file.read(), stderr_file.read()
        return subprocess.CompletedProcess(cmd, child.returncode, stdout, stderr)
    finally:
        if child is not None and child.poll() is None:
            child.kill()
            child.wait()
        if child is not None:
            close_pdb_children(child.pid, executable)
        stdout_file.close()
        stderr_file.close()
        for handle in reversed(handles):
            nt.NtClose(handle)
