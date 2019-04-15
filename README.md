# Thesis Toolkit

Welcome to this repository! Since you are here, I am assuming that Computer Arithmetic is of interest to you 
(i.e. the art and science of using transistors to do computations, in silicone).

Moreover, I am guessing that you are just as fascinated (as I am) with the paper-and-pencil digital equivalent of
of doing division and (not so often taught nowadays) the square root. This family of algorithms is formally
known as the digit-recurrence family of algorithms.

In my M.Sc. thesis, entitled *"Incorporating Multiplication Into Digit-Recurrence Division to Calculate
`sqrt(a*b)`"* ([linked here](https://core.ac.uk/download/pdf/146514396.pdf)), I have delved into the derivation 
and the hardware implementation of a a fused multiplier and a square rooting unit (just in case, you thought
having a such a machine instruction would change your life, obviously I did!).

Any such unit, while based on the digit-recurrence family of algorithms, need a "guessing table", usually
stored within a ROM (read-only memory) on-board. This ROM is formally called the "SRT table".

Basically, this *SRT Table* saves the same purpose "trial and error" serve when dividing on paper, and it looks
something like this below:

## Introduction
