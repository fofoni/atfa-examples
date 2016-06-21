
Adaptive filter algorithm examples and DSO build helper for ATFA
================================================================

ATFA is the [Ambiente de Testes para Filtros Adaptativos][1] (Testing
environment for adaptive filters).

Quick How-To
------------

The algorithms must be compiled and linked into a `*.so` file. This can
be done manually, or by using the helper script [`build.py`][2].
Aditionally, the script [`build-all.sh`][4] can also be used to compile
all of the examples at once.

If you just want to compile the examples to use from ATFA, type:

    $ cd atfa-examples
    $ chmod +x build-all.sh build.py
    $ ./build-all.sh

Algorithm examples
------------------

ATFA accepts custom adaptive filtering algorithms in the form of dynamic
shared object (*.so) files. Each adaptive filter DSO must export the
following symbols:

* `adapf_init`
* `adapf_restart`
* `adapf_close`
* `adapf_run`
* `adapf_getw`
* `adapf_title`
* `adapf_listing`

The last two (`adapf_title` and `adapf_listing`) are generated
automatically by the [`build.py`][2] script. Below, each of the
functions to be exported is explained:

* `void *adapf_init()`: The `adapf_init` function initializes (e.g.,
  acquires memory for) the data structures that the algorithm will
  use. This data structure must include the impulse response vector
  (vector of filter coefficients), the input vector, and any other information the algorithm wants to keep
  track of. This function must return a pointer to the initialized data.
* `void *adapf_restart(void *data)`: This function resets the
  information contained in `data` (e.g., zeroes out the impulse response
  that has been learned so far), and returns a pointer to the zeroed
  data. This function might free the memory pointed to by the old data,
  and return a pointer newly allocated-and-initialized memory.
* `int adapf_close(void *data)`: This function releases the resources
  acquired in `adapf_init`. ATFA will call it before closing the access
  to the DSO, in order to prevent memory leaking. The pointer passed to
  `adapf_close` is the one returned by `adapf_init`, and the value
  returned from adapf_close should be zero if it fails and nonzero
  otherwise.
* `float adapf_run(void *data, float sample, float y)`: This is the
  function which implements the algorithm itself. Given an audio sample
  from the input of the filter (`sample`) and an audio sample from the
  desired signal (`y`), `adapf_run` should calculate the filter output,
  say `y_hat`, and then return the error `y - y_hat`. This function can
  read from and write to `*data` in order to access and/or update
  values such as the filter coefficients, the input vector (with
  memory), etc. `adapf_run` is the only function that's called from
  within the audio [run-time loop][6]. The other functions are all
  allowed to request resources to the OS, and perform slow operations
  in general, but this one isn't.

How to compile an algorithm
---------------------------

To compile an algorithm into a DSO file to be used with ATFA, use the
[`build.py`][3] helper script.

### Using `build-all.sh`

You can also use [`build-all.sh`][5].

[1]: https://github.com/fofoni/atfa
[2]: #how-to-compile-an-algorithm
[3]: https://github.com/fofoni/atfa-examples/blob/master/build.py
[4]: #using-build-all-sh
[5]: https://github.com/fofoni/atfa-examples/blob/master/build-all.sh
[6]: http://portaudio.com/docs/v19-doxydocs/writing_a_callback.html
