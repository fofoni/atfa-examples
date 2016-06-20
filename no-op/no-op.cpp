/*
 * Universidade Federal do Rio de Janeiro
 * Escola Politécnica
 * Projeto Final de Graduação
 * Ambiente de Teste para Filtros Adaptativos
 * Pedro Angelo Medeiros Fonini
 * Orientador: Markus Lima
 */

extern "C" {

struct dummy {};

void *adapf_init(void)
{
    return new dummy();
}

void *adapf_restart(void *data)
{
    return data;
}

int adapf_close(void *data)
{
    delete static_cast<dummy *>(data);
    return 1; // success
}

float adapf_run(void *, float, float y)
{
    return y;
}

}
