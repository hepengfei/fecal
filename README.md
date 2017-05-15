# FEC-AL
## Forward Error Correction at the Application Layer in C

FEC-AL is a simple, portable, fast library for Forward Error Correction.
FEC-AL is a block codec derived from the Siamese streaming FEC library.
From a block of input data it generates recovery symbols that can be used
to recover lost originals.

* It requires that data pieces are all a fixed size.
* It can take as input an unlimited number of input blocks.
* It can generate an unlimited stream of recovery symbols used for decoding.

The main limitation of the software is that it gets slower as O(N^^2) in
the number of inputs or outputs.  In trade, the encoder overhead is unusually
low, and the decoder is extremely efficient when recovering from a small number
of losses.  It may be the best choice based on practical evaluation.

#### Why fecal matters:

It supports an unlimited number of inputs and outputs, similar to a Fountain Code,
but it is designed as a Convolutional Code.  This means that it does not perform
well with a large number of losses.  It is faster than existing error correction
code (ECC) software when the loss count is expected to be small.

#### Encoder API:

```
#include "fecal.h"
```

For full documentation please read `fecal.h`.

`fecal_init()` : Initialize library.
`fecal_encoder_create()`: Create encoder object.
`fecal_encode()`: Encode a recovery symbol.
`fecal_free()`: Free encoder object.

#### Decoder API:

```
#include "fecal.h"
```

For full documentation please read `fecal.h`.

`fecal_init()` : Initialize library.
`fecal_decoder_create()`: Create a decoder object.
`fecal_decoder_add_original()`: Add original data to the decoder.
`fecal_decoder_add_recovery()`: Add recovery data to the decoder.
`fecal_decode()`: Attempt to decode with what has been added so far, returning recovered data.
`fecal_decoder_get()`: Read back original data after decode.
`fecal_free()`: Free decoder object.

#### Benchmarks:

For random losses in 2 MB of data split into 1000 equal-sized 2KB pieces:

