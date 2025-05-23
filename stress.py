#!/usr/bin/env python3
import asyncio
import aiohttp
import time
import argparse
from collections import Counter
import statistics

async def fetch(session, url, index, results):
    start_time = time.time()
    try:
        async with session.get(url) as response:
            data = await response.read()
            elapsed = time.time() - start_time
            results['times'].append(elapsed)
            results['status'][response.status] += 1
            results['bytes'] += len(data)
            print(f"[{index}] Status: {response.status} Time: {elapsed:.4f}s Size: {len(data)} bytes")
            return True
    except Exception as e:
        elapsed = time.time() - start_time
        results['times'].append(elapsed)
        results['errors'] += 1
        print(f"[{index}] Request failed: {e}")
        return False

async def run_stress_test(url, total_requests, concurrency):
    connector = aiohttp.TCPConnector(limit=concurrency)
    results = {
        'times': [],
        'status': Counter(),
        'errors': 0,
        'bytes': 0
    }
    
    async with aiohttp.ClientSession(connector=connector) as session:
        tasks = []
        for i in range(total_requests):
            # Small delay to avoid exact same timestamp for all requests
            task = asyncio.create_task(fetch(session, url, i, results))
            tasks.append(task)
            
            # Optionally throttle task creation to avoid overwhelming event loop
            if i % 1000 == 999:
                await asyncio.sleep(0.01)
        
        print(f"\nLaunching {total_requests} requests with concurrency {concurrency}...")
        start = time.time()
        await asyncio.gather(*tasks)
        end = time.time()
    
    # Calculate statistics
    success_count = sum(results['status'].values())
    total_time = end - start
    rps = total_requests / total_time
    
    # Print results
    print("\n" + "=" * 60)
    print(f"STRESS TEST RESULTS:")
    print(f"Target URL: {url}")
    print(f"Total requests: {total_requests}")
    print(f"Concurrency level: {concurrency}")
    print(f"Time taken: {total_time:.2f} seconds")
    print(f"Requests per second: {rps:.2f}")
    print(f"Successful requests: {success_count}")
    print(f"Failed requests: {results['errors']}")
    print(f"Total bytes received: {results['bytes'] / 1024 / 1024:.2f} MB")
    
    if results['times']:
        print(f"\nResponse time statistics:")
        print(f"  Min: {min(results['times']) * 1000:.2f} ms")
        print(f"  Max: {max(results['times']) * 1000:.2f} ms")
        print(f"  Avg: {statistics.mean(results['times']) * 1000:.2f} ms")
        print(f"  Median: {statistics.median(results['times']) * 1000:.2f} ms")
    
    print("\nStatus code distribution:")
    for status, count in sorted(results['status'].items()):
        percentage = (count / total_requests) * 100
        print(f"  {status}: {count} ({percentage:.1f}%)")
    
    print("=" * 60)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Async HTTP Stress Tester")
    parser.add_argument("url", help="Target URL (e.g., http://localhost:8080)")
    parser.add_argument("-n", "--requests", type=int, default=100, help="Total number of requests")
    parser.add_argument("-c", "--concurrency", type=int, default=10, help="Number of concurrent requests")
    parser.add_argument("-t", "--timeout", type=float, default=10.0, help="Request timeout in seconds")

    args = parser.parse_args()

    asyncio.run(run_stress_test(args.url, args.requests, args.concurrency))
