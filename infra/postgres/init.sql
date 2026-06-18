-- ==========================================================
-- EdgeGate Database Initialization
--
-- Purpose:
-- Initial PostgreSQL bootstrap script executed during
-- container initialization.
--
-- Responsibilities:
-- • Enable required PostgreSQL extensions
-- • Configure database defaults
-- • Provide partition management utilities
--
-- Executed Automatically By:
-- docker-entrypoint-initdb.d
-- ==========================================================


-- ==========================================================
-- Extensions
-- ==========================================================

-- UUID generation support
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";

-- Fast text search and similarity matching
CREATE EXTENSION IF NOT EXISTS "pg_trgm";


-- ==========================================================
-- Database Defaults
-- ==========================================================

SET TIME ZONE 'UTC';


-- ==========================================================
-- Partition Utility
--
-- Creates monthly partitions dynamically.
--
-- Example:
--
-- SELECT create_monthly_partition(
--     'events',
--     CURRENT_DATE
-- );
--
-- Result:
-- events_2026_06
--
-- Parent table MUST already exist:
--
-- CREATE TABLE events (
--     ts TIMESTAMP NOT NULL
-- ) PARTITION BY RANGE (ts);
--
-- ==========================================================

CREATE OR REPLACE FUNCTION create_monthly_partition(
    parent_table TEXT,
    partition_date DATE
)

RETURNS VOID

LANGUAGE plpgsql

AS $$

DECLARE

    partition_name TEXT;

    start_date DATE;

    end_date DATE;

BEGIN

    -- ----------------------------------------
    -- Normalize to month boundaries
    -- ----------------------------------------

    start_date :=
        date_trunc(
            'month',
            partition_date
        )::DATE;

    end_date :=
        (
            start_date
            + INTERVAL '1 month'
        )::DATE;

    partition_name :=
        format(
            '%s_%s',
            parent_table,
            to_char(start_date, 'YYYY_MM')
        );


    -- ----------------------------------------
    -- Validate parent table exists
    -- ----------------------------------------

    IF NOT EXISTS (

        SELECT 1
        FROM pg_tables
        WHERE tablename = parent_table

    ) THEN

        RAISE EXCEPTION
            'Parent table "%" does not exist',
            parent_table;

    END IF;


    -- ----------------------------------------
    -- Create partition if missing
    -- ----------------------------------------

    IF NOT EXISTS (

        SELECT 1
        FROM pg_class
        WHERE relname = partition_name

    ) THEN

        EXECUTE format(
            '
            CREATE TABLE %I
            PARTITION OF %I
            FOR VALUES
            FROM (%L)
            TO (%L)
            ',
            partition_name,
            parent_table,
            start_date,
            end_date
        );

        RAISE NOTICE
            'Created partition: %',
            partition_name;

    ELSE

        RAISE NOTICE
            'Partition already exists: %',
            partition_name;

    END IF;

END;

$$;


-- ==========================================================
-- Future Partition Automation
--
-- Optional:
--
-- SELECT create_monthly_partition(
--     'events',
--     CURRENT_DATE + INTERVAL '1 month'
-- );
--
-- Can later be scheduled using:
-- • pg_cron
-- • backend scheduler
-- • migration runner
--
-- ==========================================================


-- ==========================================================
-- Initialization Complete
-- ==========================================================

DO $$
BEGIN
    RAISE NOTICE
    'EdgeGate PostgreSQL initialization complete';
END;
$$;