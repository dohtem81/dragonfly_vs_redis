# Redis vs Dragonfly — Pub/Sub Performance Comparison

This repository contains a practical pub/sub performance comparison between Redis and Dragonfly, using identical C++ subscribers and a shared Docker-based environment.

The goal is not to declare a "winner", but to observe how architectural differences show up under a simple, reproducible workload.

## Motivation

Redis is a widely adopted, battle-tested in-memory data store. Dragonfly is a newer, Redis-compatible system designed to better utilize modern multi-core CPUs.

I wanted to answer a straightforward question:

> Under identical pub/sub workloads, how do Redis and Dragonfly compare in subscriber throughput?

## Test Overview

- One publisher sends messages to a single channel
- Two independent subscribers (Redis and Dragonfly)
- Subscribers consume messages until a fixed maximum count is reached
- Wall-clock completion time is measured

Both subscribers:

- Use the same C++ implementation
- Subscribe to the same channel name
- Run in the same Docker environment

## Environment

- Docker / Docker Compose
- Redis running on port 6380
- Dragonfly running on port 6379
- Identical client logic and message limits

**Note:** Absolute performance is affected by Docker networking and host scheduling. Results are intended for relative comparison only.

## Results

### Pub/Sub Subscriber Completion Time

| System     | Completion Time |
|------------|-----------------|
| Redis      | 55,186 ms       |
| Dragonfly  | 31,127 ms       |

**Observation:**  
Dragonfly completed consumption of the same number of messages approximately **1.77× faster** than Redis under identical conditions.

### Raw Output

```
[REDIS] Subscriber stopped. Completed in 55186 ms.
[DRAGONFLY] Subscriber stopped. Completed in 31127 ms.
```

## Interpretation

This result is consistent with Dragonfly's multi-threaded architecture, which can more effectively utilize multiple CPU cores for pub/sub message fan-out compared to Redis' single-threaded event loop.

Even in a Dockerized environment — where performance differences are often muted — the throughput gap is still visible.

## Caveats & Limitations

- This test measures subscriber completion time, not per-message latency
- Only a single pub/sub workload is evaluated
- Results should not be generalized to all Redis or Dragonfly use cases
- No persistence, replication, or clustering features are enabled

This repository is intended as a focused, minimal comparison, not a comprehensive benchmark suite.

## Reproducibility

Build and start services:

```bash
docker-compose up --build
```

Observe subscriber logs for completion times.

All configuration and client code are included in this repository to allow independent reproduction of results.

## Future Work

- Scaling tests with increasing message counts
- Latency distribution analysis
- Multi-subscriber fan-out scenarios
- Non-pub/sub workloads (e.g. GET/SET, pipelines)

## Summary

Under a simple, controlled pub/sub workload, Dragonfly demonstrated significantly faster subscriber throughput than Redis. This highlights how architectural decisions become visible even in minimal, real-world tests.
