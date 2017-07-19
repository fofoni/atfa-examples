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

#include <atfa_api.h>

using sample_t = float;

constexpr int N = 64;
constexpr sample_t mu = 0.9;
constexpr sample_t delta = std::sqrt(std::numeric_limits<sample_t>::epsilon());

struct AdapfData {

    sample_t x[N];
    sample_t w[N];

    AdapfData()
    {
        reset();
    }

    void reset() {
        std::fill(x, x+N, 0);
        std::fill(w, w+N, 0);
    }

    void push(sample_t sample) {
        std::copy_backward(x, x+(N-1), x+N); // shift down
        x[0] = sample; // push at top
    }

    sample_t dot_product() const {
        sample_t result = 0;
        for (int i=0; i<N; ++i)
            result += x[i]*w[i];
        return result;
    }

    void update(sample_t err) {
        sample_t x_normsquared = delta;
        for (int i=0; i<N; ++i)
            x_normsquared += x[i]*x[i];
        sample_t factor = mu*err/x_normsquared;
        for (int i=0; i<N; ++i)
            w[i] += factor * x[i];
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
    *begin = data->w;
    *n = N;
}
}
