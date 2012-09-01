death
=====

Conway's Game of Death

TODO:
 portability

obfuscations TODO:
 rearrange state enum so that state_playing() can be modulus

shrinking TODO:
 compact representation of the sprites
 compact representation of splashes

bugs:
 hard-coded green color
 flicker at high speed (in spite of double-buffering)
 you don't actually get to *see* the collision, which is confusing if you're killed by a new cell

nice to add:
 score keeping
 sound
 a trail
 background graphic layer
 in-game instructions
