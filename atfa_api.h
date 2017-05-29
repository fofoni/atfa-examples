/*
 * Universidade Federal do Rio de Janeiro
 * Escola Politécnica
 * Projeto Final de Graduação
 * Ambiente de Teste para Filtros Adaptativos
 * Pedro Angelo Medeiros Fonini
 * Orientador: Markus Lima
 */

#ifndef ATFA_API_HEADER_H_
#define ATFA_API_HEADER_H_

#ifdef __cplusplus
#    define EXTERN_C_BEGIN extern "C" {
#    define EXTERN_C_END }
#    define ADAPF_DATA AdapfData
#else
#    define EXTERN_C_BEGIN
#    define EXTERN_C_END
#    define ADAPF_DATA struct AdapfData
#endif

EXTERN_C_BEGIN

struct AdapfData;

typedef ADAPF_DATA *adapf_init_t(void);
typedef ADAPF_DATA *adapf_restart_t(ADAPF_DATA *data);
typedef int adapf_close_t(ADAPF_DATA *data);
typedef float adapf_run_t(ADAPF_DATA *data, float sample, float y, int update);
typedef void adapf_getw_t(ADAPF_DATA *data, float **begin, unsigned *n);
typedef const char *adapf_title_t(void);
typedef const char *adapf_listing_t(void);

struct ATFA_API_table_t {
    adapf_init_t *init;
    adapf_restart_t *restart;
    adapf_close_t *close;
    adapf_run_t *run;
    adapf_getw_t *getw;
    adapf_title_t *title;
    adapf_listing_t *listing;
};

adapf_init_t adapf_init;
adapf_restart_t adapf_restart;
adapf_close_t adapf_close;
adapf_run_t adapf_run;
adapf_getw_t adapf_getw;
adapf_title_t adapf_title;
adapf_listing_t adapf_listing;

#ifndef __cplusplus
typedef struct ATFA_API_table_t ATFA_API_table_t;
#endif

EXTERN_C_END

#endif /* ATFA_API_HEADER_H_ */
