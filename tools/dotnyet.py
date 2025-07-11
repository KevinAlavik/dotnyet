import sys
import struct
import re
from enum import Enum
from typing import List, Dict, Tuple, Union

class Opcode(Enum):
    NOP   = 0x00
    PUSH  = 0x01
    POP   = 0x02
    ADD   = 0x03
    SUB   = 0x04
    DEF   = 0x10
    CALL  = 0x11
    RET   = 0x12
    STORE = 0x20
    LOAD  = 0x21
    JMP   = 0x30
    JZ    = 0x31
    JNZ   = 0x32
    HALT  = 0x40
    PRINT = 0x50

class ValueTypeTag(Enum):
    Null    = 0
    Integer = 1
    Double  = 2
    Boolean = 3
    String  = 4

class TokenType(Enum):
    IDENTIFIER = 1
    NUMBER = 2
    STRING = 3
    KEYWORD = 4
    SYMBOL = 5
    LABEL = 6
    NEWLINE = 7

class Token:
    def __init__(self, type: TokenType, value: str, line: int):
        self.type = type
        self.value = value
        self.line = line

class ASTNode:
    pass

class ProgramNode(ASTNode):
    def __init__(self, statements: List[ASTNode]):
        self.statements = statements

class FunctionDefNode(ASTNode):
    def __init__(self, name: str, params: List[str], body: List[ASTNode], line: int):
        self.name = name
        self.params = params
        self.body = body
        self.line = line

class VarDeclNode(ASTNode):
    def __init__(self, name: str, line: int):
        self.name = name
        self.line = line

class AssignNode(ASTNode):
    def __init__(self, name: str, value: ASTNode, line: int):
        self.name = name
        self.value = value
        self.line = line

class PushNode(ASTNode):
    def __init__(self, value: Union[str, int, float, bool], line: int):
        self.value = value
        self.line = line

class CallNode(ASTNode):
    def __init__(self, name: str, args: List[ASTNode], line: int):
        self.name = name
        self.args = args
        self.line = line

class JumpNode(ASTNode):
    def __init__(self, opcode: Opcode, label: str, line: int):
        self.opcode = opcode
        self.label = label
        self.line = line

class SimpleInstructionNode(ASTNode):
    def __init__(self, opcode: Opcode, line: int):
        self.opcode = opcode
        self.line = line

class LabelNode(ASTNode):
    def __init__(self, name: str, line: int):
        self.name = name
        self.line = line

class ReturnNode(ASTNode):
    def __init__(self, value: Union[str, int, float, bool]):
        self.value = value

class Lexer:
    def __init__(self, source: str):
        self.source = source
        self.tokens: List[Token] = []
        self.pos = 0
        self.line = 1

    def tokenize(self) -> List[Token]:
        while self.pos < len(self.source):
            char = self.source[self.pos]
            
            if char.isspace():
                if char == '\n':
                    self.tokens.append(Token(TokenType.NEWLINE, char, self.line))
                    self.line += 1
                self.pos += 1
                continue
                
            if char == '#':
                while self.pos < len(self.source) and self.source[self.pos] != '\n':
                    self.pos += 1
                continue

            if char.isalpha() or char == '_':
                identifier = self.consume_identifier()
                if identifier in {'fn', 'var', 'push', 'print', 'pop', 'add', 'sub', 'return', 'jmp', 'jz', 'jnz'}:
                    self.tokens.append(Token(TokenType.KEYWORD, identifier, self.line))
                else:
                    self.tokens.append(Token(TokenType.IDENTIFIER, identifier, self.line))
                continue

            if char.isdigit() or char == '-':
                number = self.consume_number()
                self.tokens.append(Token(TokenType.NUMBER, number, self.line))
                continue

            if char == '"':
                string = self.consume_string()
                self.tokens.append(Token(TokenType.STRING, string, self.line))
                continue

            if char in '(),=:':
                self.tokens.append(Token(TokenType.SYMBOL, char, self.line))
                self.pos += 1
                continue

            raise ValueError(f"Invalid character at line {self.line}: {char}")

        return self.tokens

    def consume_identifier(self) -> str:
        start = self.pos
        while self.pos < len(self.source) and (self.source[self.pos].isalnum() or self.source[self.pos] == '_'):
            self.pos += 1
        return self.source[start:self.pos]

    def consume_number(self) -> str:
        start = self.pos
        while self.pos < len(self.source) and (self.source[self.pos].isdigit() or self.source[self.pos] in '-.'):
            self.pos += 1
        return self.source[start:self.pos]

    def consume_string(self) -> str:
        self.pos += 1  # Skip opening quote
        start = self.pos
        while self.pos < len(self.source) and self.source[self.pos] != '"':
            self.pos += 1
        result = self.source[start:self.pos]
        self.pos += 1  # Skip closing quote
        return result

