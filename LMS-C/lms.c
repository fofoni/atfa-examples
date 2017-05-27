/*
 * Universidade Federal do Rio de Janeiro
 * Escola Politécnica
 * Projeto Final de Graduação
 * Ambiente de Teste para Filtros Adaptativos
 * Pedro Angelo Medeiros Fonini
 * Orientador: Markus Lima
 */

// TODO: Põe mais comentários
// TODO: readability >> performance; mudar o apêndice B

#include <stdlib.h>
#include <atfa_api.h>

#define N (64) /* deve ser inteiro positivo */
#define mu (0.5)

struct AdapfData {
    float w[N];
    float x[N];
    float err;
    float *x_ptr;
    float *x_end;
};
typedef struct AdapfData AdapfData;

static void lms_reset(AdapfData *data) {
    if (!data) return;

    data->err = 0;
    data->x_ptr = data->x;
    data->x_end = data->x + N;

    for (int i=0; i<N; ++i)
        data->x[i] = data->w[i] = 0;
}

static void lms_push(AdapfData *data, float sample) {
    /* unwind x_ptr */
    if (data->x_ptr == data->x)
        data->x_ptr = data->x_end - 1;
    else
        --data->x_ptr;
    /* write new sample */
    *data->x_ptr = sample;
}

static float lms_dot_product(AdapfData *data) {
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

static void lms_update(AdapfData *data) {
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

AdapfData *adapf_init(void)
{
    AdapfData *data = malloc(sizeof(AdapfData));
    lms_reset(data);
    return data;
}

AdapfData *adapf_restart(AdapfData *data)
{
    if (!data)
        return adapf_init();

    lms_reset(data);
    return data;
}

int adapf_close(AdapfData *data)
{
    if (!data)
        return 0;
    free(data);
    return 1; /* success */
}

float adapf_run(AdapfData *data, float sample, float y, int update)
{
    lms_push(data, sample);
    data->err = y - lms_dot_product(data);
    if (update)
        lms_update(data);
    return data->err;
}

void adapf_getw(AdapfData *data, float **begin, unsigned *n)
{
    *begin = data->w;
    *n = N;
}
