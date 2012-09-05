death
=====

Conway's Game of Death

hosts:
 Darwin 10.8.0 i386 (clang 1.6)
 Linux 2.6.32 i686 (gcc 4.4.3)
 Linux 3.2.0 x86_64 (gcc 4.6.3)
 Linux 3.2.0 x86_64 (clang 3.0)

obfuscations TODO:
 use a temporary member of the game struct instead of return values for neighbors

bugs:
 flicker at high speed (in spite of double-buffering, presumably because can't lock to refresh rate)
 you don't actually get to *see* the collision, which is confusing if you're killed by a new cell

nice to add:
 score keeping
 color
 sound
 leave a trail
 background graphic layer
 in-game instructions

obfuscations
 magic numbers
 arithmetic properties of magic numbers
