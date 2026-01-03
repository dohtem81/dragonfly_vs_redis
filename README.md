# Dragonfly & Redis Pub/Sub Docker Setup

This project demonstrates difference between a Docker-based pub/sub system using Redis and Dragonfly with 5 containers:
- 1 Dragonfly instance
- 1 Redis instance
- 3 C++ programs (2 publishers, 1 subscriber)

## Architecture

- **Dragonfly** (port 6379): High-performance Redis-compatible data store
- **Redis** (port 6380): Traditional Redis instance
- **cpp-pub-dragonfly**: Publishes messages to Dragonfly on drgonfly instance
- **cpp-pub-redis**: Publishes messages to Redis on redis instance
- **cpp-sub**: Subscribes to both channels simultaneously using separate threads

## Prerequisites

- Docker
- Docker Compose

## Usage

### Start all containers:
```bash
docker-compose up --build
```

### Start in detached mode:
```bash
docker-compose up -d --build
```

### Stop all containers:
```bash
docker-compose down
```

## How It Works

1. **Publishers**: Each publisher connects to its respective data store and publishes a message every 2 seconds with a timestamp
2. **Subscriber**: The subscriber maintains two persistent connections (one to Dragonfly, one to Redis) and listens for messages on both channels using separate threads
3. **Messages**: Include sequence numbers and timestamps to track message flow

## Network

All containers communicate over a bridge network called `pubsub_network`.

## Environment Variables

Each container uses environment variables for configuration:
- `HOST`: Server hostname
- `PORT`: Server port
- `CHANNEL`: Pub/sub channel name

## Running Tests

This is a test setup without healthcheck dependencies between containers. Tests must be run manually to ensure controlled conditions.

### Test Procedure

1. **Start the subscriber first:**
    ```bash
    docker exec -it cpp-sub /app/sub_both
    ```
    The subscriber will display summarized results in its console.

2. **Run publishers sequentially** (not simultaneously) to ensure identical test conditions:
    
    **First, test Redis:**
    ```bash
    docker exec -it cpp-pub-redis /app/pub_combined ${REDIS_HOST} ${REDIS_PORT} ${TEST_CHANNEL} ${REDIS_PUB_NAME} ${MESSAGE_COUNT} ${THREAD_COUNT}
    ```
    
    **Wait for completion, then test Dragonfly:**
    ```bash
    docker exec -it cpp-pub-dragonfly /app/pub_combined ${DRAGONFLY_HOST} ${DRAGONFLY_PORT} ${TEST_CHANNEL} ${DRAGONFLY_PUB_NAME} ${MESSAGE_COUNT} ${THREAD_COUNT}
    ```

3. **View results** in the subscriber container console.

### Configuration

Test parameters are defined in the `.env` file.

**Note:** This is a simplified testing approach (KISS principle) and not intended for production use.
