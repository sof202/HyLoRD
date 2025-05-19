# Tests

The tests written for HyLoRD are written using the 
[Google test](https://github.com/google/googletest) framework. This document
describes the layout and how to run said tests.

## Layout

- `/test/unit`: Unit tests
- `/test/integration`: Integration tests

## Running tests

To run all tests:

```bash
make test
```

If any tests fail, use:

```bash
# Navigate to build directory of the project
cd build/

# Rerun failed tests with additional output
ctest --rerun-failed --output-on-failure
```

This will provide you with additional information for why the tests failed.

### Running specific tests

If you want to only run specific tests (from a particular suite for example),
you need to build the test executable with:

```bash
mkdir -p build/
cd build/
cmake --build .. -DHYLORD_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug

# Run tests with specific regex
ctest -R "<test-regex>"
```

>[!INFO]
> If `make test` has already been run, one need only run the `ctest` command
> from within the `build/` directory.

#### Example

If you wanted to run all integration tests surrounding TSV reading you would
use:

```bash
# Same as above
...

# Run only TSV Reader integration tests
ctest -R "TSVReaderIntegrationTest"
```

#### Listing all available tests

A list of all tests can be found by running:

```bash
# From within build/
ctest -N
```
