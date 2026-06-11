import pytest
from fastapi.testclient import TestClient

from app.main import app
from app.security.jwt import create_gateway_token

GW = "00000000-0000-0000-0000-000000000001"


@pytest.fixture
def client():
    with TestClient(app) as c:
        yield c


def test_ingest_device_connected(client):
    token = create_gateway_token(GW)
    body = {
        "events": [
            {
                "timestamp": 1,
                "type": "device_connected",
                "device": "04:33:C2:FB:F6:D0",
                "priority": 1,
                "payload": {"raw": '{"mac":"04:33:C2:FB:F6:D0"}'},
            }
        ]
    }
    r = client.post("/v1/events", json=body, headers={"Authorization": f"Bearer {token}"})
    assert r.status_code == 200
    assert r.json()["accepted"] == 1


def test_ingest_dns_query(client):
    token = create_gateway_token(GW)
    body = {
        "events": [
            {
                "timestamp": 1,
                "type": "dns_query",
                "device": "04:33:C2:FB:F6:D0",
                "priority": 1,
                "payload": {
                    "raw": '{"domain":"example.com","blocked":false,"latency_ms":12}',
                },
            }
        ]
    }
    r = client.post("/v1/events", json=body, headers={"Authorization": f"Bearer {token}"})
    assert r.status_code == 200
    assert r.json()["accepted"] == 1
