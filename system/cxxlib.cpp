
static void reset_board() {

}

extern "C" void __cxa_pure_virtual(void) {
    // We might want to write some diagnostics to uart in this case
    reset_board();
}

extern "C" void __cxa_deleted_virtual(void) {
    // We might want to write some diagnostics to uart in this case
    reset_board();
}