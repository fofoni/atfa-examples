#include <iostream>

extern "C" {

struct LMS_data {
    float w[2];
    float x[2];
};

void *adapf_init(void) {
    auto data = new LMS_data();
    if (!data) return nullptr;
    data->w[0] = 0;
    data->w[1] = 0;
    return data;
}

void *adapf_restart(void *data) {
    float *x = static_cast<LMS_data *>(data)->x;
    float *w = static_cast<LMS_data *>(data)->w;
    w[0] = 0;
    w[1] = 0;
    x[0] = 0;
    x[1] = 0;
    std::cout << "restart!" << std::endl;
    return data;
}

int adapf_close(void *data)
{
    std::cout << "close!" << std::endl;
    delete static_cast<LMS_data *>(data);
    std::cout << "deleted!" << std::endl;
    return 1; // success
}

float adapf_run(void *data, float x_novo, float y)
{
    float *x = static_cast<LMS_data *>(data)->x;
    float *w = static_cast<LMS_data *>(data)->w;
    // atualiza x
    x[1] = x[0];
    x[0] = x_novo;
    float err = y - (x[0]*w[0] + x[1]*w[1]);
    w[0] = w[0] + 2*3*err*x[0];
    w[1] = w[1] + 2*3*err*x[1];
    return err;
}

}
