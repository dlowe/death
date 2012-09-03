death
=====

Conway's Game of Death

hosts:
 Darwin 10.8.0 i386 (clang 1.6)
 Linux 2.6.32 i686 (gcc 4.4.3)
 Linux 3.2.0 x86_64 (gcc 4.6.3)
 Linux 3.2.0 x86_64 (clang 3.0)

TODO:
 portability
 see if it compiles on ancient irix
 see if it works with toledo/layer.c, or windows x11

obfuscations TODO:
 rearrange state enum so that state_playing() can be modulus
 use a temporary member of the game struct instead of return values for neighbors
 rearrange event handler to be a single expression

shrinking TODO:
 compact representation of the sprites
 compact representation of splashes

bugs:
 hard-coded green color
 flicker at high speed (in spite of double-buffering, presumably because can't lock to refresh rate)
 you don't actually get to *see* the collision, which is confusing if you're killed by a new cell

nice to add:
 score keeping
 sound
 leave a trail
 background graphic layer
 in-game instructions
