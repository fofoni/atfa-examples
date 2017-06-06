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
constexpr sample_t mu = 0.9;
constexpr sample_t delta = std::sqrt(std::numeric_limits<sample_t>::epsilon());

struct AdapfData {

    sample_t x_array[N];
    Eigen::Map<Eigen::Matrix<sample_t, N, 1>> x;

    sample_t w_array[N];
    Eigen::Map<Eigen::Matrix<sample_t, N, 1>> w;

    AdapfData()
        : x(x_array), w(w_array)
    {
        reset();
    }

    void reset() {
        std::fill(x_array, x_array+N, 0);
        std::fill(w_array, w_array+N, 0);
    }

    void push(sample_t sample) {
        std::copy_backward(x_array, x_array+(N-1), x_array+N); // shift right
        x_array[0] = sample; // push at left end
    }

    sample_t dot_product() const {
        return x.dot(w);
    }

    void update(sample_t err) {
        w += mu * err * x / (delta + x.squaredNorm());
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
    if (update)
        data->update(err);
    *updated = update;
    return err;
}

void adapf_getw(const AdapfData *data, const float **begin, unsigned *n)
{
    *begin = data->w_array;
    *n = N;
}
}