class Parser:
    def __init__(self, tokens: List[Token]):
        self.tokens = tokens
        self.pos = 0

    def parse(self) -> ProgramNode:
        statements = []
        while self.pos < len(self.tokens):
            if self.current_token().type == TokenType.NEWLINE:
                self.pos += 1
                continue
            stmt = self.parse_statement()
            if stmt:
                statements.append(stmt)
        return ProgramNode(statements)

    def current_token(self) -> Token:
        return self.tokens[self.pos] if self.pos < len(self.tokens) else Token(TokenType.NEWLINE, '', -1)

    def consume(self, token_type: TokenType) -> Token:
        token = self.current_token()
        if token.type != token_type:
            raise ValueError(f"Expected {token_type} at line {token.line}, got {token.value}")
        self.pos += 1
        return token

    def parse_statement(self) -> ASTNode:
        token = self.current_token()
        line = token.line

        if token.type == TokenType.KEYWORD:
            if token.value == 'fn':
                return self.parse_function_def()
            elif token.value == 'var':
                return self.parse_var_decl()
            elif token.value == 'push':
                self.pos += 1
                value = self.parse_value()
                return PushNode(value, line)
            elif token.value == 'return':
                self.pos += 1
                value = self.parse_value()
                return ReturnNode(value)
            elif token.value in {'print', 'pop', 'add', 'sub'}:
                self.pos += 1
                opcode = {'print': Opcode.PRINT, 'pop': Opcode.POP, 'add': Opcode.ADD, 'sub': Opcode.SUB}[token.value]
                return SimpleInstructionNode(opcode, line)
            elif token.value in {'jmp', 'jz', 'jnz'}:
                self.pos += 1
                label = self.consume(TokenType.IDENTIFIER).value
                opcode = {'jmp': Opcode.JMP, 'jz': Opcode.JZ, 'jnz': Opcode.JNZ}[token.value]
                return JumpNode(opcode, label, line)

        elif token.type == TokenType.IDENTIFIER:
            next_token = self.tokens[self.pos + 1] if self.pos + 1 < len(self.tokens) else None
            if next_token and next_token.value == ':':
                label = token.value
                self.pos += 2  # Skip identifier and colon
                return LabelNode(label, line)
            elif next_token and next_token.value == '(':
                return self.parse_function_call()
            elif next_token and next_token.value == '=':
                return self.parse_assignment()

        raise ValueError(f"Invalid statement at line {token.line}: {token.value}")

    def parse_function_def(self) -> FunctionDefNode:
        self.pos += 1  # Skip 'fn'
        name = self.consume(TokenType.IDENTIFIER).value
        self.consume(TokenType.SYMBOL).value  # '('
        params = []
        if self.current_token().value != ')':
            params.append(self.consume(TokenType.IDENTIFIER).value)
            while self.current_token().value == ',':
                self.pos += 1
                params.append(self.consume(TokenType.IDENTIFIER).value)
        self.consume(TokenType.SYMBOL).value  # ')'
        self.consume(TokenType.NEWLINE)
        body = []
        while self.current_token().type != TokenType.KEYWORD or self.current_token().value != 'return':
            if self.current_token().type == TokenType.NEWLINE:
                self.pos += 1
                continue
            body.append(self.parse_statement())
        body.append(self.parse_statement())
        return FunctionDefNode(name, params, body, self.current_token().line)

    def parse_var_decl(self) -> VarDeclNode:
        self.pos += 1  # Skip 'var'
        name = self.consume(TokenType.IDENTIFIER).value
        return VarDeclNode(name, self.current_token().line)

    def parse_assignment(self) -> AssignNode:
        name = self.consume(TokenType.IDENTIFIER).value
        self.consume(TokenType.SYMBOL)  # '='
        value = self.parse_value()
        return AssignNode(name, value, self.current_token().line)

    def parse_function_call(self) -> CallNode:
        name = self.consume(TokenType.IDENTIFIER).value
        self.consume(TokenType.SYMBOL)  # '('
        args = []
        if self.current_token().value != ')':
            args.append(self.parse_value())
            while self.current_token().value == ',':
                self.pos += 1
                args.append(self.parse_value())
        self.consume(TokenType.SYMBOL)  # ')'
        return CallNode(name, args, self.current_token().line)

    def parse_value(self) -> Union[str, int, float, bool]:
        token = self.current_token()
        self.pos += 1
        if token.type == TokenType.IDENTIFIER:
            return token.value
        elif token.type == TokenType.NUMBER:
            if '.' in token.value:
                return float(token.value)
            return int(token.value)
        elif token.type == TokenType.STRING:
            return token.value
        elif token.type == TokenType.KEYWORD and token.value in ('true', 'false'):
            return token.value == 'true'
        elif token.type == TokenType.KEYWORD and token.value == 'null':
            return None
        raise ValueError(f"Invalid value at line {token.line}: {token.value}")

