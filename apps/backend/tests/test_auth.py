import pytest

from app.security.jwt import create_gateway_token, rotate_gateway_token, verify_gateway_token


def test_create_and_verify_token():
    token = create_gateway_token("00000000-0000-0000-0000-000000000001")
    uuid = verify_gateway_token(token)
    assert uuid == "00000000-0000-0000-0000-000000000001"


def test_rotate_token():
    token = create_gateway_token("00000000-0000-0000-0000-000000000001")
    new_token = rotate_gateway_token(token)
    assert new_token is not None
    assert verify_gateway_token(new_token) == "00000000-0000-0000-0000-000000000001"


@pytest.mark.asyncio
async def test_issue_token_endpoint(client):
    response = await client.post("/v1/auth/token?gateway_uuid=00000000-0000-0000-0000-000000000001")
    assert response.status_code == 200
    assert "token" in response.json()


@pytest.mark.asyncio
async def test_device_login_endpoint(client):
    response = await client.post(
        "/v1/auth/device-login",
        json={
            "gateway_uuid": "00000000-0000-0000-0000-000000000001",
            "gateway_name": "edgegate-01",
        },
    )
    assert response.status_code == 200
    data = response.json()
    assert "token" in data
    assert data["gateway_uuid"] == "00000000-0000-0000-0000-000000000001"
    assert verify_gateway_token(data["token"]) == "00000000-0000-0000-0000-000000000001"
