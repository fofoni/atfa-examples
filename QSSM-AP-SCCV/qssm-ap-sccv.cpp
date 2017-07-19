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

constexpr int N = 8000;
constexpr int M = 4; /* M must be <= N */
constexpr sample_t mu = 0.9;
/* delta is quite high in order to avoid numeric trouble, since the
 * recursive computation of X^T*X is unstable (see AdapfData::push) */
constexpr sample_t delta = 10 * N *
                           std::sqrt(std::numeric_limits<sample_t>::epsilon());
constexpr sample_t beta = 5.0; // GMF-mod norm-0 fidelity
constexpr sample_t alpha = 0.0025; // penalty gain
constexpr sample_t gamma_bar = 1e-3; // sigma^2 = -100dB

struct AdapfData {

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

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

    // returns whether the algorithm should update
    bool push_err(sample_t sample) {
        e_array[0] = sample - std::copysign(gamma_bar, sample);
        return (sample > 0) == (e_array[0] > 0);
    }

    sample_t dot_product() const {
        return x.dot(w);
    }

    void update() {
        w += mu * X * XtX.llt().solve(err) // cholesky
        - alpha * (2 * beta*beta * w.array() /
        (beta*beta * w.array().square() + 1).square()).matrix(); // GMF-mod
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
    sample_t err = y - data->dot_product();
    bool should_update = data->push_err(err); (void)should_update;
    if ((update==2) || (update && should_update)) {
        data->update();
        *updated = 1;
    }
    else
        *updated = 0;
    return err;
}

void adapf_getw(const AdapfData *data, const float **begin, unsigned *n)
{
    *begin = data->w_array;
    *n = N;
}
}
