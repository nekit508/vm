fun main(int val) {
    asm {
        dbg_brk;
    }
}

fun ill() {
    asm {
        ill;
    }
}