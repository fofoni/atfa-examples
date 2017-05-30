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

import glob
import sys
import os
import os.path
import pathlib
import textwrap
import unicodedata
import subprocess
import mmap
import inspect

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

# distribution directory
dist_dir = os.path.dirname(os.path.realpath(__file__))
# current directory
curr_dir = os.path.realpath(os.getcwd())

source_files = ["*.c", "*.cpp"]
source_files_str = ", ".join(source_files)

build_dir = 'build'
atfa_api_header_filename = 'atfa_api.h'
main_src_string = 'adapf_api' # string contained by the "main" source file

tw = textwrap.TextWrapper(subsequent_indent='      ')

cc            = {'C++': ["g++"],
                 'C':   ["gcc"]}
cflags_std    = {'C++': ["-std=c++11"],
                 'C':   ["-std=c11"  ]}
cflags_wng    =         ["-Wall", "-Wextra", "-Werror", "-pedantic"]
cflags_opt    =         ["-march=native", "-mtune=native", "-O3"]
cflags        = {'C++': cflags_std['C++'] + cflags_wng + cflags_opt,
                 'C'  : cflags_std['C']   + cflags_wng + cflags_opt}
compile_flags = lambda dir:(
                        ["-fPIC", "-I{}".format(dir)] if dir else
                        ["-fPIC"])
link_flags    =         ["-shared", "-rdynamic", "-Wl,--no-undefined"]
compile_cmd   = lambda dir:(
                {'C++': cc['C++'] + cflags['C++'] + compile_flags(dir),
                 'C':   cc['C']   + cflags['C']   + compile_flags(dir)})
link_cmd      =         cc['C++'] + link_flags + cflags_wng + cflags_opt

def gen_obj(f, obj, include_dir=None):
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
        die(13, "File ‘"+ f +"’ should be ‘.c’ or ‘.cpp’.")
    comp_full = compile_cmd(include_dir)[lang] + ["-o", obj, "-c", f]
    # TODO: caso algum dos parêmetros (elementos da lista 'comp_full')
    #       tenha caracteres estranhos, o check_call vai ficar de boa,
    #       mas o " ".join vai ficar confuso. Quotar os parâmetros
    #       que tiverem caracteres zoados (dica: shutils? shlex?)
    print(" ".join(comp_full))
    try:
        subprocess.check_call(comp_full)
    except subprocess.CalledProcessError:
        die(2, "Error while compiling source file ‘"+ f +"’.")

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
        die(3, "Error while linking ‘"+ dso +"’ .")

def list_all_sources(globs):
    sources = []
    for g in globs:
        sources += glob.glob(g)
    return sources

def get_src_listing(s):
    try:
        with open(s, 'r') as f:
            listing = f.readlines()
    except PermissionError:
        die(15, "Permission error: could not read listing file ‘"+ s +"’.")
    return listing

def source_has_str(src, string):
    try:
        with open(src, mode='rb', buffering=0) as f, \
             mmap.mmap(f.fileno(), length=0, access=mmap.ACCESS_READ) as s:
            result = (s.find(string.encode()) != -1)
    except PermissionError:
        die(16, "Permission error: could not read source file ‘"+ s +"’.")
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
    diemsg = tw.fill("{0}: Try ‘{0} --help’ to read more.".format(sys.argv[0]))
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

def debugmsg(msg, filename, lineno):
    prologue = "{}:{}:".format(filename, lineno)
    warnmsg = tw.fill(prologue + " " + msg)
    warnmsg = warnmsg.replace(prologue, bcolors.bold(prologue), 1)
    print(warnmsg, file=sys.stderr)

