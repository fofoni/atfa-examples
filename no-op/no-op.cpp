#include <iostream>

extern "C" {

void *adapf_init(void) {
    return nullptr;
}

void *adapf_restart(void *data) {
    return data;
}

int adapf_close(void *)
{
    return 1; // success
}

float adapf_run(void *, float, float y)
{
    return y;
}

}