```
Encoder(2 MB in 1000 pieces, 1 losses): Input=6968.64 MB/s, Output=6.96864 MB/s, (Encode create: 7225.69 MB/s)
Decoder(2 MB in 1000 pieces, 1 losses): Input=9083.06 MB/s, Output=9.08307 MB/s, (Overhead = 0 pieces)

Encoder(2 MB in 1000 pieces, 2 losses): Input=7181.33 MB/s, Output=14.5063 MB/s, (Encode create: 7663.72 MB/s)
Decoder(2 MB in 1000 pieces, 2 losses): Input=7365.13 MB/s, Output=14.7303 MB/s, (Overhead = 0.02 pieces)

Encoder(2 MB in 1000 pieces, 3 losses): Input=6805.5 MB/s, Output=20.4165 MB/s, (Encode create: 7526.72 MB/s)
Decoder(2 MB in 1000 pieces, 3 losses): Input=6312.93 MB/s, Output=18.9388 MB/s, (Overhead = 0 pieces)

Encoder(2 MB in 1000 pieces, 4 losses): Input=6751.28 MB/s, Output=27.0726 MB/s, (Encode create: 7645.84 MB/s)
Decoder(2 MB in 1000 pieces, 4 losses): Input=6387.12 MB/s, Output=25.5485 MB/s, (Overhead = 0.0100002 pieces)

Encoder(2 MB in 1000 pieces, 5 losses): Input=6502.16 MB/s, Output=32.5108 MB/s, (Encode create: 7645.55 MB/s)
Decoder(2 MB in 1000 pieces, 5 losses): Input=5982.11 MB/s, Output=29.9106 MB/s, (Overhead = 0 pieces)

Encoder(2 MB in 1000 pieces, 6 losses): Input=6014.13 MB/s, Output=36.3855 MB/s, (Encode create: 7238.51 MB/s)
Decoder(2 MB in 1000 pieces, 6 losses): Input=5520.74 MB/s, Output=33.1245 MB/s, (Overhead = 0.0500002 pieces)

Encoder(2 MB in 1000 pieces, 7 losses): Input=6284.56 MB/s, Output=44.1176 MB/s, (Encode create: 7764.88 MB/s)
Decoder(2 MB in 1000 pieces, 7 losses): Input=5601.61 MB/s, Output=39.2113 MB/s, (Overhead = 0.02 pieces)

Encoder(2 MB in 1000 pieces, 8 losses): Input=5854.97 MB/s, Output=46.8398 MB/s, (Encode create: 7388.25 MB/s)
Decoder(2 MB in 1000 pieces, 8 losses): Input=5492.54 MB/s, Output=43.9403 MB/s, (Overhead = 0 pieces)

Encoder(2 MB in 1000 pieces, 9 losses): Input=5843.34 MB/s, Output=52.6485 MB/s, (Encode create: 7645.84 MB/s)
Decoder(2 MB in 1000 pieces, 9 losses): Input=5221.11 MB/s, Output=46.99 MB/s, (Overhead = 0.0100002 pieces)

Encoder(2 MB in 1000 pieces, 10 losses): Input=5728.53 MB/s, Output=57.3998 MB/s, (Encode create: 7610.06 MB/s)
Decoder(2 MB in 1000 pieces, 10 losses): Input=5172.24 MB/s, Output=51.7224 MB/s, (Overhead = 0.0200005 pieces)

Encoder(2 MB in 1000 pieces, 11 losses): Input=5590.65 MB/s, Output=61.4972 MB/s, (Encode create: 7667.83 MB/s)
Decoder(2 MB in 1000 pieces, 11 losses): Input=5012.53 MB/s, Output=55.1378 MB/s, (Overhead = 0 pieces)

Encoder(2 MB in 1000 pieces, 13 losses): Input=5382.13 MB/s, Output=70.0753 MB/s, (Encode create: 7687.28 MB/s)
Decoder(2 MB in 1000 pieces, 13 losses): Input=4790.53 MB/s, Output=62.2769 MB/s, (Overhead = 0.0200005 pieces)

Encoder(2 MB in 1000 pieces, 15 losses): Input=5065.47 MB/s, Output=76.0327 MB/s, (Encode create: 7556.01 MB/s)
Decoder(2 MB in 1000 pieces, 15 losses): Input=4490.45 MB/s, Output=67.3567 MB/s, (Overhead = 0.0100002 pieces)

Encoder(2 MB in 1000 pieces, 16 losses): Input=4874.6 MB/s, Output=77.9936 MB/s, (Encode create: 7390.71 MB/s)
Decoder(2 MB in 1000 pieces, 16 losses): Input=4279.45 MB/s, Output=68.4712 MB/s, (Overhead = 0 pieces)

Encoder(2 MB in 1000 pieces, 18 losses): Input=4707.99 MB/s, Output=84.7438 MB/s, (Encode create: 7515.69 MB/s)
Decoder(2 MB in 1000 pieces, 18 losses): Input=4008.9 MB/s, Output=72.1602 MB/s, (Overhead = 0 pieces)

Encoder(2 MB in 1000 pieces, 20 losses): Input=4619.15 MB/s, Output=92.4754 MB/s, (Encode create: 7679.31 MB/s)
Decoder(2 MB in 1000 pieces, 20 losses): Input=3858.4 MB/s, Output=77.1679 MB/s, (Overhead = 0.0200005 pieces)

Encoder(2 MB in 1000 pieces, 25 losses): Input=4176.24 MB/s, Output=104.448 MB/s, (Encode create: 7576.33 MB/s)
Decoder(2 MB in 1000 pieces, 25 losses): Input=3374.22 MB/s, Output=84.3554 MB/s, (Overhead = 0.0100002 pieces)

Encoder(2 MB in 1000 pieces, 30 losses): Input=3731.27 MB/s, Output=111.976 MB/s, (Encode create: 7418.12 MB/s)
Decoder(2 MB in 1000 pieces, 30 losses): Input=2950.2 MB/s, Output=88.506 MB/s, (Overhead = 0.0100002 pieces)

Encoder(2 MB in 1000 pieces, 35 losses): Input=3542.46 MB/s, Output=124.021 MB/s, (Encode create: 7610.64 MB/s)
Decoder(2 MB in 1000 pieces, 35 losses): Input=2702.99 MB/s, Output=94.6048 MB/s, (Overhead = 0.00999832 pieces)

Encoder(2 MB in 1000 pieces, 40 losses): Input=3365.53 MB/s, Output=134.621 MB/s, (Encode create: 7846.52 MB/s)
Decoder(2 MB in 1000 pieces, 40 losses): Input=2410.42 MB/s, Output=96.4169 MB/s, (Overhead = 0 pieces)

Encoder(2 MB in 1000 pieces, 50 losses): Input=2658.13 MB/s, Output=132.933 MB/s, (Encode create: 6889.42 MB/s)
Decoder(2 MB in 1000 pieces, 50 losses): Input=1917.88 MB/s, Output=95.8938 MB/s, (Overhead = 0.00999832 pieces)

Encoder(2 MB in 1000 pieces, 60 losses): Input=2573.04 MB/s, Output=154.408 MB/s, (Encode create: 7578.92 MB/s)
Decoder(2 MB in 1000 pieces, 60 losses): Input=1612.62 MB/s, Output=96.757 MB/s, (Overhead = 0.00999832 pieces)

Encoder(2 MB in 1000 pieces, 70 losses): Input=2141.83 MB/s, Output=149.95 MB/s, (Encode create: 6861.77 MB/s)
Decoder(2 MB in 1000 pieces, 70 losses): Input=1325.65 MB/s, Output=92.7957 MB/s, (Overhead = 0.0100021 pieces)

Encoder(2 MB in 1000 pieces, 80 losses): Input=2052.65 MB/s, Output=164.212 MB/s, (Encode create: 7454.34 MB/s)
Decoder(2 MB in 1000 pieces, 80 losses): Input=1112 MB/s, Output=88.9601 MB/s, (Overhead = 0 pieces)

Encoder(2 MB in 1000 pieces, 90 losses): Input=1926.69 MB/s, Output=173.402 MB/s, (Encode create: 7593.01 MB/s)
Decoder(2 MB in 1000 pieces, 90 losses): Input=972.81 MB/s, Output=87.5529 MB/s, (Overhead = 0 pieces)

Encoder(2 MB in 1000 pieces, 100 losses): Input=1814.67 MB/s, Output=181.467 MB/s, (Encode create: 7866.27 MB/s)
Decoder(2 MB in 1000 pieces, 100 losses): Input=861.668 MB/s, Output=86.1668 MB/s, (Overhead = 0 pieces)

Encoder(2 MB in 1000 pieces, 110 losses): Input=1617.09 MB/s, Output=177.88 MB/s, (Encode create: 7514.28 MB/s)
Decoder(2 MB in 1000 pieces, 110 losses): Input=740.198 MB/s, Output=81.4218 MB/s, (Overhead = 0 pieces)

Encoder(2 MB in 1000 pieces, 120 losses): Input=1485.21 MB/s, Output=178.225 MB/s, (Encode create: 7274.05 MB/s)
Decoder(2 MB in 1000 pieces, 120 losses): Input=645.417 MB/s, Output=77.4501 MB/s, (Overhead = 0 pieces)
```

#### Credits

This software was written entirely by myself ( Christopher A. Taylor mrcatid@gmail.com ), making shit happen. If you find it useful and would like to buy me a coffee, consider tipping.
