# EverDrive-64 PRO Dev Sources

This repository contains libraries, tools and code examples for development targeting the [EverDrive-64 PRO](https://krikzz.com/our-products/cartridges/everdrive-64-pro.html).

Other supplementary resources are available from [here](https://krikzz.com/pub/support/everdrive-64/pro-series/)

## Repo Contents

| Path                      | Description   |
|---                        |---            |
| [edio](./edio/)           | Example ROM for low level cartridge hardware access: SD card, USB, memory, etc. |
| [edio-cmd](./edio-cmd/)   | Test USB command scripts for the [edio](./edio/) ROM. |
| [edlink.py](./edlink.py)  | Basic cross-platform launcher script for [edlink.exe](./edlink.exe). Requires Mono runtime on Linux and macOS. |
| [edlink.exe](./edlink.exe)| USB utility for communication with EverDrive over USB, latest application source code is available [here](https://github.com/krikzz/edlink). |

---

![EverDrive-64 PRO](./ed64.jpg)
