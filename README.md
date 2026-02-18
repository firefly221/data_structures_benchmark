# Custom C++ Data Structures Benchmark

Minimal C++ project implementing custom data structures and comparing them with normal std ones.

## Containers

- `SmallVector<T, N>` – vector with Small Buffer Optimization
- `SortedVectorMap<K, V>` – sorted vector based associative container
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
- Selection of benchmarks data and algorithms to test

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
./bench --all --seed 999
```

## Goal

The goal of this project is to explore

- Allocation strategies
- Algorithmic complexity vs constant factors
- Performance trade-offs between custom and std:: containers

In general the focus is on performance experimentation and understanding container design.
