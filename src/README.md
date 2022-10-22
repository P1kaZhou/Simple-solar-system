# Simple Solar System

This is a project a did as part of my computer graphics and interfaces courses.

To see the solar system, go under src, and write these 3 commands

```
(cd src)
cmake -B build
make -C build
./build/tpOpenGL
```

## Some questions you might ask

### Why is 60% of your project HTML? Isn't it a c++ project?
Well yes, but I decided to include the major dependencies in the repo, otherwise CMake will have a hard time, and you too by having to download the dependencies manually and moving them to the right directory.

### I see that there are comments with `meshMercury`, what's that?
I tried to add another planet above 3 but there are texture rendering issues, so...

### Any future plans?
Obviously!