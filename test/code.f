fun main(int val) {
    asm {
        dbg_brk;
        jmp 0;
    }
}