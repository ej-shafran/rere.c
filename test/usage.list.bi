:i count 4
:b shell 13
./build/rere

:i exit_code 256
:b stdout 95
Usage: ./build/rere [record|replay] <files..>
ERROR: missing required argument [record|replay]

:b shell 19
./build/rere other

:i exit_code 256
:b stdout 80
Usage: ./build/rere [record|replay] <files..>
ERROR: invalid subcommand 'other'

:b shell 20
./build/rere record

:i exit_code 256
:b stdout 86
Usage: ./build/rere [record|replay] <files..>
ERROR: missing required argument <file>

:b shell 37
./build/rere record nonexistent.list

:i exit_code 256
:b stdout 116
--Recording commands from nonexistent.list--
ERROR: failed to open file nonexistent.list: No such file or directory

