# .NYET Bytecode Format Specification

## Overview
The .NYET bytecode format is a binary format designed for execution by the .NYET virtual machine (VM). It is a stack-based language that supports arithmetic operations, comparison operations, control flow, function definitions, memory operations, and basic I/O. This document specifies the bytecode structure, including the header, opcodes, value types, operand formats, and runtime behavior, enabling developers to generate or interpret .NYET bytecode in any programming language.

## Bytecode Structure
A .NYET bytecode file consists of:
1. **Header**: A fixed-size header that identifies the file as .NYET bytecode and specifies the version.
2. **Instructions**: A sequence of instructions, each starting with an opcode (1 byte) followed by optional operands.
3. **Function Definitions**: Functions defined with the `DEF` opcode, typically including a `main` function as the entry point.

### Bytecode Header
The header is a 5-byte sequence at the start of every .NYET bytecode file, ensuring compatibility and version identification.

| Field          | Size (Bytes) | Description                              | Value                     |
|----------------|--------------|------------------------------------------|---------------------------|
| Magic Number   | 4            | Identifies the file as .NYET bytecode    | `{'N', 'Y', 'E', 'T'}` (ASCII: `NYET`) |
| Version        | 1            | Bytecode format version                 | `0x01` (Version 1)        |

**Format**:
- Bytes 0–3: Magic number (`0x4E 0x59 0x45 0x54`, corresponding to ASCII `NYET`).
- Byte 4: Version (`0x01` for the current version).

**Validation**:
- A .NYET VM must verify that the first 4 bytes match the magic number `NYET`.
- The version byte must be checked for compatibility (currently, only `0x01` is supported).
- If the header is invalid, the VM should reject the bytecode.

### Instructions
Each instruction begins with a 1-byte opcode, followed by zero or more operands. The instruction pointer (IP) advances sequentially, with operands specifying additional data such as values, function names, or jump targets.

## Opcodes
Opcodes are defined as 1-byte values in the `DotNyet::Bytecode::Opcode` enum. The following table lists all supported opcodes, their hexadecimal values, descriptions, and operand formats.

| Opcode  | Hex    | Description                                                                 | Operands                                                                 |
|---------|--------|-----------------------------------------------------------------------------|--------------------------------------------------------------------------|
| `NOP`   | `0x00` | No operation; does nothing.                                                 | None                                                                     |
| `PUSH`  | `0x01` | Pushes a value onto the stack.                                              | `ValueTypeTag` (1 byte) + value data (variable length, see below)         |
| `POP`   | `0x02` | Pops a value from the stack and discards it.                                 | None                                                                     |
| `ADD`   | `0x03` | Pops two values, adds them, and pushes the result.                           | None                                                                     |
| `SUB`   | `0x04` | Pops two values, subtracts the second from the first, and pushes the result. | None                                                                     |
| **`CMP`** | **`0x05`** | Pops two values, compares them for equality, and pushes a boolean result. | None                                                                     |
| `DEF`   | `0x10` | Defines a function with a given name.                                        | Name length (uint32_t, 4 bytes) + function name (variable length)         |
| `CALL`  | `0x11` | Calls a function by name, pushing the return address to the call stack.      | Name length (uint32_t, 4 bytes) + function name (variable length)         |
| `RET`   | `0x12` | Returns from a function, popping the return address from the call stack.     | None                                                                     |
| `STORE` | `0x20` | Pops a value and stores it at a specified memory address.                    | Address (uint32_t, 4 bytes)                                              |
| `LOAD`  | `0x21` | Pushes the value stored at a specified memory address onto the stack.        | Address (uint32_t, 4 bytes)                                              |
| `JMP`   | `0x30` | Unconditionally jumps to a specified bytecode position.                      | Target address (uint32_t, 4 bytes)                                       |
| `JZ`    | `0x31` | Pops a value; jumps to the target if the value is not truthy.                | Target address (uint32_t, 4 bytes)                                       |
| `JNZ`   | `0x32` | Pops a value; jumps to the target if the value is truthy.                    | Target address (uint32_t, 4 bytes)                                       |
| `HALT`  | `0x40` | Stops execution of the program.                                             | None                                                                     |
| `PRINT` | `0x50` | Pops a value and prints it to the console.                                   | None                                                                     |
| **`INPUT`** | **`0x51`** | Reads a line of input from the console and pushes it as a string.         | None                                                                     |

### Operand Details
- **uint32_t**: A 4-byte unsigned integer, stored in little-endian byte order.
- **Function Name**: A string preceded by its length (uint32_t, 4 bytes), followed by the ASCII/UTF-8 encoded characters.
- **ValueTypeTag**: A 1-byte value indicating the type of data being pushed (see below).
- **Target Address**: A 4-byte unsigned integer (uint32_t) specifying a bytecode position (offset from the start of the bytecode, after the header).