class Compiler:
    def __init__(self):
        self.bytecode: List[int] = []
        self.functions: Dict[str, int] = {}
        self.labels: Dict[str, int] = {}
        self.jump_targets: List[Tuple[int, str]] = []
        self.current_params: List[str] = []
        self.param_stack: List[List[str]] = []
        self.local_vars: Dict[str, int] = {}
        self.local_var_count: int = 0
        self.scope_stack: List[Dict[str, int]] = [{}]

    def emit_byte(self, byte: int):
        self.bytecode.append(byte)

    def emit_uint32(self, value: int):
        self.bytecode.extend(struct.pack('<I', value))

    def emit_int64(self, value: int):
        self.bytecode.extend(struct.pack('<q', value))

    def emit_double(self, value: float):
        self.bytecode.extend(struct.pack('<d', value))

    def emit_string(self, value: str):
        self.emit_uint32(len(value))
        self.bytecode.extend(value.encode('utf-8'))

    def emit_value(self, value: Union[str, int, float, bool, None], line: int):
        if isinstance(value, str) and (value in self.current_params or value in self.local_vars):
            self.emit_byte(Opcode.LOAD.value)
            if value in self.current_params:
                index = self.current_params.index(value)
            else:
                index = self.local_vars[value]
            self.emit_uint32(index)
        else:
            self.emit_byte(Opcode.PUSH.value)
            if value is None:
                self.emit_byte(ValueTypeTag.Null.value)
            elif isinstance(value, bool):
                self.emit_byte(ValueTypeTag.Boolean.value)
                self.emit_byte(1 if value else 0)
            elif isinstance(value, int):
                self.emit_byte(ValueTypeTag.Integer.value)
                self.emit_int64(value)
            elif isinstance(value, float):
                self.emit_byte(ValueTypeTag.Double.value)
                self.emit_double(value)
            elif isinstance(value, str):
                self.emit_byte(ValueTypeTag.String.value)
                self.emit_string(value)
            else:
                raise ValueError(f"Invalid value at line {line}: {value}")

    def compile(self, source: str) -> bytes:
        lexer = Lexer(source)
        tokens = lexer.tokenize()
        parser = Parser(tokens)
        ast = parser.parse()

        for stmt in ast.statements:
            self.compile_statement(stmt)

        for pos, label in self.jump_targets:
            if label not in self.labels:
                raise ValueError(f"Undefined label: {label}")
            target = self.labels[label]
            self.bytecode[pos:pos+4] = struct.pack('<I', target)

        self.emit_byte(Opcode.HALT.value)

        result = bytearray(b'NYET')
        result.append(0x01)
        result.extend(self.bytecode)
        return result

    def compile_statement(self, stmt: ASTNode):
        if isinstance(stmt, FunctionDefNode):
            self.functions[stmt.name] = len(self.bytecode)
            self.param_stack.append(self.current_params)
            self.current_params = stmt.params
            self.scope_stack.append({})
            self.local_vars = self.scope_stack[-1]
            self.local_var_count = len(stmt.params)
            self.emit_byte(Opcode.DEF.value)
            self.emit_string(stmt.name)
            for s in stmt.body:
                self.compile_statement(s)
            self.current_params = self.param_stack.pop() if self.param_stack else []
            self.scope_stack.pop()
            self.local_vars = self.scope_stack[-1] if self.scope_stack else {}
            self.local_var_count = len(self.current_params)

        elif isinstance(stmt, VarDeclNode):
            if stmt.name in self.local_vars:
                raise ValueError(f"Variable {stmt.name} already defined at line {stmt.line}")
            self.local_vars[stmt.name] = self.local_var_count
            self.local_var_count += 1

        elif isinstance(stmt, AssignNode):
            self.compile_value(stmt.value, stmt.line)
            if stmt.name not in self.local_vars and stmt.name not in self.current_params:
                raise ValueError(f"Undefined variable at line {stmt.line}: {stmt.name}")
            self.emit_byte(Opcode.STORE.value)
            if stmt.name in self.current_params:
                index = self.current_params.index(stmt.name)
            else:
                index = self.local_vars[stmt.name]
            self.emit_uint32(index)

        elif isinstance(stmt, PushNode):
            self.emit_value(stmt.value, stmt.line)

        elif isinstance(stmt, CallNode):
            for arg in reversed(stmt.args):
                self.compile_value(arg, stmt.line)
            self.emit_byte(Opcode.CALL.value)
            self.emit_string(stmt.name)

        elif isinstance(stmt, JumpNode):
            self.emit_byte(stmt.opcode.value)
            self.jump_targets.append((len(self.bytecode), stmt.label))
            self.emit_uint32(0)

        elif isinstance(stmt, SimpleInstructionNode):
            self.emit_byte(stmt.opcode.value)

        elif isinstance(stmt, LabelNode):
            self.labels[stmt.name] = len(self.bytecode)

        elif isinstance(stmt, ReturnNode):
            self.compile_value(stmt.value, 0)  # Line number not used for value compilation
            self.emit_byte(Opcode.RET.value)

    def compile_value(self, value: Union[ASTNode, str, int, float, bool], line: int):
        if isinstance(value, ASTNode):
            self.compile_statement(value)
        else:
            self.emit_value(value, line)

def main():
    if len(sys.argv) != 3:
        print("Usage: python dotnyet.py <input.nyasm> <output.bin>")
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