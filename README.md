# ELIT21coin

Prototype C++ d'une blockchain légère avec compression/décompression déterministe des blocs.

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Exécution de démonstration

```bash
./build/elit21_demo
```

## Tests

```bash
ctest --test-dir build --output-on-failure
```
