fn main()
    var counter
    counter = 0
loop_start:
    push counter
    push 10
    cmp
    jnz loop_end

    push "Loop "
    push counter
    add
    push "\n"
    add
    print

    push counter
    push 1
    add
    pop counter

    jmp loop_start
loop_end:
    return null
