import asyncio
import json
import logging
from datetime import datetime, timezone

from fastapi import WebSocket

from app.v1.core.config import settings

logger = logging.getLogger(__name__)


class WebSocketHub:
    def __init__(self):
        self.connections: list[WebSocket] = []
        self._broadcast_task: asyncio.Task | None = None

    async def connect(self, websocket: WebSocket) -> None:
        await websocket.accept()
        self.connections.append(websocket)

    def disconnect(self, websocket: WebSocket) -> None:
        if websocket in self.connections:
            self.connections.remove(websocket)

    async def broadcast(self, message: dict) -> None:
        dead: list[WebSocket] = []
        payload = json.dumps(message)
        for ws in self.connections:
            try:
                await ws.send_text(payload)
            except Exception:
                dead.append(ws)
        for ws in dead:
            self.disconnect(ws)

    async def notify_event(self, event_type: str, data: dict) -> None:
        await self.broadcast(
            {
                "type": "event",
                "data": {"event_type": event_type, **data},
                "timestamp": int(datetime.now(timezone.utc).timestamp() * 1000),
            }
        )

    async def start_broadcast_loop(self) -> None:
        if self._broadcast_task is None:
            self._broadcast_task = asyncio.create_task(self._heartbeat_loop())

    async def _heartbeat_loop(self) -> None:
        while True:
            await asyncio.sleep(settings.ws_broadcast_interval_ms / 1000)
            if self.connections:
                await self.broadcast(
                    {
                        "type": "dashboard_update",
                        "data": {"tick": True},
                        "timestamp": int(datetime.now(timezone.utc).timestamp() * 1000),
                    }
                )


ws_hub = WebSocketHub()
