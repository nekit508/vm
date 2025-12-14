fun main(int val) {
    asm {
        :start; dbg_brk;
        jmp end;
        :end;
    }
}