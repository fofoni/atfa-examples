
Adaptive filter algorithm examples and DSO build helper for ATFA
================================================================

ATFA is the [Ambiente de Testes para Filtros Adaptativos][1] (Testing
environment for adaptive filters).


Quick How-To
------------

The algorithms must be compiled and linked into an `*.so` file. This can
be done manually, or by using the helper script [`build.py`][2].
Aditionally, the script [`build-all.sh`][4] can also be used to compile
all of the examples at once.

If you just want to compile the examples to use from ATFA, type:

```bash
$ cd atfa-examples
$ bash build-all.sh
```

See [the section on using the `build.py` script][2] for more information.


How to write algorithms
-----------------------

ATFA accepts custom adaptive filtering algorithms in the form of dynamic
shared object (`*.so`) files. Each adaptive filter DSO must export the
following symbols:

* [`adapf_init`][adapf_init]
* [`adapf_restart`][adapf_restart]
* [`adapf_close`][adapf_close]
* [`adapf_run`][adapf_run]
* [`adapf_getw`][adapf_getw]
* [`adapf_title`][adapf_title]
* [`adapf_listing`][adapf_listing]

The last two (`adapf_title` and `adapf_listing`) are generated
automatically by the [`build.py`][2] script. Below, each of the
functions to be exported is explained:

### The `adapf_*` functions

#### `void *adapf_init()`

The `adapf_init` function initializes (e.g., acquires memory for) the
data structures that the algorithm will use. This data structure might
include, for example, the impulse response vector (vector of filter
coefficients), the input vector, and any other kind of parameter that
the algorithm needs to keep track of.

This function must return a pointer to the initialized data.

#### `void *adapf_restart(void *data)`

This function resets the information contained in `data` (e.g., zeroes
out the impulse response that has been learned so far), and returns a
pointer to the zeroed data. This function might free the memory pointed
to by the old data and return a pointer newly allocated-and-initialized
memory.

#### `int adapf_close(void *data)`

This function releases the resources acquired in `adapf_init`. ATFA will
call it before closing the access to the DSO, in order to prevent memory
leaking. The pointer passed to `adapf_close` is the one returned by
`adapf_init`, and the value returned from `adapf_close` should be zero
if it fails and nonzero otherwise.

#### `float adapf_run(void *data, float sample, float y)`

This is the function which implements the algorithm itself. Given an
audio sample from the input of the filter (`sample`) and an audio
sample from the desired signal (`y`), `adapf_run` should calculate
the filter output, say `y_hat`, and then return the error `y - y_hat`.

This function can read from and write to `*data` in order to access
and/or update values such as the filter coefficients, the input
vector (with memory), etc.

`adapf_run` is the only function that's called from within the audio
[run-time loop][6]. The other functions are all allowed to request
resources to the OS, and perform slow operations in general, but this
one isn't.

#### `void adapf_getw(void *data, float **begin, unsigned *n)`

This function is called by ATFA in order to introspect the filter
coefficients. It must place at `*begin` a pointer to the first element
of the array of filter coefficients, and at `*n` the number of elements
that can be read from that array (that is, the number of coefficients,
which is `N+1`, where `N` is the filter order).

#### `const char *adapf_title()`

This function should just return a static string containing a short
"name" for the adaptive filter algorithm. For example, if the algorithm
is an implementation of the NLMS algorithm with `mu=0.7` and
`Delta=1.5`, then the returned string could be `"NLMS: 0.7, 1.5"` or
maybe just `"NLMS"`.

#### `const char *adapf_listing()`

This function must return a static string, just like `adapf_title`. The
string returned from `adapf_listing` is an algorithmic description of
the adaptive filter, which is shown to the users when they click the
"Show filter code" button in ATFA.

### Examples

Inside the [`LMS/`][LMS], [`NLMS/`][NLMS], and [`AP/`][AP] directories,
you will find C++ source files implementing these three adaptive
filtering algorithms.

The [`no-op/`][no-op] direcory contains a file for a dummy algorithm,
which does nothing at all. The source file inside
[`static-w/`][static-w] implements a static (instead of an adaptive)
filter (the filter coefficients are never updated). Both of these can
be used for test purposes.

All of the above are implemented in C++. See [`LMS-C/`][LMS-C] for an
example using plain C.

How to compile an algorithm
---------------------------

To compile an algorithm into a DSO file to be used with ATFA, you can do
so manually—just generate an `*.so` that exports all of the functions
mentioned above—, or you can use the [`build.py`][3] helper script.

To use this script, open a shell inside the directory containing your
source file(s), and then run the script. For example, to compile the
[`LMS/`][LMS] algorithm, type:

```bash
$ cd atfa-examples
$ cd LMS
$ python3 ../build.py
```

By default, `build.py` will compile and link all of the `*.c` and
`*.cpp` files in the current directory. To use a custom set of sources,
just pass them as arguments:

```bash
$ python3 ../build.py source1.cpp source2.cpp
```

The full set of source files specified on the command-line (or present
in the directory, if none is specified) must contain the definitions of
all of the functions mentioned [above][7], except `atfa_title` and
`atfa_listing`. These will be automatically generated.

By default, the title is made up based on the command line, and the
algorithm listing is just a transcription of the source files. If you
want to use a custom title, use the `-t/--title` flag. If you want to
specify a custom file containing the listing, use the `-l/--listing`
flag:

```bash
$ python3 ../build.py --title "My Algorithm" --listing my_algorithm.txt
```

If you want to provide the `atfa_title` or `atfa_listing` functions
yourself, use the `+t` or `+l` flags:

```bash
$ python3 ../build.py main_source.cpp title_and_listing.cpp +tl
```

For more options, type:

```bash
$ python3 ../build.py --help
```

### Using `build-all.sh`

You can also use the convenience script [`build-all.sh`][5]. This script
will just call `python3 ../build.py` from inside each subdirectory of the
current directory. It will also pass the name of the directory in the
`--title` flag.

[1]: https://github.com/fofoni/atfa
[2]: #how-to-compile-an-algorithm
[3]: https://github.com/fofoni/atfa-examples/blob/master/build.py
[4]: #using-build-allsh
[5]: https://github.com/fofoni/atfa-examples/blob/master/build-all.sh
[6]: http://portaudio.com/docs/v19-doxydocs/writing_a_callback.html
[7]: #how-to-write-algorithms

[adapf_init]:    #void-adapf_init
[adapf_restart]: #void-adapf_restartvoid-data
[adapf_close]:   #int-adapf_closevoid-data
[adapf_run]:     #float-adapf_runvoid-data-float-sample-float-y
[adapf_getw]:    #void-adapf_getwvoid-data-float-begin-unsigned-n
[adapf_title]:   #const-char-adapf_title
[adapf_listing]: #const-char-adapf_listing

[LMS]:      https://github.com/fofoni/atfa-examples/tree/master/LMS
[NLMS]:     https://github.com/fofoni/atfa-examples/tree/master/NLMS
[AP]:       https://github.com/fofoni/atfa-examples/tree/master/AP
[no-op]:    https://github.com/fofoni/atfa-examples/tree/master/no-op
[static-w]: https://github.com/fofoni/atfa-examples/tree/master/static-w
[LMS-C]:    https://github.com/fofoni/atfa-examples/tree/master/LMS-C
