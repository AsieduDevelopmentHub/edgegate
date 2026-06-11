import pytest


def pytest_configure(config):
    config.addinivalue_line("markers", "integration: marks tests requiring live docker stack")
