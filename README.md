# SPLC

Sustech Programming Language Compiler

[![Build Status](https://github.com/ziqin/splc/workflows/CI/badge.svg)](https://github.com/ziqin/splc/actions)

## Build Dependencies

- Lexer: [Flex](https://github.com/westes/flex)
- Parser generator: [Bison](https://www.gnu.org/software/bison/)

## Try It

``` sh
git clone https://github.com/ziqin/splc.git
cd splc/
make
./bin/splc ./test/test_1_r01.spl
```

## References for Development

- <https://www.epaperpress.com/lexandyacc/download/flex.pdf>
- <https://www.gnu.org/software/bison/manual/bison.pdf>