def logmsg(vbs, level, msg):
    caller_frame = inspect.getframeinfo(inspect.currentframe().f_back)
    if vbs >= level:
        debugmsg(msg, caller_frame.filename, caller_frame.lineno)

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
    import autogen_templates

    desc = "Adaptive filter DSO builder for ATFA" # program description

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
        "+t", "++title",
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
        "+l", "++listing",
        help="Do not generate an 'adapf_listing' function. You should"
             " provide it yourself.",
        action='store_false',
        dest='gen_listing',
    )

    ### Output DSO filename

    parser.add_argument(
        "-o", "--dso",
        help="Output DSO filename.",
    )

    ### Keep temporary files

    parser.add_argument(
        "-k", "--keep-temps",
        help="Do not delete temporary auto-generated files.",
        action='store_true',
        dest='keep_temps',
    )

    ### Do not gen api table

    parser.add_argument(
        "++api-table",
        help="Do not generate the API table. You should provide it yourself.",
        action="store_false",
        dest="gen_apitb",
    )

    ### Do not fail on undefined symbols

    parser.add_argument(
        "++no-undefined",
        help="Do not pass the option ‘--no-undefined’ to the linker. The"
             " ‘--no-undefined’ option might give you some \"undefined"
             " symbol\" warnings. Most of the time, however, you should go"
             " with the default.",
        action="store_true",
        dest="accept_undefined",
    )

    ### Verbose output

    parser.add_argument(
        "-v", "--verbose",
        help="Get more descriptive output.",
        action="count",
        default=0,
    )

    ### Parse args

    args = parser.parse_args()
    vbs = args.verbose # verbosity (0 to 3)

    # user must provide a non-empty title, or no title at all
    if args.title is not None:
        if args.title == "":
            die(8, "Title must be non-empty.")
        logmsg(vbs, 3, "Non-empty title was provided.")
    else:
        logmsg(vbs, 3, "Title was not provided")

    if args.listing is not None:
        if args.listing == "":
            die(9, "Listing filename must be nonempty.")
        logmsg(vbs, 3, "Non-empty listing filename was provided.")
        if not os.path.exists(args.listing):
            die(10, "Listing file doesn't exist.")
        logmsg(vbs, 3, "Listing file exists.")
        if os.path.isdir(args.listing):
            die(11, "Listing file is a directory.")
        logmsg(vbs, 3, "Listing file is not a directory.")
        if not os.access(args.listing, os.R_OK):
            die(14, "Listing file is not readable.")
        logmsg(vbs, 3, "Listing file is readable.")
    logmsg(vbs,3, "Listing file not provided.")

    if args.dso is not None:
        if args.dso == "":
            die(12, "DSO filename must be nonempty.")
        logmsg(vbs,3, "Non-empty DSO filename was provided.")
    else:
        logmsg(vbs,3, "DSO filename was not provided.")

    ### Sanitize build dir

    try:
        os.makedirs(build_dir)
        logmsg(vbs,1, "Build dir didn't exist: created.")
    except FileExistsError:
        if not os.path.isdir(build_dir):
            die(4, "Build directory ‘"+ build_dir +"’ is a file.")
        logmsg(vbs,3, "Build dir already exists.")
    if not os.access(build_dir, os.W_OK):
        die(5, "Bad permissions on build directory ‘"+ build_dir +"’.")
    logmsg(vbs,3, "Build dir is writable.")
    if not os.access(build_dir, os.R_OK):
        die(6, "Bad permissions on build directory ‘"+ build_dir +"’.")
    logmsg(vbs,3, "Build dir is readable.")
    if not os.access(build_dir, os.X_OK):
        die(7, "Bad permissions on build directory ‘"+ build_dir +"’.")
    logmsg(vbs,3, "Build dir is listable.")

    ### Get source and object lists

    sources = {s: '' for s in args.files}
    if len(sources) == 0:
        sources = {s: '' for s in list_all_sources(source_files)}
        logmsg(vbs,1, "No source files provided. All sources in current"
                      " directory will be compiled.")
    # TODO: "sources" is plural only if {}!=1
    logmsg(vbs,2, "Detected {} user-provided sources.".format(len(sources)))

    bad_sources = []
    for k in sources:
        if not pathlib.Path(k).is_file():
            warn("Source ‘{}’ is not a file: skipping.".format(k))
            bad_sources.append(k)
        elif os.stat(k).st_size == 0:
            warn("Source ‘{}’ is empty: skipping.".format(k))
            bad_sources.append(k)
        else:
            logmsg(vbs,1,"Source ‘{}’ looks ok.".format(k))
    for k in bad_sources:
        del sources[k]

    if len(sources) == 0:
        die(1, "Couldn't find source files (" + source_files_str + ").")
    # TODO: "sources" is plural only if {}!=1
    logmsg(vbs,2, "Will compile {} user-provided sources.".format(len(sources)))

    for k in sources:
        se = os.path.splitext(os.path.split(k)[1])
        sn = safe_name(se[0])
        if not sn in sources.values():
            sources[k] = sn
            logmsg(vbs,3,"Name of ‘{}’ is: ‘{}’".format(k,sn))
            continue
        logmsg(vbs,2,"Name of ‘{}’ would be ‘{}’ but it's taken.".format(k,sn))
        sn = sn + se[1].replace('.', '_')
        if not sn in sources.values():
            sources[k] = sn
            logmsg(vbs,2,"Name of ‘{}’ is: ‘{}’.".format(k,sn))
            continue
        N = 2
        sn_bkp = sn
        while sn in sources.values():
            logmsg(vbs,2,"Name of ‘{}’ would be ‘{}’ but it's taken.".format(
                k,sn))
            sn = sn_bkp + '_' + str(N)
            N += 1
        sources[k] = sn
        logmsg(vbs,2,"Name of ‘{}’ is: ‘{}’.".format(k,sn))

    for k in sources:
        sources[k] = os.path.join(build_dir, sources[k] + ".o")
        logmsg(vbs,3, "‘{}’ will be compiled to ‘{}’".format(k,sources[k]))

    ### Find which source has 'adapf_api', to set the title

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
        logmsg(vbs,3, "‘{}’ apparently does not define ‘{}’".format(
            k, main_src_string))

    qtd_apitb = len(main_source)
    logmsg(vbs,3, "Found {} user-provided sources defining ‘{}’".format(
        qtd_apitb, main_src_string))
    if args.gen_apitb:
        qtd_apitb += 1
    logmsg(vbs,3, "Total of sources defining ‘{}’: {}".format(
        main_src_string, qtd_apitb))
    if qtd_apitb == 0:
        warn("Looks like none of the source files contains the"
             " definition of the ‘"+ main_src_string +"’ symbol table!")
    elif qtd_apitb > 1:
        warn("Looks like more than one of the source files define the"
             " ‘"+ main_src_string +"’ symbol table!")
    else:
        logmsg(vbs,2, "Apparently ‘{}’ will be defined exactly once.")
    if len(main_source) == 0:
        main_source = list(sources)
        logmsg(vbs,2, '"Main" source file: ‘{}’'.format(main_source[0]))

    if args.title is not None:
        title = args.title
        logmsg(vbs,3, "User-provided title: ‘{}’".format(title))
    elif args.dso is not None:
        title = safename_from_path(args.dso)
        title = title.upper()
        logmsg(vbs,1, ("Title extracted from user-provided DSO filename:"
                       " ‘{}’").format(title))
    elif args.listing is not None:
        title = safename_from_path(args.listing)
        title = title.upper()
        logmsg(vbs,1, ("Title extracted from user-provided listing filename:"
                       " ‘{}’").format(title))
    else:
        title = safename_from_path(main_source[0])
        title = title.upper()
        logmsg(vbs,1, "Title extracted from source ‘{}’ filename: ‘{}’".format(
            main_source[0], title))

    ### Generate listing

    if args.gen_listing:
        if args.listing is None:
            if len(sources) == 1:
                listing = get_src_listing(list(sources)[0])
                logmsg(vbs,2,("Algorithm listing generated from project's"
                              " only source file: ‘{}’").format(
                                  list(sources)[0]))
            else:
                listing = []
                logmsg(vbs,2,("Algorithm listing will be generated from"
                              " project's {} source files.").format(
                                  len(sources)))
                for k in sources:
                    logmsg(vbs,3,"    Including ‘{}’ in listing...".format(k))
                    listing.append("/"*30 + "\n")
                    listing.append("/"*5 + "\n")
                    listing.append("/"*5 + " "*5 + k + "\n")
                    listing.append("/"*5 + "\n")
                    listing.append("/"*30 + "\n")
                    listing.append("\n")
                    listing += get_src_listing(k)
                    listing.append("\n\n")
                logmsg(vbs,3,"Listing finished generating.")
        else:
            listing = get_src_listing(args.listing)
            logmsg(vbs,2, "Algorithm listing generated from ‘{}’.".format(
                args.listing))

    ### Get full path to api header

    hypo_local_apih = os.path.join(curr_dir, atfa_api_header_filename)
    hypo_dist_apih = os.path.join(dist_dir, atfa_api_header_filename)
    if os.path.isfile(hypo_local_apih):
        atfa_api_header = hypo_local_apih
    elif os.path.isfile(hypo_dist_apih):
        atfa_api_header = hypo_dist_apih
    else:
        die(17, "Can't find ‘"+atfa_api_header_filename+"’")
    logmsg(vbs,1,"ATFA API header found at: {}".format(atfa_api_header))

    ### Generate and compile adapf_autogen.cpp

    if args.gen_title:
        title_txt = autogen_templates.title_fcn.format(cstr(title))
        logmsg(vbs,3,"Algorithm title function generated.")
    else:
        title_txt = ''
        logmsg(vbs,2,"Algorithm title function not generated.")

    if args.gen_listing:
        lstng_string = '\n'
        for line in listing:
            lstng_string += '{}\n'.format(cstr(line))
        lstng_txt = autogen_templates.listing_fcn.format(lstng_string)
        logmsg(vbs,3,"Algorithm listing function generated.")
    else:
        lstng_txt = ''
        logmsg(vbs,2,"Algorithm listing function not generated.")

    if args.gen_apitb:
        apitb_txt = autogen_templates.api_table_decl.format()
        logmsg(vbs,3,"API symbol table generated.")
    else:
        apitb_txt = ''
        logmsg(vbs,2,"API symbol table not generated.")

    if title_txt or lstng_txt or apitb_txt:

        user_provided_txt = '/* user provided */'
        if not title_txt:
            title_txt = user_provided_txt
        if not lstng_txt:
            lstng_txt = user_provided_txt
        if not apitb_txt:
            apitb_txt = user_provided_txt

        sn = "atfa_autogen"
        N = 2
        sn_bkp = sn
        while os.path.join(build_dir, sn + ".o") in sources.values():
            logmsg(vbs,3, ("Autogenerated object file would be ‘{}.o’,"
                           " but this name is taken.").format(sn))
            sn = sn_bkp + '-' + str(N)
            N += 1
        sn = os.path.join(build_dir, sn + ".o")
        logmsg(vbs,2, "Autogenerated object file will be: {}".format(sn))

        pref = "atfa_autogen_"
        suff = ".cpp"
        with tempfile.NamedTemporaryFile(
            prefix=pref, suffix=suff,
            delete=not args.keep_temps,
        ) as f:
            autogen_name = f.name
            agen = "Generating:"
            agenmsg = tw.fill(agen + " source file " + autogen_name)
            agenmsg = agenmsg.replace(agen, bcolors.green(agen), 1)
            agenmsg = agenmsg.replace(autogen_name, bcolors.head(autogen_name),
                                      1)
            print(agenmsg)
            sources[autogen_name] = sn
            f.write(autogen_templates.full_source.format(
                atfa_api_header_filename, title_txt, lstng_txt, apitb_txt
            ).encode())
            f.flush()
            gen_obj(autogen_name, sources[autogen_name], dist_dir)

    else:
        autogen_name = None
        logmsg(vbs,1, "No source file will be autogenerated.")

    ### Compile other sources
    for k in sources:
        if k == autogen_name:
            continue
        gen_obj(k, sources[k], dist_dir)

    ### Link

    if args.accept_undefined:
        link_cmd.remove("-Wl,--no-undefined")
        logmsg(vbs,2,"Removing ‘-Wl,--no-undefined’ from the list of GCC"
                     " options.")

    if args.dso is not None:
        dso_filename = args.dso
        logmsg(vbs,2,"Using user-provided DSO filename: ‘{}’".format(
            dso_filename))
    else:
        dso_filename = safe_name(title) + ".so"
        dso_filename = dso_filename.lower()
        logmsg(vbs,1, ("DSO filename extracted from the algorithm title:"
                       " ‘{}’.").format(dso_filename))

    link_objs(list(sources.values()), dso_filename)
