#include <chrono>
#include <iostream>
#include <string_view>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <string>
#include <random>
#include <map>
#include <unordered_map>
#include <deque>

#include "small_vector.hpp"
#include "sorted_vector_map.hpp"
#include "ring_buffer.hpp"

using namespace std;

static size_t g_sink = 0;
static unsigned int g_seed = 123;

template <class F>
double bench_ms(string_view name, F&& f, int iters = 1) {
    using clock = chrono::steady_clock;
    auto t0 = clock::now();
    for (int i = 0; i < iters; ++i) f();
    auto t1 = clock::now();
    double ms = chrono::duration<double, milli>(t1 - t0).count();
    cout << name << ": " << ms << " ms\n";
    return ms;
}

// ----- benchmarki --------------

void b_smallvector_many_tiny() {
    constexpr size_t N = 8;
    constexpr size_t K = 8;
    constexpr size_t R = 300'000;

    for (size_t rep = 0; rep < R; ++rep) {
        SmallVector<int, N> v;
        for (size_t i = 0; i < K; ++i) v.push_back((int)i);
        g_sink += (size_t)v.back();
    }
}

void b_stdvector_many_tiny() {
    constexpr size_t K = 8;
    constexpr size_t R = 300'000;

    for (size_t rep = 0; rep < R; ++rep) {
        vector<int> v;
        for (size_t i = 0; i < K; ++i) v.push_back((int)i);
        g_sink += (size_t)v.back();
    }
}

void b_smallvector_reuse() {
    constexpr size_t N = 8;
    constexpr size_t K = 8;
    constexpr size_t R = 2'000'000;

    SmallVector<int, N> v;
    for (size_t rep = 0; rep < R; ++rep) {
        for (size_t i = 0; i < K; ++i) v.push_back((int)i);
        g_sink += (size_t)v.back();
        for (size_t i = 0; i < K; ++i) v.pop_back();
    }
}

void b_stdvector_reuse() {
    constexpr size_t K = 8;
    constexpr size_t R = 2'000'000;

    vector<int> v;
    for (size_t rep = 0; rep < R; ++rep) {
        for (size_t i = 0; i < K; ++i) v.push_back((int)i);
        g_sink += (size_t)v.back();
        for (size_t i = 0; i < K; ++i) v.pop_back();
    }
}

// sorted_vector_map vs normal map / unordered_map
void b_sortedvectormap_lookup_heavy() {
    constexpr int N = 128;
    constexpr int OPS = 600'000;

    SortedVectorMap<int,int> m;
    for (int i = 0; i < N; ++i) m.insert(i, i*i);

    mt19937 rng(g_seed);
    uniform_int_distribution<int> dist(0, N-1);

    for (int i = 0; i < OPS; ++i) {
        int k = dist(rng);
        if (m.contains(k)) g_sink += (size_t)m.at(k);
    }
}

void b_stdmap_lookup_heavy() {
    constexpr int N = 128;
    constexpr int OPS = 600'000;

    map<int,int> m;
    for (int i = 0; i < N; ++i) m.emplace(i, i*i);

    mt19937 rng(g_seed);
    uniform_int_distribution<int> dist(0, N-1);

    for (int i = 0; i < OPS; ++i) {
        int k = dist(rng);
        auto it = m.find(k);
        if (it != m.end()) g_sink += (size_t)it->second;
    }
}

void b_unordered_lookup_heavy() {
    constexpr int N = 128;
    constexpr int OPS = 600'000;

    unordered_map<int,int> m;
    m.reserve((size_t)N);
    for (int i = 0; i < N; ++i) m.emplace(i, i*i);

    mt19937 rng(g_seed);
    uniform_int_distribution<int> dist(0, N-1);

    for (int i = 0; i < OPS; ++i) {
        int k = dist(rng);
        auto it = m.find(k);
        if (it != m.end()) g_sink += (size_t)it->second;
    }
}

// ring_buffer vs normal deque
void b_ringbuffer_push_pop() {
    constexpr size_t CAP = 1024;
    constexpr size_t OPS = 8'000'000;

    RingBuffer<int> rb(CAP);
    for (size_t i = 0; i < CAP; ++i) rb.push((int)i);

    for (size_t i = 0; i < OPS; ++i) {
        g_sink += (size_t)rb.front();
        rb.pop();
        rb.push((int)i);
    }
}

void b_deque_push_pop() {
    constexpr size_t CAP = 1024;
    constexpr size_t OPS = 8'000'000;

    deque<int> dq;
    dq.resize(CAP);
    for (size_t i = 0; i < CAP; ++i) dq[i] = (int)i;

    for (size_t i = 0; i < OPS; ++i) {
        g_sink += (size_t)dq.front();
        dq.pop_front();
        dq.push_back((int)i);
    }
}