## Value Types
The `PUSH` opcode is followed by a `ValueTypeTag` (1 byte) that specifies the type of value to push onto the stack, followed by the value's data. The `DotNyet::Bytecode::ValueTypeTag` enum defines the supported types:

| ValueTypeTag | Value | Description               | Data Format                           |
|--------------|-------|---------------------------|---------------------------------------|
| `Null`       | `0`   | Null value                | No additional data                    |
| `Integer`    | `1`   | 64-bit signed integer     | 8 bytes (int64_t, little-endian)      |
| `Double`     | `2`   | 64-bit floating-point     | 8 bytes (double, little-endian)       |
| `Boolean`    | `3`   | Boolean value             | 1 byte (0 = false, non-zero = true)   |
| `String`     | `4`   | String value              | Length (uint32_t, 4 bytes) + string data (ASCII/UTF-8) |

### Value Encoding Examples
- **Null**: `0x01 0x00` (opcode `PUSH` + `Null` tag).
- **Integer (42)**: `0x01 0x01 0x2A 0x00 0x00 0x00 0x00 0x00 0x00 0x00` (opcode `PUSH` + `Integer` tag + 8-byte value).
- **Double (3.14)**: `0x01 0x02 [8 bytes for 3.14 in IEEE 754 double format]` (opcode `PUSH` + `Double` tag + 8-byte value).
- **Boolean (true)**: `0x01 0x03 0x01` (opcode `PUSH` + `Boolean` tag + 1-byte true).
- **String ("hello")**: `0x01 0x04 0x05 0x00 0x00 0x00 0x68 0x65 0x6C 0x6C 0x6F` (opcode `PUSH` + `String` tag + 4-byte length + ASCII "hello").

## Runtime Behavior
The .NYET VM executes bytecode by processing instructions sequentially, maintaining a stack, call stack, and memory. Key runtime behaviors include:

- **Function Return (`RET`)**:
  - The `RET` instruction is used to return from a function call.
  - **Return Value Requirement**: A function must push a value onto the stack before executing `RET`. This value serves as the return value and is **not** popped by the `RET` instruction. It remains on the stack for the caller to access.
  - **Call Stack**: The `RET` instruction pops the return address from the call stack and sets the instruction pointer (IP) to that address, resuming execution at the instruction following the corresponding `CALL`.
  - **Error Handling**: If the call stack is empty when `RET` is executed, the VM throws an error (e.g., `Core::RuntimeException` in the reference implementation).
- **Execution Start**: Execution begins at the `main` function, which must be defined in the function table. The VM simulates a `CALL` to `main` by pushing a sentinel return address (typically the bytecode length) to the call stack.
- **Stack Operations**: Instructions like `PUSH`, `POP`, `ADD`, `SUB`, and `CMP` manipulate the stack, which holds values of type `Null`, `Integer`, `Double`, `Boolean`, or `String`.
- **Comparison (`CMP`)**:
  - Pops two values (`a` and `b`) from the stack.
  - Compares them for equality based on their types:
    - For `Integer`: Compares `int64_t` values.
    - For `Double`: Compares `double` values.
    - For `String`: Compares string contents.
  - Pushes a `Boolean` value (`true` if equal, `false` otherwise).
  - Throws a `Core::RuntimeException` if the types differ or if the types are not comparable (e.g., `Null` or `Boolean`).
- **Input (`INPUT`)**:
  - Reads a line of input from the console (using `std::getline` in the reference implementation).
  - Pushes the input as a `String` value onto the stack.
- **Control Flow**: Instructions like `JMP`, `JZ`, and `JNZ` modify the instruction pointer to implement jumps and conditional branching.
- **Memory Operations**: The `STORE` and `LOAD` instructions use a memory map with 32-bit unsigned integer addresses to store and retrieve values.

## Program Structure
A .NYET program typically follows this structure:
1. **Header**: The 5-byte header (`NYET` + version `0x01`).
2. **Function Definitions**: Zero or more `DEF` instructions, each defining a function name and its body. The function body consists of instructions until the next `DEF` or the end of the bytecode.
3. **Main Function**: A function named `main` is required as the entry point. The VM looks for `main` in the function table and begins execution at its bytecode position.
4. **Instructions**: The program logic, consisting of opcodes and operands, executed sequentially unless modified by control flow instructions (`JMP`, `JZ`, `JNZ`, `CALL`, `RET`).

