# Pelican Engine

The Pelican Engine is a Vulkan game engine/renderer that I'm working on to learn more about Vulkan and game engines.

This project is currently very much WIP. The current master branch *should* be able to build and run without crashing. Of course, oversights might always occur.


## Cloning

The repository contains some dependencies that are added as submodules, so when cloning make sure to run this command:

```
git clone --recurse-submodules https://github.com/SeppahBaws/PelicanEngine.git
```
You can also update a pre-existing clone:

```
git submodule init
git submodule update
```

## Building

I use [Premake](https://premake.github.io/) to generate the project files, because it allows for quick and easy cross-platform and cross-IDE project file generation.

Although most of the code will probably work cross-platform, I have only tested the current code on Windows, with Visual Studio.

To generate the project files, please run the GenerateSolution.bat` file. This will call Premake and generate a Visual Studio solution, from which you can then build and run the project.

**Important:** for development, I also use [Visual Leak Detector](https://oneiric.github.io/vld/) to detect for memory leaks. If you wish to not use this, please remove the `--use-vld` option in the `GenerateSolution.bat` file.

## Goal

My main goal of this engine is to be able to make a small game with it, by utilizing an editor to edit all the properties, and a scripting system for dynamic gameplay elements.

## License

This project is licensed under the MIT license, which can be found in the [LICENSE.md](https://github.com/SeppahBaws/PelicanEngine/blob/master/LICENSE.md) file in this repository


## External links

Portfolio project: https://seppedekeyser.be/projects/vulkan-renderer
