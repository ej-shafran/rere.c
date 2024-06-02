:i count 2
:b shell 20
echo 'hello, world'

:i exit_code 0
:b stdout 13
hello, world

:b shell 16
echo 'foo, bar'

:i exit_code 0
:b stdout 9
foo, bar

