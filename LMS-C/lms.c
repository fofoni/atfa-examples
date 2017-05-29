/*
 * Universidade Federal do Rio de Janeiro
 * Escola Politécnica
 * Projeto Final de Graduação
 * Ambiente de Teste para Filtros Adaptativos
 * Pedro Angelo Medeiros Fonini
 * Orientador: Markus Lima
 */

#include <stdlib.h>
#include <atfa_api.h>

#define N (64) /* must be a positive integer */
#define mu (0.5)

/* algorithm memory */
struct AdapfData {
    float w[N]; /* vector of coefficients */
    float x[N]; /* vector of past input samples */
    float err; /* error sample */
};
typedef struct AdapfData AdapfData;

static void lms_reset(AdapfData *data) {
    if (!data) return;
    data->err = 0;
    for (int i=0; i<N; ++i)
        data->x[i] = data->w[i] = 0;
}

/* push a new sample to the vector data->x of input samples */
static void lms_push(AdapfData *data, float sample) {
    /* shift to the right */
    for (int i=N-1; i>=1; --i)
        data->x[i] = data->x[i-1];
    /* push new sample at the left end */
    data->x[0] = sample;
}

static float lms_dot_product(AdapfData *data) {
    float result = 0;
    for (int i=0; i<N; ++i)
        result += data->x[i] * data->w[i];
    return result;
}

/* LMS update equation */
static void lms_update(AdapfData *data) {
    for (int i=0; i<N; ++i)
        data->w[i] += 2 * mu * data->err * data->x[i];
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
        return 0; /* failure */
    free(data);
    return 1; /* success */
}

/* given new input and reference samples, return the error sample */
float adapf_run(AdapfData *data, float sample, float y, int update)
{
    lms_push(data, sample);
    data->err = y - lms_dot_product(data);
    if (update)
        lms_update(data);
    return data->err;
}

/* provide a pointer to the vector of coefficients, for inspection */
void adapf_getw(AdapfData *data, float **begin, unsigned *n)
{
    *begin = data->w;
    *n = N;
}
