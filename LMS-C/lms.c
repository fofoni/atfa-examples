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

/* `N', the filter length, must be a *positive* integer */
#define N (512)
/* `mu', the step size, should be slightly less than 1/trR;
 * `trR' is N times the input power
 * "input power", also called `sigma_x', is the mean of the squared input;
 * Input power is normally less than 1e-2.
 */
#define mu (0.15)

/* algorithm memory */
struct AdapfData {
    float w[N]; /* vector of coefficients */
    float x[N]; /* vector of past input samples */
};
typedef struct AdapfData AdapfData;

static void lms_reset(AdapfData *data) {
    if (!data) return;
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

static float lms_dot_product(const AdapfData *data) {
    float result = 0;
    for (int i=0; i<N; ++i)
        result += data->x[i] * data->w[i];
    return result;
}

/* LMS update equation */
static void lms_update(AdapfData *data, float err) {
    for (int i=0; i<N; ++i)
        data->w[i] += 2 * mu * err * data->x[i];
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
float adapf_run(AdapfData *data, float sample, float y, int update,
                int *updated)
{
    lms_push(data, sample);
    float err = y - lms_dot_product(data);
    if (update)
        lms_update(data, err);
    *updated = update;
    return err;
}

/* provide a pointer to the vector of coefficients, for inspection */
void adapf_getw(const AdapfData *data, const float **begin, unsigned *n)
{
    *begin = data->w;
    *n = N;
}
