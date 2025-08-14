# Building

The project builds as a `CMake` library target (`fiber`), a demo executable and a
benchmarks executable. Benchmarks must be built in Release.
The project adds third-party dependencies (i.e. `Boost`) via `Conan`.
A `Makefile` is provided to streamline building and running executables.

To configure your environment
```
make configure
```

To build the `fiber` library
```
make build
```

To run the benchmarks
```
make run-benchmark
```
