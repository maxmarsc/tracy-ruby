# tracy-ruby
`tracy-ruby` is a Ruby binding for the [Tracy profiler Client](https://github.com/wolfpld/tracy).

This extension is a work in progress. It currently targets Tracy v0.13.1.

## Installation
I do not plan to make it a ruby gem, so you will have to build it and install it yourself.

*Contributions are welcome though!*

### Linux / MacOS
#### Requirements
You will need:
 - CMake >= 3.28
 - A working ruby installation (>= 3.0). This must match the target ruby version you want to use the extension with.
 - Ninja (cross-platform, faster than make)
 - git

#### Build
```bash
git clone --recurse-submodules https://github.com/maxmarsc/tracy-ruby.git && cd tracy-ruby
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -G Ninja .
cmake --build build
cmake --install build --prefix <YOUR_INSTALL_DIR> # to easily retrive tracy_ruby & libTracyClient
```

You should find both `tracy_ruby.so` and `libTracyClient.so` in `<YOUR_INSTALL_DIR>/lib`.

These should be installed as relocatable libraries, so you can move them around as long as you keep in the same folder.

### Windows
#### Requirements
You will need:
 - CMake >= 3.28
 - Ruby + Devkit >= 3.0.x
 - Ninja (cross-platform, faster than make)
 - git
 - scoop (optional)

You can install all these using `scoop`:
```bat
scoop install cmake ninja git
REM for ruby 32-bit 3.0.0 - adapt to your needs
scoop install -a 32 ruby@3.0.0
```

#### Build
```bat
git clone --recurse-submodules https://github.com/maxmarsc/tracy-ruby.git && cd tracy-ruby
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON ^
      -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
cmake --build build
cmake --install build --prefix <YOUR_INSTALL_DIR>
```

You should find both `tracy_ruby.dll` and `libTracyClient.dll` in `<YOUR_INSTALL_DIR>/lib`.

These should be installed as relocatable libraries, so you can move them around as long as you keep in the same folder.

## Instrument your ruby code with Tracy
This binding provide a limited set of Tracy's features so far:
 - zones
 - frame marks
 - messages
 - on-demande start/stop of the profiler

To learn more about these, I invite you to read the [Tracy v0.13.1 documentation](https://github.com/wolfpld/tracy/releases/download/v0.13.1/tracy.pdf)

### Ruby code example:
```ruby
require 'tracy_ruby'

# Profile a specific block of code, called a zone
def my_very_expensive_function
  Tracy.zone("my_very_expensive_function") {
    # ...
  }
end

# Start the profiler
Tracy.startup

# Run my game loop - mark the frame for FPS display in Tracy
while running
  wait_for_next_frame

  # Write a Tracy message
  Tracy.message("Rotating the sprite")
  sprite.x += ax_x * 0.05
  sprite.y += ax_y * 0.05
  sprite.angle = (sprite.angle + 5) % 360 if rotating

  # Run the profiled function
  my_very_expensive_function

  # Mark the end of the frame
  Tracy.frame_mark
end

# Shutdown the profiler
Tracy.shutdown
```

## More about Tracy
### Selection options
This extension is built with the following options:
 - `TRACY_ENABLE` (default: ON) - Enable Tracy support
 - `TRACY_ON_DEMAND` (default: ON) - Enable profiling only when there’s an established connection with the
server
 - `TRACY_DELAYED_INIT` & `TRACY_MANUAL_LIFETIME` (REQUIRED) - Provide a way to manually start and stop the profiling

Be very careful when sharing the `libTracyClient.so` library with other applications/libraries. These must use the exact same options
or Tracy might invoke a dragon to burn down your favorite book shop. Or worse, it might malfunction.

### Tracy Client vs Tracy Server
The Tracy Client is the library that you link to your application. It collects profiling data and sends it to the Tracy Server.

The Tracy Server is a separate application that receives the profiling data and displays it in a user-friendly way. See the [Tracy repository](https://github.com/wolfpld/tracy) on how to build/download and use the Tracy Server. **You must make sure to match the version of the Tracy Server with the version of the Tracy Client you are using.**
