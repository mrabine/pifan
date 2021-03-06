# pifan
[![Build Status](https://github.com/mrabine/pifan/workflows/build/badge.svg)](https://github.com/mrabine/pifan/actions?query=workflow%3Abuild)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/mrabine/pifan.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/mrabine/pifan/context:cpp)
[![GitHub Releases](https://img.shields.io/github/release/mrabine/pifan.svg)](https://github.com/mrabine/pifan/releases/latest)
[![GitHub License](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/mrabine/pifan/blob/main/LICENSE)

Raspberry Pi fan controller daemon

Start fan when CPU temperature is high and stop it when CPU temperature is low.

## Download

To download the latest source do this:
```bash
git clone https://github.com/mrabine/pifan.git
```

## Configuration

To configure pifan do this:
```bash
cd pifan
mkdir build && cd build
cmake ..
```

## Installation

To install pifan do this:
```bash
make && sudo make install
```

## Enabling

To enable pifan do this:
```bash
sudo systemctl enable pifan
sudo systemctl start pifan
```

## Usage

**pifan** [options]

**-c**\
&emsp;print the cpu temperature\
**-h**\
&emsp;show available options\
**-i interval**\
&emsp;sleep interval (default: 2 seconds)\
**-l threshold**\
&emsp;lower threshold (default: 60&deg;C)\
**-n**\
&emsp;don't fork into background\
**-p pin**\
&emsp;gpio pin (default: 14)\
**-u threshold**\
&emsp;upper threshold (default: 70&deg;C)\
**-v**\
&emsp;print version

## License

[MIT](https://choosealicense.com/licenses/mit/)
