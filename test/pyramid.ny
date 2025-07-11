fn main()
    var i
    var j
    var line
    var stars
    var spaces
    var rows

    i = 0
    rows = 0

    # Ask user how many rows to print
    push "How many rows? "
    print
    input
    toint
    pop rows

loop_i:
    push i
    push rows
    cmp
    jnz end

    # stars = i * 2 + 1
    push i
    push 2
    mul
    push 1
    add
    pop stars

    # spaces = rows - i - 1
    push rows
    push i
    sub
    push 1
    sub
    pop spaces

    # line = ""
    push ""
    pop line

    # Add spaces
    j = 0
loop_spaces:
    push j
    push spaces
    cmp
    jnz end_spaces

    push line
    push " "
    add
    pop line

    push j
    push 1
    add
    pop j
    jmp loop_spaces
end_spaces:

    # Add stars
    j = 0
loop_stars:
    push j
    push stars
    cmp
    jnz end_stars

    push line
    push "*"
    add
    pop line

    push j
    push 1
    add
    pop j
    jmp loop_stars
end_stars:
    # Append newline
    push line
    push "\n"
    add
    pop line

    # Print line
    push line
    print

    # i += 1
    push i
    push 1
    add
    pop i
    jmp loop_i

end:
    return 0
