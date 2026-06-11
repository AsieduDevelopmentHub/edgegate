import json
import random
import uuid

from locust import HttpUser, between, task

GATEWAY_UUID = "00000000-0000-0000-0000-000000000001"


class TelemetryUser(HttpUser):
    wait_time = between(0.01, 0.05)

    def on_start(self):
        r = self.client.post(f"/v1/auth/token?gateway_uuid={GATEWAY_UUID}")
        if r.status_code == 200:
            self.token = r.json()["token"]
        else:
            self.token = ""

    @task(10)
    def ingest_events(self):
        events = []
        for _ in range(10):
            events.append({
                "gateway": GATEWAY_UUID,
                "timestamp": random.randint(1700000000000, 1800000000000),
                "type": random.choice(["dns_query", "device_connected", "gateway_health"]),
                "device": "AA:BB:CC:DD:EE:FF",
                "payload": {"domain": f"example{random.randint(1, 100)}.com"},
            })
        headers = {"Authorization": f"Bearer {self.token}"} if self.token else {}
        self.client.post(
            "/v1/events",
            data=json.dumps({"events": events}),
            headers={**headers, "Content-Type": "application/json"},
        )

    @task(2)
    def dashboard(self):
        self.client.get("/v1/dashboard")

    @task(1)
    def policies(self):
        self.client.get("/v1/policies")
