from app.security.signing import sign_config, verify_signature


def test_sign_and_verify():
    config = {"version": 1, "rules": [{"pattern": "ads.example.com", "action": "deny"}]}
    sig = sign_config(config)
    assert verify_signature(config, sig)
    assert not verify_signature({"version": 2}, sig)
