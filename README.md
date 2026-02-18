# Custom C++ Containers Benchmark

Minimal C++ project implementing custom STL-like containers and benchmarking them against standard library equivalents.

## Containers

- `SmallVector<T, N>` – vector with Small Buffer Optimization (SBO)
- `SortedVectorMap<K, V>` – sorted vector-based associative container
- `RingBuffer<T>` – fixed-capacity circular buffer

## Benchmark Runner

CLI-based benchmark suite comparing custom containers with:

- `std::vector`
- `std::map`
- `std::unordered_map`
- `std::deque`

### Features

- Reproducible RNG seed (default is 123)
- Custom seed via `--seed`
- Selective benchmark execution

## Usage

### Run all benchmarks
```bash
./bench --all
```

### List available benchmarks

```bash
./bench --list
```

### Run specific benchmark

```bash
./bench --bench sv.many_tiny
```

### Run multiple benchmarks

```bash
./bench --bench rb.push_pop --bench deque.push_pop
```

### Filter benchmarks by name

```bash
./bench --filter lookup
```

### Change random seed

```bash
./bench --all --seed 768
```

## Goal

The goal of this project is to explore

- Algorithmic complexity vs constant factors
- Performance trade-offs between custom and STL containers
- Allocation strategies

In general the focus is on performance experimentation and understanding container design.
