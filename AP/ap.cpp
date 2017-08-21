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

    sample_t x[N+1];
    sample_t w[N];
    sample_t e[2];

    AdapfData()
    {
        reset();
    }

    void reset() {
        std::fill(x, x+N+1, 0);
        std::fill(w, w+N,   0);
        std::fill(e, e+2,   0);
    }

    void push(sample_t sample) {
        // shift to the right
        std::copy_backward(x, x+N, x+N+1);
        // push at top-left corner
        x[0] = sample;
    }

    void push_err(sample_t sample) {
        std::copy_backward(e, e+1, e+2);
        e[0] = sample;
    }

    sample_t dot_product() const {
        sample_t result = 0;
        for (int i=0; i<N; ++i)
            result += x[i]*w[i];
        return result;
    }

    void update() {

        sample_t s00 = delta;
        for (int i=0; i<N; ++i)
            s00 += x[i]*x[i];

        sample_t s01 = 0;
        for (int i=0; i<N; ++i)
            s01 += x[i]*x[i+1];

        sample_t s11 = delta;
        for (int i=0; i<N; ++i)
            s11 += x[i+1]*x[i+1];

        // X' * X + delta*I = [s00 s01
        //                     s01 s11]

        // 2x2 cholesky
        s00 = std::sqrt(s00);
        s01 = s01 / s00;
        s11 = s11 - s01*s01; // s11 squared

        // X' * X + delta*I = R' * R, R = [s00 s01
        //                                 0   sqrt(s11)]

        // double backsubsitution
        sample_t a0 = e[0] / s00;
        sample_t a1 = mu*(e[1] - s01*a0) / s11;
                 a0 = (mu*a0 - s01*a1) / s00;

        // mu * (X'*X)^(-1) * e = [a0 a1]'

        for (int i=0; i<N; ++i)
            w[i] += x[i]*a0 + x[i+1]*a1;

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
    return data->e[0];
}

void adapf_getw(const AdapfData *data, const float **begin, unsigned *n)
{
    *begin = data->w;
    *n = N;
}
}
