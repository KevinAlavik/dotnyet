fn main()
    # Variables
    var name

    # Ask user for their name
    push "Enter your name: "
    print
    input

    # Populate 'name' variable with users input
    pop name

    # Greet them
    push "Hello "
    push name
    add
    push "\n"
    add
    print

    # Return
    return 0
