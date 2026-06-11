import json
import logging
import sys
from datetime import datetime, timezone


class JSONFormatter(logging.Formatter):
    def format(self, record: logging.LogRecord) -> str:
        log = {
            "ts": datetime.now(timezone.utc).isoformat(),
            "level": record.levelname,
            "logger": record.name,
            "message": record.getMessage(),
        }
        if record.exc_info:
            log["exception"] = self.formatException(record.exc_info)
        return json.dumps(log)


def setup_logging() -> None:
    """JSON logs for app.* only — do not replace uvicorn's root handlers (breaks access logs)."""
    handler = logging.StreamHandler(sys.stdout)
    handler.setFormatter(JSONFormatter())
    app_log = logging.getLogger("app")
    app_log.handlers.clear()
    app_log.addHandler(handler)
    app_log.setLevel(logging.INFO)
    app_log.propagate = False
