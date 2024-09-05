[English](README.md) / [日本語](README.ja.md)

---

# Live2D Cubism Core

This folder contains header and platform-specific library files for developing native applications.

What's the Core anyway? It's a small library that allows you to load Cubism models into your application.


## Getting Started

You probably want to play back animations and want to display your models on screen, too.

[Go here](https://github.com/Live2D/CubismNativeSamples) for further information.


## File Structure

```
Core
├─ dll          # Shared (Dynamic) Library files
├─ include      # Header file
└─ lib          # Static Library files
```


## Library List

| Platform | Architecture | dll | lib | Path | Note |
| --- | --- | --- | --- | --- | --- |
| Windows | x86 | ✓ | ✓ | windows/x86 |   |
| Windows | x86_64 | ✓ | ✓ | windows/x86_64 |   |

### Windows Static Library

Libraries are located under the VC++ version name directory.

Below is the Visual Studio version for the VC++ version.

| VC++ version | Visual Studio version |
| ---: | --- |
| 120 | Visual Studio 2013 |
| 140 | Visual Studio 2015 |
| 141 | Visual Studio 2017 |
| 142 | Visual Studio 2019 |
| 143 | Visual Studio 2022 |

Also, the suffix library (`_MD`, `_MDd`, `_MT`, `_MTd`) refers to the [Use Run-Time Library] option used to compile the library.

### Calling convention

When using the dynamic library of *Windows/x86*, explicitly use `__stdcall` as the calling convention.
