/*
 * Universidade Federal do Rio de Janeiro
 * Escola Politécnica
 * Projeto Final de Graduação
 * Ambiente de Teste para Filtros Adaptativos
 * Pedro Angelo Medeiros Fonini
 * Orientador: Markus Lima
 */

#include <atfa_api.h>

constexpr unsigned N = 64;
constexpr float mu = 0.5;

struct AdapfData {
    float w[N];
    float x[N];
    float err;
    float *x_ptr;
    float * const x_end;
    AdapfData() :
        err(0), x_ptr(x), x_end(x+N)
    {
        for (float& sample : x)
            sample = 0;
        for (float& weight : w)
            weight = 0;
    }
    void reset(void) {
        for (float& sample : x)
            sample = 0;
        for (float& weight : w)
            weight = 0;
        x_ptr = x;
    }
    void push(float sample) {
        // unwind x_ptr
        if (x_ptr == x)
            x_ptr = x_end - 1;
        else
            --x_ptr;
        // write new sample
        *x_ptr = sample;
    }
    float dot_product() {
        float result = 0;
        float *it_x, *it_w;
        for (it_x = x_ptr, it_w = w; it_x != x_end; ++it_x, ++it_w)
            result += *it_x * *it_w;
        for (it_x = x; it_x != x_ptr; ++it_x, ++it_w)
            result += *it_x * *it_w;
        return result;
    }
    void update() {
        float *it_x, *it_w;
        for (it_x = x_ptr, it_w = w; it_x != x_end; ++it_x, ++it_w)
            *it_w += 2 * mu * err * *it_x;
        for (it_x = x; it_x != x_ptr; ++it_x, ++it_w)
            *it_w += 2 * mu * err * *it_x;
    }
};

extern "C" {
AdapfData *adapf_init(void)
{
    return new AdapfData();
}

AdapfData *adapf_restart(AdapfData *data)
{
    if (!data)
        return adapf_init();
    data->reset();
    return data;
}

int adapf_close(AdapfData *data)
{
    if (!data)
        return 0;
    delete data;
    return 1; // success
}

float adapf_run(AdapfData *data, float sample, float y, int update)
{
    data->push(sample);
    data->err = y - data->dot_product();
    if (update)
        data->update();
    return data->err;
}

void adapf_getw(AdapfData *data, float **begin, unsigned *n)
{
    *begin = data->w;
    *n = N;
}
}
