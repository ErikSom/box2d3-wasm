# Box2d3-wasm

Let's try and use box2d v3 on the web.

```bash
git clone --recurse-submodules https://github.com/Birch-san/box2d3-wasm.git
```

## Building

Install [Emscripten](https://emscripten.org/docs/getting_started/downloads.html).

Let's use Node 20 since that's what Emscripten's using.  

```bash
cd box2d
./build_emscripten.sh
```

This will output a Debug build:

```
box2d/build/bin/test.js
box2d/build/bin/test.wasm
```

I think specifically this is for running unit tests, so not sure if it'll have everything we need. Also we'll want a Release build and to know what symbols we can bind to.