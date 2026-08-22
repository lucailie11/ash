#include "shell_state.h"

static int last_exit_code;

void set_last_exit_code(int status) {
    last_exit_code = status;
}

int get_last_exit_code() {
    return last_exit_code;
}
