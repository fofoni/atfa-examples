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


// // TODO DEBUG
// #include <iostream>
// // using namespace std;

using sample_t = float;

constexpr int N = 4;
constexpr sample_t lambda = 0.9;
constexpr sample_t delta = std::sqrt(std::numeric_limits<sample_t>::epsilon());


constexpr sample_t lambda_inv = 1/lambda;
// // TODO DEBUG
// void print_mat(sample_t *A, char nome) {
//     std::cout << nome << " = [" << '\n';
//     for (int i=0; i<N; ++i) {
//         for (int j=0; j<N; ++j)
//             std::cout << A[i+N*j] << "   ";
//         std::cout << '\n';
//     }
//     std::cout << "];\n\n";
// }
// void print_vec(sample_t *A, char nome) {
//     std::cout << nome << " = [" << '\n';
//     for (int i=0; i<N; ++i) {
//         std::cout << A[i];
//         std::cout << '\n';
//     }
//     std::cout << "];\n\n";
// }


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

    int count_samples;

    sample_t x[N];
    sample_t w[N
                    +N*N+N+1+1+1 //TODO:DEBUG
              ];
    sample_t S[N*N]; // big! N should be small
    sample_t err;

    AdapfData()
    {
        reset();
    }

    void reset() {
//         std::cout << "reset():\n";
        count_samples = 0;
        std::fill(x, x+N+1, 0);
        std::fill(w, w+N,   0);
        err = 0;
        for (int i=0; i<N; ++i)
        for (int j=0; j<N; ++j)
            S[i+j*N] = delta*(i==j);
//         print_vec(x,'x');
//         print_vec(w,'w');
//         std::cout << "err = " << err << ";\n\n";
//         print_mat(S,'S');
    }

    void push(sample_t sample) {
//         std::cout << "push( " << sample << " ):\n";
        // shift to the right
        std::copy_backward(x, x+N-1, x+N);
        // push at top-left corner
        x[0] = sample;
//         print_vec(x,'x');
    }

    sample_t dot_product() const {
//         std::cout << "dot_product():\n";
        sample_t result;
        dot<N,1,1>(x,w,&result);
//         std::cout << "result = " << result << ";\n\n";
        return result;
    }

    void update() {

        // :::TODO MASTER TODO MASTER TODO:::
        //>> % plotar evolução dos autovalores de S, da norma de S, do condicionamento de S, e da norma de S-S'
        //>> % plotar também, juntamente com linhas horizontais em y=0 e y=-0.9, o gráfico de psi'*x
        //>>
        // ( matlab ta aberto.... (: )


//         std::cout << "update():\n";

        // TODO: otimização a ser feita: usar só metade da memória para S,
        //          já que é simétrica.
        std::copy(S,    S+N*N,  w+N);
        std::copy(&err, &err+1, w+N+N*N+N+1);

        // psi = S * x
//         std::cout << "psi = S * x\n";
        sample_t psi[N];
        dot<N,N,1>(S,x,psi);
//         print_vec(psi,'p');
        std::copy(psi,  psi+N,  w+N+N*N);

        // phi = 1 / (lambda+psi'*x)
//         std::cout << "phi = 1 / (lambda+psi'*x)\n";
        sample_t phi;
        dot<N,1,1>(psi,x,&phi);
        phi = 1 / (lambda + phi);
//         std::cout << "phi = " << phi << ";\n\n";
        std::copy(&phi, &phi+1, w+N+N*N+N);

        // S = (1/lambda)*(S - psi*phi*psi')
//         std::cout << "S = (1/lambda)*(S - psi*phi*psi')\n";
        for (int i=0; i<N; ++i) {
            S[(N+1)*i] = lambda_inv * (S[(N+1)*i] - psi[i]*phi*psi[i]);
            // The loop is structure in this way because it forces S to stay
            // symmetric
            for (int j=i+1; j<N; ++j)
                S[i+N*j] = S[j+N*i] =
                    lambda_inv * (S[i+N*j] - psi[i]*phi*psi[j]);
        }
//         print_mat(S,'S');

        // make use of the (no more needed) space taken up by psi
        dot<N,N,1>(S,x,psi);
        for (int i=0; i<N; ++i)
            w[i] += psi[i]*err;
//         print_vec(w,'w');

        std::copy(&count_samples, &count_samples+1, w+N+N*N+N+1+1);
        ++count_samples;

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
//     std::cout << "err = " << data->err << ";\n\n";
    if (update)
        data->update();
    *updated = update;
    return data->err;
}

void adapf_getw(const AdapfData *data, const float **begin, unsigned *n)
{
    *begin = data->w;
    *n = N
                            +N*N+N+1+1+1; //TODO:DEBUG
}
}




/*
// TODO DEBUG
int main() {

    int updated;

    AdapfData data;

    adapf_run(&data,  0,  0, 1, &updated);
    adapf_run(&data,  1,  1, 1, &updated);
    adapf_run(&data,  4,  3, 1, &updated);
    adapf_run(&data,  9,  5, 1, &updated);
    adapf_run(&data, 16,  7, 1, &updated);
    adapf_run(&data, 25,  9, 1, &updated);
    adapf_run(&data, 36, 11, 1, &updated);

}*/
