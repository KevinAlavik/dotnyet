import struct
import sys
from enum import Enum
from typing import List, Dict, Tuple
import re

class Opcode(Enum):
    NOP   = 0x00
    PUSH  = 0x01
    POP   = 0x02
    ADD   = 0x03
    PRINT = 0x04
    DEF   = 0x10
    CALL  = 0x11
    RET   = 0x12
    STORE = 0x20
    LOAD  = 0x21
    JMP   = 0x30
    JZ    = 0x31
    JNZ   = 0x32
    HALT  = 0x40

class ValueTypeTag(Enum):
    Null    = 0
    Integer = 1
    Double  = 2
    Boolean = 3
    String  = 4

class Compiler:
    def __init__(self):
        self.bytecode: List[int] = []
        self.functions: Dict[str, int] = {}
        self.variables: Dict[str, int] = {}
        self.next_var_address = 0
        self.jump_targets: List[Tuple[int, str]] = []
        self.labels: Dict[str, int] = {}

    def emit_byte(self, byte: int):
        self.bytecode.append(byte)

    def emit_uint32(self, value: int):
        self.bytecode.extend(struct.pack('<I', value))

    def emit_int64(self, value: int):
        self.bytecode.extend(struct.pack('<q', value))

    def emit_string(self, value: str):
        self.emit_uint32(len(value))
        self.bytecode.extend(value.encode('utf-8'))

    def compile_push(self, value: str):
        if value.isdigit() or (value.startswith('-') and value[1:].isdigit()):
            self.emit_byte(Opcode.PUSH.value)
            self.emit_byte(ValueTypeTag.Integer.value)
            self.emit_int64(int(value))
        else:
            # Remove quotes for string literals
            if value.startswith('"') and value.endswith('"'):
                value = value[1:-1]
            self.emit_byte(Opcode.PUSH.value)
            self.emit_byte(ValueTypeTag.String.value)
            self.emit_string(value)

    def compile(self, source: str):
        lines = source.split('\n')
        i = 0
        while i < len(lines):
            line = lines[i].strip()
            if not line or line.startswith('#'):
                i += 1
                continue

            # Function definition
            if line.startswith('fn '):
                parts = line.split()
                if len(parts) < 2:
                    raise ValueError(f"Invalid function definition at line {i+1}")
                fname = parts[1].split('(')[0]
                self.functions[fname] = len(self.bytecode)
                self.emit_byte(Opcode.DEF.value)
                self.emit_string(fname)
                i += 1
                continue

            # Variable assignment
            if match := re.match(r'(\w+)\s*=\s*(.+)', line):
                var_name, value = match.groups()
                if var_name not in self.variables:
                    self.variables[var_name] = self.next_var_address
                    self.next_var_address += 1
                self.compile_push(value.strip())
                self.emit_byte(Opcode.STORE.value)
                self.emit_uint32(self.variables[var_name])
                i += 1
                continue

            # If statement
            if line.startswith('if '):
                condition = line[3:].strip()
                self.compile_push(condition)
                self.emit_byte(Opcode.JZ.value)
                jump_pos = len(self.bytecode)
                self.emit_uint32(0)  # Placeholder
                self.jump_targets.append((jump_pos, f"endif_{i}"))
                i += 1
                continue

            # Label (for jumps)
            if line.endswith(':'):
                label = line[:-1]
                self.labels[label] = len(self.bytecode)
                i += 1
                continue

            # Jump
            if line.startswith('goto '):
                label = line[5:].strip()
                self.emit_byte(Opcode.JMP.value)
                jump_pos = len(self.bytecode)
                self.emit_uint32(0)  # Placeholder
                self.jump_targets.append((jump_pos, label))
                i += 1
                continue

            # End of block
            if line == 'endif':
                self.labels[f"endif_{i-1}"] = len(self.bytecode)
                i += 1
                continue

            # Function call
            if match := re.match(r'(\w+)\(\)', line):
                fname = match.group(1)
                self.emit_byte(Opcode.CALL.value)
                self.emit_string(fname)
                i += 1
                continue

            # Print
            if line.startswith('print '):
                value = line[6:].strip()
                self.compile_push(value)
                self.emit_byte(Opcode.PRINT.value)
                i += 1
                continue

            # Variable reference
            if line in self.variables:
                self.emit_byte(Opcode.LOAD.value)
                self.emit_uint32(self.variables[line])
                i += 1
                continue

            # Add operation
            if line == 'add':
                self.emit_byte(Opcode.ADD.value)
                i += 1
                continue

            # Return
            if line == 'return':
                self.emit_byte(Opcode.RET.value)
                i += 1
                continue

            raise ValueError(f"Unknown instruction at line {i+1}: {line}")

        # Resolve jump targets
        for pos, label in self.jump_targets:
            if label not in self.labels:
                raise ValueError(f"Undefined label: {label}")
            target = self.labels[label]
            self.bytecode[pos:pos+4] = struct.pack('<I', target)

        # Append HALT
        self.emit_byte(Opcode.HALT.value)

        # Add magic header and version
        result = bytearray(b'NYET')
        result.append(0x01)  # Version
        result.extend(self.bytecode)
        return result

def main():
    if len(sys.argv) != 3:
        print("Usage: python nyetscript.py <input.nyet> <output.bin>")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]

    with open(input_file, 'r') as f:
        source = f.read()

    compiler = Compiler()
    bytecode = compiler.compile(source)

    with open(output_file, 'wb') as f:
        f.write(bytecode)

if __name__ == "__main__":
    main()