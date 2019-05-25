#How to debug on linux

##GDB
1. how to use gdb

1.1 add debug message
~~~
g++ -g hello.cpp -o hello
~~~

if you want to use gdb, please add **-g**, otherwise　you can't use gdb 

1.2 start gdb debug
~~~
gdb <program>
~~~

2 Gdb interactive command

- Run: abbreviated as r, its role is to run the program, when the breakpoint is encountered, the program will stop running at the breakpoint, waiting for the user to enter the next command.
- Continue (short) c: continue execution to the next breakpoint (or end of run)
- Next: (shorthand n), single-step tracking program, when encountering a function call, does not enter this function body; the main difference between this command and step is that step encounters a user-defined function, it will step into the function To run, and next to call the function directly, will not enter the body of the function.
- Step (shorthand s): Stepping through the function if there is a function call; unlike the command n, n is the function that does not enter the call.
- Until: When you are tired of stepping through a loop, this command can run the program until it exits the loop body.
- Until+ line number: Run to a line, not just to jump out of the loop
- Finish: Run the program until the current function completes returning, and print the stack address and return value and parameter values ​​when the function returns.
Call function (parameter): Call the function visible in the program and pass the "parameter", such as: call gdb_test (55)
- Quit: abbreviated as q , exit gdb
