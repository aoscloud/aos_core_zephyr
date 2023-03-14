# AosCore zephyr application

## Overview

This project contains code of the Aos core application for zephyr OS.

## Prerequisites

Zephyr SDK is required to fetch and build this project. Follow
[Getting Started Guide](https://docs.zephyrproject.org/latest/getting_started/index.html) to install mentioned
dependencies and SDK.

## Fetch

Use zephyr `west` tool to fetch required repos:

```sh
west init -m  https://github.com/aoscloud/aos_core_zephyr --mr main aos_zephyr_sdk
cd aos_zephyr_sdk
west update
west zephyr-export
```

## Build

```sh
cd aos_core_zephyr

west build -b ${BOARD} -p auto
west build -t fix_dtb
west build -t run
```

For test and debug purpose `native_posix_64` or `native_posix` board can be used.
For simulation `qemu_x86` or `qemu_x86_64` board can be used.
For xen based system `xenvm-qemu` board can be used.

For `xenvm-qemu` additioal step before running should be executed
```sh
west build -t fix_dtb
```

## Run

```sh
west build -t run
```

## Unit tests

Unit tests are implemented using zephyr [Test Framework](https://docs.zephyrproject.org/latest/develop/test/ztest.html).

Use the following commands to run the application unit tests:

```sh
../zephyr/scripts/twister -c -v -T tests
```

All test reports will be saved in `twister-out` folder.

## Code coverage

Use the following command to calculate unit tests code coverage:

```sh
../zephyr/scripts/twister -c -v --coverage --coverage-basedir src/ --coverage-tool gcovr -p unit_testing -T tests
```

Open `twister-out/coverage/index.html` with your browser to see the code coverage result.

To see summary:

```sh
gcovr twister-out/unit_testing -f src/
```
