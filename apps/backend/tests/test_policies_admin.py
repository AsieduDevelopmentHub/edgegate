import pytest


@pytest.mark.asyncio
async def test_policy_crud(client):
    create = await client.post(
        "/v1/policies",
        json={"type": "domain", "pattern": "block.me", "action": "deny", "enabled": True},
    )
    assert create.status_code == 200
    policy_id = create.json()["id"]

    update = await client.patch(
        f"/v1/policies/{policy_id}",
        json={"pattern": "block.updated", "enabled": False},
    )
    assert update.status_code == 200
    body = update.json()
    assert body["pattern"] == "block.updated"
    assert body["enabled"] is False

    listed = await client.get("/v1/policies")
    assert listed.status_code == 200
    assert any(p["id"] == policy_id for p in listed.json())

    delete = await client.delete(f"/v1/policies/{policy_id}")
    assert delete.status_code == 200

    missing = await client.patch(f"/v1/policies/{policy_id}", json={"enabled": True})
    assert missing.status_code == 404


@pytest.mark.asyncio
async def test_admin_system_and_clear(client):
    await client.post(
        "/v1/policies",
        json={"type": "domain", "pattern": "temp.test", "action": "deny"},
    )

    system = await client.get("/v1/admin/system")
    assert system.status_code == 200
    counts = system.json()["counts"]
    assert "policies" in counts
    assert counts["policies"] >= 1

    bad = await client.post(
        "/v1/admin/clear",
        json={"scopes": ["policies"], "confirm": "NOPE"},
    )
    assert bad.status_code == 400

    cleared = await client.post(
        "/v1/admin/clear",
        json={"scopes": ["policies"], "confirm": "CLEAR"},
    )
    assert cleared.status_code == 200
    assert cleared.json()["cleared"]["policies"] >= 1


@pytest.mark.asyncio
async def test_admin_export(client):
    res = await client.get("/v1/admin/export?scopes=policies")
    assert res.status_code == 200
    assert res.headers["content-type"].startswith("application/json")
    assert "edgegate-export" in res.headers.get("content-disposition", "")
