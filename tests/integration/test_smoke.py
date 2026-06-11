"""Integration smoke tests — run against a live stack (docker compose up)."""

import os

import httpx
import pytest

BASE_URL = os.getenv("EDGEGATE_API_URL", "http://localhost:8000")


@pytest.fixture
def api():
    return httpx.Client(base_url=BASE_URL, timeout=10.0)


@pytest.mark.integration
def test_health(api):
    r = api.get("/health")
    assert r.status_code == 200


@pytest.mark.integration
def test_dashboard_overview(api):
    r = api.get("/v1/dashboard")
    assert r.status_code == 200
    data = r.json()
    assert "active_devices" in data


@pytest.mark.integration
def test_policy_lifecycle(api):
    r = api.post("/v1/policies", json={
        "type": "domain",
        "pattern": "test-blocked.example.com",
        "action": "deny",
        "enabled": True,
    })
    assert r.status_code == 200

    r = api.post("/v1/policies/deploy", json={})
    assert r.status_code == 200
    data = r.json()
    assert "signature" in data
    assert data["deployed"] >= 1
