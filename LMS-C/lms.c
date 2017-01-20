/*
 * Universidade Federal do Rio de Janeiro
 * Escola Politécnica
 * Projeto Final de Graduação
 * Ambiente de Teste para Filtros Adaptativos
 * Pedro Angelo Medeiros Fonini
 * Orientador: Markus Lima
 */

#include <stdlib.h>

#define N (64)
#define mu (0.5)

typedef struct LMS_data {
    float w[N];
    float x[N];
    float err;
    float *x_ptr;
    float *x_end;
} LMS_data;

void lms_reset(LMS_data *data)
{
    if (!data) return;

    data->err = 0;
    data->x_ptr = data->x;
    data->x_end = data->x + N;

    for (int i=0; i<N; ++i)
        data->x[i] = data->w[i] = 0;
}

void lms_push(LMS_data *data, float sample) {
    // unwind x_ptr
    if (data->x_ptr == data->x)
        data->x_ptr = data->x_end - 1;
    else
        --data->x_ptr;
    // write new sample
    *data->x_ptr = sample;
}

float lms_dot_product(LMS_data *data) {
    float result = 0;
    float *it_x, *it_w;
    for (it_x = data->x_ptr, it_w = data->w;
         it_x != data->x_end;
         ++it_x, ++it_w)
        result += *it_x * *it_w;
    for (it_x = data->x;
         it_x != data->x_ptr;
         ++it_x, ++it_w)
        result += *it_x * *it_w;
    return result;
}

void lms_update(LMS_data *data) {
    float *it_x, *it_w;
    for (it_x = data->x_ptr, it_w = data->w;
         it_x != data->x_end;
         ++it_x, ++it_w)
        *it_w += 2 * mu * data->err * *it_x;
    for (it_x = data->x;
         it_x != data->x_ptr;
         ++it_x, ++it_w)
        *it_w += 2 * mu * data->err * *it_x;
}

void *adapf_init(void)
{
    LMS_data *data = malloc(sizeof(LMS_data));
    if (!data) return NULL;

    lms_reset(data);

    return data;
}

void *adapf_restart(void *data)
{
    if (!data)
        return adapf_init();

    lms_reset(data);

    return data;
}

int adapf_close(void *data)
{
    if (!data)
        return 0;
    free((LMS_data *)data);
    return 1; // success
}

float adapf_run(void *data, float sample, float y)
{
    lms_push(data, sample);
    ((LMS_data *)data)->err = y - lms_dot_product(data);
    lms_update(data);
    return ((LMS_data *)data)->err;
}

void adapf_getw(void *data, float **begin, unsigned *n)
{
    *begin = ((LMS_data *)data)->w;
    *n = N;
}
