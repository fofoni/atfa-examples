/*
 * Universidade Federal do Rio de Janeiro
 * Escola Politécnica
 * Projeto Final de Graduação
 * Ambiente de Teste para Filtros Adaptativos
 * Pedro Angelo Medeiros Fonini
 * Orientador: Markus Lima
 */

#include <cmath>

#include <algorithm>
#include <limits>

#include <Eigen/Eigen>

#include <atfa_api.h>

using sample_t = float;

constexpr int N = 64;
constexpr int M = 4; /* M must be <= N */
constexpr sample_t mu = 0.9;
/* delta is quite high in order to avoid numeric trouble, since the
 * recursive computation of X^T*X is unstable (see AdapfData::push) */
constexpr sample_t delta = (N/3+1) *
std::sqrt(std::numeric_limits<sample_t>::epsilon());

struct AdapfData {

    sample_t x_array[N*M];
    sample_t xtx_array[M*M];
    sample_t w_array[N];
    sample_t e_array[M];

    Eigen::Map<Eigen::Matrix<sample_t, N, M>> X; // input matrix
    Eigen::Map<Eigen::Matrix<sample_t, N, 1>> x; // first column of X
    Eigen::Map<Eigen::Matrix<sample_t, 1, M>> x_first; // first row of X
    Eigen::Map<Eigen::Matrix<sample_t, 1, M>> x_last; // last row of X
    Eigen::Map<Eigen::Matrix<sample_t, M, M>> XtX; // delta*I + X^T * X
    Eigen::Map<Eigen::Matrix<sample_t, N, 1>> w;
    Eigen::Map<Eigen::Matrix<sample_t, M, 1>> err;

    AdapfData()
    : X(x_array), x(x_array), x_first(x_array /* X is "symmetric" */),
    x_last(x_array+(N-1)*M), XtX(xtx_array), w(w_array), err(e_array)
    {
        reset();
    }

    void reset() {
        std::fill(x_array, x_array+N*M, 0);
        std::fill(w_array, w_array+N, 0);
        std::fill(e_array, e_array+M, 0);
        XtX = delta * Eigen::Matrix<sample_t, M, M>::Identity();
    }

    void push(sample_t sample) {

        // first part of the update of the precomputed X^T * X matrix
        Eigen::Matrix<sample_t, M, M> XtX_update_first =
            -x_last.transpose()*x_last;

        // shift columns to the right
        std::copy_backward(x_array, x_array+N*(M-1), x_array+N*M);
        // shift first column downwards
        std::copy_backward(x_array, x_array+(N-1), x_array+N);
        // push at top-left corner
        x_array[0] = sample;

        // second part of the X^T * X update
        XtX += XtX_update_first + x_first.transpose()*x_first;

    }

    void push_err(sample_t sample) {
        std::copy_backward(e_array, e_array+(M-1), e_array+M);
        e_array[0] = sample;
    }

    sample_t dot_product() const {
        return x.dot(w);
    }

    void update() {
        w += mu * X * XtX.llt().solve(err); // cholesky
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

float adapf_run(AdapfData *data, float sample, float y, int update,
                int *updated)
{
    data->push(sample);
    data->push_err(y - data->dot_product());
    if (update)
        data->update();
    *updated = update;
    return data->e_array[0];
}

void adapf_getw(const AdapfData *data, const float **begin, unsigned *n)
{
    *begin = data->w_array;
    *n = N;
}
}
