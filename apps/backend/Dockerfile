# -------------------------------------------------
# EdgeGate Backend
# Production Dockerfile
# -------------------------------------------------

FROM python:3.12-slim

# ----------------------------------------------
# Environment
# ----------------------------------------------

ENV PYTHONDONTWRITEBYTECODE=1 \
    PYTHONUNBUFFERED=1 \
    PIP_NO_CACHE_DIR=1

WORKDIR /app

# ----------------------------------------------
# System packages
# ----------------------------------------------

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        curl \
        gcc \
        build-essential \
        libpq-dev \
    && rm -rf /var/lib/apt/lists/*

# ----------------------------------------------
# Dependencies
# ----------------------------------------------

COPY pyproject.toml ./

# Optional if available
COPY poetry.lock* ./

RUN pip install --upgrade pip && \
    pip install -e .

# ----------------------------------------------
# Application
# ----------------------------------------------

COPY app ./app
COPY alembic ./alembic
COPY alembic.ini .

# ----------------------------------------------
# Security
# ----------------------------------------------

RUN addgroup --system appgroup && \
    adduser --system appuser --ingroup appgroup

RUN chown -R appuser:appgroup /app

USER appuser

# ----------------------------------------------
# Networking
# ----------------------------------------------

EXPOSE 8000

# ----------------------------------------------
# Healthcheck
# ----------------------------------------------

HEALTHCHECK \
    --interval=30s \
    --timeout=5s \
    --retries=3 \
CMD curl -f http://localhost:8000/health || exit 1

# ----------------------------------------------
# Startup
# ----------------------------------------------

CMD ["sh","-c","alembic upgrade head && exec uvicorn app.main:app --host 0.0.0.0 --port 8000"]