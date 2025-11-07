# BU-Act General Libraries

## General
* Customer: Melexis
* Product: Gen3
* Description: Libraries

## Getting started

### Clean unit-test build using docker

```
rm -r ./_verification/unit_test/framework/build
rm -r ./_verification/unit_test/framework/libunit_test_framework.a
```

### Run all unit-tests using docker

```
docker run --rm -v $(pwd):$(pwd) -ti dsl.melexis.com:5001/bu-act/mlx16-gcc-3-1-115-3:1.0.0 make utest -C $(pwd)
```

### Run one unit-test (ie swtimer) using docker

```
rm -r ./_verification/unit_test/simulation/swtimer
docker run --rm -v $(pwd):$(pwd) -ti dsl.melexis.com:5001/bu-act/mlx16-gcc-3-1-115-3:1.0.0 make utest UTESTS=swtimer -C $(pwd)
```

## Continuous Integration

| Branch         | Build                              | Coverage                           |
| --------------:|:----------------------------------:|:----------------------------------:|
| Master         | ![master build status](https://gitlab.melexis.com/bu-act/libraries/badges/master/pipeline.svg) | ![master coverage report](https://gitlab.melexis.com/bu-act/libraries/badges/master/coverage.svg) |
| Develop        | ![develop build status](https://gitlab.melexis.com/bu-act/libraries/badges/develop/pipeline.svg) | ![develop coverage report](https://gitlab.melexis.com/bu-act/libraries/badges/develop/coverage.svg) |
| utest          | ![utest build status](https://gitlab.melexis.com/bu-act/libraries/badges/utest/pipeline.svg) | ![utest coverage report](https://gitlab.melexis.com/bu-act/libraries/badges/utest/coverage.svg) |

## License
Copyright (C) 2015-2021 Melexis N.V.

The Software is being delivered 'AS IS' and Melexis, whether explicitly or implicitly, makes no warranty as to its Use or performance.
The user accepts the Melexis Firmware License Agreement.

Melexis confidential & proprietary
