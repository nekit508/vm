.const-pool
start = "Your input is ""
end = "".\n"

.code

:restart
load64c 8
load64c 0
write64s64s

:read
load64c $std_cin_char
calln64s
load64c 0
read6464s
write64s8s

load64c 0
read6464s
add64c64s 1
load64c 0
write64s64s

load64c 0
read6464s
sub64c64s 1
read864s

load8c 10
cmpeq8s8s
jmpf8s64c $:read

load64c 9
load64c 0
read6464s
cmpeq64s64s
jmpt8s64c $:end

load8c 0
load64c 0
read6464s
sub64c64s 1
write64s8s

load64c $start
load64c $std_cout_str
calln64s

load64c 8
mapmem64s
load64c $std_cout_str
calln64s

load64c $end
load64c $std_cout_str
calln64s

jmp64c $:restart
end
:end
end