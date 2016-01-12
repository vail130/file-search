# filesearch ![Build Status](https://travis-ci.org/vail130/filesearch.svg)

## Setup

```sh
./configure && make
```

## Run Tests

```sh
make check
```

## Install

```sh
make install
```

## Usage

```sh
filesearch /path-to-search/ "*.js"
filesearch /path-to-search/ "^.+\.js$" -r
filesearch /path-to-search/ "*.js" -t f
```

### Show stats

Executing `filesearch` with the `--stats` or `-s` flag will print stats for each result.
These will be the following separated by tabs:

  1. Full file path
  2. File size in bytes
  3. Unix time of last access
  4. Unix time of last modification
  5. Unix time of last status change
  6. User ID of owner
  7. Group ID of owner
  8. ID of device containing file

See http://linux.die.net/man/2/stat for details.

