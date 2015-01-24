byteshift_strstr
================

Simple and fast drop-in replacements for the stdlib's strstr() and memmem()
sub-sequence search functions.

Performance
===========

`byteshift_strstr()` can be significantly faster than most sub-string search
algorithms when searching for relatively small sub-strings (such as words) or
when searching through text with a very small alphabet (such as DNA sequences).

If performance is important, benchmarking the relevant function with actual
data on the intended hardware is highly recommended. This should be relatively
easy since these functions have the same interface as their common
counterparts.

It is worth noting that the worst case complexity of the algorithm used by
these functions is `O(n × m)`, where `n` is the length of the string and
`m` the length of the sub-string. This is the same as the naive brute-force
algorithm.

However, in most realistic scenarios it will run with linear complexity
(`O(n)`). `O(n)` is also the best-case complexity of the algorithm, since it
processes every character of the string being searched. The high performance
is achieved by doing very little processing for the majority of characters.

Algorithm and Implementation
----------------------------

The core algorithm is simple. At each index of the searched string, the first
`k` characters are compared with the first `k` characters of the sub-string
being searched for (the "needle"). If these match, the next `len(needle) - k`
characters are compared with the rest of the needle.

The previously mentioned `k` is the number of characters which fit inside a
single `unsigned long` ("`ulong`"). On 64-bit machines this is usually 8,
while on 32-bit machines it is usually either 4 or 8. This was chosen because
comparing two `ulong`-s is a very fast operation, and "packing" characters into
`ulong`-s can also be done very quickly.

The first `k` characters of the needle need to be "packed" into a `ulong` only
once. This, along with finding the needle's length, are the only pre-processing
steps required.

During the search, the "packed" `ulong` containing the first `k` characters at
an index can be quickly updated to contain the first `k` characters at the next
index. This is done by shifting the `unsigned long` 8 bits to the "left" and
then `XOR`-ing or `OR`-ing the `k`-th character after the next index.

If the first `k` characters match, the rest are compared via `strcmp()` or
`memcmp()` (as appropriate).

Other Implementation Details
----------------------------

This implemenation also employs a number of additional optimizations:

* Single-character needles are special-cased, and the search just uses
  `strchr()` / `memchr()`.
* `strchr()` / `memchr()` is called to find the first occurence for the first
  character of the needle in the searched string.
* Needles of length <= `k` are also special cased: no `memcmp()` is required.
* The initial `ulong`-s for the needle and searched string are built very
  efficiently using some "bit magic": the memory of the first `k` bytes is
  interpreted as a `ulong`, and then converted to big-endian format if
  necessary (i.e. if the machine uses little-endian). This is equavalent to
  starting with zero and then shifting it and xoring the next character `k`
  times.

Benchmarks
----------

The algorithm has been benchmarked together with four other algorithms by
searching all occurrences of 100 random Latin words in portions of *Commentarii
de Bello Gallico* by *Julius Caesar*. Each test was run 100 times and the mean
time was taken. The tests were run on a late 2012 MacBook Pro 13" Retina, which
has an *Intel Core i5-3210M* processor.

* `strstr` is from the *GNU C Library* (version 2.19) and uses the *Two Ways
Algorithm*
* `naive strstr` is a naive brute-force implementation
* `Volnitsky's strstr` is
[an algorithm by Leonid Volnitsky](http://volnitsky.com/project/str_search/)
* `fast_strstr` is
[a similar lightweight implementation by Raphael Javaux]
(https://github.com/RaphaelJ/fast_strstr).
* `byteshift_strstr` is this implementation.

Scores were compared to `strstr`.

Sections of 10, 100, 500, 1000, 5000, 10000, 50000 characters of *Bello
Gallico* along with the full text (147277 characters) have been used.

| Algorithm \ Size   | 10                 | 100                | 500                | 1000               |
| ------------------ |:------------------:|:------------------:|:------------------:|:------------------:|
| strstr             |  2.9 us (   1x)    | 12.2 us (   1x)    | 59.0 us (  1x)     | 127 us (  1x)      |
| naive strstr       |  6.0 us ( 2.0x)    | 46.4 us ( 3.8x)    |  229 us (3.9x)     | 459 us (3.6x)      |
| Volnitsky's strstr |  187 us (63.9x)    |  198 us (16.3x)    |  242 us (4.1x)     | 312 us (2.5x)      |
| fast_strstr        |  3.0 us ( 1.0x)    | 12.2 us ( 1.0x)    | 61.0 us (1.0x)     | 113 us (0.9x)      |
| byteshift_strstr   | **3.0 us ( 1.0x)** | **11.8 us (1.0x)** | **52.1 us (0.9x)** | **104 us (0.8x)**  |

| Algorithm \ Size   | 5000               | 10000              | 50000              | 147277             |
| ------------------ |:------------------:|:------------------:|:------------------:|:------------------:|
| strstr             |  668 us (  1x)     |  1.3 ms (  1x)     |  6.7 ms (  1x)     | 19.8 ms (  1x)     |
| naive strstr       |  2.3 ms (3.4x)     |  4.6 ms (3.4x)     | 23.1 ms (3.4x)     | 67.7 ms (3.4x)     |
| Volnitsky's strstr |  811 us (1.2x)     |  1.6 ms (1.2x)     | 12.6 ms (1.9x)     | 72.6 ms (3.7x)     |
| fast_strstr        |  559 us (0.8x)     |  1.1 ms (0.8x)     |  5.5 ms (0.8x)     | 16.3 ms (0.8x)     |
| byteshift_strstr   | **509 us (0.8x)**  | **1.0 ms (0.8x)**  | **5.1 ms (0.8x)**  | **15.0 ms (0.8x)** |

`byteshift_strstr` is faster than all of the alternatives, including the native
`strstr`, for all but the shortest of search strings.

Note the good performance with short search strings. This is since the
algorithm doesn't require complex pre-processing the sub-string.

Benchmark scripts are available [here](benchmark). These have been written in
*Haskell* and require the *Criterion* benchmarking library to run.

If you want to run the benchmarks on your hardware, download and install the
*Glasgow Haskell Compiler* and its library manager *Cabal*. Then go in the
[benchmark/](benchmark) directory and ask `cabal` to install the required
libraries :

```
cabal install --only-dependencies
```

Then compile the benchmark executable using `cabal build`. The resulting
executable should be located at `benchmark/dist/build/benchmark/benchmark`.

License
-------

This algorithm is licensed under the open-source
[MIT license](http://opensource.org/licenses/MIT):

The MIT License (MIT)

Copyright (c) 2014-2015 Tal Einat

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
