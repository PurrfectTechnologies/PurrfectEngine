# PurrfectEngine

PurrfectEngine is a game/application engine made in C++ designed for beginners and experts. It is developed by 2 humans ([CatDev](https://github.com/RealCatDev) and [ImagineForgee](https://github.com/ImagineForgee)).

> [!NOTE]
> PurrfectEngine is still under development, things may and probably will change.

## Usage

> [!NOTE]
> This is how we imagine PurrfectEngine to look like at it's release, things might not look like that at the moment you're reading this.

Unlike cigarettes, PurrfectEngine is designed to be good for anyone, whether you are a beginner, intermediate or an expert.

- Expert: Ultimately, PurrfectEngine is just a C++ API like [raylib](https://github.com/raysan5/raylib) that has capabilities of creating a window, rendering geometry, playing audio, etc.
- Beginner: But above this API (will) lie an editor that will make work easier for those not experienced with PurrfectEngine. But worry not! When PurrfectEngine will have a full release, you should be able to build an editor for your game that uses PurrfectEngine, exciting! Right?
- Intermediate: Lastly, there's an intermediate. We're creating a programming language for (but not limited only to) PurrfectEngine. It will be built into PurrfectEngine and used in it's editor.

## Building

To build this project you have to probably clone this repo, which you can do by running this command:
```bash
$ git clone --recurse-submodules https://github.com/PurrfectEngine/PurrfectEngine.git
```
Don't worry, it won't download malware on your machine (at least I want you to think it won't ;3).
Now that you have PurrfectEngine installed on your disk you need to build `nob`.
```bash
$ cc -o nob nob.c
```
Now that blah blah, just run this:
```bash
$ ./nob init
```
Yet again, you don't have to worry, it will just build the CMake project (if you don't believe me just check [this](./nob.c) and ask yourself if you really want this engine).

### Commands

Let's say that you're **really** lazy and just want chunk of commands to paste and be good to go, laziness is a sin but here you go:
```bash
$ git clone --recurse-submodules https://github.com/PurrfectEngine/PurrfectEngine.git
$ cc -o nob nob.c
$ ./nob init
```

## Dependencies

Even though possible, PurrfectEngine doesn't implement everything, that's where [dependencies](./dependencies) come in to action. Bellow is a list of every dependency PurrfectEngine depends on:
- C compiler
- [nob.h](https://github.com/tsoding/ht/blob/master/nob.h)
- CMake
- probably something to build this project with.
- [GLFW](https://github.com/glfw/glfw)
- [Vulkan](https://github.com/KhronosGroup/Vulkan-Headers)
- [glm](https://github.com/g-truc/glm)
- [assimp](https://github.com/assimp/assimp)
- [stb](https://github.com/nothings/stb)
- [openal-soft](https://github.com/kcat/openal-soft)
- [libsndfile](https://github.com/libsndfile/libsndfile)
- [minimp3](https://github.com/lieff/minimp3)
- [entt](https://github.com/skypjack/entt)

## Issues

We're only humans, we make mistakes, if you've spoted one, opening a new issue would be appreciated.

[Here](https://docs.github.com/en/issues/tracking-your-work-with-issues/creating-an-issue)'s an official GitHub blog about creating an issue.

## Team

Our team (as mentioned above) consists at the moment of 2 humans (not much), if you'd like to join our team you can always DM me on Discord: real_catdev

Members:
- CatDev: Creator of PurrfectEngine I guess.
- ImagineForgee: Really important person for PurrfectEngine.

## Ending

I (CatDev) would like to thank you for reading this, I know for sure younger me wouldn't read this (and probably present me neither :p), I can't end this README without mentioning people that this project wouldn't be possible without:
- First (and probably the most obvious) is ImagineForgee, without him I probably wouldn't even try to create this engine (not to mention the progress I've made thanks to him).
- Second, is God, he's creator of me, you and everything around us, this note is probably the best way to thank him for the abilities that I have from the finite other ways that I could.
- Third is Tsoding, I just love his streams, every stream fills me with emotions, whether it's when his code works first time and chat spams "stfu", or whether it's when he misses such an obvious mistake that it makes me so mad :3.
- Even though God knew me before I was born, I wouldn't be here without my mom who showed me the way world works, I would be no one without her.

Again, thank you very much for reading this... Even tho I don't have power to know whether you did or didn't it really matters to me (mostly because I am writing this at 2:30 am instead going to get some needed sleep), have a good day and may God be with you.
