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

"""
TODO:
* fornecer nome do build-dir                       -b, --build-dir
* fornecer CPPFLAGS                                -f, --cppflags
* fornecer compile-only flags                      --compile-flags
* fornecer link-only flags                         --link-flags
* verificar se é C ou C++ e compilar de acordo
* fornecer compilador customizado                  -c, --compiler-name
* fornecer linker customizado                      -l, --linker-name
* não usar compile flags default                   ++compile-flags
* não usar link flags default                      ++link-flags
* fazer TODO dentro do gen_obj ("quotar" parâmetros do bash). O mesmo
  deve ser feito em todos os outros lugares que imprimimos linhas de
  comando.
* dry run                                          -n, --dry-run
* caso haja mais de um source e o usuário não especificar um título,
  usar
  http://eli.thegreenplace.net/2011/07/03/parsing-c-in-python-with-clang
  para descobrir qual dos sources tem a definição do adapf_run, e usar o
  nome dele como título. Se o não for possível acessar o libclang (fazer
  o 'import' dentro de um try/catch -- descobrir qual a maneira mais
  recomendada de fazer um import somente se o módulo estiver instalado),
  fallback no caso anterior (usar o primeiro source que tem a string
  'adapf_run')
  OBS: a gente pode usar o 'nm' ou o 'objdump', depois de compilar cada
       source separadamente, para verificar qual delas define o símbolo
       'adapf_run'. Muito mais fácil do que essa treta da libclang.
* após compilar, usar:
    nm --extern-only --no-sort --portability schrubbles.so
  para verificar se todos os símbolos necessários estão presentes no DSO
  gerado. Mostrar um warning (e as linhas relevantes do output do nm)
  para cada símbolo necessário que não esteja presente. Também mostrar
  warning para cada símbolo que esteja presente, mas não na secão 'text'
  (com um T após o nome na saída do nm --portability). (testar se
  o símbolo vai deixar de ser compilado como 'text' se for uma variável
  por exemplo. Se esse for o caso, adicionar a dica no warning: "tem
  certeza que adapf_schrubbles é uma função, e não uma variável?")
* o item anterior (usar nm para checar a DSO) deve ser executado somente
  se o sistema for Linux. O nm não funciona direito no OSX.
* aceitar defaults para a maioria das opções passados via environment
  (e.g. CC, CFLAGS, etc)
* fornecer mais de uma code-file (usar action='append' no argparse)
* testar se o stdout é terminal ou não, e se não for, não imprimir
  cores. adicionar flag --color={never,auto,always}, igual ao grep
  (default é auto, 'false' e 'no' são sinônimos de never, 'true' e 'yes'
   são sinônimos de 'always'. Será que o add_argument() aceita uma lista
   de choices que seja case-insensitive?)
  (já testamos se é linux, né? Se não for linux, também não bota cor.
   procurar no google por "how to test if output accepts ANSI color
   codes" e "python colored output cross-platform")
* opção para não deletar a tempfile (-k, --keep-temps). A princípio, não
  muda o fato de que usaremos o NamedTemporaryFile para gerá-la -- só o
  que muda é que a gente vai falar para não deletar depois.
* opção para escolher o nome da tempfile. Isso significa que não
  usaremos mais o NamedTemporaryFile -- vamos criar o arquivo na mão.
  Mas isso não muda o fato de que o arquivo será deletado depois. (e
  portanto, será deletado manualmente por esse script, já que foi criado
  manualmente.)
"""

import glob
import sys
import os
import os.path
import pathlib
import textwrap
import unicodedata
import subprocess
import mmap

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'
    @classmethod
    def warn(cls, s):
        return cls.BOLD + cls.WARNING + s + cls.ENDC
    @classmethod
    def fail(cls, s):
        return cls.BOLD + cls.FAIL + s + cls.ENDC
    @classmethod
    def bold(cls, s):
        return cls.BOLD + s + cls.ENDC
    @classmethod
    def green(cls, s):
        return cls.BOLD + cls.OKGREEN + s + cls.ENDC
    @classmethod
    def head(cls, s):
        return cls.HEADER + s + cls.ENDC
    @classmethod
    def ok(cls, s):
        return cls.OKBLUE + s + cls.ENDC

