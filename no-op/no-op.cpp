/*
 * Universidade Federal do Rio de Janeiro
 * Escola Politécnica
 * Projeto Final de Graduação
 * Ambiente de Teste para Filtros Adaptativos
 * Pedro Angelo Medeiros Fonini
 * Orientador: Markus Lima
 */

#include <atfa_api.h>

extern "C" {

struct AdapfData {
    float placeholder;
};

AdapfData *adapf_init(void)
{
    return new AdapfData();
}

AdapfData *adapf_restart(AdapfData *data)
{
    return data;
}

int adapf_close(AdapfData *data)
{
    if (!data)
        return 0;
    delete data;
    return 1; // success
}

float adapf_run(AdapfData *, float, float y, int)
{
    return y;
}

void adapf_getw(AdapfData *data, float **begin, unsigned *n)
{
    *begin = &data->placeholder;
    *n = 0;
}

}
