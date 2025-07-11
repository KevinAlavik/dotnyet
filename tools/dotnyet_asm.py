import struct
import sys

# Opcodes
OPCODES = {
    "nop": 0x00,
    "push": 0x01,
    "pop": 0x02,
    "add": 0x03,
    "print": 0x04,
}

# Type tags
TYPES = {
    "null": 0x00,
    "int": 0x01,
    "double": 0x02,
    "bool": 0x03,
    "string": 0x04,
}

def encode_push(value: str) -> bytes:
    data = bytearray()
    # Null
    if value.lower() == "null":
        data.append(TYPES["null"])
    # String
    elif value.startswith('"') and value.endswith('"'):
        data.append(TYPES["string"])
        unquoted = value[1:-1]
        encoded = unquoted.encode("utf-8")
        data += struct.pack("<I", len(encoded))
        data += encoded
    # Boolean
    elif value.lower() in ("true", "false"):
        data.append(TYPES["bool"])
        data.append(1 if value.lower() == "true" else 0)
    # Float
    elif "." in value:
        data.append(TYPES["double"])
        data += struct.pack("<d", float(value))
    # Int
    else:
        data.append(TYPES["int"])
        data += struct.pack("<q", int(value))
    return data

def assemble(lines: list[str]) -> bytes:
    bytecode = bytearray()

    # { char magic[4]; uint8_t ver; }
    bytecode += b"NYET"
    bytecode.append(0x01)  # version 1

    for line in lines:
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        tokens = line.split(maxsplit=1)
        op = tokens[0].lower()
        if op not in OPCODES:
            raise ValueError(f"Unknown instruction: {op}")
        bytecode.append(OPCODES[op])
        if op == "push":
            if len(tokens) < 2:
                raise ValueError("Missing value for PUSH")
            bytecode += encode_push(tokens[1])
    return bytes(bytecode)

def main():
    if len(sys.argv) != 3:
        print("Usage: python3 dotnyet_asm.py <input> <output>")
        return

    with open(sys.argv[1], "r") as f:
        lines = f.readlines()

    bytecode = assemble(lines)

    with open(sys.argv[2], "wb") as f:
        f.write(bytecode)

    print(f"Wrote {len(bytecode)} bytes to {sys.argv[2]}")

if __name__ == "__main__":
    main()
