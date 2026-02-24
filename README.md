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
- Rejet des blocs réseau corrompus, non supportés, en régression temporelle ou horodatés trop loin dans le futur.
- Sérialisation de bloc avec champs préfixés par taille pour supporter les payloads contenant des délimiteurs.
- Garde-fou configurable sur la taille maximale décompressée des blocs réseau.
- Mempool locale avec tri des transactions par frais pour la production de blocs.
- Portefeuille local avec signature déterministe, gestion de nonce et contrôle de solde.
- Nœud applicatif (`elit21::Node`) orchestrant wallets + mempool + blockchain.


## Options CMake

- `-DELIT21_BUILD_TESTS=ON|OFF` active ou non la compilation des tests.
- `-DELIT21_ENABLE_SANITIZERS=ON` active ASan/UBSan (hors MSVC).
- `-DELIT21_ENABLE_IPO=ON` active l'optimisation inter-procédurale (LTO) si supportée.
- `-DELIT21_ENABLE_CLANG_TIDY=ON` active `clang-tidy` pendant la compilation si l'outil est disponible.