source_files = ["*.c", "*.cpp"]
source_files_str = ", ".join(source_files)

build_dir = 'build'
main_src_string = 'adapf_run'

tw = textwrap.TextWrapper(subsequent_indent='      ')

cc            = {'C++': ["g++"],
                 'C':   ["gcc"]}
cflags_std    = {'C++': ["-std=c++11"],
                 'C':   ["-std=c11"  ]}
cflags_wng    =         ["-Wall", "-Werror", "-pedantic"]
cflags_opt    =         ["-march=native", "-mtune=native", "-O3"]
cflags        = {'C++': cflags_std['C++'] + cflags_wng + cflags_opt,
                 'C'  : cflags_std['C']   + cflags_wng + cflags_opt}
compile_flags =         ["-fPIC"]
link_flags    =         ["-shared"]
compile_cmd   = {'C++': cc['C++'] + cflags['C++'] + compile_flags,
                 'C':   cc['C']   + cflags['C']   + compile_flags}
link_cmd      =         cc['C++'] + link_flags + cflags_wng + cflags_opt

def gen_obj(f, obj):
    comp = "Compiling:"
    compmsg = tw.fill(comp + " " + f + " into " + os.path.split(obj)[-1])
    compmsg = compmsg.replace(comp, bcolors.green(comp), 1)
    compmsg = compmsg.replace(f, bcolors.head(f), 1)
    compmsg = compmsg.replace(os.path.split(obj)[-1],
                              bcolors.head(os.path.split(obj)[-1]), 1)
    print(compmsg)
    if f.endswith(".c"):
        lang = 'C'
    elif f.endswith(".cpp"):
        lang = 'C++'
    else:
        die(13, "File " + f + " should be `.c' or `.cpp'.")
    comp_full = compile_cmd[lang] + ["-o", obj, "-c", f]
    # TODO: caso algum dos parêmetros (elementos da lista 'comp_full')
    #       tenha caracteres estranhos, o check_call vai ficar de boa,
    #       mas o " ".join vai ficar confuso. Quotar os parâmetros
    #       que tiverem caracteres zoados (dica: shutils?)
    print(" ".join(comp_full))
    try:
        subprocess.check_call(comp_full)
    except subprocess.CalledProcessError:
        die(2, "Error while compiling source file " + f + " .")

def link_objs(objs, dso):
    link = "Linking:"
    linkmsg = tw.fill(link + " dynamic shared object " + dso)
    linkmsg = linkmsg.replace(link, bcolors.green(link), 1)
    linkmsg = linkmsg.replace(dso, bcolors.head(dso), 1)
    print(linkmsg)
    link_full = link_cmd + ["-o", dso] + objs
    print(" ".join(link_full))
    try:
        subprocess.check_call(link_full)
    except subprocess.CalledProcessError:
        die(3, "Error while linking " + dso + " .")

def list_all_sources(globs):
    sources = []
    for g in globs:
        sources += glob.glob(g)
    return sources

def get_src_listing(s):
    with open(s, 'r') as f:
        listing = f.readlines()
    return listing

def source_has_str(src, string):
    with open(src, mode='rb', buffering=0) as f, \
         mmap.mmap(f.fileno(), length=0, access=mmap.ACCESS_READ) as s:
        result = (s.find(string.encode()) != -1)
    return result

def safe_name(name):
    name = unicodedata.normalize('NFKD', name)
    name = name.encode('ascii', 'ignore').decode()
    for c in r'!"#$&*<>?\`' + r"'":
        name = name.replace(c, '')
    for c in r'/;|':
        name = name.replace(c, '-')
    for c in r' ~':
        name = name.replace(c, '_')
    if len(name) == 0:
        return '_'
    if name[0] == '.':
        name = 'dot-' + name[1:]
    return name

def die(code, msg):
    argv0 = sys.argv[0] + ":"
    errmsg = "error:"
    diemsg = tw.fill(argv0 + " " + errmsg + " " + msg)
    diemsg = diemsg.replace(argv0, bcolors.bold(argv0), 1)
    diemsg = diemsg.replace(errmsg, bcolors.fail(errmsg), 1)
    print(diemsg, file=sys.stderr)
    diemsg = tw.fill("{0}: Try '{0} --help' to read more.".format(sys.argv[0]))
    diemsg = diemsg.replace(argv0, bcolors.bold(argv0), 1)
    print(diemsg, file=sys.stderr)
    sys.exit(code)

