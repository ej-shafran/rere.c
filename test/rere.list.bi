:i count 2
:b shell 30
./build/rere record test.list

:i exit_code 0
:b stdout 104
--Recording commands from test.list--
Recording `echo 'hello, world'`...
Recording `echo 'foo, bar'`...

:b shell 30
./build/rere replay test.list

:i exit_code 0
:b stdout 104
--Replaying commands from test.list--
Replaying `echo 'hello, world'`...
Replaying `echo 'foo, bar'`...

