'''
Author: Jan Chleboun
Date: 09-11-2025
Email: chlebja3@fel.cvut.cz
Description: 

'''
#
# import torch, torch.nn.functional as F
# import time
#
# # 1. Fast
# x = torch.randn(10_000_000, device='cuda')
# start = time.time()
# y = F.relu(x).square() 
# print("Took:", time.time() - start)
#
# # 2. Slow
# x = torch.randn(10_000_000, device='cuda')
# start = time.time()
# y = torch.pow(F.relu(x), 2.5)
# print("Took:", time.time() - start)
#
# # 3. Optimized
# x = torch.randn(10_000_000, device='cuda')
# start = time.time()
# y = F.relu(x).square() * torch.sqrt(F.relu(x))  # ~0.15 ms
# print("Took:", time.time() - start)

import torch
import torch.nn.functional as F
import time

torch.manual_seed(0)
device = "cuda"
x = torch.randn(10_000_000, device=device)

# --- define variants ---
def relu_square(x):
    return F.relu(x).square()

def relu_pow2(x):
    return torch.pow(F.relu(x), 2)

def relu_pow2_5(x):
    return torch.pow(F.relu(x), 2.5)

def relu_square_sqrt(x):
    x = F.relu(x)
    return x.square() * torch.sqrt(x)

# --- compile all ---
compiled_variants = {
    "relu().square()": torch.compile(relu_square),
    "relu().pow(2)": torch.compile(relu_pow2),
    "relu().pow(2.5)": torch.compile(relu_pow2_5),
    "relu().square() * sqrt()": torch.compile(relu_square_sqrt),
}

# --- timing function ---
def benchmark(fn, x, iters=100):
    # warm-up (compile + cache kernels)
    for _ in range(10):
        fn(x)
    torch.cuda.synchronize()

    start = time.perf_counter()
    for _ in range(iters):
        y = fn(x)
    torch.cuda.synchronize()
    end = time.perf_counter()

    return (end - start) / iters * 1000  # ms per iteration

# --- run benchmarks ---
results = {}
for name, fn in compiled_variants.items():
    t = benchmark(fn, x)
    results[name] = t
    print(f"{name:<30} {t:.4f} ms")


