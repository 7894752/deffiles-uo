"""Normalize recorded PE link metadata in a separate image, without loading it."""
import struct


def apply_policy(data, policy):
    image = bytearray(data)
    pe = struct.unpack_from('<I', image, 0x3c)[0]
    if image[pe:pe + 4] != b'PE\0\0':
        raise ValueError('expected a linked PE image')
    count = struct.unpack_from('<H', image, pe + 6)[0]
    optional_size = struct.unpack_from('<H', image, pe + 20)[0]
    optional = pe + 24
    sections = []
    for i in range(count):
        offset = optional + optional_size + 40 * i
        virtual_size, address, raw_size, raw = struct.unpack_from('<IIII', image, offset + 8)
        sections.append((address, max(virtual_size, raw_size), raw))

    def file_offset(rva):
        for address, size, raw in sections:
            if address <= rva < address + size:
                return raw + rva - address
        raise ValueError('metadata RVA outside PE sections')

    timestamp = policy['timestamp']
    struct.pack_into('<I', image, pe + 8, timestamp)
    export_rva = struct.unpack_from('<I', image, optional + 96)[0]
    if export_rva:
        struct.pack_into('<I', image, file_offset(export_rva) + 4, timestamp)
    debug_rva, debug_size = struct.unpack_from('<II', image, optional + 96 + 6 * 8)
    codeview_count = 0
    for i in range(debug_size // 28):
        offset = file_offset(debug_rva) + i * 28
        struct.pack_into('<I', image, offset + 4, timestamp)
        kind, size, address, raw = struct.unpack_from('<IIII', image, offset + 12)
        if kind == 2 and 'rsds_guid_hex' in policy:
            guid = bytes.fromhex(policy['rsds_guid_hex'])
            if len(guid) != 16 or image[raw:raw + 4] != b'RSDS' or size < 24:
                raise ValueError('expected a valid RSDS record')
            image[raw + 4:raw + 20] = guid
            struct.pack_into('<I', image, raw + 20, policy['rsds_age'])
            codeview_count += 1
    if 'rsds_guid_hex' in policy and codeview_count != 1:
        raise ValueError('expected exactly one RSDS record')
    return bytes(image)
