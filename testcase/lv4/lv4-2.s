decl @getint(): i32
decl @getch(): i32
decl @getarray(*i32): i32
decl @putint(i32)
decl @putch(i32)
decl @putarray(i32, *i32)
decl @starttime()
decl @stoptime()

fun @main(): i32 {
%entry:
    @x_1 = alloc i32
    store 10, @x_1
    %0 = load @x_1
    %1 = add %0, 1
    store %1, @x_1
    %2 = load @x_1
    ret %2
}


