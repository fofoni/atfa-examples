/*
 * Universidade Federal do Rio de Janeiro
 * Escola Politécnica
 * Projeto Final de Graduação
 * Ambiente de Teste para Filtros Adaptativos
 * Pedro Angelo Medeiros Fonini
 * Orientador: Markus Lima
 */

extern "C" {

constexpr unsigned N = 10;
constexpr float mu = 1;

struct LMS_data {
    float w[N];
    float x[N];
    float err;
    float *x_ptr;
    float * const x_end;
    LMS_data() :
        err(0), x_ptr(x), x_end(x+N)
    {
        for (float& sample : x)
            sample = 0;
        for (float& weight : w)
            weight = 0;
    }
    void reset(void) {
        for (float& sample : x)
            sample = 0;
        for (float& weight : w)
            weight = 0;
        x_ptr = x;
    }
    void push(float sample) {
        // unwind x_ptr
        if (x_ptr == x)
            x_ptr = x_end - 1;
        else
            --x_ptr;
        // write new sample
        *x_ptr = sample;
    }
    float dot_product() {
        float result = 0;
        float *it_x, *it_w;
        for (it_x = x_ptr, it_w = w; it_x != x_end; ++it_x, ++it_w)
            result += *it_x * *it_w;
        for (it_x = x; it_x != x_ptr; ++it_x, ++it_w)
            result += *it_x * *it_w;
        return result;
    }
    void update() {
        float *it_x, *it_w;
        for (it_x = x_ptr, it_w = w; it_x != x_end; ++it_x, ++it_w)
            *it_w += 2 * mu * err * *it_x;
        for (it_x = x; it_x != x_ptr; ++it_x, ++it_w)
            *it_w += 2 * mu * err * *it_x;
    }
};

void *adapf_init(void)
{
    LMS_data *data = new LMS_data();
    if (!data) return nullptr;
    return data;
}

void *adapf_restart(void *data)
{
    if (!data)
        return adapf_init();
    static_cast<LMS_data *>(data)->reset();
    return data;
}

int adapf_close(void *data)
{
    if (!data)
        return 0;
    delete static_cast<LMS_data *>(data);
    return 1; // success
}

float adapf_run(void *data, float sample, float y)
{
    LMS_data *&& lms = static_cast<LMS_data *>(data);
    lms->push(sample);
    lms->err = y - lms->dot_product();
    lms->update();
    return lms->err;
}

void adapf_getw(void *data, float **begin, unsigned *n)
{
    *begin = static_cast<LMS_data *>(data)->w;
    *n = N;
}

}
