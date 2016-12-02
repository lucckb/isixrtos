#!/bin/octave -qf
pkg load signal;
arg_list = argv();
nl = str2num(arg_list{1});
mpy = str2num(arg_list{2});
h = hann(nl)' .* mpy;
printf("%f\n", h );

