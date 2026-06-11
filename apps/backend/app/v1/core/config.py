from pydantic_settings import BaseSettings, SettingsConfigDict


class Settings(BaseSettings):
    model_config = SettingsConfigDict(env_file=".env", extra="ignore")

    database_url: str = "sqlite+aiosqlite:///./edgegate.db"
    local_dev: bool = True
    redis_url: str = "redis://localhost:6379/0"
    secret_key: str = "dev-secret-change-in-production"
    gateway_jwt_secret: str = "gateway-jwt-dev-secret"
    cors_origins: str = "http://localhost:3000,http://localhost"
    dashboard_cache_ttl: int = 5
    ws_broadcast_interval_ms: int = 250

    @property
    def cors_origin_list(self) -> list[str]:
        return [o.strip() for o in self.cors_origins.split(",") if o.strip()]


settings = Settings()
