# pifan
### Raspberry Pi fan controller daemon

[![Test Status](https://github.com/mrabine/pifan/actions/workflows/cd.yml/badge.svg?branch=main)](https://github.com/mrabine/pifan/actions?query=workflow%3Acd+branch%3Amain)
[![Security Status](https://github.com/mrabine/pifan/actions/workflows/audit.yml/badge.svg?branch=main)](https://github.com/mrabine/pifan/actions?query=workflow%3Aaudit+branch%3Amain)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/aa8d30be1e104cdebdaf1861de8a8db1)](https://app.codacy.com/gh/mrabine/pifan/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
[![GitHub Releases](https://img.shields.io/github/release/mrabine/pifan.svg)](https://github.com/mrabine/pifan/releases/latest)
[![GitHub License](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/mrabine/pifan/blob/main/LICENSE)

Start fan when CPU temperature is high and stop it when CPU temperature is low.
Supports Raspberry Pi 4 and Raspberry Pi 5. The GPIO chip is auto-detected at startup and can be overridden with the `-g` option.

> **Note:** tested on Ubuntu 24.04.

## Dependencies

Pifan requires `libgpiod`, to install it do this:
```bash
sudo apt install libgpiod-dev
```

The `gpiod` package (CLI tools such as `gpiodetect`, `gpioinfo`) is optional but useful for troubleshooting:
```bash
sudo apt install gpiod
```

## Download

To download the latest source do this:
```bash
git clone https://github.com/mrabine/pifan.git
```

Move to `pifan` directory:
```bash
cd pifan
```

## Configuration

To configure pifan do this:
```bash
cmake --preset gcc-release -DCMAKE_INSTALL_PREFIX=/usr
```

## Build

To build pifan do this:
```bash
cmake --build --preset gcc-release
```

## Installation

To install pifan do this:
```bash
sudo cmake --install build/gcc-release
```

## Enabling

To enable pifan do this:
```bash
sudo systemctl enable --now pifan
```

## Usage

**pifan** [options]

**-c**\
&emsp;print the cpu temperature\
**-g chip**\
&emsp;gpio chip device name (default: auto detection, `"gpiochip0"`, `"gpiochip4"`).\
**-h**\
&emsp;show available options\
**-i interval**\
&emsp;sleep interval in seconds (default: 2)\
**-l threshold**\
&emsp;lower threshold in °C, fan stops below this value (default: 55)\
**-n**\
&emsp;don't fork into background\
**-p pin**\
&emsp;gpio pin number (default: 14)\
**-u threshold**\
&emsp;upper threshold in °C, fan starts above this value (default: 65)\
**-v**\
&emsp;print version

## Recommended thresholds

A 10°C hysteresis between lower and upper thresholds avoids rapid on/off cycling.
The Raspberry Pi 5 begins thermal throttling at 80°C, so the following values provide
a comfortable safety margin while keeping the fan off most of the time:

```bash
pifan -l 55 -u 65
```

These are also the default values.

## License

[MIT](https://choosealicense.com/licenses/mit/)
