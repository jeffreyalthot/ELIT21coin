# ELIT21coin

Prototype C++ d'une blockchain légère avec compression/décompression déterministe des blocs, négociation de codec côté réseau et validation renforcée de l'intégrité de chaîne.

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

## Capacités implémentées

- Chaîne avec bloc genesis, contrôle `index`, `previous_hash` et hash calculé.
- Compression transport avec codec `RLE` ou `RAW`.
- Négociation de codec selon les capacités du pair distant.
- Rejet des blocs réseau corrompus, non supportés ou incohérents temporellement.
- Garde-fou configurable sur la taille maximale décompressée des blocs réseau.


## Options CMake

- `-DELIT21_BUILD_TESTS=ON|OFF` active ou non la compilation des tests.
- `-DELIT21_ENABLE_SANITIZERS=ON` active ASan/UBSan (hors MSVC).