def warn(msg):
    argv0 = sys.argv[0] + ":"
    errmsg = "warning:"
    warnmsg = tw.fill(argv0 + " " + errmsg + " " + msg)
    warnmsg = warnmsg.replace(argv0, bcolors.bold(argv0), 1)
    warnmsg = warnmsg.replace(errmsg, bcolors.warn(errmsg), 1)
    print(warnmsg, file=sys.stderr)

def cstr(s):
    return (
        '"'
        + s.replace('\\', '\\\\').replace('"', r'\"').replace('\n', '\\n')
        + '"'
    )

def safename_from_path(path):
    return safe_name(os.path.splitext(os.path.split(path)[-1])[0])

if __name__ == '__main__':

    import argparse
    import atexit
    import tempfile

    desc = "Adaptive filter DSO builder for ATFA"

    parser = argparse.ArgumentParser(description=desc, prefix_chars='-+')

    ### List of sources

    parser.add_argument(
        "files",
        help="List of files to compile into a DSO. If not given, the"
             " default is to use all " + source_files_str + " files in"
             " the current directory.",
        nargs='*',
        metavar='FILES',
    )

    ### Title of the algorithm

    title_group = parser.add_mutually_exclusive_group()

    title_group.add_argument(
        "-t", "--title",
        help="Title for your adaptive filter algorithm.",
    )

    title_group.add_argument(
        "+t",
        help="Do not generate an 'adapf_title' function. You should provide it"
             " yourself.",
        action='store_false',
        dest='gen_title',
    )

    ### Listing file

    listing_group = parser.add_mutually_exclusive_group()

    listing_group.add_argument(
        "-l", "--listing",
        help="File with a custom algorithmic description of the adaptive"
             " filter.",
        metavar="FILE",
    )

    listing_group.add_argument(
        "+l",
        help="Do not generate an 'adapf_listing' function. You should provide"
             " it yourself.",
        action='store_false',
        dest='gen_listing',
    )

    ### Output DSO filename

    parser.add_argument(
        "-o", "--dso",
        help="Output DSO filename."
    )

    ### Parse args

    args = parser.parse_args()
    #print(args)

    if args.title is not None:
        if args.title == "":
            die(8, "Title must be non-empty.")

    if args.listing is not None:
        if args.listing == "":
            die(9, "Listing filename must be nonempty.")
        if not os.path.exists(args.listing):
            die(10, "Listing file doesn't exist.")
        if os.path.isdir(args.listing):
            die(11, "Listing file is a directory.")
        if not os.access(args.listing, os.R_OK):
            die(11, "Listing file is not readable.")

    if args.dso is not None:
        if args.dso == "":
            die(12, "DSO filename must be nonempty.")

    ### Sanitize build dir

    try:
        os.makedirs(build_dir)
    except FileExistsError:
        if not os.path.isdir(build_dir):
            die(4, "Build directory '" + build_dir + "' is a file.")
    if not os.access(build_dir, os.W_OK):
        die(5, "Bad permissions on build directory '" + build_dir + "'.")
    if not os.access(build_dir, os.R_OK):
        die(6, "Bad permissions on build directory '" + build_dir + "'.")
    if not os.access(build_dir, os.X_OK):
        die(7, "Bad permissions on build directory '" + build_dir + "'.")

    ### Get source and object lists

    sources = {s: '' for s in args.files}
    if len(sources) == 0:
        sources = {s: '' for s in list_all_sources(source_files)}


    bad_sources = []
    for k in sources:
        if not pathlib.Path(k).is_file():
            warn("Source '{}' is not a file: skipping.".format(k))
            bad_sources.append(k)
        elif os.stat(k).st_size == 0:
            warn("Source '{}' is empty: skipping.".format(k))
            bad_sources.append(k)
    for k in bad_sources:
        del sources[k]

    if len(sources) == 0:
        die(1, "Couldn't find source files (" + source_files_str + ").")

    for k in sources:
        se = os.path.splitext(os.path.split(k)[1])
        sn = safe_name(se[0])
        if not sn in sources.values():
            sources[k] = sn
            continue
        sn = sn + se[1].replace('.', '_')
        if not sn in sources.values():
            sources[k] = sn
            continue
        N = 2
        sn_bkp = sn
        while sn in sources.values():
            sn = sn_bkp + '-' + str(N)
            N += 1
        sources[k] = sn

    for k in sources:
        sources[k] = os.path.join(build_dir, sources[k] + ".o")

    ### Find which source has 'adapf_run', to set the title

    main_source = []

    for k in sources:
        if source_has_str(k, main_src_string):
            fond = "Found:"
            fondmsg = tw.fill(fond + " " + main_src_string + " in " + k)
            fondmsg = fondmsg.replace(fond, bcolors.green(fond), 1)
            fondmsg = fondmsg.replace(main_src_string,
                                      bcolors.ok(main_src_string), 1)
            fondmsg = fondmsg.replace(k, bcolors.head(k), 1)
            print(fondmsg)
            main_source.append(k)

    if len(main_source) == 0:
        warn("Looks like none of the source files contains the"
             " definition of the '" + main_src_string + "' routine!")
        main_source = list(sources)
    elif len(main_source) > 1:
        warn("Looks like more than one of the source files define the"
             " '" + main_src_string + "' routine!")

    if args.gen_title:
        if args.title is not None:
            title = args.title
        elif args.dso is not None:
            title = safename_from_path(args.dso)
            title = title.upper()
        elif args.listing is not None:
            title = safename_from_path(args.listing)
            title = title.upper()
        else:
            title = safename_from_path(main_source[0])
            title = title.upper()

    ### Generate listing

    if args.gen_listing:
        if args.listing is None:
            if len(sources) == 1:
                listing = get_src_listing(list(sources)[0])
            else:
                listing = []
                for k in sources:
                    listing.append("/"*30 + "\n")
                    listing.append("/"*5 + "\n")
                    listing.append("/"*5 + " "*5 + k + "\n")
                    listing.append("/"*5 + "\n")
                    listing.append("/"*30 + "\n")
                    listing.append("\n")
                    listing += get_src_listing(k)
                    listing.append("\n\n")
        else:
            listing = get_src_listing(args.listing)

    ### Generate and compile adapf_autogen.cpp

    if args.gen_title:
        #print(title)
        title_fmt = ('extern "C" const char *adapf_title(void)'
                     ' {{ return {}; }}\n')
        title_txt = title_fmt.format(cstr(title))
    else:
        title_txt = ''

    if args.gen_listing:
        lstng_txt = 'extern "C" const char *adapf_listing(void) { return\n'
        for line in listing:
            lstng_txt += '    {}\n'.format(cstr(line))
        lstng_txt += '; }\n'
    else:
        lstng_txt = ''

    if not (title_txt == '' and lstng_txt == ''):

        sn = "atfa_autogen"
        N = 2
        sn_bkp = sn
        while os.path.join(build_dir, sn + ".o") in sources.values():
            sn = sn_bkp + '-' + str(N)
            N += 1
        sn = os.path.join(build_dir, sn + ".o")

        pref = "atfa_autogen_"
        suff = ".cpp"
        with tempfile.NamedTemporaryFile(prefix=pref, suffix=suff) as f:
            autogen_name = f.name
            agen = "Generating:"
            agenmsg = tw.fill(agen + " source file " + autogen_name)
            agenmsg = agenmsg.replace(agen, bcolors.green(agen), 1)
            agenmsg = agenmsg.replace(autogen_name, bcolors.head(autogen_name),
                                      1)
            print(agenmsg)
            sources[autogen_name] = sn
            f.write(title_txt.encode())
            f.write("\n".encode())
            f.write(lstng_txt.encode())
            f.flush()
            gen_obj(autogen_name, sources[autogen_name])

    else:
        autogen_name = None

    ### Compile other sources

    for k in sources:
        if k == autogen_name:
            continue
        gen_obj(k, sources[k])

    ### Link

    if args.dso is not None:
        dso_filename = args.dso
    else:
        dso_filename = safe_name(title) + ".so"
        dso_filename = dso_filename.lower()

    link_objs(list(sources.values()), dso_filename)
