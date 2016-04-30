# LDoom Lua API

LDoom is scriptable with Lua, and there is a lot of functionality exposed
through Lua. The idea is to do anything that is not performance sensitive in Lua,
and some things that are performance sensitive in Lua anyways for ease of
development. Anything exposed to Lua can also be done in C.

## Events

LDoom exposes the main game loop as a number of callbacks. The callbacks are all
under the global table `levent`.

* `levent.load()`
* `levent.unload()`
* `levent.update(dt)`
* `levent.draw()`
* `levent.mouse(button, x, y)`
* `levent.keyboard(key, action, scancode)`
* `levent.resize(width, height)`
* `levent.tick()`

## Functions

## Modules

### ldoom.console

### ldoom.text

### ldoom.math
