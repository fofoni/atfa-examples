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

constexpr int N = 30;
constexpr sample_t lambda = 0.9;
constexpr sample_t delta = std::sqrt(std::numeric_limits<sample_t>::epsilon());

constexpr sample_t lambda_inv = 1/lambda;

namespace {
// A is PxO, B is OxQ, R=A*B
// In other words, O is the order of the product, and PxQ is the result size
template <int O, int P, int Q, bool init_zero=true>
void dot(const sample_t *A, const sample_t *B, sample_t *R) {
    if (init_zero)
        std::fill(R, R+P*Q, 0);
    for (int i=0; i<P; ++i)
    for (int j=0; j<Q; ++j)
    for (int k=0; k<O; ++k)
        R[i+P*j] += A[i+P*k] * B[k+O*j];
}
}

struct AdapfData {

    sample_t x[N];
    sample_t w[N+N*N+1];
    sample_t S[N*N]; // big! N should be small
    sample_t err;

    AdapfData()
    {
        reset();
    }

    void reset() {
        std::fill(x, x+N, 0);
        std::fill(w, w+N, 0);
        err = 0;
        for (int i=0; i<N; ++i)
        for (int j=0; j<N; ++j)
            S[i+j*N] = delta*(i==j);
    }

    void push(sample_t sample) {
        // shift to the right
        std::copy_backward(x, x+N-1, x+N);
        // push at top-left corner
        x[0] = sample;
    }

    sample_t dot_product() const {
        sample_t result;
        dot<N,1,1>(x,w,&result);
        return result;
    }

    void update() {

        // TODO: otimização a ser feita: usar só metade da memória para S,
        //          já que é simétrica.

//         std::copy(S,    S+N*N,  w+N);
//         std::copy(&err, &err+1, w+N+N*N);

        // psi = S * x
        sample_t psi[N];
        dot<N,N,1>(S,x,psi);

        // phi = 1 / (lambda+psi'*x)
        sample_t phi;
        dot<N,1,1>(psi,x,&phi);
        phi = 1 / (lambda + phi);

        // S = (1/lambda)*(S - psi*phi*psi')
        for (int i=0; i<N; ++i) {
            S[(N+1)*i] = lambda_inv * (S[(N+1)*i] - psi[i]*phi*psi[i]);
            // The loop is structured in this way because it forces S to stay
            // symmetric
            for (int j=i+1; j<N; ++j)
                S[i+N*j] = S[j+N*i] =
                    lambda_inv * (S[i+N*j] - psi[i]*phi*psi[j]);
        }

        // make use of the (no more needed) space taken up by psi
        dot<N,N,1>(S,x,psi);
        for (int i=0; i<N; ++i)
            w[i] += psi[i]*err;

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
    data->err = y - data->dot_product();
    if (update)
        data->update();
    *updated = update;
    return data->err;
}

void adapf_getw(const AdapfData *data, const float **begin, unsigned *n)
{
    *begin = data->w;
//     *n = N+N*N+1;
    *n = N;  // ????
}
}