### Example Bytecode
Below is an updated example bytecode sequence for a program that defines a `main` function, reads input from the console, compares it to a string ("hello"), and prints the result:

| Byte Offset | Value | Description |
|-------------|-------|-------------|
| 0–3         | `0x4E 0x59 0x45 0x54` | Magic number (`NYET`) |
| 4           | `0x01` | Version (1) |
| 5           | `0x10` | Opcode: `DEF` |
| 6–9         | `0x04 0x00 0x00 0x00` | Function name length (4) |
| 10–13       | `0x6D 0x61 0x69 0x6E` | Function name (`main`) |
| 14          | `0x51` | Opcode: `INPUT` |
| 15          | `0x01` | Opcode: `PUSH` |
| 16          | `0x04` | ValueTypeTag: `String` |
| 17–20       | `0x05 0x00 0x00 0x00` | String length (5) |
| 21–25       | `0x68 0x65 0x6C 0x6C 0x6F` | String value ("hello") |
| 26          | `0x05` | Opcode: `CMP` |
| 27          | `0x50` | Opcode: `PRINT` |
| 28          | `0x40` | Opcode: `HALT` |

This bytecode:
- Starts with the header (`NYET` + `0x01`).
- Defines a `main` function using `DEF`.
- Reads a line of input from the console and pushes it as a string.
- Pushes the string "hello" onto the stack.
- Compares the two strings for equality and pushes a boolean result.
- Prints the boolean result (`true` if the input was "hello", `false` otherwise).
- Halts execution.

## Implementing a .NYET Bytecode Parser
To parse or generate .NYET bytecode in another language, follow these steps:

### 1. Validate the Header
- Read the first 4 bytes and verify they match `{'N', 'Y', 'E', 'T'}` (ASCII `NYET`).
- Read the 5th byte and verify it is `0x01` (version 1).
- If either check fails, reject the bytecode as invalid.

### 2. Parse Instructions
- Start at byte offset 5 (after the header).
- Read each byte as an opcode and process its operands based on the opcode table.
- Advance the byte offset by the opcode size (1 byte) plus the size of its operands.

### 3. Handle Operands
- **uint32_t**: Read 4 bytes and interpret as a little-endian unsigned 32-bit integer.
- **int64_t**: Read 8 bytes and interpret as a little-endian signed 64-bit integer.
- **double**: Read 8 bytes and interpret as an IEEE 754 double-precision floating-point number.
- **String**: Read 4 bytes for the length (uint32_t), then read the specified number of bytes as a string.
- **Boolean**: Read 1 byte (0 = false, non-zero = true).

### 4. Build a Function Table
- Scan the bytecode for `DEF` instructions to build a mapping of function names to their starting bytecode positions (offset after the function name).
- The `main` function must exist for the program to be valid.
- **Change**: During function table construction, handle the new `CMP` and `INPUT` opcodes in the instruction scan loop:
  - `CMP`: No additional operands (advance by 1 byte).
  - `INPUT`: No additional operands (advance by 1 byte).

## Notes for Implementation
- **Endianness**: All multi-byte values (uint32_t, int64_t, double) are stored in little-endian format.
- **Error Handling**:
  - Validate bytecode bounds before reading operands to prevent out-of-bounds errors.
  - For `CMP`, ensure the two popped values have the same type and are comparable (Integer, Double, or String).
  - For `INPUT`, ensure the input operation (e.g., `std::getline`) is handled appropriately in the target environment.
- **Function Table**: Ensure the `main` function exists before execution.
- **Truthy Values**: For `JZ` and `JNZ`, a value is truthy if it is not `Null`, `0` (integer), `0.0` (double), `false` (boolean), or an empty string.

## Changes from Previous Specification
1. **New Opcodes**:
   - **`CMP` (0x05)**: Added to compare two values for equality, pushing a boolean result. Supports `Integer`, `Double`, and `String` types. Throws an error for mismatched types or unsupported types (e.g., `Null` or `Boolean`).
   - **`INPUT` (0x51)**: Added to read a line of input from the console and push it as a `String` value onto the stack.
2. **Updated Function Table Construction**:
   - The `LoadFunctionTable` function now recognizes `CMP` and `INPUT` opcodes during the bytecode scan.
   - Both opcodes require no additional operands, so the instruction pointer advances by 1 byte.
3. **Runtime Behavior**:
   - Added handling for `CMP` to compare values and push a boolean result.
   - Added handling for `INPUT` to read console input and push it as a string.
4. **Example Bytecode**:
   - Updated to demonstrate the use of `INPUT` and `CMP` in a program that reads input and compares it to a string.

This specification provides all necessary details to generate, parse, or execute .NYET bytecode with the updated opcodes and behavior.