// -------cli  -----------------

struct Bench {
    string_view name;
    void (*fn)();
    int iters;
};

static const Bench kBenches[] = {
    {"sv.many_tiny",        &b_smallvector_many_tiny, 3},
    {"stdv.many_tiny",      &b_stdvector_many_tiny,   3},
    {"sv.reuse",            &b_smallvector_reuse,     2},
    {"stdv.reuse",          &b_stdvector_reuse,       2},

    {"svm.lookup_heavy",    &b_sortedvectormap_lookup_heavy, 3},
    {"map.lookup_heavy",    &b_stdmap_lookup_heavy,          3},
    {"umap.lookup_heavy",   &b_unordered_lookup_heavy,       3},

    {"rb.push_pop",         &b_ringbuffer_push_pop,   2},
    {"deque.push_pop",      &b_deque_push_pop,        2},
};

static void print_usage(const char* prog) {
    cout
        << "Usage:\n" << "  " << prog << " --list\n" << "  " << prog << " --all\n"
        << "  " << prog << " --bench NAME [--bench NAME2 ...]\n"
        << "  " << prog << " --filter SUBSTR\n"
        << "  " << prog << " --seed N\n"
        << "\nExamples:\n"  << "  " << prog << " --list\n"
        << "  " << prog << " --bench sv.many_tiny\n"
        << "  " << prog << " --bench rb.push_pop --bench deque.push_pop\n"
        << "  " << prog << " --filter lookup\n"
        << "  " << prog << " --all --seed 999\n";
}

static void list_benches() {
    for (const auto& b : kBenches) cout << b.name << "\n";
}

static const Bench* find_bench(string_view name) {
    for (const auto& b : kBenches) {
        if (b.name == name) return &b;
    }
    return nullptr;
}

static bool contains_substr(string_view s, string_view sub) {
    return s.find(sub) != string_view::npos;
}

static void warmup() {
    bench_ms("warmup", []{
        int x = 0;
        for (int i = 0; i < 1'000'000; ++i) x += i;
        g_sink += (size_t)x;
    }, 1);
    cout << "\n";
}

int main(int argc, char** argv) {
    if (argc <= 1) {
        warmup();
        cout << "seed=" << g_seed << "\n\n";
        for (const auto& b : kBenches) bench_ms(b.name, b.fn, b.iters);
        cerr << "sink=" << g_sink << "\n";
        return 0;
    }

    bool do_list = false;
    bool do_all = false;
    vector<string_view> chosen;
    string_view filter;

    for (int i = 1; i < argc; ++i) {
        const char* a = argv[i];

        if (strcmp(a, "--help") == 0 || strcmp(a, "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(a, "--list") == 0) {
            do_list = true;
        } else if (strcmp(a, "--all") == 0) {
            do_all = true;
        } else if (strcmp(a, "--bench") == 0) {
            if (i + 1 >= argc) {
                cerr << "error: --bench requires NAME\n";
                return 1;
            }
            chosen.emplace_back(argv[++i]);
        } else if (strcmp(a, "--filter") == 0) {
            if (i + 1 >= argc) {
                cerr << "error: --filter requires SUBSTR\n";
                return 1;
            }
            filter = argv[++i];
        } else if (strcmp(a, "--seed") == 0) {
            if (i + 1 >= argc) {
                cerr << "error: --seed requires NUMBER\n";
                return 1;
            }
            g_seed = static_cast<unsigned int>(stoul(argv[++i]));
        } else {
            cerr << "error: unknown arg: " << a << "\n";
            print_usage(argv[0]);
            return 1;
        }
    }

    if (do_list) {
        list_benches();
        return 0;
    }

    warmup();
    cout << "seed=" << g_seed << "\n\n";

    if (do_all || (chosen.empty() && filter.empty())) {
        for (const auto& b : kBenches) bench_ms(b.name, b.fn, b.iters);
        cerr << "sink=" << g_sink << "\n";
        return 0;
    }

    if (!filter.empty()) {
        bool any = false;
        for (const auto& b : kBenches) {
            if (contains_substr(b.name, filter)) {
                bench_ms(b.name, b.fn, b.iters);
                any = true;
            }
        }
        if (!any) cerr << "no benches match filter: " << filter << "\n";
        cerr << "sink=" << g_sink << "\n";
        return 0;
    }

    for (auto name : chosen) {
        const Bench* b = find_bench(name);
        if (!b) {
            cerr << "unknown bench: " << name << "\n";
            cerr << "hint: run --list\n";
            return 1;
        }
        bench_ms(b->name, b->fn, b->iters);
    }

    cerr << "sink=" << g_sink << "\n";
    return 0;
}
