#ifndef INPUT_H
#define INPUT_H

#include "../utils/str/str.h"

String *read_input();
String *read_eof(int fd, size_t bufsize);

#endif
