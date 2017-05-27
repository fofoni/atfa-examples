#!/usr/bin/python3
# -*- coding: utf-8 -*-

"""
Universidade Federal do Rio de Janeiro
Escola Politécnica
Projeto Final de Graduação
Ambiente de Teste para Filtros Adaptativos
Pedro Angelo Medeiros Fonini <pedro.fonini@smt.ufrj.br>
Orientador: Markus Lima
"""

title_fcn = """
    const char *adapf_title(void) {{
        return {};
    }}
"""

listing_fcn = """
    const char *adapf_listing(void) {{
        return {};
    }}
"""

api_table_decl = """
    ATFA_API_table_t adapf_api = {{
        adapf_init,
        adapf_restart,
        adapf_close,
        adapf_run,
        adapf_getw,
        adapf_title,
        adapf_listing,
    }};
"""

full_source = """
/* ATFA API header */
#include <{}>

extern "C" {{

/* title fcn */
{}

/* listing fcn */
{}

/* api table */
{}

}}
"""
